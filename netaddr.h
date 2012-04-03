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

struct NetAddr
{		
	uint    IPv4; 
	// = ntohl( GetIPVal(IP text) ) 
	// IP text = GetAddrText(GetAddrFromSockAddr(sock_addr))
	ushort  port; 
};

static __inline struct NetAddr
NetAddr(const char *pa_pIPText,ushort pa_port)
{
	struct NetAddr res;
	res.IPv4 = ntohl(GetIPVal(pa_pIPText));
	res.port = pa_port;

	return res;
}

static __inline void
GetIPText( struct NetAddr *pa_pAddr , char *out_str)
{
	struct sockaddr_in SockAddr;
	char *pResBuff;
	SockAddr.sin_addr.s_addr = htonl(pa_pAddr->IPv4);
	pResBuff = (char*)inet_ntoa(SockAddr.sin_addr);
	strcpy( out_str , pResBuff );
}

static __inline void
GetAddrText( struct NetAddr *pa_pAddr , char *out_str)
{
	char buff[16];
	GetIPText(pa_pAddr,out_str);
	sprintf(buff,"/%d",pa_pAddr->port);
	strcat(out_str,buff);
}

static __inline struct NetAddr
GetAddrFromSockAddr( struct sockaddr_in *in_SockAddr)
{
	struct NetAddr res;
	res.port = ntohs(in_SockAddr->sin_port);
	res.IPv4 = ntohl(in_SockAddr->sin_addr.s_addr);

	return res;
}

static __inline BOOL
ifNetAddrEqual(struct NetAddr *pa_0,struct NetAddr *pa_1)
{
	return ( pa_0->IPv4 == pa_1->IPv4 && pa_0->port == pa_1->port );
}
