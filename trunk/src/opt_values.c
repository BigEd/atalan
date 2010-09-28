/*

Code optimalization routines
Values optimization.

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php


- common subexpression
- copy propagation
- constant folding

*/

#include "language.h"

extern Var * VARS;		// global variables

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
		if (var->adr == exp->var || exp->var->adr == var) return true;
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

Bool VarUsesVar(Var * var, Var * test_var);

Bool ExprUsesVar(Instr * i, Var * var)
{
	Bool b;

	if (i->src1 != NULL) {
		b = ExprUsesVar(i->src1, var);
	} else {
		b = VarUsesVar(i->arg1, var);
	}

	if (!b) {
		if (i->src2 != NULL) {
			b = ExprUsesVar(i->src2, var);
		} else {
			b = VarUsesVar(i->arg2, var);
//			b = (var == i->arg2);
		}
	}
	return b;
}

void ResetValue(Var * res)
/*
Purpose:
	Set all references to specified value to NULL in all values.
*/
{
	Var * var;
	Instr * i;
//	Bool b;

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
//				if (ExprUsesVar(i, res)) {
				if (i->arg1 == res) {
//					printf("");
//				}
//				if (b) {
//					var->src_i = NULL;
					i->result->src_i = i->arg1->src_i;
				}
			} else {
				//TODO: Only reset the instruction, if there is no source
//				if (ExprUsesVar(i, res)) {
				if (i->arg1 == res || i->arg2 == res) {
					var->src_i = NULL;
				}
			}
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
//	Bool b;

	FOR_EACH_VAR(var)
		exp = var->dep;
		if (exp != NULL) {
			if (ExpUsesValue(exp, res)) {
				ExpFree(&var->dep);
			}
		}
	NEXT_VAR
}

Bool InstrEquivalent(Instr * i1, Instr * i2);

Bool VarEquivalent(Var * v1, Var * v2)
{
	Bool eq = false;
	Instr * i1, * i2;
	// Two references to array using index variable are not same

	if (v1 == v2) {
		if (v1 == NULL) return true;
		if (v1->mode == MODE_ELEMENT) {
			if (v1->var->mode != MODE_CONST) return false;
		}
		return true;
	}

	i1 = v1->src_i; i2 = v2->src_i;
	if (i1 != NULL && i2 != NULL) {
		if (v1 == i1->arg1) v1->src_i = i1->src1;
		if (v1 == i1->arg2) v1->src_i = i1->src2;
//		v1->src_i = i1->src1;
//		v2->src_i = i2->src1;
		if (v2 == i2->arg1) v2->src_i = i2->src1;
		if (v2 == i2->arg2) v2->src_i = i2->src2;
		eq = InstrEquivalent(i1, i2);
		v1->src_i = i1; v2->src_i = i2;
	}

	//TODO: Compare values (there may be named constant with same value as unnamed constant and it will be different var)
	//TODO: Exclude IN, OUT variables
	return eq;
}

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

	if (i1->op == i2->op) {

		// If the instructions have source, check source equivalence
//		if (i1->source != NULL && i2->source != NULL) {
//			eq = InstrEquivalent(i1->source, i2->source);
//			if (!eq) return false;
//		}

		eq =  VarEquivalent(i1->arg1, i2->arg1)
			&& VarEquivalent(i1->arg2, i2->arg2)
			&& VarEquivalent(i1->result, i2->result);

		if (eq) return true;
	}

//	if (i1->op == i2->op && VarEquivalent(i1->result, i2->result) && VarEquivalent(i1->arg1, i2->arg1) && VarEquivalent(i1->arg2, i2->arg2)) return true;

	// Let is equivalent, if arg1->src_i is same let
	if (i2->op == INSTR_LET) {
		eq = InstrEquivalent(i1, i2->arg1->src_i);
	}
	if (!eq) {
		if (i1->op == INSTR_LET) {
			eq = InstrEquivalent(i1->arg1->src_i, i2);
		}
	}
