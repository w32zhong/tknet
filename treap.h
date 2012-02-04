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

