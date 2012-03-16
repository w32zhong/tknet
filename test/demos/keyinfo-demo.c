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
	BOOL res;
	tkNetInit();

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
exit:

	KeyInfoTrace(&KeyInfoCache);
	KeyInfoUpdate( &KeyInfoCache );
	KeyInfoTrace(&KeyInfoCache);
	KeyInfoWriteFile(&KeyInfoCache,"baba.info");
	KeyInfoFree(&KeyInfoCache);

	tkNetUninit();

	return 0;
}
