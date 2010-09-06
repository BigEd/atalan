/*

Code optimalization routines

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"

#define PEEPHOLE_SIZE 2
extern Bool VERBOSE;
extern Var * VARS;		// global variables
extern Var   ROOT_PROC;

/*
Bool OptimizePeephole(Var * proc)

Peephole optimization.
Remove following instruction patterns:

- let a,a

TODO: Isn't the following removed by Value analysis?

- let a,b
  let b,a		-> is superfluous

Currently, peephole optimizer is not used, as optimization performed by it are done by
other types of optimization.

{
	Instr * instr[PEEPHOLE_SIZE];
	Instr * i;
	UInt16 cnt = 0;
	UInt32 n;
	Bool modified = false;
	InstrBlock * blk;

	memset(instr, 0, PEEPHOLE_SIZE * sizeof(Instr *));

	n = 0;
	blk = proc->instr;
	for(i = blk->first; i != NULL; i = i->next) {
		n++;
		if (cnt < PEEPHOLE_SIZE) {
			instr[cnt++] = i;
		} else {
			memcpy(&instr[0], &instr[1], sizeof(Instr *) * (PEEPHOLE_SIZE-1));
			instr[PEEPHOLE_SIZE-1] = i;
		}

		// let a,a

		if (i->op == INSTR_LET) {
			if (i->result == i->arg1) {
				if (VERBOSE) {
					printf("Removing %ld:", n);
					InstrPrint(i);
				}
//				i = InstrPrev(&en);
				i = InstrDelete(blk, i);
				modified = true;
				cnt--;
			}
		}

		if (cnt >= 2) {

			// let a,b
			// let b,a		-> is superfluous

			if (instr[0]->op == INSTR_LET && instr[1]->op == INSTR_LET) {
				if (instr[0]->result == instr[1]->arg1 && instr[0]->arg1 == instr[1]->result) {
//					i = InstrPrev(&en);
					i = InstrDelete(blk, i);
					modified = true;
				}
			}
		}
	}
	return modified;
}
*/
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

Bool VarIsLive(Var * var)
{
	Bool live = true;

	if (var->mode == MODE_ELEMENT) {
		live = VarIsLive(var->var);
	}

	return var->flags != 0;
}

Bool OptimizeLive(Var * proc)
{
	Bool modified = false;
	InstrBlock * blk;
	Instr * i;
	Var * var, * result;
	UInt32 n = 0;
	InstrOp op;

	// At the beginning, all variables are dead (except procedure output variables and output variables)

	FOR_EACH_VAR(var)
		if (FlagOff(var->submode, SUBMODE_OUT)) {
			var->flags = 0;
		} else {
			var->flags = 1;
		}
	NEXT_VAR

	if (VERBOSE) {
		blk = proc->instr;
		for(i = blk->last; i != NULL; i = i->prev) n++;
		printf("------ optimize live ------\n");
		PrintProc(proc);
	}

	blk = proc->instr;

	// Skip the data part (data etc.)
	// At the end of main procedure (but possibly in others too) there is a set of constant data initialization
	// and allocation.
	// There are label instructions, to which it will not be jumped, but that are used to reference the data. (data labels)
	// These labels would cause marking all variables as live, so we skip that part of code.

	for(i = blk->last; i != NULL; i = i->prev, n--) {
		op = i->op;
		if (op != INSTR_INCLUDE && op != INSTR_LINE && op != INSTR_LABEL && op != INSTR_DATA && op != INSTR_PTR && op != INSTR_ALLOC 
			&& op != INSTR_FILE && op != INSTR_ALIGN && op != INSTR_ARRAY_INDEX) break;
	}

	for(; i != NULL; i = i->prev, n--) {

		if (i->op == INSTR_LINE) continue;

		// End of basic block, all variables are marked as live

		if (IS_INSTR_JUMP(i->op) || i->op == INSTR_LABEL || i->op == INSTR_CALL || i->op == INSTR_FORMAT || i->op == INSTR_PRINT) {

			// TODO: If this label is not target of any jump instruction (it is data label), it may be removed

			FOR_EACH_VAR(var)
				var->flags = 1;
			NEXT_VAR
		}

		// Mark arguments as live (used)
		// We must first mark the arguments, as we do not want the instructions like add x, x, 1 to be removed (x is argument)

		if (n == 8) {
			n = 8;
		}

		result = i->result;
		if (result != NULL) {
			if (i->op != INSTR_LABEL && i->op != INSTR_CALL && i->op != INSTR_PRINT && i->op != INSTR_FORMAT) {
				if (result->flags == 0 && !VarIsLabel(result) && !VarIsArray(result)) {
					if (VERBOSE) {
						printf("removed dead %ld:", n); InstrPrint(i);
					}
					i = InstrDelete(blk, i);
					modified = true;
					continue;
				}
			}
		}

		// Mark result as dead (unused)

		//TODO: We must mark all array references as dead or live
		if (result != NULL && FlagOff(result->submode, SUBMODE_OUT)) {
			VarMark(result, 0);
		}

		if (i->op == INSTR_CALL) {
			// Function input arguments are marked as live
			// Output arguments are marked as dead
			FOR_EACH_VAR(var)
				if (var->scope == i->result) {
					if (FlagOn(var->submode, SUBMODE_ARG_IN)) {
						var->flags = 1;
					} else {
						var->flags = 0;		// output arguments and local variables are marked as dead
					}
				}
			NEXT_VAR
		} else {
			// Mark arguments as live (used)
			VarMark(i->arg1, 1);
			VarMark(i->arg2, 1);
		}
	}
	return modified;
}

