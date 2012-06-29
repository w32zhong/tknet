#include "tknet.h"

#define PIPE_NAME_MAXLEN 25
//Now, the max len is defined by the 
//longest possible name string:
//"net123.123.123.123:12345\0"

#define PIPE_READ  0
#define PIPE_WRITE 1

struct pipe
{
	char  **ppBuff;
	uint  *pTrigger;
	char  name[PIPE_NAME_MAXLEN];
	BOOL  rw;
	struct ListNode ln;
	struct Iterator IDirection;
	struct Iterator IReference;
};

struct connection
{
	struct Iterator    *pIterator;
	struct connection  *pCounterPart;
	struct ListNode    ln;
};

struct PipeReference
{
	struct PipeDirection *pDirection;
	struct ListNode  ln;
};

struct FindPipeByName
{
	char        *name;
	struct pipe *found;
};

struct PipeMap
{
	struct Iterator IPipe;
};

static struct PipeMap sta_PipeMap;

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
	VCK(pa_pTo->rw != PIPE_WRITE , return 0;);

	CutConnections(&pa_pFrom->IDirection);
	MakeConnection(&pa_pFrom->IDirection,&pa_pTo->IReference);
	return 1;
}

BOOL
PipeDirectTo(struct pipe *pa_pFrom,struct pipe *pa_pTo)
{
	VCK(pa_pTo->rw != PIPE_WRITE , return 0;);

	MakeConnection(&pa_pFrom->IDirection,&pa_pTo->IReference);
	return 1;
}

void
PipeDele(struct pipe *pa_pPipe)
{
	CutConnections(&pa_pPipe->IDirection);
	CutConnections(&pa_pPipe->IReference);
	ListDragOneOut(&sta_PipeMap.IPipe,&pa_pPipe->ln);
	tkfree(pa_pPipe);
}

DEF_FREE_LIST_ELEMENT_CALLBACK_FUNCTION(PipeDeleCallbk,struct pipe,ln,
		CutConnections(&pNow->IDirection);
		CutConnections(&pNow->IReference);)

void 
PipeModuleUninit()
{
	ForEach(&sta_PipeMap.IPipe,&PipeDeleCallbk,NULL);
}

void 
PipeModuleInit()
{
	sta_PipeMap.IPipe  = GetIterator(NULL);
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
PipeMap(char *pa_pName,char **pa_ppBuff,uint *pa_pTrigger,BOOL pa_rw)
{
	struct pipe* pPipe = PipeFindByName(pa_pName);

	if(pPipe == NULL)
	{
		pPipe = tkmalloc(struct pipe);

		pPipe->ppBuff   = pa_ppBuff;
		pPipe->pTrigger = pa_pTrigger;
		strcpy(pPipe->name,pa_pName);
		pPipe->rw = pa_rw;

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
		printf("%s%s","NULL",end?".":",");
	}
	else
	{
		pDir = GET_STRUCT_ADDR(pC->pCounterPart->pIterator,struct pipe,IReference);
		printf("%s%s",pDir->name,end?".":",");
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
		printf("%s%s","NULL",end?".":",");
		return end;
	}
	else
	{
		pRef = GET_STRUCT_ADDR(pC->pCounterPart->pIterator,struct pipe,IDirection);
		printf("%s%s",pRef->name,end?".":",");
	}
		
	return end;
}

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION(PrintPipeCallbk)
{
	struct pipe *pPipe = GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct pipe,ln);

	printf("%s %s",pPipe->name,(pPipe->rw==PIPE_READ)?"r":"w");
	printf(" dir:");
	ForEach(&pPipe->IDirection,&PrintPipeDirectionsCallbk,NULL);
	printf(" ref:");
	ForEach(&pPipe->IReference,&PrintPipeReferencesCallbk,NULL);
	printf("\n");
		
	return pa_pINow->now == pa_pIHead->last;
}

void
PipeTablePrint()
{
	printf("Pipe table:\n");
	ForEach(&sta_PipeMap.IPipe,&PrintPipeCallbk,NULL);
	printf("\n");
}

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION(FlowToCallbk)
{
	struct connection *pC = GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct connection,ln);
	struct pipe *pDir = GET_STRUCT_ADDR(pC->pCounterPart->pIterator,struct pipe,IReference);
	DEF_AND_CAST(pFrom,struct pipe,pa_else);

	if(pDir->ppBuff && pDir->pTrigger)
	{
		memcpy(*pDir->ppBuff);
	}
	else
	//flow to a null pipe
	{
		;//do nothing
	}

	*pFrom->pTrigger = 0; //clear flag

	return pa_pINow->now == pa_pIHead->last;
}

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION(PipeFlowCallbk)
{
	struct pipe *pPipe = GET_STRUCT_ADDR_FROM_IT(pa_pINow,struct pipe,ln);
	
	if(pPipe->rw == PIPE_READ)
		ForEach(&pPipe->IDirection,&FlowToCallbk,pPipe);

	return pa_pINow->now == pa_pIHead->last;
}

void
PipeFlow()
{
	ForEach(&sta_PipeMap.IPipe,&PipeFlowCallbk,NULL);
}

int main()
{
	char buff[256],*pArg0,*pArg1;
	struct pipe *pPipe0,*pPipe1;
	uint i;
	char get_c;

	PipeModuleInit();

	while(1)
	{
		fgets(buff,255,stdin);
		buff[strlen(buff)-1] = '\0';
		
		pArg0 = strtok(buff," ");
		pArg1 = strtok(NULL," ");
		
		if(pArg0 == NULL)
			continue;

		if(pArg1)
		{
			if(strcmp(pArg0,"dele") == 0)
			{
				pPipe1 = PipeFindByName(pArg1);
				
				if(pPipe1)
				{
					PipeDele(pPipe1);
				}
				else
				{
					printf("can't find %s.\n",pArg1);
				}
			}
			else
			{
				pPipe0 = PipeMap(pArg0,(char**)&buff,&i,PIPE_WRITE);
				pPipe1 = PipeFindByName(pArg1);

				if(pPipe1)
				{
					printf("only? y/n \n");
					get_c = getchar();
					if(get_c=='y')
						PipeDirectOnlyTo(pPipe0,pPipe1);
					else
						PipeDirectTo(pPipe0,pPipe1);

					printf("\n");
				}
				else
				{
					printf("can't find %s.\n",pArg1);
				}
			}
		}
		else if(strcmp(pArg0,"exit") == 0)
		{
			break;
		}
		else
		{
			PipeMap(pArg0,(char**)&buff,&i,PIPE_READ);
		}

		PipeTablePrint();
	}

	PipeModuleUninit();
	printf("unfree memory:%d \n",g_allocs);
	return 0;
}
