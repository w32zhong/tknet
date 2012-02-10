#include "headers.h"

DEF_STRUCT_CONSTRUCTOR( BridgeProc ,
		out_cons->pPeerDataRoot = NULL;
		out_cons->pSeedPeerCache = NULL;
		ProcessCons(&out_cons->proc);
		out_cons->pSock = NULL;
		out_cons->pProcList = NULL;

		out_cons->s.IPv4 = 0;
		out_cons->s.port = 0;
		out_cons->a.IPv4 = 0;
		out_cons->a.port = 0;
		out_cons->b.IPv4 = 0;
		out_cons->b.port = 0;
		)

static struct BridgeProc sta_BdgSubServerProc;

static __inline void 
BdgSubServerProcFree()
{
	ProcessFree(&sta_BdgSubServerProc.proc);
}

static struct BridgeMsg*
BdgMsgRead(struct Process *in_proc , uchar pa_option , uchar pa_msg , struct NetAddr *pa_pAddr)
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
	else if( pa_option == BDG_READ_OPT_ANY )
	{
		printf("Bdg Recved any %s\n",AddrText);
		return pBdgMsg;
	}
	else if( pa_option == BDG_READ_OPT_ADDR_FILTER && 
			ifNetAddrEqual(&FromAddr,pa_pAddr) )
	{
		printf("Bdg Recved addr %s\n",AddrText);
		return pBdgMsg;
	}
	else if( pa_option == BDG_READ_OPT_MSG_FILTER && 
			pBdgMsg->info == pa_msg )
	{
		printf("Bdg Recved msg %s\n",AddrText);
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
	printf("Bdg Write to %s\n",AddrText);
	
	SockLocateTa(pBdgProc->pSock,htonl(pa_pAddr->IPv4),pa_pAddr->port);
	SockWrite(pBdgProc->pSock,BYS(SendingMsg));
}

STEP( BdgEchoServer )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct BridgeMsg  SendingMsg;
	struct BridgeMsg  *pBdgMsg;
	struct PeerData *pPD = GET_STRUCT_ADDR(pBdgProc,struct PeerData,BdgProc);
	struct PeerData *pFoundPD;

	pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_ADDR_FILTER,0,&pBdgProc->a);

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
			pFoundPD = PeerDataFind(pBdgProc->pPeerDataRoot,pBdgMsg->NameID);

			if(pFoundPD)
			{
				pBdgProc->b = pFoundPD->addr;
			}
			else
			{
				pBdgProc->b.port = 0;
			}

			BdgMsgClean(pa_pProc);
			return PS_CALLBK_RET_DONE;
		}
	}

	if( pa_state == PS_STATE_OVERTIME )
	{
		SendingMsg.info = BRIDGE_MSG_INFO_ECHO_REQUEST;
		BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgProc->a);
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( BdgPreconnectionServer )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct BridgeMsg  SendingMsg;
	struct BridgeMsg  *pBdgMsg;
	struct PeerData *pPD = GET_STRUCT_ADDR(pBdgProc,struct PeerData,BdgProc);

	pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_ADDR_FILTER,0,&pBdgProc->a);

	if((pBdgMsg && pBdgMsg->info == BRIDGE_MSG_INFO_IAM_WAIT) ||
			pa_state == PS_STATE_LAST_TIME)
	{
		//Perhaps client's only 1 send of IAM_WAIT haven't recved
		// so ,let it go
		BdgMsgClean(pa_pProc);

		if( pBdgProc->b.port == 0 )
		{
			return FlagName(pa_pProc,"BdgEchoServer");
		}
		else
		{
			return PS_CALLBK_RET_DONE;
		}
	}

	if(pa_state == PS_STATE_FIRST_TIME || 
			pa_state == PS_STATE_OVERTIME )
	{
		if( pBdgProc->b.port == 0 )
		{
			SendingMsg.info = BRIDGE_MSG_ERR_NO_NAMEID;
		}
		else
		{
			SendingMsg.info = BRIDGE_MSG_INFO_CONNECT_BEGIN;
		}

		BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgProc->a);
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( BdgConnectionServer )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct BridgeMsg  SendingMsg;
	struct BridgeMsg  *pBdgMsg;
	struct PeerData *pPD = GET_STRUCT_ADDR(pBdgProc,struct PeerData,BdgProc);

	pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_ADDR_FILTER,0,&pBdgProc->a);

	if(pBdgMsg)
	{
		/*if(pBdgMsg->info == BRIDGE_MSG_INFO_IAM_WAIT)
		{
			BdgMsgClean(pa_pProc);
			return PS_CALLBK_RET_DONE;
		}*/
	}

	if(pa_state == PS_STATE_FIRST_TIME || 
			pa_state == PS_STATE_OVERTIME )
	{
	//	SendingMsg.info = BRIDGE_MSG_INFO_CONNECT_BEGIN;
	//	BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgProc->a);
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		return PS_CALLBK_RET_ABORT;
	}

	printf("connecting");

	return PS_CALLBK_RET_GO_ON;
}

