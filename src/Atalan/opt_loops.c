/*

Loop optimization routines

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"

Var * FindMostUsedVar()
/*
Purpose:
	Find most used variable in loop.
*/
{
	UInt32 max_cnt, cnt;
	Var * top_var, * var;

	max_cnt = 0; top_var = NULL;

	FOR_EACH_VAR(var)
		//TODO: Exclude registers, in or out variables
		//Exclude:
		//   - labels
		//   - variables with specified address (typically registers)
		//   - constants
		//   - arrays (we may optimize array element access though)
		//   - variables whose size is bigger than register

		cnt = var->read + var->write;
		if (cnt > 0) {

//			if (VERBOSE) {
//				printf("Var: "); PrintVar(var);
//			}

			if (var->mode == MODE_ELEMENT) {
				if (FlagOn(var->adr->submode, SUBMODE_IN | SUBMODE_OUT | SUBMODE_REG)) continue;
				if (var->var->mode != MODE_CONST) continue;
//				continue;
				// If array index is loop dependent, do not attempt to replace it with register
//				if (FlagOn(var->var->flags, VarLoopDependent)) continue;
			}

			if (FlagOff(var->submode, SUBMODE_IN | SUBMODE_OUT | SUBMODE_REG /*| SUBMODE_REF*/) 
//			 && (var->mode != MODE_ELEMENT || FlagOff(var->adr->submode, SUBMODE_IN | SUBMODE_OUT | SUBMODE_REG))
			 && var->mode != MODE_CONST 
			 && var->mode != MODE_DEREF
			 && var->type != NULL && var->type->variant != TYPE_PROC
			 && !VarIsLabel(var) 
			 && !VarIsArray(var)
			 && FlagOff(var->flags, VarLoopDependent)
			) {
				if (cnt > max_cnt) {
					max_cnt = cnt;
					top_var = var;
				}
			}
		}
	NEXT_VAR

	return top_var;
}

void InstrVarLoopDependent(InstrBlock * code, InstrBlock * end)
/*
	Compute dependency of variables on loop variables.
*/
{
	Instr * i;
	Var * result;
	UInt16 flags;
	Bool  modified;

	InstrBlock * blk;
	
	do {
		modified = false;

		for(blk = code; blk != end; blk = blk->next) {
			for(i = blk->first; i != NULL; i = i->next) {

				if (i->op == INSTR_LINE) continue;

				result = i->result;
				if (result != NULL) {
					if (FlagOff(result->flags, VarLoopDependent)) {
						// In case of direct let, we want to distribute VarLoop flag too
						if (i->op == INSTR_LET) {
							flags = i->arg1->flags & (VarLoopDependent|VarLoop);
							if (result->flags != (result->flags | flags)) {
								result->flags |= flags;
								modified = true;
							}
						} else {
							flags = 0;
							if (i->arg1 != NULL && i->arg1 != result) flags |= i->arg1->flags;
							if (i->arg2 != NULL && i->arg2 != result) flags |= i->arg2->flags;
							if (FlagOn(flags, VarLoop | VarLoopDependent)) {
								result->flags |= VarLoopDependent;
								modified = true;
							}
						}
					}
				}
			}
		}
	} while(modified);
}

InstrBlock * FindLoopDominator(Var * proc, InstrBlock * header)
{
	InstrBlock * prev = NULL, * blk;
	for(blk = proc->instr; blk != header; blk = blk->next) prev = blk;
	return prev;
}

UInt32 NumberBlocks(InstrBlock * block)
{
	UInt32 seq_no;
	InstrBlock * nb;

	seq_no = 1;
	for(nb = block; nb != NULL; nb = nb->next) {
		nb->seq_no = seq_no++;
	}
	return seq_no;
}

