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
