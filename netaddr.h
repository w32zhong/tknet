struct NetAddr
{		
	uint    IPv4;
	ushort  port;
};

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

static __inline BOOL
ifNetAddrLessThan(struct NetAddr *pa_0,struct NetAddr *pa_1)
{
	if( pa_0->port != pa_1->port )
	{
		return pa_0->port < pa_1->port;
	}
	else
	{
		return pa_0->IPv4 < pa_1->IPv4;
	}
}