Int32 UsageQuotient(InstrBlock * header, InstrBlock * end, Var * top_var, Var * reg, Bool * p_init)
//===== Compute usage quotient (q)
//      The bigger the value, the more suitable the register is
//      0 means no gain, >0 means using the register would lead to less optimal code
{
	Var * prev_var;
	Int32 q, q1;
	InstrBlock * blk, * exit;
	Instr * i2, ti;
	UInt32 n2;
	Bool spill;

	exit = end->next;

	*p_init = false;

	q = 0;
	reg->current_val = top_var;	// we expect initialization by top_var before loop
	prev_var = NULL;			// previous variable contained in the register
								// this variable must be loaded, when instruction using the register is encountered
								// (if it is not top_var)

	// Compute usage quotient
	for(blk = header; blk != exit; blk = blk->next) {
		for(i2 = blk->first, n2 = 0; i2 != NULL; i2 = i2->next, n2++) {

			if (i2->op == INSTR_LINE) continue;

			// Call to subroutine destroys all registers, there will be spill
			if (i2->op == INSTR_CALL) {
				q = 1;
				goto done;
			}
			// If there is jump except last instruction
			if (IS_INSTR_JUMP(i2->op) && (i2 != blk->last || blk != end)) {
				*p_init = true;
			}

			if (i2->op == INSTR_LET) {
				if (i2->result == top_var && i2->arg1 == reg) {
					// Register currently contains the replaced variable
					if (reg->current_val == top_var) {
						q -= 3;
						continue;
					}
				}

				if (i2->result == reg && i2->arg1 == top_var) {
					if (reg->current_val == top_var) {
						q -= 3;
						continue;
					} else {
						prev_var = reg->current_val;
						reg->current_val = i2->arg1;
						continue;
					}
				}
			}

			// If current instruction uses the register, and it is
			// we need to save the register and load some other

			if (InstrUsesVar(i2, reg) && reg->current_val != top_var) {
				if (prev_var != NULL) {
					q += 3;
				}
			}

			// register value becomes unknown (has different value than top_val)
			if (i2->result == reg) {
				reg = NULL;
			}

			memcpy(&ti, i2, sizeof(Instr));

			q1 = VarTestReplace(&ti.result, top_var, reg);
			q1 += VarTestReplace(&ti.arg1, top_var, reg);
			q1 += VarTestReplace(&ti.arg2, top_var, reg);

			// If the instruction was changed (it used top_var),
			// test, whether we are able to compile it (some register/adress mode combinations must not be available)

			if (q1 != 0) {
				if (EmitRule(&ti) == NULL) {
//					if (VERBOSE) {
//						printf("     %ld: invalid code\n", n2);
//					}
					q = 1;		// do not use this register, as invalid code would get generated
					goto done;
				}
				// If there is currently not the value of top_var in replaced register,
				// we would have to load it
				if (reg->current_val != top_var && prev_var != NULL) {
//					if (VERBOSE) {
//						printf("     %ld: load\n", n2);
//					}
					q += 3;
				}
				q -= q1;
//				if (VERBOSE) {
//					printf("     %ld: usage\n", n2);
//				}
			}

			// Will it be necessary to spill?
			// We use the variable (array) that is stored to register

			spill = InstrSpill(i2, top_var);
			if (spill) {
				q += 4;
//				if (VERBOSE) {
//					printf("     %ld: spill\n", n2);
//				}

			}
		} // instr
	} // blk
	// Value of register is not known at the end of loop, we need to load it before first use
	if (reg == NULL) {
		q += 3;
	}
done:
	return q;
}


typedef struct {
	InstrBlock * header;
	InstrBlock * end;
} Loop;


void LoopPreheader(Var * proc, Loop * loop, Loc * loc)
{
	loc->blk = FindLoopDominator(proc, loop->header);
	loc->i = loc->blk->last;

	if (loc->i->op != INSTR_GOTO) {
		loc->i = NULL;
	}

}

void LoopMoveToPrologue(Var * proc, InstrBlock * header, InstrBlock * from, Instr * first, Instr * last)
{
	InstrBlock * blk;
	Instr * i;
	
	blk = FindLoopDominator(proc, header);

	// Header may be terminating point of multiple loops.
	// We must detect this situation.

	if (blk->callers != NULL) {
	}


	i = blk->last;

	// Loops with condition at the beginning may start with jump to condition
	// We need to insert the initialization code before this jump.

	if (i->op == INSTR_GOTO) {
//		i = i->prev;
	} else {
		i = NULL;
	}

	InstrMoveCode(blk, i, from, first, last);

//	printf("========== move to prologue ===========\n");
//	PrintProc(proc);
}

