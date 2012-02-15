#include "tknet.h"

uint g_Relays = 0;
static struct Iterator   sta_IRelayProc;
static struct RelayProc  sta_RelayProcess;

struct FindRelayProcByRelayIDPa
{
	uint RelayID;
	struct RelayProc *pFound;
};

DEF_STRUCT_CONSTRUCTOR( RelayProc ,
		out_cons->RelayID = 0;
		PeerCons(&out_cons->peer0);
		PeerCons(&out_cons->peer1);
		ProcessConsAndSetSteps(&out_cons->proc,&sta_RelayProcess.proc);
		out_cons->pSock = NULL;
		ListNodeCons(&out_cons->ln);
		)

static __inline void
RelayTo(struct RelayProc *pa_pRelayProc,struct NetAddr *pa_pToAddr)
{
	DEF_AND_CAST(pMsg,struct TkNetMsg,pa_pRelayProc->pSock->RecvBuff);
	SockLocateTa(pa_pRelayProc->pSock,htonl(pa_pToAddr->IPv4),pa_pToAddr->port);
	SockWrite(pa_pRelayProc->pSock,BYS(*pMsg)); 
}

DEF_FREE_LIST_ELEMENT_SAFE_FUNCTION(FreeRelayProc,struct RelayProc,ln,;)

EXTERN_STEP( Relay )

void
RelayModuleInit()
{
	sta_IRelayProc = GetIterator(NULL);
	
	ProcessCons(&sta_RelayProcess.proc);
	PROCESS_ADD_STEP( &sta_RelayProcess.proc , Relay , 30000 , 0 );
}

void 
RelayMuduleDestruction()
{
	ForEach(&sta_IRelayProc,&FreeRelayProc,NULL);
	ProcessFree(&sta_RelayProcess.proc);
}

STEP( Relay )
{
	struct RelayProc *pRelayProc = GET_STRUCT_ADDR(pa_pProc,struct RelayProc,proc);
	struct NetAddr   FromAddr = GetAddrFromSockAddr(&pRelayProc->pSock->AddrRecvfrom);
	struct NetAddr   *pAddr;
	struct TkNetMsg   SendingMsg;
	struct TkNetMsg  *pMsg;

	pMsg = ifReadTkMsg(pRelayProc->pSock);

	if(pMsg && pMsg->flag != TK_NET_BDG_MSG_FLAG)
	{
		if(pRelayProc->peer1.addr.port != 0)
		{
			if(ifNetAddrEqual(&FromAddr,&(pRelayProc->peer0.addr)))
			{
				RelayTo(pRelayProc,&(pRelayProc->peer1.addr));
				return PS_CALLBK_RET_REDO;
			}
			else if(ifNetAddrEqual(&FromAddr,&(pRelayProc->peer1.addr)))
			{
				RelayTo(pRelayProc,&(pRelayProc->peer0.addr));
				return PS_CALLBK_RET_REDO;
			}
		}
		else
		{
			if(ifNetAddrEqual(&FromAddr,&(pRelayProc->peer0.addr)))
			{

				SendingMsg.flag = TK_NET_BDG_MSG_FLAG;
				SendingMsg.msg.BdgMsg.info = BRIDGE_MSG_INFO_WAIT_RELAY;
				
				pAddr = &(pRelayProc->peer0.addr);
				SockLocateTa(pRelayProc->pSock,htonl(pAddr->IPv4),pAddr->port);
				SockWrite(pRelayProc->pSock,BYS(SendingMsg)); 
				
				return PS_CALLBK_RET_REDO;
			}
		}
	}

	if(pa_state == PS_STATE_LAST_TIME)
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

BOOL
LIST_ITERATION_CALLBACK_FUNCTION(FindRelayProcByRelayID)
{
	struct RelayProc *pRelayProc
		= GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct RelayProc,ln);
	DEF_AND_CAST(pFRPBRIPa,struct FindRelayProcByRelayIDPa ,pa_else);

	if(pFRPBRIPa->RelayID == pRelayProc->RelayID)
	{
		pFRPBRIPa->pFound = pRelayProc;
		return 1;
	}
	else
	{
		return pa_pINow->now == pa_pIHead->last;
	}
}

void
RelayProcNotify(struct Process *pa_)
{
	struct RelayProc *pRelayProc = GET_STRUCT_ADDR(pa_,struct RelayProc,proc);
	struct Iterator IForward,INow;

	INow = GetIterator(&pRelayProc->ln);
	IForward = GetIterator(INow.now->next);
	FreeRelayProc(&sta_IRelayProc,&INow,&IForward,NULL);
		
	g_Relays --;
	printf("relay proc end\n");
}

uchar 
RelayProcMerge(uint pa_RelayID,struct NetAddr pa_addr,struct ProcessingList *pa_pProcList,struct Iterator *pa_pINow, struct Iterator *pa_pIForward,struct Sock *pa_pSock)
{
	struct RelayProc *pRelayProc;
	struct FindRelayProcByRelayIDPa FRPBRIPa;
	
	FRPBRIPa.pFound = NULL;
	FRPBRIPa.RelayID = pa_RelayID;
	ForEach(&sta_IRelayProc,&FindRelayProcByRelayID,&FRPBRIPa);

	if(FRPBRIPa.pFound == NULL)
	{
		pRelayProc = tkmalloc(struct RelayProc);
		RelayProcCons(pRelayProc);

		pRelayProc->RelayID = pa_RelayID;
		pRelayProc->peer0.addr = pa_addr;
		pRelayProc->pSock = pa_pSock;
		pRelayProc->proc.NotifyCallbk = &RelayProcNotify;

		AddOneToListTail(&sta_IRelayProc,&pRelayProc->ln);
		ProcessSafeStart(&pRelayProc->proc,pa_pProcList,pa_pINow,pa_pIForward);

		return RELAY_MERGE_RES_NEW_RELAY;
	}
	else if(FRPBRIPa.pFound->peer1.addr.port == 0)
	{
		FRPBRIPa.pFound->peer1.addr = pa_addr;
		g_Relays ++;

		return RELAY_MERGE_RES_MERGED;
	}

	return RELAY_MERGE_RES_WAITING;
}

BOOL
LIST_ITERATION_CALLBACK_FUNCTION(TraceRelayProc)
{
	struct RelayProc *pRelayProc
		= GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct RelayProc,ln);
	char AddrText0[32];
	char AddrText1[32];
		
	GetAddrText(&(pRelayProc->peer0.addr),AddrText0);

	if(pRelayProc->peer1.addr.port == 0)
	{
		strcpy(AddrText1,"waiting");
	}
	else
	{
		GetAddrText(&(pRelayProc->peer1.addr),AddrText1);
	}
	
	printf("RelayID=%d,between %s & %s \n",pRelayProc->RelayID,AddrText0,AddrText1);

	return pa_pINow->now == pa_pIHead->last;
}

void 
RelayProcTrace()
{
	printf("relays(total = %d): \n",g_Relays);
	ForEach(&sta_IRelayProc,&TraceRelayProc,NULL);
}
