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

void
SockInit()
{
	WSAINIT;
}

void
SockDestory()
{
	WSACLEAN;
}

void
SockSSLConnect( struct Sock* out_sock )
{
#ifdef TK_CONFIG_SOCK_SSL_ENABLE
	SSL_CTX*     ctx;
	SSL*         ssl;

	SSL_load_error_strings();
	VCK( SSL_library_init() != 1 ,return);

	ctx = SSL_CTX_new (SSLv3_client_method());
	VCK( ctx == NULL ,return);
	ssl = SSL_new (ctx);
	VCK( ssl == NULL ,return);

	VCK( SSL_set_fd ( ssl, out_sock->socket ) != 1 , return);
	//enable socket SSL

	VCK( SSL_connect (ssl) != 1 ,
			TK_EXCEPTION("SSLConnect failed or be shutdown");
			return);
	//connect SSL

	out_sock->ctx = ctx;
	out_sock->ssl = ssl;
#endif
}

// 'pa_port' is discarded when use TCP
int
SockOpen(struct Sock* out_sock , int pa_proto , ushort pa_port)
{
	int res;
	VCK( (out_sock->socket = socket(AF_INET, pa_proto ,0))
			SOCK_ERROR_CDT , WSACLEAN; return 0 );

	if( pa_proto == UDP )
	{
		out_sock->AddrMe.sin_family = AF_INET;
		out_sock->AddrMe.sin_port = htons(pa_port);
		out_sock->AddrMe.sin_addr.s_addr = htonl(INADDR_ANY);
		//since INADDR_ANY is defined in host byte order ,
		//it need to be converted to network byte order.

		res = bind( out_sock->socket ,(struct sockaddr*)&out_sock->AddrMe ,
					sizeof(struct sockaddr_in));

		VCK( res SOCK_ERROR_CDT ,return 0 );
	}

	out_sock->proto = pa_proto;

#ifdef TK_CONFIG_SOCK_SSL_ENABLE
	out_sock->ctx = NULL;
	out_sock->ssl = NULL;
#endif

	return 1;
}

void
GetIPTextAndPort(struct sockaddr_in* in_addr , ushort* out_pPort , char* pa_pBuff )
{
	char* pStatBuff;

	if( out_pPort )
	{
		*out_pPort = ntohs(in_addr->sin_port);
	}

	pStatBuff = (char*)inet_ntoa(in_addr->sin_addr);
	//for implementation to support IPv6 ,
	//inet_ntoa(), inet_aton(), inet_addr() are deprecated.

	strcpy( pa_pBuff , pStatBuff );
}

uint GetIPVal(const char* in_text)
{
	//for implementation to support IPv6 ,
	//inet_ntoa(), inet_aton(), inet_addr() are deprecated.

	return inet_addr(in_text);
}

//when select UDP , we can call this function repeatedly.
//However , TCP socket must do connect function only once.
int
SockLocateTa( struct Sock* pa_sock , uint in_IPVal , ushort pa_port )
{
	pa_sock->AddrTa.sin_family = AF_INET;
	pa_sock->AddrTa.sin_port = htons(pa_port);
	pa_sock->AddrTa.sin_addr.s_addr = in_IPVal;

	if( pa_sock->proto == TCP )
	{
		VCK( connect( pa_sock->socket , (struct sockaddr*)&pa_sock->AddrTa ,
					sizeof( struct sockaddr_in)) SOCK_ERROR_CDT ,
				return 0);
	}

	return 1;
}

void
SockWrite( struct Sock* pa_sock , struct Bys pa_bys )
{
#ifdef TK_CONFIG_SOCK_SSL_ENABLE
	if( pa_sock->ssl != NULL )
	{
		SSL_write( pa_sock->ssl , pa_bys.pBytes , pa_bys.len );
	}else
#endif
	if( pa_sock->proto == TCP )
	{
		VCK( send(pa_sock->socket,pa_bys.pBytes,pa_bys.len,0) SOCK_ERROR_CDT,
				return );
	}
	else if( pa_sock->proto == UDP )
	{
		VCK( sendto(pa_sock->socket,pa_bys.pBytes,pa_bys.len,0,
					(struct sockaddr*)&pa_sock->AddrTa,
					sizeof( struct sockaddr_in ) ) SOCK_ERROR_CDT,
					return );
	}
}

__inline uint
SockGetLastErr()
{
#if defined( __GNUC__ ) && defined( __linux__ )
	return errno;
#elif defined _MSC_VER
	return WSAGetLastError();
#endif
}

