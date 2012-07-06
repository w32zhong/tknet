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

STEP( SMTPConnect )
{
	struct SMTPProc *pProc = GET_STRUCT_ADDR( pa_pProc , struct SMTPProc , proc );

	if( !SockLocateTa( &pProc->Sock , pProc->HostIPVal , pProc->HostPort ) )
	{
		return PS_CALLBK_RET_ABORT;
	}

	if( pProc->ifEnableSSL )
	{
		SockSSLConnect( &pProc->Sock );
	}

	SockSetNonblock( &pProc->Sock );

	return PS_CALLBK_RET_DONE;
}

STEP( SMTPFirstRecv )
{
	struct SMTPProc *pProc = GET_STRUCT_ADDR( pa_pProc , struct SMTPProc , proc );

	if( SockRead( &pProc->Sock ) )
	{
		PROMPT(Usual,"return %s\n", pProc->Sock.RecvBuff );
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME )
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( SMTPHello  )
{
	struct SMTPProc *pProc = GET_STRUCT_ADDR( pa_pProc , struct SMTPProc , proc );
	char StrBuff[] = "HELO you\r\n";
	
	if( SockRead( &pProc->Sock ) )
	{
		PROMPT(Usual,"return %s\n", pProc->Sock.RecvBuff );
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		PROMPT(Usual,"write: %s \n", StrBuff );
		SockWrite( &pProc->Sock , StrBys(StrBuff) );
	}
	else if( pa_state == PS_STATE_LAST_TIME )
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( SMTPStartAuth  )
{
	struct SMTPProc *pProc = GET_STRUCT_ADDR( pa_pProc , struct SMTPProc , proc );
	char StrBuff[] = "AUTH LOGIN\r\n";
	
	if( SockRead( &pProc->Sock ) )
	{
		PROMPT(Usual,"return %s\n", pProc->Sock.RecvBuff );
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		PROMPT(Usual,"write: %s \n", StrBuff );
		SockWrite( &pProc->Sock , StrBys(StrBuff) );
	}
	else if( pa_state == PS_STATE_LAST_TIME )
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( SMTPUsrName  )
{
	struct SMTPProc *pProc = GET_STRUCT_ADDR( pa_pProc , struct SMTPProc , proc );
	char Base64Buff[SMTP_PROTO_USRNAME_MAX_LEN];

	if( SockRead( &pProc->Sock ) )
	{
		PROMPT(Usual,"return %s\n", pProc->Sock.RecvBuff );
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		Base64Encode( pProc->UsrName, strlen(pProc->UsrName) , Base64Buff );
		PROMPT(Usual,"write %s \n",Base64Buff);
		strcat(Base64Buff,"\r\n");
		SockWrite( &pProc->Sock , StrBys(Base64Buff) );
	}
	else if( pa_state == PS_STATE_LAST_TIME )
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( SMTPPassWord  )
{
	struct SMTPProc *pProc = GET_STRUCT_ADDR( pa_pProc , struct SMTPProc , proc );
	char Base64Buff[SMTP_PROTO_PASSWORD_MAX_LEN];

	if( SockRead( &pProc->Sock ) )
	{
		PROMPT(Usual,"return %s\n", pProc->Sock.RecvBuff );
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		Base64Encode( pProc->PassWord, strlen(pProc->PassWord) , Base64Buff );
		PROMPT(Usual,"write %s \n",Base64Buff);
		strcat(Base64Buff,"\r\n");
		SockWrite( &pProc->Sock , StrBys(Base64Buff) );
	}
	else if( pa_state == PS_STATE_LAST_TIME )
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( SMTPMailFrom  )
{
	struct SMTPProc *pProc = GET_STRUCT_ADDR( pa_pProc , struct SMTPProc , proc );
	char StrBuff[ SMTP_PROTO_MAIL_ADDR_MAX_LEN + 12 ];

	if( SockRead( &pProc->Sock ) )
	{
		PROMPT(Usual,"return %s\n", pProc->Sock.RecvBuff );
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		sprintf(StrBuff , "MAIL FROM:<%s>\r\n",pProc->MailAddr);
		PROMPT(Usual,"write: %s \n", StrBuff );
		SockWrite( &pProc->Sock , StrBys(StrBuff) );
	}
	else if( pa_state == PS_STATE_LAST_TIME )
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( SMTPRcptTo  )
{
	struct SMTPProc *pProc = GET_STRUCT_ADDR( pa_pProc , struct SMTPProc , proc );
	char StrBuff[ SMTP_PROTO_MAIL_ADDR_MAX_LEN + 12 ];

	if( SockRead( &pProc->Sock ) )
	{
		PROMPT(Usual,"return %s\n", pProc->Sock.RecvBuff );
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		sprintf(StrBuff , "RCPT TO:<%s>\r\n",pProc->MailAddr);
		PROMPT(Usual,"write: %s \n", StrBuff );
		SockWrite( &pProc->Sock , StrBys(StrBuff) );
	}
	else if( pa_state == PS_STATE_LAST_TIME )
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( SMTPData  )
{
	struct SMTPProc *pProc = GET_STRUCT_ADDR( pa_pProc , struct SMTPProc , proc );
	char StrBuff[] = "DATA\r\nSubject: t.k. net\r\n";
	
	if( SockRead( &pProc->Sock ) )
	{
		PROMPT(Usual,"return %s\n", pProc->Sock.RecvBuff );
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		PROMPT(Usual,"write: %s \n", StrBuff );
		SockWrite( &pProc->Sock , StrBys(StrBuff) );
	}
	else if( pa_state == PS_STATE_LAST_TIME )
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( SMTPTitle  )
{
	struct SMTPProc *pProc = GET_STRUCT_ADDR( pa_pProc , struct SMTPProc , proc );
	char StrBuff[ SMTP_PROTO_MAIL_ADDR_MAX_LEN + 12 ];

	sprintf(StrBuff , "FROM:tknet<%s>\r\n",pProc->MailAddr);
	PROMPT(Usual,"write: %s \n", StrBuff );
	SockWrite( &pProc->Sock , StrBys(StrBuff) );

	sprintf(StrBuff , "TO:tknet<%s>\r\n",pProc->MailAddr);
	PROMPT(Usual,"write: %s \n", StrBuff );
	SockWrite( &pProc->Sock , StrBys(StrBuff) );

	return PS_CALLBK_RET_DONE;
}

STEP( SMTPContent  )
{
	struct SMTPProc *pProc = GET_STRUCT_ADDR( pa_pProc , struct SMTPProc , proc );
	char StrBuff[ MAX_MAIL_CONTENT_LEN + 8 ];

	if( SockRead( &pProc->Sock ) )
	{
		PROMPT(Usual,"return %s\n", pProc->Sock.RecvBuff );
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		sprintf(StrBuff , "\r\n%s\r\n.\r\n",pProc->SendBuff);
		PROMPT(Usual,"write: %s \n", StrBuff );
		SockWrite( &pProc->Sock , StrBys(StrBuff) );
	}
	else if( pa_state == PS_STATE_LAST_TIME )
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( SMTPQuit  )
{
	struct SMTPProc *pProc = GET_STRUCT_ADDR( pa_pProc , struct SMTPProc , proc );
	char StrBuff[] = "QUIT\r\n";
	
	if( SockRead( &pProc->Sock ) )
	{
		PROMPT(Usual,"return %s\n", pProc->Sock.RecvBuff );
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		PROMPT(Usual,"write: %s \n", StrBuff );
		SockWrite( &pProc->Sock , StrBys(StrBuff) );
	}
	else if( pa_state == PS_STATE_LAST_TIME )
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

void 
SMTPProcMake( struct SMTPProc *pa_pSMTPProc , const char *pa_pHostIPText , ushort pa_HostPort , BOOL pa_ifEnableSSL , const char *pa_pUsrName , const char *pa_pPassWord , const char *pa_pMailAddress , const char *pa_pSendBuff )
{
	ProcessCons( &pa_pSMTPProc->proc );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPConnect , g_WaitLevel[1] );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPFirstRecv , g_WaitLevel[1] );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPHello , g_WaitLevel[1] );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPStartAuth , g_WaitLevel[2] );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPUsrName , g_WaitLevel[2] );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPPassWord , g_WaitLevel[2] );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPMailFrom , g_WaitLevel[2] );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPRcptTo , g_WaitLevel[2] );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPData , g_WaitLevel[2] );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPTitle , g_WaitLevel[2] );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPContent , g_WaitLevel[2] );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPQuit , g_WaitLevel[2] );

	SockOpen( &pa_pSMTPProc->Sock , TCP , 0 );

	pa_pSMTPProc->HostIPVal = GetIPVal( pa_pHostIPText ); 
	pa_pSMTPProc->HostPort = pa_HostPort;
	pa_pSMTPProc->ifEnableSSL = pa_ifEnableSSL;

	strcpy( pa_pSMTPProc->UsrName , pa_pUsrName );
	strcpy( pa_pSMTPProc->PassWord , pa_pPassWord );
	strcpy( pa_pSMTPProc->MailAddr , pa_pMailAddress );
	strcpy( pa_pSMTPProc->SendBuff , pa_pSendBuff  );
}
