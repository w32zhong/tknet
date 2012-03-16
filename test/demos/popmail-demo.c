#include "headers.h"
BOOL flag = 1;

void Notify( struct Process *pa_pProc)
{
	printf(" proc end notify \n");
	flag = 0;
	ProcessTraceSteps( pa_pProc );
}

void tkNetInit();
void tkNetUninit();

int main()
{
	struct ProcessingList ProcList;
	struct Sock       pop3sock;
	struct POP3Proc   NetProtoPOP3;

	tkNetInit();

	ProcessingListCons( &ProcList );
	SockOpen( &pop3sock , TCP , 0);
	
	MakeProtoPOP3Proc( &NetProtoPOP3 , "220.181.12.101" ,110,0,"li28jhyxy76223","g131517");
	NetProtoPOP3.proc.NotifyCallbk = &Notify;
	NetProtoPOP3.pSock = &pop3sock;

	ProcessStart( &NetProtoPOP3.proc , &ProcList );

	while( flag )
	{
		DoProcessing( &ProcList );
		usleep( 100 * 1000);
	}

	ForEach( &NetProtoPOP3.IMailsHead , &TraceMail , NULL );
	
	POP3ProcFree( &NetProtoPOP3 );

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
