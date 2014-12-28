/*

Generator

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php


This module contains functions, that are used to generate instructions.
Instructions are generated to specified block of code.

*/

#include "language.h"

GLOBAL InstrBlock * FIRST_BLK;				// First generated block (while generating instructions, multiple block can be created)
GLOBAL InstrBlock * BLK;					// Current generated block (it is always the last block)
GLOBAL Instr *      INSTR;					// Last generated instruction. The next generated instruction will be appended after this instruction.

GLOBAL InstrBlock * IBLOCK_STACK[128];
GLOBAL UInt16       IBLOCK_STACK_SIZE;
GLOBAL Var          ROOT_PROC;
GLOBAL RuleSet      GEN_RULES;


void GenSetDestination(InstrBlock * blk, Instr * i)
/*
Purpose:
	Set block and instruction, before which will be generated next instruction.
*/
{
	BLK   = blk;
	INSTR = i;
}

InstrBlock * GenBegin()
/*
Purpose:
	Create new code block and let next instructions be generated into this block.
*/
{
	IBLOCK_STACK[IBLOCK_STACK_SIZE] = FIRST_BLK;
	IBLOCK_STACK_SIZE++;
	FIRST_BLK = BLK = NewCode();
	BLK->seq_no = 1;
	INSTR = NULL;
	return BLK;
}

InstrBlock * GenEnd()
/*
Purpose:
	Pop last pushed code block.
*/
{
	InstrBlock * blk;
	blk = FIRST_BLK;
	IBLOCK_STACK_SIZE--;
	FIRST_BLK = IBLOCK_STACK[IBLOCK_STACK_SIZE];
	BLK = CodeLastBlock(FIRST_BLK);
	if (BLK != NULL) {
		INSTR = BLK->last;
	}
	return blk;
}

InstrBlock * GenNewBlock()
{
	InstrBlock * blk = NewCode();
	if (BLK != NULL) {
		blk->seq_no = BLK->seq_no+1;
	}
	GenBlock(blk);
	return blk;
}

void GenRegisterRule(Rule * rule)
/*
Purpose:
	Register generator rule.
*/
{
	RuleSetAddRule(&GEN_RULES, rule);
}

void GenBlock(InstrBlock * blk)
/*
Purpose:
	Generate block of code.
	The code is attached to the generated output (copy is not made).
*/
{
	if (blk != NULL) {
		InstrBlock * last = LastBlock(blk);
		if (BLK == NULL) {
			FIRST_BLK = blk;
			blk->seq_no = 1;
		} else {
			last->next = BLK->next;
			BLK->next = blk;
			blk->seq_no = BLK->seq_no+1;
		}

		BLK = last;
		INSTR = BLK->last;
/*
		blk->first->prev = BLK->last;
		blk->last->next  = NULL;
		if (BLK->last != NULL) {
			BLK->last->next = blk->first;
		}
		BLK->last = blk->last;
		if (BLK->first == NULL) {
			BLK->first = blk->first;
		}
		free(blk);	// free just block head
*/
	}
}

void GenFromLine(SrcLine * line, InstrOp op, Var * result, Var * arg1, Var * arg2)
{
	Var * var;
	Instr * i;

	// For commutative or relational operations make sure the constant is the other operator
	// This simplifies further code processing.

	if (op == INSTR_ADD || op == INSTR_MUL || op == INSTR_OR || op == INSTR_AND || op == INSTR_XOR) {
		if (op != INSTR_OVERFLOW && op != INSTR_NOVERFLOW) {
			if (CellIsConst(arg1)) {
				var = arg1; arg1 = arg2; arg2 = var;

				op = OpRelSwap(op);

			}
		}
	}
	i = InstrInsert(BLK, INSTR, op, result, arg1, arg2);
	i->line = line;
	i->line_pos = 0;
}

void GenInternal(InstrOp op, Var * result, Var * arg1, Var * arg2)
{
	GenFromLine(SRC_LINE, op, result, arg1, arg2);
}

void Gen(InstrOp op, Var * result, Var * arg1, Var * arg2)
/*
Purpose:
	Generate instruction into current code block.
*/
{
	Rule * rule = NULL;

	if (!ParsingRule()) {
		rule = RuleSetFindRule(&GEN_RULES, op, result, arg1, arg2);
	}
	if (rule == NULL) {
		GenInternal(op, result, arg1, arg2);
	} else {
		GenMatchedRule(rule);
	}

}

