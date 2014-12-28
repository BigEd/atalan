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
