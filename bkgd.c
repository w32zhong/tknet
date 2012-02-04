#include "headers.h"
#define BKGD_CMD_MAX_LEN 32

tkMutex g_BkgdMutex;

char    sta_BkgdCmd[BKGD_CMD_MAX_LEN];
BOOL    sta_ifBkgdCmdComing;
BOOL    sta_ifBkgdSubProcess = 0;
extern  BOOL g_MainLoopFlag;

static void 
BkgdProcEndCallbk(struct Process *pa_)
{
	printf("bkgd process end.\n");
	sta_ifBkgdSubProcess = 0;
}

STEP( ProtoPOP3BackGround )
{
	struct POP3Proc *pProc = GET_STRUCT_ADDR( pa_pProc , struct POP3Proc , proc );
	char cmd[BKGD_CMD_MAX_LEN];
	char *pChar;

	if( SockRead( pProc->pSock ) )
	{
		if( pProc->pSock->RecvLen == 0 )
		{
			//socket is closed by server side
			return PS_CALLBK_RET_ABORT;
		}
		else
		{
			printf("%s\n",pProc->pSock->RecvBuff);
		}
	}
	else if( pa_state == PS_STATE_LAST_TIME )
	{
		SockWrite( pProc->pSock , StrBys("NOOP\r\n") );
		return PS_CALLBK_RET_REDO;
	}

	if( sta_ifBkgdCmdComing )
	{
		strcpy(cmd , sta_BkgdCmd);
		pChar = strstr( cmd , " " );
		if( NULL != pChar )
		{
			*pChar = '\0';
		}

		if( strcmp(cmd , "quit") && 
		    strcmp(cmd , "dele") && 
		    strcmp(cmd , "list") && 
		    strcmp(cmd , "retr") )
		{
			printf("unknown POP3 cmd.\n");
		}
		else
		{
			strcat( sta_BkgdCmd , "\r\n" );
			SockWrite( pProc->pSock , StrBys(sta_BkgdCmd) );
		}
		
		sta_ifBkgdCmdComing = 0;
	}

	return PS_CALLBK_RET_GO_ON;
}

static void 
BackGroundPOP3ProcMake( struct POP3Proc *pa_pPop3Proc , const char *pa_pHostIP , ushort pa_HostPort , BOOL pa_ifEnableSSL , const char *pa_pUsrName , const char *pa_pPassWord )
{
	ProcessCons( &pa_pPop3Proc->proc );
	PROCESS_ADD_STEP( &pa_pPop3Proc->proc , ProtoPOP3Connect , 1000 , 0 );
	PROCESS_ADD_STEP( &pa_pPop3Proc->proc , ProtoPOP3FirstRecv , 1000 , 0 );
	PROCESS_ADD_STEP( &pa_pPop3Proc->proc , ProtoPOP3User , 2000 , 2 );
	PROCESS_ADD_STEP( &pa_pPop3Proc->proc , ProtoPOP3Password , 2000 , 2 );
	PROCESS_ADD_STEP( &pa_pPop3Proc->proc , ProtoPOP3BackGround , 9000 , 0 );

	pa_pPop3Proc->HostIPVal = GetIPVal( pa_pHostIP );
	pa_pPop3Proc->HostPort = pa_HostPort;
	pa_pPop3Proc->ifEnableSSL = pa_ifEnableSSL;
	strcpy( pa_pPop3Proc->UsrName , pa_pUsrName );
	strcpy( pa_pPop3Proc->PassWord , pa_pPassWord );
}

static void 
BkgdNatTypeNotify(struct Process *pa_)
{
	struct STUNProc *pProc = GET_STRUCT_ADDR(pa_ , struct STUNProc , proc);

	switch( pProc->NatTypeRes )
	{
		case NAT_T_FULL_CONE:
			printf("NAT type: full cone. \n");
			break;
		case NAT_T_RESTRICTED:
			printf("NAT type: restricted cone. \n");
			break;
		case NAT_T_PORT_RESTRICTED:
			printf("NAT type: port restricted cone. \n");
			break;
		case NAT_T_SYMMETRIC:
			printf("NAT type: symmetric \n");
			break;
		default:
			printf("NAT type: unknown. \n");
	}
	
	BkgdProcEndCallbk( pa_ );
}

