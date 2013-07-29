/*

Cell comparison operators

(c) 2013 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php


*/

#include "../language.h"

extern Var * MACRO_ARG[MACRO_ARG_CNT];

Var * Min(Var * l, Var * r)
{
	if (IsLowerEq(l, r)) return l;
	return r;
}

Var * Max(Var * l, Var * r)
{
	if (IsHigherEq(l, r)) return l;
	return r;
}

Var * Min4(Var * m1, Var * m2, Var * m3, Var * m4)
{
	return Min(Min(m1, m2), Min(m3,m4));
}

Var * Max4(Var * m1, Var * m2, Var * m3, Var * m4)
{
	return Max(Max(m1, m2), Max(m3,m4));
}

Bool IsSubset(Type * type, Type * master)
/*
Purpose:
	Return true, if first type is subset of second type.
*/
{
	Var * smin, * smax;
	Var * mmin, * mmax;
	Var * result;
	UInt8 j;

	if (type == master) return true;

//	if (type == NULL) return true;
//	if (type->variant == TYPE_UNDEFINED) return true;

	if (type == NULL || master == NULL) return false;

	if (VarIsRuleArg(type)) {
		j = type->idx-1;
		if (MACRO_ARG[j] == NULL) {
			if (IsSubset(type, master)) {
				MACRO_ARG[j] = type;
				return true;
			}
		} else {
			if (MACRO_ARG[j] == type) return true;
		}
		return false;
//		type = MACRO_ARG[type->idx-1];
	}

	// Type may be defined using name type. Use the parent then.

	if (CellRange(type, &smin, &smax) && CellRange(master, &mmin, &mmax)) {
		return IsHigherEq(smin, mmin) && IsLowerEq(smax, mmax);
	}

	if (type->mode == INSTR_TEXT) {
		if (master->mode == INSTR_TEXT) return strcmp(type->str, master->str) == 0;
		if (master->mode == INSTR_TYPE) {
			if (master->variant == TYPE_STRING) {
				return master->possible_values == NULL || IsSubset(type, master->possible_values);
			}
		}
		return false;
	}

	if (type->mode == INSTR_VAR) return IsSubset(type->type, master);
	if (type->mode == INSTR_TUPLE) return IsSubset(type->l, master) && IsSubset(type->r, master);
	if (type->mode == INSTR_VARIANT) return IsSubset(type->l, master) || IsSubset(type->r, master);

	if (master->mode == INSTR_VAR) return IsSubset(type, master->type);
	if (master->mode == INSTR_VARIANT) return IsSubset(type, master->l) || IsSubset(type, master->r);

	if (type->mode == INSTR_TYPE) {
		if (type->possible_values != NULL) return IsSubset(type->possible_values, master);

		if (master->mode == INSTR_TYPE) {

			if (type->variant != master->variant) return false;

			switch(type->variant) {
			case TYPE_PROC:
				result = FirstArg(type, SUBMODE_ARG_OUT);
				if (result == NULL) return false;		// this is function with no argument
				return IsSubset(result, master);

			case TYPE_ARRAY:
				return IsSubset(type->index, master->index) && IsSubset(type->element, master->element);

			default:
				if (master->possible_values == NULL) return true;
				if (type->possible_values == NULL) return false;
				return IsSubset(type->possible_values, master->possible_values);
			}

		}
	}
	return false;	// true
}

Bool TypeIsEqual(Type * left, Type * right)
{
	return (left == right) || (IsSubset(left, right) && IsSubset(right, left));
}

Bool TypeMatches(Type * subset, Type * master)
{
	Var * var;
	UInt8 idx;
	UInt8 j;

	if (master->variant == TYPE_ARG) {
		j = master->arg_no-1;
		if (MACRO_ARG[j] == NULL) {
			if (TypeMatches(subset, master->arg_type)) {
				MACRO_ARG[j] = TypeAlloc(TYPE_TYPE);
				MACRO_ARG[j]->type = subset;
				return true;
			}
		} else {
			if (MACRO_ARG[j]->type == subset) return true;
		}
		return false;
	}

	if (subset->variant == master->variant) {
		if (subset->variant == TYPE_TUPLE) {
			return TypeMatches(subset->left, master->left) && TypeMatches(subset->right, master->right);
		}
	}

	if (VarIsRuleArg(master)) {
		idx = master->idx-1;
		if (MACRO_ARG[idx] == NULL) {
			var = VarNewType(subset);
			MACRO_ARG[idx] = var;
			return true;
		} else {
			var = MACRO_ARG[idx];
			if (VarIsType(var)) {
				master = var->type_value;
			} else {
				InternalError("Expected type var");
			}
		}
	} else if (subset->variant == TYPE_ARRAY && master->variant == TYPE_ARRAY) {
		return TypeMatches(subset->index, master->index) && TypeMatches(subset->element, master->element);
	} else {
		return IsSubset(subset, master);
	}
	return true;
}

