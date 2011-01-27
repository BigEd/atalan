/*

Code optimalization routines

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"

//#define PEEPHOLE_SIZE 2
//extern Bool VERBOSE;
extern Var   ROOT_PROC;

void VarIncRead(Var * var)
{
	if (var != NULL) {
		var->read++;
		if (var->write == 0) {
			// variable should not be marked as uninitialized, if there has been label or jump or call
			var->flags |= VarUninitialized;
		}
		if (var->mode == MODE_ELEMENT) {
			VarIncRead(var->adr);
			VarIncRead(var->var);
		} else if (var->mode == MODE_TUPLE) {
			VarIncRead(var->adr);
			VarIncRead(var->var);			
		} else {
			if (var->adr != NULL) VarIncRead(var->adr);
		}
	}
}

void VarIncWrite(Var * var)
{
	if (var != NULL) {
		var->write++;
		if (var->mode == MODE_ELEMENT) {
			VarIncWrite(var->adr);
			VarIncRead(var->var);
		} else if (var->mode == MODE_TUPLE) {
			VarIncWrite(var->adr);
			VarIncWrite(var->var);
		} else {
			if (var->adr != NULL) VarIncRead(var->adr);
		}
	}
}

void InstrVarUse(InstrBlock * code, InstrBlock * end)
{
	Instr * i;
	Var * result;
	InstrBlock * blk;
	
	for(blk = code; blk != end; blk = blk->next) {
		for(i = blk->first; i != NULL; i = i->next) {

			if (i->op == INSTR_LINE) continue;
			if (i->op == INSTR_CALL) continue;		// Calls are used to compute call chains and there are other rules of computation

			// Writes are registered as last to correctly detect uninitialized variable access
			VarIncRead(i->arg1);
			VarIncRead(i->arg2);

			result = i->result;
			VarIncWrite(result);

			// In instructions like op X, X, ? or op X, ?, X, X is induction variable

			if (result != NULL) {
				if (result == i->arg1 || result == i->arg2) {
					result->flags |= VarLoop;
				}
			}
		}
	}
}

void InstrVarLoopDependent(InstrBlock * code, InstrBlock * end)
/*
	Compute dependency of variables on loop variables.
*/
{
	Instr * i;
	Var * result;
	UInt8 flags;

	InstrBlock * blk;
	
	for(blk = code; blk != end; blk = blk->next) {
		for(i = blk->first; i != NULL; i = i->next) {

			if (i->op == INSTR_LINE) continue;

			flags = 0;
			result = i->result;
			if (result != NULL) {
				if (i->op == INSTR_LET) {
					result->flags &= ~(VarLoopDependent|VarLoop);
					result->flags |= i->arg1->flags & (VarLoopDependent|VarLoop);
				} else {
					// Exclude self dependency
					if (i->arg1 != NULL && i->arg1 != result) flags |= i->arg1->flags;
					if (i->arg2 != NULL && i->arg2 != result) flags |= i->arg2->flags;
					if (FlagOn(flags, VarLoop | VarLoopDependent)) {
						result->flags |= VarLoopDependent;
					}
				}
			}
		}
	}
}

void VarUse()
/*
Purpose:
	Compute use of variables.
*/
{
	Var * proc;
	Var * var;

	VarResetUse();

	FOR_EACH_VAR(proc)
		if (proc->type != NULL && proc->type->variant == TYPE_PROC && proc->read > 0) {
			if (proc->instr != NULL) {
				InstrVarUse(proc->instr, NULL);
			} else {
				// Procedure that has no defined body can still define variables and arguments it uses.
				// We must mark these variables as used, if the procedure is used.
				for(var = VarFirstLocal(proc); var != NULL; var = VarNextLocal(proc, var)) {
					VarIncRead(var);
				}
			}
		}
	NEXT_VAR

	InstrVarUse(ROOT_PROC.instr, NULL);
}

//TODO: Replace variable management (keep array of those variables and reuse them)

