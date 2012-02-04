#include "headers.h"

void Notify( struct Process *pa_pProc)
{
	printf(" proc end notify \n");
}

STEP( First )
{
	if( pa_state == PS_STATE_FIRST_TIME )
	{
		printf("first step begin:");
		ProcessTraceSteps( pa_pProc );
	}
	else if( pa_state == PS_STATE_OVERTIME )
	{
		printf("first step retry:");
		ProcessTraceSteps( pa_pProc );

		if( tkGetRandom() % 10 == 1 ) 
		{ 
			printf(" random abort! \n"); 
			return PS_CALLBK_RET_ABORT; 
		}
	}
	else if( pa_state == PS_STATE_LAST_TIME )
	{
		printf("first step over:");
		ProcessTraceSteps( pa_pProc );
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( Second )
{
	uint ran = 0;
	if( pa_state == PS_STATE_OVERTIME )
	{
		printf("second step first over-time: \n");
		ProcessTraceSteps( pa_pProc );
		
		ran = tkGetRandom()%10; 
		printf("random=%d ,", ran);
		if( ran < 3)
		{
			printf("Go back to step 0\n");
			return FlagNum(0);
		}
		else if( ran < 7 )
		{
			printf("Go to step 1\n");
			return FlagNum(1);
		}
		else
		{
			printf("Go to step 2\n");
			return FlagNum(2);
		}
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( Third )
{
	if( pa_state == PS_STATE_FIRST_TIME )
	{
		printf("Third step first time: ");
		ProcessTraceSteps( pa_pProc );
	}
	else if( pa_state == PS_STATE_OVERTIME )
	{
		printf("Third step retry: ");
		ProcessTraceSteps( pa_pProc );
	}

	return PS_CALLBK_RET_GO_ON;
}

STEP( circle )
{
	if( pa_state == PS_STATE_FIRST_TIME )
	{
		printf("circle step: \n");
		ProcessTraceSteps( pa_pProc );
	}
	else if( pa_state == PS_STATE_OVERTIME )
	{
		return PS_CALLBK_RET_REDO;
	}

	return PS_CALLBK_RET_GO_ON;
}

int main()
{
	struct ProcessingList ProcList;
	struct Process ProcTemplate;
	struct Process ProcTemplate2;
	struct Process Proc[6];
	int i;
	
	tkInitRandom();
	tkLogInit();
	ProcessingListCons( &ProcList );

	ProcessCons( &ProcTemplate );
	ProcTemplate.NotifyCallbk = &Notify;
	PROCESS_ADD_STEP( &ProcTemplate , First , 1000 , 8 );
	PROCESS_ADD_STEP( &ProcTemplate , Second , 5000 , 3 );
	PROCESS_ADD_STEP( &ProcTemplate , Third , 2000 , 5 );

	ProcessCons( &ProcTemplate2 );
	PROCESS_ADD_STEP( &ProcTemplate2 , circle , 1500 , 3 );
	
	for(i=0;i<6;i++)
	{
		ProcessConsAndSetSteps( Proc + i , &ProcTemplate );
		Proc[i].NotifyCallbk = &Notify;
		ProcessStart( Proc + i , &ProcList );
	}

	ProcessStart( &ProcTemplate , &ProcList );
	ProcessStart( &ProcTemplate2 , &ProcList );

	while(1)
	{
		DoProcessing( &ProcList );
		sleep(1);
	}

	tkLogClose();

	return 0;
}
