#include "tknet.h"

struct bintr_node {
	struct bintr_node *father, *rson, *lson;
};

struct treap_node {
	struct bintr_node btnd;
	uint ran_priority;
};

#define BINTR_NODE_CONS(_node) \
	(_node).father = NULL; \
	(_node).rson = NULL; \
	(_node).lson = NULL

typedef int CMP;
#define CMP_GREATER 1
#define CMP_EQUAL   0
#define CMP_LESS  (-1)

typedef CMP bintr_cmp_fun(struct bintr_node*,
		struct bintr_node*, void*);

void 
bintr_insert(struct bintr_node *node, 
		struct bintr_node *to, bintr_cmp_fun *cmp,
		void *extra)
{
	if (node->father)
		return;

	if (CMP_GREATER == (*cmp)(node, to, extra)) {
		if (to->rson) {
			bintr_insert(node, to->rson, cmp, extra);
		} else { 
			to->rson = node;
		}
	} else {
		if (to->lson) {
			bintr_insert(node, to->lson, cmp, extra);
		} else {
			to->lson = node;
		}
	}
			
	node->father = to;
}

struct bintr_node*
bintr_find(struct bintr_node *from, 
		struct bintr_node *find, bintr_cmp_fun *cmp,
		void *extra)
{
	BOOL res = (*cmp)(find, from, extra);

	if (res == CMP_EQUAL)
		return from;
	else if (res == CMP_GREATER && from->rson)
		return bintr_find(from->rson, find, cmp, extra);
	else if (res == CMP_LESS && from->lson)
		return bintr_find(from->lson, find, cmp, extra);

	return NULL;
}

typedef void bintr_it_fun(struct bintr_node*, void*);

void
bintr_inorder_DFS(struct bintr_node *from,
		bintr_it_fun *each_do, void *extra)
{
	if (from->lson)
		bintr_inorder_DFS(from->lson, each_do, extra);

	(*each_do)(from, extra);

	if (from->rson)
		bintr_inorder_DFS(from->rson, each_do, extra);
}

void
bintr_postorder_DFS(struct bintr_node *from,
		bintr_it_fun *each_do, void *extra)
{
	if (from->lson)
		bintr_postorder_DFS(from->lson, each_do, extra);

	if (from->rson)
		bintr_postorder_DFS(from->rson, each_do, extra);

	(*each_do)(from, extra);
}

void
bintr_preorder_DFS(struct bintr_node *from,
		bintr_it_fun *each_do, void *extra)
{
	(*each_do)(from, extra);

	if (from->lson)
		bintr_preorder_DFS(from->lson, each_do, extra);

	if (from->rson)
		bintr_preorder_DFS(from->rson, each_do, extra);
}

int 
main()
{
	return 0;
}
