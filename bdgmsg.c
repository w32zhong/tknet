#include "headers.h"

DEF_STRUCT_CONSTRUCTOR( BridgeProc ,
		out_cons->pPeerDataRoot = NULL;
		out_cons->pSeedPeerCache = NULL;
		ProcessCons(&out_cons->proc);
		out_cons->pSock = NULL;
		out_cons->WaitAddr.IPv4 = 0;
		out_cons->WaitAddr.port = 0;
		out_cons->pProcList = NULL;
		)

static struct BridgeProc sta_BdgSubServerProc;

static __inline void 
BdgSubServerProcFree()
{
	ProcessFree(&sta_BdgSubServerProc.proc);
}

static struct BridgeMsg*
BdgMsgRead(struct Process *in_proc , uchar pa_option)
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(in_proc,struct BridgeProc,proc);
	struct NetAddr FromAddr = GetAddrFromSockAddr(&pBdgProc->pSock->AddrRecvfrom);
	DEF_AND_CAST(pMsg,struct TkNetMsg,pBdgProc->pSock->RecvBuff);
	DEF_AND_CAST(pBdgMsg,struct BridgeMsg,&(pMsg->msg.BdgMsg));

	//---------
	char AddrText[32];
	GetAddrText(&FromAddr,AddrText);
	//--------

	if( pMsg->flag != TK_NET_BDG_MSG_FLAG ||
		pBdgProc->pSock->RecvLen <= 0 )
	{
		return NULL;
	}
	else if( pa_option == READ_MSG_OPT_ANY )
	{
		printf("Bdg Recved any %s(%d)\n",AddrText,pMsg->flag);

		return pBdgMsg;
	}
	else if( ifNetAddrEqual(&FromAddr,&pBdgProc->WaitAddr) )
	{
		printf("Bdg Recved spe %s(%d)\n",AddrText,pMsg->flag);
		return pBdgMsg;
	}
	else
	{
		return NULL;
	}
}

static __inline void
BdgMsgClean(struct Process *in_proc )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(in_proc,struct BridgeProc,proc);
	pBdgProc->pSock->RecvLen = 0;
}

static __inline void
BdgMsgWrite(struct Process *in_proc ,struct BridgeMsg *in_msg , struct NetAddr *pa_pAddr)
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(in_proc,struct BridgeProc,proc);
	struct TkNetMsg   SendingMsg;
	char AddrText[32];
	
	SendingMsg.flag = TK_NET_BDG_MSG_FLAG;
	SendingMsg.msg.BdgMsg = *in_msg;

	GetAddrText(pa_pAddr,AddrText);
	printf("Bdg Write to %s,(%d) \n",AddrText,SendingMsg.flag);
	
	SockLocateTa(pBdgProc->pSock,htonl(pa_pAddr->IPv4),pa_pAddr->port);
	SockWrite(pBdgProc->pSock,BYS(SendingMsg));
}

STEP( BdgConnectionServer )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct BridgeMsg  SendingMsg;
	struct BridgeMsg  *pBdgMsg;
	struct PeerData *pPD = GET_STRUCT_ADDR(pBdgProc,struct PeerData,BdgProc);

	pBdgMsg = BdgMsgRead(pa_pProc,READ_MSG_OPT_SPECIFIC);

	if(pBdgMsg)
	{
		if(pBdgMsg->info == BRIDGE_MSG_INFO_ECHO)
		{
			PeerDataUpdateSeedInfo(pPD,pBdgMsg->Relays);
			
			BdgMsgClean(pa_pProc);
			return PS_CALLBK_RET_REDO;
		}
		else if(pBdgMsg->info == BRIDGE_MSG_INFO_CONNECT)
		{
			//...
		}
	}

	if( pa_state == PS_STATE_OVERTIME )
	{
		SendingMsg.info = BRIDGE_MSG_INFO_ECHO_REQUEST;
		BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgProc->WaitAddr);
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

static void 
BdgSubServerProcInit()
{
	ProcessCons(&sta_BdgSubServerProc.proc);
	PROCESS_ADD_STEP( &sta_BdgSubServerProc.proc , BdgConnectionServer , 4000 , 3 );
}

static void
BdgSubServerProcNotify(struct Process *pa_)
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_ , struct BridgeProc , proc);
	struct PeerData *pPD = GET_STRUCT_ADDR(pBdgProc,struct PeerData,BdgProc);

	PeerDataDele(pPD,pBdgProc->pSeedPeerCache);
	printf("PROC END\n");
}

