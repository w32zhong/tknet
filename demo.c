#include "headers.h"

struct ChatProcess
{
	struct Process proc;
	struct Sock    *pSock;
	struct NetAddr addr;
};

static BOOL sta_ifConnected = 0;

static
STEP( chat )
{
	struct ChatProcess *pProc = GET_STRUCT_ADDR(pa_pProc,struct ChatProcess,proc);
	struct TkNetMsg   *pRecvMsg;
	struct TkNetMsg   SendingMsg;
	char*             pMsgStr;
	struct NetAddr    *pAddr = &pProc->addr;

	pRecvMsg =  ifReadTkMsg(pProc->pSock);

	if(pRecvMsg)
	{
		if(pRecvMsg->flag != TK_NET_BDG_MSG_FLAG)
		{
			pMsgStr = pRecvMsg->msg.DataMsg;
			printf("%s\n",pMsgStr);

			return PS_CALLBK_RET_REDO;
		}
		else if(pRecvMsg->msg.BdgMsg.info == BRIDGE_MSG_INFO_WAIT_RELAY)
		{
			printf("relay has not finished yet, "
					"This message can't reach the end.\n");
		}
	}
	else
	{
		pMsgStr = BkgdGetBackGroundMsg();

		if(pMsgStr)
		{
			SendingMsg.flag = TK_NET_BDG_MSG_FLAG + 1;
			strcpy(SendingMsg.msg.DataMsg,pMsgStr);

			SockLocateTa(pProc->pSock,htonl(pAddr->IPv4),pAddr->port);
			SockWrite(pProc->pSock,BYS(SendingMsg)); 
			
			return PS_CALLBK_RET_REDO;
		}
	}
	
	return PS_CALLBK_RET_GO_ON;
}

static void
ConnectionOvertimeNotify(struct Process *pa_)
{
	struct ChatProcess *pProc = GET_STRUCT_ADDR(pa_,struct ChatProcess,proc);
	tkfree(pProc);
	sta_ifConnected = 0;
	BkgdLeaveSubProcess();
	printf("Connection overtime.\n");
}

void
ON_CONNECT()
{
	struct ChatProcess *pNewProc;

	if(!sta_ifConnected)
	{
		pNewProc = tkmalloc(struct ChatProcess);
		ProcessCons(&pNewProc->proc);
		pNewProc->pSock = pa_pSock;
		pNewProc->addr = pa_addr;
		pNewProc->proc.NotifyCallbk = &ConnectionOvertimeNotify;

		PROCESS_ADD_STEP( &pNewProc->proc , chat , 60000 , 0 );
		ProcessSafeStart(&pNewProc->proc,pa_pProcList,pa_pINow,pa_pIForward);

		sta_ifConnected = 1;
		BkgdEnterSubProcess();
		printf("Connection started.\n");
	}
	else
	{
		printf("There is already a connection.\n");
	}
}
