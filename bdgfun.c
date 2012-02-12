#include "headers.h"

DEF_STRUCT_CONSTRUCTOR( Peer ,
		out_cons->addr.IPv4 = 0;
		out_cons->addr.port = 0;
		out_cons->NATType = NAT_T_UNKNOWN;
		)

DEF_STRUCT_CONSTRUCTOR( BridgeProc ,
		out_cons->pPeerDataRoot = NULL;
		out_cons->pSeedPeerCache = NULL;
		ProcessCons(&out_cons->proc);
		out_cons->pSock = NULL;
		out_cons->pProcList = NULL;

		PeerCons(&out_cons->s);
		PeerCons(&out_cons->a);
		PeerCons(&out_cons->b);
		PeerCons(&out_cons->sx);
		)

static struct BridgeProc sta_BdgSubServerProc;

void
BdgClientProcNotify(struct Process *pa_)
{
	struct BridgeProc *pProc = GET_STRUCT_ADDR(pa_ , struct BridgeProc , proc);
	if(pProc->Else)
		tkfree(pProc->Else);
	
	ProcessTraceSteps(pa_);
	ProcessFree(pa_);

	printf("client proc end\n");
}

void
BdgSubServerProcNotify(struct Process *pa_)
{
	struct BridgeProc *pBdgProc = GET_STRUCT_ADDR(pa_ , struct BridgeProc , proc);
	struct PeerData *pPD = GET_STRUCT_ADDR(pBdgProc,struct PeerData,BdgProc);

	ProcessTraceSteps(pa_);
	PeerDataDele(pPD,pBdgProc->pSeedPeerCache);

	printf("sub server proc end\n");
}

struct PeerData*
NewPeerDataWithBdgProc(struct NetAddr pa_addr,uchar pa_NATType,char *pa_pName,struct BridgeProc *pa_pBdgProc)
{
	struct PeerData *pPD = tkmalloc(struct PeerData);
	PeerDataCons(pPD);

	pPD->addr = pa_addr;
	pPD->NATType = pa_NATType;
	strcpy(pPD->NameID,pa_pName);

	pPD->BdgProc.pPeerDataRoot = pa_pBdgProc->pPeerDataRoot;
	pPD->BdgProc.pSeedPeerCache = pa_pBdgProc->pSeedPeerCache;
	pPD->BdgProc.pSock = pa_pBdgProc->pSock;
	pPD->BdgProc.pProcList = pa_pBdgProc->pProcList;

	ProcessConsAndSetSteps(&(pPD->BdgProc.proc), &sta_BdgSubServerProc.proc );
	pPD->BdgProc.proc.NotifyCallbk = &BdgSubServerProcNotify;
	
	return pPD;
}

void
SetPeerByPeerData(struct Peer *pa_pPeer,struct PeerData *pa_pPD)
{
	pa_pPeer->addr = pa_pPD->addr;
	pa_pPeer->NATType = pa_pPD->NATType;
}

void 
FreeBridgeServer(struct BridgeProc *pa_pBdgProc)
{
	ProcessFree(&sta_BdgSubServerProc.proc);
	ProcessFree(&pa_pBdgProc->proc);
}

struct BridgeMsg*
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

void
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
BridgeClientTryBdgServerProc(struct BridgeProc *pa_pBdgProc,struct BridgeHelloStepPa *pa_pProcPa , struct Sock *pa_pMainSock)
{
	BridgeProcCons(pa_pBdgProc);
	pa_pBdgProc->pSock = pa_pMainSock;
	pa_pBdgProc->Else = pa_pProcPa;	
	PROCESS_ADD_STEP( &pa_pBdgProc->proc , BdgClientTryBdgServer , 2000 , 2);
}

void 
FreeBdgClientProc(struct BridgeProc *pa_pBdgProc)
{
	ProcessFree(&pa_pBdgProc->proc);
	if(pa_pBdgProc->Else)
		tkfree(pa_pBdgProc->Else);
}

void 
BdgSubServerProcInit()
{
	ProcessCons(&sta_BdgSubServerProc.proc);
	PROCESS_ADD_STEP( &sta_BdgSubServerProc.proc , BdgBeginSubServer , 2000 , 2 );
	PROCESS_ADD_STEP( &sta_BdgSubServerProc.proc , BdgConnectRequireServer , 4000 , 3 );
	PROCESS_ADD_STEP( &sta_BdgSubServerProc.proc , BdgConnectRequireReply , 2000 , 2 );
	PROCESS_ADD_STEP( &sta_BdgSubServerProc.proc , BdgConnectDecision , 2000 , 2 );
	PROCESS_ADD_STEP( &sta_BdgSubServerProc.proc , BdgPunchingServer , 2000 , 2 );
	PROCESS_ADD_STEP( &sta_BdgSubServerProc.proc , BdgConnectAddrServer , 2000 , 2 );
}

void 
BridgeMakeClientProc(struct BridgeProc *pa_pBdgProc, struct Sock *pa_pMainSock ,struct NetAddr *pa_pAddr, char *pa_pMyNameID ,uchar pa_MyNatType , char *pa_pTargetNameID)
{
	struct BridgeClientProcPa *pBCPPa = tkmalloc(struct BridgeClientProcPa);

	BridgeProcCons(pa_pBdgProc);

	pa_pBdgProc->pSock = pa_pMainSock;
	pa_pBdgProc->s.addr = *pa_pAddr;
	pa_pBdgProc->a.NATType = pa_MyNatType;
	pa_pBdgProc->Else = pBCPPa;
	pa_pBdgProc->proc.NotifyCallbk = &BdgClientProcNotify;

	pBCPPa->pMyNameID = pa_pMyNameID;
	pBCPPa->pTargetNameID = pa_pTargetNameID;

	PROCESS_ADD_STEP( &pa_pBdgProc->proc , BdgClientRegister , 2000 , 2);
	PROCESS_ADD_STEP( &pa_pBdgProc->proc , BdgClientWait , 4000 , 3);
	PROCESS_ADD_STEP( &pa_pBdgProc->proc , BdgClientConnectRequire , 2000 , 2);
	PROCESS_ADD_STEP( &pa_pBdgProc->proc , BdgClientDoConnectAddr , 2000 , 2);
	PROCESS_ADD_STEP( &pa_pBdgProc->proc , BdgClientMultiSendNotify , 800 , 3);
}