STEP( BridgeMain )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct NetAddr FromAddr; 
	struct BridgeMsg  *pBdgMsg;
	struct BridgeMsg  SendingMsg;
	struct PeerData   *pPD;

	if(!SockRead(pBdgProc->pSock))
	{
		return PS_CALLBK_RET_REDO;
	}

	pBdgMsg = BdgMsgRead(pa_pProc,READ_MSG_OPT_ANY);

	if(pBdgMsg)
	{
		FromAddr = GetAddrFromSockAddr(&pBdgProc->pSock->AddrRecvfrom);

		switch(pBdgMsg->info)
		{
		case BRIDGE_MSG_INFO_HELLO:

			SendingMsg.info = BRIDGE_MSG_INFO_IAM_HERE;
			BdgMsgWrite(pa_pProc,&SendingMsg,&FromAddr);

			BdgMsgClean(pa_pProc);
			break;

		case BRIDGE_MSG_INFO_REGISTER:

			if(PeerDataFind(pBdgProc->pPeerDataRoot,pBdgMsg->NameID))
			{
				SendingMsg.info = BRIDGE_MSG_ERR_NAMEID_EXIST;
			}
			else
			{
				pPD = tkmalloc(struct PeerData);
				PeerDataCons(pPD);
				pPD->addr = FromAddr;
				pPD->NATType = pBdgMsg->NATType;
				if( pBdgMsg->NATType != NAT_T_SYMMETRIC)
				{
					PeerDataSelectAsSeed(pPD,pBdgProc->pSeedPeerCache);
				}
				strcpy(pPD->NameID,pBdgMsg->NameID);

				pPD->BdgProc.pPeerDataRoot = pBdgProc->pPeerDataRoot;
				pPD->BdgProc.pSeedPeerCache = pBdgProc->pSeedPeerCache;
				pPD->BdgProc.pSock = pBdgProc->pSock;
				pPD->BdgProc.pProcList = pBdgProc->pProcList;
				pPD->BdgProc.WaitAddr = FromAddr;

				PeerDataInsert(pPD,pBdgProc->pPeerDataRoot);
				ProcessConsAndSetSteps(&(pPD->BdgProc.proc), &sta_BdgSubServerProc.proc );
				pPD->BdgProc.proc.NotifyCallbk = &BdgSubServerProcNotify;
				ProcessSafeStart(&(pPD->BdgProc.proc),pBdgProc->pProcList,pa_pINow,pa_pIForward);

				SendingMsg.info = BRIDGE_MSG_INFO_RGST_OK;
			}

			BdgMsgWrite(pa_pProc,&SendingMsg,&FromAddr);

			BdgMsgClean(pa_pProc);
			break;
		}
	}

	return PS_CALLBK_RET_REDO;
}

void 
ConsAndStartBridgeServer(struct BridgeProc *pa_pBdgProc , struct PeerData *pa_pPeerDataRoot , struct ProcessingList *pa_pProcList , struct Sock *pa_pMainSock , struct Iterator *pa_pSeedPeerCache)
{
	BdgSubServerProcInit();
	BridgeProcCons(pa_pBdgProc);
	pa_pBdgProc->pPeerDataRoot = pa_pPeerDataRoot;	
	pa_pBdgProc->pSeedPeerCache = pa_pSeedPeerCache;	
	pa_pBdgProc->pSock = pa_pMainSock;
	pa_pBdgProc->pProcList = pa_pProcList;	

	PROCESS_ADD_STEP( &pa_pBdgProc->proc , BridgeMain , 1000 , 0);
	ProcessStart( &pa_pBdgProc->proc , pa_pProcList );
}

void 
FreeBridgeServer(struct BridgeProc *pa_pBdgProc)
{
	BdgSubServerProcFree();
	ProcessFree(&pa_pBdgProc->proc);
}

STEP( BridgeHello )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct BridgeMsg  SendingMsg;
	struct BridgeMsg  *pBdgMsg;
	DEF_AND_CAST(pStepPa , struct BridgeHelloStepPa , pBdgProc->Else);

	if(SockRead(pBdgProc->pSock))
	{
		pBdgMsg = BdgMsgRead(pa_pProc,READ_MSG_OPT_SPECIFIC);

		if(pBdgMsg && pBdgMsg->info == BRIDGE_MSG_INFO_IAM_HERE)
		{
			pStepPa->res = 1;
			pStepPa->ValidAddr = pBdgProc->WaitAddr;
			return PS_CALLBK_RET_DONE;
		}
		//else deal with 'redirection'
	}

	if(pa_state == PS_STATE_FIRST_TIME || 
			pa_state == PS_STATE_OVERTIME)
	{
		SendingMsg.info = BRIDGE_MSG_INFO_HELLO;
		BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgProc->WaitAddr);
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		pStepPa->res = 0;
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

