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

DEF_STRUCT_CONSTRUCTOR( TreeNode ,
		out_cons->pBranch = NULL;
		out_cons->pFather = NULL;
		out_cons->IChildren = GetIterator( NULL );
		)

void
AttachTo(struct TreeNode* pa_pFrom ,struct TreeNode* pa_pTo )
{
	struct Branch* pNewBranch;

	if(pa_pFrom->pFather != NULL)
	{
		return;
	}

	pNewBranch = tkmalloc( struct Branch );
	ListNodeCons(&pNewBranch->ln);
	pNewBranch->pChild = pa_pFrom;

	AddOneToListTail( &pa_pTo->IChildren , &pNewBranch->ln );

	pa_pFrom->pFather = pa_pTo;
	pa_pFrom->pBranch = pNewBranch;
}

static
DEF_FREE_LIST_ELEMENT_SAFE_FUNCTION( FreeBranchFromList ,struct Branch,ln, ;)

void
Disattach(struct TreeNode* pa_pTn)
{
	struct Iterator IBranch , INextBranch;
	
	if(pa_pTn->pFather == NULL)
	{
		return;
	}
	
	IBranch = GetIterator( &pa_pTn->pBranch->ln );
	INextBranch = GetIterator( IBranch.now->next );

	FreeBranchFromList( &pa_pTn->pFather->IChildren ,
						&IBranch ,
						&INextBranch ,
						NULL);

	pa_pTn->pFather = NULL;
	pa_pTn->pBranch = NULL;
}

struct TreeSearchPa
{
	ListNodeCallBack  ln_cbk;
	void*             else_pa;
};

BOOL
LIST_ITERATION_CALLBACK_FUNCTION( PreorderDFS )
{
	struct Branch* pBranch = GET_STRUCT_ADDR_FROM_IT( pa_pINow ,struct Branch ,ln );
	struct TreeNode* pChild = pBranch->pChild;

	DEF_AND_CAST( pTspa , struct TreeSearchPa , pa_else );

	BOOL res = pTspa->ln_cbk( pa_pIHead , pa_pINow , pa_pIForward , pTspa->else_pa );

	ForEach( &pChild->IChildren , &PreorderDFS , pa_else );

	return res;
}

BOOL
LIST_ITERATION_CALLBACK_FUNCTION( PostorderDFS )
{
	struct Branch* pBranch = GET_STRUCT_ADDR_FROM_IT( pa_pINow ,struct Branch ,ln );
	struct TreeNode* pChild = pBranch->pChild;

	DEF_AND_CAST( pTspa , struct TreeSearchPa , pa_else );

	ForEach( &pChild->IChildren , &PostorderDFS , pa_else );

	return pTspa->ln_cbk( pa_pIHead , pa_pINow , pa_pIForward , pTspa->else_pa );
}

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION( CopyBranchesCallback )
{
	struct Branch* pBranch = GET_STRUCT_ADDR_FROM_IT( pa_pINow ,struct Branch ,ln );
	
	DEF_AND_CAST( pITo , struct Iterator , pa_else );
	
	struct Branch* pNewBranch = tkmalloc( struct Branch );

	pNewBranch->pChild = pBranch->pChild;
	ListNodeCons( &pNewBranch->ln );

	AddOneToListTail( pITo , &pNewBranch->ln );

	return pa_pINow->now == pa_pIHead->last;
}

BOOL
LIST_ITERATION_CALLBACK_FUNCTION( BFS )
{
	struct Branch* pBranch = GET_STRUCT_ADDR_FROM_IT( pa_pINow ,struct Branch ,ln );
	struct TreeNode* pChild = pBranch->pChild;

	DEF_AND_CAST( pTspa , struct TreeSearchPa , pa_else );

	ForEach( &pChild->IChildren , &CopyBranchesCallback , pa_pIHead );
	
	*pa_pINow = GetIterator( pa_pINow->now );
	*pa_pIForward = GetIterator( pa_pINow->now->next );

	pTspa->ln_cbk( pa_pINow  , pa_pINow , pa_pIForward , pTspa->else_pa );

	tk( pa_pINow , pa_pIForward );
	tkfree(pBranch);
	
	*pa_pIHead = *pa_pIForward;
	//becaue we will use pa_pIHead to add branches to queue on
	//next iteration, we need to make it point to a right position.

	return pa_pINow->now == pa_pIForward->now;
}

