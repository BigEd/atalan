/*

Cell properties

 - minimal & maximal value
 - limits

(c) 2013 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php


*/

#include "../language.h"

static BigInt LIM_MIN = INTLIMIT_MIN;
static BigInt LIM_MAX = INTLIMIT_MAX;

Var * CellMin(Var * v)
{
	Var * l, * r;
	if (v == NULL) return NULL;
	switch(v->mode) {
	case INSTR_INT:
	case INSTR_TEXT:
		return v;

	case INSTR_RANGE:
		return v->adr;

	case INSTR_TUPLE:
		l = CellMin(v->adr); r = CellMin(v->var);
		return IsLowerEq(l, r)?l:r;

	case INSTR_VAR:
		return CellMin(v->type);

	case INSTR_TYPE:
		return CellMin(v->possible_values);

	default:
		TODO("Unknown variable.")
			return NULL;
	}
}

Var * CellMax(Var * v)
{
	Var * l, * r;
	if (v == NULL) return NULL;
	switch(v->mode) {
	case INSTR_INT:
	case INSTR_TEXT:
		return v;

	case INSTR_RANGE:
		return v->var;

	case INSTR_TUPLE:
		l = CellMax(v->adr); r = CellMax(v->var);
		return IsHigherEq(l, r)?l:r;
		break;

	case INSTR_VAR:
		return CellMax(v->type);
		break;

	case INSTR_TYPE:
		return CellMax(v->possible_values);

	default:
		TODO("Unknown variable.")
			return NULL;
	}

}

BigInt * TypeMin(Type * type)
{
	if (type != NULL) {
		if (type->variant == TYPE_INT) {
			return &type->range.min;
		} else if (type->mode == INSTR_SEQUENCE) {
			switch(type->seq.op) {
			case INSTR_ADD:
			case INSTR_MUL:
				return TypeMin(type->seq.init);
			default:
				break;
			}
		}
	}
	return &LIM_MIN;
}

BigInt * TypeMax(Type * type)
{
	if (type != NULL) {
		if (type->variant == TYPE_INT) {
			return &type->range.max;
		} else if (type->mode == INSTR_SEQUENCE) {
			switch(type->seq.op) {
			case INSTR_SUB:
			case INSTR_DIV:
			case INSTR_MOD:
				return TypeMax(type->seq.init);
			case INSTR_ADD:
				if (CellIsN(type->seq.step, 1)) {
					return TypeMax(type->seq.limit);
				}
			default:
				break;
			}

		}
	}
	return &LIM_MAX;
}

Bool CellRange(Var * var, Var ** p_min, Var ** p_max)
/*
Purpose:
	If the type represents continuous range, return it's limits.
*/
{
	if (var == NULL) return false;
	if (var->mode == INSTR_INT) {
		*p_min = *p_max = var;
		return true;
	} else if (var->mode == INSTR_RANGE) {
		*p_min = var->adr;
		*p_max = var->var;
		return true;
	} else if (var->mode == INSTR_VAR) {
		return CellRange(var->type, p_min, p_max);
	} else if (var->mode == INSTR_TYPE) {
		return CellRange(var->possible_values, p_min, p_max);
	}
	return false;
}

void TypeLimits(Type * type, Var ** p_min, Var ** p_max)
/*
Purpose:
	Return integer type limits as two variables.
*/
{
	Var * min, * max;
	min = CellMin(type);
	max = CellMax(type);
	*p_min = min;
	*p_max = max;
}

void VarRange(Var * var, BigInt ** p_min, BigInt ** p_max)
{
	*p_min = *p_max = NULL;
	if (var != NULL) {
		*p_min = *p_max = IntFromCell(var);
		if (*p_min == NULL) {
			if (var->mode == INSTR_RANGE) {
				*p_min = IntFromCell(var->adr);
				*p_max = IntFromCell(var->var);
			}
		}
	}
}

void VarCount(Var * var, BigInt * cnt)
{
	BigInt * min, * max;

	if (var == NULL) return;

	if (var->mode == INSTR_INT) {
		IntInit(cnt, 1);
	}
	if (var->mode == INSTR_RANGE) {
		VarRange(var, &min, &max);
		IntSub(cnt, max, min);
		IntAddN(cnt, 1);
	}
}

UInt32 VarByteSize(Var * var)
/*
Purpose:
	Return size of variable in bytes.
*/
{
	Type * type;
	if (var != NULL) {
		type = var->type;
		if (var->mode == INSTR_ELEMENT) {
			return 1;		//TODO: Compute size in a better way
		} else if (var->mode == INSTR_BYTE) {
			return 1;
		} else if (var->mode == INSTR_INT) {
			return IntByteSize(&var->n);
		} else if (var->mode == INSTR_TEXT) {
			return StrLen(var->str);
		}
		return TypeSize(type);
	}
	return 0;
}

UInt8 IntByteSize(BigInt * n)
{
	UInt8 size;
	if (IntLowerEqN(n, 255)) size = 1;
	else if (IntLowerEqN(n,  65535)) size = 2;
	else if (IntLowerEqN(n, 0xffffff)) size = 3;
	else size = 4;		// we currently do not support bigger numbers than 4 byte integers
	return size;
}

void ArrayItemCount(Type * index, BigInt * dest)
{
	BigInt bi1, bi2;
	if (index == NULL) {
		IntInit(dest, 0);
	} else if (index->variant == TYPE_INT) {
		IntRangeSize(dest, &index->range.min, &index->range.max);
	} else if (index->variant == TYPE_TUPLE) {
		ArrayItemCount(index->left, &bi1);
		ArrayItemCount(index->right, &bi2);
		IntMul(dest, &bi1, &bi2);
		IntFree(&bi1); IntFree(&bi2);
	} else {
		IntInit(dest, 0);
	}
}

UInt32 TypeStructSize(Var * var);

UInt32 TypeSize(Type * type)
/*
Purpose:
	Return number of bytes required to represent this type in memory.
*/
{
	UInt32 size;
	UInt32 sizen;
	BigInt bi;
	BigInt * min, * max;
	size = 0;
	if (type != NULL) {

		VarRange(type, &min, &max);
		if (min != NULL && max != NULL) {
			size = IntByteSize(max);
			sizen = IntByteSize(min);
			if (size < sizen) size = sizen;
			return size;
		}

		switch(type->mode) {

		case INSTR_ARRAY_TYPE:
			ArrayItemCount(IndexType(type), &bi);
			size = TypeSize(ItemType(type)) * IntN(&bi);
			IntFree(&bi);
			break;

		case INSTR_TYPE:

			switch(type->variant) {
			case TYPE_ADR:
				size = TypeAdrSize();
				break;

			case TYPE_STRUCT:
				size = TypeStructSize(type);
				break;

			default: break;
			}
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
	FOR_EACH_LOCAL(var, item)
		if (item->mode == INSTR_VAR) {
			size += TypeSize(item->type);
		}
	NEXT_LOCAL
	return size;
}