BOOL
SockRead( struct Sock* pa_sock )
{
	int   res;
	BOOL  ret;
	uint  alen;

#ifdef TK_CONFIG_SOCK_SSL_ENABLE
    int   SSL_err_res;

	if( pa_sock->ssl != NULL )
	{
		res = SSL_read( pa_sock->ssl , pa_sock->RecvBuff , SOCK_RECV_BUFF_LEN );
		SSL_err_res = SSL_get_error( pa_sock->ssl , res );

		if( res < 0 && SSL_err_res == SSL_ERROR_WANT_READ )
		{
			ret = 0;
		}
		else
		{
			ret = 1;
		}

	}else
#endif
	{
		if( pa_sock->proto == TCP )
		{
			res = recv(pa_sock->socket , pa_sock->RecvBuff , SOCK_RECV_BUFF_LEN , 0);
		}
		else if( pa_sock->proto == UDP )
		{
			alen = sizeof( struct sockaddr_in );
			res = recvfrom( pa_sock->socket , pa_sock->RecvBuff ,
					SOCK_RECV_BUFF_LEN , 0 ,
					(struct sockaddr*)&pa_sock->AddrRecvfrom , &alen);
		}
		else
		{
			TK_EXCEPTION( Sock Proto );
			res = 0;
		}

#if defined( __GNUC__ ) && defined( __linux__ )
		if( res < 0 &&
			( SockGetLastErr() == EAGAIN || SockGetLastErr() == EWOULDBLOCK )
		)
		//when the socket is non-blocking in which case the value -1 is
		//returned and the external variable errno is set to EAGAIN or EWOULDBLOCK.

#elif defined _MSC_VER
		if( res < 0 && SockGetLastErr() == WSAEWOULDBLOCK )

#endif
		{
			ret = 0;
		}
		else
		{
			ret = 1;
		}
	}

	if(ret == 0)
	{
		// we need to let res be 0 so that in this 
		//case that non-block sock waiting for
		//message is not regarded as an error.
		res = 0;
	}

	VCK( res >= SOCK_RECV_BUFF_LEN , res = SOCK_RECV_BUFF_LEN - 1 );

	VCK( res < 0 ,  res = 0; );
	pa_sock->RecvBuff[res] = '\0';
	pa_sock->RecvLen = res;

	return ret;
}

void
SockSetNonblock( struct Sock* pa_sock )
{
#if defined( __GNUC__ ) && defined( __linux__ )
	fcntl( pa_sock->socket , F_SETFL, O_NONBLOCK);
#elif defined _MSC_VER
	int Mode = 1;
	ioctlsocket( pa_sock->socket ,FIONBIO, (u_long FAR*) &Mode);
#endif
}

void
SockGetLocalIP( void (*Callback)(char* ,void*) , void* pa_else )
{
	char buff[32];

#if defined( __GNUC__ ) && defined( __linux__ )
	struct ifaddrs *ifAddrStruct=NULL;
	struct ifaddrs *ifa=NULL;

	getifaddrs(&ifAddrStruct);

	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
	{
		GetIPTextAndPort((struct sockaddr_in *)ifa->ifa_addr ,
					 NULL , buff );
		Callback( buff , pa_else );
	}

	if (ifAddrStruct!=NULL)
		freeifaddrs(ifAddrStruct);

#elif defined _MSC_VER
	char HostName[128];
	struct hostent *pHostent;
	struct in_addr AddrRes;
	int i;

	gethostname( HostName , sizeof( HostName ) );
	pHostent = gethostbyname(HostName);

	for ( i = 0; pHostent->h_addr_list[i] != 0; i++ )
	{
		memcpy(&AddrRes, pHostent->h_addr_list[i], sizeof(struct in_addr));
		memcpy( buff , inet_ntoa(AddrRes) , strlen(inet_ntoa(AddrRes)) + 1 );
		Callback( buff , pa_else );
	}
#endif
}

void
SockClose( struct Sock* pa_sock )
{
#if defined( __GNUC__ ) && defined( __linux__ )
	shutdown( pa_sock->socket , SHUT_RDWR );
	close( pa_sock->socket );
#elif defined _MSC_VER
	shutdown( pa_sock->socket , SD_BOTH );
	closesocket( pa_sock->socket );
#endif

#ifdef TK_CONFIG_SOCK_SSL_ENABLE
	if( pa_sock->ssl != NULL )
	{
		SSL_shutdown(pa_sock->ssl);
		SSL_free(pa_sock->ssl);
		SSL_CTX_free(pa_sock->ctx);
		pa_sock->ssl = NULL;
		pa_sock->ctx = NULL;
	}
#endif
}
