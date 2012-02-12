#include "headers.h"

DEF_STRUCT_CONSTRUCTOR( Process ,
		out_cons->IProcessHead = GetIterator(NULL);
		out_cons->IProcessNow = GetIterator(NULL);
		ListNodeCons(&out_cons->UndergoLN);
		out_cons->CurrentStepStartTime = 0;
		out_cons->CurrentStepRetrys = 0;
		out_cons->NotifyCallbk = NULL;
		out_cons->steps = 0;
		)

DEF_STRUCT_CONSTRUCTOR( ProcessingList ,
		out_cons->IUndergoProcess = GetIterator(NULL);
		)

static void
ProcessUpdateCurrentStep(struct Process *pa_pProc)
{
	struct ProcessStep *pStep = GET_STRUCT_ADDR_FROM_IT( &pa_pProc->IProcessNow , struct ProcessStep , ProcStepLN );
	pa_pProc->CurrentStepStartTime = tkMilliseconds();
	pa_pProc->CurrentStepRetrys = pStep->MaxRetrys;
	pa_pProc->isCurrentStepFirstTime = 1;
}

void 
ProcessAddStep( struct Process *pa_pProc , StepCallbk pa_StepDo , uint pa_WaitClocks , uchar pa_MaxRetrys , const char *pa_pName )
{
	struct ProcessStep *pNewStep = tkmalloc( struct ProcessStep );

	pNewStep->FlagNum = pa_pProc->steps;
	pNewStep->WaitClocks = pa_WaitClocks;
	pNewStep->MaxRetrys = pa_MaxRetrys;
	pNewStep->StepDo = pa_StepDo;
	ListNodeCons(&pNewStep->ProcStepLN);
	pNewStep->pName = pa_pName;

	AddOneToListTail( &pa_pProc->IProcessHead , &pNewStep->ProcStepLN );
	pa_pProc->steps ++;
}

void 
ProcessConsAndSetSteps( struct Process *out_proc , struct Process *in_proc )
{
	ProcessCons( out_proc );
	out_proc->IProcessHead = in_proc->IProcessHead;
	out_proc->steps = in_proc->steps;
}

void 
ProcessStart( struct Process *pa_pProc , struct ProcessingList *pa_pProcList )
{
	pa_pProc->IProcessNow = pa_pProc->IProcessHead;
	ProcessUpdateCurrentStep( pa_pProc );
	AddOneToListTail( &pa_pProcList->IUndergoProcess , &pa_pProc->UndergoLN );
}

void 
ProcessSafeStart( struct Process *pa_pProc , struct ProcessingList *pa_pProcList , struct Iterator *pa_pINow , struct Iterator *pa_pIForward )
{
	pa_pProc->IProcessNow = pa_pProc->IProcessHead;
	ProcessUpdateCurrentStep( pa_pProc );
	AddOneToListTailSafe( &pa_pProcList->IUndergoProcess , pa_pINow , pa_pIForward , &pa_pProc->UndergoLN );
}

static BOOL 
LIST_ITERATION_CALLBACK_FUNCTION( FindStepWithFlagNum )
{
	struct ProcessStep *pStep = GET_STRUCT_ADDR_FROM_IT( pa_pINow , struct ProcessStep , ProcStepLN );
	DEF_AND_CAST( pFswfnp , struct FindStepWithFlagNumPa , pa_else );

	if( pStep->FlagNum == pFswfnp->FlagNumToFind )
	{
		pFswfnp->IFound = *pa_pINow;
		return 1;
	}
	else
	{
		return pa_pINow->now == pa_pIHead->last;
	}
}

