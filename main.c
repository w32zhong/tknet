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

BOOL             g_MainLoopFlag = 1;
BOOL             g_ifConfigAsFullCone = 0;
struct NetAddr   g_BdgPeerAddr;
char             g_TargetName[PEER_NAME_ID_LEN];
const char      *g_pTargetName = NULL;
char             g_MyName[PEER_NAME_ID_LEN];

#define STDOUT_BUFFLEN (PROMT_BUFFERSIZE*3)
static char      sta_StdoutBuff[STDOUT_BUFFLEN];
static uint      sta_StdoutBuffIndex = 0;//point to the first unfilled buffer.
struct pipe      *sta_pStdoutPipe = NULL;

static
FLOW_CALLBK_FUNCTION( StdoutFlowCallbk )
{
	int i;

	for(i=0;i< pa_DataLen ;i++)
	{
		VCK(pa_pData[i] == '\0',continue);
		putchar(pa_pData[i]);
	}

	fflush(stdout);
	
	if(sta_StdoutBuffIndex + pa_DataLen > STDOUT_BUFFLEN)
	{
		memcpy(sta_StdoutBuff + sta_StdoutBuffIndex ,
				pa_pData, STDOUT_BUFFLEN - sta_StdoutBuffIndex);
		sta_StdoutBuffIndex = STDOUT_BUFFLEN;

		StdoutPipeFlush();
	}
	else
	{
		memcpy(sta_StdoutBuff + sta_StdoutBuffIndex ,
				pa_pData, pa_DataLen);
		sta_StdoutBuffIndex += pa_DataLen;
	}
}

void
StdoutPipeFlush()
{
	if(NULL == sta_pStdoutPipe)
		sta_pStdoutPipe = PipeMap("stdout");

	if(sta_StdoutBuffIndex == 0)
		return;
	else
		PipeFlow(sta_pStdoutPipe,sta_StdoutBuff,sta_StdoutBuffIndex,NULL);
	
	sta_StdoutBuffIndex = 0;
}

TK_THREAD( StdinThread )
{
 	static char buff[BKGD_CMD_MAX_LEN];
	DEF_AND_CAST(pPipe,struct pipe,pa_else);
	
	while(g_MainLoopFlag)
	{
		fgets(buff,BKGD_CMD_MAX_LEN,stdin);

		if(strcmp(buff,"pipet_debug\n") == 0)
			PipeTablePrint();

		MutexLock(&g_BkgdMutex);
		PipeFlow(pPipe,buff,strlen(buff),NULL);
		//Do not flow '\0' in string pipe.
		MutexUnlock(&g_BkgdMutex);

		tkMsSleep(SHORT_SLEEP_INTERVAL);
	}

	return NULL;
}

static
FLOW_CALLBK_FUNCTION( LogFlowCallbk )
{
	tkLogLenDat(1,(const char*)pa_pData, pa_DataLen);
	PipeFlow(pa_pPipe,pa_pData,pa_DataLen,NULL);
}

void
tkNetDefaultPipeInit()
{
	struct pipe *pPipe;
	
	pPipe = PipeMap("null");
	
	pPipe = PipeMap("log");
	pPipe->FlowCallbk = &LogFlowCallbk;

	pPipe = PipeMap("stdout");
	pPipe->FlowCallbk = &StdoutFlowCallbk;
	
	pPipe = PipeMap("stdin");
	tkBeginThread( &StdinThread , pPipe );
}

void
tkNetCommonInit()
{
	MutexInit(&g_BkgdMutex);
	tkInitRandom();
	tkLogInit();
	SockInit();
	
	PipeModuleInit();
	tkNetDefaultPipeInit();
}

void 
tkNetCommonUninit()
{
	g_MainLoopFlag = 0;
	PipeModuleUninit();
	SockDestory();
	tkLogClose();
	tkMsSleep(SHORT_SLEEP_INTERVAL);//waiting for threads
	MutexDelete(&g_BkgdMutex);
}

void 
tkNetConnect(const char *pa_pName)
{
	if( pa_pName != NULL )
		strcpy( g_TargetName , pa_pName );
	
	g_pTargetName = pa_pName;
}

static void
InitPipeStdoutDirection()
{
	struct pipe *pPipe;
	g_pUsualPrompt = PipeMap((char*)g_UsualPromptName);
	pPipe = PipeFindByName("stdout");

	if(pPipe)
		PipeDirectTo(g_pUsualPrompt,pPipe);
	else
		TK_EXCEPTION("stdout");
}

