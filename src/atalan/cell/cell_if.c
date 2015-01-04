/*

If cell

(c) 2015 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "../language.h"

void PrintIf(Cell * cell)
/*
Purpose:
	Used to implement printing for binary operators.
*/
{
	PrintKeyword("if ");
	PrintCell(cell->l);
	PrintKeyword(" goto ");
	PrintCell(cell->r);
}

Cell * NewIf(Cell * condition, Cell * label)
{
	return NewOp(INSTR_IF, condition, label);
}

