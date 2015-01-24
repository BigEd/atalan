/*

Cell arithmetics

(c) 2013 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

Arithmetic functions support interval arithmetics.

*/

#include "../language.h"

Cell * Add(Cell * l, Cell * r)
{
	Cell * result = NULL;
	BigInt * il, * ir, ii;
	SequenceCell * sl, * sr, * s_step;

	if (r == NULL || l == NULL) return NULL;

	switch(l->mode) {
	case INSTR_SEQUENCE:
		if (r->mode == INSTR_SEQUENCE) {
			// * [sequence 0,..+(sequence 1,..,<>11)..,()] + [sequence 1,..,<>11]  ->  sequence 1,..+(sequence 1,..,<>11),()
			// * [sequence A,..+(sequence B,..,<>C)..,()] + [sequence B,..,<>C]  ->  sequence A+B,..+(sequence B,..,<>C),()
			//        step in both sequences must be same
			//        init_1 += init_2
			//        
	
			sl = &l->seq; sr = &r->seq;
			if (sl->step != NULL && sl->step->mode == INSTR_SEQUENCE) {
				s_step = &sl->step->seq;
				if (s_step->op == sr->op) {
					if (sr->op == INSTR_ADD) {
						if (IsEqual(s_step->step, sr->step)) {
							result = NewSequence(Add(sl->init, sr->init), Add(sl->step, s_step->step), sl->op, sl->limit, sl->compare_op);
						}
					}
				}
			}
			//TODO: How do we add two sequences?
		} else {
			result = NewSequence(Add(l->seq.init,r), l->seq.step, l->seq.op, Add(l->seq.limit, r), l->seq.compare_op);
		}
		break;

	case INSTR_RANGE:
		if (r->mode == INSTR_RANGE) {
			result = NewRange(Add(l->l, r->l), Add(l->r, r->r));
		} else {
			result = NewRange(Add(l->l, r), Add(l->r, r));
		}
		break;

	case INSTR_TUPLE:
		result = NewTuple(Add(l->l, r), Add(l->r, r));
		break;

	default:
		if (r->mode == INSTR_RANGE || r->mode == INSTR_TUPLE || r->mode == INSTR_SEQUENCE) {
			result = Add(r, l);
		} else {
			// Try to add the cells as integers
			il = IntFromCell(l);
			ir = IntFromCell(r);

			if (il != NULL && ir != NULL) {
				IntAdd(&ii, il, ir);
				result = IntCell(&ii);
				IntFree(&ii);
			}
		}
	}

	if (result == NULL) {
		result = NewOp(INSTR_ADD, l, r);
	}
	return result;
}

