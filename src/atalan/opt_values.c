/*

Code optimization routines
Values optimization.

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php


- common subexpression
- copy propagation
- constant folding

*/

#include "language.h"

void PrintExp(Exp * exp);

void ExpFree(Exp ** p_exp)
{
	Exp * exp;
	exp = *p_exp;
	*p_exp = NULL;
}

Bool ExpUsesValue(Exp * exp, Var * var)
/*
Purpose:
	Test, if expression uses specified value.
	Var may be array element.
*/
{
	if (exp == NULL) return false;
	if (exp->op == INSTR_VAR) {
		if (exp->var == var) return true;
		// If variables are aliases (have same address)
		if (var->adr == exp->var || exp->var->adr == var || (var->adr != NULL && exp->var->adr == var->adr)) return true;
		return false;
	} else {
		return ExpUsesValue(exp->arg[0], var) || ExpUsesValue(exp->arg[1], var);
	}
}

void ResetValues()
{
	Var * var;
	FOR_EACH_VAR(var)
		var->current_val = NULL;
		var->src_i       = NULL;
		ExpFree(&var->dep);		// TODO: Release the expression objects
	NEXT_VAR
}

void ResetValue(Var * res)
/*
Purpose:
	The value is modified in code to different value.
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

//			if (i->op == INSTR_LET) {
//				//TODO: Here may be error, when we get reference to instruction, which in fact uses the variable
//				if (i->arg1 == res) {
//					i->result->src_i = i->arg1->src_i;
//				}
//			} else {
				//TODO: Only reset the instruction, if there is no source
				if (i->arg1 == res || i->arg2 == res) {
					var->src_i = NULL;
				}
//			}
		}
	NEXT_VAR

	// If value is alias to some other value, reset it too
	//TODO: How about element (non constant, let's say?)

	var = res->adr;
	if (var != NULL) {
		if (var->mode == INSTR_VAR || var->mode == INSTR_TUPLE) {
			ResetValue(var);
		}
	} else {
		if (res->mode == INSTR_TUPLE) {
			ResetValue(res->adr);
			ResetValue(res->var);
		}
	}

	res->src_i = NULL;
}

Bool VarIsAlias(Var * var, Var * alias)
/*
Purpose:
	Return true, if the two variables are alias for same memory location.
	This may also mean than one variable is array and the other is element of that array.
*/
{
	if (var == NULL || alias == NULL) return false;
	if (var == alias) return true;
	if (var->adr != NULL && var->adr->mode == INSTR_VAR) if (VarIsAlias(var->adr, alias)) return true;
	if (alias->adr != NULL &&  alias->adr->mode == INSTR_VAR) if (VarIsAlias(var, alias->adr)) return true;
	return false;
}

void ResetVarDepRoot(Var * res)
{
	Var * var;

	FOR_EACH_VAR(var)
		if (VarIsAlias(var, res)) {
			ExpFree(&var->dep);
		}
	NEXT_VAR
}

void ResetVarDep(Var * res)
/*
Purpose:
	Specified variable is being set to some other value.
	Reset all references to specified value to NULL in all variables.
*/
{
	Var * var;
	Exp * exp;

	FOR_EACH_VAR(var)

//		if (var->adr != NULL && StrEqual(var->adr->name, "_arr")) {
//			printf("");
//			PrintExp(var->dep);
//		}

//		if (VarIsAlias(var, res)) {
//			ExpFree(&var->dep);
//		} else {
			exp = var->dep;
			if (exp != NULL) {
				if (ExpUsesValue(exp, res)) {
					ExpFree(&var->dep);
				}
			}
//		}
	NEXT_VAR

	// If the variable is alias for some other variable,
	// reset aliased variable too

	var = res->adr;
	if (var != NULL) {
		if (var->mode == INSTR_VAR) {
			ResetVarDep(var);
		}
	} else {
		if (res->mode == INSTR_TUPLE) {
			ResetVarDep(var->adr);
			ResetVarDep(var->var);
		}
	}
}

/*********************************
  Expressions
**********************************/

Exp * ExpAlloc(InstrOp op)
{
	Exp * exp = MemAllocStruct(Exp);
	exp->op = op;
	return exp;
}

