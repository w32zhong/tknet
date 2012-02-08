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
	struct Sock           MainSock;
	struct BridgeProc     BdgServerProc;
	struct BridgeProc     BdgClientProc;
	char                  BdgPeerAddrStr[32];
	int                   LocalBindingPort;
	char MyName[32] = "Unnamed";

	tkNetInit();

	PeerDataCons(&PeerDataRoot);
	PeerDataRoot.tpnd.RanPriority = 0;
	PeerDataRoot.addr.port = 0;
	PeerDataRoot.addr.IPv4 = 0;

	ProcessingListCons( &ProcList );

	KeyInfoCacheCons(&KeyInfoCache);
	KeyInfoReadFile(&KeyInfoCache,"tknet.info");

	if(pa_argn == 3)
	{
		sscanf(in_args[1],"%d",&LocalBindingPort);
		strcpy(MyName,in_args[2]);
		
		SockOpen(&MainSock,UDP,(ushort)LocalBindingPort);
		SockSetNonblock(&MainSock);
		
		printf("port %d & %s \n",LocalBindingPort,MyName);
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

	BridgeMakeClientProc(&BdgClientProc,&MainSock,&g_BdgPeerAddr,MyName,NAT_T_FULL_CONE);
	ProcessStart(&BdgClientProc.proc,&ProcList);

go_on:

	MutexInit(&g_BkgdMutex);

	BkgdArgs.pPeerDataRoot = &PeerDataRoot;
	BkgdArgs.pInfoCache = &KeyInfoCache;
	BkgdArgs.pProcList = &ProcList;
	tkBeginThread( &BackGround , &BkgdArgs );

	ConsAndStartBridgeServer(&BdgServerProc,&PeerDataRoot,&ProcList,&MainSock);

	while( g_MainLoopFlag )
	{
		MutexLock(&g_BkgdMutex);
		DoProcessing( &ProcList );
		MutexUnlock(&g_BkgdMutex);

		tkMsSleep(100);
	}

	SockClose(&MainSock);

exit:

	FreeBridgeServer(&BdgServerProc);
	PeerDataDestroy(&PeerDataRoot);
	KeyInfoUpdate( &KeyInfoCache );
	KeyInfoWriteFile(&KeyInfoCache,"baba.info");
	KeyInfoFree(&KeyInfoCache);
	MutexDelete(&g_BkgdMutex);
	tkNetUninit();

	return 0;
}
