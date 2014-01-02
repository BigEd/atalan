/*

Cell arithmetics

(c) 2013 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

Arithmetic functions support interval arithmetics.

*/

#include "../language.h"

Var * Add(Var * l, Var * r)
{
	Var * result = NULL;
	BigInt * il, * ir, ii;

	if (r == NULL || l == NULL) return NULL;

	if (l->mode == INSTR_RANGE) {
		if (r->mode == INSTR_RANGE) {
			result = NewRange(Add(l->l, r->l), Add(l->r, r->r));
		} else {
			result = NewRange(Add(l->l, r), Add(l->r, r));
		}
	} else if (l->mode == INSTR_TUPLE) {
		result = NewTuple(Add(l->adr, r), Add(l->var, r));
	} else {
		if (r->mode == INSTR_RANGE || r->mode == INSTR_TUPLE) {
			result = Add(r, l);
		} else {
			// Try to add the cells as integers
			il = IntFromCell(l);
			ir = IntFromCell(r);

			if (il != NULL && ir != NULL) {
				IntAdd(&ii, il, ir);
				result = IntCell(&ii);
				IntFree(&ii);
			} else {
				result = NewOp(INSTR_ADD, l, r);
			}
		}
	}
	return result;
}

Var * Sub(Var * l, Var * r)
{
	Var * result = NULL;
	BigInt * il, * ir, ii;

	if (r == NULL || l == NULL) return NULL;

	if (l->mode == INSTR_RANGE) {
		if (r->mode == INSTR_RANGE) {
			result = NewRange(Sub(l->l, r->r), Sub(l->r, r->l));
		} else {
			result = NewRange(Sub(l->adr, r), Sub(l->var, r));
		}
	} else if (l->mode == INSTR_TUPLE) {
		result = NewTuple(Sub(l->adr, r), Sub(l->var, r));
	} else {
		if (l->mode == INSTR_INT && r->mode == INSTR_RANGE) {
			return NewRange(Sub(l, r->r), Sub(l, r->l));
		}
		il = IntFromCell(l);
		ir = IntFromCell(r);

		if (il != NULL && ir != NULL) {
			IntSub(&ii, il, ir);
			result = IntCell(&ii);
			IntFree(&ii);
		} else {
			result = NewOp(INSTR_SUB, l, r);
		}
	}
	return result;
}

Var * Mul(Var * l, Var * r)
{
	Var * result = NULL;
	BigInt * il, * ir, ii;
	Var * m1, * m2, * m3, * m4;

	if (r == NULL || l == NULL) return NULL;

	if (l->mode == INSTR_RANGE) {
		if (r->mode == INSTR_RANGE) {
			m1 = Mul(l->l, r->l);
			m2 = Mul(l->l, r->r);
			m3 = Mul(l->r, r->l);
			m4 = Mul(l->r, r->r);
			result = NewRange(Min4(m1,m2,m3,m4), Max4(m1,m2,m3,m4));
		} else {
			result = NewRange(Mul(l->l, r), Mul(l->r, r));
		}
	} else if (l->mode == INSTR_TUPLE) {
		result = NewTuple(Mul(l->l, r), Mul(l->r, r));
	} else {
		if (r->mode == INSTR_RANGE) return Mul(r, l);

		// Multiply two simple integers if possible
		il = IntFromCell(l);
		ir = IntFromCell(r);

		if (il != NULL && ir != NULL) {
			IntMul(&ii, il, ir);
			result = IntCell(&ii);
			IntFree(&ii);
		} else {
			result = NewOp(INSTR_MUL, l, r);
		}
	}
	return result;
}

Var * DivInt(Var * l, Var * r)
/*
Purpose:
	Perform integer division of two cells.
*/
{
	Var * result = NULL;
	BigInt * il, * ir, ii;
	Var * m1, * m2, * m3, * m4;

	if (r == NULL || l == NULL) return NULL;

	if (l->mode == INSTR_RANGE) {
		if (r->mode == INSTR_RANGE) {
			m1 = DivInt(l->l, r->l);
			m2 = DivInt(l->l, r->r);
			m3 = DivInt(l->r, r->l);
			m4 = DivInt(l->r, r->r);
			result = NewRange(Min4(m1,m2,m3,m4), Max4(m1,m2,m3,m4));
		} else {
			result = NewRange(DivInt(l->l, r), DivInt(l->r, r));
		}
	} else if (l->mode == INSTR_TUPLE) {
		result = NewTuple(DivInt(l->l, r), DivInt(l->r, r));
	} else {
		il = IntFromCell(l);
		ir = IntFromCell(r);

		if (il != NULL && ir != NULL) {
			IntDiv(&ii, il, ir);
			result = IntCell(&ii);
			IntFree(&ii);
		} else {
			result = NewOp(INSTR_DIV, l, r);
		}
	}
	return result;
}

Var * Mod(Var * l, Var * r)
/*
Purpose:
	Compute modulus of two cells.
*/
{
	Var * result = NULL;
	BigInt * il, * ir, ii;

	if (r == NULL || l == NULL) return NULL;

	if (l->mode == INSTR_TUPLE) {
		result = NewTuple(Mod(l->l, r), Mod(l->r, r));
	} else {
		il = IntFromCell(l);
		ir = IntFromCell(r);

		if (il != NULL && ir != NULL) {
			IntMod(&ii, il, ir);
			result = IntCell(&ii);
			IntFree(&ii);
		} else {
			result = NewOp(INSTR_MOD, l, r);
		}
	}
	return result;
}


Bool IsEqual(Var * left, Var * right)
{
	BigInt * l, * r;

	if (left == NULL || right == NULL) return false;
	if (left == right) return true;
	if (left->mode == INSTR_VAR && left->adr != NULL) return IsEqual(left->adr, right);
	if (right->mode == INSTR_VAR && right->adr != NULL) return IsEqual(left, right->adr);

	// Try to compare as two integers
	l = IntFromCell(left);
	r = IntFromCell(right);

	if (l != NULL && r != NULL) {
		return IntEq(l, r);
	}

	if (left->mode == right->mode) {
		if (left->mode == INSTR_TUPLE) {
			return IsEqual(left->adr, right->adr) && IsEqual(left->var, right->var);
		}
	}
	return false;
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
	if (left->mode == INSTR_VAR && left->adr != NULL) return CellCompare(left->adr, right);
	if (right->mode == INSTR_VAR && right->adr != NULL) return CellCompare(left, right->adr);

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

Var * CellOp(InstrOp op, Var * left, Var * right)
{
	switch(op) {
	case INSTR_ADD: return Add(left, right);
	case INSTR_SUB: return Sub(left, right);
	case INSTR_MUL: return Mul(left, right);
	case INSTR_DIV: return DivInt(left, right);

	default:
		return NewOp(op, left, right);
	}
}