int 
tkNetMain(int pa_argn,char **in_args)
{
	struct KeyInfoCache        KeyInfoCache;
	struct ProcessingList      ProcList;
	struct BackGroundArgs      BkgdArgs;
	struct PeerData            PeerDataRoot;
	struct Iterator            ISeedPeer;
	struct Sock                MainSock;
	struct BridgeProc          BdgServerProc;
	struct BridgeProc          BdgClientProc;
	char                       BdgPeerAddrStr[32];
	BOOL                       ifClientSkipRegister = 1;
	int                        TestPurposeNatType;
	struct BridgeClientProcPa  *pBCPPa = NULL;

	g_TargetName[0]='\0';
	g_MyName[0]='\0';

	tkNetCommonInit();
	InitPipeStdoutDirection();

	PROMPT(Usual,"tknet \n build: " TKNET_VER "\n");

	ProcessSetCondition(1);

	MkCmdModePipe();
	MkChatModePipe();

	ISeedPeer = GetIterator(NULL);

	PeerDataCons(&PeerDataRoot);
	PeerDataRoot.tpnd.RanPriority = 0;
	PeerDataRoot.addr.port = 0;
	PeerDataRoot.addr.IPv4 = 0;

	ProcessingListCons( &ProcList );

	RelayModuleInit();

	KeyInfoCacheCons(&KeyInfoCache);
	if(!KeyInfoReadFile(&KeyInfoCache,"tknet.info"))
	{
		PROMPT(Usual,"config file lost.\n");
		goto exit;
	}

	if(!KeyInfoTry(&KeyInfoCache,KEY_INFO_TYPE_CONFIG,&MainSock))
	{
		PROMPT(Usual,"bad config format.\n");
		goto exit;
	}
	
	if( g_TargetName[0] != '\0' )
	{
		PROMPT(Usual,"target name: %s \n", g_TargetName);
		tkNetConnect(g_TargetName);
	}
	else
	{
		PROMPT(Usual,"target name unset. \n");
	}

	if(g_ifConfigAsFullCone)
	{
		g_NATtype = NAT_T_FULL_CONE;
		PROMPT(Usual,"config NAT type as fullcone.\n");
	}
	else
	{
		while(!KeyInfoTry(&KeyInfoCache,KEY_INFO_TYPE_STUNSERVER,&MainSock))
		{
			if(!KeyInfoTry(&KeyInfoCache,KEY_INFO_TYPE_MAILSERVER,&MainSock))
			{
				PROMPT(Usual,"No way to get NAT type.\n");
				goto exit;
			}
		}
		
		PROMPT(Usual,"NAT type got from STUN.\n");
	}

	if(pa_argn == 2)
	{
		sscanf(in_args[1],"%d",&TestPurposeNatType);
		g_NATtype = (uchar)TestPurposeNatType;
		
		PROMPT(Usual,"NAT type assigned by argument.\n");
	}
		
	NatTypePrint(g_NATtype);

	while(!KeyInfoTry(&KeyInfoCache,KEY_INFO_TYPE_BRIDGEPEER,&MainSock))
	{
		if(!KeyInfoTry(&KeyInfoCache,KEY_INFO_TYPE_MAILSERVER,&MainSock))
		{
			PROMPT(Usual,"no avalible Bridge peer.\n");
			tkNetConnect(NULL);

			if(g_NATtype == NAT_T_FULL_CONE)
			{
				BkgdArgs.pCheckNATProc = 
					CheckNATProcConsAndBegin(&ProcList, &KeyInfoCache);
				PROMPT(Usual,"checking NAT enabled.\n");
			}

			goto no_bdg_peer;
		}
	}

	BkgdArgs.pCheckNATProc = NULL;
	GetAddrText(&g_BdgPeerAddr,BdgPeerAddrStr);
	PROMPT(Usual,"using Bridge peer: %s\n",BdgPeerAddrStr);
	ifClientSkipRegister = 0;

no_bdg_peer:

	pBCPPa = BridgeMakeClientProc(&BdgClientProc,&MainSock,&ProcList,&g_BdgPeerAddr,
			g_MyName,g_NATtype,&g_pTargetName,ifClientSkipRegister);
	ProcessStart(&BdgClientProc.proc,&ProcList);

	if(g_ifStdinToCmd)
		PROMPT(Usual,"back ground enabled.\n");
	else
		PROMPT(Usual,"back ground disabled.\n");

	BkgdArgs.pPeerDataRoot = &PeerDataRoot;
	BkgdArgs.pInfoCache = &KeyInfoCache;
	BkgdArgs.pProcList = &ProcList;
	BkgdArgs.pBdgClientProc = &BdgClientProc;
	BkgdArgs.pMainSock = &MainSock;
	tkBeginThread( &BackGround , &BkgdArgs );

	ConsAndStartBridgeServer(&BdgServerProc,&PeerDataRoot,&ProcList,&MainSock,&ISeedPeer);

	while( g_MainLoopFlag )
	{
		MutexLock(&g_BkgdMutex);

		if(!ifBkgdStunProc())
			SockRead(&MainSock);

		DoProcessing( &ProcList );

		if(!ifBkgdStunProc())
			MainSock.RecvLen = 0;

		StdoutPipeFlush();

		MutexUnlock(&g_BkgdMutex);
		tkMsSleep(SHORT_SLEEP_INTERVAL);
	}

	ProcessDisattach(&BdgClientProc.proc,&ProcList);
	FreeBdgClientProc(&BdgClientProc);

	ProcessDisattach(&BdgServerProc.proc,&ProcList);
	FreeBridgeServer(&BdgServerProc);

	ProcessListFree(&ProcList);
	FreeSubBridgeServerTemplate();

	SockClose(&MainSock);

exit:

	PeerDataDestroy(&PeerDataRoot,&ISeedPeer);
	KeyInfoUpdate( &KeyInfoCache );
	KeyInfoWriteFile(&KeyInfoCache,"tknet.info");
	KeyInfoFree(&KeyInfoCache);
	RelayMuduleDestruction();
	tkNetCommonUninit();

	printf("unfree memory:%d \n",g_allocs);
	return 0;
}
