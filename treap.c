
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

DEF_STRUCT_CONSTRUCTOR( Treap ,
		out_cons->RanPriority = tkGetRandom();
		BinTreeNodeCons(&out_cons->btnd);
		)

void 
TreapRoateUp(struct Treap* pa_pTp)
{
	struct BinTreeNode *pFather,*pGrandpa,*pSave = NULL;
	BOOL Pos,FatherPos;

	if( pa_pTp->btnd.tnd.pFather == NULL )
	{
		return ;
	}

	pFather = GET_STRUCT_ADDR( pa_pTp->btnd.tnd.pFather , struct BinTreeNode , tnd );
	Pos = ( pFather->RightChild == &pa_pTp->btnd );
	BinDisattach(&pa_pTp->btnd);

	if( pFather->tnd.pFather )
	{
		pGrandpa = GET_STRUCT_ADDR( pFather->tnd.pFather , struct BinTreeNode , tnd );
		FatherPos = ( pGrandpa->RightChild == pFather );
		BinDisattach(pFather);
	
		BinAttachTo( &pa_pTp->btnd , pGrandpa , FatherPos );
	}

	if( Pos )
	{
		pSave = pa_pTp->btnd.LeftChild;
	}
	else
	{
		pSave = pa_pTp->btnd.RightChild;
	}

	if( pSave )
	{
		BinDisattach( pSave );
		BinAttachTo( pSave , pFather , Pos );
	}

	BinAttachTo( pFather , &pa_pTp->btnd , !Pos );
}

void
TreapInsert(struct Treap* pa_pFrom , struct Treap* pa_pTo , BinTreeCompareCallback pa_cmp , void* pa_else)
{
	struct TreeNode *pFather;
	struct Treap *pFatherTp;

	BinTreeInsert( &pa_pFrom->btnd , &pa_pTo->btnd , pa_cmp , pa_else );

	while(1)
	{
		pFather = pa_pFrom->btnd.tnd.pFather;
		if( pFather )
		{
			pFatherTp = GET_STRUCT_ADDR( pFather , struct Treap , btnd.tnd );
			if( pa_pFrom->RanPriority < pFatherTp->RanPriority )
			{
				TreapRoateUp( pa_pFrom );
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
}

void 
TreapDragOut( struct Treap* pa_pTp )
{
	struct Treap *pTp;
	while(1)
	{
		if( pa_pTp->btnd.RightChild )
		{
			pTp = GET_STRUCT_ADDR( pa_pTp->btnd.RightChild , struct Treap , btnd );
			TreapRoateUp( pTp );
		}
		else if( pa_pTp->btnd.LeftChild )
		{
			pTp = GET_STRUCT_ADDR( pa_pTp->btnd.LeftChild , struct Treap , btnd );
			TreapRoateUp( pTp );
		}
		else
		{
			break;
		}
	}

	BinDisattach( &pa_pTp->btnd );
}