void ExpArg(Exp * exp, UInt8 arg_idx, Var * arg)
/*
Purpose:
	Specify argument for expression.
	If the argument has assigned dependency expression, it is used as dependency.
	If it has no dependency argument, source (INSTR_LET) dependency is created.
*/
{
	Exp * src_dep;

	if (arg != NULL) {

		src_dep = arg->dep;

		if (arg->mode == INSTR_DEREF) {
			src_dep = ExpAlloc(INSTR_DEREF);
			ExpArg(src_dep, 0, arg->var);
		} else if (VarIsArrayElement(arg)) {
			//TODO: Support for 2d arrays
			//      In assembler phase is not required (we do not have instructions for 2d indexed arrays).
			if (arg->var->mode != INSTR_CONST) {
				src_dep = ExpAlloc(INSTR_ELEMENT);
				ExpArg(src_dep, 0, arg->adr);
				ExpArg(src_dep, 1, arg->var);
			}
		} else if (arg->mode == INSTR_TUPLE) {
			src_dep = ExpAlloc(arg->mode);
			ExpArg(src_dep, 0, arg->adr);
			ExpArg(src_dep, 1, arg->var);
		} else if (arg->mode == INSTR_VAR && arg->adr != NULL && arg->adr->mode == INSTR_TUPLE) {
			ExpArg(exp, arg_idx, arg->adr);
			return;
		}

		if (src_dep != NULL) {
			exp->arg[arg_idx] = src_dep;
		} else {
			src_dep = ExpAlloc(INSTR_VAR);
			src_dep->var = arg;
			exp->arg[arg_idx] = src_dep;
		}
	}
}

void PrintExp(Exp * exp)
{
	if (exp != NULL) {
		if (exp->op == INSTR_DEREF) {
			printf("@");
			PrintExp(exp->arg[0]);
		} else if (exp->op == INSTR_VAR) {
			PrintVarVal(exp->var);
		} else if (exp->op == INSTR_ELEMENT) {
			PrintExp(exp->arg[0]);
			printf("(");
			PrintExp(exp->arg[1]);
			printf(")");
		} else {
			// Unary instruction
			if (exp->arg[1] == NULL) {
				printf(" %s ", OpName(exp->op));
				PrintExp(exp->arg[0]);
			} else {
				printf("(");
				PrintExp(exp->arg[0]);
				printf(" %s ", OpName(exp->op));
				PrintExp(exp->arg[1]);
				printf(")");
			}
		}
	}
}

Exp * ExpInstr(Instr * i)
/*
Purpose:
	Build dependency for result of instruction i.
*/
{
	Exp * exp;
	Var * arg;
	InstrOp op;

	op = i->op;
	arg = i->arg1;

	if (op == INSTR_LET && (!VarIsArrayElement(arg) || arg->var->mode == INSTR_CONST)) {
		if (arg->dep != NULL) {
			if (op == INSTR_VAR) {
				arg = arg->dep->var;
			} else {
				exp = arg->dep;
				goto done;
			}
		}
		exp = ExpAlloc(INSTR_VAR);
		exp->var = arg;
	} else {
		exp = ExpAlloc(op);
		//todo: kill dependency, if source variables are not equal to result

		ExpArg(exp, 0, i->arg1);
		ExpArg(exp, 1, i->arg2);
	}
done:
	return exp;
}

void SetDependency(Var * var, Exp * exp)
{
	if (var == NULL) return;
	if (var->mode == INSTR_TUPLE) {
		SetDependency(var->adr, exp);
		SetDependency(var->var, exp);
	} else {
		var->dep = exp;
	}

	if (var->mode == INSTR_VAR && var->adr != NULL && (var->adr->mode == INSTR_TUPLE || var->adr->mode == INSTR_VAR)) {
		SetDependency(var->adr, exp);
	}
}

void Dependency(Instr * i)
{
	Exp * exp;

	exp = ExpInstr(i);

	SetDependency(i->result, exp);
//	i->result->dep = exp;

	// If we set a value to result and arg1 is NULL, set the dependency to source value too (they are both same)
	// This may happen, when some self-reference expression is calculated.

	if (i->op == INSTR_LET && i->arg1->dep == NULL && i->arg1->mode != INSTR_CONST) {
		exp = ExpAlloc(INSTR_VAR);
		exp->var = i->result;
		SetDependency(i->arg1, exp);
	}

/*
	InstrPrintInline(i);
	printf("       ");
	PrintVarVal(i->result);
	printf(" = ");
	PrintExp(exp);
	printf("\n");
*/
}