Var * Sub(Var * l, Var * r)
{
	Var * result = NULL;
	BigInt * il, * ir, ii;

	if (r == NULL || l == NULL) return NULL;

	if (l->mode == INSTR_SEQUENCE) {
		if (r->mode == INSTR_SEQUENCE) {
			//TODO: How do we sub two sequences?
		} else {
			result = NewSequence(Sub(l->seq.init,r), l->seq.step, l->seq.op, Sub(l->seq.limit, r), l->seq.compare_op);
		}
	} else if (l->mode == INSTR_RANGE) {
		if (r->mode == INSTR_RANGE) {
			result = NewRange(Sub(l->l, r->r), Sub(l->r, r->l));
		} else {
			result = NewRange(Sub(l->l, r), Sub(l->r, r));
		}
	} else if (l->mode == INSTR_TUPLE) {
		result = NewTuple(Sub(l->l, r), Sub(l->r, r));
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
			// Power has higher priority than multiplication and it leads to unnecessary 'spaghetti' expression to distribute it
			if (r->mode != INSTR_POWER) {
				result = NewRange(Mul(l->l, r), Mul(l->r, r));
			}
		}
	} else if (l->mode == INSTR_TUPLE) {
		result = NewTuple(Mul(l->l, r), Mul(l->r, r));
	} else if (l->mode == INSTR_DIV) {
		m1 = Mul(l->l, r);
		m2 = l->r;
		//TODO: Find Biggest common divisor
		if (m1->mode == INSTR_INT && m2->mode == INSTR_INT) {
			m3 = DivInt(m1, m2);
			m4 = Mul(m3, m2);
			if (IsEqual(m4, m1)) {
				return m3;
			}
		}
		result = NewOp(INSTR_DIV, m1, m2);
	} else {
		if (r->mode == INSTR_RANGE) return Mul(r, l);

		// Multiply two simple integers if possible
		il = IntFromCell(l);
		ir = IntFromCell(r);

		if (il != NULL && ir != NULL) {
			IntMul(&ii, il, ir);
			result = IntCell(&ii);
			IntFree(&ii);
		}
	}

	if (result == NULL) {
		result = NewOp(INSTR_MUL, l, r);
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

Var * Div(Var * l, Var * r)
{
	Var * result = NULL;
	BigInt * il, * ir, ii, ii2;

	if (IsEqual(r, ONE)) return l;

	// If both values are integer and L is divisible by R, perform the division directly
	il = IntFromCell(l);
	ir = IntFromCell(r);

	if (il != NULL && ir != NULL) {
		IntDiv(&ii, il, ir);
		IntMul(&ii2, &ii, ir);

		if (IntEq(&ii2, il)) {
			result = IntCell(&ii);
		}
		IntFree(&ii);
		IntFree(&ii2);
	}

	if (result == NULL) {
		result = NewOp(INSTR_DIV, l, r);
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

Var * Power(Var * l, Var * r)
{
	Var * result = NULL;
	BigInt * il, * ir, ii, ii2;

	il = IntFromCell(l);
	ir = IntFromCell(r);
	if (il != NULL && ir != NULL) {		
		if (IntEqN(ir, 0)) {
			result = ONE;
		} else if (IntHigherN(ir, 0)) {
			IntSet(&ii, il);
			IntSet(&ii2, ir);

			while (!IntEqN(&ii2, 0)) {
				IntMul(&ii, &ii, il);
				IntSubN(&ii2, 1);
			}
			result = IntCell(&ii);
			IntFree(&ii);
			IntFree(&ii2);
		}
	}

	if (result == NULL) {
		result = NewOp(INSTR_POWER, l, r);
	}
	return result;
}

Cell * Sqrt(Cell * l)
{
	Cell * r = NewOp(INSTR_SQRT, l, NULL);
	return r;
}

Var * CellOp(InstrOp op, Var * left, Var * right)
{
	switch(op) {
	case INSTR_ADD: return Add(left, right);
	case INSTR_SUB: return Sub(left, right);
	case INSTR_MUL: return Mul(left, right);
	case INSTR_DIV: return DivInt(left, right);
	case INSTR_POWER: return Power(left, right);
	default:
		return NewOp(op, left, right);
	}
}

Cell * AddEval(Cell * cell)
{
	ASSERT(cell->mode == INSTR_ADD);
	return Add(Eval(cell->l), Eval(cell->r));
}

Cell * SubEval(Cell * cell)
{
	ASSERT(cell->mode == INSTR_SUB);
	return Sub(Eval(cell->l), Eval(cell->r));
}

Cell * MulEval(Cell * cell)
{
	ASSERT(cell->mode == INSTR_MUL);
	return Mul(Eval(cell->l), Eval(cell->r));
}

Cell * DivIntEval(Cell * cell)
{
	ASSERT(cell->mode == INSTR_DIV);
	return DivInt(Eval(cell->l), Eval(cell->r));
}

Cell * PowerEval(Cell * cell)
{
	ASSERT(cell->mode == INSTR_POWER);
	return Power(Eval(cell->l), Eval(cell->r));
}

Cell * ModEval(Cell * cell)
{
	ASSERT(cell->mode == INSTR_MOD);
	return Mod(Eval(cell->l), Eval(cell->r));
}

Cell * SqrtEval(Cell * cell)
{
	ASSERT(cell->mode == INSTR_SQRT);
	return Sqrt(Eval(cell->l));
}

