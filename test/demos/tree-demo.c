#include "headers.h"
#include <stdio.h>

struct Test
{
	int level;
	int i;
	struct BinTreeNode btnd;
};

BOOL test_compare(struct BinTreeNode* pa_bigger,
		struct BinTreeNode* pa_than, void* pa_else)
{
	struct Test* p0 = GET_STRUCT_ADDR( pa_bigger , struct Test , btnd );
	struct Test* p1 = GET_STRUCT_ADDR( pa_than , struct Test , btnd );
	return p0->i > p1->i;
}

BOOL test_eq(struct BinTreeNode* pa_bigger,
		struct BinTreeNode* pa_than, void* pa_else)
{
	struct Test* p0 = GET_STRUCT_ADDR( pa_bigger , struct Test , btnd );
	struct Test* p1 = GET_STRUCT_ADDR( pa_than , struct Test , btnd );
	return p0->i == p1->i;
}

BOOL
LIST_ITERATION_CALLBACK_FUNCTION( TraceBinTree )
{
	struct Branch* pBranch =
		GET_STRUCT_ADDR_FROM_IT( pa_pINow , struct Branch , ln);

	struct Test* p =
		GET_STRUCT_ADDR( pBranch->pChild , struct Test , btnd.tnd);

	int i;
	for(i=0;i<p->level;i++)
	{
		printf("  ");
	}
	printf("%d at level%d \n", p->i ,p->level );

	return pa_pINow->now == pa_pIHead->last;
}

DEF_FREE_TREE_ELEMENT_CALLBACK_FUNCTION( FreeMeAgain , struct Test , btnd.tnd , printf("dele %d \n", pTreeNow->i); )

void SetLevel(struct TreeNode* pa_pTnd , int pa_level)
{
	struct Test *pTest = GET_STRUCT_ADDR( pa_pTnd , struct Test , btnd.tnd );
	pTest->level = pa_level;
}

int main()
{
	struct Test *p[7];
	struct Test root;
	struct Test* pFindTest;
	struct BinTreeNode* pFindBTN;
	struct Test find;
	BinTreeNodeCons(&root.btnd);
	int i;
	for(i=0;i<7;i++)
	{
		p[i] = tkmalloc(struct Test);
		p[i]->i = i;
		p[i]->level = 0;
		BinTreeNodeCons(&(p[i]->btnd));
	}

	printf("allocs=%d\n",g_allocs);

	BinAttachTo(&(p[3]->btnd),&(root.btnd),0);

	BinTreeInsert(&(p[1]->btnd),&(p[3]->btnd),&test_compare,NULL);
	BinTreeInsert(&(p[5]->btnd),&(p[3]->btnd),&test_compare,NULL);
	BinTreeInsert(&(p[0]->btnd),&(p[3]->btnd),&test_compare,NULL);
	BinTreeInsert(&(p[2]->btnd),&(p[3]->btnd),&test_compare,NULL);
	BinTreeInsert(&(p[4]->btnd),&(p[3]->btnd),&test_compare,NULL);
	BinTreeInsert(&(p[6]->btnd),&(p[3]->btnd),&test_compare,NULL);

	TreeGetNodesLevel(&root.btnd.tnd,&SetLevel);

	find.i = 3;
	pFindBTN = BinTreeFind(&(p[3]->btnd),&find.btnd,&test_compare,&test_eq,NULL);
	if(pFindBTN)
	{
		pFindTest = GET_STRUCT_ADDR(pFindBTN,struct Test , btnd);
		printf("find %d at level%d \n", pFindTest->i ,pFindTest->level );
	}
	else
	{
		printf("not find\n");
	}
	
	Traversal(&(root.btnd.tnd),&PreorderDFS,&TraceBinTree,NULL);
	BinDisattach(&(p[0]->btnd));
	printf("--- --- -- -- \n\n");
	Traversal(&(root.btnd.tnd),&PreorderDFS,&TraceBinTree,NULL);
	BinAttachTo(&(p[0]->btnd),&(p[1]->btnd),0);

	printf("Inorder:\n");
	BinTreeInorderTraversal(&(root.btnd),&TraceBinTree,NULL);
	printf("PreorderDFS:\n");
	Traversal(&(root.btnd.tnd),&PreorderDFS,&TraceBinTree,NULL);
	printf("PostorderDFS:\n");
	Traversal(&(root.btnd.tnd),&PostorderDFS,&TraceBinTree,NULL);
	printf("BFS:\n");
	Traversal(&(root.btnd.tnd),&BFS,&TraceBinTree,NULL);

	Traversal(&(root.btnd.tnd),&PostorderDFS,&FreeMeAgain,NULL);

	printf("allocs=%d\n",g_allocs);

	return 0;
}