Int16 VarTestReplace(Var ** p_var, Var * from, Var * to)
{
	Var * var, * var2;
	Int16 n = 0;
	Int16 n2, n3;
	Var * v2, * v3;

	var = *p_var;
	if (var == from) {
		*p_var = to;
		n++;
	} else {
		//TODO: We should probably alloc new element variable here
		//      because this one may be used somewhere else
		if (var != NULL) {
			if (var->mode == MODE_ELEMENT) {
				v2 = var->adr;
				v3 = var->var;
				n2 = VarTestReplace(&v2, from, to);
				n3 = VarTestReplace(&v3, from, to);

				if (n2 > 0 || n3 > 0) {
					var2 = MemAllocStruct(Var);
					memcpy(var2, var, sizeof(Var));
					var2->adr = v2;
					var2->var = v3;
					var2->next = NULL;

					n += n2 + n3;
					*p_var = var2;
				}
			}
		}
	}
	return n;
}

Int16 VarReplace(Var ** p_var, Var * from, Var * to)
{
	Var * var;
	Int16 n = 0;

	var = *p_var;
	if (var == from) {
		*p_var = to;
		n++;
	} else {
		//TODO: We should probably alloc new element variable here
		//      because this one may be used somewhere else
		if (var != NULL) {
			if (var->mode == MODE_ELEMENT) {
				n += VarReplace(&var->adr, from, to);
				n += VarReplace(&var->var, from, to);
			}
		}
	}
	return n;
}

Bool ArgNeedsSpill(Var * arg, Var * var)
{
	Bool spill = false;
	if (arg != NULL) {
		if (arg->mode == MODE_ELEMENT) {
			if (arg->adr->mode == MODE_DEREF && arg->adr->var == var->adr) {
//			if (arg->adr == var->adr && FlagOn(arg->submode, SUBMODE_REF)) {
				spill = true;
			}			
		}
	}
	return spill;
}

Bool InstrSpill(Instr * i, Var * var)
{
	Bool spill = false;
	//TODO: What about call?
//	if (i->op == INSTR_PRINT || i->op == INSTR_FORMAT) return true;

	if (var->mode == MODE_ELEMENT) {

		spill = ArgNeedsSpill(i->result, var) 
			 || ArgNeedsSpill(i->arg1, var) 
			 || ArgNeedsSpill(i->arg2, var);

		if (i->arg1 == var->adr || i->arg2 == var->adr) {
			spill = true;
		}
	}
	return spill;
}

Bool VarUsesVar(Var * var, Var * test_var)
{
	Bool uses = false;
	if (var != NULL) {
		if (var->mode == MODE_ELEMENT) {
			uses = VarUsesVar(var->var, test_var) || VarUsesVar(var->adr, test_var);
		} else {
			uses = (var == test_var);
		}
	}
	return uses;	
}

Bool InstrUsesVar(Instr * i, Var * var)
{
	if (i == NULL || i->op == INSTR_LINE) return false;

	return VarUsesVar(i->result, var) 
		|| VarUsesVar(i->arg1, var)
		|| VarUsesVar(i->arg2, var);
}

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

/*
void NumberLabels(InstrBlock * blk)
{
	Instr * i;
	UInt32 n;
	Var * lab;

	for(n = 1, i = blk->first; i != NULL; i = i->next, n++) {
		if (i->op == INSTR_LABEL) {
			lab = i->result;
			lab->seq_no = n;
		}
	}
}
*/