Bool ExpEquivalent(Exp * e1, Exp * e2)
{
	Bool eq = false;
	Var  * v1, * v2;
	if (e1 == NULL || e2 == NULL) return false;
	if (e1 == e2) return true;

	if (FlagOn(e1->flags, FlagExpProcessed) || FlagOn(e2->flags, FlagExpProcessed)) return false;

	SetFlagOn(e1->flags, FlagExpProcessed);
	SetFlagOn(e2->flags, FlagExpProcessed);

	if (e1->op == e2->op) {
		if (e1->op == INSTR_VAR) {
			v1 = e1->var; v2 = e2->var;
			if (FlagOn(v1->submode, SUBMODE_IN)) goto done;
			if (v1 == v2) { eq = true; goto done; }

			// Detect mutual dependency of two variables
			if (v1->dep != NULL && v1->dep->op == INSTR_VAR && v1->dep->var == v2) { eq = true; goto done; }

			if (v1->mode == v2->mode) {
				if (v1->type->variant == v2->type->variant) {
					if (v1->mode == INSTR_CONST) {
						eq = (v1->n == v2->n);
						goto done;
					}
				}
			}
			eq = ExpEquivalent(v1->dep, v2->dep);
		} else {
			eq = ExpEquivalent(e1->arg[0], e2->arg[0]);
			if (eq && (e1->arg[1] != NULL || e2->arg[1] != NULL)) {
				eq = ExpEquivalent(e1->arg[1], e2->arg[1]);
			}
		}
	}
done:
	SetFlagOff(e1->flags, FlagExpProcessed);
	SetFlagOff(e2->flags, FlagExpProcessed);

	return eq;
}

Bool ExpEquivalentInstr(Exp * exp, Instr * i)
{
	Exp * exp2;
	Bool r;
	exp2 = ExpInstr(i);
	r = ExpEquivalent(exp, exp2);
	return r;
}

Bool CodeModifiesVar(Instr * from, Instr * to, Var * var)
{
	Instr * i;
	Var * result;
	for(i = from; i != NULL && i != to; i = i->next) {
		if (i->op != INSTR_LINE) {
			result = i->result;
			if (result != NULL)  {
				if (result == var) return true;
				if (var->mode == INSTR_ELEMENT || var->mode == INSTR_BYTE) {
					if (result == var->adr) return true;
					if (result == var->var) return true;
				}
			}
		}
	}
	return false;
}

void VarResetProcessed()
{
}

void ProcValuesUse(Var * proc)
{
	Instr * i;
	InstrBlock * blk;
	Var * var;

	if (FlagOff(proc->flags, VarProcessed)) {
		SetFlagOn(proc->flags, VarProcessed);

		// Some external procedures may be only declared, in such case,
		// we just clear it's output variables

		if (proc->instr == NULL) {
			FOR_EACH_LOCAL(proc, var)
				if (!VarIsInArg(var)) {
//				if (var->mode != INSTR_ARG || FlagOn(var->submode, SUBMODE_ARG_OUT)) {
					ResetValue(var);
					ResetVarDep(var);
					ResetVarDepRoot(var);
				}
			NEXT_LOCAL
		} else {
			for(blk = proc->instr; blk != NULL; blk = blk->next) {
				for(i = blk->first; i != NULL; i = i->next) {
					if (i->op == INSTR_LINE) {
					} else if (i->op == INSTR_CALL) {
						ProcValuesUse(i->result);
					} else if (IS_INSTR_JUMP(i->op)) {
						// jump instructions do have result, but it is label we jump to
					} else {
						if (i->result != NULL) {
							ResetValue(i->result);
							ResetVarDep(i->result);
							ResetVarDepRoot(i->result);
						}
					}
				}
			}
		}
		SetFlagOff(proc->flags, VarProcessed);
	}
}

