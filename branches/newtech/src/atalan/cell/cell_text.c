/*
Text value cells

(c) 2012 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "../language.h"

void VarInitStr(Var * var, char * str)
{
	var->str = StrAlloc(str);
}

Var * TextCell(char * str)
{
	Var * var;
	var = NewCell(INSTR_TEXT);
	VarInitStr(var, str);
	return var;
}


void TextCellFree(Cell * cell)
{
	StrFree(cell->str);
	cell->str = NULL;
}

void TextCellPrint(Cell * cell)
{
	PrintChar('\"');
	Print(cell->str);
	PrintChar('\"');
}
