#include "sysi.h"
#include "comdef.h"
#include "sock.h"

int
main(int argc, char **argv)
{
	struct Sock client;
	char buff[128];
	SockInit();

	printf("port: %d -> %d \n", atoi(argv[1]) , atoi(argv[2]) );

	SockOpen( &client , UDP , atoi(argv[1]) );

	SockLocateTa( &client , GetIPVal("127.0.0.1") , atoi(argv[2]) );

	SockSetNonblock( &client );

	while(1)
	{
		if(SockRead( &client ))
		{
			printf("%s \n",client.RecvBuff);
		}

		scanf("%s",buff);
		SockWrite( &client , StrBys(buff) );

		sleep(1);
	}

	SockClose( &client );
	SockDestory();
	
	return 0;
}