Bool VarIsZeroNonzero(Var * var, Var ** p_zero)
/*
Purpose:
	Test, if variable may have just two values, 0 and some other.
	This is basically true for 0..1 integers or enumerators with 0 and other value.
*/
{
	Type * t;
	Var * zero = NULL, * non_zero = NULL;
	Var * item;

	if (var == NULL) return false;
	t = var->type;
	if (t->variant == TYPE_INT) {
		if (t->is_enum) {

			FOR_EACH_LOCAL(t->owner, item)
				if (VarIsIntConst(item)) {
					if (item->n == 0) {
						zero = item;
					} else {
						if (non_zero != NULL) {
							// There are two different non-zero constants!
							if (non_zero->n != item->n) {
								zero = NULL;
								break;
							}
						} else {
							non_zero = item;
						}
					}
				}
			NEXT_LOCAL
		} else {
			if ((t->range.min == 0 && t->range.max == 1) || (t->range.min == -1 && t->range.max == 0)) {
				zero = VarNewInt(0);
			}
		}
	}
	*p_zero = zero;
	return zero != NULL;
}

Var * SrcVar(Var * var)
{
	if (var != NULL) {
		while (FlagOff(var->submode, SUBMODE_IN) && var->src_i != NULL && var->src_i->op == INSTR_LET) var = var->src_i->arg1;
	}
	return var;
}