void 
Traversal(struct TreeNode* pa_pRoot , ListNodeCallBack pa_TraversalWay , ListNodeCallBack pa_NodeCallback , void* pa_else )
{
	struct TreeSearchPa tspa;
	struct Iterator IQueue = GetIterator( NULL );
	tspa.ln_cbk = pa_NodeCallback;
	tspa.else_pa = pa_else;

	if( pa_TraversalWay == &BFS)
	{
		ForEach( &pa_pRoot->IChildren , &CopyBranchesCallback , &IQueue );
		ForEach( &IQueue , &BFS , &tspa );
	}
	else
	{
		ForEach( &pa_pRoot->IChildren , pa_TraversalWay , &tspa );
	}
}

DEF_STRUCT_CONSTRUCTOR( BinTreeNode ,
		out_cons->LeftChild = NULL;
		out_cons->RightChild = NULL;
		TreeNodeCons(&out_cons->tnd);
		)

void 
BinAttachTo(struct BinTreeNode* pa_pFrom , struct BinTreeNode* pa_pTo , BOOL pa_ifRight)
{
	if( pa_ifRight )
	{
		if(pa_pTo->RightChild)
		{
			return ;
		}
		pa_pTo->RightChild = pa_pFrom;
	}
	else
	{
		if(pa_pTo->LeftChild)
		{
			return ;
		}
		pa_pTo->LeftChild = pa_pFrom;

	}

	AttachTo(&pa_pFrom->tnd,&pa_pTo->tnd);
}

void 
BinDisattach(struct BinTreeNode* pa_pBtn)
{
	struct BinTreeNode *pBinFather;

	if( pa_pBtn->tnd.pFather == NULL )
	{
		return ;
	}

	pBinFather = 
	GET_STRUCT_ADDR( pa_pBtn->tnd.pFather , struct BinTreeNode , tnd );

	if( pBinFather->RightChild == pa_pBtn )
	{
		pBinFather->RightChild = NULL;
	}
	else if( pBinFather->LeftChild == pa_pBtn )
	{
		pBinFather->LeftChild = NULL;
	}
	
	Disattach( &pa_pBtn->tnd );
}

void 
BinTreeInsert(struct BinTreeNode* pa_pFrom , struct BinTreeNode* pa_pTo ,BinTreeCompareCallback pa_cmp , void* pa_else)
{
	BOOL CompRes = pa_cmp( pa_pFrom , pa_pTo , pa_else );

	if( CompRes )
	{
		if( pa_pTo->RightChild )
		{
			BinTreeInsert(pa_pFrom, pa_pTo->RightChild , pa_cmp ,pa_else);
			return;
		}
	}
	else
	{
		if( pa_pTo->LeftChild )
		{
			BinTreeInsert(pa_pFrom, pa_pTo->LeftChild , pa_cmp ,pa_else);
			return;
		}
	}
	
	BinAttachTo(pa_pFrom , pa_pTo , CompRes);
}

struct BinTreeNode*
BinTreeFind(struct BinTreeNode* pa_pBtn , struct BinTreeNode* pa_pFind , BinTreeCompareCallback pa_cmp ,BinTreeCompareCallback pa_CmpEq , void* pa_else)
{
	BOOL CompRes = pa_CmpEq( pa_pBtn , pa_pFind , pa_else );
	
	if( CompRes )
	{
		return pa_pBtn;
	}

	CompRes = pa_cmp( pa_pFind , pa_pBtn , pa_else );

	if( CompRes )
	{
		if( pa_pBtn->RightChild )
		{
			return BinTreeFind( pa_pBtn->RightChild , pa_pFind , pa_cmp , pa_CmpEq , pa_else );
		}
	}
	else
	{
		if( pa_pBtn->LeftChild )
		{
			return BinTreeFind( pa_pBtn->LeftChild , pa_pFind , pa_cmp , pa_CmpEq , pa_else );
		}
	}

	return NULL;
}

