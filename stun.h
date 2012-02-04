#define STUN_DEFAULT_PORT 3478
#define STUN_MAX_VALUES 20

#define STUN_CHANGE_BOTH_IP_PORT  0x01
#define STUN_CHANGE_IP            0x02
#define STUN_CHANGE_PORT          0x03

struct StunHead
{
	ushort type;
	ushort length;
	uint transId1;//MagicCookie
	uint transId2;
	uint transId3;
	uint transId4;
};

struct StunAttribute
{
	ushort type;
	ushort length;
	uint   value[STUN_MAX_VALUES];
};

struct ChangeRequest
{
	struct StunHead head;
	ushort type;
	ushort length;
	uint  value;
};

int 
StunFormulateRequest(struct StunHead*);

int 
StunFormulateChangeRequest(struct ChangeRequest* , uchar);

struct NetAddr
StunGetPublicNetAddr(void*, uint , uint );

static __inline BOOL
ifGetPublicAddrSucc(struct NetAddr *pa_pNetAddr)
{
	if(pa_pNetAddr->IPv4 == 0 ||
		pa_pNetAddr->port == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
