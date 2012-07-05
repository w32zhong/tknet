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

static struct PipeMap sta_PipeMap;
static uint           sta_PipeIDSeed = 0;

void
MakeConnection(struct Iterator *pa_pI0,struct Iterator *pa_pI1)
{
	struct connection 
		*pC0 = tkmalloc(struct connection),
		*pC1 = tkmalloc(struct connection);

	pC0->pIterator = pa_pI0;
	pC0->pCounterPart = pC1;
	ListNodeCons(&pC0->ln);
	
	pC1->pIterator = pa_pI1;
	pC1->pCounterPart = pC0;
	ListNodeCons(&pC1->ln);

	AddOneToListTail(pa_pI0,&pC0->ln);
	AddOneToListTail(pa_pI1,&pC1->ln);
}

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION(CutConnectionCallbk)
{
	struct connection *pC0 = GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct connection,ln),
					  *pC1 = pC0->pCounterPart;

	ListDragOneOut(pC1->pIterator,&pC1->ln);
	pC0->pIterator = NULL;
	pC0->pCounterPart = NULL;
	tkfree(pC1);
		
	return pa_pINow->now == pa_pIHead->last;
}

static
DEF_FREE_LIST_ELEMENT_CALLBACK_FUNCTION(FreeConnectionCallbk , struct connection , ln , ;)

void
CutConnections(struct Iterator *pa_pI)
{
	ForEach(pa_pI,&CutConnectionCallbk,NULL);
	ForEach(pa_pI,&FreeConnectionCallbk,NULL);
}

BOOL
PipeDirectOnlyTo(struct pipe *pa_pFrom,struct pipe *pa_pTo)
{
	CutConnections(&pa_pFrom->IDirection);
	MakeConnection(&pa_pFrom->IDirection,&pa_pTo->IReference);
	return 1;
}

BOOL
PipeDirectTo(struct pipe *pa_pFrom,struct pipe *pa_pTo)
{
	MakeConnection(&pa_pFrom->IDirection,&pa_pTo->IReference);
	return 1;
}

void
PipeDele(struct pipe *pa_pPipe)
{
	CutConnections(&pa_pPipe->IDirection);
	CutConnections(&pa_pPipe->IReference);
	ListDragOneOut(&sta_PipeMap.IPipe,&pa_pPipe->ln);
	if(pa_pPipe->pFlowPa)
		tkfree(pa_pPipe->pFlowPa);
	tkfree(pa_pPipe);
}

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION(ResetPipeCallbk)
{
	struct pipe *pPipe = GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct pipe,ln);

	CutConnections(&pPipe->IDirection);
	CutConnections(&pPipe->IReference);
		
	return pa_pINow->now == pa_pIHead->last;
}

void 
PipeReset()
{
	ForEach(&sta_PipeMap.IPipe,&ResetPipeCallbk,NULL);
}

void 
PipeModuleInit()
{
	sta_PipeMap.IPipe  = GetIterator(NULL);
}

DEF_FREE_LIST_ELEMENT_CALLBACK_FUNCTION(PipeDeleCallbk,struct pipe,ln,
		CutConnections(&pNow->IDirection);
		CutConnections(&pNow->IReference);
		if(pNow->pFlowPa)
			tkfree(pNow->pFlowPa);
		)

void 
PipeModuleUninit()
{
	ForEach(&sta_PipeMap.IPipe,&PipeDeleCallbk,NULL);
}

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION(FindPipeByNameCallbk)
{
	struct pipe *pPipe = GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct pipe,ln);
	DEF_AND_CAST(pFpbnpa,struct FindPipeByName ,pa_else);

	if( strcmp(pFpbnpa->name,pPipe->name) == 0 )
	{
		pFpbnpa->found = pPipe;
		return 1;
	}
	else
	{
		return pa_pINow->now == pa_pIHead->last;
	}
}

struct pipe*
PipeFindByName(char *pa_pName)
{
	struct FindPipeByName fpbnpa;
	fpbnpa.found = NULL;
	fpbnpa.name  = pa_pName;
		
	ForEach( &sta_PipeMap.IPipe , &FindPipeByNameCallbk , &fpbnpa );
	return fpbnpa.found;
}

