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

#define GET_STRUCT_ADDR_FROM_IT( _iterator , _type_tag , _list_node_member_name ) \
	( _type_tag *)((int)( ( _iterator )->now) - MEMBER_OFFSET( _type_tag , _list_node_member_name ) )

#define LIST_ITERATION_CALLBACK_FUNCTION( _fun_name ) \
	_fun_name (struct Iterator* pa_pIHead, struct Iterator* pa_pINow , struct Iterator* pa_pIForward , void* pa_else)

#define DEF_FREE_LIST_ELEMENT_CALLBACK_FUNCTION( _fun_name , _type_tag , _list_node_member_name , _other_statements ) \
	BOOL \
	LIST_ITERATION_CALLBACK_FUNCTION( _fun_name ) \
	{ \
		_type_tag * pNow = GET_STRUCT_ADDR_FROM_IT( pa_pINow , _type_tag , _list_node_member_name ); \
		_other_statements \
		tk( pa_pINow , pa_pIForward ); \
		tkfree(pNow); \
		if( pa_pINow->now == pa_pIHead->last ) \
		{ \
			*pa_pIHead = GetIterator( NULL ); \
			return 1; \
		} else \
		{ \
			return 0; \
		} \
	}

#define LIST_SAFE_RETURN \
	if( pa_pINow->now == pa_pIForward->last ) \
	{ \
		*pa_pIHead = GetIterator( NULL ); \
		return 1; \
	} else \
	{ \
		if( pa_pINow->now == pa_pIHead->now || \
				pa_pINow->now == pa_pIHead->last ) \
		{ \
			*pa_pIHead = *pa_pIForward; \
		} \
		return 0; \
	}do{}while(0)

#define DEF_FREE_LIST_ELEMENT_SAFE_FUNCTION( _fun_name , _type_tag , _list_node_member_name , _other_statements ) \
	BOOL \
	LIST_ITERATION_CALLBACK_FUNCTION( _fun_name ) \
	{ \
		_type_tag * pNow = GET_STRUCT_ADDR_FROM_IT( pa_pINow , _type_tag , _list_node_member_name ); \
		_other_statements \
		tk( pa_pINow , pa_pIForward ); \
		tkfree(pNow); \
		LIST_SAFE_RETURN; \
	}

struct ListNode
{
	struct ListNode *next,*last;
};

DECLARATION_STRUCT_CONSTRUCTOR( ListNode )

struct Iterator
{
	struct ListNode *now,*last;
};

typedef BOOL (*SortingInsertCallBack)(struct Iterator*, struct Iterator* , void*);

struct SortingInsertPa
{
	SortingInsertCallBack 	si_cbk;
	struct Iterator*	pIInsert;
	void*			else_pa;
};

typedef BOOL (*ListNodeCallBack)(struct Iterator* , struct Iterator* ,struct Iterator* ,void*);

struct Iterator
GetIterator(struct ListNode* );

void
tk(struct Iterator* ,struct Iterator* );

void
AddOneToListTail(struct Iterator*  , struct ListNode* );

void
AddOneToListHead(struct Iterator* , struct ListNode* );

void 
AddOneToListTailSafe(struct Iterator* , struct Iterator* , struct Iterator* , struct ListNode* );

void 
AddOneToListHeadSafe(struct Iterator* , struct Iterator* , struct Iterator* , struct ListNode* );

BOOL
ListDragOneOut(struct Iterator *,struct ListNode *);

void
ForEach(struct Iterator* , ListNodeCallBack ,void* );

BOOL
LIST_ITERATION_CALLBACK_FUNCTION( SortingInsert );

void
SortList( struct Iterator*  , struct SortingInsertPa* );

void
ReverseList( struct Iterator* );
