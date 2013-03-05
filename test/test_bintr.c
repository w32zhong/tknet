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

typedef int cmp_t;
#define CMP_GREATER 1
#define CMP_EQUAL   0
#define CMP_LESS  (-1)

typedef cmp_t bintr_cmp_fun(struct bintr_node*,
		struct bintr_node*, void*);

#define BINTR_CMP_CALLBK(_fun_name) \
	cmp_t _fun_name(struct bintr_node *pa_node0, \
			struct bintr_node *pa_node1, \
			void *pa_extra)

typedef void bintr_it_fun(struct bintr_node*, 
		tree_depth_t,
		void*);

#define BINTR_IT_CALLBK(_fun_name) \
	void _fun_name(struct bintr_node *pa_node, \
			tree_depth_t pa_depth, \
			void *pa_extra)

#define BINTR_OBJ(_type, _name, _tree_node_name) \
	_type* _name = MEMBER_2_STRUCT(pa_node, _type, _tree_node_name)

void 
bintr_insert(struct bintr_node *node, 
		struct bintr_node *to, bintr_cmp_fun *cmp,
		void *extra)
{
	if (node->father)
		return;

	if (CMP_GREATER == (*cmp)(node, to, extra)) {
		if (to->rson)
			bintr_insert(node, to->rson, cmp, extra);
		else { 
			to->rson = node;
			node->father = to;
		}
	} else {
		if (to->lson)
			bintr_insert(node, to->lson, cmp, extra);
		else {
			to->lson = node;
			node->father = to;
		}
	}
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

void
bintr_inorder_DFS(struct bintr_node *from, tree_depth_t depth,
		bintr_it_fun *each_do, void *extra)
{
	depth ++;

	if (from->lson)
		bintr_inorder_DFS(from->lson, depth, each_do, extra);

	(*each_do)(from, depth, extra);

	if (from->rson)
		bintr_inorder_DFS(from->rson, depth, each_do, extra);
}

void
bintr_postorder_DFS(struct bintr_node *from, tree_depth_t depth,
		bintr_it_fun *each_do, void *extra)
{
	depth ++;

	if (from->lson)
		bintr_postorder_DFS(from->lson, depth, each_do, extra);

	if (from->rson)
		bintr_postorder_DFS(from->rson, depth, each_do, extra);

	(*each_do)(from, depth, extra);
}

void
bintr_preorder_DFS(struct bintr_node *from, tree_depth_t depth,
		bintr_it_fun *each_do, void *extra)
{
	depth ++;
	(*each_do)(from, depth, extra);

	if (from->lson)
		bintr_preorder_DFS(from->lson, depth, each_do, extra);

	if (from->rson)
		bintr_preorder_DFS(from->rson, depth, each_do, extra);
}

void
bintr_foreach(struct bintr_node *root, 
		void (*traversal)(struct bintr_node*, tree_depth_t, 
			bintr_it_fun*, void*), 
		bintr_it_fun *each_do, void *extra)
{
	tree_depth_t depth = 0;
	(*traversal)(root, depth, each_do, extra);
}

/*
 * test
 */
#include <stdio.h>
#include <stdlib.h>

struct T0 {
	int i;
	struct bintr_node btnd;
};

static
BINTR_CMP_CALLBK(compare) 
{
	struct T0 *p0 = MEMBER_2_STRUCT(pa_node0, struct T0, btnd);
	struct T0 *p1 = MEMBER_2_STRUCT(pa_node1, struct T0, btnd);

	if (p0->i > p1->i)
		return CMP_GREATER;
	else if (p0->i < p1->i)
		return CMP_LESS;
	else 
		return CMP_EQUAL;
}

static BOOL sta_depth_flag[64];

#define DEPTH_END        0
#define DEPTH_BEGIN      1
#define DEPTH_GOING_END  2

static
BINTR_IT_CALLBK(print)
{
	int i;
	BINTR_OBJ(struct T0, p, btnd);

	if (pa_node->father == NULL) 
		sta_depth_flag[pa_depth] = DEPTH_GOING_END;
	else if (pa_node == pa_node->father->rson)
		sta_depth_flag[pa_depth] = DEPTH_GOING_END;
	else if (pa_node == pa_node->father->lson)
		sta_depth_flag[pa_depth] = DEPTH_BEGIN;
	else 
		printf("NO!\n");

	for (i = 0; i<pa_depth; i++) {
		switch (sta_depth_flag[i + 1]) {
		case DEPTH_END:
			printf("   ");
			break;
		case DEPTH_BEGIN:
			printf("  |");
			break;
		case DEPTH_GOING_END:
			printf("  └");
			break;
		default:
			break;
		}
	}
	
	printf("──%d \n", p->i);
	
	if (sta_depth_flag[pa_depth] == DEPTH_GOING_END)
		sta_depth_flag[pa_depth] = DEPTH_END;
}

static BOOL sta_list_flag;

static
BINTR_IT_CALLBK(list_by_order)
{
	BINTR_OBJ(struct T0, p, btnd);

	if (sta_list_flag)
		printf(", ");

	printf("%d", p->i);
	sta_list_flag = 1;
}

#define INSERT(_i) \
	p[_i] = malloc(sizeof(struct T0)); \
	p[_i]->i = _i; \
	BINTR_NODE_CONS(p[_i]->btnd); \
	bintr_insert(&p[_i]->btnd, &root.btnd, &compare, NULL);

int 
main()
{
	struct T0 root, *p[64], to_find, *tmp;
	struct bintr_node *found;
	root.i = 5;
	BINTR_NODE_CONS(root.btnd);

	INSERT(3);
	INSERT(6);
	INSERT(7);
	INSERT(9);
	INSERT(2);
	INSERT(4);

	bintr_foreach(&root.btnd, &bintr_preorder_DFS, &print, NULL);
	printf("\n");
	
	bintr_foreach(&root.btnd, &bintr_inorder_DFS, 
			&list_by_order, NULL);
	printf(". \n");

	to_find.i = 8;
	found = bintr_find(&root.btnd, &to_find.btnd, &compare, NULL);
	tmp = MEMBER_2_STRUCT(found, struct T0, btnd);
	if (found)
		printf("%i found. \n", tmp->i);
	else
		printf("%i not found. \n", to_find.i);

	to_find.i = 9;
	found = bintr_find(&root.btnd, &to_find.btnd, &compare, NULL);
	tmp = MEMBER_2_STRUCT(found, struct T0, btnd);
	if (found)
		printf("%i found. \n", tmp->i);
	else
		printf("%i not found. \n", to_find.i);

	return 0;
}