UInt32 InstrVarUseCount(Instr * from, Instr * to, Var * var)
/*
Purpose:
	Count number of uses of the variable in the specified block of instructions.
*/
{
	Instr * i;
	UInt32 cnt = 0;
	for(i = from; i != to; i = i->next) {
		if (InstrUsesVar(i, var)) cnt++;
	}
	return cnt;
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
//      0 means no gain, >0 means using the register would lead to worser code
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

	// Compute usage coeficient
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

InstrBlock * FindLoopDominator(Var * proc, InstrBlock * header)
{
	InstrBlock * prev = NULL, * blk;
	for(blk = proc->instr; blk != header; blk = blk->next) prev = blk;
	return prev;
}

void OptimizeLoop(Var * proc, InstrBlock * header, InstrBlock * end)
/*
1. Find loop (starting with inner loops)
   - Every jump to label preceeding the jump (backjump) forms a label
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
//	Instr * ls, * le;		// loop init start and end instruction 
	Var * top_var, * reg, * top_reg;
	UInt16 r, regi;
	UInt32 var_size;
	Int32 q, top_q;
	Bool init;
	InstrBlock * blk;
//	UInt32 lbl_cnt, lbl_loop_cnt;
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
			//TODO: Check, that the label belongs to the loop and not to someone else)
			//      All jumps to the label go from the loop
/*
			while(i2 != NULL && i2->op == INSTR_LABEL) {
				lbl_cnt = InstrVarUseCount(blk->first, NULL, i2->result);
				lbl_loop_cnt = InstrVarUseCount(ls, le, i2->result);						
				if (lbl_loop_cnt > 0) {
					if (lbl_cnt - lbl_loop_cnt > 1) {
						InternalError("label used both by loop interior and exterior");
					}
					i2 = i2->next;
				} else {
					// This is not a loop label
					break;
				}
			}
*/
			if (exit == NULL) {
				exit = MemAllocStruct(InstrBlock);
				end->next = exit;
				end->to   = exit;
			}

			//TODO: Create new block if necessary (there may be NULL)
			//      We may test, if variable is live
			InstrInsert(exit, exit->first, INSTR_LET, top_var, top_reg, NULL);
		}

		if (FlagOn(top_var->flags, VarLoopDependent)) {
			reg->flags |= VarLoopDependent;
		}

		//Reset the usage count of this variable, so next variable will be selected
//		if (VERBOSE) {
//			NumberLabels(blk);
//		}
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
		}

	}
}

void OptimizeCombined(Var * proc)
{
	Bool modified;
	do {
		do {
			modified = OptimizeLive(proc);
			modified |= OptimizeValues(proc);
		} while(modified);

		modified |= OptimizeVarMerge(proc);
	} while(modified);

}

void ProcOptimize(Var * proc)
{
//	if (VERBOSE) {
//		printf("************************************** before blocks\n");
//		PrintProc(proc);
//	}

//	if (StrEqual(proc->name, "changeDirection")) {
//		printf("");
//	}

	if (Verbose(proc)) {
		printf("**************************************\n");
		PrintProc(proc);
	}
	OptimizeCombined(proc);
	OptimizeLoops(proc);
	OptimizeCombined(proc);
}

void ProcInline(Var * proc)
/*
Purpose:
	Replace every call to function that should be inlined with actual code from the function.
*/
{
	Instr * i, * next;
	InstrBlock * blk;
	Var * subproc, * var;
	InstrBlock * subblk;

	for(blk = proc->instr; blk != NULL; blk = blk->next) {
		for(i = blk->first; i != NULL; i = i->next) {
			if (i->op == INSTR_CALL) {
				subproc = i->result;

				// We inline procedures that are called just once and have some body (so we have something to inline)
				if (subproc->read == 1 && subproc->instr != NULL) {
					// To inline a procedure, we just insert it's code at the place of call
					// Parameters have already been stored into appropriate input registers
					// The scope of the procedure should be made local scope in this procedure.

					// When we know, there is just one call, we may link the code of the procedure to new place.
					// If there were multiple places where procedure could be called, we would have to make a copy (and copy of local variables too).
					// There is no such case currently.

					subblk = subproc->instr;

					next = i->next;
					i->next = subblk->first;
					subblk->first->prev = i;

					if (next != NULL) {
						next->prev = subblk->last;
						subblk->last->next = next;
					}

					i = InstrDelete(blk, i);

					// Detach the block from procedure
					MemFree(subblk);
//					subblk->first = NULL;
//					subblk->last = NULL;
//					InstrBlockFree(subblk);
					subproc->instr = NULL;
					subproc->read--;

					// Reown procedure local variables

					for(var = VarFirstLocal(subproc); var != NULL; var = VarNextLocal(subproc, var)) {
						var->scope = proc;
					}

//					PrintProc(proc);
				}
			}
		}
	}
}