struct pipe*
PipeMap(char *pa_pName)
{
	struct pipe* pPipe = PipeFindByName(pa_pName);

	if(pPipe == NULL)
	{
		pPipe = tkmalloc(struct pipe);
		strcpy(pPipe->name,pa_pName);
		pPipe->FlowCallbk = NULL;
		pPipe->pFlowPa = NULL;
		pPipe->id = ++ sta_PipeIDSeed;

		ListNodeCons(&pPipe->ln);
		pPipe->IDirection = GetIterator(NULL);
		pPipe->IReference = GetIterator(NULL);

		AddOneToListTail(&sta_PipeMap.IPipe,&pPipe->ln);
	}

	return pPipe;
}

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION(PrintPipeDirectionsCallbk)
{
	struct connection *pC = GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct connection,ln);
	struct pipe *pDir;
	BOOL end;
	
	if(pa_pINow->now == pa_pIHead->last)
		end = 1;
	else
		end = 0;

	if(pC->pCounterPart == NULL)
	{
		PROMPT(Usual,"%s%s","NULL",end?".":",");
	}
	else
	{
		pDir = GET_STRUCT_ADDR(pC->pCounterPart->pIterator,struct pipe,IReference);
		PROMPT(Usual,"%s%s",pDir->name,end?".":",");
	}
		
	return end;
}

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION(PrintPipeReferencesCallbk)
{
	struct connection *pC = GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct connection,ln);
	struct pipe *pRef;
	BOOL end;
	
	if(pa_pINow->now == pa_pIHead->last)
		end = 1;
	else
		end = 0;

	if(pC->pCounterPart == NULL)
	{
		PROMPT(Usual,"%s%s","NULL",end?".":",");
		return end;
	}
	else
	{
		pRef = GET_STRUCT_ADDR(pC->pCounterPart->pIterator,struct pipe,IDirection);
		PROMPT(Usual,"%s%s",pRef->name,end?".":",");
	}
		
	return end;
}

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION(PrintPipeCallbk)
{
	struct pipe *pPipe = GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct pipe,ln);

	PROMPT(Usual,"%d: %s",pPipe->id,pPipe->name);
	PROMPT(Usual," dir:");
	ForEach(&pPipe->IDirection,&PrintPipeDirectionsCallbk,NULL);
	PROMPT(Usual," ref:");
	ForEach(&pPipe->IReference,&PrintPipeReferencesCallbk,NULL);
	PROMPT(Usual,"\n");
		
	return pa_pINow->now == pa_pIHead->last;
}

void
PipeTablePrint()
{
	PROMPT(Usual,"Pipe table:\n");
	ForEach(&sta_PipeMap.IPipe,&PrintPipeCallbk,NULL);
	PROMPT(Usual,"\n");
}

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION(FlowToCallbk)
{
	struct connection *pC = GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct connection,ln);
	struct pipe *pTo = GET_STRUCT_ADDR(pC->pCounterPart->pIterator,struct pipe,IReference);
	DEF_AND_CAST(pFcbkpa,struct FlowCallbkPa,pa_else);

	if(pTo->FlowCallbk)
	{
		pTo->FlowCallbk(pFcbkpa->pData,pFcbkpa->DataLen,
				pTo,pTo->pFlowPa,pFcbkpa->Else);
	}
	else
	//flow to a null-out pipe
	{
		;//do nothing
	}

	return pa_pINow->now == pa_pIHead->last;
}

void
PipeFlow(struct pipe *pa_pPipe,char *pa_pData,uint pa_DataLen,void *pa_else)
{
	struct FlowCallbkPa fcbkpa;
	fcbkpa.pData = pa_pData;
	fcbkpa.DataLen = pa_DataLen;
	fcbkpa.Else = pa_else;

	ForEach(&pa_pPipe->IDirection,&FlowToCallbk,&fcbkpa);
}

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION(ifPipeToCallbk)
{
	struct connection *pC = GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct connection,ln);
	struct pipe *pDir = GET_STRUCT_ADDR(pC->pCounterPart->pIterator,struct pipe,IReference);
	DEF_AND_CAST(pIptPa,struct IfPipeToPa,pa_else);

	if(pDir == pIptPa->pTo)
	{
		pIptPa->res = 1;
		return 1;
	}
	else
		return pa_pINow->now == pa_pIHead->last;
}

BOOL 
ifPipeTo(struct pipe *pa_pFrom,struct pipe *pa_pTo)
{
	struct IfPipeToPa iptpa;
	iptpa.res   = 0;
	iptpa.pTo   = pa_pTo;

	ForEach(&pa_pFrom->IDirection,&ifPipeToCallbk,&iptpa);
	return iptpa.res;
}

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION(FindPipeByIDCallbk)
{
	struct pipe *pPipe = GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct pipe,ln);
	DEF_AND_CAST(pFpbipa,struct FindPipeByID ,pa_else);

	if( pFpbipa->id == pPipe->id )
	{
		pFpbipa->found = pPipe;
		return 1;
	}
	else
	{
		return pa_pINow->now == pa_pIHead->last;
	}
}

struct pipe*
PipeFindByID(uint pa_ID)
{
	struct FindPipeByID fpbipa;
	fpbipa.found = NULL;
	fpbipa.id    = pa_ID;
		
	ForEach( &sta_PipeMap.IPipe , &FindPipeByIDCallbk , &fpbipa );
	return fpbipa.found;
}
