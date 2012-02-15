#include "tknet.h"
#define BKGD_CMD_MAX_LEN 128

tkMutex g_BkgdMutex;

char    sta_BkgdCmd[BKGD_CMD_MAX_LEN];
BOOL    sta_ifBkgdCmdComing;
BOOL    sta_ifBkgdSubProcess = 0;
extern  BOOL g_MainLoopFlag;

char*
BkgdGetBackGroundMsg()
{
	if(sta_ifBkgdCmdComing)
	{
		sta_ifBkgdCmdComing = 0;
		return sta_BkgdCmd;
	}
	else
	{
		return NULL;
	}
}

void
BkgdEnterSubProcess()
{
	sta_ifBkgdSubProcess = 1;
}

void
BkgdLeaveSubProcess()
{
	sta_ifBkgdSubProcess = 0;
}

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

static void
SetPop3ProcByKeyInfo(struct POP3Proc *pa_pPop3Proc ,struct KeyInfo *pa_pKeyInfo)
{
	char *pStr,buff[KEY_INFO_MAX_LEN];
	char *pBuff = buff;
	uint i;
	strcpy(buff,pa_pKeyInfo->text);

	pStr = GetNextSeparateStr(&pBuff);
	pStr = GetNextSeparateStr(&pBuff);
	pStr = GetNextSeparateStr(&pBuff);
	//skip info type, IP , port.

	pa_pPop3Proc->HostIPVal = htonl(pa_pKeyInfo->addr.IPv4);
	pa_pPop3Proc->HostPort = pa_pKeyInfo->addr.port;

	pStr = GetNextSeparateStr(&pBuff);
	sscanf(pStr,"%d",&i);
	pa_pPop3Proc->ifEnableSSL = (BOOL)i;

	pStr = GetNextSeparateStr(&pBuff);
	strcpy(pa_pPop3Proc->UsrName,pStr);

	pStr = GetNextSeparateStr(&pBuff);
	strcpy(pa_pPop3Proc->PassWord,pStr);
}

static void
SetSmtpProcByKeyInfo(struct SMTPProc *pa_pSmtpProc ,struct KeyInfo *pa_pKeyInfo)
{
	char *pStr,buff[KEY_INFO_MAX_LEN];
	char *pBuff = buff;
	uint i;
	strcpy(buff,pa_pKeyInfo->text);

	pStr = GetNextSeparateStr(&pBuff);
	pStr = GetNextSeparateStr(&pBuff);
	pStr = GetNextSeparateStr(&pBuff);
	//skip info type, IP , port.

	pa_pSmtpProc->HostIPVal = htonl(pa_pKeyInfo->addr.IPv4);
	pa_pSmtpProc->HostPort = pa_pKeyInfo->addr.port;

	pStr = GetNextSeparateStr(&pBuff);
	sscanf(pStr,"%d",&i);
	pa_pSmtpProc->ifEnableSSL = (BOOL)i;

	pStr = GetNextSeparateStr(&pBuff);
	strcpy(pa_pSmtpProc->UsrName,pStr);

	pStr = GetNextSeparateStr(&pBuff);
	strcpy(pa_pSmtpProc->PassWord,pStr);

	pStr = GetNextSeparateStr(&pBuff);
	strcpy(pa_pSmtpProc->MailAddr,pStr);
}

static void
SetStunProcByKeyInfo(struct STUNProc *pa_pStunProc ,struct KeyInfo *pa_pKeyInfo)
{
	pa_pStunProc->HostIPVal = htonl(pa_pKeyInfo->addr.IPv4);
	pa_pStunProc->HostPort = pa_pKeyInfo->addr.port;
}

static struct KeyInfo*
FindKeyInfoByStrArg(struct KeyInfoCache *pa_pInfoChache,char *pa_pStrArg)
{
	struct FindKeyInfoByNumPa fkipa;
	int i;
	sscanf(pa_pStrArg,"%d",&i);

	fkipa.found = NULL;
	fkipa.NumToFind = i;

	ForEach( &pa_pInfoChache->IKeyInfo , &FindKeyInfoByNum , &fkipa );

	return fkipa.found;
}