void GenRule(Rule * rule, Var * result, Var * arg1, Var * arg2)
/*
Purpose:
	Generate CPU instruction translated using specified rule.
*/
{
	InstrInsert(BLK, INSTR, rule->op, result, arg1, arg2);
	BLK->last->rule = rule;
}

void GenPos(InstrOp op, Var * result, Var * arg1, Var * arg2)
{
	Instr * i;
	Gen(op, result, arg1, arg2);
	if (INSTR == NULL) {
		i = BLK->last;
	} else {
		i = INSTR->prev;
	}
	i->line_pos = OP_LINE_POS;
}

void GenLetPos(Var * result, Var * arg1)
{
	Type * rtype, * atype;

	rtype = result->type;
	atype = arg1->type;

	// TODO: We should test for chain of ADR OF ADR OF ADR ....
	//       Error should be reported when assigning address of incorrect type

	if (rtype != NULL && atype != NULL && rtype->variant == TYPE_ADR && atype->variant != TYPE_ADR) {
		GenPos(INSTR_LET_ADR, result, arg1, NULL);
	} else {
		GenPos(INSTR_LET, result, arg1, NULL);
	}
}

void GenLet(Var * result, Var * arg1)
{
	OP_LINE_POS = 0;
	GenLetPos(result, arg1);
}

void GenGoto(Var * label)
{
	if (label != NULL) {
		GenInternal(INSTR_IF, NULL, ONE, label);
	}
}

void GenLabel(Var * var)
{
	if (var != NULL) {
		GenInternal(INSTR_LABEL, var, NULL, NULL);
	}
}

void GenLastResult(Var * var, Var * item)
/*
Purpose:
	Set result of last generated instruction.
*/
{
	Instr * i = BLK->last;
	if (i->op != INSTR_LABEL) {
		i->result = var;
	} else {
		GenLet(var, item);
	}
}

Var * GenArg(Var * macro, Var * var, Var ** args, VarSet * locals)
/*
Purpose:
	Find function (macro) argument or structure member.
*/
{

	Var * arg, * arr, * l, * r;
	UInt16 n;
	Var * en;
	InstrOp op;
	Var * fn_type;

	if (var == NULL) return NULL;

	op = var->mode;

	// If this is element reference and either array or index is macro argument,
	// create new array element referencing actual array and index.
	
	if (op == INSTR_DEREF) {
		arg = GenArg(macro, var->l, args, locals);
		if (arg != var->l) {
			var = VarNewDeref(arg);
		}
	} else if (op == INSTR_ITEM || op == INSTR_ELEMENT || op == INSTR_BYTE || op == INSTR_BIT) {

		arr = GenArg(macro, var->l, args, locals);

		if (arr != var->l && var->r->mode == INSTR_TEXT) {
			var = VarField(arr, var->r->str);
		} else {
			arg = GenArg(macro, var->r, args, locals);	// index

			if (arr != var->l || arg != var->r) {
				if (op == INSTR_ITEM) {
					var = NewItem(arr, arg);
				} else if (op == INSTR_ELEMENT) {
					var = VarNewElement(arr, arg);
				} else if (op == INSTR_BYTE) {
					var = VarNewByteElement(arr, arg);
				} else {
					var = VarNewBitElement(arr, arg);
				}
			}
		}
	} else if (op == INSTR_TUPLE || FlagOn(INSTR_INFO[op].flags, INSTR_OPERATOR)) {
		l = GenArg(macro, var->l, args, locals);
		r = GenArg(macro, var->r, args, locals);

		if (l != var->l || r != var->r) {			
			var = InstrEvalConst(op, l, r);
			if (var == NULL) {
				var = NewOp(op, l, r);
			}
		}

	} else if (op == INSTR_VAR) {

		// Optimization for instruction rule
		if (VarIsRuleArg(var)) {
			return args[var->idx-1];
		}

		// If the variable is format arguments or result, return one of specified actual arguments

		fn_type = macro->type->type;


		if (fn_type->mode == INSTR_FN_TYPE) {
			//TODO: Make more efficient mechanism for finding argument index
			n = 0;
			FOR_EACH_ITEM(en, arg, ArgType(fn_type))
				if (arg == var) return args[n];
				n++;
			NEXT_ITEM(en, arg)

			FOR_EACH_ITEM(en, arg, ResultType(fn_type))
				if (arg == var) return args[n];
				n++;
			NEXT_ITEM(en, arg)
		}

		goto ddd;
	} else {
ddd:
		// This is local variable in macro
		// All labels are local in macro
		if (VarIsLocal(var, macro) || VarIsLabel(var)) {
			arg = VarSetFind(locals, var);
			if (arg == NULL) {
				arg = NewTempVar(CellType(var));
				VarSetAdd(locals, var, arg);
			}
			return arg;
		}
	}
	return var;
}

