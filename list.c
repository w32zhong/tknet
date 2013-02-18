#include "tknet.h"

#define CMP(_node) \
	(*sort->cmp)(expa->it.now, pa_now->_node, expa->extra)

LIST_IT_CALLBK(list_insert)
{
	P_CAST(sort, struct list_sort_arg, pa_extra);
	P_CAST(expa, struct __list_sort_tmp_arg, sort->extra);

	if (pa_now->now == pa_head->now) {
		if(CMP(now)) {
			list_tk(&expa->it, pa_head);
			*pa_head = list_get_it(pa_head->last);

		} else if (!CMP(last)) 
			list_tk(&expa->it, pa_head);

		return LIST_RET_BREAK;
	} else if (CMP(now)) {
		list_tk(&expa->it, pa_now);
		return LIST_RET_BREAK;

	} else
		return LIST_RET_CONTINUE;
}
