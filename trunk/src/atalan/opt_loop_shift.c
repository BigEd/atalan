/*

Loop shift/reversal optimization routines

(c) 2012 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"

/*
There routines are performed after type inferencer made it's work.
This way, we still have the loop range information deduced by inferencer.

*/

Bool VarShiftIndex(Var ** p_var, Var * idx, IntLimit shift)
/*
Purpose:
	If variable is in the form arr(idx) where idx is specified value, shift it down by specified offset.
*/
{
	Var * var_idx;
	Var * var = *p_var;

	if (var != NULL) {
		if (var->mode == INSTR_ELEMENT) {
			var_idx = var->var;
			if (VarIsEqual(var_idx, idx)) {
				idx = VarNewOp(INSTR_SUB, idx, VarNewInt(shift));
				*p_var = VarNewElement(var->adr, idx);
				return true;
			} else if (var_idx->mode == INSTR_SUB) {
			} else if (var_idx->mode == INSTR_ADD) {
			}
		}
	}
	return false;
}

Bool VarShiftIsPossible(InstrBlock * head, InstrBlock * loop_end, Var * var, IntLimit shift)
{
	Bool reversal_possible = true;
	InstrBlock * blk;
	Instr * i;
	Instr i2;

	for(blk = head; blk != loop_end; blk = blk->next) {
		for(i = blk->first; i != NULL; i = i->next) {
			if (i->op == INSTR_LINE) continue;
			if (VarIsEqual(i->result, var) || VarIsEqual(i->arg1, var) || VarIsEqual(i->arg2, var)) {
				if (IS_INSTR_BRANCH(i->op) && VarIsConst(i->arg2) || VarIsConst(i->arg1)) {
					// this is comparison against constant, that's possible
				}  else {
					if (!InstrIsSelfReferencing(i)) return false;
				}
			} else {
				MemMove(&i2, i, sizeof(Instr));
				if (VarShiftIndex(&i2.result, var, shift) || VarShiftIndex(&i2.arg1, var, shift) || VarShiftIndex(&i2.arg2, var, shift)) {
					if (!InstrTranslate3(i2.op, i2.result, i2.arg1, i2.arg2, TEST_ONLY)) {
						return false;
					}
				}
			}
		}
	}
	return true;
}
/*
Var * VarShift(Var * var, Var * to_shift, Var * shift)
{
	Var * idx;
	if (var == to_shift) {
		return VarNewOp(INSTR_ADD, var, shift);
	} if (var->mode == INSTR_ELEMENT) {
		idx = VarShift(var->var, to_shift, shift);
		if (idx != var->var) {
			return VarNewOp(INSTR_ELEMENT, var->adr, idx);
		}
	}
	return var;
}
*/
Var * VarAddNMod(Var * left, IntLimit right, IntLimit modulo)
{
	Var * var = NULL;
	if (VarIsIntConst(left)) {
		var = VarNewInt((left->n + right) % modulo);
	}
	return var;
}

void LoopShift(InstrBlock * head, InstrBlock * loop_end, Var * var, IntLimit shift, IntLimit top)
{
	InstrBlock * blk;
	Instr * i;
	for(blk = head; blk != loop_end; blk = blk->next) {
		for(i = blk->first; i != NULL; i = i->next) {
			if (i->op == INSTR_LINE) continue;
			if (IS_INSTR_BRANCH(i->op)) {
				if (VarIsEqual(i->arg1, var)) {
					i->arg2 = VarAddNMod(i->arg2, shift, top);
				} else if (VarIsEqual(i->arg2, var)) {
					i->arg1 = VarAddNMod(i->arg1, shift, top);
				}
			} else {
				VarShiftIndex(&i->result, var, shift);
				VarShiftIndex(&i->arg1, var, shift);
				VarShiftIndex(&i->arg2, var, shift);
			}
		}
	}
}

void OptimizeLoopShift(Var * proc)
{
	Bool modified = false;
	InstrBlock * header;
	Loc preheader;
	Instr * i, * init_i;
	Loc loc;
	Var * loop_var;
	Type * type;
	IntLimit shift;
	IntLimit top;

//	return;

	if (Verbose(proc)) {
		PrintHeader(3, "Loop shift");
		PrintProc(proc);
	}

	loc.proc = proc;

	for(loc.blk = proc->instr; loc.blk != NULL; loc.blk = loc.blk->next) {
		header = NULL;
		if (loc.blk->jump_type == JUMP_LOOP) {
			header = loc.blk->cond_to;
			if (header == NULL) header = loc.blk->to;
		}

		if (header != NULL) {
			i = LastInstr(loc.blk);

			if (i != NULL && IS_INSTR_BRANCH(i->op)) {

				loop_var = i->arg1;

				type = i->type[ARG1];
				if (type->variant == TYPE_SEQUENCE) {
					if (type->seq.op == INSTR_ADD && TypeIsIntConst(type->seq.step) && TypeIsIntConst(type->seq.init) && TypeIsIntConst(type->seq.limit)) {

						// Find the initialization of loop variable
						// We do not need to find the initialization, we can just increment it
						
						LoopPreheader(proc, header, &preheader);

						i = preheader.i;
						if (i == NULL) {
							i = preheader.blk->last;
						} else {
							i = i->prev;
						}

						if (i != NULL) {
							if (i->op == INSTR_LET && VarIsEqual(i->result, loop_var)) {
								if (VarIsIntConst(i->arg1)) {
									init_i = i;
								}
							}
						}

						// Compute the difference (we compute defference up to 256 - zero overflow)
						
						IntInit(&top, 256);
						IntSub(&shift, &top, TypeMax(type));

						if (init_i != NULL && VarShiftIsPossible(header, loc.blk->next, loop_var, shift)) {
							LoopShift(header, loc.blk->next, loop_var, shift, top);
							init_i->arg1 = VarAddNMod(init_i->arg1, shift, top);
						}
						
					}
				}
			}
		}
	}
}
