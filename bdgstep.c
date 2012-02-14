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
		//printf("I am free.\n");
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
			if( pBdgProc->b.addr.port == 0 )
			{
				return FlagName(pa_pProc,"BdgConnectRequireServer");
			}
			else
			{
				pBdgProc->DecisionFlag = CONNECT_DECISION_FLAG_BEGIN;
				return PS_CALLBK_RET_DONE;
			}
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

uint sta_AccRelayID = 1;

STEP( BdgConnectDecision )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	uchar aType = pBdgProc->a.NATType;
	uchar bType = pBdgProc->b.NATType;
	struct PeerData *pSeedPD;

	if(CONNECT_DECISION_FLAG_BEGIN == pBdgProc->DecisionFlag)
	{
		if(IF_NEAD_RELAY(aType,bType))
		{
			pSeedPD = SeedPeerSelectOne(pBdgProc->pSeedPeerCache);

			if(pSeedPD == NULL)
			{
				pBdgProc->ErrCode = BRIDGE_MSG_ERR_NO_SEED_TO_RELAY;
				return FlagName(pa_pProc,"BdgErrReturnServer");
			}
			else
			{
				SetPeerByPeerData(&pBdgProc->s,pSeedPD);

				pBdgProc->DecisionPunAddr = pBdgProc->s.addr;
				pBdgProc->DecisionConAddr = pBdgProc->a.addr;

				pBdgProc->DecisionFlag = CONNECT_DECISION_FLAG_A_SIDE_RELAY;
				pBdgProc->DecisionRelayID = sta_AccRelayID;

				sta_AccRelayID ++;
				if(sta_AccRelayID == 0)
				{//over-flow
					sta_AccRelayID = 1;
				}
			}
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
	else if(CONNECT_DECISION_FLAG_DIRECT == pBdgProc->DecisionFlag)
	{
		printf("successful direct connection bridged.\n");
		return FlagName(pa_pProc,"BdgConnectRequireServer");
	}
	else if(CONNECT_DECISION_FLAG_A_SIDE_RELAY == pBdgProc->DecisionFlag)
	{
		pBdgProc->DecisionPunAddr = pBdgProc->s.addr;
		pBdgProc->DecisionConAddr = pBdgProc->b.addr;
				
		pBdgProc->DecisionFlag = CONNECT_DECISION_FLAG_B_SIDE_RELAY;
	}
	else if(CONNECT_DECISION_FLAG_B_SIDE_RELAY == pBdgProc->DecisionFlag)
	{
		printf("successful relay connection bridged.\n");
		return FlagName(pa_pProc,"BdgConnectRequireServer");
	}
	else if(CONNECT_DECISION_FLAG_ERR == pBdgProc->DecisionFlag)
	{
		printf("One bridge task failed.\n");
		
		pBdgProc->ErrCode = BRIDGE_MSG_ERR_ERROR;
		return FlagName(pa_pProc,"BdgErrReturnServer");
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
		pBdgProc->DecisionFlag = CONNECT_DECISION_FLAG_ERR;
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
			//we don't set DecisionFlag, because we just keep the 
			//flag what it was if no error occur.
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
		pBdgProc->DecisionFlag = CONNECT_DECISION_FLAG_ERR;
		return FlagName(pa_pProc,"BdgConnectDecision");
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( BdgErrReturnServer )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct BridgeMsg  SendingMsg;
	struct BridgeMsg  *pBdgMsg;

	pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_ADDR_FILTER,0,BDG_ADDR(a,pBdgProc));

	if(pBdgMsg)
	{
		if(pBdgMsg->info == BRIDGE_MSG_INFO_ACKNOWLEDGE )
		{
			return FlagName(pa_pProc,"BdgConnectRequireServer");
		}
	}

	if(pa_state == PS_STATE_FIRST_TIME || 
			pa_state == PS_STATE_OVERTIME )
	{
		SendingMsg.info = pBdgProc->ErrCode;
		BdgMsgWrite(pa_pProc,&SendingMsg,BDG_ADDR(a,pBdgProc));
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		return PS_CALLBK_RET_ABORT;
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
	struct NetAddr    FromAddr; 
	uchar             MergeRes;

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
			pBdgProc->DecisionRelayID = pBdgMsg->RelayID;
			
			return FlagName(pa_pProc,"BdgClientDoConnectAddr");
		}
		else if(pBdgMsg->info == BRIDGE_MSG_INFO_HELLO)
		{
			if(pBdgMsg->RelayID == 0)
			{
				printf("Nice To Meet you too.\n");
			}
			else
			{
				MergeRes = RelayProcMerge(pBdgMsg->RelayID,FromAddr,
						pBdgProc->pProcList,pa_pINow,pa_pIForward,pBdgProc->pSock);

				if(MergeRes == RELAY_MERGE_RES_NEW_RELAY)
				{
					printf("Start relaying...\n");
					RelayProcTrace();
				}
				else if(MergeRes == RELAY_MERGE_RES_MERGED)
				{
					printf("relay merged.\n");
					RelayProcTrace();
				}
			}

			pBdgProc->MultiSendTo = FromAddr;
			pBdgProc->MultiSendInfo = BRIDGE_MSG_INFO_HELLO;
			
			return FlagName(pa_pProc,"BdgClientMultiSendNotify");
		}
		else if(pBdgMsg->info == BRIDGE_MSG_ERR_NO_SEED_TO_RELAY )
		{
			SendingMsg.info = BRIDGE_MSG_INFO_ACKNOWLEDGE;
			BdgMsgWrite(pa_pProc,&SendingMsg,BDG_ADDR(s,pBdgProc));

			printf("There is no seed to relay on the server side.\n");
		}
		else if(pBdgMsg->info == BRIDGE_MSG_ERR_ERROR )
		{
			SendingMsg.info = BRIDGE_MSG_INFO_ACKNOWLEDGE;
			BdgMsgWrite(pa_pProc,&SendingMsg,BDG_ADDR(s,pBdgProc));

			printf("Server side failed to make connection. Try again.\n");
		}
	}

	if(pBCPPa->pTargetNameID)
	{
		return PS_CALLBK_RET_DONE;
	}
	else if(pa_state == PS_STATE_OVERTIME)
	{
		SendingMsg.info = BRIDGE_MSG_INFO_WAITING;
		SendingMsg.Relays = g_Relays;

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

			pBdgProc->MultiSendTo = pBdgProc->sx.addr;
			pBdgProc->MultiSendInfo = BRIDGE_MSG_INFO_ESTABLISHED;

			return PS_CALLBK_RET_DONE;
		}
	}

	if(pa_state == PS_STATE_FIRST_TIME || 
			pa_state == PS_STATE_OVERTIME)
	{
		printf("Sending Hello..\n");
		SendingMsg.info = BRIDGE_MSG_INFO_HELLO;
		SendingMsg.RelayID = pBdgProc->DecisionRelayID;

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
