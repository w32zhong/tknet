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
		return FlagName(pa_pProc,"BdgConnectRequireServer");
		//do not return PS_CALLBK_RET_ABORT , because under some
		//situations , over-time can be a result of the other side
		//peer's trying to response another peer in his 'wait' step.
		//we need to return BdgConnectRequireServer to confirm whether
		//it is alive.
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
	struct Peer     Peer0,Peer1;

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
				
				strcpy(pBdgProc->PunAddrNameID,pBdgProc->s.NameID);
				strcpy(pBdgProc->ConAddrNameID,pBdgProc->a.NameID);

				pBdgProc->DecisionFlag = CONNECT_DECISION_FLAG_A_SIDE_RELAY;
				pBdgProc->DecisionRelayID = sta_AccRelayID;

				sta_AccRelayID ++;
				if(sta_AccRelayID == 0)
				{//overflow
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
				
				strcpy(pBdgProc->PunAddrNameID,pBdgProc->a.NameID);
				strcpy(pBdgProc->ConAddrNameID,pBdgProc->b.NameID);
			}
			else
			{
				pBdgProc->DecisionPunAddr = pBdgProc->b.addr;
				pBdgProc->DecisionConAddr = pBdgProc->a.addr;
				
				strcpy(pBdgProc->PunAddrNameID,pBdgProc->b.NameID);
				strcpy(pBdgProc->ConAddrNameID,pBdgProc->a.NameID);
			}

			pBdgProc->DecisionFlag = CONNECT_DECISION_FLAG_DIRECT;
			pBdgProc->DecisionRelayID = 0;
		}
	}
	else if(CONNECT_DECISION_FLAG_DIRECT == pBdgProc->DecisionFlag)
	{
		PROMPT(Usual,"successful direct connection bridged.\n");
		BetweenMacro(&pBdgProc->a,&pBdgProc->b);

		return FlagName(pa_pProc,"BdgConnectRequireServer");
	}
	else if(CONNECT_DECISION_FLAG_A_SIDE_RELAY == pBdgProc->DecisionFlag)
	{
		pBdgProc->DecisionPunAddr = pBdgProc->s.addr;
		pBdgProc->DecisionConAddr = pBdgProc->b.addr;
				
		strcpy(pBdgProc->PunAddrNameID,pBdgProc->s.NameID);
		strcpy(pBdgProc->ConAddrNameID,pBdgProc->b.NameID);
				
		pBdgProc->DecisionFlag = CONNECT_DECISION_FLAG_B_SIDE_RELAY;
	}
	else if(CONNECT_DECISION_FLAG_B_SIDE_RELAY == pBdgProc->DecisionFlag)
	{
		PROMPT(Usual,"successful relay connection bridged.\n");
		BetweenMacro(&pBdgProc->s,&pBdgProc->a);
		BetweenMacro(&pBdgProc->s,&pBdgProc->b);
		
		return FlagName(pa_pProc,"BdgConnectRequireServer");
	}
	else if(CONNECT_DECISION_FLAG_ERR == pBdgProc->DecisionFlag)
	{
		PROMPT(Usual,"One bridge task failed.\n");
		
		//trace who and who
		Peer0.addr = pBdgProc->DecisionPunAddr;
		Peer1.addr = pBdgProc->DecisionConAddr;
		strcpy(Peer0.NameID,pBdgProc->PunAddrNameID);
		strcpy(Peer1.NameID,pBdgProc->ConAddrNameID);
		BetweenMacro(&Peer0,&Peer1);

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
	
	struct NetAddr    SessionMaintainAddr = NetAddr("220.181.111.85",80);
	//Baidu.com :)

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

	if(pa_state == PS_STATE_LAST_TIME)
	{
		PROMPT(Usual,"*");//indicating public session maintaining
		BdgMsgWrite(pa_pProc,&SendingMsg,&SessionMaintainAddr);
	
		return PS_CALLBK_RET_REDO;
	}

	return PS_CALLBK_RET_GO_ON;
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
			PROMPT(Usual,"client register ok\n");
			pBCPPa->ifConnected = 1;//we say it is connected, we will clean this
			                        //flag in the client proc end notification.

			pBCPPa->ifFastSendWait = 1;//send fast wait to finish the server-side
			// "BdgConnectRequireReply" step.

			return PS_CALLBK_RET_DONE;
		}
		else if(pBdgMsg->info == BRIDGE_MSG_ERR_NAMEID_EXIST)
		{
			PROMPT(Usual,"client register recv name exist\n");

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
			PROMPT(Usual,".");//indicating "I am waiting"
			return PS_CALLBK_RET_REDO;
		}
		else if(pBdgMsg->info == BRIDGE_MSG_INFO_PUNCHING)
		{
			PROMPT(Usual,"Punching ...\n");

			SendingMsg.info = BRIDGE_MSG_INFO_UNKNOWN;

			BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgMsg->addr);
			tkMsSleep(100);
			BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgMsg->addr);
			tkMsSleep(100);
			//send twice to gain the possibility of a valid punching
			
			SendingMsg.info = BRIDGE_MSG_INFO_PUNCHING_FINISH;
			BdgMsgWrite(pa_pProc,&SendingMsg,&FromAddr);

			PROMPT(Usual,"Punching Finish\n");
		}
		else if(pBdgMsg->info == BRIDGE_MSG_INFO_CONNECT_ADDR)
		{
			PROMPT(Usual,"Recieved a Connect Address ...\n");
			pBdgProc->sx.addr = FromAddr;
			pBdgProc->b.addr = pBdgMsg->addr;
			pBdgProc->DecisionRelayID = pBdgMsg->RelayID;
			
			return FlagName(pa_pProc,"BdgClientDoConnectAddr");
		}
		else if(pBdgMsg->info == BRIDGE_MSG_INFO_HELLO)
		{
			if(pBdgMsg->RelayID == 0)
			{
				PROMPT(Usual,"Nice To Meet you too.\n");

				SessionStart(FromAddr,pBdgProc->pSock,
						pBdgProc->pProcList,pa_pINow,pa_pIForward);

				if(g_ConnectionNotify)
				{
					g_ConnectionNotify(FromAddr,pBdgProc->pSock,
							pBdgProc->pProcList,pa_pINow,pa_pIForward);
				}
			}
			else
			{
				MergeRes = RelayProcMerge(pBdgMsg->RelayID,FromAddr,
						pBdgProc->pProcList,pa_pINow,pa_pIForward,pBdgProc->pSock);

				if(MergeRes == RELAY_MERGE_RES_NEW_RELAY)
				{
					PROMPT(Usual,"Start relaying...\n");
					RelayProcTrace();
				}
				else if(MergeRes == RELAY_MERGE_RES_MERGED)
				{
					PROMPT(Usual,"relay merged.\n");
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

			PROMPT(Usual,"There is no seed to relay on the server side.\n");
		}
		else if(pBdgMsg->info == BRIDGE_MSG_ERR_ERROR )
		{
			SendingMsg.info = BRIDGE_MSG_INFO_ACKNOWLEDGE;
			BdgMsgWrite(pa_pProc,&SendingMsg,BDG_ADDR(s,pBdgProc));

			PROMPT(Usual,"Server side failed to make connection. Try again.\n");
		}
	}

	if(*pBCPPa->ppTargetNameID)
	{
		//connection begin
		return PS_CALLBK_RET_DONE;
	}
	else if(pBCPPa->DirectConnectAddr.port != 0)
	{
		//direct connection begin

		pBdgProc->sx.addr = NetAddr("127.0.0.1",0);
		pBdgProc->b.addr = pBCPPa->DirectConnectAddr;
		pBdgProc->DecisionRelayID = 0;

		return FlagName(pa_pProc,"BdgClientDoConnectAddr");
	}
	else if(pBCPPa->ifSkipRegister)
	{
		return PS_CALLBK_RET_REDO;
	}
	else if(pa_state == PS_STATE_OVERTIME || pBCPPa->ifFastSendWait )
	{
		pBCPPa->ifFastSendWait = 0;

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
				PROMPT(Usual,"remote server can't find %s\n",*pBCPPa->ppTargetNameID);
			}
			else
			{
				pBCPPa->ifFastSendWait = 1;
				PROMPT(Usual,"go to wait\n");
			}
			
			*pBCPPa->ppTargetNameID = NULL;

			return FlagName(pa_pProc,"BdgClientWait");
		}
	}

	if(pa_state == PS_STATE_FIRST_TIME || 
			pa_state == PS_STATE_OVERTIME)
	{
		SendingMsg.info = BRIDGE_MSG_INFO_CONNECT;
		strcpy(SendingMsg.NameID,*pBCPPa->ppTargetNameID);

		PROMPT(Usual,"Connecting %s ...\n",*pBCPPa->ppTargetNameID);
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
	DEF_AND_CAST(pBCPPa,struct BridgeClientProcPa,pBdgProc->Else);

	pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_ADDR_FILTER,0,BDG_ADDR(b,pBdgProc));

	if(pBdgMsg)
	{
		if(pBdgMsg->info == BRIDGE_MSG_INFO_HELLO)
		{
			PROMPT(Usual,"Nice To Meet U!\n");

			SessionStart(pBdgProc->b.addr,pBdgProc->pSock,
					pBdgProc->pProcList,pa_pINow,pa_pIForward);

			if(g_ConnectionNotify)
			{
				g_ConnectionNotify(pBdgProc->b.addr,pBdgProc->pSock,
					pBdgProc->pProcList,pa_pINow,pa_pIForward);
			}

			pBdgProc->MultiSendTo = pBdgProc->sx.addr;
			pBdgProc->MultiSendInfo = BRIDGE_MSG_INFO_ESTABLISHED;

			pBCPPa->DirectConnectAddr = NetAddr("0.0.0.0",0);
			return PS_CALLBK_RET_DONE;
		}
	}

	if(pa_state == PS_STATE_FIRST_TIME || 
			pa_state == PS_STATE_OVERTIME)
	{
		PROMPT(Usual,"Sending Hello..\n");
		SendingMsg.info = BRIDGE_MSG_INFO_HELLO;
		SendingMsg.RelayID = pBdgProc->DecisionRelayID;

		BdgMsgWrite(pa_pProc,&SendingMsg,BDG_ADDR(b,pBdgProc));
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		pBdgProc->MultiSendTo = pBdgProc->sx.addr;
		pBdgProc->MultiSendInfo = BRIDGE_MSG_ERR_CONNECT_ADDR;

			
		pBCPPa->DirectConnectAddr = NetAddr("0.0.0.0",0);
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
		PROMPT(Usual,"Multi Send Notify ...\n");
		SendingMsg.info = pBdgProc->MultiSendInfo;
		BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgProc->MultiSendTo);
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		return FlagName(pa_pProc,"BdgClientWait");
	}

	return PS_CALLBK_RET_GO_ON;
}
