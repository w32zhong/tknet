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

#define DEF_FREE_TREE_ELEMENT_CALLBACK_FUNCTION( _name , _type_tag , _member_name , _other_statments ) \
			DEF_FREE_LIST_ELEMENT_CALLBACK_FUNCTION( _name , struct Branch , ln , \
			_type_tag * pTreeNow = GET_STRUCT_ADDR( pNow->pChild , _type_tag , _member_name ); \
			_other_statments \
			tkfree(pTreeNow); \
			)

struct TreeNode;

struct Branch
{
	struct TreeNode* pChild;
	struct ListNode  ln;
};

struct TreeNode
{
	struct Branch	*pBranch;
	struct TreeNode	*pFather;
	struct Iterator IChildren;
};

DECLARATION_STRUCT_CONSTRUCTOR( TreeNode )

void
AttachTo(struct TreeNode*  ,struct TreeNode* );

void
Disattach(struct TreeNode* );

BOOL
LIST_ITERATION_CALLBACK_FUNCTION( PreorderDFS );

BOOL
LIST_ITERATION_CALLBACK_FUNCTION( PostorderDFS );

BOOL
LIST_ITERATION_CALLBACK_FUNCTION( BFS );

void 
Traversal(struct TreeNode* , ListNodeCallBack , ListNodeCallBack , void* );

struct BinTreeNode
{
	struct BinTreeNode 	*LeftChild,*RightChild;
	struct TreeNode 	tnd;
};

DECLARATION_STRUCT_CONSTRUCTOR( BinTreeNode )

void 
BinAttachTo(struct BinTreeNode* , struct BinTreeNode* , BOOL );

void 
BinDisattach(struct BinTreeNode* );

typedef BOOL (*BinTreeCompareCallback)(struct BinTreeNode* ,struct BinTreeNode* , void*);

void 
BinTreeInsert(struct BinTreeNode* , struct BinTreeNode* ,BinTreeCompareCallback , void* );

struct BinTreeNode*
BinTreeFind(struct BinTreeNode* , struct BinTreeNode* , BinTreeCompareCallback ,BinTreeCompareCallback , void* );

void 
BinTreeInorderTraversal(struct BinTreeNode* , ListNodeCallBack , void* );

typedef void (*SetLevelCallback)(struct TreeNode* , uint );

void 
TreeGetNodesLevel(struct TreeNode* , SetLevelCallback );