/*

When parsing macro, any assignment to temporary variable other than specified by argument is executed, instead of
generating it.

*/

extern Var * MACRO_ARG[MACRO_ARG_CNT];
/*
void GenMacroParse(Var * macro, Var ** args)
Purpose:
	Expand macro when parsing.
	This will generate line instruction if necessary.
{
	GenMacro(macro, args);
}
*/
#define FOR_EACH_INSTR(I, BLK) for(I = BLK->first; I != NULL; I = I->next) {
#define NEXT_INSTR }

void GenMacro(Var * macro, Var ** args)
/*
Purpose:
	Generate instructions from specified macro or procedure.
	Macro arguments are replaced by variables specified in 'args' array.
	Variables and labels local to macro are replaced by copies with automatic names.
Argument:
	macro	Variable containing the macro to expand.
	args	Macro arguments (according to macro header).
*/{
	Instr * i;	//, * i2;
	InstrOp op;
	Var * result, * arg1, * arg2, * r;
	VarSet locals;
	InstrBlock * blk;
	Bool local_result;

	ASSERT(macro->mode == INSTR_VAR);
	ASSERT(macro->type->mode == INSTR_FN);

	
//	blk = macro->instr;
//	if (blk == NULL) return;

	VarSetInit(&locals);

	for(blk = FnVarCode(macro); blk != NULL; blk = blk->next) {

		if (blk->label != NULL) {
			arg1 = GenArg(macro, blk->label, args, &locals);
			GenLabel(arg1);
		}

		FOR_EACH_INSTR(i, blk)
			op = i->op;
			local_result = false;

			// In case, we are generating variable (means we are inlining a procedure), generate lines too
			// Macro may contain NOP instruction, we do not generate it to result
			if (op != INSTR_VOID) {

				result = i->result;
				if (result != NULL) {
					// %Z variable is used as forced local argument.
					local_result = VarIsArg(result) && result->idx == ('Z' - 'A' + 1);
					if (!local_result) {
						result = GenArg(macro, result, args, &locals);
					}
				}
				arg1 = GenArg(macro, i->arg1, args, &locals);
				arg2 = GenArg(macro, i->arg2, args, &locals);

				arg1 = VarEvalConst(arg1);
				arg2 = VarEvalConst(arg2);

				// Try to evaluate constant instruction to prevent generating excess instructions.

				r = InstrEvalConst(op, arg1, arg2);
				if (r != NULL) {
					op = INSTR_LET; arg1 = r; arg2 = NULL;
				}

				// There can be temporary variable, which didn't get type, because macro argument type
				// was not known. We should derive types in a better way, but this hack
				// should suffice for now.

				if (result != NULL && result->type == NULL) result->type = arg1->type;

				// If we are setting the value to local variable, do not generate instruction

				if (local_result) {
					if (op == INSTR_LET) {
						args[25] = arg1;
					} else {
						SyntaxError("failed to evaluate constant");
					}
				} else {
					if (PHASE == PHASE_TRANSLATE) {
	//					if (result->mode == INSTR_VAR && StrEqual(result->name, "eadx")) {
	//						Print("x");
	//					}
						if (!InstrTranslate3(op, result, arg1, arg2, 0)) {
							SyntaxError("Translation for instruction not found");
						}
					} else {
						GenInternal(op, result, arg1, arg2);
					}
				}

			}
		NEXT_INSTR
	}
	VarSetCleanup(&locals);
}

void GenerateInit()
{
	FIRST_BLK = BLK = NewCode();
	BLK->seq_no = 1;


	memset(&ROOT_PROC, 0, sizeof(ROOT_PROC));
	ROOT_PROC.mode = INSTR_VAR;
	ROOT_PROC.name2 = "root";
	ROOT_PROC.idx  = 0;
	ROOT_PROC.type = NewFn(NewFnType(VOID, VOID), FIRST_BLK);

	// Initialize procedure used to evaluate rules and it's arguments (A-Z)

	InScope(&ROOT_PROC);

	IBLOCK_STACK_SIZE = 0;
	INSTR = NULL;

	RuleSetInit(&GEN_RULES);

}
