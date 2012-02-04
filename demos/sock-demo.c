#include "headers.h"
#include <stdio.h>

//-----------------TCP SSL demo------------------------ 
int 
main()
{
	struct Sock client;
	ushort port;
	char buff[32];
	int res = 0;

	buff[0]='a';
	buff[1]='\0';
	SockInit();
	tkLogInit();

	SockOpen( &client , TCP , 0 );
	//
	
	GetIPTextAndPort(&client.AddrMe,&port,buff);
	printf("my addr: %s/%d \n",buff,port);

	if (!SockLocateTa( &client , GetIPVal("123.125.50.23") , 995 ))
	{
		printf("last error: %d \n", SockGetLastErr() );
		return 0;
	}

	GetIPTextAndPort(&client.AddrTa,&port,buff);
	printf("ta addr: %s/%d \n",buff,port);
	
	SockSSLConnect( &client );

	res = SockRead( &client );
	GetIPTextAndPort(&client.AddrRecvfrom,&port,buff);
	printf("read:%s ,from %s/%d res=%d \n",client.RecvBuff,buff,port,res);

	SockWrite( &client , StrBys("USER clock126\r\n") );

	res = SockRead( &client );
	GetIPTextAndPort(&client.AddrRecvfrom,&port,buff);
	printf("read:%s ,from %s/%d res=%d \n",client.RecvBuff,buff,port,res);
	
	SockClose( &client );
	SockDestory();

	return 0;

}

/*
//-----------------TCP demo----------------------------- 
int
main()
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

	GetIPTextAndPort(&client.AddrTa,&port,buff);
	printf("ta addr: %s/%d \n",buff,port);
	
	res = SockRead( &client );
	GetIPTextAndPort(&client.AddrRecvfrom,&port,buff);
	printf("read:%s ,from %s/%d res=%d \n",client.RecvBuff,buff,port,res);

	SockWrite( &client , StrBys("USER clock126\r\n") );

	res = SockRead( &client );
	GetIPTextAndPort(&client.AddrRecvfrom,&port,buff);
	printf("read:%s ,from %s/%d res=%d \n",client.RecvBuff,buff,port,res);
	
	SockClose( &client );
	SockDestory();
	
	return 0;
}

//-----------------TCP POP3 demo------------------------
int main()
{
	struct Sock s;

	tkLogInit();
	SockInit();

	SockOpen( &s , TCP , 8812 );
	SockLocateTa( &s , GetIPVal("220.181.15.128") ,110 );

	SockSetNonblock( &s );
	
	while(!SockRead(&s));
	printf("\n read:%s \n", s.RecvBuff);
	SockWrite( &s , StrBys("USER clock126\r\n") );
	
	while(!SockRead(&s));
	printf("\n read:%s \n", s.RecvBuff);
	SockWrite( &s , StrBys("PASS bula..bula\r\n") );
	while(!SockRead(&s));
	printf("\n read:%s \n", s.RecvBuff);
	
	SockWrite( &s , StrBys("LIST\r\n") );
	while(!SockRead(&s));
	printf("\n read:%s \n", s.RecvBuff);

	SockWrite( &s , StrBys("RETR 278\r\n") );
	while(!SockRead(&s));
	printf("\n read:%s \n", s.RecvBuff);

	while(!SockRead(&s));
	printf("\n read:%s \n", s.RecvBuff);

	SockDestory();
	tkLogClose();
	return 0;
}
*/
