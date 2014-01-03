/*
Sequences

Sequence cell is used in type inferring.
It describes how is a variable modified in loop, when type can not be directly defined.
It is partial type. It cannot be used to define variables.

(c) 2013 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"

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

Bool  SequenceRange(Type * type, Var ** p_min, Var ** p_max)
{
	Var * init, * step, * limit, * max_over, * step_min;
	InstrOp op;

	*p_min = * p_max = NULL;

	if (type != NULL && type->mode == INSTR_SEQUENCE) {
		if (TypeIsInt2(type->seq.step) && TypeIsInt2(type->seq.init) && TypeIsInt2(type->seq.limit)) {
			init = CellMin(type->seq.init);
			step = CellMax(type->seq.step);
			step_min = CellMin(type->seq.step);
			limit = CellMax(type->seq.limit);

			if (type->seq.op == INSTR_ADD) {
				
				max_over = Mod(Sub(limit, init), step);
				op = type->seq.compare_op;

				// If the comparison is exact, we know the sequence stops even in case of IFNE
				// In other case, the sequence is infinite, as the condition does not 'catch' any value of sequence
				if (op == INSTR_NE && CellIsIntConst(type->seq.step) && CellIsIntConst(type->seq.init) && CellIsIntConst(type->seq.limit) && IsEqual(max_over, ZERO)) {
					limit = Sub(limit, step);  // compared value will not be part of the sequence
				} else if (op == INSTR_LE) {
				} else if (op == INSTR_LT) {

					// We only make the limit smaller, if the step is constant
					if (IsEqual(step, step_min)) {
						limit = Add(Mul(DivInt(Sub(Sub(limit, init), ONE), step_min), step_min), init);
						max_over = ZERO;
					}
				} else {
					goto done;
				}

				*p_min = init;
				*p_max = Add(limit, max_over);
				return true;
			}
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

