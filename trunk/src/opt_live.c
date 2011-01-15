/*

Dead store elimination & next use information computation

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

Removal of assignments to variables that are not subsequently read, either because the lifetime of the variable ends 
or because of a subsequent assignment that will overwrite the first value.

*/

#include "language.h"

// We use variable flag VarLive to mark the variable as used.
// src_i is used as next_use information for variable.

#define VarMarkLive(V) VarMark((V), VarLive)
#define VarMarkDead(V) VarMark((V), VarDead)

//TODO: Could we use special value of next_use as live information?
//      I.e. next_use != NULL means dead, otherwise live

void VarMark(Var * var, VarFlags state)
/*
Purpose:
	Mark variable as used or dead.

	Reference to array using variable may not be marked as dead or alive, as 
	it may in fact reference other variable than we think in case the variable is changes.
Input:
	var		Variable to mark as live or dead
	state	VarLive or VarDead
*/
{
	Var * var2;
	if (var == NULL) return;

	// If this is array access variable, mark indices as live (used)

	if (var->mode == MODE_DEREF) {
		VarMarkLive(var->var);
	} if (var->mode == MODE_ELEMENT) {

		if (var->adr->mode == MODE_DEREF) {
			VarMarkLive(var->adr);
		}

		// Reference uses the array variable
//		if (var->submode == SUBMODE_REF) {
//			VarMarkLive(var->adr);
//		}
			
		VarMarkLive(var->var);

		// Array references with variable indexes are always live

		if (var->var->mode != MODE_CONST) state = VarLive;
	} else {
		// If variable is alias for some other variable, mark the other variable too
		if (var->adr != NULL) {
			if (var->adr->mode == MODE_VAR) {
				VarMark(var->adr, state);
			}
		}
	}

	var->flags = (var->flags & ~VarLive) | state;

	// Each element, which has this variable as an array is marked same
	FOR_EACH_VAR(var2)
		if (var2->mode == MODE_ELEMENT) {
			if (var2->adr == var) {
				var2->flags = (var2->flags & ~VarLive) | state;
			}
		}
	NEXT_VAR
}

// 0 dead
// 1 live
// 2 undecided

void MarkBlockAsUnprocessed(InstrBlock * block)
{
	InstrBlock * nb;
	for(nb = block; nb != NULL; nb = nb->next) {
		nb->processed = false;
	}
}

UInt8 VarIsLiveInBlock(Var * proc, InstrBlock * block, Var * var)
/*
Purpose:
	Test, if variable is live after specified block.
*/
{
	Instr * i;
	UInt8 res1, res2;
	if (block == NULL) return 0;
	if (block->processed) return 2;

	block->processed = true;

	res1 = res2 = 2;

	for (i = block->first; i != NULL; i = i->next) {
		if (i->arg1 == var || i->arg2 == var) { res1 = 1; goto done; }
		if (i->result == var) { res1 = 0; goto done;}
		if (i->op == INSTR_LET_ADR) {
			if (VarIsArrayElement(i->arg1)) {
				if (var->mode == MODE_ELEMENT) {
					if (var->adr == i->arg1->adr) { res1 = 1; goto done; }
				}
			}
		}
	}

	// If block ends and the variable is one of results, it is live

	if (block->next == NULL && FlagOn(var->submode, SUBMODE_ARG_OUT) && var->scope == proc) return 1;

	// We haven't encountered the variable, let's try blocks following this block

	res1 = VarIsLiveInBlock(proc, block->to, var);
	if (res1 != 1) {
		res2 = VarIsLiveInBlock(proc, block->cond_to, var);
	}

	if (res1 == 1 || res2 == 1) { res1 = 1; goto done; }
	if (res1 == 0 && res2 == 0) res1 = 0;
done:
//	block->processed = false;
	return res1;
}

void MarkProcLive(Var * proc)
{
	//TODO: We should actually step through the code, as procedure may access and modify global variables
	Var * var;

	var = VarFirstLocal(proc);
	while(var != NULL) {
		if (FlagOff(var->submode, SUBMODE_ARG_IN | SUBMODE_ARG_OUT)) {
			VarMarkDead(var);
		} else {
			if (FlagOn(var->submode, SUBMODE_ARG_OUT)) VarMarkDead(var);
		}
		var = VarNextLocal(proc, var);
	}

	// Procedure may use same variable both for input and output (for example using aliasing)
	// a:proc >x@_a <y@_a
	// In such case, marking variable as live has precence.

	for (var = VarFirstLocal(proc); var != NULL; var = VarNextLocal(proc, var)) {
		if (FlagOn(var->submode, SUBMODE_ARG_IN)) VarMarkLive(var);
	}
}