TK_THREAD( BackGround )
{
	DEF_AND_CAST( pProcList , struct ProcessingList , pa_else );
	struct Sock sock;
	BOOL SockOpened = 0;
	struct POP3Proc Pop3Proc;
	struct STUNProc StunProc;
	struct SMTPProc SmtpProc;
	BOOL SmtpSockOpened = 0;

	BackGroundPOP3ProcMake( &Pop3Proc ,"220.181.12.101" ,110,0,"li28jhyxy76223","g131517");
	Pop3Proc.proc.NotifyCallbk = &BkgdProcEndCallbk;
	Pop3Proc.pSock = &sock;

	MakeProtoStunProc(&StunProc ,&sock , "132.177.123.13",STUN_DEFAULT_PORT);
	StunProc.proc.NotifyCallbk = &BkgdNatTypeNotify;

	SMTPProcMake(&SmtpProc ,"113.108.225.10" , 25,0,
			"li28jhyxy76223","g131517","li28jhyxy76223@163.com","Ok");
	SmtpProc.proc.NotifyCallbk = &BkgdProcEndCallbk;

	while(1)
	{
		fgets(sta_BkgdCmd,32,stdin);
		sta_BkgdCmd[ strlen(sta_BkgdCmd) - 1 ] = '\0';
		MutexLock(&g_BkgdMutex);
		
		if( sta_ifBkgdSubProcess )
		{
			if( sta_ifBkgdCmdComing == 0 )
			{
				sta_ifBkgdCmdComing = 1;
			}
			else
			{
				sta_ifBkgdCmdComing = 0;
			}
		}
		else if( strcmp(sta_BkgdCmd ,"help") == 0 )
		{
			printf("valid cmd: pop3,exit,nat,smtp,peers.\n");
		}
		else if( strcmp(sta_BkgdCmd ,"pop3") == 0 )
		{
			if(SockOpened)
			{
				SockClose( &sock );
			}

			SockOpen( &sock , TCP , 0);
			SockOpened = 1;

			ProcessStart( &Pop3Proc.proc , pProcList );
			sta_ifBkgdSubProcess = 1;
		}
		else if( strcmp(sta_BkgdCmd ,"exit") == 0 )
		{
			break;
		}
		else if( strcmp(sta_BkgdCmd ,"nat") == 0 )
		{
			if(SockOpened)
			{
				SockClose( &sock );
			}

			SockOpen( &sock , UDP , 8821);
			SockSetNonblock( &sock );
			SockOpened = 1;

			ProcessStart( &StunProc.proc , pProcList );
			sta_ifBkgdSubProcess = 1;
		}
		else if( strcmp(sta_BkgdCmd ,"smtp") == 0 )
		{
			if(SmtpSockOpened)
			{
				SockClose( &SmtpProc.Sock );
				SockOpen(  &SmtpProc.Sock , TCP , 0);
			}

			ProcessStart( &SmtpProc.proc , pProcList );
			SmtpSockOpened = 1;
			sta_ifBkgdSubProcess = 1;
		}
		else if( strcmp(sta_BkgdCmd ,"peers") == 0 )
		{
			//Not implemented.
		}
		else
		{
			printf("unknown bkgd cmd.\n");
		}
			
		MutexUnlock(&g_BkgdMutex);
	}

	ProcessFree( &Pop3Proc.proc );
	//without using RETR step, there is
	//no mails to free, so we don't need 
	//to call POP3ProcFree() function.

	ProcessFree( &StunProc.proc );
	ProcessFree( &SmtpProc.proc );

	g_MainLoopFlag = 0;
	return NULL;
}