TK_THREAD( BackGround )
{
	DEF_AND_CAST( pBkgdArgs , struct BackGroundArgs , pa_else );
	struct Sock sock;
	BOOL SockOpened = 0;
	struct POP3Proc Pop3Proc;
	struct STUNProc StunProc;
	struct SMTPProc SmtpProc;
	BOOL SmtpSockOpened = 0;
	char *pCmd,*pArg0,*pArg1;
	struct KeyInfo *pKeyInfo;
	struct BridgeClientProcPa *pBCPPa;
	struct PeerData           *pFoundPD;

	BackGroundPOP3ProcMake( &Pop3Proc ,"IP" ,0,0,"username","password");
	Pop3Proc.proc.NotifyCallbk = &BkgdProcEndCallbk;
	Pop3Proc.pSock = &sock;

	MakeProtoStunProc(&StunProc ,&sock , "IP",STUN_DEFAULT_PORT);
	StunProc.proc.NotifyCallbk = &BkgdNatTypeNotify;

	SMTPProcMake(&SmtpProc ,"IP" , 0,0,"username","password","adress","content");
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
		else
		{
			pArg1 = sta_BkgdCmd;
			pCmd = GetNextSeparateStr(&pArg1);
			pArg0 = GetNextSeparateStr(&pArg1);

			if( strcmp(pCmd ,"help") == 0 )
			{
				printf("valid cmd:\n"
						"  pop3 [pop3 key]\n"
						"  exit\n"
						"  nat [stun key]\n"
						"  smtp [smtp key] [content key]\n"
						"  connect [peer name]\n"
						"  direct [peer name]\n"
						"  direct \n"
						"  key\n"
						"  cproc\n"
						"  relays\n"
						"  peers\n");
			}
			else if( strcmp(pCmd ,"relays") == 0 )
			{
				RelayProcTrace();
			}
			else if( strcmp(pCmd ,"key") == 0 )
			{
				KeyInfoTrace(pBkgdArgs->pInfoCache);
			}
			else if( strcmp(pCmd ,"cproc") == 0 )
			{
				ProcessTraceSteps(&(pBkgdArgs->pBdgClientProc->proc));
			}
			else if( strcmp(pCmd ,"connect") == 0 )
			{
				strcpy(g_TargetName,pArg0);
				pBCPPa = (struct BridgeClientProcPa*)(pBkgdArgs->pBdgClientProc->Else);
				pBCPPa->pTargetNameID = g_TargetName;
			}
			else if( strcmp(pCmd ,"direct") == 0 )
			{
				pBCPPa = (struct BridgeClientProcPa*)(pBkgdArgs->pBdgClientProc->Else);
				
				if(*pArg0 == '\0')
				{
					printf("direct connect to BDG server...\n");
					pBCPPa->DirectConnectAddr = g_BdgPeerAddr;
				}
				else
				{
					pFoundPD = PeerDataFind(pBkgdArgs->pPeerDataRoot,pArg0);
					
					if(pFoundPD)
					{
						printf("direct connect to peer from BDG server.\n");
						pBCPPa->DirectConnectAddr = pFoundPD->addr;
					}
					else
					{
						printf("unable to find the name in peer data.\n");
					}
				}
			}
			else if( strcmp(pCmd ,"pop3") == 0 )
			{
				if(SockOpened)
				{
					SockClose( &sock );
				}

				SockOpen( &sock , TCP , 0);
				SockOpened = 1;

				pKeyInfo = FindKeyInfoByStrArg(pBkgdArgs->pInfoCache,pArg0);

				if(pKeyInfo)
				{
					SetPop3ProcByKeyInfo(&Pop3Proc,pKeyInfo);
					ProcessStart( &Pop3Proc.proc , pBkgdArgs->pProcList );
					sta_ifBkgdSubProcess = 1;
				}
				else
				{
					printf("unable to find the key.\n");
				}
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

				pKeyInfo = FindKeyInfoByStrArg(pBkgdArgs->pInfoCache,pArg0);

				if(pKeyInfo)
				{
					SetStunProcByKeyInfo(&StunProc,pKeyInfo);
					ProcessStart( &StunProc.proc , pBkgdArgs->pProcList );
					sta_ifBkgdSubProcess = 1;
				}
				else
				{
					printf("unable to find the key.\n");
				}
			}
			else if( strcmp(sta_BkgdCmd ,"smtp") == 0 )
			{
				if(SmtpSockOpened)
				{
					SockClose( &SmtpProc.Sock );
					SockOpen(  &SmtpProc.Sock , TCP , 0);
				}

				pKeyInfo = FindKeyInfoByStrArg(pBkgdArgs->pInfoCache,pArg0);

				if(pKeyInfo)
				{
					SetSmtpProcByKeyInfo(&SmtpProc,pKeyInfo);

					pKeyInfo = FindKeyInfoByStrArg(pBkgdArgs->pInfoCache,pArg1);

					if(pKeyInfo)
					{
						strcpy(SmtpProc.SendBuff,pKeyInfo->text);
						ProcessStart( &SmtpProc.proc , pBkgdArgs->pProcList );
						SmtpSockOpened = 1;
						sta_ifBkgdSubProcess = 1;
					}
					else
					{
						printf("unable to find the key to the content.\n");
					}
				}
				else
				{
					printf("unable to find the key.\n");
				}
			}
			else if( strcmp(sta_BkgdCmd ,"peers") == 0 )
			{
				PeerDataTrace(pBkgdArgs->pPeerDataRoot);
			}
			else
			{
				printf("unknown bkgd cmd.\n");
			}
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
