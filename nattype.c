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

STEP( BindingRequest )
{
	struct STUNProc *pProc = GET_STRUCT_ADDR(pa_pProc , struct STUNProc , proc);
	struct StunHead stun;
	char            AddrStr[32];
	struct NetAddr  AddrTemp;

	if( SockRead( pProc->pSock ) )
	{
		VCK( 0 == StunGetResult(pProc->pSock->RecvBuff,
				pProc->pSock->RecvLen,pProc->MagicCookieTemp,
				&pProc->MapAddr,&pProc->ChangeAddr) ,return PS_CALLBK_RET_GO_ON; );

		//print details 
		AddrTemp = GetAddrFromSockAddr(&pProc->pSock->AddrRecvfrom);
		GetAddrText(&AddrTemp,AddrStr);
		printf("recv STUN from %s ,",AddrStr);
		GetAddrText(&pProc->MapAddr,AddrStr);
		printf("saying that my public address is %s",AddrStr);
		GetAddrText(&pProc->ChangeAddr,AddrStr);
		printf(" and 'change address' is %s\n",AddrStr);

		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		pProc->MagicCookieTemp = StunFormulateRequest( &stun );
		SockLocateTa( pProc->pSock , pProc->HostIPVal , pProc->HostPort );
		SockWrite( pProc->pSock , BYS(stun) );
		
		AddrTemp = GetAddrFromSockAddr(&pProc->pSock->AddrTa);
		GetAddrText(&AddrTemp,AddrStr);
		printf("Binding request to %s..\n",AddrStr);
	}
	else if( pa_state == PS_STATE_LAST_TIME )
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( ChangeIPAndPort )
{
	struct STUNProc *pProc = GET_STRUCT_ADDR(pa_pProc , struct STUNProc , proc);
	struct ChangeRequest cr_stun;
	char            AddrStr[32];
	struct NetAddr  AddrTemp;

	if( SockRead( pProc->pSock ) )
	{
		VCK( 0 == StunGetResult(pProc->pSock->RecvBuff,
				pProc->pSock->RecvLen,pProc->MagicCookieTemp,
				&AddrTemp,&AddrTemp) ,return PS_CALLBK_RET_GO_ON; );
		//just to check whether it is a valid STUN datagram

		AddrTemp = GetAddrFromSockAddr(&pProc->pSock->AddrRecvfrom);

		VCK( AddrTemp.IPv4 == ntohl(pProc->HostIPVal) , return PS_CALLBK_RET_GO_ON; );
		VCK( AddrTemp.port == pProc->HostPort , return PS_CALLBK_RET_GO_ON; );
		//check if IP and port both has changed.

		GetAddrText(&AddrTemp,AddrStr);
		printf("recv STUN from %s \n",AddrStr);
		//print details 

		pProc->NatTypeRes = NAT_T_FULL_CONE;
		//we are not sure if it is a 'cone NAT' actually , but we mark it as full-cone NAT
		//in advance . we make confirm in next step by testing whether it map the same port
		//session when sending to a different IP.

		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		pProc->MagicCookieTemp = StunFormulateChangeRequest( &cr_stun , STUN_CHANGE_BOTH_IP_PORT );
		SockLocateTa( pProc->pSock , pProc->HostIPVal , pProc->HostPort );
		SockWrite( pProc->pSock , BYS(cr_stun) );
		
		AddrTemp = GetAddrFromSockAddr(&pProc->pSock->AddrTa);
		GetAddrText(&AddrTemp,AddrStr);
		printf("Sending ChangeIPAndPort request to %s ..\n",AddrStr);
	}
	else if( pa_state == PS_STATE_LAST_TIME )
	{
		return PS_CALLBK_RET_DONE;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( BindingRequestToAnotherServer )
{
	struct STUNProc *pProc = GET_STRUCT_ADDR(pa_pProc , struct STUNProc , proc);
	struct StunHead stun;
	char            AddrStr[32];
	struct NetAddr  AddrTemp;

	if( SockRead( pProc->pSock ) )
	{
		AddrTemp = GetAddrFromSockAddr(&pProc->pSock->AddrRecvfrom);
		VCK( AddrTemp.IPv4 != pProc->ChangeAddr.IPv4 , return PS_CALLBK_RET_GO_ON; );

		VCK( 0 == StunGetResult(pProc->pSock->RecvBuff,
				pProc->pSock->RecvLen,pProc->MagicCookieTemp,
				&AddrTemp,&pProc->ChangeAddr) ,return PS_CALLBK_RET_GO_ON; );
		
		if( AddrTemp.port != pProc->MapAddr.port )
		{
			pProc->NatTypeRes = NAT_T_SYMMETRIC;
		}
		
		pProc->MapAddr = AddrTemp;

		//print details 
		AddrTemp = GetAddrFromSockAddr(&pProc->pSock->AddrRecvfrom);
		GetAddrText(&AddrTemp,AddrStr);
		printf("recv STUN from %s ,",AddrStr);
		GetAddrText(&pProc->MapAddr,AddrStr);
		printf("saying that my public address is %s\n",AddrStr);

		if( pProc->NatTypeRes == NAT_T_SYMMETRIC )
		{
			return PS_CALLBK_RET_ABORT;
		}
		else if( pProc->NatTypeRes == NAT_T_FULL_CONE )
		{
			//now we assure that it is full-cone.
			return PS_CALLBK_RET_ABORT;
		}
		else
		{
			return PS_CALLBK_RET_DONE;
		}
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		pProc->MagicCookieTemp = StunFormulateRequest( &stun );
		SockLocateTa( pProc->pSock , htonl(pProc->ChangeAddr.IPv4) , STUN_DEFAULT_PORT );
		//use STUN_DEFAULT_PORT instead of the port returned by CHANGE_ADDRESS
		SockWrite( pProc->pSock , BYS(stun) );
		
		AddrTemp = GetAddrFromSockAddr(&pProc->pSock->AddrTa);
		GetAddrText(&AddrTemp,AddrStr);
		printf("Binding request to server #2(%s)..\n",AddrStr);
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
		VCK( 0 == StunGetResult(pProc->pSock->RecvBuff,
				pProc->pSock->RecvLen,pProc->MagicCookieTemp,
				&AddrTemp,&AddrTemp) ,return PS_CALLBK_RET_GO_ON; );
		//just to check whether it is a valid STUN datagram

		AddrTemp = GetAddrFromSockAddr(&pProc->pSock->AddrRecvfrom);

		VCK( AddrTemp.IPv4 != ntohl(pProc->HostIPVal) , return PS_CALLBK_RET_GO_ON; );
		VCK( AddrTemp.port == pProc->HostPort , return PS_CALLBK_RET_GO_ON; );
		//check if port has changed while IP stays the same.

		GetAddrText(&AddrTemp,AddrStr);
		printf("recv STUN from %s \n",AddrStr);
		//print details 

		pProc->NatTypeRes = NAT_T_RESTRICTED;
		return PS_CALLBK_RET_ABORT;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		pProc->MagicCookieTemp = StunFormulateChangeRequest( &cr_stun , STUN_CHANGE_PORT );
		SockLocateTa( pProc->pSock , pProc->HostIPVal , pProc->HostPort );
		SockWrite( pProc->pSock , BYS(cr_stun) );
		
		AddrTemp = GetAddrFromSockAddr(&pProc->pSock->AddrTa);
		GetAddrText(&AddrTemp,AddrStr);
		printf("Sending ChangePort request to %s ..\n",AddrStr);
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
	PROCESS_ADD_STEP( &pa_pStunProc->proc , ChangeIPAndPort , 2000 , 2 );
	//put ChangeIPAndPort to test whether it's under a Full-cone NAT *Before* BindingRequestToAnotherServer
	//because BindingRequestToAnotherServer will punch a hole which will lead a false judgement later.
	PROCESS_ADD_STEP( &pa_pStunProc->proc , BindingRequestToAnotherServer , 2000 , 2 );
	PROCESS_ADD_STEP( &pa_pStunProc->proc , ChangePort , 2000 , 2 );
	//we can assert that we were under a Port-restricted cone NAT if nothing recved from ChangePort step
	//without further test, because Port-restricted cone NAT actually is the most stricted NAT of all types of
	//cone NATs and we have already confirmed our NAT is a kind of cone NAT in the BindingRequestToAnotherServer
	//step.

	pa_pStunProc->HostIPVal = GetIPVal( pa_pHostIP );
	pa_pStunProc->HostPort = pa_HostPort;
	pa_pStunProc->pSock = pa_pSock;
	pa_pStunProc->NatTypeRes = NAT_T_UNKNOWN;
}
