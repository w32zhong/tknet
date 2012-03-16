#include "headers.h"

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
	struct PeerData PeerDataRoot,*p;
	int i;
	char buff[32];
	struct NetAddr addr;
	tkNetInit();

	PeerDataCons(&PeerDataRoot);
	PeerDataRoot.tpnd.RanPriority = 0;
	PeerDataRoot.addr.port = 0;
	PeerDataRoot.addr.IPv4 = 0;

	for(i=0;i<9;i++)
	{
		p = tkmalloc(struct PeerData);
		PeerDataCons(p);
		p->addr.port = (ushort)tkGetRandom();
		p->addr.IPv4 = tkGetRandom();
		
		GetAddrText(&addr,buff);
		if(PeerDataFind(&PeerDataRoot,&addr))
		{
			printf("find\n");
		}
		else
		{
			printf("NOT find %s\n",buff);
		}

		p->NATType = NAT_T_UNKNOWN;
		PeerDataInsert(p,&PeerDataRoot);
		addr = p->addr;
	}

	PeerDataTrace(&PeerDataRoot);
	PeerDataDestroy(&PeerDataRoot);

	tkNetUninit();
	return 0;
}
