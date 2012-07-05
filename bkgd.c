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

tkMutex        g_BkgdMutex;
BOOL           g_ifStdinToCmd = 0;
static char    sta_BkgdCmd[BKGD_CMD_MAX_LEN];
static BOOL    sta_ifBkgdCmdComing = 0;
static BOOL    sta_ifBkgdSubProcess = 0;
static BOOL    sta_ifBkgdStunProc = 0;
extern BOOL    g_MainLoopFlag;
static BOOL    sta_ifBkgdFlow = 0;
uchar          g_BkgdNatTestRes = NAT_T_UNKNOWN;
struct NetAddr g_BkgdNatTestAddrRes;

BOOL 
ifBkgdStunProc()
{
	return sta_ifBkgdStunProc;
}

BOOL
ifBkgdSubProcess()
{
	return sta_ifBkgdSubProcess;
}

static void 
BkgdProcEndCallbk(struct Process *pa_)
{
	PROMPT(Usual,"bkgd process end.\n");
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
			PROMPT(Usual,"%s\n",pProc->pSock->RecvBuff);
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
			PROMPT(Usual,"unknown POP3 cmd. Available:\n"
					"quit,dele,list,retr. \n");
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
	PROCESS_ADD_STEP( &pa_pPop3Proc->proc , ProtoPOP3Connect , g_WaitLevel[1] );
	PROCESS_ADD_STEP( &pa_pPop3Proc->proc , ProtoPOP3FirstRecv , g_WaitLevel[1] );
	PROCESS_ADD_STEP( &pa_pPop3Proc->proc , ProtoPOP3User , g_WaitLevel[2] );
	PROCESS_ADD_STEP( &pa_pPop3Proc->proc , ProtoPOP3Password , g_WaitLevel[2] );
	PROCESS_ADD_STEP( &pa_pPop3Proc->proc , ProtoPOP3BackGround , g_WaitLevel[4] );

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

	NatTypePrint(pProc->NatTypeRes);
	
	g_BkgdNatTestRes = pProc->NatTypeRes;
	g_BkgdNatTestAddrRes = pProc->MapAddr;
	
	sta_ifBkgdStunProc =0;
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

static
FLOW_CALLBK_FUNCTION( CmdFlowCallbk )
{
	VCK(pa_DataLen >= BKGD_CMD_MAX_LEN,return);

	memcpy(sta_BkgdCmd,pa_pData,pa_DataLen);
	sta_BkgdCmd[ BKGD_CMD_MAX_LEN - 1 ] = '\0';//make it safe
	
	if(sta_BkgdCmd[strlen(sta_BkgdCmd)-1] == '\n')
		sta_BkgdCmd[strlen(sta_BkgdCmd)-1] = '\0'; //strip the '\n'

	sta_ifBkgdFlow = 1;
}

TK_THREAD( BackGround )
{
	DEF_AND_CAST( pBkgdArgs , struct BackGroundArgs , pa_else );
	DEF_AND_CAST(pBCPPa,struct BridgeClientProcPa,pBkgdArgs->pBdgClientProc->Else);

	struct Sock                sock;
	BOOL                       SockOpened = 0;
	struct POP3Proc            Pop3Proc;
	struct STUNProc            StunProc;
	struct SMTPProc            SmtpProc;
	BOOL                       SmtpSockOpened = 0;
	char                       *pCmd,*pArg0,*pArg1;
	struct KeyInfo             *pKeyInfo;
	struct PeerData            *pFoundPD;
	struct pipe                *pPipe0,*pPipe1;

	pPipe0 = PipeMap("cmd");
	pPipe0->FlowCallbk = &CmdFlowCallbk;

	if(g_ifStdinToCmd)
	{
		pPipe1 = PipeFindByName("stdin");
		if(pPipe1)
			PipeDirectTo(pPipe1,pPipe0);
		else
			PROMPT(Usual,"stdin not found by BackGround.\n");
	}

	BackGroundPOP3ProcMake( &Pop3Proc ,"IP" ,0,0,"username","password");
	Pop3Proc.proc.NotifyCallbk = &BkgdProcEndCallbk;
	Pop3Proc.pSock = &sock;

	MakeProtoStunProc(&StunProc ,pBkgdArgs->pMainSock , "IP",STUN_DEFAULT_PORT);
	StunProc.proc.NotifyCallbk = &BkgdNatTypeNotify;

	SMTPProcMake(&SmtpProc ,"IP" , 0,0,"username","password","adress","content");
	SmtpProc.proc.NotifyCallbk = &BkgdProcEndCallbk;

	while(1)
	{
		while(!sta_ifBkgdFlow)
		{
			tkMsSleep(SHORT_SLEEP_INTERVAL);
		}

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
				PROMPT(Usual,"valid cmd:\n"
						"  pop3 [pop3 key] (enter mail through POP3)\n"
						"  exit (exit this program)\n"
						"  nat [stun key] (get public NAT type through STUN)\n"
						"  smtp [smtp key] [content key] (send tknet info by SMTP)\n"
						"  connect [peer name] (connect a peer)\n"
						"  direct [peer name] (directly connect a peer)\n"
						"  direct (directly connect the BDG server)\n"
						"  key (print tknet info keys)\n"
						"  readkey (read in new keys from the tknet.info file)\n"
						"  cproc (print client processes)\n"
						"  ckproc (print check NAT process)\n"
						"  relays (print current relay processes)\n"
						"  pltrace (Process List trace)\n"
						"  pipet (print pipe table)\n"
						"  pipe     [a] [b] (pipe from a to b)\n"
						"  pipeonly [a] [b] (pipe from a only to b)\n"
						"  setc (set tknet condition, from 0 to %d)\n"
						"  peers (print peers connected to BDG server)\n"
						"  ikey [key] [1/0] (validated/invalidated a key manually)\n",
						TKNET_CONDITIONS-1);
			}
			else if( strcmp(pCmd ,"ikey") == 0 )
			{
				pKeyInfo = FindKeyInfoByStrArg(pBkgdArgs->pInfoCache,pArg0);
				
				if(pKeyInfo == NULL)
				{
					PROMPT(Usual,"Unable to find the key.\n");
				}
				else if( 0 == atoi(pArg1) )
				{
					PROMPT(Usual,"invalidated the key.\n");
					
					pKeyInfo->valid = KEY_INFO_VALID_NOT;
					KeyInfoTrace(pBkgdArgs->pInfoCache);
				}
				else
				{
					PROMPT(Usual,"validated the key.\n");
					
					pKeyInfo->valid = KEY_INFO_VALID_UNSURE;
					KeyInfoTrace(pBkgdArgs->pInfoCache);
				}
			}
			else if( strcmp(pCmd ,"pipet") == 0 )
			{
				PipeTablePrint();
			}
			else if( strcmp(pCmd ,"pipe") == 0 )
			{
				pPipe0 = PipeFindByID(atoi(pArg0));
				pPipe1 = PipeFindByID(atoi(pArg1));

				if(pPipe0 == NULL || pPipe1 == NULL)
					PROMPT(Usual,"can't find pipe.\n");
				else
				{
					PipeDirectTo(pPipe0,pPipe1);
					PipeTablePrint();
				}
			}
			else if( strcmp(pCmd ,"pipeonly") == 0 )
			{
				pPipe0 = PipeFindByID(atoi(pArg0));
				pPipe1 = PipeFindByID(atoi(pArg1));

				if(pPipe0 == NULL || pPipe1 == NULL)
					PROMPT(Usual,"can't find pipe.\n");
				else
				{
					PipeDirectOnlyTo(pPipe0,pPipe1);
					PipeTablePrint();
				}
			}
			else if( strcmp(pCmd ,"relays") == 0 )
			{
				RelayProcTrace();
			}
			else if( strcmp(pCmd ,"key") == 0 )
			{
				KeyInfoTrace(pBkgdArgs->pInfoCache);
			}
			else if( strcmp(pCmd ,"setc") == 0 )
			{
				ProcessSetCondition(atoi(pArg0));
			}
			else if( strcmp(pCmd ,"cproc") == 0 )
			{
				PROMPT(Usual,"status: ");
				if(pBCPPa->ifConnected)
				{
					PROMPT(Usual,"Connected.\n");
				}
				else
				{
					PROMPT(Usual,"Disconnected.\n");
				}

				ProcessTraceSteps(&(pBkgdArgs->pBdgClientProc->proc));
			}
			else if( strcmp(pCmd ,"ckproc") == 0 )
			{
				if(pBkgdArgs->pCheckNATProc)
					ProcessTraceSteps(&pBkgdArgs->pCheckNATProc->proc);
				else
					PROMPT(Usual,"Check NAT process is not started.\n");
			}
			else if( strcmp(pCmd ,"connect") == 0 )
			{
				if(pBCPPa->ifConnected)
				{
					tkNetConnect(pArg0);
				}
				else
				{
					PROMPT(Usual,"Failed: Client is disconnected to BDG server now.\n");
				}
			}
			else if( strcmp(pCmd ,"direct") == 0 )
			{
				if(*pArg0 == '\0')
				{
					if(pBCPPa->ifConnected)
					{
						PROMPT(Usual,"direct connect to BDG server...\n");
						pBCPPa->DirectConnectAddr = g_BdgPeerAddr;
					}
					else
					{
						PROMPT(Usual,"Failed: Client is disconnected to BDG server now.\n");
					}
				}
				else
				{
					pFoundPD = PeerDataFind(pBkgdArgs->pPeerDataRoot,pArg0);
					
					if(pFoundPD)
					{
						PROMPT(Usual,"direct connect to peer from BDG server.\n");
						pBCPPa->DirectConnectAddr = pFoundPD->addr;
					}
					else
					{
						PROMPT(Usual,"unable to find the name in peer data.\n");
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
					PROMPT(Usual,"unable to find the key.\n");
				}
			}
			else if( strcmp(sta_BkgdCmd ,"exit") == 0 )
			{
				break;
			}
			else if( strcmp(sta_BkgdCmd ,"pltrace") == 0 )
			{
				ProcessingListTrace(pBkgdArgs->pProcList);
			}
			else if( strcmp(sta_BkgdCmd ,"nat") == 0 )
			{
				pKeyInfo = FindKeyInfoByStrArg(pBkgdArgs->pInfoCache,pArg0);

				if(pKeyInfo)
				{
					StunProc.NatTypeRes = NAT_T_UNKNOWN;
					//reset the result flag.

					SetStunProcByKeyInfo(&StunProc,pKeyInfo);
					ProcessStart( &StunProc.proc , pBkgdArgs->pProcList );

					sta_ifBkgdSubProcess = 1;
					sta_ifBkgdStunProc =1;
				}
				else
				{
					PROMPT(Usual,"unable to find the key.\n");
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
						PROMPT(Usual,"unable to find the key to the content.\n");
					}
				}
				else
				{
					PROMPT(Usual,"unable to find the key.\n");
				}
			}
			else if( strcmp(sta_BkgdCmd ,"peers") == 0 )
			{
				PeerDataTrace(pBkgdArgs->pPeerDataRoot);
			}
			else if( strcmp(sta_BkgdCmd ,"readkey") == 0 )
			{
				if(!KeyInfoReadFile(pBkgdArgs->pInfoCache,"tknet.info"))
				{
					PROMPT(Usual,"config file lost.\n");
				}
				else
				{
					KeyInfoTrace(pBkgdArgs->pInfoCache);
				}
			}
			else
			{
				PROMPT(Usual,"bad.\n");
			}
		}

		sta_ifBkgdFlow = 0;
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
