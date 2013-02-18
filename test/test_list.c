/* 
*      This file is part of the tknet project. 
*    which be used under the terms of the GNU General Public 
*    License version 3.0 as published by the Free Software 
*    Foundation and appearing in the file LICENSE.GPL included 
*    in the packaging of this file.  Please review the following 
*    information to ensure the GNU General Public License 
*    version 3.0 requirements will be met: 
* 
*    http://www.gnu.org/copyleft/gpl.html 
* 
*    Copyright  (C)   2012   Zhong Wei <clock126@126.com>  . 
*/ 

#include <stdio.h>
#include <stdlib.h>
#include "tknet.h"

struct T
{
	int i;
	struct list_node ln;
};

static
LIST_IT_CALLBK(print)
{
	LIST_OBJ(struct T, p, ln);
	printf("%d--", p->i);
	LIST_GO_OVER;
}

static
LIST_IT_CALLBK(release)
{
	LIST_OBJ(struct T, p, ln);
	BOOL res;
	printf("free %d \n", p->i);
	res = list_detach_one(pa_now->now, 
			pa_head, pa_now, pa_fwd);

	free(p);
	return res;
}

static
LIST_CMP_CALLBK(compare)
{
	struct T *p0 = MEMBER_2_STRUCT(pa_node0, struct T, ln);
	struct T *p1 = MEMBER_2_STRUCT(pa_node1, struct T, ln);
	P_CAST(extra, int, pa_extra);

	printf("%d \n", *extra);
	return p0->i > p1->i;
}

/* print list */
#define PRINT_LIST \
	list_foreach(&list, &print, NULL); \
	printf("NULL \n")

int
main()
{
	struct list_it list = LIST_NULL;
	struct T *p;
	struct list_sort_arg sort;
	int i;
	int extra = 123;

	/* insert into list some entries orderly */
	for (i=0; i<9; i++) {
		p = malloc(sizeof(struct T));
		p->i = i;
		LIST_NODE_CONS(p->ln);
		list_insert_one_at_tail(&p->ln, &list, NULL, NULL);
	}
	PRINT_LIST;

	/* sort list */
	sort.cmp = &compare;
	sort.extra = &extra;
	list_sort(&list, &sort);
	PRINT_LIST;
	
	/* free list */
	list_foreach(&list, &release, NULL);
	
	exit(0);
}
