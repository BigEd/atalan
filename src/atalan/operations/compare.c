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
	UInt8 j;

	if (type == master) return true;

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
	}

	if (master->mode == INSTR_MATCH) {
		if (VarIsRuleArg(master->l)) {
			j = master->l->idx-1;
			if (MACRO_ARG[j] == NULL) {
				if (IsSubset(type, master->r)) {
					MACRO_ARG[j] = type;
					return true;
				}
			} else {
				if (MACRO_ARG[j] == type) return true;
			}
			return false;			
		}
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
	if (master->mode == INSTR_TYPE && master->variant == TYPE_TYPE && master->possible_values != NULL) return IsSubset(type, master->possible_values);

	if (type->mode == INSTR_ARRAY_TYPE) {
		return master->mode == INSTR_ARRAY_TYPE 
			&& IsSubset(IndexType(type), IndexType(master)) 
			&& IsSubset(ItemType(type), ItemType(master));

	}

	if (type->mode == INSTR_TYPE) {
		if (type->possible_values != NULL) return IsSubset(type->possible_values, master);

		if (master->mode == INSTR_TYPE) {

			if (type->variant != master->variant) return false;

			switch(type->variant) {

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

Int16 CellCompare(Var * left, Var * right)
/*
Purpose:
	0    left == right
	-1   left < right
	1    left > right
	127  uncomparable
*/
{
	BigInt * l, * r;
	if (left == right) return 0;
	if (left == NULL || right == NULL) return 127;
	if (left->mode == INSTR_VAR && VarAdr(left) != NULL) return CellCompare(VarAdr(left), right);
	if (right->mode == INSTR_VAR && VarAdr(right) != NULL) return CellCompare(left, VarAdr(right));

	// Try to compare as two integers
	l = IntFromCell(left);
	r = IntFromCell(right);

	if (l != NULL && r != NULL) {
		if (IntEq(l,r)) return 0;
		if (IntLower(l,r)) return -1;
		return 1;
	}

	return 127;
}

Bool IsHigher(Var * left, Var * right)
{
	Int16 r = CellCompare(left, right);
	return r == 1;
}

Bool IsHigherEq(Var * left, Var * right)
{
	Int16 r = CellCompare(left, right);
	return r == 0 || r == 1;
}

Bool IsLowerEq(Var * left, Var * right)
{
	Int16 r = CellCompare(left, right);
	return r == 0 || r == -1;
}

Bool IsLower(Var * left, Var * right)
{
	Int16 r = CellCompare(left, right);
	return r == -1;
}

Bool IsEqual(Var * left, Var * right)
{
	BigInt * l, * r;

	if (left == NULL || right == NULL) return false;
	if (left == right) return true;
	if (left->mode == INSTR_VAR && VarAdr(left) != NULL) return IsEqual(VarAdr(left), right);
	if (right->mode == INSTR_VAR && VarAdr(right) != NULL) return IsEqual(left, VarAdr(right));

	// Try to compare as two integers
	l = IntFromCell(left);
	r = IntFromCell(right);

	if (l != NULL && r != NULL) {
		return IntEq(l, r);
	}

	if (left->mode == right->mode) {
		if (left->mode == INSTR_TUPLE) {
			return IsEqual(left->l, right->l) && IsEqual(left->r, right->r);
		} else if (left->mode == INSTR_SEQUENCE) {
			if (IsEqual(left->seq.init, right->seq.init) && IsEqual(left->seq.step, right->seq.step) && left->seq.op == right->seq.op) {
				if (left->seq.compare_op == right->seq.compare_op && IsEqual(left->seq.limit, right->seq.limit)) return true;

				//TODO: We should compute steps in a better way
				if (left->seq.op == INSTR_ADD) {
					if (left->seq.compare_op == INSTR_LT && right->seq.compare_op == INSTR_LE) {
						if (IsEqual(Sub(left->seq.limit, left->seq.step), right->seq.limit)) return true;
					} else if (left->seq.compare_op == INSTR_LT && right->seq.compare_op == INSTR_LE) {
						if (IsEqual(left->seq.limit, Sub(right->seq.limit, right->seq.step))) return true;
					}
				} else if (left->seq.op == INSTR_SUB) {
					if (left->seq.compare_op == INSTR_GT && right->seq.compare_op == INSTR_GE) {
						if (IsEqual(Add(left->seq.limit, left->seq.step), right->seq.limit)) return true;
					} else if (left->seq.compare_op == INSTR_GT && right->seq.compare_op == INSTR_GE) {
						if (IsEqual(left->seq.limit, Sub(right->seq.limit, right->seq.step))) return true;
					}
				}
			}
		}
	}
	return false;
}

Bool IsNotEqual(Cell * l, Cell * r)
{
	return !IsEqual(l, r);
}

Cell * EqEval(Cell * cell)
{
	ASSERT(cell->mode == INSTR_EQ);
	return ToBool(IsEqual(Eval(cell->l), Eval(cell->r)));
}

Cell * NeEval(Cell * cell)
{
	ASSERT(cell->mode == INSTR_NE);
	return ToBool(IsNotEqual(Eval(cell->l), Eval(cell->r)));
}

Cell * LtEval(Cell * cell)
{
	ASSERT(cell->mode == INSTR_LT);
	return ToBool(IsLower(Eval(cell->l), Eval(cell->r)));
}

Cell * LeEval(Cell * cell)
{
	ASSERT(cell->mode == INSTR_LE);
	return ToBool(IsLowerEq(Eval(cell->l), Eval(cell->r)));
}

Cell * GeEval(Cell * cell)
{
	ASSERT(cell->mode == INSTR_GT);
	return ToBool(IsHigher(Eval(cell->l), Eval(cell->r)));
}

Cell * GtEval(Cell * cell)
{
	ASSERT(cell->mode == INSTR_GE);
	return ToBool(IsHigherEq(Eval(cell->l), Eval(cell->r)));
}
