#include "headers.h"

DEF_STRUCT_CONSTRUCTOR( ListNode ,
		out_cons->next = out_cons;
		out_cons->last = out_cons;
		)

__inline struct Iterator
GetIterator(struct ListNode* in_node)
{
	struct Iterator i;

	if(in_node == NULL)
	{
		i.now = i.last = NULL;
	}
	else
	{
		i.now  = in_node;
		i.last = in_node->last;
	}

	return i;
}

__inline void
tk(struct Iterator* i0 ,struct Iterator* i1)
{
	if(i0->now == NULL)
	{
		*i0 = *i1;
	}
	else if(i1->now == NULL)
	{
		*i1 = *i0;
	}

	i0->now->last  = i1->last;
	i0->last->next = i1->now;

	i1->now->last   = i0->last;
	i1->last->next  = i0->now;

	i0->last = i1->last;
	i1->last = i1->now->last;
}

void
AddOneToListTail(struct Iterator* io_IHead , struct ListNode* in_NodeOfAdding )
{
	struct Iterator IAdd = GetIterator( in_NodeOfAdding );
	tk( io_IHead , &IAdd );
}

void
AddOneToListHead(struct Iterator* io_IHead , struct ListNode* in_NodeOfAdding )
{
	AddOneToListTail( io_IHead , in_NodeOfAdding );
	*io_IHead = GetIterator( io_IHead->last );
}

void
ForEach(struct Iterator* io_IHead , ListNodeCallBack pa_node_callback ,void* pa_else )
{
	struct Iterator INow;
	struct Iterator IForward;
	INow = *io_IHead;

	if(io_IHead->now == NULL)
	{
		return;
	}

	while(1)
	{
		IForward = GetIterator( INow.now->next );

		if( pa_node_callback( io_IHead , &INow , &IForward , pa_else ) )
		{
			break;
		}

		INow = IForward;
	}
}

BOOL
LIST_ITERATION_CALLBACK_FUNCTION( SortingInsert )
{
	DEF_AND_CAST(pSip , struct SortingInsertPa , pa_else);

	if( pSip->si_cbk(pSip->pIInsert, pa_pINow , pSip->else_pa) )
	{
		tk(pSip->pIInsert , pa_pINow);

		if(pa_pINow->now == pa_pIHead->now)
		{
			*pa_pIHead = *(pSip->pIInsert);
		}

		return 1;

	}
	else if(pa_pINow->now == pa_pIHead->last)
	{
		tk(pSip->pIInsert,pa_pIHead);

		return 1;
	}
	else
	{
		return 0;
	}
}

struct SortListCallbackPa
{
	struct Iterator*		pNewList;
	struct SortingInsertPa*	pSip;
};

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION(SortListCallback)
{
	DEF_AND_CAST(pSlcbp , struct SortListCallbackPa , pa_else);
	tk( pa_pINow , pa_pIForward );

	if(pSlcbp->pNewList->now == NULL)
	{
		tk( pa_pINow , pSlcbp->pNewList );
	}
	else
	{
		pSlcbp->pSip->pIInsert = pa_pINow;
		ForEach( pSlcbp->pNewList , &SortingInsert , pSlcbp->pSip );
	}

	return pa_pINow->now == pa_pIHead->last;
}

void
SortList( struct Iterator* io_IHead , struct SortingInsertPa* in_sip)
{
	struct Iterator INewList = GetIterator(NULL);
	struct SortListCallbackPa slcbp;
	slcbp.pNewList = &INewList;
	slcbp.pSip = in_sip;

	ForEach( io_IHead , &SortListCallback , &slcbp );
	*io_IHead = INewList;
}

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION(ReverseListCallback)
{
	DEF_AND_CAST( pNewList , struct Iterator , pa_else );

	tk( pa_pINow , pa_pIForward );
	tk( pa_pINow , pNewList );
	*pNewList = *pa_pINow;

	return pa_pINow->now == pa_pIHead->last;
}

void
ReverseList( struct Iterator* io_IHead)
{
	struct Iterator INewList = GetIterator(NULL);

	ForEach( io_IHead , &ReverseListCallback , &INewList);
	*io_IHead = INewList;
}

BOOL
LIST_ITERATION_CALLBACK_FUNCTION( CleanedOutOfList )
{
	tk( pa_pINow , pa_pIForward ); 
	if( pa_pINow->now == pa_pIHead->last ) 
	{ 
		*pa_pIHead = GetIterator( NULL ); 
		return 1; 
	} else 
	{ 
		return 0; 
	} 
}