void ResetValues()
{
	Var * var;
	FOR_EACH_VAR(var)
		var->current_val = NULL;
		var->src_i       = NULL;
	NEXT_VAR
}

void ResetValue(Var * res)
/*
Purpose:
	Set all references to specified value to NULL in all values.
*/
{
	Var * var;
	Instr * i;

	FOR_EACH_VAR(var)
		i = var->src_i;
		if (i != NULL) {
			// If source instruction is assignment, set the result to source of assignment argument
			// This makes sure, that we can utilize common values in cases like
			// let _a, 10
			// let p, _a
			// let _a, 20
			// let q, _a
			// add r, p, q

			if (i->op == INSTR_LET) {
				if (i->arg1 == res) {
					i->result->src_i = i->arg1->src_i;
				}
			} else {
				if (i->arg1 == res || i->arg2 == res) {
					var->src_i = NULL;
				}
			}
		}
	NEXT_VAR
}

Bool VarEquivalent(Var * v1, Var * v2)
{
	Bool eq = false;
	// Two references to array using index variable are not same

	if (v1 == v2) {
		if (v1 == NULL) return true;
		if (v1->mode == MODE_ELEMENT) {
			if (v1->var->mode != MODE_CONST) return false;
		}
		return true;
	}
	//TODO: Compare values (there may be named constant with same value as unnamed constant and it will be different var)
	//TODO: Exclude IN, OUT variables
	return eq;
}
/*
Bool InstrEquivalent(Instr * i1, Instr * i2) 

Purpose:
	Test, whether the two instructions have same effect or not.

{
	Bool eq = false;

	// If either instruction is NULL, they are not equivalent
	if (i1 == NULL || i2 == NULL) return false;

	if (i1 == i2) return true;

	if (i1->op == i2->op) {
		eq =  VarEquivalent(i1->arg1, i2->arg1)
		   && VarEquivalent(i1->arg2, i2->arg2)
		   && VarEquivalent(i1->result, i2->result);
	} else {
		// Let is equivalent, if arg1->src_i is same let
		if (i2->op == INSTR_LET) {
			eq = InstrEquivalent(i1, i2->arg1->src_i);
		}
		if (!eq) {
			if (i1->op == INSTR_LET) eq = InstrEquivalent(i1->arg1->src_i, i2);
		}

	}
	return eq;
}
*/