void LoopInsertPrologue(Var * proc, InstrBlock * header, InstrOp op, Var * result, Var * arg1, Var * arg2)
{
	Instr * i = MemAllocStruct(Instr);
	i->op = op;
	i->result = result;
	i->arg1 = arg1;
	i->arg2 = arg2;
	LoopMoveToPrologue(proc, header, NULL, i, i);
}

Bool VarIsLoopDependent(Var * var, VarSet * liveset)
{
	if (var == NULL) return false;
	if (VarSetFind(liveset, var)) return false;
	if (FlagOn(var->flags, VarLoop|VarLoopDependent)) return true;
	if (var->mode != MODE_CONST) {
		if (VarIsLoopDependent(var->adr, liveset)) return true;
		if (VarIsLoopDependent(var->adr, liveset)) return true;
	}
	return false;
}

void DefsAdd(Defs * defs, InstrBlock * blk, Instr * i)
{
	Loc * def = &defs->defs[defs->count];
	def->blk = blk;
	def->i   = i;
	defs->count++;
}

void DefsInit(Defs * defs)
{
	defs->count = 0;
}

Bool LoopContainsBlock(Loop * loop, InstrBlock * blk)
{
	return loop->header->seq_no <= blk->seq_no && loop->end->seq_no >= blk->seq_no;
}

void ReachingDefsBlock(Var * var, Loc * loc, InstrBlock * blk, Instr * instr, Defs * defs)
{
	Instr * i;
	InstrBlock * caller;

	if (blk == NULL || blk->processed) return;
	if (instr == NULL) {
		i = blk->last;
	} else {
		i = instr->prev;
	}

	// Definition of the variable may be in this block
	for(; i != NULL; i = i->prev) {
		if (i->op == INSTR_LINE) continue;
		if (i->result == var) {
			DefsAdd(defs, blk, i);
			return;
		}
	}

	// Or it may be in some calling blocks
	
	// If we haven't parsed whole block, do not mark it as processed to allow parsing rest of the instructions.
	// In case this is loop block, we will parse it again, but parsing will stop at defining instruction.
	if (instr == NULL) {
		blk->processed = true;
	}
	ReachingDefsBlock(var, loc, blk->from, NULL, defs);

	for(caller = blk->callers; caller != NULL; caller = caller->next_caller) {
		ReachingDefsBlock(var, loc, caller, NULL, defs);
	}
}

void ReachingDefs(Var * proc, Var * var, Loc * loc, Defs * defs)
{
	MarkBlockAsUnprocessed(proc->instr);
	DefsInit(defs);
	ReachingDefsBlock(var, loc, loc->blk, loc->i, defs);
}

void NextDefsBlock(Var * var, Loc * loc, InstrBlock * blk, Instr * instr, Defs * defs)
{
	Instr * i;

	if (blk == NULL || blk->processed) return;

	// If the starting instruction is not defined, we use first instruction, otherwise we start from following instruction
	// (the specified one is the one we search for).
	if (instr == NULL) {
		i = blk->first;
		blk->processed = true;
	} else {
		i = instr->next;
	}

	// Next definition may be in this block
	for(; i != NULL; i = i->next) {
		if (i->op == INSTR_LINE) continue;
		if (i->result == var && !VarUsesVar(i->arg1, i->result) && !VarUsesVar(i->arg2, i->result)) {
			DefsAdd(defs, blk, i);
			return;
		}
	}

	// Or it may be in some calling blocks
	
	// If we haven't parsed whole block, do not mark it as processed to allow parsing rest of the instructions.
	// In case this is loop block, we will parse it again, but parsing will stop at defining instruction.
//	if (instr == NULL) {
//		blk->processed = true;
//	}
	NextDefsBlock(var, loc, blk->to, NULL, defs);
	NextDefsBlock(var, loc, blk->cond_to, NULL, defs);
}

void NextDefs(Var * proc, Var * var, Loc * loc, Defs * defs)
{
	MarkBlockAsUnprocessed(proc->instr);
	DefsInit(defs);
	NextDefsBlock(var, loc, loc->blk, loc->i, defs);
}

