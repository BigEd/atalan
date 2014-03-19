/*

Op cells

Cells representing operations like ADD, SUB, AND, etc.

- it may reference place in source code, where the operator was specified

(c) 2012 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"

Bool CellIsOp(Var * cell)
{
	return cell != NULL && FlagOn(INSTR_INFO[cell->mode].flags, INSTR_OPERATOR);
}

Var * VarFindOp(InstrOp op, Var * left, Var * right, Var * middle)
/*
Purpose:
	Find variable created as combination of two other variables.
Argument:
	ref		Array is accessed using reference.
*/
{
	Var * var;
	FOR_EACH_VAR(var)
		if (var->mode == op && var->l == left && var->r == right) return var;
	NEXT_VAR
	return NULL;
}


Var * VarNewDeref(Var * adr)
{
	Var * var;
	FOR_EACH_VAR(var)
		if (var->mode == INSTR_DEREF && var->var == adr) return var;
	NEXT_VAR

	var = NewCell(INSTR_DEREF);
	var->var = adr;
	if (adr->type != NULL && adr->type->variant == TYPE_ADR) {
		var->type = adr->type->element;
	}
	return var;
}

Var * NewOp(InstrOp op, Var * l, Var * r)
/*
Purpose:
	Alloc new operator cell.
Argument:
	ref		Array is accessed using reference.
*/
{
	Var * item;

	// Try to find same element

	Var * var = VarFindOp(op, l, r, NULL);
	if (var != NULL) return var;

	item = NewCell(op);
	item->l    = l;
	item->m    = NULL;
	item->type = NULL;

	// If the right is in, out or register, whole operation is the same
	item->var  = r;

	// If this is element from in or out variable, it is in or out too
	item->submode |= (l->submode & (SUBMODE_IN|SUBMODE_OUT|SUBMODE_REG));
	item->submode |= (l->submode & (SUBMODE_IN|SUBMODE_OUT|SUBMODE_REG));
	return item;
}

Var * VarNewElement(Var * arr, Var * idx)
{
	Var * var = NewOp(INSTR_ELEMENT, arr, idx);
	var->type = ANY;
	return var;
}

Var * VarNewByteElement(Var * arr, Var * idx)
/*
Purpose:
	Alloc new reference to specified byte of variable.
Argument:
	ref		Array is accessed using reference.
*/
{	
	//TODO: VarNewByteElement may create simple integer when used with two integer constants

	Var * item = NewOp(INSTR_BYTE, arr, idx);
	item->type = TypeByte();
	return item;
}

Var * VarNewBitElement(Var * arr, Var * idx)
{
	Var * item = NewOp(INSTR_BYTE, arr, idx);
	return item;
}

Var * NewVariant(Var * left, Var * right)
{
	if (left == NULL) return right;
	if (right == NULL) return left;
	if (right == left) return left;
	return NewOp(INSTR_VARIANT, left, right);
}
