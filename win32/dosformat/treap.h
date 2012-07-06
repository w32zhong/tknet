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

struct Treap
{
	uint RanPriority;
	struct BinTreeNode btnd;
};

DECLARATION_STRUCT_CONSTRUCTOR( Treap )

void 
TreapRoateUp(struct Treap* );

void
TreapInsert(struct Treap* , struct Treap* , BinTreeCompareCallback , void* );

void 
TreapDragOut( struct Treap* );

