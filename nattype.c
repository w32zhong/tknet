#include "tknet.h"

STEP( BindingRequest )
{
	struct STUNProc *pProc = GET_STRUCT_ADDR(pa_pProc , struct STUNProc , proc);
	struct StunHead stun;
	char            AddrStr[32];
	struct NetAddr  AddrTemp;

	if( SockRead( pProc->pSock ) )
	{
		AddrTemp = StunGetPublicNetAddr(pProc->pSock->RecvBuff,pProc->pSock->RecvLen,pProc->MagicCookieTemp);
		VCK( ifGetPublicAddrSucc(&AddrTemp) == 0 , return PS_CALLBK_RET_GO_ON; );
		pProc->MapAddrTemp = AddrTemp; 

		AddrTemp = GetAddrFromSockAddr(&pProc->pSock->AddrRecvfrom);
		pProc->ServerAddrTemp = AddrTemp;
		
		GetAddrText(&pProc->ServerAddrTemp,AddrStr);
		printf("recv STUN from %s ,",AddrStr);
		GetAddrText(&pProc->MapAddrTemp,AddrStr);
		printf("saying that my public address is %s\n",AddrStr);

		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		pProc->MagicCookieTemp = StunFormulateRequest( &stun );
		SockLocateTa( pProc->pSock , pProc->HostIPVal , pProc->HostPort );
		SockWrite( pProc->pSock , BYS(stun) );
		printf("Binding request..\n");
	}
	else if( pa_state == PS_STATE_LAST_TIME )
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( ChangeIP )
{
	struct STUNProc *pProc = GET_STRUCT_ADDR(pa_pProc , struct STUNProc , proc);
	struct ChangeRequest cr_stun;
	char   AddrStr[32];
	struct NetAddr AddrTemp;

	if( SockRead( pProc->pSock ) )
	{
		AddrTemp = StunGetPublicNetAddr(pProc->pSock->RecvBuff,pProc->pSock->RecvLen,pProc->MagicCookieTemp);
		VCK( ifGetPublicAddrSucc(&AddrTemp) == 0 , return PS_CALLBK_RET_GO_ON; );
		pProc->MapAddrTemp = AddrTemp; 

		AddrTemp = GetAddrFromSockAddr(&pProc->pSock->AddrRecvfrom);
		VCK( AddrTemp.IPv4 == pProc->ServerAddrTemp.IPv4 , return PS_CALLBK_RET_ABORT; );//check if IP has been changed. 
		pProc->ServerAddrTemp = AddrTemp;
		
		GetAddrText(&pProc->ServerAddrTemp,AddrStr);
		printf("recv STUN from %s ,",AddrStr);
		GetAddrText(&pProc->MapAddrTemp,AddrStr);
		printf("saying that my public address is %s\n",AddrStr);

		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		pProc->MagicCookieTemp = StunFormulateChangeRequest( &cr_stun , STUN_CHANGE_IP );
		SockLocateTa( pProc->pSock , pProc->HostIPVal , pProc->HostPort );
		SockWrite( pProc->pSock , BYS(cr_stun) );
	}
	else if( pa_state == PS_STATE_LAST_TIME )
	{
		return FlagNum(3);
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( BindingRequestToAnotherIP )
{
	struct STUNProc *pProc = GET_STRUCT_ADDR(pa_pProc , struct STUNProc , proc);
	struct StunHead stun;
	char            AddrStr[32];
	struct NetAddr  AddrTemp;

	if( SockRead( pProc->pSock ) )
	{
		AddrTemp = StunGetPublicNetAddr(pProc->pSock->RecvBuff,pProc->pSock->RecvLen,pProc->MagicCookieTemp);
		VCK( ifGetPublicAddrSucc(&AddrTemp) == 0 , return PS_CALLBK_RET_GO_ON; );
		pProc->MapAddrTemp = AddrTemp; 

		AddrTemp = GetAddrFromSockAddr(&pProc->pSock->AddrRecvfrom);
		VCK( AddrTemp.IPv4 != pProc->ServerAddrTemp.IPv4 , return PS_CALLBK_RET_ABORT; );
		
		if( AddrTemp.port == pProc->ServerAddrTemp.port )
		{
			pProc->NatTypeRes = NAT_T_FULL_CONE;
		}
		else
		{
			pProc->NatTypeRes = NAT_T_SYMMETRIC;
		}

		pProc->ServerAddrTemp = AddrTemp;
		
		GetAddrText(&pProc->ServerAddrTemp,AddrStr);
		printf("recv STUN from %s ,",AddrStr);
		GetAddrText(&pProc->MapAddrTemp,AddrStr);
		printf("saying that my public address is %s\n",AddrStr);

		return PS_CALLBK_RET_ABORT;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		pProc->MagicCookieTemp = StunFormulateRequest( &stun );
		SockLocateTa( pProc->pSock , htonl(pProc->ServerAddrTemp.IPv4) , pProc->ServerAddrTemp.port );
		SockWrite( pProc->pSock , BYS(stun) );
	}
	else if( pa_state == PS_STATE_LAST_TIME )
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( ChangePort )
{
	struct STUNProc *pProc = GET_STRUCT_ADDR(pa_pProc , struct STUNProc , proc);
	struct ChangeRequest cr_stun;
	char            AddrStr[32];
	struct NetAddr  AddrTemp;

	if( SockRead( pProc->pSock ) )
	{
		AddrTemp = StunGetPublicNetAddr(pProc->pSock->RecvBuff,pProc->pSock->RecvLen,pProc->MagicCookieTemp);
		VCK( ifGetPublicAddrSucc(&AddrTemp) == 0 , return PS_CALLBK_RET_GO_ON; );
		pProc->MapAddrTemp = AddrTemp; 

		AddrTemp = GetAddrFromSockAddr(&pProc->pSock->AddrRecvfrom);
		pProc->ServerAddrTemp = AddrTemp;
		
		GetAddrText(&pProc->ServerAddrTemp,AddrStr);
		printf("recv STUN from %s ,",AddrStr);
		GetAddrText(&pProc->MapAddrTemp,AddrStr);
		printf("saying that my public address is %s\n",AddrStr);

		pProc->NatTypeRes = NAT_T_RESTRICTED;

		return PS_CALLBK_RET_ABORT;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		pProc->MagicCookieTemp = StunFormulateChangeRequest( &cr_stun , STUN_CHANGE_PORT );
		SockLocateTa( pProc->pSock , pProc->HostIPVal , pProc->HostPort );
		SockWrite( pProc->pSock , BYS(cr_stun) );
	}
	else if( pa_state == PS_STATE_LAST_TIME )
	{
		pProc->NatTypeRes = NAT_T_PORT_RESTRICTED;

		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

void 
MakeProtoStunProc( struct STUNProc *pa_pStunProc ,struct Sock *pa_pSock , const char *pa_pHostIP , ushort pa_HostPort )
{
	ProcessCons( &pa_pStunProc->proc );
	PROCESS_ADD_STEP( &pa_pStunProc->proc , BindingRequest , 2000 , 2 );
	PROCESS_ADD_STEP( &pa_pStunProc->proc , ChangeIP , 2000 , 2 );
	PROCESS_ADD_STEP( &pa_pStunProc->proc , BindingRequestToAnotherIP , 2000 , 2 );
	PROCESS_ADD_STEP( &pa_pStunProc->proc , ChangePort , 2000 , 2 );

	pa_pStunProc->HostIPVal = GetIPVal( pa_pHostIP );
	pa_pStunProc->HostPort = pa_HostPort;
	pa_pStunProc->pSock = pa_pSock;
	pa_pStunProc->NatTypeRes = NAT_T_UNKNOWN;
}