Bool VarInvariant(Var * proc, Var * var, Loc * loc, Loop * loop)
{
	Defs defs;
	UInt16 n;
	Bool out_of_loop;

	if (var == NULL) return true;
	if (var->mode == MODE_CONST) return true;
	if (InVar(var)) return false;

	// For array access, array adr is constant (except referenced array), important is index change
	if (var->mode == MODE_ELEMENT) {
		return VarInvariant(proc, var->var, loc, loop);
	}

	DefsInit(&defs);
	ReachingDefs(proc, var, loc, &defs);

	// 0 would be undefined variable

	out_of_loop = true;
	for(n=0; n<defs.count; n++) {
		if (LoopContainsBlock(loop, defs.defs[n].blk)) {
			out_of_loop = false;
			break;
		}
	}

	if (out_of_loop) return true;
	return defs.count == 1 && FlagOn(defs.defs[0].i->flags, InstrInvariant);
}

Bool VarLoopDepBlock(Var * var, Loc * loc, InstrBlock * blk, Instr * instr)
{
	Instr * i;

	if (blk == NULL || blk->processed) return false;

	// If the starting instruction is not defined, we use first instruction, otherwise we start from following instruction
	// (the specified one is the one we search for).
	if (instr == NULL) {
		i = blk->first;
		blk->processed = true;
	} else {
		i = instr->next;
	}

	// Next definition may be in this block
	for(; i != NULL; i = i->next) {
		if (i->op == INSTR_LINE) continue;
		if (FlagOff(i->flags, InstrInvariant) && (VarUsesVar(i->arg1, var) || VarUsesVar(i->arg2, var))) {
			return true;
		}
		// This instruction sets new value to variable
		if (i->result == var) return false;
	}

	return VarLoopDepBlock(var, loc, blk->to, NULL) || VarLoopDepBlock(var, loc, blk->cond_to, NULL);
}

Bool VarLoopDep(Var * proc, Var * var, Loc * loc, Loop * loop)
{

	if (var == NULL) return false;
	if (var->mode == MODE_CONST) return false;
	if (InVar(var)) return true;

	// For array access, array adr is constant (except referenced array), important is index change
	if (var->mode == MODE_ELEMENT) {
		return VarLoopDep(proc, var->var, loc, loop);
	}

	MarkBlockAsUnprocessed(proc->instr);
	
	return VarLoopDepBlock(var, loc, loc->blk, loc->i);

}

Bool VarInvariant2(Var * proc, Var * var, Loc * loc, Loop * loop)
{
	Defs defs;

	NextDefs(proc, var, loc, &defs);

	// In loop, number of definitions may be 0 in case of loop variables
	//:::::::::::::::::::::
	//   x = 0
	//l@
	//   x <- x + 1
	//   if x < 10 goto l@
	//:::::::::::::::::::::
	// Instructions like x = x + 1, are not considered definitions (they modify the value of x, but do not define it).
	// Zero killer must therefore mean the instruction is not invariant.

	if (defs.count == 0) return false;

	if (defs.count == 1) {
//		if (defs.defs[0].i   == loc->i) return true;
		if (defs.defs[0].blk == loc->blk) return true;
	}
	return false;
}

void PrintLoopInvariants(Loop * loop)
{
	InstrBlock * blk, * exit;
	Instr * i;
	UInt32 n;

	exit = loop->end->next;

	for(blk = loop->header; blk != exit; blk = blk->next) {
		i = blk->first; n = 1;
		while(i != NULL) {
			printf("#%d/%d ", blk->seq_no, n);
			if (FlagOn(i->flags, InstrInvariant)) {
				printf("+");
			} else if (FlagOn(i->flags, InstrLoopDep)) {
				printf("-");
			} else {
				printf(" ");
			}
			InstrPrint(i);
			i = i->next; n++;
		}
	}
}