static BOOL 
LIST_ITERATION_CALLBACK_FUNCTION( DoProcess )
{
	uchar    ps_res,ps_state = PS_STATE_NORMAL;
	long     dt;
	struct Process *pProc = GET_STRUCT_ADDR_FROM_IT( pa_pINow , struct Process , UndergoLN );
	struct ProcessStep *pStep = GET_STRUCT_ADDR_FROM_IT( &pProc->IProcessNow , struct ProcessStep , ProcStepLN );
	struct FindStepWithFlagNumPa fswfpa;

	if( pProc->isCurrentStepFirstTime )
	{
		pProc->isCurrentStepFirstTime = 0;
		ps_state = PS_STATE_FIRST_TIME;
	}
	else if( tkMilliseconds() < pProc->CurrentStepStartTime )
	{
		pProc->CurrentStepStartTime = tkMilliseconds();
	}
	else
	{
		dt = tkMilliseconds() - pProc->CurrentStepStartTime;
		if( dt > pStep->WaitClocks )
		{
			if( pProc->CurrentStepRetrys == 0 )
			{
				ps_state = PS_STATE_LAST_TIME;
			}
			else
			{
				pProc->CurrentStepRetrys --;
				pProc->CurrentStepStartTime = tkMilliseconds();
				ps_state = PS_STATE_OVERTIME;
			}
		}
	}

	ps_res = pStep->StepDo( pProc , ps_state , pa_pINow , pa_pIForward );

	if( (ps_res & 0x80) == 0x00 )
	{
		fswfpa.FlagNumToFind = ps_res;
		fswfpa.IFound = GetIterator(NULL);
		ForEach( &pProc->IProcessHead , &FindStepWithFlagNum , &fswfpa );

		if(fswfpa.IFound.now == NULL)
		{
			ps_res = PS_CALLBK_RET_DONE;
		}
		else
		{
			pProc->IProcessNow = fswfpa.IFound;
			ProcessUpdateCurrentStep( pProc );
			goto ret;
		}
	}

	switch( ps_res )
	{
		case PS_CALLBK_RET_ABORT:
			
			tk(pa_pINow,pa_pIForward);
			if( pProc->NotifyCallbk != NULL )
				pProc->NotifyCallbk( pProc );
			LIST_SAFE_RETURN;
		
		case PS_CALLBK_RET_GO_ON:
			
			if( ps_state != PS_STATE_LAST_TIME )
			{
				break;
			}

		case PS_CALLBK_RET_DONE:

			if( pProc->IProcessNow.now == pProc->IProcessHead.last )
			{
				tk(pa_pINow,pa_pIForward);
				if( pProc->NotifyCallbk != NULL )
					pProc->NotifyCallbk( pProc );
				LIST_SAFE_RETURN;
			}
			else
			{
				pProc->IProcessNow = GetIterator( pProc->IProcessNow.now->next );
			}

		case PS_CALLBK_RET_REDO:

			ProcessUpdateCurrentStep( pProc );
			break;

		default:
			TK_EXCEPTION( "proc step callbk return" );
			break;

	}

ret:
	return pa_pINow->now == pa_pIHead->last;
}

void 
DoProcessing( struct ProcessingList *pa_pProcList )
{
	ForEach( &pa_pProcList->IUndergoProcess , &DoProcess , pa_pProcList );
}

DEF_FREE_LIST_ELEMENT_CALLBACK_FUNCTION( FreeProcStep , struct ProcessStep , ProcStepLN , ; )

void ProcessFree( struct Process *pa_pProc )
{
	ForEach(&pa_pProc->IProcessHead,&FreeProcStep,NULL);
}

static BOOL 
LIST_ITERATION_CALLBACK_FUNCTION( TraceProcStep )
{
	struct ProcessStep *pProcStep = GET_STRUCT_ADDR_FROM_IT( pa_pINow , struct ProcessStep , ProcStepLN );
	char PrintStr[128];
	char BuffStr[128];
	DEF_AND_CAST( pProc , struct Process , pa_else );

	if( pa_pINow->now == pProc->IProcessNow.now )
	{
		strcpy(PrintStr , "Now(");
	}
	else
	{
		strcpy(PrintStr , "(");
	}

	if( pProcStep->pName != NULL )
	{
		sprintf( BuffStr , "%s" , pProcStep->pName );
		strcat( PrintStr , BuffStr );
		strcat( PrintStr , ",");
	}
	
	sprintf( BuffStr , "%ld" , pProcStep->WaitClocks );
	strcat( PrintStr , BuffStr );
	strcat( PrintStr , ",");
	
	sprintf( BuffStr , "%d" , pProcStep->FlagNum );
	strcat( PrintStr , BuffStr );
	strcat( PrintStr , ",");
	
	sprintf( BuffStr , "%d" , pProcStep->MaxRetrys );
	strcat( PrintStr , BuffStr );
	strcat( PrintStr , ")");

	if( pa_pINow->now == pa_pIHead->last )
	{
		printf("%s \n" ,PrintStr);
		return 1;
	}
	else
	{
		printf("%s->" ,PrintStr);
		return 0;
	}
}

void
ProcessTraceSteps(struct Process *pa_pProc)
{
	ForEach( &pa_pProc->IProcessHead , &TraceProcStep , pa_pProc );
}

static BOOL 
LIST_ITERATION_CALLBACK_FUNCTION( FindStepByName )
{
	struct ProcessStep *pProcStep = GET_STRUCT_ADDR_FROM_IT( pa_pINow , struct ProcessStep , ProcStepLN );
	DEF_AND_CAST(pFsbnpa,struct FindStepByNamePa,pa_else);

	if( strcmp(pFsbnpa->pNameToFind , pProcStep->pName) == 0 )
	{
		pFsbnpa->pFound = pProcStep;
		return 1;
	}
	else
	{
		return pa_pINow->now == pa_pIHead->last;
	}
}

uchar
FlagName(struct Process *pa_pProc ,const char *pa_pName)
{
	struct FindStepByNamePa fsbnpa;
	fsbnpa.pFound = NULL;
	fsbnpa.pNameToFind = pa_pName;

	ForEach( &pa_pProc->IProcessHead , &FindStepByName , &fsbnpa );

	if(fsbnpa.pFound)
	{
		return fsbnpa.pFound->FlagNum;
	}
	else
	{
		return PS_CALLBK_RET_DONE;
	}
}
