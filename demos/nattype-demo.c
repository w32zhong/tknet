#include "headers.h"

void tkNetInit();
void tkNetUninit();
BOOL g_flag = 1;
void Notify(struct Process *pa_)
{
	struct STUNProc *pProc = GET_STRUCT_ADDR(pa_ , struct STUNProc , proc);

	switch( pProc->NatTypeRes )
	{
	case NAT_T_FULL_CONE:
		printf("NAT type: full cone. \n");
		break;
	case NAT_T_RESTRICTED:
		printf("NAT type: restricted cone. \n");
		break;
	case NAT_T_PORT_RESTRICTED:
		printf("NAT type: port restricted cone. \n");
		break;
	case NAT_T_SYMMETRIC:
		printf("NAT type: symmetric \n");
		break;
	default:
		printf("NAT type: unknown. \n");
	}

	ProcessTraceSteps(pa_);
	g_flag = 0;
}

int main()
{
	struct ProcessingList ProcList;
	struct Sock       StunSock;
	struct STUNProc   StunProc;
	char buff[32];
	tkNetInit();

	ProcessingListCons( &ProcList );
	SockOpen( &StunSock , UDP , 8821 );
	SockSetNonblock( &StunSock );

	MakeProtoStunProc(&StunProc ,&StunSock , "132.177.123.13",STUN_DEFAULT_PORT);
	StunProc.proc.NotifyCallbk = &Notify;

	ProcessStart( &StunProc.proc , &ProcList );

	while( g_flag )
	{
		DoProcessing( &ProcList );
		usleep( 100 * 1000);
	}

	ProcessFree(&StunProc.proc);
	tkNetUninit();
	return 0;
}

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