void OptimizeLoopInvariants(Var * proc, Loop * loop)
{
	InstrBlock * blk, * exit;
	Instr * i, * i2;
	Bool change;
	Loc loc, preheader;
	UInt32 n;

//	printf("========== Invariants ================\n");
//	PrintProc(proc);

	exit = loop->end->next;

	//=== Mark all instructions as variant

	for(blk = loop->header; blk != exit; blk = blk->next) {
		for(i = blk->first; i != NULL; i = i->next) {
			i->flags = 0;
		}
	}

	do {
		change = false;
		for (blk = loop->header; blk != exit; blk = blk->next) {
			loc.blk = blk;
			for (i = blk->first, n=1; i != NULL; i = i->next, n++) {
				loc.i = i;
				if (i->op == INSTR_LINE || IS_INSTR_BRANCH(i->op)) continue;
				if (FlagOff(i->flags, InstrInvariant)) {
					if (i->result != NULL && !OutVar(i->result)) {
						if (VarInvariant(proc, i->arg1, &loc, loop) && VarInvariant(proc, i->arg2, &loc, loop)) {
							if (VarInvariant2(proc, i->result, &loc, loop)) {
								SetFlagOn(i->flags, InstrInvariant);
								change = true;
							}
						}
					}
				}
			}
		}
	} while(change);

	// Mark all instructions that are self-referencing and are not marked as constant
/*
	for (blk = loop->header; blk != exit; blk = blk->next) {
		for (i = blk->first, n=1; i != NULL; i = i->next, n++) {
			if (FlagOff(i->flags, InstrInvariant) && i->op != INSTR_LINE) {
				if (VarUsesVar(i->arg1, i->result) || VarUsesVar(i->arg2, i->result)) {
					i->flags = InstrLoopDep;
				}
			}
		}
	}

	printf("-----\n");
	PrintLoopInvariants(loop);
*/

	do {
		change = false;
		for (blk = loop->header; blk != exit; blk = blk->next) {
			loc.blk = blk;
			i2 = NULL;
			for (i = blk->first, n=1; i != NULL; i = i->next, n++) {
//				loc.i = i;
				if (i->op == INSTR_LINE) continue;
				if (i2 != NULL && FlagOn(i2->flags, InstrInvariant) && FlagOff(i->flags, InstrInvariant)) {
					if (i->op == INSTR_LET && i2->op == INSTR_LET && i2->result == i->arg1) {
						SetFlagOff(i2->flags, InstrInvariant);
					}
				}
				i2 = i;
			}
		}
//		if (change) {
//			printf("-----\n");
//			PrintLoopInvariants(loop);
//		}
	} while(change);


//	printf("-----\n");
//	PrintLoopInvariants(loop);

	//==== Move all invariant instructions to preheader
	
	LoopPreheader(proc, loop, &preheader);
	for(blk = loop->header; blk != exit; blk = blk->next) {
		i = blk->first;
		while(i != NULL) {
			i2 = i->next;
			if (FlagOn(i->flags, InstrInvariant)) {
				InstrMoveCode(preheader.blk, preheader.i, blk, i, i);
			}
			i = i2;
		}
	}
}

