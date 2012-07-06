/*
*      This file is part of the tknet project. 
*    which be used under the terms of the GNU General Public 
*    License version 3.0 as published by the Free Software
*    Foundation and appearing in the file LICENSE.GPL included 
*    in the packaging of this file.  Please review the following 
*    information to ensure the GNU General Public License 
*    version 3.0 requirements will be met: 
*    http://www.gnu.org/copyleft/gpl.html
*
*    Copyright  (C)   2012   Zhong Wei <clock126@126.com>  .
*/ 

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
			PROMPT(Usual,"relay has not finished yet, "
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
	struct TkNetMsg               *pRecvMsg = UsrDatRead(pProc);
	struct TkNetMsg               SendingMsg;
	struct NetAddr                *pAddr = &pProc->addr;
	struct pipe                   *pPipe0,*pPipe1;
	char                          AddrText[32];

	if(pRecvMsg)
	{
		if(pRecvMsg->flag == SES_MAINTAIN_FLAG)
		{
			PROMPT(Usual,"~");//indicating session maintaining
			pProc->ifAlive = 1;
				
			goto discard;
		}
		else if(pRecvMsg->flag == SES_DAT_FLAG)
		{
			//not implemented yet.
		}
		else if(pRecvMsg->flag == SES_CMD_FLAG)
		{
			//direct net pipe to cmd pipe
			pPipe0 = PipeFindByName("cmd");

			if(pPipe0)
			{
				if(!ifPipeTo(pProc->pPipe,pPipe0)) //make sure only pipe once
				{
					PipeDirectTo(pProc->pPipe,pPipe0);
				}
			}
			else
			{
				TK_EXCEPTION("finding cmd pipe.");
				goto discard;
			}

			//then find the net pipe to send the result back.
			pPipe0 = PipeFindByName("stdout");

			GetAddrText(pAddr,AddrText);
			pPipe1 = PipeFindByName(AddrText);

			if(pPipe0 && pPipe1)
			{
				if(!ifPipeTo(pPipe0,pPipe1)) //make sure only pipe once
				{
					PipeDirectTo(pPipe0,pPipe1);
				}
			}
			else
			{
				TK_EXCEPTION("finding net or stdout pipe.");
				goto discard;
			}
		}
		else if(pRecvMsg->flag == SES_CHAT_FLAG)
		{
			//enable the other side to print recved.
			pPipe0 = PipeFindByName("stdout");

			GetAddrText(pAddr,AddrText);
			pPipe1 = PipeFindByName(AddrText);

			if(pPipe0 && pPipe1)
			{
				if(!ifPipeTo(pPipe1,pPipe0)) //make sure only pipe once
				{
					PipeDirectTo(pPipe1,pPipe0);
				}
			}
			else
			{
				TK_EXCEPTION("finding net or stdout pipe.");
				goto discard;
			}
			
			//enable the other side send stdin here.
			pPipe0 = PipeFindByName("stdin");

			if(pPipe0)
			{
				if(!ifPipeTo(pPipe0,pPipe1)) //make sure only pipe once
				{
					PipeDirectTo(pPipe0,pPipe1);
				}
			}
			else
			{
				TK_EXCEPTION("finding stdin pipe.");
				goto discard;
			}
		}
		else
		{
			TK_EXCEPTION("session msg flag.");
			goto discard;
		}

		PipeFlow(pProc->pPipe,pRecvMsg->msg.UsrDat,
				pRecvMsg->UsrDatLen,NULL);
	}

discard:

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

	PROMPT(Usual,"session proc ends.\n");
}

static
FLOW_CALLBK_FUNCTION(SessionFlowCallbk)
{
	DEF_AND_CAST(pProc,struct SessionMaintainProcess,pa_pFlowPa);
	DEF_AND_CAST(pFpe,struct FlowPaElse,pa_else);
	struct TkNetMsg               SendingMsg;
	struct NetAddr                *pAddr = &pProc->addr;
	uint                          now = 0;
	uchar                         *pUchar;

	if(pFpe && 0 == strcmp(pFpe->PaName,"uint:SET_FLAG"))
	{
		pUchar = (uchar*)pFpe->pPa;
		SendingMsg.flag = *pUchar;
	}
	else
	{
		SendingMsg.flag = SES_DAT_FLAG;
	}

	SockLocateTa(pProc->pSock,htonl(pAddr->IPv4),pAddr->port);

	while(pa_DataLen > TK_NET_DATA_LEN)
	{
		SendingMsg.UsrDatLen = TK_NET_DATA_LEN;
		memcpy(SendingMsg.msg.UsrDat , pa_pData + now,TK_NET_DATA_LEN);
		SockWrite(pProc->pSock,BYS(SendingMsg));

		pa_DataLen -= TK_NET_DATA_LEN;
		now += TK_NET_DATA_LEN;
	}
	
	SendingMsg.UsrDatLen = pa_DataLen;
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
	
	PROMPT(Usual,"session proc starts.\n");
}

static
FLOW_CALLBK_FUNCTION(CmdModeFlowCallbk)
{
	struct FlowPaElse *pFpe    = tkmalloc(struct FlowPaElse);
	uint              *pSetNum = tkmalloc(uint);

	strcpy(pFpe->PaName,"uint:SET_FLAG");
	pFpe->pPa = pSetNum;

	*pSetNum = SES_CMD_FLAG;

	PipeFlow(pa_pPipe,pa_pData,pa_DataLen,pFpe);//redirection

	tkfree(pSetNum);
	tkfree(pFpe);
}

void
MkCmdModePipe()
{
	struct pipe* pPipe = PipeMap("CmdMode");
	pPipe->FlowCallbk = &CmdModeFlowCallbk;
}

static
FLOW_CALLBK_FUNCTION(ChatModeFlowCallbk)
{
	struct FlowPaElse *pFpe    = tkmalloc(struct FlowPaElse);
	uint              *pSetNum = tkmalloc(uint);

	strcpy(pFpe->PaName,"uint:SET_FLAG");
	pFpe->pPa = pSetNum;

	*pSetNum = SES_CHAT_FLAG;

	PipeFlow(pa_pPipe,pa_pData,pa_DataLen,pFpe);//redirection
		
	tkfree(pSetNum);
	tkfree(pFpe);
}

void
MkChatModePipe()
{
	struct pipe* pPipe = PipeMap("ChatMode");
	pPipe->FlowCallbk = &ChatModeFlowCallbk;
}