void 
BridgeMakeHelloProc(struct BridgeProc *pa_pBdgProc,struct BridgeHelloStepPa *pa_pProcPa , struct Sock *pa_pMainSock,struct NetAddr *pa_pAddr)
{
	BridgeProcCons(pa_pBdgProc);
	pa_pBdgProc->pSock = pa_pMainSock;
	pa_pBdgProc->WaitAddr = *pa_pAddr;
	pa_pBdgProc->Else = pa_pProcPa;	
	PROCESS_ADD_STEP( &pa_pBdgProc->proc , BridgeHello , 2000 , 2);
}

STEP( BdgClientRegister )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct BridgeMsg  SendingMsg;
	struct BridgeMsg  *pBdgMsg;
	DEF_AND_CAST(pBCPPa,struct BridgeClientProcPa,pBdgProc->Else);

	pBdgMsg = BdgMsgRead(pa_pProc,READ_MSG_OPT_SPECIFIC);

	if(pBdgMsg)
	{
		if(pBdgMsg->info == BRIDGE_MSG_INFO_RGST_OK)
		{
			BdgMsgClean(pa_pProc);
			return PS_CALLBK_RET_DONE;
		}
		else if(pBdgMsg->info == BRIDGE_MSG_ERR_NAMEID_EXIST)
		{
			BdgMsgClean(pa_pProc);
			return PS_CALLBK_RET_ABORT;
		}
	}

	if(pa_state == PS_STATE_FIRST_TIME || 
			pa_state == PS_STATE_OVERTIME)
	{
		SendingMsg.info = BRIDGE_MSG_INFO_REGISTER;
		strcpy(SendingMsg.NameID,pBCPPa->pMyNameID);
		SendingMsg.NATType = pBCPPa->MyNatType;

		BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgProc->WaitAddr);
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

	pBdgMsg = BdgMsgRead(pa_pProc,READ_MSG_OPT_SPECIFIC);

	if(pBdgMsg)
	{
		if(pBdgMsg->info == BRIDGE_MSG_INFO_ECHO_REQUEST)
		{
			SendingMsg.info = BRIDGE_MSG_INFO_ECHO;
			SendingMsg.Relays = 5;
			BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgProc->WaitAddr);
			BdgMsgClean(pa_pProc);
	
			return PS_CALLBK_RET_REDO;
		}
	}

	if(pa_state == PS_STATE_LAST_TIME)
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

static void
BdgClientProcNotify(struct Process *pa_)
{
	struct BridgeProc *pProc = GET_STRUCT_ADDR(pa_ , struct BridgeProc , proc);
	tkfree(pProc->Else);
	ProcessFree(pa_);
}

void 
BridgeMakeClientProc(struct BridgeProc *pa_pBdgProc, struct Sock *pa_pMainSock ,struct NetAddr *pa_pAddr, char *pa_pMyNameID ,uchar pa_MyNatType)
{
	struct BridgeClientProcPa *pBCPPa = tkmalloc(struct BridgeClientProcPa);

	BridgeProcCons(pa_pBdgProc);

	pa_pBdgProc->pSock = pa_pMainSock;
	pa_pBdgProc->WaitAddr = *pa_pAddr;
	pa_pBdgProc->Else = pBCPPa;
	pa_pBdgProc->proc.NotifyCallbk = &BdgClientProcNotify;

	pBCPPa->pMyNameID = pa_pMyNameID;
	pBCPPa->pTargetNameID = NULL;
	pBCPPa->MyNatType = pa_MyNatType;

	PROCESS_ADD_STEP( &pa_pBdgProc->proc , BdgClientRegister , 2000 , 2);
	PROCESS_ADD_STEP( &pa_pBdgProc->proc , BdgClientWait , 4000 , 3);
}

void 
FreeBdgClientProc(struct BridgeProc *pa_pBdgProc)
{
	ProcessFree(&pa_pBdgProc->proc);
	tkfree(pa_pBdgProc->Else);
}
