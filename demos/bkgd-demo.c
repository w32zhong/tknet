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
	struct ProcessingList ProcList;

	tkNetInit();
	ProcessingListCons( &ProcList );

	MutexInit(&g_BkgdMutex);
	tkBeginThread( &BackGround , &ProcList );

	while( g_MainLoopFlag )
	{
		MutexLock(&g_BkgdMutex);
		DoProcessing( &ProcList );
		MutexUnlock(&g_BkgdMutex);

		tkMsSleep(100);
	}

	MutexDelete(&g_BkgdMutex);
	tkNetUninit();
	return 0;
}
