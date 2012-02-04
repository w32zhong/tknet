#include "headers.h"
#include <stdio.h>

struct Test
{
	uint i;
	uint level;
	struct	Treap trp;
};

BOOL Test_compare(struct BinTreeNode* pa_bigger,
		struct BinTreeNode* pa_than, void* pa_else)
{
	struct Test* p0 = GET_STRUCT_ADDR( pa_bigger , struct Test , trp.btnd );
	struct Test* p1 = GET_STRUCT_ADDR( pa_than , struct Test , trp.btnd );
	return p0->i > p1->i;
}

BOOL Test_eq(struct BinTreeNode* pa_bigger,
		struct BinTreeNode* pa_than, void* pa_else)
{
	struct Test* p0 = GET_STRUCT_ADDR( pa_bigger , struct Test , trp.btnd );
	struct Test* p1 = GET_STRUCT_ADDR( pa_than , struct Test , trp.btnd );
	return p0->i == p1->i;
}

BOOL
LIST_ITERATION_CALLBACK_FUNCTION( TraceTreap )
{
	struct Branch* pBranch =
		GET_STRUCT_ADDR_FROM_IT( pa_pINow , struct Branch , ln);

	struct Test* p =
		GET_STRUCT_ADDR( pBranch->pChild , struct Test , trp.btnd.tnd);

	int i;char c;
	struct BinTreeNode *pBinFather;
	if(p->level < 32)
	{
		for(i=0;i<p->level;i++)
		{
			printf("    ");
		}
	}
	
	pBinFather = GET_STRUCT_ADDR(p->trp.btnd.tnd.pFather, struct BinTreeNode , tnd);
	if(pBinFather->RightChild == &(p->trp.btnd))
	{
		c='R';
	}
	else if(pBinFather->LeftChild == &(p->trp.btnd))
	{
		c='L';
	}
	else
	{
		c='x';
	}

	printf("%d %c(%d) at level%d \n", p->i ,c, p->trp.RanPriority ,p->level );

	return pa_pINow->now == pa_pIHead->last;
}

DEF_FREE_TREE_ELEMENT_CALLBACK_FUNCTION( FreeTreap , struct Test , trp.btnd.tnd , printf("dele %d \n", pTreeNow->i); )

void SetLevel(struct TreeNode* pa_pTnd , uint pa_level)
{
	struct Test *pTest = 
		GET_STRUCT_ADDR( pa_pTnd , struct Test , trp.btnd.tnd );

	pTest->level = pa_level;
}

#define MAX_NUM 9
int main()
{
	struct Test root,*pFindTest,ToFind,*p[MAX_NUM];
	struct BinTreeNode *pFind;
	uint i;
	tkInitRandom();

	TreapCons(&root.trp);
	root.i = 0;
	root.trp.RanPriority = 0;

	for(i=0;i<MAX_NUM;i++)
	{
		p[i] = tkmalloc(struct Test);
		p[i]->level = 0;
		TreapCons(&(p[i]->trp));
		
		while(1)
		{
			p[i]->i = tkGetRandom() % (MAX_NUM * 5);
			ToFind.i = p[i]->i;
			pFind = BinTreeFind(&root.trp.btnd,&ToFind.trp.btnd,&Test_compare,&Test_eq,NULL);
			if(pFind == NULL)
			{
				break;
			}
		}
		TreapInsert(&(p[i]->trp) ,&(root.trp) , Test_compare , NULL);
	}

	while(1)
	{
		printf("allocs = %d \n",g_allocs);

		TreeGetNodesLevel(&(root.trp.btnd.tnd),&SetLevel);
		Traversal(&(root.trp.btnd.tnd),&PreorderDFS,&TraceTreap,NULL);
		printf("--------------\n");
		BinTreeInorderTraversal( &(root.trp.btnd) ,&TraceTreap,NULL);

		printf("\n Input the num to drag out \n");
		scanf("%d",&i);

		if( i == 0 )
		{
			break;
		}

		ToFind.i = i;
		pFind = BinTreeFind(&root.trp.btnd,&ToFind.trp.btnd,&Test_compare,&Test_eq,NULL);
		if(pFind == NULL)
		{
			printf("unable to find. \n");
			continue;
		}
		
		pFindTest = GET_STRUCT_ADDR( pFind , struct Test , trp.btnd );
		TreapDragOut( &pFindTest->trp );
		tkfree(pFindTest);
	}
		
	printf("allocs = %d \n",g_allocs);
	printf("END. \n");
	return 0;
}
