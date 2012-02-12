#include "headers.h"

STEP( BdgBeginSubServer )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct BridgeMsg  SendingMsg;
	struct BridgeMsg  *pBdgMsg;

	pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_ADDR_FILTER,0,BDG_ADDR(a,pBdgProc));

	if(pBdgMsg)
	{
		if(pBdgMsg->info == BRIDGE_MSG_INFO_WAITING)
		{
			return PS_CALLBK_RET_DONE;
		}
		else if(pBdgMsg->info == BRIDGE_MSG_INFO_CONNECT)
		{
			//in this case, which is very common that client side 
			//quickly start connect without sending 'waiting' message
			return PS_CALLBK_RET_DONE;
		}
	}

	if(pa_state == PS_STATE_FIRST_TIME || 
			pa_state == PS_STATE_OVERTIME )
	{
		SendingMsg.info = BRIDGE_MSG_INFO_RGST_OK;
		BdgMsgWrite(pa_pProc,&SendingMsg,BDG_ADDR(a,pBdgProc));
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( BdgConnectRequireServer )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct BridgeMsg  SendingMsg;
	struct BridgeMsg  *pBdgMsg;
	struct PeerData *pPD = GET_STRUCT_ADDR(pBdgProc,struct PeerData,BdgProc);
	struct PeerData *pFoundPD;

	pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_ADDR_FILTER,0,BDG_ADDR(a,pBdgProc));

	if(pBdgMsg)
	{
		if(pBdgMsg->info == BRIDGE_MSG_INFO_WAITING)
		{
			PeerDataUpdateSeedInfo(pPD,pBdgMsg->Relays);

			SendingMsg.info = BRIDGE_MSG_INFO_ECHO;
			BdgMsgWrite(pa_pProc,&SendingMsg,BDG_ADDR(a,pBdgProc));
			
			return PS_CALLBK_RET_REDO;
		}
		else if(pBdgMsg->info == BRIDGE_MSG_INFO_CONNECT)
		{
			pFoundPD = PeerDataFind(pBdgProc->pPeerDataRoot,pBdgMsg->NameID);

			if(pFoundPD)
			{
				SetPeerByPeerData(&pBdgProc->b ,pFoundPD);
			}
			else
			{
				PeerCons(&pBdgProc->b);
			}

			return PS_CALLBK_RET_DONE;
		}
	}

	if( pa_state == PS_STATE_OVERTIME)
	{
		printf("I am free.\n");
	}
	if(pa_state == PS_STATE_LAST_TIME)
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( BdgConnectRequireReply )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct BridgeMsg  SendingMsg;
	struct BridgeMsg  *pBdgMsg;

	pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_ADDR_FILTER,0,BDG_ADDR(a,pBdgProc));

	if(pBdgMsg)
	{
		if(pBdgMsg->info == BRIDGE_MSG_INFO_WAITING )
		{
			pBdgProc->DecisionFlag = CONNECT_DECISION_FLAG_BEGIN;
			return PS_CALLBK_RET_DONE;
		}
	}

	if(pa_state == PS_STATE_FIRST_TIME || 
			pa_state == PS_STATE_OVERTIME )
	{
		if( pBdgProc->b.addr.port == 0 )
		{
			SendingMsg.info = BRIDGE_MSG_ERR_NO_NAMEID;
		}
		else
		{
			SendingMsg.info = BRIDGE_MSG_INFO_CONNECT_BEGIN;
		}

		BdgMsgWrite(pa_pProc,&SendingMsg,BDG_ADDR(a,pBdgProc));
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( BdgConnectDecision )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	uchar aType = pBdgProc->a.NATType;
	uchar bType = pBdgProc->b.NATType;

	if(CONNECT_DECISION_FLAG_BEGIN == pBdgProc->DecisionFlag)
	{
		if(IF_NEAD_RELAY(aType,bType))
		{
			//...
		}
		else
		{
			if(aType < bType)
			{
				pBdgProc->DecisionPunAddr = pBdgProc->a.addr;
				pBdgProc->DecisionConAddr = pBdgProc->b.addr;
			}
			else
			{
				pBdgProc->DecisionPunAddr = pBdgProc->b.addr;
				pBdgProc->DecisionConAddr = pBdgProc->a.addr;
			}

			pBdgProc->DecisionFlag = CONNECT_DECISION_FLAG_DIRECT;
			pBdgProc->DecisionRelayID = 0;
		}
	}
	else
	{
		TK_EXCEPTION("BdgConnectDecision step flag");
		return PS_CALLBK_RET_ABORT;
	}
	
	return PS_CALLBK_RET_DONE;
}

