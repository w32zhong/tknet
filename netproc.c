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

#include "tknet.h"

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


struct WatiLevel g_WaitLevel[TKNET_WAIT_LEVELS];


long   sta_WaitValue[TKNET_CONDITIONS][TKNET_WAIT_LEVELS] = {
		{600,1000,1500,9000,20000},  //under good net condition (condition0)
		{600,1500,2000,9000,20000},  //under nomal net condition (condition1)
		{600,2000,2000,9000,20000}}; //under bad net condition (condition2)
//level:  0    1    2    3    4

//NOTE: we currently use level 0 for result muti-sending, its wait value doesn't change.
//we currently use level 3 and 4 for client "wait step" and main server loop step, 
//its wait value doesn't change either.

uchar  sta_RetryValue[TKNET_CONDITIONS][TKNET_WAIT_LEVELS] = {
	{  2,   1,   1,   1,    1},
	{  2,   2,   2,   2,    2},
	{  3,   3,   3,   3,    3}};

static void 
ProcessTraceCondition(uint pa_condition)
{
	int i;
	long   **ppl;
	uchar  **ppc;

	VCK( pa_condition >= TKNET_CONDITIONS ,return);
	printf("tknet condition: %d \n",pa_condition);
	
	for(i=0 ; i < TKNET_WAIT_LEVELS ; i++ )
	{
		ppl=&g_WaitLevel[i].pInterval;
		ppc=&g_WaitLevel[i].pRetrys;
		printf("level%d: %ld*%d \n",i,**ppl,**ppc);
	}
}

void
ProcessSetCondition(uint pa_condition)
{
	int i;
	VCK( pa_condition >= TKNET_CONDITIONS ,
			printf("Setting condition failed, invalid num.\n");return);

	for(i=0 ; i < TKNET_WAIT_LEVELS ; i++ )
	{
		g_WaitLevel[i].pInterval = &sta_WaitValue[pa_condition][i];
		g_WaitLevel[i].pRetrys   = &sta_RetryValue[pa_condition][i];
	}

	printf("Set ");ProcessTraceCondition(pa_condition);
}

static void
ProcessUpdateCurrentStep(struct Process *pa_pProc)
{
	struct ProcessStep *pStep = GET_STRUCT_ADDR_FROM_IT( &pa_pProc->IProcessNow , struct ProcessStep , ProcStepLN );
	pa_pProc->CurrentStepStartTime = tkMilliseconds();
	pa_pProc->CurrentStepRetrys = **pStep->ppMaxRetrys;
	pa_pProc->isCurrentStepFirstTime = 1;
}

void 
ProcessAddStep( struct Process *pa_pProc , StepCallbk pa_StepDo , long **pa_ppWaitClocks , uchar **pa_ppMaxRetrys , const char *pa_pName )
{
	struct ProcessStep *pNewStep = tkmalloc( struct ProcessStep );

	pNewStep->FlagNum = pa_pProc->steps;
	pNewStep->ppWaitClocks = pa_ppWaitClocks;
	pNewStep->ppMaxRetrys = pa_ppMaxRetrys;
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
	DEF_AND_CAST(pProcList,struct ProcessingList,pa_else);

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
		if( dt > **pStep->ppWaitClocks )
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
			
			if( pProc->NotifyCallbk != NULL )
				pProc->NotifyCallbk( pProc );

			return ProcessDisattach(pProc,pProcList);
		
		case PS_CALLBK_RET_GO_ON:
			
			if( ps_state != PS_STATE_LAST_TIME )
			{
				break;
			}

		case PS_CALLBK_RET_DONE:

			if( pProc->IProcessNow.now == pProc->IProcessHead.last )
			{
				if( pProc->NotifyCallbk != NULL )
					pProc->NotifyCallbk( pProc );
				
				return ProcessDisattach(pProc,pProcList);
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

void 
ProcessFree( struct Process *pa_pProc )
{
	ForEach(&pa_pProc->IProcessHead,&FreeProcStep,NULL);
}

BOOL 
ProcessDisattach( struct Process* pa_pProc , struct ProcessingList *pa_pProcList )
{
	if(pa_pProc->UndergoLN.next == &pa_pProc->UndergoLN)
		return 0;//not attached, do nothing.
	else
		return ListDragOneOut(&pa_pProcList->IUndergoProcess,&pa_pProc->UndergoLN);
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

	sprintf( BuffStr , "STEP%d" , pProcStep->FlagNum );
	strcat( PrintStr , BuffStr );
	strcat( PrintStr , ",");

	if( pProcStep->pName != NULL )
	{
		sprintf( BuffStr , "%s" , pProcStep->pName );
		strcat( PrintStr , BuffStr );
		strcat( PrintStr , ",");
	}
	
	sprintf( BuffStr , "%ld" , **pProcStep->ppWaitClocks );
	strcat( PrintStr , BuffStr );
	strcat( PrintStr , "*");
	
	sprintf( BuffStr , "%d" , **pProcStep->ppMaxRetrys );
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

static BOOL 
LIST_ITERATION_CALLBACK_FUNCTION( ProcessingListTraceCallbk )
{
	DEF_AND_CAST(pCount,int,pa_else);
	(*pCount) ++;
		
	return pa_pINow->now == pa_pIHead->last;
}

void
ProcessingListTrace(struct ProcessingList *pa_pProcList)
{
	//only implemented the counting function now.
	int count = 0;
	ForEach( &pa_pProcList->IUndergoProcess , &ProcessingListTraceCallbk , &count );
	printf("Processing %d processes. \n",count);
}

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION( FreeProcess )
{
	struct Process *pProc = GET_STRUCT_ADDR_FROM_IT( pa_pINow , struct Process , UndergoLN );

	tk( pa_pINow , pa_pIForward );

	printf("freeeeeeeing..\n");
			
	if( pProc->NotifyCallbk != NULL )
		pProc->NotifyCallbk( pProc );

	if( pa_pINow->now == pa_pIHead->last )
	{
		*pa_pIHead = GetIterator( NULL );
		return 1;
	}else
	{
		return 0;
	}
}

void 
ProcessListFree(struct ProcessingList * pa_pProcList)
{
	ForEach( &pa_pProcList->IUndergoProcess , &FreeProcess , NULL );
}
