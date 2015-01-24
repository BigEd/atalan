/*
Symbols

(c) 2015 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "../language.h"
#include <ctype.h>

void SymbolInitStr(Cell * cell, char * str)
{
	if (str != NULL) {
		cell->name = StrAlloc(str);
	}
}

Cell * NewSymbolWithIndex(char * str, VarIdx idx)
{
	Cell * cell = NewCell(INSTR_SYMBOL);
	SymbolInitStr(cell, str);
	cell->idx2 = idx;
	return cell;
}

Cell * NewSymbol(char * str)
{
	return NewSymbolWithIndex(str, 0);
}

void SymbolCellPrint(Cell * cell)
{
	Bool alpha = true;
	char c;
	char * p = cell->name;

	// If the name contains some non alpha characters, print it enclosed in single quotes
	c = *p++;
	alpha = isalpha(c) || c == '_';
	while(alpha && *p != 0) {
		c = *p++;
		alpha = isalpha(c) || isdigit(c) || c == '_' || c == '\'';
	}
	if (!alpha) Print("\'");
	Print(cell->str);
	if (cell->idx2 > 0) {
		PrintInt(cell->idx2-1);
	}
	if (!alpha) Print("\'");
}

Bool SymbolIsNamed(Cell * symb, char * name)
{
	ASSERT(symb != NULL);
	ASSERT(symb->mode == INSTR_SYMBOL);
	return StrEqual(symb->name, name);
}

Bool SymbolIsTmp(Cell * symbol)
{
	return symbol->name == NULL;
}

GLOBAL char SYMBOL_NAME[128];

char * SymbolName(Cell * symbol)
/*
Purpose:
	Return name of the specified symbol.
	Only one name at any moment may be used.
	Next call to the function will render the name invalid.
*/
{
	SYMBOL_NAME[0] = 0;
	if (symbol == NULL) return "";
	ASSERT(symbol->mode == INSTR_SYMBOL);
	if (SymbolIdx(symbol) == 0) return symbol->name;
	sprintf(SYMBOL_NAME, "%s%d", symbol->name, SymbolIdx(symbol)-1);
	return SYMBOL_NAME;
}

VarIdx SymbolIdx(Cell * symbol)
{
	return symbol->idx2;
}