STEP( BdgPunchingServer )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct BridgeMsg  SendingMsg;
	struct BridgeMsg  *pBdgMsg;

	pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_ADDR_FILTER,0,&pBdgProc->DecisionPunAddr);

	if(pBdgMsg)
	{
		if(pBdgMsg->info == BRIDGE_MSG_INFO_PUNCHING_FINISH)
		{
			return PS_CALLBK_RET_DONE;
		}
	}

	if(pa_state == PS_STATE_FIRST_TIME || 
			pa_state == PS_STATE_OVERTIME )
	{
		SendingMsg.info = BRIDGE_MSG_INFO_PUNCHING;
		SendingMsg.addr = pBdgProc->DecisionConAddr;
		
		BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgProc->DecisionPunAddr);
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		pBdgProc->DecisionFlag = CONNECT_DECISION_FLAG_ERROR;
		return FlagName(pa_pProc,"BdgConnectDecision");
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( BdgConnectAddrServer )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct BridgeMsg  SendingMsg;
	struct BridgeMsg  *pBdgMsg;

	pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_ADDR_FILTER,0,&pBdgProc->DecisionConAddr);

	if(pBdgMsg)
	{
		if(pBdgMsg->info == BRIDGE_MSG_INFO_ESTABLISHED)
		{
			pBdgProc->DecisionFlag = CONNECT_DECISION_FLAG_ESTABLISHED;
			return FlagName(pa_pProc,"BdgConnectDecision");
		}
	}

	if(pa_state == PS_STATE_FIRST_TIME || 
			pa_state == PS_STATE_OVERTIME )
	{
		SendingMsg.info = BRIDGE_MSG_INFO_CONNECT_ADDR;
		SendingMsg.addr = pBdgProc->DecisionPunAddr;
		SendingMsg.RelayID = pBdgProc->DecisionRelayID;
		
		BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgProc->DecisionConAddr);
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		pBdgProc->DecisionFlag = CONNECT_DECISION_FLAG_ERROR;
		return FlagName(pa_pProc,"BdgConnectDecision");
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( BridgeMain )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct NetAddr FromAddr; 
	struct BridgeMsg  *pBdgMsg;
	struct BridgeMsg  SendingMsg;
	struct PeerData   *pPD;

	pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_ANY,0,NULL);

	if(pBdgMsg)
	{
		FromAddr = GetAddrFromSockAddr(&pBdgProc->pSock->AddrRecvfrom);

		switch(pBdgMsg->info)
		{
		case BRIDGE_MSG_INFO_HELLO_BDG:

			SendingMsg.info = BRIDGE_MSG_INFO_IAM_HERE;
			BdgMsgWrite(pa_pProc,&SendingMsg,&FromAddr);

			break;

		case BRIDGE_MSG_INFO_REGISTER:

			if(PeerDataFind(pBdgProc->pPeerDataRoot,pBdgMsg->NameID))
			{
				SendingMsg.info = BRIDGE_MSG_ERR_NAMEID_EXIST;
				BdgMsgWrite(pa_pProc,&SendingMsg,&FromAddr);
			}
			else
			{
				pPD = NewPeerDataWithBdgProc(FromAddr,pBdgMsg->NATType,pBdgMsg->NameID,pBdgProc);
				SetPeerByPeerData(&(pPD->BdgProc.a),pPD);

				if( pBdgMsg->NATType == NAT_T_FULL_CONE ||
					pBdgMsg->NATType == NAT_T_RESTRICTED)
				{
					PeerDataSelectAsSeed(pPD,pBdgProc->pSeedPeerCache);
				}

				PeerDataInsert(pPD,pBdgProc->pPeerDataRoot);
				ProcessSafeStart(&(pPD->BdgProc.proc),pBdgProc->pProcList,pa_pINow,pa_pIForward);
			}

			break;
		}
	}

	return PS_CALLBK_RET_REDO;
}