Bool InstrEquivalent(Instr * i1, Instr * i2) 
/*
Purpose:
	Test, whether the two instructions have same effect or not.
	Current state of variables (src_i) is used to perform the test.
*/
{
	Bool eq = false;

	// If either instruction is NULL, they are not equivalent
	if (i1 == NULL || i2 == NULL) return false;

	if (i1 == i2) return true;

	// Let is equivalent, if arg1->src_i is same let
	if (i2->op == INSTR_LET) {
		eq = InstrEquivalent(i1, i2->arg1->src_i);
	}
	if (!eq) {
		if (i1->op == INSTR_LET) eq = InstrEquivalent(i1->arg1->src_i, i2);
	}

	if (!eq) {
		if (i1->op == i2->op) {
			eq =  VarEquivalent(i1->arg1, i2->arg1)
			   && VarEquivalent(i1->arg2, i2->arg2)
			   && VarEquivalent(i1->result, i2->result);
		}
	}
	return eq;
}


Bool OptimizeValues(Var * proc)
/*
   1. If assigning some value to variable (let) and the variable already contains such value, remove the let instruction

	  Do not use if:
         - destination variable is marked as out
	     - source variable is marked as in

   2. Copy propagation a <- b where b <- c  to a <- c

   3. Constant folding (Evaluate constant instructions)
*/
{
	Bool modified, m2;
	Instr * i, i2;
	UInt32 n;
	Var * r, * result, * arg1, * arg2;
	InstrBlock * blk;
	InstrOp op, src_op;

	if (VERBOSE) {
		printf("------ optimize values -----\n");
		PrintProc(proc);
	}
	modified = false;
	blk = proc->instr;
	ResetValues();

	for(n = 1, i = blk->first; i != NULL; i = i->next, n++) {
retry:
		if (i->op == INSTR_LINE) continue;

		// When label is encountered, we must reset all values, because we may come from other places
		if (i->op == INSTR_LABEL || i->op == INSTR_CALL || i->op == INSTR_FORMAT || i->op == INSTR_PRINT) {
			ResetValues();
		}

		result = i->result;

		// We are only interested in instructions that have result
		if (result != NULL) {
			if (IS_INSTR_JUMP(i->op)) {
				// jump istructions use label as result, we do not want to remove jumps
			} else {
				arg1 = i->arg1;

				//TODO: Split OUT & Equivalent
				// If result is equal to arg1, instructions are not equivalent, because they accumulate the result,
				// (for example sequence of mul a,a,2  mul a,a,2
				if (FlagOff(result->submode, SUBMODE_OUT) && result != arg1 && InstrEquivalent(i, result->src_i)) {

					// Array references, that have non-const index may not be removed, as
					// we can not be sure, that the index variable has not changed since last
					// use.
					if (result->mode == MODE_ELEMENT && result->var->mode != MODE_CONST) {
						
					} else {
delete_instr:
						if (VERBOSE) {
							printf("Removing %ld:", n); InstrPrint(i);
						}
						i = InstrDelete(blk, i);
						n++;
						modified = true;
						goto retry;
					}
				} else {
					// Instruction result is set to different value, we must therefore reset
					// all references to this value from all other values.
					ResetValue(result);
				}

				op = i->op;
				m2 = false;

				if (arg1 != NULL && arg1->src_i != NULL && FlagOff(arg1->submode, SUBMODE_IN)) {
					
					src_op = arg1->src_i->op;

					// Try to replace LO b,n LET a,b  => LO a,n LO a,n

					if (src_op == INSTR_LO || src_op == INSTR_HI) {
						if (op == INSTR_LET) {
							op = src_op;
							arg1 = arg1->src_i->arg1;
							m2 = true;
						}
					} else if (src_op == INSTR_LET) {
						// If instruction uses register, do not replace with instruction that does not use it
						if (! (FlagOn(arg1->submode, SUBMODE_REG) && FlagOff(arg1->src_i->arg1->submode, SUBMODE_REG)) ) {
							// Do not replace simple variable with array access
							if (!(arg1->mode == MODE_VAR && arg1->src_i->arg1->mode == MODE_ELEMENT)) {
								arg1 = arg1->src_i->arg1;
								m2 = true;
							}
						}
					}
				}

				arg2 = i->arg2;
				if (arg2 != NULL && FlagOff(arg2->submode, SUBMODE_IN) && arg2->src_i != NULL && arg2->src_i->op == INSTR_LET) {
					arg2 = arg2->src_i->arg2;
					m2 = true;
				}

				// let x,x is always removed
				if (op == INSTR_LET && result == arg1) {
					goto delete_instr;
				}

				if (m2) {
					i2.op = op; i2.result = result; i2.arg1 = arg1; i2.arg2 = arg2;
					if (EmitRule(&i2)) {
						i->op   = op;
						i->arg1 = arg1;
						i->arg2 = arg2;
					}
				}

				//==== Try to evaluate contant instructions
				// We first try to traverse the chain of assignments to it's root.
				// If there is IN variable on the road, we have to stop there
				// and replacing will not be performed.

				r = NULL;
				arg1 = i->arg1;

				if (arg1 != NULL) {
					while (FlagOff(arg1->submode, SUBMODE_IN) && arg1->src_i != NULL && arg1->src_i->op == INSTR_LET) arg1 = arg1->src_i->arg1;
				}

				arg2 = i->arg2;
				if (arg2 != NULL) {
					while(FlagOff(arg2->submode, SUBMODE_IN) && arg2->src_i != NULL && arg2->src_i->op == INSTR_LET) arg2 = arg2->src_i->arg1;
				}

				r = InstrEvalConst(i->op, arg1, arg2);

				// We have evaluated the instruction, change it to LET <result>,r
				if (r != NULL) {
					i2.op = INSTR_LET; i2.result = i->result; i2.arg1 = r; i2.arg2 = NULL;
					if (EmitRule(&i2)) {
						i->op = INSTR_LET;
						i->arg1 = r;
						i->arg2 = NULL;
						result->src_i       = i;
						modified = true;
					}
				}

				result->src_i = i;
			}
		} // result != NULL
	}
	return modified;
}

