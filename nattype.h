#define NAT_T_FULL_CONE       0x00
#define NAT_T_RESTRICTED      0x01
#define NAT_T_PORT_RESTRICTED 0x02
#define NAT_T_SYMMETRIC       0x03
#define NAT_T_UNKNOWN         0x04

struct STUNProc
{
	struct Process proc;
	struct Sock    *pSock;
	int            HostIPVal;
	ushort         HostPort;
	uint           MagicCookieTemp;
	struct NetAddr MapAddrTemp;
	struct NetAddr ServerAddrTemp;
	uchar          NatTypeRes;
};

void 
MakeProtoStunProc( struct STUNProc * ,struct Sock * , const char * , ushort );
