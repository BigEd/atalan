/*

Let cell

(c) 2015 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "../language.h"

void PrintLet(Cell * cell)
/*
Purpose:
	Used to implement printing for binary operators.
*/
{
	PrintCell(cell->l);
	PrintKeyword(" = ");
	PrintCell(cell->r);
}