void VarIncRead(Var * var)
{
	if (var != NULL) {
		var->read++;
		if (var->write == 0) {
			// variable should not be marked as uninitialized, if there has been label or jump or call
			var->flags |= VarUninitialized;
		}
		if (var->mode == MODE_ELEMENT) {
			var->adr->read++;
			var->var->read++;
		}
	}
}

void VarIncWrite(Var * var)
{
	if (var != NULL) {
		var->write++;
		if (var->mode == MODE_ELEMENT) {
			var->adr->write++;
			var->var->read++;
		}
	}
}

void InstrVarUse(InstrBlock * code, Instr * from, Instr * to)
{
	Instr * i;
	Var * result;

	for(i = from; i != to; i = i->next) {

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

void InstrVarLoopDependent(InstrBlock * code, Instr * from, Instr * to)
/*
	Compute dependency of variables on loop variables.
*/
{
	Instr * i;
	Var * result;
	UInt8 flags;

	for(i = from; i != to; i = i->next) {

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

void VarUse()
{
	Var * var;

	VarResetUse();

	FOR_EACH_VAR(var)
		if (var->type != NULL && var->type->variant == TYPE_PROC) {
			if (var->instr != NULL) {
				InstrVarUse(var->instr, var->instr->first, NULL);
			}
		}
	NEXT_VAR

	InstrVarUse(ROOT_PROC.instr, ROOT_PROC.instr->first, NULL);
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

UInt32 VarByteSize(Var * var)
/*
Purpose:
	Return size of variable in bytes.
*/
{
	Int32 lrange;

	Type * type;
	if (var != NULL) {
		type = var->type;
		if (var->mode == MODE_ELEMENT) {
			return 1;		//TODO: Compute size in a better way
		}

		if (type != NULL) {
			switch(type->variant) {
				case TYPE_INT:
					// Some integer variables may have ranges starting higher than 0.
					// (Like 100..300).
					// Byte size of such integers must be computed as if their min range was 0.

					lrange = type->range.min;
					if (lrange > 0) lrange = 0;
					return (type->range.max - lrange) / 256 + 1;
				default: break;
			}
		}
	}
	return 0;
}

Bool ArgNeedsSpill(Var * arg, Var * var)
{
	Bool spill = false;
	if (arg != NULL) {
		if (arg->mode == MODE_ELEMENT) {
			if (arg->adr == var->adr && FlagOn(arg->submode, SUBMODE_REF)) {
				spill = true;
			}			
		}
	}
	return spill;
}

Bool InstrSpill(Instr * i, Var * var)
{
	Bool spill = false;

	if (i->op == INSTR_PRINT || i->op == INSTR_FORMAT) return true;

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

			if (FlagOff(var->submode, SUBMODE_IN | SUBMODE_OUT | SUBMODE_REG | SUBMODE_REF) 
//			 && (var->mode != MODE_ELEMENT || FlagOff(var->adr->submode, SUBMODE_IN | SUBMODE_OUT | SUBMODE_REG))
			 && var->mode != MODE_CONST 
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


void OptimizeLoops(Var * proc)
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
	Instr * i, * i2, ti;
	Instr * ls, * le;		// loop init start and end instruction 
	UInt32 n, n2;	//, le_n;
	Var * lab;
	Var * top_var, * reg, * top_reg, * prev_var;
	UInt16 r, regi;
	UInt32 var_size;
	Int32 q, q1, top_q;
	Bool spill, init;
	InstrBlock * blk;
	UInt32 lbl_cnt, lbl_loop_cnt;

	if (VERBOSE) {
		printf("========== Optimize loops ==============\n");
		PrintProc(proc);
	}

	// Find loops
	//
	// Label value (n) is set to instruction sequence number.
	// Label starting loop has src_i set to jump instruction, that ends the loop


	blk = proc->instr;

	// 1. Number all labels in procedure consecutivelly, reset loop ends

	NumberLabels(blk);
	for(n = 1, i = blk->first; i != NULL; i = i->next, n++) {
		if (i->op == INSTR_LABEL) {
			lab = i->result;
			lab->seq_no = n;
			lab->src_i = NULL;
			lab->lab_adr = i;
		}
	}


	// 2. For every backjump, set it's destination label to the jump
	//    Last jump to set the label is remembered

	for(n = 1, i = blk->first; i != NULL; i = i->next, n++) {
		if (IS_INSTR_JUMP(i->op)) {
			lab = i->result;
			if (lab->seq_no != 0 && (UInt32)lab->seq_no < n) {
				// This makes sure, the last jump instruction is set to loop label
				lab->src_i = i;
			}
		}
	}

	//TODO: Loop end may be stored into instruction declaring the label (block start?)
	//      There are 2 empty arguments.

	for(n = 1, i = blk->first; i != NULL; i = i->next, n++) {

		if (i->op == INSTR_LINE) continue;

		lab = i->result;

		// Find jump instruction that is marked as last jump to the specified label.
		// (There can be multiple jumps to that label - the last one is end of the loop)

		if (IS_INSTR_JUMP(i->op) && lab->src_i == i) {

			// TODO: How to get the pointer to label instruction?

			ls = lab->lab_adr; le = i;

			if (VERBOSE) {
				printf("\n*** Loop %ld-%ld\n", ls->result->seq_no, n);
			}

			//===== Compute usage of variables

			if (ls->result->seq_no == 272) {
				printf("@");
			}

			VarResetUse();
			InstrVarUse(blk, ls, le);
			InstrVarLoopDependent(blk, ls, le);

			// When processing, we assign var to register, if 
			for(regi = 1; regi < REG_CNT; regi++) {
				reg = REG[regi];
				reg->var = NULL;
			}

			while(top_var = FindMostUsedVar()) {

				if (VERBOSE) {
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

					if (VERBOSE) {
						printf("  Testing register: "); 
						PrintVar(reg);
					}

//					if (ls->result->seq_no == 248 && top_var->name != NULL && strcmp(top_var->name, "collision") == 0) {
//						printf("@");
//					}

					//===== Compute usage quotient (q)
					//      The bigger the value, the more suitable the register is
					//      0 means no gain, >0 means using the register would lead to worser code

					q = 0;
					reg->current_val = top_var;	// we expect initialization by top_var before loop
					prev_var = NULL;			// previous variable contained in the register
												// this variable must be loaded, when instruction using the register is encountered
												// (if it is not top_var)
					init = false;

					// Compute usage coeficient
					for(i2 = ls, n2 = ls->result->seq_no; i2 != le->next; i2 = i2->next, n2++) {

						if (i2->op == INSTR_LINE) continue;

						// Call to subroutine destroys all registers
						if (i2->op == INSTR_CALL) {
							q = 1;
							break;
						}

						if (IS_INSTR_JUMP(i2->op) && i2 != le) {
							init = true;
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
								if (VERBOSE) {
									printf("     %ld: invalid code\n", n2);
								}
								q = 1;		// do not use this register, as invalid code would get generated
								break;
							}
							// If there is currently not the value of top_var in replaced register,
							// we would have to load it
							if (reg->current_val != top_var && prev_var != NULL) {
								if (VERBOSE) {
									printf("     %ld: load\n", n2);
								}
								q += 3;
							}
							q -= q1;
							if (VERBOSE) {
								printf("     %ld: usage\n", n2);
							}
						}

						// Will it be necessary to spill?
						// We use the variable (array) that is stored to register

						spill = InstrSpill(i2, top_var);
						if (spill) {
							q += 4;
							if (VERBOSE) {
								printf("     %ld: spill\n", n2);
							}

						}

					}

					if (VERBOSE) {
						printf("     Quotient: %d\n", q);
					}

					if (q < top_q) {
						top_q = q;
						top_reg = reg;
					}
				}

				if (top_reg == NULL) continue;

				if (VERBOSE) {
					printf("Selected register: "); PrintVar(top_reg);
				}

				//TODO: If there is Let reg = var and var is not top_var, we need to spill store

				//=== Replace the use of registers
				
				// Generate instruction initializing the register used to replace the variable
				// before the start of the loop.
				// We only do this, if the variable is not initialized inside the loop.
				if (FlagOn(top_var->flags, VarUninitialized) || init) {
					i2 = ls;

					// Loops with condition at the beginning may start with jump to condition
					// We need to insert the initialization code before this jump.

					if (i2->prev && i2->prev->op == INSTR_GOTO) {
						i2 = i2->prev;
					}

					InstrInsert(blk, i2, INSTR_LET, top_reg, top_var, NULL);
					n++;
					top_reg->var = top_var;		// to prevent using the register in subsequent steps
				}
				r = 0;
				 
				for(i2 = ls; i2 != le->next; i2 = i2->next) {
					if (i2->op == INSTR_LINE) continue;

					if (InstrSpill(i2, top_var)) {
						InstrInsert(blk, i2, INSTR_LET, top_var, top_reg, NULL);
					}
					r += VarReplace(&i2->result, top_var, top_reg);
					VarReplace(&i2->arg1, top_var, top_reg);
					VarReplace(&i2->arg2, top_var, top_reg);

					if (i2->op == INSTR_LET && i2->result == i2->arg1) {
						i2 = InstrDelete(blk, i2);
						continue;
					}

				}

				// If we replaced some destination by the register, store the register to destination
				if (r > 0) {
					// There may be exit label as part of the loop
					// We need to spill after it
					//TODO: Check, that the label belongs to the loop and not to someone else)
					//      All jumps to the label go from the loop

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
					InstrInsert(blk, i2, INSTR_LET, top_var, top_reg, NULL);
				}

				if (FlagOn(top_var->flags, VarLoopDependent)) {
					reg->flags |= VarLoopDependent;
				}

				//Reset the usage count of this variable, so next variable will be selected
				if (VERBOSE) {
					NumberLabels(blk);
				}
			}
		}
	} // instr
}

void OptimizeCombined(Var * proc)
{
	Bool modified;
	do {
//		modified = OptimizePeephole(proc);
		modified = OptimizeLive(proc);
		modified |= OptimizeValues(proc);
	} while(modified);
}

void Optimize(Var * proc)
{
	OptimizeCombined(proc);
	OptimizeLoops(proc);
	OptimizeCombined(proc);
}
