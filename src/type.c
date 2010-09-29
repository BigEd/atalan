/*

Type opearations

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"

GLOBAL Type TVOID;
GLOBAL Type TINT;		// used for int constants
GLOBAL Type TSTR;
GLOBAL Type TLBL;
GLOBAL Type TBYTE;		//0..255

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
	Int32 min, max;

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

		// When setting constant N to the variable, it's range is set to N..N
		// Variables with type like this are in fact constants.
		if (var->mode == MODE_CONST) {
			min = max = var->n;
		} else {
			min = vtype->range.min;
			max = vtype->range.max;
		}
	
		if (type->variant == TYPE_UNDEFINED) {
			type->base = vtype;
			type->variant = TYPE_INT;
			type->range.min = min;
			type->range.max = max;
			type->owner = NULL;
		} else if (type->variant == TYPE_INT) {
			if (min < type->range.min) type->range.min = min;
			if (max > type->range.max) type->range.max = max;

		}
		break;
	default:
		break;
	}
}

void TAdd(Int32 * x, Int32 tr) { *x = *x + tr; }
void TSub(Int32 * x, Int32 tr) { *x = *x - tr; }
void TMul(Int32 * x, Int32 tr) { *x = *x * tr; }
void TDiv(Int32 * x, Int32 tr) 
{ 
	// We may divide by zero.
	// In such case, no type transformation is performed
	if (tr != 0) {
		*x = *x / tr; 
	}
}

void TMod(Int32 * x, Int32 tr)  
{ 
	// For types like 0..255 etc., tr can be zero.
	// In such case, we do not modify the type.
	if (tr != 0) {
		*x = *x % tr;
	}
}

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
		default: break;
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
	default:
		break;
	}
}

UInt16 TypeItemCount(Type * type)
{
	UInt16 cnt;
	Type * idx;
	if (type->variant != TYPE_ARRAY) return 1;
	idx = type->dim[0];
	cnt = idx->range.max - idx->range.min + 1;
	return cnt;
}

void TypeAddConst(Type * type, Var * var)
/*
Purpose:
	Add specified variable as associated constant to type.
*/
{

	if (type->variant == TYPE_INT) {
		// Register type of this constant as specified type
		var->type  = type;
		var->scope = var->type->owner;

		if (!var->value_nonempty) {
			if (type->range.flexible) {
				type->range.max++;
				var->n = type->range.max;
				var->value_nonempty = true;
			} else {
				SyntaxError("it is necessary to define constant value explicitly for this type");
			}
		} else {
			if (var->n < type->range.min) {
				if (type->range.flexible) {
					type->range.min = var->n;
				} else {
					SyntaxError("constant out of available range");
				}
			}
			if (var->n > type->range.max) {
				if (type->range.flexible) {
					type->range.max = var->n;
				} else {
					SyntaxError("constant out of available range");
				}
			}
		}
	}
}

void TypeLimits(Type * type, Var ** p_min, Var ** p_max)
/*
Purpose:
	Return integer type limits as two variables.
*/
{
	Var * min, * max;
	min = VarNewInt(type->range.min);
	max = VarNewInt(type->range.max);
	*p_min = min;
	*p_max = max;
}

Bool TypeIsSubsetOf(Type * type, Type * master)
/*
Purpose:
	Return true, if first type is subset of second type.
*/
{
	if (type == master) return true;
	if (type == NULL || master == NULL) return false;
	if (type->variant != master->variant) return false;
	if (type->variant == TYPE_INT) {
		if (type->range.max > master->range.max) return false;
		if (type->range.min < master->range.min) return false;
	}
	return true;
}

UInt32 TypeAdrSize()
{
	//TODO: should be platform defined
	return 2;
}

Type * TypeByte()
{
	return &TBYTE;
}

Type * TypeLongInt()
{
	return &TINT;
}

Var * NextItem(Var * scope, Var * arg, VarSubmode submode)
{
	Var * var = arg->next;
	while(var != NULL && (var->mode != MODE_VAR || var->scope != scope || (submode != 0 && FlagOff(var->submode, submode)))) var = var->next;
	return var;
}

Var * FirstItem(Var * scope, VarSubmode submode)
{
	return NextItem(scope, scope, submode);
}

UInt32 TypeStructSize(Var * var);

UInt32 TypeSize(Type * type)
{
	UInt32 size;
	size = 0;
	if (type != NULL) {
		switch(type->variant) {
		case TYPE_INT:
			size = type->range.max - type->range.min;
			if (size <= 255) size = 1;
			else if (size <= 65535) size = 2;
			else if (size <= 0xffffff) size = 3;
			else size = 4;		// we currently do not support bigger numbers than 4 byte integers
			break;

		case TYPE_ADR:
			size = TypeAdrSize();
			break;
		case TYPE_STRUCT:
			size = TypeStructSize(type->owner);
			break;
		default: break;
		}
	}
	return size;
}

UInt32 TypeStructSize(Var * var)
/*
Purpose:
	Compute size of structure variable in bytes.
*/
{
	UInt32 size = 0;
	Var * item;
	item = FirstItem(var, 0);
	while(item != NULL) {
		if (item->mode == MODE_VAR) {
			size += TypeSize(item->type);
		}
		item = NextItem(var, item, 0);
	}
	return size;
}

UInt32 TypeStructAssignOffsets(Type * type)
/*
Purpose:
	Asign offsets to elements of structure.
*/
{
	UInt32 offset = 0;
	Var * item;
	item = FirstItem(type->owner, 0);
	while(item != NULL) {
		if (item->mode == MODE_VAR) {
			if (item->adr == NULL) {
				item->adr = VarNewInt(offset);
				offset += TypeSize(item->type);
			}
		}
		item = NextItem(type->owner, item, 0);
	}
	return offset;			// offset now contains total size of structure
}

void TypeInit()
{
	TINT.variant = TYPE_INT;
	TINT.range.min = -(long)2147483648L;
	TINT.range.max = 2147483647L;
	TINT.base      = NULL;

	TBYTE.variant = TYPE_INT;
	TBYTE.range.min = 0;
	TBYTE.range.max = 255;
	TBYTE.base      = NULL;

	TSTR.variant = TYPE_STRING;
	TSTR.base      = NULL;

	TLBL.variant = TYPE_LABEL;
	TLBL.range.min = -(long)2147483648L;
	TLBL.range.max = 2147483647L;
	TLBL.base      = NULL;
}

Bool TypeIsBool(Type * type)
{
	if (type == NULL) return false;
	if (type->variant != TYPE_INT) return false;
	return type->range.min == 0 && type->range.max == 1;
}