/*
Tuple cells

(c) 2013 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

  mode: INSTR_TUPLE

*/

#include "../language.h"

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

Var * NewTuple(Var * left, Var * right)
/*
Purpose:
	Create new tuple from the two variables.
	If the right variable is NULL, left is returned.
*/
{
	Var * var;

	if (right == NULL) return left;
	if (left == NULL) return right;

	var = NewOp(INSTR_TUPLE, left, right);

	return var;
}

UInt32 ItemCount(Cell * list)
{
	Var * en, * var;
	UInt32 n = 0;
	FOR_EACH_ITEM(en, var, list)
		n++;
	NEXT_ITEM(en, var)

	return n;

}

Cell * NthItem(Cell * list, UInt32 index)
{
	Var * en, * var;
	UInt32 n = 0;
	FOR_EACH_ITEM(en, var, list)
		n++;
		if (n == index) return var;
	NEXT_ITEM(en, var)
	return NULL;
}

void ListInit(ListBuilder * cl)
{
	cl->list = cl->last_tuple = NULL;
}

void ListAppend(ListBuilder * cl, Cell * item, InstrOp op)
{
	if (item != NULL) {
		if (cl->list == NULL) {			// count == 0
			cl->list = item;
		} else if (cl->last_tuple == NULL) {  // count == 1
			cl->list = NewOp(op, cl->list, item);
			cl->last_tuple = cl->list;
		} else {
			cl->last_tuple->r = NewOp(op, cl->last_tuple->r, item);
			cl->last_tuple = cl->last_tuple->r;
		}
	}
}

