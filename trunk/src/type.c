/*

Type opearations

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"

Type * TypeAlloc(TypeVariant variant)
{
	Type * type = MemAllocStruct(Type);
	type->variant = variant;
	type->owner   = SCOPE;
	type->flexible = false;
	type->base = NULL;
	return type;
}

Type * TypeAllocInt(Int32 min, Int32 max)
{
	Type * type = TypeAlloc(TYPE_INT);
	type->range.flexible = false;
	type->range.min = min;
	type->range.max = max;
	return type;
}

Type * TypeCopy(Type * base)
{
	Type * type = MemAllocStruct(Type);
	memcpy(type, base, sizeof(Type));
	return type;
}

Type * TypeDerive(Type * base)
{
	Type * type = MemAllocStruct(Type);
	memcpy(type, base, sizeof(Type));
	type->base = base;
	type->owner   = SCOPE;
	return type;
}

void TypeLet(Type * type, Var * var)
{
	Type * vtype;
	Var * arr;
	
	vtype = var->type;

	if (vtype == NULL) return;

	// Assigning array element sets the type to the type of the array element

	if (var->mode == MODE_ELEMENT) {
		arr = var->adr;
		if (arr->type->variant == TYPE_ARRAY) {
			vtype = arr->type->element;
		} else {
			// Element variables on non-array variables are byte access to variable elements
			vtype = TypeByte();
		}
	}

	switch(vtype->variant)
	{
	case TYPE_INT:
		if (type->variant == TYPE_UNDEFINED) {
			type->base = vtype;
			type->variant = TYPE_INT;

			// When setting constant N to the variable, it's range is set to N..N
			// Variables with type like this are in fact constants.

			if (var->mode == MODE_CONST) {
				type->range.min = type->range.max = var->n;
			} else {
				type->range.min = vtype->range.min;
				type->range.max = vtype->range.max;
			}
			type->owner = NULL;
		}
		break;
	}
}

typedef void (*RangeTransform)(Int32 * x, Int32 tr);

void TAdd(Int32 * x, Int32 tr) { *x = *x + tr; }
void TSub(Int32 * x, Int32 tr) { *x = *x - tr; }
void TMul(Int32 * x, Int32 tr) { *x = *x * tr; }
void TDiv(Int32 * x, Int32 tr) { *x = *x / tr; }
void TMod(Int32 * x, Int32 tr)  { *x = *x % tr; }
void TAnd(Int32 * x, Int32 tr) { *x = *x & tr; }
void TOr(Int32 * x, Int32 tr)  { *x = *x | tr; }
void TXor(Int32 * x, Int32 tr)  { *x = *x ^ tr; }

RangeTransform InstrFn(InstrOp op)
{
	RangeTransform r = NULL;
	switch(op) {
		case INSTR_DIV: r = &TDiv; break;
		case INSTR_MOD: r = &TMod; break;
		case INSTR_MUL: r = &TMul; break;
		case INSTR_ADD: r = &TAdd; break;
		case INSTR_SUB: r = &TSub; break;
		case INSTR_AND: r = &TAnd; break;
		case INSTR_OR:  r = &TOr; break;
		case INSTR_XOR: r = &TXor; break;
	}
	return r;
}

void TypeTransform(Type * type, Var * var, InstrOp op)
{
	Type * vtype;
	Var * arr;
	RangeTransform r_fn;
	Int32 min, max, t;

	vtype = var->type;
	if (vtype == NULL) return;

	if (var->mode == MODE_ELEMENT) {
		arr = var->adr;
		if (arr->type->variant == TYPE_ARRAY) {
			vtype = arr->type;
		} else {
			// Element variables on non-array variables are byte access to variable elements
			vtype = TypeByte();
		}
	}

	switch(type->variant) 
	{
	case TYPE_INT:

		r_fn = InstrFn(op);
		if (r_fn == NULL) {
			SyntaxError("type transformation function not found");
			return;
		}

		if (vtype->variant == TYPE_INT) {
			if (var->mode == MODE_CONST) {
				min = max = var->n;
			} else {
				min = vtype->range.min;
				max = vtype->range.max;
				if (op == INSTR_DIV || op == INSTR_MOD || op == INSTR_SUB) {
					t = min;
					min = max;
					max = t;
				}
			}
			r_fn(&type->range.min, min);
			r_fn(&type->range.max, max);
			
		}
		break;
	}
}

