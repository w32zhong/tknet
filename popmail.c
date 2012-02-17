
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

STEP( ProtoPOP3Connect )
{
	struct POP3Proc *pProc = GET_STRUCT_ADDR( pa_pProc , struct POP3Proc , proc );

	if( !SockLocateTa( pProc->pSock , pProc->HostIPVal , pProc->HostPort ) )
	{
		return PS_CALLBK_RET_ABORT;
	}

	if( pProc->ifEnableSSL )
	{
		SockSSLConnect( pProc->pSock );
	}

	SockSetNonblock( pProc->pSock );

	return PS_CALLBK_RET_DONE;
}

STEP( ProtoPOP3FirstRecv )
{
	struct POP3Proc *pProc = GET_STRUCT_ADDR( pa_pProc , struct POP3Proc , proc );

	if( SockRead( pProc->pSock ) )
	{
		printf("%s\n",pProc->pSock->RecvBuff);
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME )
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( ProtoPOP3User  )
{
	struct POP3Proc *pProc = GET_STRUCT_ADDR( pa_pProc , struct POP3Proc , proc );
	char StrBuff[POP3_PROTO_USRNAME_MAX_LEN];
	strcpy( StrBuff , "USER " );
	strcat( StrBuff , pProc->UsrName );
	strcat( StrBuff , "\r\n" );

	if( SockRead( pProc->pSock ) )
	{
		printf("%s\n",pProc->pSock->RecvBuff);
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		printf("write: %s \n", StrBuff );
		SockWrite( pProc->pSock , StrBys(StrBuff) );
	}
	else if( pa_state == PS_STATE_LAST_TIME )
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( ProtoPOP3Password  )
{
	struct POP3Proc *pProc = GET_STRUCT_ADDR( pa_pProc , struct POP3Proc , proc );
	char StrBuff[POP3_PROTO_PASSWORD_MAX_LEN];
	strcpy( StrBuff , "PASS " );
	strcat( StrBuff , pProc->PassWord );
	strcat( StrBuff , "\r\n" );

	if( SockRead( pProc->pSock ) )
	{
		printf("%s\n",pProc->pSock->RecvBuff);
		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		printf("write: %s \n", StrBuff );
		SockWrite( pProc->pSock , StrBys(StrBuff) );
	}
	else if( pa_state == PS_STATE_LAST_TIME )
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( ProtoPOP3List  )
{
	struct POP3Proc *pProc = GET_STRUCT_ADDR( pa_pProc , struct POP3Proc , proc );
	char *pBuff;
	int i ;
	struct NetInfoMail *pNewMail;

	if( SockRead( pProc->pSock ) )
	{
		pProc->ifEnterSucc = 1;

		printf("%s \n",pProc->pSock->RecvBuff);

		pBuff = pProc->pSock->RecvBuff;

		while(1)
		{
			pBuff = strstr( pBuff , "\r\n" );
	
			if(pBuff != NULL && pBuff[2] != '.' )
			{
				pBuff += 2;
				for(i = 0; pBuff[i] != '\r' ; i++)
				{
					if( pBuff[i] == ' ')
					{
						pBuff[i] = '\0';
						i ++;
						break;
					}
				}

				pNewMail = tkmalloc( struct NetInfoMail );
				ListNodeCons( &pNewMail->ln );
				sscanf( pBuff ,"%d", &pNewMail->num );
				pNewMail->content[0] = '\0';
				AddOneToListTail( &pProc->IMailsHead , &pNewMail->ln );

				pBuff += i;
			}
			else
			{
				break;
			}
		}

		if( pProc->IMailsHead.now != NULL )
		{
			pProc->IRetrieveNow = GetIterator( pProc->IMailsHead.now->last );
			pProc->MailContentBuff[0] = '\0';
		}
		else
		{
			return FlagNum(6);
		}

		return PS_CALLBK_RET_DONE;
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		SockWrite( pProc->pSock , StrBys("LIST\r\n") );
	}
	else if( pa_state == PS_STATE_LAST_TIME )
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

static void 
ParseSpecialContentToSpace(char *);
	
STEP( ProtoPOP3Retr )
{
	struct POP3Proc *pProc = GET_STRUCT_ADDR( pa_pProc , struct POP3Proc , proc );
	struct NetInfoMail *pMail = NULL;
	char StrBuff[16];
	char *pUsefulContent , *pUsefulContentEnd;

	if( pProc->IRetrieveNow.now == NULL )
	{
		return PS_CALLBK_RET_DONE;
	}

	pMail = GET_STRUCT_ADDR_FROM_IT( &pProc->IRetrieveNow , struct NetInfoMail , ln );

	if( SockRead( pProc->pSock ) )
	{
		if( pProc->pSock->RecvBuff[0] != '+')
		{
			VCK( strlen(pProc->MailContentBuff) + strlen(pProc->pSock->RecvBuff) > SOCK_RECV_BUFF_LEN - 1 , goto nextmail; );

			strcat( pProc->MailContentBuff , pProc->pSock->RecvBuff );
		}
		else
		{
			strcpy( pProc->MailContentBuff , pProc->pSock->RecvBuff );
		}

		pUsefulContent = pProc->MailContentBuff;
		pUsefulContent = strstr(pUsefulContent ,"\r\n\r\n");

		if( pUsefulContent != NULL )
		{
			pUsefulContentEnd = strstr(pUsefulContent ,"\r\n.\r\n");
			
			if( pUsefulContentEnd != NULL )
			{
				pUsefulContent += 4;
				*pUsefulContentEnd = '\0';

				VCK( strlen(pUsefulContent) > MAX_MAIL_CONTENT_LEN - 1, goto nextmail;);
				strcpy( pMail->content , pUsefulContent );
				ParseSpecialContentToSpace(pMail->content);
			}
			else
			{
				return PS_CALLBK_RET_GO_ON;
			}
		}
		else
		{
			return PS_CALLBK_RET_GO_ON;
		}

nextmail:

		pProc->IRetrieveNow = GetIterator( pProc->IRetrieveNow.now->last );

		if( pProc->IRetrieveNow.now == pProc->IMailsHead.last )
		{
			return PS_CALLBK_RET_DONE;
		}
		else
		{
			if( pProc->Retrieves != 0 )
			{
				pProc->Retrieves --;
			}

			if( pProc->Retrieves != 0 )
			{
				pProc->MailContentBuff[0] = '\0';
				return PS_CALLBK_RET_REDO;
			}
			else
			{
				return PS_CALLBK_RET_DONE;
			}
		}
	}
	else if( pa_state == PS_STATE_OVERTIME || pa_state == PS_STATE_FIRST_TIME )
	{
		sprintf( StrBuff , "RETR %d\r\n" , pMail->num );
		SockWrite( pProc->pSock , StrBys(StrBuff) );
		printf("write: %s \n", StrBuff );
	}
	else if( pa_state == PS_STATE_LAST_TIME )
	{
		return PS_CALLBK_RET_ABORT;
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( ProtoPOP3Quit  )
{
	struct POP3Proc *pProc = GET_STRUCT_ADDR( pa_pProc , struct POP3Proc , proc );
	char StrBuff[] = "QUIT\r\n";

	printf("write: %s \n", StrBuff );
	SockWrite( pProc->pSock , StrBys(StrBuff) );

	return PS_CALLBK_RET_DONE;
}

void 
MakeProtoPOP3Proc( struct POP3Proc *pa_pPop3Proc , const char *pa_pHostIP , ushort pa_HostPort , BOOL pa_ifEnableSSL , const char *pa_pUsrName , const char *pa_pPassWord )
{
	ProcessCons( &pa_pPop3Proc->proc );
	PROCESS_ADD_STEP( &pa_pPop3Proc->proc , ProtoPOP3Connect , 1000 , 0 );
	PROCESS_ADD_STEP( &pa_pPop3Proc->proc , ProtoPOP3FirstRecv , 1000 , 0 );
	PROCESS_ADD_STEP( &pa_pPop3Proc->proc , ProtoPOP3User , 2000 , 2 );
	PROCESS_ADD_STEP( &pa_pPop3Proc->proc , ProtoPOP3Password , 2000 , 2 );
	PROCESS_ADD_STEP( &pa_pPop3Proc->proc , ProtoPOP3List , 2000 , 2 );
	PROCESS_ADD_STEP( &pa_pPop3Proc->proc , ProtoPOP3Retr , 2000 , 2 );
	PROCESS_ADD_STEP( &pa_pPop3Proc->proc , ProtoPOP3Quit , 1000 , 0 );

	pa_pPop3Proc->HostIPVal = GetIPVal( pa_pHostIP );
	pa_pPop3Proc->HostPort = pa_HostPort;
	pa_pPop3Proc->ifEnableSSL = pa_ifEnableSSL;
	strcpy( pa_pPop3Proc->UsrName , pa_pUsrName );
	strcpy( pa_pPop3Proc->PassWord , pa_pPassWord );
	pa_pPop3Proc->IMailsHead = GetIterator(NULL);
	pa_pPop3Proc->IRetrieveNow = GetIterator(NULL);
	pa_pPop3Proc->Retrieves = 5;
	pa_pPop3Proc->ifEnterSucc = 0;
}

BOOL 
LIST_ITERATION_CALLBACK_FUNCTION( TraceMail )
{
	struct NetInfoMail *pMail = GET_STRUCT_ADDR_FROM_IT( pa_pINow , struct NetInfoMail , ln);
	printf("mail No.%d :\n", pMail->num);
	StrTraceFormat(pMail->content);
	printf("END\n\n");

	return pa_pINow->now == pa_pIHead->last;
}

DEF_FREE_LIST_ELEMENT_CALLBACK_FUNCTION( FreeNetInfoMail , struct NetInfoMail , ln , ; )

void 
POP3ProcFree( struct POP3Proc *pa_pPop3Proc )
{
	ForEach( &pa_pPop3Proc->IMailsHead , &FreeNetInfoMail , NULL );
	ProcessFree(&pa_pPop3Proc->proc);
}

static void 
ParseSpecialContentToSpace(char *pa_pStr)
{
	char *res;

	while(1)
	{
		res = strstr(pa_pStr,"=C2=A0");
		//=C2=A0 represents the bytes C2 A0
		//which is the Unicode for non-breaking 
		//space(&nbsp;) in MIME.

		if(res)
		{
			res[0] = ' ';
			res[1] = ' ';
			res[2] = ' ';
			res[3] = ' ';
			res[4] = ' ';
			res[5] = ' ';
		}
		else
		{
			break;
		}
	}

	while(1)
	{
		res = strstr(pa_pStr,"\r");
		if(res)
		{
			res[0] = ' ';
		}
		else
		{
			break;
		}
	}

	while(1)
	{
		res = strstr(pa_pStr,"\n");
		if(res)
		{
			res[0] = ' ';
		}
		else
		{
			break;
		}
	}
}
