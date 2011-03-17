/*

Loop optimalization routines

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

void LoopInsertPrologue(Var * proc, InstrBlock * header, InstrOp op, Var * result, Var * arg1, Var * arg2)
{

	InstrBlock * blk;
	Instr * i;

	blk = FindLoopDominator(proc, header);

	i = blk->last;

	// Loops with condition at the beginning may start with jump to condition
	// We need to insert the initialization code before this jump.

	if (i->op == INSTR_GOTO) {
		i = i->prev;
	} else {
		i = NULL;
	}
	InstrInsert(blk, i, op, result, arg1, arg2);
}

void OptimizeLoopInvariants(Var * proc, InstrBlock * header, InstrBlock * end)
/*
Purpose:
	Move loop invariants before the loop.
	Instruction is considered loop invariant if:

	1. instruction is not jump
	2. it has result
	3. it's result is set exactly once in the loop (result->write == 1) 
	4. it's result is not OUT
	5. it's result is not loop dependent (meaning none of it's arguments is loop dependent)
	6. none of it's arguments is IN
	7. it's result is not live after the loop (this condition may be made weaker)
	8. loop prologue does not end with jump instruction (it is not loop with condition at the beginning)
*/
{
	InstrBlock * blk;
	InstrBlock * exit, *  prologue;
	Instr * i;
	Var * result;
	UInt32 n;

//	printf("========== Invariants ================\n");
//	PrintProc(proc);

	prologue = NULL;
	exit = end->next;

	VarResetUse();
	InstrVarUse(header, exit);
	InstrVarLoopDependent(header, exit);
	MarkBlockAsUnprocessed(proc->instr);

	for(blk = header; blk != exit; blk = blk->next) {
		i = blk->first; n=1;
		while(i != NULL) {
			if (i->op == INSTR_LINE) goto next;
			result = i->result;
			if (result != NULL && !VarIsLabel(result) && !OutVar(result) && result->write == 1) {
				if ( (i->arg1 == NULL || (!InVar(i->arg1) && FlagOff(i->arg1->flags, VarLoop|VarLoopDependent)))
					&& (i->arg2 == NULL || (!InVar(i->arg2) && FlagOff(i->arg2->flags, VarLoop|VarLoopDependent)))
				) {

					//TODO: There can be multiple exits!!!!
					if (VarIsLiveInBlock(proc, exit, result) == 0) {
						if (Verbose(proc)) {
							printf("Moving loop invariant #%d/%d: ", blk->seq_no, n); InstrPrint(i);
						}

						//TODO: If there are multiple places we may have come from, or prologue is NULL we must insert new block
						//TODO: Call should count use of arguments as read & write

						LoopInsertPrologue(proc, header, i->op, i->result, i->arg1, i->arg2);
						i = InstrDelete(blk, i);
						continue;
					}
				}
			}
next:
			i = i->next;
			n++;
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
/*
			blk = FindLoopDominator(proc, header);

			i2 = blk->last;

			// Loops with condition at the beginning may start with jump to condition
			// We need to insert the initialization code before this jump.

			if (i2->op == INSTR_GOTO) {
				i2 = i2->prev;
			} else {
				i2 = NULL;
			}

			InstrInsert(blk, i2, INSTR_LET, top_reg, top_var, NULL);
*/
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

void OptimizeLoops(Var * proc)
/*
Purpose:
	Find loops in flow graph and call optimization for it.
*/
{
	InstrBlock * nb, * header;

	NumberBlocks(proc->instr);

	if (Verbose(proc)) {
		printf("========== Optimize loops ==============\n");
		PrintProc(proc);
	}

	// Test for each block, if it is end of an loop

	for(nb = proc->instr; nb != NULL; nb = nb->next) {
		if (nb->cond_to != NULL) {
			if (nb->cond_to->seq_no <= nb->seq_no) {
				nb->cond_to->loop_end = nb;
			}
		}

		if (nb->to != NULL) {
			if (nb->to->seq_no <= nb->seq_no) {
				nb->to->loop_end = nb;
			}
		}
	}

	for(nb = proc->instr; nb != NULL; nb = nb->next) {
		header = NULL;
		if (nb->cond_to != NULL && nb->cond_to->loop_end == nb) header = nb->cond_to;
		if (nb->to != NULL && nb->to->loop_end == nb) header = nb->to;

		if (header != NULL) {
			if (Verbose(proc)) {
				printf("*** Loop %d..%d\n", header->seq_no, nb->seq_no);
			}
			OptimizeLoop(proc, header, nb);
//			OptimizeLoopInvariants(proc, header, nb);
		}

	}
}
