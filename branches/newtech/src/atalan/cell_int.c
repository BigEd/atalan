/*
Integer constants

Integer constants are represented as INSTR_INT cells.
They are kept in their own private scope.

(c) 2012 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"

GLOBAL Var * ZERO;
GLOBAL Var * ONE;
GLOBAL Var * MINUS_ONE;

GLOBAL Var * INTS[256];
GLOBAL Var INT_VAR;

Var * IntCellN(Int32  n)
{
	Var * var;
	BigInt bi;
	IntInit(&bi, n);
	var = IntCell(&bi);
	IntFree(&bi);
	return var;
}

Bool CellIsIntConst(Var * var)
{
	return var != NULL && (
		var->mode == INSTR_INT 
		|| (var->mode == INSTR_RANGE && IsEqual(var->l, var->r)) 
		|| (var->mode == INSTR_TYPE && CellIsIntConst(var->possible_values))
		);
}

BigInt * IntFromCell(Var * var)
/*
Purpose:
	Return integer value represented by the variable, if it is constant.
*/
{
	if (var == NULL) return NULL;

	// For named const, work with it's referenced value
	if (var->mode == INSTR_VAR && var->type->mode == INSTR_INT) var = var->type;

	if (var->mode == INSTR_INT) return &var->n;

	return NULL;
}

Var * IntCell(BigInt * n)
/*
Purpose:
	Return variable representing integer constant.
*/
{
	Var * var;

	// Try to find the integer variable in hash
//	if (n>=0 && n<=255) {
//		var = INTS[n];
//		if (var != NULL) return var;
//	}

	// Try to find the integer constant in the scope

	FOR_EACH_LOCAL(&INT_VAR, var)
		ASSERT(var->mode == INSTR_INT);
		if (var->mode == INSTR_INT && IntEq(&var->n, n)) return var;
	NEXT_LOCAL

	// Constant was not found, create new one
	var = NewCell(INSTR_INT);
	CellSetScope(var, &INT_VAR);

//	var->type = &TINT;						// In future, the type of the constant should be the constant itself (self reference)
	IntSet(&var->n, n);

	// Put the variable into hash
//	if (n>=0 && n<=255) {
//		INTS[n] = var;
//	}

	return var;
}

Bool CellIsN(Var * var, Int32 n)
{
	BigInt * i;

	i = IntFromCell(var);
	return i != NULL && IntEqN(i, n);
}

void IntCellInit()
/*
Purpose:
	Initialize integer management module.
*/
{
	MemEmpty(&INT_VAR, sizeof(INT_VAR));
	MemEmpty(&INTS, sizeof(INTS));
	ZERO = IntCellN(0);
	ONE  = IntCellN(1);
	MINUS_ONE = IntCellN(-1);
}