static void 
_BinTreeInorderTraversal(struct BinTreeNode* pa_pBtn , ListNodeCallBack pa_NodeCallback , void* pa_else )
{
	struct Iterator INow = GetIterator( &(pa_pBtn->tnd.pBranch->ln) );
	struct Iterator IForward = GetIterator( INow.now->next );

	if( pa_pBtn->LeftChild )
	{
		_BinTreeInorderTraversal(pa_pBtn->LeftChild , pa_NodeCallback , pa_else );
	}

	pa_NodeCallback( &(pa_pBtn->tnd.pFather->IChildren) , &INow , &IForward , pa_else );

	if( pa_pBtn->RightChild )
	{
		_BinTreeInorderTraversal(pa_pBtn->RightChild , pa_NodeCallback , pa_else );
	}
}

void 
BinTreeInorderTraversal(struct BinTreeNode* pa_pRoot , ListNodeCallBack pa_NodeCallback , void* pa_else )
{
	struct Branch*       pBranch ;
	struct BinTreeNode*  pChildBinTree;

	if( pa_pRoot->tnd.IChildren.now == NULL )
	{
		return;
	}

	pBranch = GET_STRUCT_ADDR( pa_pRoot->tnd.IChildren.now , struct Branch , ln );
	pChildBinTree = GET_STRUCT_ADDR( pBranch->pChild , struct BinTreeNode , tnd );

	_BinTreeInorderTraversal( pChildBinTree , pa_NodeCallback , pa_else );
}

struct LevelStackEle
{
	uint      level;
	struct ListNode  ln;
};

struct GetLevelPa
{
	uint level;
	struct Iterator LevelStack;
	SetLevelCallback slcbk;
};

#define IS_LIST_END (pa_pINow->now == pa_pIHead->last)
#define IS_DEPTH_END (pBranch->pChild->IChildren.now == NULL)

DEF_FREE_LIST_ELEMENT_SAFE_FUNCTION(PopLevelStack,struct LevelStackEle ,ln, ;)

static BOOL
LIST_ITERATION_CALLBACK_FUNCTION( GetLevelCallback )
{
	struct Branch* pBranch =
		GET_STRUCT_ADDR_FROM_IT( pa_pINow , struct Branch , ln);

	DEF_AND_CAST( pGlpa , struct GetLevelPa , pa_else );

	struct LevelStackEle *pLse;
	struct Iterator IForward,*pIHead;

	if( pa_pINow->now == pa_pIHead->now )
	{
		pGlpa->level ++;
	}

	pGlpa->slcbk( pBranch->pChild , pGlpa->level );

	if( !IS_LIST_END && !IS_DEPTH_END )
	{
		pLse = tkmalloc( struct LevelStackEle );
		ListNodeCons( &pLse->ln );
		pLse->level = pGlpa->level;
		AddOneToListHead( &pGlpa->LevelStack , &pLse->ln );
	}
	else if( IS_LIST_END && IS_DEPTH_END )
	{
		pIHead = &pGlpa->LevelStack;
		if(pIHead->now)
		{
			pLse = GET_STRUCT_ADDR_FROM_IT(pIHead,struct LevelStackEle,ln);
			pGlpa->level = pLse->level;
			IForward = GetIterator( pIHead->now->next );
			PopLevelStack( pIHead ,pIHead ,&IForward , NULL);
		}
	}

	return pa_pINow->now == pa_pIHead->last;
}

void 
TreeGetNodesLevel(struct TreeNode* pa_pRoot , SetLevelCallback pa_slcbk)
{
	struct GetLevelPa glpa;
	glpa.level = 0;
	glpa.LevelStack = GetIterator( NULL );
	glpa.slcbk = pa_slcbk;
	Traversal(pa_pRoot,&PreorderDFS,&GetLevelCallback,&glpa);
}
