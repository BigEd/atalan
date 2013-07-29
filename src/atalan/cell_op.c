/*

Op cells

Cells representing operations like ADD, SUB, AND, etc.

- it may reference place in source code, where the operator was specified

(c) 2012 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"

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

	var = VarFindOp(INSTR_TUPLE, left, right, NULL);
	if (var == NULL) {
		var = NewCell(INSTR_TUPLE);
		var->type = TypeTuple(left->type, right->type);
		var->l = left;
		var->r = right;
	}
	return var;
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

Var * NewOp(InstrOp op, Var * arr, Var * idx)
/*
Purpose:
	Alloc new operator cell.
Argument:
	ref		Array is accessed using reference.
*/
{
	Var * item;

	// Try to find same element

	Var * var = VarFindOp(op, arr, idx, NULL);
	if (var != NULL) return var;

	item = NewCell(op);
	item->adr  = arr;
	item->m    = NULL;

	// Type of array element variable is type of array element
	// We may attempt to address individual bytes of non-array variable as an array
	// in such case the type of the element is byte.
	if (arr->type != NULL) {
		if (arr->type->variant == TYPE_ARRAY) {
			item->type = arr->type->element;
		} else if (arr->type->variant == TYPE_STRUCT) {
			item->type = idx->type;
			item->submode |= (idx->submode & (SUBMODE_IN|SUBMODE_OUT|SUBMODE_REG));
		} else {
			item->type = TypeByte();
		}
	} else {
	}
	item->var  = idx;
	// If this is element from in or out variable, it is in or out too
	item->submode |= (arr->submode & (SUBMODE_IN|SUBMODE_OUT|SUBMODE_REG));
	return item;
}

Var * VarNewElement(Var * arr, Var * idx)
{
	return NewOp(INSTR_ELEMENT, arr, idx);
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

Var * VarNewVariant(Var * left, Var * right)
{
	if (left == NULL) return right;
	if (right == NULL) return left;
	if (right == left) return left;
	return NewOp(INSTR_VARIANT, left, right);
}
