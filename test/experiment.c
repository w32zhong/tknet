#include "tknet.h"

int main()
{
	struct Sock client;
        ushort port;
        char buff[32];
        int res = 0;

        SockInit();
        tkLogInit();

        SockOpen( &client , TCP , 8812 );
        
        GetIPTextAndPort(&client.AddrMe,&port,buff);
        printf("my addr: %s/%d \n",buff,port);

        if (!SockLocateTa( &client , GetIPVal("123.125.50.23") , 110 ))
        //126 mail server
        {
                printf("last error: %d \n", SockGetLastErr() );
                return 0;
        }

        res = SockRead( &client );
        GetIPTextAndPort(&client.AddrRecvfrom,&port,buff);
        printf("read:%s ,from %s/%d res=%d \n",client.RecvBuff,buff,port,res);

        SockWrite( &client , StrBys("USER clock126\r\n") );

        res = SockRead( &client );
        GetIPTextAndPort(&client.AddrRecvfrom,&port,buff);
        printf("read:%s ,from %s/%d res=%d \n",client.RecvBuff,buff,port,res);

	printf("HELLO!!!!!!11 \n");
        
        SockClose( &client );
        SockDestory();
}
