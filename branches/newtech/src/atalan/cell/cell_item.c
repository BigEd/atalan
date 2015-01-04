/*
Item cells

(c) 2014 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php


Items reference array elements.

  mode: INSTR_ITEM

  Local cells define local variables in the function.

*/

#include "../language.h"

Cell * NewItem(Cell * arr, Cell * index)
{
	return NewOp(INSTR_ITEM, arr, index);
}

Cell * ItemEval(Cell * cell)
{
	BigInt * bi;
	Int32 n;
	Cell * arr;
	Cell * idx = Eval(cell->r);
	if (idx->mode == INSTR_INT) {
		bi = IntFromCell(idx);
		n = IntN(bi);
		arr = Eval(cell->l);
		return NthItem(arr, n);
/*		if (n > 1) {
			while(n>1) {
				if (arr->mode == INSTR_TUPLE) {
					arr = arr->r;
					n--;
				} else {
					InternalError("Array index too big.");
					break;
				}
			}
			return arr->l;
		}
*/
	} else {
		RuntimeError("Array index must be integer.");
	}
	return NULL;		//TODO: We should return CELL_ERROR
}