STEP( BdgClientTryBdgServer )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct BridgeMsg  SendingMsg;
	struct BridgeMsg  *pBdgMsg;
	DEF_AND_CAST(pStepPa , struct BridgeHelloStepPa , pBdgProc->Else);

	if(SockRead(pBdgProc->pSock))
	{
		pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_ADDR_FILTER,0,&pStepPa->addr);

		if(pBdgMsg && pBdgMsg->info == BRIDGE_MSG_INFO_IAM_HERE)
		{
			pStepPa->res = 1;
			return PS_CALLBK_RET_DONE;
		}
		//else deal with 'redirection'
	}

	if(pa_state == PS_STATE_FIRST_TIME || 
			pa_state == PS_STATE_OVERTIME)
	{
		SendingMsg.info = BRIDGE_MSG_INFO_HELLO_BDG;
		BdgMsgWrite(pa_pProc,&SendingMsg,&pStepPa->addr);
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		pStepPa->res = 0;
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( BdgClientRegister )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct BridgeMsg  SendingMsg;
	struct BridgeMsg  *pBdgMsg;
	DEF_AND_CAST(pBCPPa,struct BridgeClientProcPa,pBdgProc->Else);

	pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_ADDR_FILTER,0,BDG_ADDR(s,pBdgProc));

	if(pBdgMsg)
	{
		if(pBdgMsg->info == BRIDGE_MSG_INFO_RGST_OK)
		{
			printf("client register ok\n");
			return PS_CALLBK_RET_DONE;
		}
		else if(pBdgMsg->info == BRIDGE_MSG_ERR_NAMEID_EXIST)
		{
			printf("client register recv name exist\n");
			return PS_CALLBK_RET_ABORT;
		}
	}

	if(pa_state == PS_STATE_FIRST_TIME || 
			pa_state == PS_STATE_OVERTIME)
	{
		SendingMsg.info = BRIDGE_MSG_INFO_REGISTER;
		strcpy(SendingMsg.NameID,pBCPPa->pMyNameID);
		SendingMsg.NATType = pBdgProc->a.NATType;

		BdgMsgWrite(pa_pProc,&SendingMsg,BDG_ADDR(s,pBdgProc));
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( BdgClientWait )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct BridgeMsg  SendingMsg;
	struct BridgeMsg  *pBdgMsg;
	DEF_AND_CAST(pBCPPa,struct BridgeClientProcPa,pBdgProc->Else);
	struct NetAddr FromAddr; 

	pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_ADDR_FILTER,0,BDG_ADDR(s,pBdgProc));
	
	if(NULL == pBdgMsg)
		pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_MSG_FILTER
				,BRIDGE_MSG_INFO_PUNCHING,NULL);
	
	if(NULL == pBdgMsg)
		pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_MSG_FILTER
				,BRIDGE_MSG_INFO_CONNECT_ADDR,NULL);
	
	if(NULL == pBdgMsg)
		pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_MSG_FILTER
				,BRIDGE_MSG_INFO_HELLO,NULL);

	if(pBdgMsg)
	{
		FromAddr = GetAddrFromSockAddr(&pBdgProc->pSock->AddrRecvfrom);

		printf("client recv a BdgMsg when waiting\n");

		if(pBdgMsg->info == BRIDGE_MSG_INFO_ECHO)
		{
			return PS_CALLBK_RET_REDO;
		}
		else if(pBdgMsg->info == BRIDGE_MSG_INFO_PUNCHING)
		{
			printf("Punching ...\n");

			SendingMsg.info = BRIDGE_MSG_INFO_UNKNOWN;

			BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgMsg->addr);
			tkMsSleep(100);
			BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgMsg->addr);
			tkMsSleep(100);
			//send twice to gain the possibility of a valid punching
			
			SendingMsg.info = BRIDGE_MSG_INFO_PUNCHING_FINISH;
			BdgMsgWrite(pa_pProc,&SendingMsg,&FromAddr);

			printf("Punching Finish\n");
		}
		else if(pBdgMsg->info == BRIDGE_MSG_INFO_CONNECT_ADDR)
		{
			printf("Recieved a Connect Address ...\n");
			pBdgProc->sx.addr = FromAddr;
			pBdgProc->b.addr = pBdgMsg->addr;
			return FlagName(pa_pProc,"BdgClientDoConnectAddr");
		}
		else if(pBdgMsg->info == BRIDGE_MSG_INFO_HELLO)
		{
			printf("Nice To Meet you\n");
			pBdgProc->MultiSendTo = FromAddr;
			pBdgProc->MultiSendInfo = BRIDGE_MSG_INFO_HELLO;
			return FlagName(pa_pProc,"BdgClientMultiSendNotify");
		}
	}

	if(pBCPPa->pTargetNameID)
	{
		return PS_CALLBK_RET_DONE;
	}
	else if(pa_state == PS_STATE_OVERTIME)
	{
		SendingMsg.info = BRIDGE_MSG_INFO_WAITING;
		SendingMsg.Relays = 5;

		BdgMsgWrite(pa_pProc,&SendingMsg,BDG_ADDR(s,pBdgProc));
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( BdgClientConnectRequire )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct BridgeMsg  SendingMsg;
	struct BridgeMsg  *pBdgMsg;
	DEF_AND_CAST(pBCPPa,struct BridgeClientProcPa,pBdgProc->Else);

	pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_ADDR_FILTER,0,BDG_ADDR(s,pBdgProc));

	if(pBdgMsg)
	{
		if(pBdgMsg->info == BRIDGE_MSG_INFO_CONNECT_BEGIN ||
				pBdgMsg->info == BRIDGE_MSG_ERR_NO_NAMEID)
		{

			if(pBdgMsg->info == BRIDGE_MSG_ERR_NO_NAMEID)
			{
				printf("remote server can't find %s\n",pBCPPa->pTargetNameID);
			}
			else
			{
				printf("go to wait\n");
			}
			
			pBCPPa->pTargetNameID = NULL;

			return FlagName(pa_pProc,"BdgClientWait");
		}
	}

	if(pa_state == PS_STATE_FIRST_TIME || 
			pa_state == PS_STATE_OVERTIME)
	{
		SendingMsg.info = BRIDGE_MSG_INFO_CONNECT;
		strcpy(SendingMsg.NameID,pBCPPa->pTargetNameID);

		printf("Connect to %s \n",pBCPPa->pTargetNameID);
		BdgMsgWrite(pa_pProc,&SendingMsg,BDG_ADDR(s,pBdgProc));
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( BdgClientDoConnectAddr )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct BridgeMsg  SendingMsg;
	struct BridgeMsg  *pBdgMsg;

	pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_ADDR_FILTER,0,BDG_ADDR(b,pBdgProc));

	if(pBdgMsg)
	{
		if(pBdgMsg->info == BRIDGE_MSG_INFO_HELLO)
		{
			printf("^_^3 I find my friend!\n");
			return PS_CALLBK_RET_DONE;
		}
	}

	if(pa_state == PS_STATE_FIRST_TIME || 
			pa_state == PS_STATE_OVERTIME)
	{
		printf("Sending Hello..\n");
		SendingMsg.info = BRIDGE_MSG_INFO_HELLO;
		BdgMsgWrite(pa_pProc,&SendingMsg,BDG_ADDR(b,pBdgProc));
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		pBdgProc->MultiSendTo = pBdgProc->sx.addr;

		pBdgProc->MultiSendInfo = BRIDGE_MSG_ERR_CONNECT_ADDR;
		return FlagName(pa_pProc,"BdgClientMultiSendNotify");
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( BdgClientMultiSendNotify )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct BridgeMsg  SendingMsg;

	if(pa_state == PS_STATE_FIRST_TIME || 
			pa_state == PS_STATE_OVERTIME)
	{
		printf("Multi Send Notify ...\n");
		SendingMsg.info = pBdgProc->MultiSendInfo;
		BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgProc->MultiSendTo);
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		return FlagName(pa_pProc,"BdgClientWait");
	}

	return PS_CALLBK_RET_GO_ON;
}
