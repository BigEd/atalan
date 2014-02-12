/*
Tuple cells

(c) 2013 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

  mode: INSTR_TUPLE

*/

#include "language.h"

Var * FirstItem(Var ** p_en, Var * list)
{
	Var * item;
	*p_en = item = list;
	if (item->mode == INSTR_TUPLE) {
		item = item->l;
	} else if (item->mode == INSTR_EMPTY) {
		item = NULL;
	}
	return item;
}

Var * NextItem(Var ** p_en)
{
	Var * item;
	Var * en = *p_en;
	if (en->mode == INSTR_TUPLE) {
		item = en->r;
		*p_en = item;
		if (item->mode == INSTR_TUPLE) item = item->l;
	} else {
		item = NULL;
	}
	return item;
}

Bool ContainsItem(Var * tuple, Var * item)
{
	Var * en, * var;

	FOR_EACH_ITEM(en, var, tuple)
		if (var == item) return true;
	NEXT_ITEM(en, var)

	return false;
}