/*
	if (!eq) {
		if (i1->op == i2->op) {

			// If the instructions have source, check source equivalence
			if (i1->source != NULL && i2->source != NULL) {
				eq = InstrEquivalent(i1->source, i2->source);
				if (!eq) return false;
			}

			eq =  VarEquivalent(i1->arg1, i2->arg1)
			   && VarEquivalent(i1->arg2, i2->arg2)
			   && VarEquivalent(i1->result, i2->result);
		}
	}
*/
	return eq;
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

		if (VarIsArrayElement(arg)) {
			//TODO: Support for 2d arrays
			//      In assembler phase is not required (we do not have instructions for 2d indexed arrays).
			if (arg->var->mode != MODE_CONST) {
				src_dep = ExpAlloc(INSTR_ELEMENT);
				ExpArg(src_dep, 0, arg->adr);
				ExpArg(src_dep, 1, arg->var);
			}
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

char * g_InstrName[INSTR_CNT] = 
{
	"", // INSTR_VOID = 0,
	"<-", //INSTR_LET,		// var, val

	"=",  //INSTR_IFEQ,		// must be even!!!.
	"<>", //INSTR_IFNE,
	"<",   //INSTR_IFLT,
	">=",   // INSTR_IFGE,
	">",   // INSTR_IFGT,
	"<=",   // INSTR_IFLE,
	"ov",   // INSTR_IFOVERFLOW,
	"!ov",   // INSTR_IFNOVERFLOW,

	"",   // INSTR_PROLOGUE,
	"",   // INSTR_EPILOGUE,
	"",   // INSTR_EMIT,
	"",   // INSTR_VARDEF,
	"",   // INSTR_LABEL,
	"",   // INSTR_GOTO,
	"+",   // INSTR_ADD,
	"-",   // INSTR_SUB,
	"*",   // INSTR_MUL,
	"/",   // INSTR_DIV,
	"sqrt",   // INSTR_SQRT,

	"and",   // INSTR_AND,
	"or",   // INSTR_OR,

	"",   // INSTR_ALLOC,
	"",   // INSTR_PRINT,
	"",   // INSTR_FORMAT,
	"",   // INSTR_PROC,
	"",   // INSTR_ENDPROC,
	"",   // INSTR_CALL,
	"",   // INSTR_VAR_ARG,
	"",   // INSTR_STR_ARG,			// generate str
	"",   // INSTR_DATA,
	"",   // INSTR_FILE,
	"",   // INSTR_ALIGN,
	"hi",   // INSTR_HI,
	"lo",   // INSTR_LO,
	"ptr",   // INSTR_PTR,
	"",   // INSTR_ARRAY_INDEX,		// generate index for array
	"=@",   // INSTR_LET_ADR,
	"<<",   // INSTR_ROL,				// bitwise rotate right
	">>",   // INSTR_ROR,				// bitwise rotate left
	"",   // INSTR_DEBUG,
	"mod",   // INSTR_MOD,
	"xor",   // INSTR_XOR,
	"not",   // INSTR_NOT,

	"",   // INSTR_LINE,				// reference line in the source code
	"",   // INSTR_INCLUDE,
	"",   // INSTR_MULA,				// templates for 8 - bit multiply 
	"",   // INSTR_MULA16,           // templates for 8 - bit multiply 

	"",   // INSTR_REF,				// this directive is not translated to any code, but declares, that some variable is used
	"",   // INSTR_DIVCARRY,

	// Following 'instructions' are used in expressions
	"[",   // INSTR_ELEMENT,		// access array element (left operand is array, right is index)
	","    // INSTR_LIST,			// create list of two elements
};

void PrintExp(Exp * exp)
{
	if (exp != NULL) {
		if (exp->op == INSTR_VAR) {
			PrintVarVal(exp->var);
//		} else if (exp->op == INSTR_LET_ADR) {
//			printf("@");
//			PrintVarVal(exp->var);
		} else {
			// Unary instruction
			if (exp->arg[1] == NULL) {
				printf(" %s ", g_InstrName[exp->op]);
				PrintExp(exp->arg[0]);
			} else {
				printf("(");
				PrintExp(exp->arg[0]);
				printf(" %s ", g_InstrName[exp->op]);
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

	if (op == INSTR_LET && (!VarIsArrayElement(arg) || arg->var->mode == MODE_CONST)) {
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

void Dependency(Instr * i)
{
	Exp * exp;

	exp = ExpInstr(i);
	i->result->dep = exp;

	// If we set a value to result and arg1 is NULL, set the dependency to source value too (they are both same)
	// This may happen, when some self-reference expression is calculated.

	if (i->op == INSTR_LET && i->arg1->dep == NULL && i->arg1->mode != MODE_CONST) {
		exp = ExpAlloc(INSTR_VAR);
		exp->var = i->result;
		i->arg1->dep = exp;
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

	if (e1->op == e2->op) {
		if (e1->op == INSTR_VAR) {
			v1 = e1->var; v2 = e2->var;
			if (v1 == v2) return true;

			// Detect mutual dependency of two variables
			if (v1->dep != NULL && v1->dep->op == INSTR_VAR && v1->dep->var == v2) return true;

			if (v1->mode == v2->mode) {
				if (v1->type->variant == v2->type->variant) {
					if (v1->mode == MODE_CONST) {
						return v1->n == v2->n;
					}
				}
			}
			return ExpEquivalent(v1->dep, v2->dep);
		}
		eq = ExpEquivalent(e1->arg[0], e2->arg[0]);
		if (eq && (e1->arg[1] != NULL || e2->arg[1] != NULL)) {
			eq = ExpEquivalent(e1->arg[1], e2->arg[1]);
		}
	}
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
	Instr * i, i2, * src_i;
	UInt32 n;
	Var * r, * result, * arg1, * arg2;
	InstrBlock * blk;
	InstrOp op, src_op;

	if (VERBOSE) {
		printf("------ optimize values -----\n");
		PrintProc(proc);
	}

	modified = false;
	n = 1;
	for(blk = proc->instr; blk != NULL; blk = blk->next) {
		ResetValues();

		for(i = blk->first; i != NULL; i = i->next, n++) {
retry:
			if (i == NULL) break;
			if (i->op == INSTR_LINE) continue;

			// Create tree of expressions.
			// Instruction has reference to instructions, whose result is used as argument to this instruction

			if (i->arg1 != NULL) {
				src_i = i->arg1->src_i;
				// Do not add temporary let instructions into the tree.
				if (src_i != NULL) {
					if (src_i->op == INSTR_LET && src_i->src1 != NULL) src_i = src_i->src1;
				}
				i->src1 = src_i;
			}
			if (i->arg2 != NULL) {
				src_i = i->arg2->src_i;
				if (src_i != NULL) {
					if (src_i->op == INSTR_LET && src_i->src1 != NULL) src_i = src_i->src1;
				}
				i->src2 = src_i;
			}

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
					src_i = result->src_i;

					//TODO: Split OUT & Equivalent
					// If result is equal to arg1, instructions are not equivalent, because they accumulate the result,
					// (for example sequence of mul a,a,2  mul a,a,2

					m2 = false;
					m3 = false;
					if (FlagOff(result->submode, SUBMODE_OUT) && result != arg1 && result != i->arg2) {
						m2 = InstrEquivalent(i, src_i);						
//						printf("-%d\n", n);
						m3 = ExpEquivalentInstr(result->dep, i);
/*
						if (!m2 && m3) {
							printf("Exp remove %d\n", n);
							printf("Result: "); PrintExp(result->dep); 
							printf("\n");
						}
						if (m2 && !m3) {
							printf("Exp not remove %d\n", n);
							printf("Result: "); PrintExp(result->dep); 
							printf("\n");
						}
*/
					}
					 
					if (m3) {

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
						ResetVarDep(result);
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

					//==== Try to evaluate constant instructions
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
					// Create dependency tree (we only create it for instructions with result)

					Dependency(i);

				} // BRANCH
			} // result != NULL
		}
	} // block
	return modified;
}

