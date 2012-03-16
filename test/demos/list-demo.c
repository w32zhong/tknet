#include "headers.h"
#include <stdlib.h>
#include <stdio.h>

struct I
{
	int i;
	struct ListNode sajfkjh;
};

DEF_STRUCT_CONSTRUCTOR( I ,
		out_cons-> i = rand() % 12;
		ListNodeCons( &( out_cons->sajfkjh ) );
		)

BOOL
compare(struct Iterator* compared0 , struct Iterator* compared1 , void* pa_else)
{
	struct I* p0 = NULL;
	struct I* p1 = NULL;

	p0 = GET_STRUCT_ADDR_FROM_IT( compared0 , struct I , sajfkjh );
	p1 = GET_STRUCT_ADDR_FROM_IT( compared1 , struct I , sajfkjh );

	return p0->i < p1->i ;
}

BOOL
LIST_ITERATION_CALLBACK_FUNCTION(trace)
{
	struct I* p = GET_STRUCT_ADDR_FROM_IT( pa_pINow , struct I , sajfkjh );

	printf("%d->",p->i);

	return pa_pINow->now == pa_pIHead->last;
}

static
DEF_FREE_LIST_ELEMENT_CALLBACK_FUNCTION( FreeI , struct I, sajfkjh , printf("dele %d ,",pNow->i); )

static
DEF_FREE_LIST_ELEMENT_SAFE_FUNCTION( FreeIs , struct I, sajfkjh , printf("dele %d :\n",pNow->i);
		ForEach(pa_pIHead,&trace,NULL);
		printf(" that's all. \n");
		)

int main()
{
	int seed,res,i = 0;
	struct Iterator i_list = GetIterator(NULL);
	struct I* add;
	struct SortingInsertPa sip;
	sip.si_cbk = &compare;

	printf("random seed \n");
	res = scanf("%d", &seed);
	srand(seed + 12745);

	for( i = 0 ; i < 9 ; i++ )
	{
		add = (struct I*)malloc( sizeof (struct I) );
		ICons(add);
		AddOneToListHead( &i_list, &(add->sajfkjh) );
    	printf("add %d ,",add->i);
	}
	printf(" that's all. \n");

	ForEach(&i_list,&trace,NULL);
	printf(" that's all. \n");

	ReverseList(&i_list);
	ForEach(&i_list,&trace,NULL);
	printf(" that's all. \n");

	SortList( &i_list , &sip);
	ForEach(&i_list,&trace,NULL);
	printf(" that's all. \n");

	//ForEach(&i_list,&FreeI,NULL);
	ForEach(&i_list,&FreeIs,NULL);
	printf(" that's all... \n");

	return 0;
}