UInt32 GOG = 0;
//UInt32 GOG2 = 0;

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
	Bool modified, m2, m3;
	Instr * i, * src_i;
	UInt32 n;
	Var * r, * result, * arg1, * arg2;
	InstrBlock * blk;
	InstrOp op, src_op;
	char buf[32];

	if (Verbose(proc)) {
		printf("------ optimize values -----\n");
		PrintProc(proc);
	}

	VarUse();

	modified = false;

	for(blk = proc->instr; blk != NULL; blk = blk->next) {
		
		ResetValues();
		n = 0;
		for(i = blk->first; i != NULL; i = i->next) {
retry:
			n++;
			// Instruction may be NULL here, if we have deleted the last instruction in the block.
			if (i == NULL) break;
			// Line instructions are not processed.
			if (i->op == INSTR_LINE) continue;

			if (i->op == INSTR_CALL) {
//				GOG2++;
				ProcValuesUse(i->result);
				continue;
			}

			result = i->result;

			// We are only interested in instructions that have result
			if (result != NULL) {

				if (IS_INSTR_JUMP(i->op)) {

					// jump instructions use label as result, we do not want to remove jumps
				} else {

					// Remove equivalent instructions.
					// If the instruction sets it's result to the same value it already contains,
					// remove the instruction.

					arg1 = i->arg1;
					src_i = result->src_i;

					// If result is equal to arg1, instructions are not equivalent, because they accumulate the result,
					// (for example sequence of mul a,a,2  mul a,a,2

					m3 = false;

					if (result != arg1 && result != i->arg2 && !OutVar(result)
					&& (arg1 == NULL || FlagOff(arg1->submode, SUBMODE_IN)))  {
//						GOG++;
//						if (GOG == 18) {
//							PrintExp(result->dep);
//						}
						m3 = ExpEquivalentInstr(result->dep, i);
					}
					 
					if (m3) {

						// Array references, that have non-const index may not be removed, as
						// we can not be sure, that the index variable has not changed since last
						// use.
						if ((result->mode == INSTR_ELEMENT || result->mode == INSTR_BYTE) && result->var->mode != INSTR_CONST) {
						
						} else {
	delete_instr:
							if (Verbose(proc)) {
								printf("Removing %ld#%ld:", blk->seq_no, n); InstrPrint(i);
							}
							i = InstrDelete(blk, i);
							modified = true;
							goto retry;
						}
					} else {
						// Instruction result is set to different value, we must therefore reset
						// all references to this value from all other values.
						ResetValue(result);
						ResetVarDep(result);
					}

					// Try to replace arguments of operation by it's source (or eventually constant)

					op = i->op;
					m2 = false;

					if (arg1 != NULL && arg1->src_i != NULL && FlagOff(arg1->submode, SUBMODE_IN)) {
						src_i = arg1->src_i;
						src_op = src_i->op;

						// Try to replace LO b,n LET a,b  => LO a,n LO a,n

						if (src_op == INSTR_LO || src_op == INSTR_HI || src_op == INSTR_LET_ADR) {
							if (op == INSTR_LET) {
								op = src_op;
								arg1 = src_i->arg1;
								m2 = true;
							}
						} else if (src_op == INSTR_LET) {
							// If instruction uses register, do not replace with instruction that does not use it
							if (FlagOff(src_i->arg1->submode, SUBMODE_IN) && !(FlagOn(arg1->submode, SUBMODE_REG) && FlagOff(src_i->arg1->submode, SUBMODE_REG)) ) {
								// Do not replace simple variable with array access
								if (!(arg1->mode == INSTR_VAR && src_i->arg1->mode == INSTR_ELEMENT)) {
									arg1 = src_i->arg1;
									m2 = true;
								}
							}
						}
					}

					arg2 = i->arg2;
					if (arg2 != NULL && !InVar(arg2) && arg2->src_i != NULL ) {					
						src_i = arg2->src_i;
						src_op = src_i->op;

						if (src_op == INSTR_LET) {
							if (!InVar(src_i->arg1) && !(FlagOn(arg2->submode, SUBMODE_REG) && FlagOff(src_i->arg1->submode, SUBMODE_REG)) ) {
								// Do not replace simple variable with array access
								if (arg2->read == 1 || !(arg2->mode == INSTR_VAR && src_i->arg1->mode == INSTR_ELEMENT)) {
									if (src_i->arg1->mode != INSTR_ELEMENT || !CodeModifiesVar(src_i->next, i, src_i->arg1)) {
										arg2 = src_i->arg1;
										m2 = true;
									}
								}
							}
						}
					}

					// let x,x is always removed
					if (op == INSTR_LET && result == arg1) {
						goto delete_instr;
					}

					if (m2) {
						if (InstrRule2(op, result, arg1, arg2)) {
							i->op   = op;
							i->arg1 = arg1;
							i->arg2 = arg2;
						}
					}

					//==== Try to evaluate constant instructions
					// We first try to traverse the chain of assignments to it's root.
					// If there is IN variable on the road, we have to stop there
					// and replacing will not be performed.

					r = NULL;
					arg1 = SrcVar(i->arg1);
					arg2 = SrcVar(i->arg2);
					r = InstrEvalConst(i->op, arg1, arg2);

					// We have evaluated the instruction, change it to LET <result>,r
					if (r != NULL) {
						if (InstrRule2(INSTR_LET, i->result, r, NULL)) {
							i->op = INSTR_LET;
							i->arg1 = r;
							i->arg2 = NULL;
							result->src_i       = i;
							modified = true;
						}
					}
					result->src_i = i;

					// Create dependency tree
					Dependency(i);

				} // BRANCH
			} else { // result != NULL

				// Convert reference to constant value in expression to constant text
				if (i->op == INSTR_VAR_ARG) {
					arg1 = i->arg1;
					if (arg1 != NULL) {
						while (!InVar(arg1) && arg1->src_i != NULL && arg1->src_i->op == INSTR_LET) arg1 = arg1->src_i->arg1;
					}
					if (VarIsIntConst(arg1)) {
						if (Verbose(proc)) {
							printf("Arg to const %ld#%ld:", blk->seq_no, n); InstrPrint(i);
						}
						i->op = INSTR_STR_ARG;
						sprintf(buf, "%d", arg1->n);
						i->arg1 = VarNewStr(buf);
						i->arg2 = VarNewInt(StrLen(buf));
					}
				}
			}
		}
	} // block
	return modified;
}