static void 
BdgSubServerProcInit()
{
	ProcessCons(&sta_BdgSubServerProc.proc);
	PROCESS_ADD_STEP( &sta_BdgSubServerProc.proc , BdgEchoServer , 4000 , 3 );
	PROCESS_ADD_STEP( &sta_BdgSubServerProc.proc , BdgPreconnectionServer , 2000 , 2 );
	PROCESS_ADD_STEP( &sta_BdgSubServerProc.proc , BdgConnectionServer , 2000 , 2 );
}

static void
BdgSubServerProcNotify(struct Process *pa_)
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_ , struct BridgeProc , proc);
	struct PeerData *pPD = GET_STRUCT_ADDR(pBdgProc,struct PeerData,BdgProc);

	PeerDataDele(pPD,pBdgProc->pSeedPeerCache);
	printf("sub server proc end\n");
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

	pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_ANY,0,NULL);

	if(pBdgMsg)
	{
		FromAddr = GetAddrFromSockAddr(&pBdgProc->pSock->AddrRecvfrom);

		switch(pBdgMsg->info)
		{
		case BRIDGE_MSG_INFO_HELLO_BDG:

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
				pPD->BdgProc.a = FromAddr;

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

STEP( BdgClientHello )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct BridgeMsg  SendingMsg;
	struct BridgeMsg  *pBdgMsg;
	DEF_AND_CAST(pStepPa , struct BridgeHelloStepPa , pBdgProc->Else);

	if(SockRead(pBdgProc->pSock))
	{
		pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_ADDR_FILTER,0,&pBdgProc->s);

		if(pBdgMsg && pBdgMsg->info == BRIDGE_MSG_INFO_IAM_HERE)
		{
			pStepPa->res = 1;
			pStepPa->ValidAddr = pBdgProc->s;
			return PS_CALLBK_RET_DONE;
		}
		//else deal with 'redirection'
	}

	if(pa_state == PS_STATE_FIRST_TIME || 
			pa_state == PS_STATE_OVERTIME)
	{
		SendingMsg.info = BRIDGE_MSG_INFO_HELLO_BDG;
		BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgProc->s);
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
	pa_pBdgProc->s = *pa_pAddr;
	pa_pBdgProc->Else = pa_pProcPa;	
	PROCESS_ADD_STEP( &pa_pBdgProc->proc , BdgClientHello , 2000 , 2);
}

STEP( BdgClientRegister )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct BridgeMsg  SendingMsg;
	struct BridgeMsg  *pBdgMsg;
	DEF_AND_CAST(pBCPPa,struct BridgeClientProcPa,pBdgProc->Else);

	pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_ADDR_FILTER,0,&pBdgProc->s);

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

		BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgProc->s);
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

	pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_ADDR_FILTER,0,&pBdgProc->s);

	if(pBdgMsg)
	{
		if(pBdgMsg->info == BRIDGE_MSG_INFO_ECHO_REQUEST)
		{
			SendingMsg.info = BRIDGE_MSG_INFO_ECHO;
			SendingMsg.Relays = 5;
			BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgProc->s);
			
			BdgMsgClean(pa_pProc);
			return PS_CALLBK_RET_REDO;
		}
	}


