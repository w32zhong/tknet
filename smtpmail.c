#include "headers.h"

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
		printf("return %s\n", pProc->Sock.RecvBuff );
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
		printf("return %s\n", pProc->Sock.RecvBuff );
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		printf("write: %s \n", StrBuff );
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
		printf("return %s\n", pProc->Sock.RecvBuff );
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		printf("write: %s \n", StrBuff );
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
		printf("return %s\n", pProc->Sock.RecvBuff );
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		Base64Encode( pProc->UsrName, strlen(pProc->UsrName) , Base64Buff );
		printf("write %s \n",Base64Buff);
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
		printf("return %s\n", pProc->Sock.RecvBuff );
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		Base64Encode( pProc->PassWord, strlen(pProc->PassWord) , Base64Buff );
		printf("write %s \n",Base64Buff);
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
		printf("return %s\n", pProc->Sock.RecvBuff );
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		sprintf(StrBuff , "MAIL FROM:<%s>\r\n",pProc->MailAddr);
		printf("write: %s \n", StrBuff );
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
		printf("return %s\n", pProc->Sock.RecvBuff );
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		sprintf(StrBuff , "RCPT TO:<%s>\r\n",pProc->MailAddr);
		printf("write: %s \n", StrBuff );
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
		printf("return %s\n", pProc->Sock.RecvBuff );
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		printf("write: %s \n", StrBuff );
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
	printf("write: %s \n", StrBuff );
	SockWrite( &pProc->Sock , StrBys(StrBuff) );

	sprintf(StrBuff , "TO:tknet<%s>\r\n",pProc->MailAddr);
	printf("write: %s \n", StrBuff );
	SockWrite( &pProc->Sock , StrBys(StrBuff) );

	return PS_CALLBK_RET_DONE;
}

STEP( SMTPContent  )
{
	struct SMTPProc *pProc = GET_STRUCT_ADDR( pa_pProc , struct SMTPProc , proc );
	char StrBuff[ MAX_MAIL_CONTENT_LEN + 8 ];

	if( SockRead( &pProc->Sock ) )
	{
		printf("return %s\n", pProc->Sock.RecvBuff );
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		sprintf(StrBuff , "\r\n%s\r\n.\r\n",pProc->SendBuff);
		printf("write: %s \n", StrBuff );
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
		printf("return %s\n", pProc->Sock.RecvBuff );
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		printf("write: %s \n", StrBuff );
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
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPConnect , 1000 , 0 );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPFirstRecv , 1000 , 0 );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPHello , 2000 , 2 );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPStartAuth , 2000 , 2 );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPUsrName , 2000 , 2 );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPPassWord , 2000 , 2 );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPMailFrom , 2000 , 2 );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPRcptTo , 2000 , 2 );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPData , 2000 , 2 );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPTitle , 1000 , 0 );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPContent , 2000 , 2 );
	PROCESS_ADD_STEP( &pa_pSMTPProc->proc , SMTPQuit , 2000 , 2 );

	SockOpen( &pa_pSMTPProc->Sock , TCP , 0 );

	pa_pSMTPProc->HostIPVal = GetIPVal( pa_pHostIPText ); 
	pa_pSMTPProc->HostPort = pa_HostPort;
	pa_pSMTPProc->ifEnableSSL = pa_ifEnableSSL;

	strcpy( pa_pSMTPProc->UsrName , pa_pUsrName );
	strcpy( pa_pSMTPProc->PassWord , pa_pPassWord );
	strcpy( pa_pSMTPProc->MailAddr , pa_pMailAddress );
	strcpy( pa_pSMTPProc->SendBuff , pa_pSendBuff  );
}
