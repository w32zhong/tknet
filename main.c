#include "headers.h"

void tkNetInit()
{
	tkInitRandom();
	tkLogInit();
	SockInit();
}

void tkNetUninit()
{
	SockDestory();
	tkLogClose();
	printf("unfree memory:%d \n",g_allocs);
}

BOOL g_MainLoopFlag = 1;
uchar g_NATtype = NAT_T_UNKNOWN;
struct NetAddr g_BdgPeerAddr;


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
	int                   LocalBindingPort;
	char MyName[32] = "no one";
	char TaName[32] = "no one";
	char *pTaName;

	printf("test version \n");
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

	if(pa_argn >= 3)
	{
		sscanf(in_args[1],"%d",&LocalBindingPort);
		strcpy(MyName,in_args[2]);
		if(pa_argn == 4)
		{
			pTaName = TaName;
			strcpy(TaName,in_args[3]);
		}
		else
		{
			pTaName = NULL;
		}
		
		SockOpen(&MainSock,UDP,(ushort)LocalBindingPort);
		SockSetNonblock(&MainSock);
		
		printf("port %d  %s->%s \n",LocalBindingPort,MyName,TaName);
	}
	else
	{
		printf("please specify port & name\n");
		goto exit;
	}
/*
	while(!KeyInfoTry(&KeyInfoCache,KEY_INFO_TYPE_STUNSERVER,&MainSock))
	{
		if(!KeyInfoTry(&KeyInfoCache,KEY_INFO_TYPE_MAILSERVER,&MainSock))
		{
			printf("No way to get NAT type :( \n");
			goto exit;
		}
	}

	printf("get NAT type: %d\n",g_NATtype);
*/

	while(!KeyInfoTry(&KeyInfoCache,KEY_INFO_TYPE_BRIDGEPEER,&MainSock))
	{
		if(!KeyInfoTry(&KeyInfoCache,KEY_INFO_TYPE_MAILSERVER,&MainSock))
		{
			printf("no avalible Bridge peer :( \n");
			goto go_on;
		}
	}

	GetAddrText(&g_BdgPeerAddr,BdgPeerAddrStr);
	printf("using Bridge peer: %s\n",BdgPeerAddrStr);

	BridgeMakeClientProc(&BdgClientProc,&MainSock,&g_BdgPeerAddr,MyName,NAT_T_FULL_CONE,pTaName);//TaName can be NULL
	ProcessStart(&BdgClientProc.proc,&ProcList);
	ifBdgClientProcMade = 1;

go_on:

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
	KeyInfoWriteFile(&KeyInfoCache,"baba.info");
	KeyInfoFree(&KeyInfoCache);
	MutexDelete(&g_BkgdMutex);
	tkNetUninit();

	return 0;
}
