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

#define DEF_FREE_LIST_ELEMENT_SAFE_FUNCTION( _fun_name , _type_tag , _list_node_member_name , _other_statements ) \
	BOOL \
	LIST_ITERATION_CALLBACK_FUNCTION( _fun_name ) \
	{ \
		_type_tag * pNow = GET_STRUCT_ADDR_FROM_IT( pa_pINow , _type_tag , _list_node_member_name ); \
		_other_statements \
		tk( pa_pINow , pa_pIForward ); \
		tkfree(pNow); \
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
		} \
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
ForEach(struct Iterator* , ListNodeCallBack ,void* );

BOOL
LIST_ITERATION_CALLBACK_FUNCTION( SortingInsert );

void
SortList( struct Iterator*  , struct SortingInsertPa* );

void
ReverseList( struct Iterator* );

BOOL
LIST_ITERATION_CALLBACK_FUNCTION( CleanedOutOfList );