void CheckValues(Var * proc)
/*
Purpose:
	Check procedure before translation and perform some basic optimizations.
*/
{
	Instr * i, * i2;
	UInt32 n;
	Var * r, * result, * arg1, * arg2, * zero;
	InstrBlock * blk;
	InstrOp op;

	VarUse();
	for(blk = proc->instr; blk != NULL; blk = blk->next) {
		ResetValues();
		n = 1;
		for(i = blk->first; i != NULL; i = i->next, n++) {

			op = i->op;
			// Line instructions are not processed.
			if (op == INSTR_LINE) continue;

			// Try to convert compare with non-zero to compare versus zero
			// If we compare variable that has only two possible values and one of them is 0, we want
			// to always compare with the zero, because on most platforms, that test is more effective.

			if (op == INSTR_IFEQ || op == INSTR_IFNE) {
				arg1 = i->arg1;
				arg2 = i->arg2;
				if (VarIsIntConst(arg2) && arg2->n != 0 && VarIsZeroNonzero(arg1, &zero)) {
					i->op = OpNot(i->op);
					i->arg2 = zero;
				}
			}

			result = i->result;
			if (result != NULL && !VarIsLabel(result)) {
				arg1 = SrcVar(i->arg1);
				arg2 = SrcVar(i->arg2);

				r = InstrEvalConst(op, arg1, arg2);

				// We have evaluated the instruction, change it to LET <result>,r
				if (r != NULL) {
					i->op = INSTR_LET;
					i->arg1 = r;
					i->arg2 = NULL;

				//OPTIMIZATION: Instruction merging
				// Try to convert
				//      add R1, Y, #c1
				//      add R2, R1, #c2
				//
				// We convert it to:
				//      add R1, Y, #c1
				//      add R2, Y, #c1+#c2
				} else {
					if (i->op == INSTR_ADD && VarIsConst(arg2)) {

						// Get source instruction or directly previous instruction
						i2 = arg1->src_i;
						if (i2 == NULL) {
							i2 = i->prev; while(i2 != NULL && i2->op == INSTR_LINE) i2 = i2->prev;
						}
						if (i2 != NULL && i2->result == arg1) {
							if (i2->op == INSTR_ADD) {
								if (VarIsConst(i2->arg2)) {
									if (result != arg1) {
										r = InstrEvalConst(i->op, arg2, i2->arg2);
										i2->arg2 = r;
										i->arg1 = i2->arg1;
										i->arg2 = r;

										// We don't need the first instruction anymore
										if (result->read == 1) {
											InstrDelete(blk, i2);
										}
									}
								}
							}
						}
					}
				}

				ResetValue(i->result);

				if (i->op == INSTR_LET) {
					result->src_i = i;
				}
			}
		}
	}
}

Bool OptimizeVarMerge(Var * proc)
/*
Purpose:
	Optimize instruction sequences like:

			let a,10
			let b,a
			dead a
	to

			let b,10

*/{

	Instr * i, * i2, ni;
	Var * result, * arg1;
	InstrBlock * blk;
	InstrOp op;
	Bool modified = false;
	int q;
	UInt32 n;

//	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
//	PrintProc(proc);

	for (blk = proc->instr; blk != NULL; blk = blk->next) {
		for (i = blk->first, n = 1; i != NULL; i = i->next, n++) {
next:
			if (i == NULL) break;
			op = i->op;
			if (op == INSTR_LET || op == INSTR_LET_ADR || op == INSTR_HI || op == INSTR_LO) {
				result = i->result;
				arg1   = i->arg1;
				if (!OutVar(result) && VarIsFixed(arg1) && i->next_use[1] == NULL) {
					for /*test*/ (i2 = i->prev; i2 != NULL; i2 = i2->prev) {

						if (i2->op == INSTR_LINE) continue;

						// Test, that it is possible to translate the instruction using new register
						// (and possibly new instruction - we may change let to let_adr etc.)

						memcpy(&ni, i2, sizeof(Instr));
						ni.op = op;
						q = VarTestReplace(&ni.result, arg1, result);
						q += VarTestReplace(&ni.arg1, arg1, result);
						q += VarTestReplace(&ni.arg2, arg1, result);
						if (q != 0 && InstrRule(&ni) == NULL) break;

						// We successfully found source instruction, this means we can replace arg1 with result
						if (i2->result == arg1 && !VarUsesVar(i2->arg1, arg1) && !VarUsesVar(i2->arg2, arg1)) {

							//==== We have succeeded, replacing the register
							
							if (Verbose(proc)) {
								printf("Merging %ld", n); InstrPrint(i);
							}

							modified = true;

							for (;i2 != i; i2 = i2->next) {
								if (i2->op == INSTR_LINE) continue;
								VarReplace(&i2->result, arg1, result);
								VarReplace(&i2->arg1, arg1, result);
								VarReplace(&i2->arg2, arg1, result);
							}
							i = InstrDelete(blk, i);
							goto next;
						}
						if (i2->op == INSTR_CALL) break;
						if (InstrUsesVar(i2, result)) break;

					} /* test */
				}
			}
		}
	}

//	if (modified) {
//		printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
//		PrintProc(proc);
//	}
	return modified;
}