Bool VarDereferences(Var * var)
{
	if (var != NULL) {
		if (var->mode == MODE_DEREF) return true;
		if (var->mode == MODE_ELEMENT) return VarDereferences(var->adr) || VarDereferences(var->var);
	}
	return false;
}

Bool VarIsLocal(Var * var, Var * scope)
{
	while (var != NULL) {
		if (var->scope == scope) return true;
		var = var->scope;
	}
	return false;
}

Bool OptimizeLive(Var * proc)
{
	Bool modified = false;
	InstrBlock * blk;
	Instr * i;
	Var * var, * result;
	UInt32 n = 0, blk_n = 0;
	InstrOp op;

	if (VERBOSE) {
		printf("------ optimize live ------\n");
		PrintProc(proc);
	}

	for(blk = proc->instr; blk != NULL; blk = blk->next) {
		
		if (VERBOSE) {
			n += blk_n;
			blk_n = 0;
			for(i = blk->last; i != NULL; i = i->prev) blk_n++;
			n += blk_n;
		}

		// At the beginning, all variables are dead (except procedure output variables for tail blocks)

		FOR_EACH_VAR(var)

			if (StrEqual(var->name, "x") /* && var->var->mode == MODE_CONST && var->var->n == 0*/) {
				printf("");
			}

			// Non-local variables are always considered live
			if (!VarIsLocal(var, proc)) {
				SetFlagOn(var->flags, VarLive);
			} else {
				// Procedure output argument are live in last block
				if (blk->to == NULL && var->submode == SUBMODE_ARG_OUT) {
					SetFlagOn(var->flags, VarLive);
				// Local variables in last block
				} else if (blk->to == NULL && FlagOff(var->submode, SUBMODE_ARG_OUT) /*&& VarIsLocal(var, proc)*/) {
					SetFlagOff(var->flags, VarLive);

				// Out variables are always live
				} else if (FlagOn(var->submode, SUBMODE_OUT)) {
					SetFlagOn(var->flags, VarLive);
				} else {
					SetFlagOff(var->flags, VarLive);
					MarkBlockAsUnprocessed(proc->instr);
					if (VarIsLiveInBlock(proc, blk->to, var) == 1 || VarIsLiveInBlock(proc, blk->cond_to, var) == 1) {
						SetFlagOn(var->flags, VarLive);
					}
				}
			}
		NEXT_VAR

		for(i = blk->last; i != NULL; i = i->prev, n--) {

			op = i->op;
			if (op == INSTR_LINE) continue;

			result = i->result;
			if (result != NULL) {
				if (op != INSTR_LABEL && op != INSTR_REF && op != INSTR_CALL) {
					if (FlagOff(result->flags, VarLive) && !VarIsLabel(result) && !VarIsArray(result) && !VarDereferences(result) && FlagOff(result->submode, /*SUBMODE_REF|*/SUBMODE_OUT)) {
						if (VERBOSE) {
							printf("removed dead %ld:", n); InstrPrint(i);
						}
						if ((i->arg1 == NULL || FlagOff(i->arg1->submode, SUBMODE_IN_SEQUENCE)) && (i->arg2 == NULL || FlagOff(i->arg2->submode, SUBMODE_IN_SEQUENCE))) {
							i = InstrDelete(blk, i);
							modified = true;
							if (i == NULL) break;		// we may have removed last instruction in the block
							continue;
						}
					}
				}
			}

			//===== Mark result as dead
			//      Result must be marked first dead first, to properly handle instructions like x = x + 1

			if (result != NULL) {
				// For reference, the adr is marked live, reference is not marked dead, as we may not know, what adr there is
//				if (FlagOn(result->submode, SUBMODE_REF)) {
//					VarMarkLive(result->adr);
//				} else 
				if (FlagOff(result->submode, SUBMODE_OUT)) {
					VarMarkDead(result);
				}
				// Array indexes are marked live (even if value itself is marked dead)
//				if (result->mode == MODE_ELEMENT) {
//					VarMarkLive(result->var);
//				}
				result->src_i = NULL;			// next use
			}

			//===== Mark arguments as live (used)

			// For procedure call, we mark as live variable all variables used in that procedure
			if (op == INSTR_CALL) {
				MarkProcLive(i->result);
			} else {
				VarMarkLive(i->arg1);
				VarMarkLive(i->arg2);

				// Mark next use of the parameter argument to this instruction

				if (i->arg1 != NULL) {
					i->next_use[1] = i->arg1->src_i;
					i->arg1->src_i = i;
				}
				if (i->arg2 != NULL) {
					i->next_use[2] = i->arg2->src_i;
					i->arg2->src_i = i;
				}
			}
		}
	}
	return modified;
}
