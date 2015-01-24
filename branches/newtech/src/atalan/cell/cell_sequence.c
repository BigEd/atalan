/*
Sequences

(c) 2013 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "../language.h"

Var * NewSequence(Var * init, Var * step, InstrOp step_op, Var * limit, InstrOp compare_op)
{
	Var * cell;
	cell =  NewCell(INSTR_SEQUENCE);
	cell->seq.op = step_op;
	cell->seq.init = init;
	cell->seq.step = step;
	cell->seq.limit = limit;
	cell->seq.compare_op = compare_op;
	return cell;
}

Bool SequenceStepCount(Cell * cell, Cell ** p_min_steps, Cell ** p_max_steps)
{
	Cell * step_min, * step_max, * init_min, * init_max, * limit_min, * limit_max;
	Cell * min_span, * max_span;

	if (cell == NULL || cell->mode != INSTR_SEQUENCE) return false;

	if (CellRange(cell->seq.step, &step_min, &step_max) && CellRange(cell->seq.init, &init_min, &init_max) && CellRange(cell->seq.limit, &limit_min, &limit_max)) {

		max_span = Sub(limit_max, init_min);
		min_span = Sub(limit_min, init_max);

		if (cell->seq.op == INSTR_ADD) {
			*p_min_steps = DivInt(min_span, step_max);
			*p_max_steps = DivInt(max_span, step_min);
			return true;
		}
	}
	return false;
}

Bool  SequenceRange(Type * type, Var ** p_min, Var ** p_max)
{
	Var * init, * step, * limit, * max_over, * step_min, * step_max;
	Var * size;
	InstrOp op;
	SequenceCell * seq, * step_s;

	*p_min = * p_max = NULL;

	if (type == NULL || type->mode != INSTR_SEQUENCE) return false;

	seq = &type->seq;

	// Arithmetic sequence defined with incremented step

	if (TypeIsInt2(seq->init) && seq->step != NULL && seq->step->mode == INSTR_SEQUENCE) {
		step_s = &seq->step->seq;

		if (step_s->op == INSTR_ADD && CellIsIntConst(step_s->step)) {
			if (SequenceStepCount(seq->step, &size, &step_max)) {
				if (IsEqual(size, step_max)) {
					if (SequenceRange(seq->step, &step_min, &step_max)) {
						*p_min = seq->init;
						*p_max = Div(Mul(Add(seq->init, step_max), size), TWO);
						return true;
					}
				}
			}
		}


	} else if (TypeIsInt2(type->seq.step) && TypeIsInt2(type->seq.init) && TypeIsInt2(type->seq.limit)) {
		init = CellMin(type->seq.init);
		step = CellMax(type->seq.step);
		step_min = CellMin(type->seq.step);
		limit = CellMax(type->seq.limit);

		if (type->seq.op == INSTR_ADD) {
			size = Sub(limit, init);
			max_over = Mod(size, step);
			op = type->seq.compare_op;

			// If the comparison is exact, we know the sequence stops even in case of IFNE
			// In other case, the sequence is infinite, as the condition does not 'catch' any value of sequence
			if (op == INSTR_NE && CellIsIntConst(type->seq.step) && CellIsIntConst(type->seq.init) && CellIsIntConst(type->seq.limit) && IsEqual(max_over, ZERO)) {
				limit = Sub(limit, step);  // compared value will not be part of the sequence
			} else if (op == INSTR_LE) {
			} else if (op == INSTR_LT) {

				// We only make the limit smaller, if the step is constant
				if (CellIsIntConst(step)) {
					limit = Add(Mul(DivInt(Sub(size, ONE), step_min), step_min), init);
					max_over = ZERO;
				}
			} else {
				goto done;
			}

			*p_min = init;
			*p_max = Add(limit, max_over);
			return true;
		} else if (type->seq.op == INSTR_SUB) {
			// init >= limit  (we go down from init o limit)
			size = Sub(init, limit);
			max_over = Mod(size, step);
			op = type->seq.compare_op;
			switch(op) {
			case INSTR_GE:
				break;
			case INSTR_GT:
				if (CellIsIntConst(step)) {
					limit = Add(limit, step_min);
				}
				break;
			default:
				goto done;
			}
			*p_min = limit;
			*p_max = Add(init, max_over);
			return true;
		}
	}
done:
	return false;
}

Var * ResolveSequence(Var * cell)
/*
Purpose:
	Try to combine sequence type into integer type.
*/
{
	Var * min, * max;

	if (SequenceRange(cell, &min, &max)) {
		return NewRange(min, max);
	}
	return NULL;
}

Cell * CollapseSequence(Cell * cell)
{
	Var * min, * max;

	if (SequenceRange(cell, &min, &max)) {
		if (cell->seq.op == INSTR_ADD) return max;
		if (cell->seq.op == INSTR_SUB) return min;
	}
	return cell;
}

void PrintBraced(Cell * cell)
{
	Bool braced = !CellIsIntConst(cell);
	if (braced) {
		Print("(");
	}
	PrintCell(cell);
	if (braced) {
		Print(")");
	}
}

void PrintSequence(Cell * cell)
{
	SequenceCell * seq = &cell->seq;
	Print("sequence "); 
	PrintCell(seq->init);
	Print(",..");

	//---- Step
	if (seq->op == INSTR_ADD && IsEqual(seq->step, ONE)) {
	} else {
		Print(INSTR_INFO[seq->op].symbol);
		PrintBraced(seq->step);
		Print("..");
	}

	//---- Limit

	Print(",");
	if (seq->compare_op != INSTR_LE && seq->compare_op != INSTR_VOID) {
		Print(INSTR_INFO[seq->compare_op].symbol);
	}
	PrintBraced(seq->limit);
}
