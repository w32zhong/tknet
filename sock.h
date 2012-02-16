
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


#if defined( __GNUC__ ) && defined( __linux__ )

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h> //declaration of function fcntl
#include <errno.h> //get last erro value error
#include <ifaddrs.h> //declarations to get local IP address
#include <unistd.h>//close() declaration to close socket

typedef int SOCKVAL;
#define SOCK_ERROR_CDT < 0
#define WSAINIT
#define WSACLEAN

#elif defined _MSC_VER

#ifdef TK_CONFIG_SOCK_SSL_ENABLE
#pragma comment(lib,"libeay32.lib" )
#pragma comment(lib,"ssleay32.lib" )
#endif

#undef BOOL
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

typedef SOCKET SOCKVAL;
#define SOCK_ERROR_CDT == SOCKET_ERROR
#define WSACLEAN WSACleanup()
#define WSAINIT  WSADATA wsa ; \
	WSAStartup(MAKEWORD(2,2), &wsa)

#endif

#ifdef TK_CONFIG_SOCK_SSL_ENABLE

#include <openssl/ssl.h>
#include <openssl/rand.h>

#define CERTF "client.pem"
#define KEYF "client.key"
#define CACERT "ca.pem"

#endif

#define SOCK_RECV_BUFF_LEN 2048

#define UDP SOCK_DGRAM
#define TCP SOCK_STREAM

struct Sock
{
	struct sockaddr_in AddrTa,AddrMe,AddrRecvfrom;
	SOCKVAL	socket;
	int 	proto;
	uint	RecvLen;
	char	RecvBuff[ SOCK_RECV_BUFF_LEN ];
	
#ifdef TK_CONFIG_SOCK_SSL_ENABLE
	SSL_CTX     *ctx;
	SSL         *ssl;
#endif
};

void
SockInit();

void
SockDestory();

void
SockSSLConnect( struct Sock* );

int 
SockOpen(struct Sock* , int , ushort);

void
GetIPTextAndPort(struct sockaddr_in* , ushort* , char* );

int 
SockLocateTa( struct Sock* , uint, ushort );

uint 
GetIPVal(const char*);

void 
SockWrite( struct Sock* , struct Bys );

uint 
SockGetLastErr();

BOOL 
SockRead( struct Sock* );

void
SockSetNonblock( struct Sock* );

void 
SockGetLocalIP( void (*Callback)(char* ,void*) , void* );

void 
SockClose( struct Sock* pa_sock );
