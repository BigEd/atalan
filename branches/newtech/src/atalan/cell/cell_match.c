/*

Match cell

(c) 2015 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "../language.h"

void PrintMatch(Cell * cell)
/*
Purpose:
	Used to implement printing for binary operators.
*/
{
	PrintKeyword("[");
	// We want to print the name of argument simplified (without %)
	if (cell->l->mode == INSTR_VAR && VarIsArg(cell->l)) {
		PrintChar(VarArgIdx(cell->l)+'A');
	} else {
		PrintCell(cell->l);
	}
	if (cell->r->mode != INSTR_ANY) {
		PrintKeyword(":");
		PrintCell(cell->r);
	}
	PrintKeyword("]");
}
