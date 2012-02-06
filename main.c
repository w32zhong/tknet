#include "headers.h"

BOOL g_MainLoopFlag = 1;
uchar g_NATtype = NAT_T_UNKNOWN;

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

int main()
{
	struct KeyInfoCache KeyInfoCache;
	struct ProcessingList ProcList;
	struct BackGroundArgs BkgdArgs;
	tkNetInit();
	
	ProcessingListCons( &ProcList );

	KeyInfoCacheCons(&KeyInfoCache);
	KeyInfoReadFile(&KeyInfoCache,"tknet.info");
	
	while(!KeyInfoTry(&KeyInfoCache,KEY_INFO_TYPE_STUNSERVER))
	{
		if(!KeyInfoTry(&KeyInfoCache,KEY_INFO_TYPE_MAILSERVER))
		{
			printf("No way :( \n");
			goto exit;
		}
	}
	printf("NAT: %d\n",g_NATtype);

	/*while(!KeyInfoTry(&KeyInfoCache,KEY_INFO_TYPE_BRIDGEPEER))
	{
		if(!KeyInfoTry(&KeyInfoCache,KEY_INFO_TYPE_MAILSERVER))
		{
			printf("No way :( \n");
			goto exit;
		}
	}*/

	MutexInit(&g_BkgdMutex);

	BkgdArgs.pInfoCache = &KeyInfoCache;
	BkgdArgs.pProcList = &ProcList;
	tkBeginThread( &BackGround , &BkgdArgs );

	while( g_MainLoopFlag )
	{
		MutexLock(&g_BkgdMutex);
		DoProcessing( &ProcList );
		MutexUnlock(&g_BkgdMutex);

		tkMsSleep(100);
	}

exit:

	KeyInfoUpdate( &KeyInfoCache );
	KeyInfoWriteFile(&KeyInfoCache,"baba.info");
	KeyInfoFree(&KeyInfoCache);
	MutexDelete(&g_BkgdMutex);
	tkNetUninit();
	return 0;
}
