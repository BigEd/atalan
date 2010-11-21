/*

Dead store elimination

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

removal of assignments to variables that are not subsequently read, either because the lifetime of the variable ends or because of a subsequent assignment that will overwrite the first value.

*/

#include "language.h"
extern Var * VARS;		// global variables

// flag is set to 1, if the variable is live

void VarMark(Var * var, Bool state)
/*
Purpose:
	Mark variable as used or dead.

	Reference to array using variable may not be marked as dead or alive, as 
	it may in fact reference other variable than we think in case the variable is changes.
*/
{
	Var * var2;
	if (var == NULL) return;

	// If this is array access variable, mark indices as live (used)

	if (var->mode == MODE_ELEMENT) {

		// Reference uses the array variable
		if (var->submode == SUBMODE_REF) {
			VarMark(var->adr, 1);
		}
			
		VarMark(var->var, 1);

		// Array references with variable indexes are always live

		if (var->var->mode != MODE_CONST) state = 1;
	} else {
		if (var->adr != NULL) {
			if (var->adr->mode == MODE_VAR) {
				VarMark(var->adr, state);
			}
		}
	}

	var->flags = state;

	FOR_EACH_VAR(var2)
		if (var2->mode == MODE_ELEMENT) {
			if (var2->adr == var) {
				var2->flags = state;
			}
		}
	NEXT_VAR
}

void VarMarkLive(Var * var)
{
	VarMark(var, 1);
}

void VarMarkDead(Var * var)
{
	VarMark(var, 0);
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
	Var * var;

	var = VarFirstLocal(proc);
	while(var != NULL) {
		if (FlagOff(var->submode, SUBMODE_ARG_IN | SUBMODE_ARG_OUT)) {
			VarMarkDead(var);
		} else {
			if (FlagOn(var->submode, SUBMODE_ARG_IN)) VarMarkLive(var);
			if (FlagOn(var->submode, SUBMODE_ARG_OUT)) VarMarkDead(var);
		}
		var = VarNextLocal(proc, var);
	}
}

Bool VarIsOut(Var * var)
{
	if (var == NULL) return false;
	if (FlagOn(var->submode, SUBMODE_OUT)) return true;
	if (var->mode == MODE_ELEMENT) {
		return VarIsOut(var->adr);
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

			if (var->mode == MODE_ELEMENT && StrEqual(var->adr->name, "count") && var->var->mode == MODE_CONST && var->var->n == 0) {
				printf("");
			}

			if (blk->to == NULL && var->submode == SUBMODE_ARG_OUT) {
				var->flags = 1;		// procedure output arguments are live in last block
			} else if (blk->to == NULL && !VarIsOut(var)) {
				var->flags = 0;
			} else if (VarIsOut(var)) {
				var->flags = 1;		// out variables are always live, procedure output arguments are line in last block
			} else {
				var->flags = 0;
				MarkBlockAsUnprocessed(proc->instr);
				if (VarIsLiveInBlock(proc, blk->to, var) == 1 || VarIsLiveInBlock(proc, blk->cond_to, var) == 1) {
					var->flags = 1;
				}
			}
		NEXT_VAR

		// Skip the data part (data etc.)
		// At the end of main procedure (but possibly in others too) there is a set of constant data initialization
		// and allocation.
		// There are label instructions, to which it will not be jumped, but that are used to reference the data. (data labels)
		// These labels would cause marking all variables as live, so we skip that part of code.
/*
		for(i = blk->last; i != NULL; i = i->prev, n--) {
			op = i->op;
			if (op != INSTR_INCLUDE && op != INSTR_LINE && op != INSTR_LABEL && op != INSTR_DATA && op != INSTR_PTR && op != INSTR_ALLOC 
				&& op != INSTR_FILE && op != INSTR_ALIGN && op != INSTR_ARRAY_INDEX) break;
		}
*/
		for(i = blk->last; i != NULL; i = i->prev, n--) {

			op = i->op;
			if (op == INSTR_LINE) continue;

			// Mark arguments as live (used)
			// We must first mark the arguments, as we do not want the instructions like add x, x, 1 to be removed (x is argument)

			result = i->result;
			if (result != NULL) {
				if (op != INSTR_LABEL && op != INSTR_REF && op != INSTR_CALL) {
					if (result->flags == 0 && !VarIsLabel(result) && !VarIsArray(result) && FlagOff(result->submode, SUBMODE_REF|SUBMODE_OUT)) {
						if (VERBOSE) {
							printf("removed dead %ld:", n); InstrPrint(i);
						}
						i = InstrDelete(blk, i);
						modified = true;
						if (i == NULL) break;		// we may have removed last instruction in the block
						continue;
					}
				}
			}

			// Mark result as dead

			if (result != NULL) {
				// For reference, the adr is marked live, reference is not marked dead, as we may not know, what adr there is
				if (FlagOn(result->submode, SUBMODE_REF)) {
					VarMarkLive(result->adr);
				} else if (!VarIsOut(result)) {
					VarMarkDead(result);
				}
				// Array indexes are marked live (even if value itself is marked dead)
				if (result->mode == MODE_ELEMENT) {
					VarMarkLive(result->var);
				}
			}


			if (op == INSTR_CALL) {
				MarkProcLive(i->result);
			} else {
				// Mark arguments as live (used)
				VarMarkLive(i->arg1);
				VarMarkLive(i->arg2);
			}
		}
	}
	return modified;
}
