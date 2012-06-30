#include "tknet.h"

struct TkNetMsg*
UsrDatRead(struct SessionMaintainProcess *in_SesProc)
{
	DEF_AND_CAST(pMsg,struct TkNetMsg,in_SesProc->pSock->RecvBuff);
	struct NetAddr FromAddr = GetAddrFromSockAddr(&in_SesProc->pSock->AddrRecvfrom);
	
	if(in_SesProc->pSock->RecvLen <= 0)
	{
		return NULL;
	}
	else if(!ifNetAddrEqual(&FromAddr,&in_SesProc->addr))
	{
		return NULL;
	}
	else if(pMsg->flag == TK_NET_BDG_MSG_FLAG)
	{
		if(pMsg->msg.BdgMsg.info == BRIDGE_MSG_INFO_WAIT_RELAY)
		{
			printf("relay has not finished yet, "
					"This message can't reach the end.\n");
		}

		return NULL;
	}
	else
	{
		return pMsg;
	}
}

static
STEP( SessionMaintain )
{
	struct SessionMaintainProcess *pProc = 
		GET_STRUCT_ADDR(pa_pProc,struct SessionMaintainProcess,proc);
	struct TkNetMsg* pRecvMsg = UsrDatRead(pProc);
	struct TkNetMsg               SendingMsg;
	struct NetAddr                *pAddr = &pProc->addr;

	if(pRecvMsg)
	{
		if(pRecvMsg->flag == SES_MAINTAIN_FLAG)
		{
			printf("~");
			pProc->ifAlive = 1;
		}
		else if(pRecvMsg->flag == SES_DAT_FLAG)
		{
			PipeFlow(pProc->pPipe,pRecvMsg->msg.UsrDat,
					pProc->pSock->RecvLen,NULL);
		}
		else
		{
			TK_EXCEPTION("session msg flag.");
		}

	}

	if(pa_state == PS_STATE_OVERTIME)
	{
		SendingMsg.flag = SES_MAINTAIN_FLAG;
		SockLocateTa(pProc->pSock,htonl(pAddr->IPv4),pAddr->port);
		SockWrite(pProc->pSock,BYS(SendingMsg));
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		if(pProc->ifAlive)
		{
			pProc->ifAlive = 0;
			return PS_CALLBK_RET_REDO;
		}
		else
		{
			return PS_CALLBK_RET_ABORT;
		}
	}

	return PS_CALLBK_RET_GO_ON;
}

static void
SessionOvertimeNotify(struct Process *pa_)
{
	struct SessionMaintainProcess *pProc = 
		GET_STRUCT_ADDR(pa_,struct SessionMaintainProcess,proc);

	ProcessTraceSteps(pa_);
	ProcessFree(pa_);
	//Do not free pProc, because pProc is passed into the flow parameter,
	//pipe will take care of it.
	
	PipeDele(pProc->pPipe);

	printf("session proc ends.\n");
}

static
FLOW_CALLBK_FUNCTION(SessionFlowCallbk)
{
	struct TkNetMsg   SendingMsg;
	DEF_AND_CAST(pFlowPa,struct SessionMaintainProcess,pa_pFlowPa);
	struct SessionMaintainProcess *pProc = pFlowPa;
	struct NetAddr                *pAddr = &pProc->addr;
	uint                          now = 0;

	SendingMsg.flag = SES_DAT_FLAG;
	SockLocateTa(pProc->pSock,htonl(pAddr->IPv4),pAddr->port);

	while(pa_DataLen > TK_NET_DATA_LEN)
	{
		memcpy(SendingMsg.msg.UsrDat,pa_pData + now,TK_NET_DATA_LEN);
		SockWrite(pProc->pSock,BYS(SendingMsg));

		pa_DataLen -= TK_NET_DATA_LEN;
		now += TK_NET_DATA_LEN;
	}

	memcpy(SendingMsg.msg.UsrDat,pa_pData + now,pa_DataLen);
	SockWrite(pProc->pSock,BYS(SendingMsg));
}

void
SessionStart(struct NetAddr pa_addr,struct Sock *pa_pSock,struct ProcessingList *pa_pProcList,
		struct Iterator* pa_pINow,struct Iterator *pa_pIForward)
{
	struct SessionMaintainProcess *pProc;
	struct pipe *pPipe;
	char   AddrText[PIPE_NAME_MAXLEN];

	GetAddrText(&pa_addr,AddrText);
	pPipe = PipeFindByName(AddrText);

	VCK(pPipe != NULL, return);

	pProc = tkmalloc(struct SessionMaintainProcess);
	ProcessCons(&pProc->proc);
	pPipe = PipeMap(AddrText);
	
	pProc->proc.NotifyCallbk = &SessionOvertimeNotify;
	pProc->pSock = pa_pSock;
	pProc->addr = pa_addr;
	pProc->pPipe = pPipe;
	pProc->ifAlive = 0;
		
	pPipe->FlowCallbk = &SessionFlowCallbk;
	pPipe->pFlowPa = pProc;

	PROCESS_ADD_STEP( &pProc->proc , SessionMaintain , g_WaitLevel[3] );
	ProcessSafeStart(&pProc->proc,pa_pProcList,pa_pINow,pa_pIForward);
	
	printf("session proc starts.\n");
}