/*		if(pBdgMsg->info == BRIDGE_MSG_INFO_PUNCHING)
		{
			SendingMsg.info = BRIDGE_MSG_INFO_UNKNOWN;
			
			BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgMsg->addr);
			tkMsSleep(200);
			BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgMsg->addr);
			tkMsSleep(200);
			//send twice to confirm a valid punching

			SendingMsg.info = BRIDGE_MSG_INFO_PUNCHING_FINISH;
			BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgProc->WaitAddr);

			BdgMsgClean(pa_pProc);
			return PS_CALLBK_RET_DONE;
		}
		else if(pBdgMsg->info == BRIDGE_MSG_INFO_CONNECT_ADDR)
		{
			BdgMsgClean(pa_pProc);
			return PS_CALLBK_RET_ABORT;
		}
*/

	if(pBCPPa->pTargetNameID)
	{
		return PS_CALLBK_RET_DONE;
	}
	else if(pa_state == PS_STATE_OVERTIME)
	{
		printf("waiting ...\n");
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( BdgClientConnect )
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_pProc,struct BridgeProc,proc);
	struct BridgeMsg  SendingMsg;
	struct BridgeMsg  *pBdgMsg;
	DEF_AND_CAST(pBCPPa,struct BridgeClientProcPa,pBdgProc->Else);

	pBdgMsg = BdgMsgRead(pa_pProc,BDG_READ_OPT_ADDR_FILTER,0,&pBdgProc->s);

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

			SendingMsg.info = BRIDGE_MSG_INFO_IAM_WAIT;
			BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgProc->s);	

			BdgMsgClean(pa_pProc);
			return FlagName(pa_pProc,"BdgClientWait");
		}
	}

	if(pa_state == PS_STATE_FIRST_TIME || 
			pa_state == PS_STATE_OVERTIME)
	{
		SendingMsg.info = BRIDGE_MSG_INFO_CONNECT;
		strcpy(SendingMsg.NameID,pBCPPa->pTargetNameID);

		printf("Connect to %s \n",pBCPPa->pTargetNameID);
		BdgMsgWrite(pa_pProc,&SendingMsg,&pBdgProc->s);
	}
	else if(pa_state == PS_STATE_LAST_TIME)
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

static void
BdgClientProcNotify(struct Process *pa_)
{
	struct BridgeProc *pProc = GET_STRUCT_ADDR(pa_ , struct BridgeProc , proc);
	if(pProc->Else)
		tkfree(pProc->Else);
	ProcessFree(pa_);

	printf("client proc end\n");
}

void 
BridgeMakeClientProc(struct BridgeProc *pa_pBdgProc, struct Sock *pa_pMainSock ,struct NetAddr *pa_pAddr, char *pa_pMyNameID ,uchar pa_MyNatType , char *pa_pTargetNameID)
{
	struct BridgeClientProcPa *pBCPPa = tkmalloc(struct BridgeClientProcPa);

	BridgeProcCons(pa_pBdgProc);

	pa_pBdgProc->pSock = pa_pMainSock;
	pa_pBdgProc->s    = *pa_pAddr;
	pa_pBdgProc->Else = pBCPPa;
	pa_pBdgProc->proc.NotifyCallbk = &BdgClientProcNotify;

	pBCPPa->pMyNameID = pa_pMyNameID;
	pBCPPa->pTargetNameID = pa_pTargetNameID;
	pBCPPa->MyNatType = pa_MyNatType;

	PROCESS_ADD_STEP( &pa_pBdgProc->proc , BdgClientRegister , 2000 , 2);
	PROCESS_ADD_STEP( &pa_pBdgProc->proc , BdgClientWait , 4000 , 3);
	PROCESS_ADD_STEP( &pa_pBdgProc->proc , BdgClientConnect , 2000 , 2);
}

void 
FreeBdgClientProc(struct BridgeProc *pa_pBdgProc)
{
	ProcessFree(&pa_pBdgProc->proc);
	if(pa_pBdgProc->Else)
		tkfree(pa_pBdgProc->Else);
}
