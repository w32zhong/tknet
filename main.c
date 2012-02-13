#include "headers.h"

BOOL g_MainLoopFlag = 1;
uchar g_NATtype = NAT_T_UNKNOWN;
struct NetAddr g_BdgPeerAddr;
char g_TargetName[PEER_NAME_ID_LEN];
char g_MyName[PEER_NAME_ID_LEN];

void tkNetInit()
{
	tkInitRandom();
	tkLogInit();
	SockInit();

	g_TargetName[0]='\0';
	g_MyName[0]='\0';
}

void tkNetUninit()
{
	SockDestory();
	tkLogClose();
	printf("unfree memory:%d \n",g_allocs);
}

int main(int pa_argn,char **in_args)
{
	struct KeyInfoCache   KeyInfoCache;
	struct ProcessingList ProcList;
	struct BackGroundArgs BkgdArgs;
	struct PeerData       PeerDataRoot;
	struct Iterator       ISeedPeer;
	struct Sock           MainSock;
	struct BridgeProc     BdgServerProc;
	struct BridgeProc     BdgClientProc;
	BOOL                  ifBdgClientProcMade = 0;
	char                  BdgPeerAddrStr[32];
	char                  *pTargetName = NULL;

	tkNetInit();
	MutexInit(&g_BkgdMutex);

	ISeedPeer = GetIterator(NULL);

	PeerDataCons(&PeerDataRoot);
	PeerDataRoot.tpnd.RanPriority = 0;
	PeerDataRoot.addr.port = 0;
	PeerDataRoot.addr.IPv4 = 0;

	ProcessingListCons( &ProcList );

	KeyInfoCacheCons(&KeyInfoCache);
	KeyInfoReadFile(&KeyInfoCache,"tknet.info");

	if(!KeyInfoTry(&KeyInfoCache,KEY_INFO_TYPE_CONFIG,&MainSock))
	{
		printf("please config port & name\n");
		goto exit;
	}

	if(pa_argn == 2)
	{
		strcpy(g_TargetName,in_args[1]);
	}
	
	if( g_TargetName[0] != '\0' )
	{
		printf("Target Name: %s \n",g_TargetName);
		pTargetName = g_TargetName; 
	}
	else
	{
		printf("Target Name unset. \n");
	}

//	while(!KeyInfoTry(&KeyInfoCache,KEY_INFO_TYPE_STUNSERVER,&MainSock))
//	{
//		if(!KeyInfoTry(&KeyInfoCache,KEY_INFO_TYPE_MAILSERVER,&MainSock))
//		{
//			printf("No way to get NAT type :( \n");
//			goto exit;
//		}
//	}

	g_NATtype = NAT_T_FULL_CONE;
	printf("NAT type: %d\n",g_NATtype);


	while(!KeyInfoTry(&KeyInfoCache,KEY_INFO_TYPE_BRIDGEPEER,&MainSock))
	{
		if(!KeyInfoTry(&KeyInfoCache,KEY_INFO_TYPE_MAILSERVER,&MainSock))
		{
			printf("no avalible Bridge peer :( \n");
			goto only_server;
		}
	}

	GetAddrText(&g_BdgPeerAddr,BdgPeerAddrStr);
	printf("using Bridge peer: %s\n",BdgPeerAddrStr);

	BridgeMakeClientProc(&BdgClientProc,&MainSock,&g_BdgPeerAddr,g_MyName,g_NATtype,pTargetName);
	//TaName can be NULL
	ProcessStart(&BdgClientProc.proc,&ProcList);
	ifBdgClientProcMade = 1;

only_server:

	BkgdArgs.pPeerDataRoot = &PeerDataRoot;
	BkgdArgs.pInfoCache = &KeyInfoCache;
	BkgdArgs.pProcList = &ProcList;
	tkBeginThread( &BackGround , &BkgdArgs );

	ConsAndStartBridgeServer(&BdgServerProc,&PeerDataRoot,&ProcList,&MainSock,&ISeedPeer);

	while( g_MainLoopFlag )
	{
		MutexLock(&g_BkgdMutex);
		SockRead(&MainSock);
		DoProcessing( &ProcList );
		MainSock.RecvLen = 0;
		MutexUnlock(&g_BkgdMutex);

		tkMsSleep(100);
	}

	SockClose(&MainSock);

	if(ifBdgClientProcMade)
		FreeBdgClientProc(&BdgClientProc);
	FreeBridgeServer(&BdgServerProc);

exit:

	PeerDataDestroy(&PeerDataRoot);
	KeyInfoUpdate( &KeyInfoCache );
	KeyInfoWriteFile(&KeyInfoCache,"tknet.updateinfo");
	KeyInfoFree(&KeyInfoCache);
	MutexDelete(&g_BkgdMutex);
	tkNetUninit();

	return 0;
}
