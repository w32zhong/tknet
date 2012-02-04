#include "headers.h"

void tkNetInit();
void tkNetUninit();
BOOL g_flag = 1;
void Notify(struct Process *pa_)
{
	ProcessTraceSteps(pa_);
	g_flag = 0;
}

int main()
{
	struct ProcessingList ProcList;
	struct SMTPProc SmtpProc;
	tkNetInit();

	ProcessingListCons( &ProcList );
	
	SMTPProcMake(&SmtpProc ,"113.108.225.10" , 25,0,"li28jhyxy76223","g131517","li28jhyxy76223@163.com","Ok");
	SmtpProc.proc.NotifyCallbk = &Notify;
	ProcessStart( &SmtpProc.proc , &ProcList );

	while( g_flag )
	{
		DoProcessing( &ProcList );
		usleep( 100 * 1000);
	}

	ProcessFree(&SmtpProc.proc);
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