void OptimizeLoop(Var * proc, InstrBlock * header, InstrBlock * end)
/*
1. Find loop (starting with inner loops)
   - Every jump to label preceding the jump (backjump) forms a label
   - In nested labels, we encounter the backjump first
				<code1>
			l1@
				<code2>
			l2@
				<code3>
				if.. l2@
				<code4>
				if.. l3@
				<code5>

2. Select variable to put to register
   - Most used variable should be used
   - Some variables are already moved to index register (this is considered use too)

3. Compute cost of moving the variable to register

*/
{
	Instr * i2;
	Var * top_var, * reg, * top_reg;
	UInt16 r, regi;
	UInt32 var_size;
	Int32 q, top_q;
	Bool init;
	InstrBlock * blk;
	InstrBlock * exit;

	exit = end->next;

	VarResetUse();
	InstrVarUse(header, exit);
	InstrVarLoopDependent(header, end);

	// When processing, we assign var to register
	for(regi = 1; regi < REG_CNT; regi++) REG[regi]->var = NULL;

//	if (header->seq_no == 41) {
//		printf("");
//	}

	while(top_var = FindMostUsedVar()) {

		if (Verbose(proc)) {
			printf("Most user var: "); PrintVar(top_var);
		}

		top_var->read = top_var->write = 0;
		var_size = VarByteSize(top_var);

		//====== Select the best register for the given variable
		//      let %A,%A   => index -3
		//      use of register instead of variable -1
		//      spill +3

		top_q = 0; top_reg = NULL;

		for(regi = 1; regi < REG_CNT; regi++) {

			reg = REG[regi];
			if (reg->type->range.max == 1) continue;			// exclude flag registers
			if (var_size != VarByteSize(reg)) continue;			// exclude registers with different size
			if (FlagOn(reg->submode, SUBMODE_OUT)) continue;	// out registers can not be used to replace variables
			if (reg->var != NULL) continue;

			if (Verbose(proc)) {
				printf("  Testing register: "); 
				PrintVar(reg);
			}

			q = UsageQuotient(header, end, top_var, reg, &init);

//			if (VERBOSE) {
//				printf("     Quotient: %d\n", q);
//			}

			if (q < top_q) {
				top_q = q;
				top_reg = reg;
			}
		}

		if (top_reg == NULL) continue;

		if (Verbose(proc)) {
			printf("Var: "); PrintVar(top_var);
			printf("Register: "); PrintVar(top_reg);
		}


		//TODO: If there is Let reg = var and var is not top_var, we need to spill store

		//=== Replace the use of registers
				
		// Generate instruction initializing the register used to replace the variable
		// before the start of the loop.
		// We only do this, if the variable is not initialized inside the loop.
		if (FlagOn(top_var->flags, VarUninitialized) || init) {

			LoopInsertPrologue(proc, header, INSTR_LET, top_reg, top_var, NULL);

			top_reg->var = top_var;		// to prevent using the register in subsequent steps
		}
		r = 0;
		
		for(blk = header; blk != exit; blk = blk->next) {
			for(i2 = blk->first; i2 != NULL; i2 = i2->next) {
				if (i2->op == INSTR_LINE) continue;

				if (InstrSpill(i2, top_var)) {
					InstrInsert(blk, i2, INSTR_LET, top_var, top_reg, NULL);
				}
				r += VarReplace(&i2->result, top_var, top_reg);
				VarReplace(&i2->arg1, top_var, top_reg);
				VarReplace(&i2->arg2, top_var, top_reg);

				if (i2->op == INSTR_LET && i2->result == i2->arg1) {
					i2 = InstrDelete(blk, i2);
					if (i2 == NULL) break;
					continue;
				}

			}
		}

		// If we replaced some destination by the register, store the register to destination
		if (r > 0) {
			// There may be exit label as part of the loop
			// We need to spill after it

			if (exit == NULL) {
				exit = MemAllocStruct(InstrBlock);
				end->next = exit;
				end->to   = exit;
			}

			InstrInsert(exit, exit->first, INSTR_LET, top_var, top_reg, NULL);
		}

		if (FlagOn(top_var->flags, VarLoopDependent)) {
			reg->flags |= VarLoopDependent;
		}

	}
}

void MarkLoops(Var * proc)
{
	InstrBlock * blk;
	NumberBlocks(proc->instr);

	for(blk = proc->instr; blk != NULL; blk = blk->next) {
		blk->loop_end = NULL;
	}

	// Test for each block, if it is end of an loop

	for(blk = proc->instr; blk != NULL; blk = blk->next) {
		if (blk->cond_to != NULL) {
			// We jump to some previous spot in the sequence
			if (blk->cond_to->seq_no <= blk->seq_no) {
				blk->cond_to->loop_end = blk;
			}
		}

		if (blk->to != NULL) {
			if (blk->to->seq_no <= blk->seq_no) {
				blk->to->loop_end = blk;
			}
		}
	}

}

void OptimizeLoops(Var * proc)
/*
Purpose:
	Find loops in flow graph and call optimization for it.
*/
{
	Loop loop;
	InstrBlock * nb, * header;


	MarkLoops(proc);

	if (Verbose(proc)) {
		printf("========== Optimize loops ==============\n");
		PrintProc(proc);
	}


	for(nb = proc->instr; nb != NULL; nb = nb->next) {
		header = NULL;
		if (nb->cond_to != NULL && nb->cond_to->loop_end == nb) header = nb->cond_to;
		if (nb->to != NULL && nb->to->loop_end == nb) header = nb->to;

		if (header != NULL) {
			if (Verbose(proc)) {
				printf("*** Loop %d..%d\n", header->seq_no, nb->seq_no);
			}
			loop.header = header;
			loop.end    = nb;
//			OptimizeLoopInvariants(proc, &loop);
			OptimizeLoop(proc, header, nb);
		}

	}
}
