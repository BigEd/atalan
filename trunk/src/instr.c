/*

Compiler instructions management

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php


*/

#include "language.h"
#define MAX_RULE_UNROLL 20
/*

Intruction generating function always write to this block.
Blocks may be nested, because procedures may be nested.

*/

GLOBAL Var * INSTRSET;			// enumerator with instructions

GLOBAL InstrBlock * CODE;
GLOBAL InstrBlock * IBLOCK_STACK[128];
GLOBAL UInt16       IBLOCK_STACK_SIZE;
GLOBAL Var * MACRO_ARG[MACRO_ARG_CNT];
GLOBAL Type  ROOT_PROC_TYPE;
GLOBAL Var   ROOT_PROC;

extern GLOBAL Var * MACRO_ARG_VAR[MACRO_ARG_CNT];
extern Var * VARS;		// global variables

Var * InstrFind(char * name)
{
	Var * var;

	if (INSTRSET == NULL) {
		INSTRSET  = VarFindScope(&ROOT_PROC, "instrs", 0);
	}

	var = VarFindScope(INSTRSET, name, 0);
	return var;
}
// Code used for reading

Var * InstrFindCode(UInt16 code)
{
	Var * var;
	var = VarFindInt(INSTRSET, code);
	return var;
}

void InstrBlockPush()
/*
Purpose:
	Create new code block and let next instructions be generated into this block.
*/
{
	IBLOCK_STACK[IBLOCK_STACK_SIZE] = CODE;
	IBLOCK_STACK_SIZE++;
	CODE = MemAllocStruct(InstrBlock);
}

InstrBlock * InstrBlockPop()
/*
Purpose:
	Pop last pushed code block.
*/
{
	InstrBlock * blk;
	blk = CODE;
	IBLOCK_STACK_SIZE--;
	CODE = IBLOCK_STACK[IBLOCK_STACK_SIZE];
	return blk;
}

void GenBlock(InstrBlock * blk)
/*
Purpose:
	Generate block of code.
	The code is attached to the generated output (copy is not made).
*/
{
	if (blk != NULL && blk->first != NULL) {
		blk->first->prev = CODE->last;
		blk->last->next  = NULL;
		if (CODE->last != NULL) {
			CODE->last->next = blk->first;
		}
		CODE->last = blk->last;
		if (CODE->first == NULL) {
			CODE->first = blk->first;
		}
		free(blk);	// free just block head
	}
}

void InstrFree(Instr * i)
{
	if (i != NULL) {
		free(i);
	}
}

void InstrBlockFree(InstrBlock * blk)
{
	Instr * i, * next;

	if (blk != NULL) {
		for(i = blk->first; i != NULL; i = next) {
			next = i->next;
			InstrFree(i);
		}
		blk->first = blk->last = NULL;
	}
}

Instr * InstrDelete(InstrBlock * blk, Instr * i)
/*
Purpose:
	Delete the instruction from specified block.
	Instruction MUST be part of the specified block.
	Return pointer to next instruction after the deleted instruction.
*/
{
	Instr * next = NULL;

	if (blk != NULL && i != NULL) {

		next = i->next;

		if (i->prev != NULL) {
			i->prev->next = next;
		} else {
			blk->first = next;
		}

		if (next != NULL) {
			i->next->prev = i->prev;
		} else {
			blk->last = i->prev;
		}

		InstrFree(i);
	}
	return next;
}

void InstrInsert(InstrBlock * blk, Instr * before, InstrOp op, Var * result, Var * arg1, Var * arg2)
/*
Purpose:
	Insert instruction before specified instruction in specified block.
	If before instruction is NULL, instruction is inserted to the end of block.
*/
{
	Instr * i = MemAllocStruct(Instr);

	// Attach the new instruction to doubly linked list

	if (before == NULL) {
		i->prev = blk->last;
		if (blk->last != NULL) {
			blk->last->next = i;
		}
		blk->last = i;
		if (blk->first == NULL) blk->first = i;
	} else {
		i->prev = before->prev;
		i->next = before;

		if (before->prev != NULL) {
			before->prev->next = i;
		} else {
			blk->first = i;
		}
		before->prev = i;
	}


	i->op = op;
	i->result = result;
	i->arg1 = arg1;
	i->arg2 = arg2;

}

InstrOp OpNot(InstrOp op)
{
	return op ^ 1;
}


/*
char * INSTR_NAME[] = {
	"nop",     
	"let",    

	"ifeq",
	"ifne",
	"iflt",
	"ifge",
	"ifgt",
	"ifle",

	"prologue",
	"epilogue",
	"emit",      
	"vardef",  
	"label",   
	"goto",   
	"add",
	"sub",
	"mul",
	"div",
	"and",
	"or",
	"letarr",
	"getarr",

	"alloc",
	"print",
	"proc",
	"endproc",
	"call",
	"byte"
};
*/

LineNo CURRENT_LINE_NO;

void GenLine()
{
	char * line;
	UInt32 line_no;
	UInt16 line_len;

	// Generate LINE instruction.
	// Line instructions are used to be able to reference back from instructions to line of source code.
	// That way, we can report logical errors detected in instructions to user.

	if (!SYSTEM_PARSE && CURRENT_LINE_NO != LINE_NO) {
		InstrInsert(CODE, NULL, INSTR_LINE, NULL, NULL, NULL);
		line = LINE;
		line_no = LINE_NO;
		if (LINE_POS == 0) {
			line = PREV_LINE;
			line_no--;
		}
		line_len = StrLen(line)-1;
		CODE->last->result    = SRC_FILE;
		CODE->last->line_no = line_no;
		CODE->last->line = StrAllocLen(line, line_len);
		CURRENT_LINE_NO = line_no;
	}
}

void InternalGen(InstrOp op, Var * result, Var * arg1, Var * arg2)
{
	Var * var;
	// For commutative or relational operations make sure the constant is the other operator
	// This simplifies further code processign.

	if (op == INSTR_ADD || op == INSTR_MUL || op == INSTR_OR || op == INSTR_AND || op == INSTR_XOR || IS_INSTR_BRANCH(op)) {
		if (op != INSTR_IFOVERFLOW && op != INSTR_IFNOVERFLOW) {
			if (arg1->mode == MODE_CONST) {
				var = arg1; arg1 = arg2; arg2 = var;

				// Change oriantation of non commutative relational operators
				// This is different from NOT operation.

				switch(op) {
				case INSTR_IFLE: op = INSTR_IFGE; break;
				case INSTR_IFGE: op = INSTR_IFLE; break;
				case INSTR_IFGT: op = INSTR_IFLT; break;
				case INSTR_IFLT: op = INSTR_IFGT; break;
				default: break;
				}

			}
		}
	}
	InstrInsert(CODE, NULL, op, result, arg1, arg2);
}

void Gen(InstrOp op, Var * result, Var * arg1, Var * arg2)
/*
Purpose:
	Generate instruction into current code block.
*/
{
	GenLine();
	InternalGen(op, result, arg1, arg2);
}

void GenLet(Var * result, Var * arg1)
{
	Gen(INSTR_LET, result, arg1, NULL);
}

void GenGoto(Var * label)
{
	if (label != NULL) {
		Gen(INSTR_GOTO, label, NULL, NULL);
	}
}

void GenLabel(Var * var)
{
	if (var != NULL) {
		if (var->type == NULL) {
			VarToLabel(var);
		}
		InternalGen(INSTR_LABEL, var, NULL, NULL);
	}
}

void GenLastResult(Var * var)
/*
Purpose:
	Set result of last generated instruction.
*/
{
	CODE->last->result = var;
}

Var * InstrEvalConst(InstrOp op, Var * arg1, Var * arg2)
/*
Purpose:
	Try to evaluate instruction.
	Instruction may be avaluated if it's arguments are constant.
Result:
	Return evaluated variable as constant or NULL, if instruction can not be evaluated.
*/
{
	Var * r = NULL;

	if (VarIsConst(arg1) && (arg2 == NULL || VarIsConst(arg2))) {
		
		if (arg1->type == NULL || arg1->type->variant != TYPE_INT) return NULL;

		switch(op) {
			case INSTR_SQRT:
				if (arg1->n >= 0) {
					r = VarNewInt((UInt32)sqrt(arg1->n));
				} else {
					// Error: square root of negative number
				}
				break;
			case INSTR_LO:
				r = VarNewInt(arg1->n & 0xff);
				break;
			case INSTR_HI:
				r = VarNewInt((arg1->n >> 8) & 0xff);
				break;
			case INSTR_DIV:
				r = VarNewInt(arg1->n / arg2->n);
				break;
			case INSTR_MOD:
				r = VarNewInt(arg1->n % arg2->n);
				break;
			case INSTR_MUL:
				r = VarNewInt(arg1->n * arg2->n);
				break;
			case INSTR_ADD:
				r = VarNewInt(arg1->n + arg2->n);
				break;
			case INSTR_SUB:
				r = VarNewInt(arg1->n - arg2->n);
				break;
			case INSTR_AND:
				r = VarNewInt(arg1->n & arg2->n);
				break;
			case INSTR_OR:
				r = VarNewInt(arg1->n | arg2->n);
				break;
			case INSTR_XOR:
				r = VarNewInt(arg1->n ^ arg2->n);
				break;
			default: break;
		}
	}
	return r;
}

/**************************************************************

 Rules

 Rules define, how to process instructions.

**************************************************************/
//$R

GLOBAL Rule * RULES[INSTR_CNT];
GLOBAL Rule * LAST_RULE[INSTR_CNT];

GLOBAL Rule * EMIT_RULES[INSTR_CNT];
GLOBAL Rule * LAST_EMIT_RULE[INSTR_CNT];

void RuleRegister(Rule * rule)
{
	InstrOp op = rule->op;
	if (!rule->to->first) InternalError("Empty rule");

	if (rule->to->first->op == INSTR_EMIT) {
		if (LAST_EMIT_RULE[op] != NULL) LAST_EMIT_RULE[op]->next = rule;
		if (EMIT_RULES[op] == NULL) EMIT_RULES[op] = rule;
		LAST_EMIT_RULE[op] = rule;
	} else {
		if (LAST_RULE[op] != NULL) LAST_RULE[op]->next = rule;
		if (RULES[op] == NULL) RULES[op] = rule;
		LAST_RULE[op] = rule;
	}
}


static Bool ArgMatch(RuleArg * pattern, Var * arg, Bool in_tuple);

Bool VarMatchesPattern(Var * var, RuleArg * pattern)
{
	Type * type = pattern->type;
	Type * vtype = var->type;
//	RuleArg * idx, * idx2;

//	if (pattern->variant == RULE_ELEMENT) {
//		if (!VarIsArrayElement(var)) return false;		// pattern expects element, and variable is not an element
//		if (!ArgMatch(pattern->index, var->var)) return false;
//		return ArgMatch(pattern->arr, var->adr);
//	}

	// Pattern expects reference to array with one or more indices
	if (pattern->index != NULL) {
		if (var->mode == MODE_ELEMENT) {
			if (VarIsStructElement(var)) {
				// This is reference to structure
				// We may treat it as normal variable
//				printf("");
			} else {
				// 1D index
				if (!ArgMatch(pattern->index, var->var, false)) return false;
//				if (!VarMatchesType(var->adr, type)) return false;
				return true;
			}
		} else {
			return false;
		}

	// Pattern does not expect any index, but we have some, so there is no match
	} else {
		if (VarIsArrayElement(var)) return false;
	}

	return VarMatchesType(var, type);
}

static Bool ArgMatch(RuleArg * pattern, Var * arg, Bool in_tuple)
{
	Type * atype;
	Var * pvar;
	UInt8 j;

	if (arg == NULL) return pattern->variant == RULE_ANY;
	atype = arg->type;
	
	switch(pattern->variant) {
	case RULE_RANGE:
		if (arg->mode != MODE_RANGE) return false;		// pattern expects element, and variable is not an element
		if (!ArgMatch(pattern->index, arg->var, false)) return false;
		return ArgMatch(pattern->arr, arg->adr, false); 
		break;

	case RULE_TUPLE:
		if (arg->mode != MODE_TUPLE) return false;
		if (!ArgMatch(pattern->index, arg->var, true)) return false;
		return ArgMatch(pattern->arr, arg->adr, true); 
		break;

	case RULE_ELEMENT:
		if (arg->mode != MODE_ELEMENT) return false;		// pattern expects element, and variable is not an element
		if (!ArgMatch(pattern->index, arg->var, false)) return false;
		return ArgMatch(pattern->arr, arg->adr, false); 
		break;

	case RULE_CONST:	// var
		if (!VarIsConst(arg)) return false;
		if (!VarMatchesPattern(arg, pattern)) return false;
		break;

	case RULE_VALUE:
		if (!VarIsConst(arg)) return false;
		pvar = pattern->var;
		if (pvar->value_nonempty) {
			switch (pvar->type->variant) {
			case TYPE_INT: 
				if (pvar->n != arg->n) return false;
				break;
			case TYPE_STRING:
				if (!StrEqual(pvar->str, arg->str)) return false;
				break;
			default: break;
			}
		} else {
			// TODO: Test, that arg const matches the specified type
		}
		break;

	case RULE_REGISTER:
		if (pattern->var != NULL && arg != pattern->var) return false;
		break;

	case RULE_VARIABLE:
		if (arg->mode == MODE_CONST) return false;
		if (FlagOn(arg->submode, SUBMODE_REG)) return false;
		if (!VarMatchesPattern(arg, pattern)) return false;
		break;
	
	case RULE_DEREF:
		if (arg->mode != MODE_DEREF) return false;
//		if (arg->mode == MODE_CONST) return false;
//		if (FlagOff(arg->submode, SUBMODE_REF)) return false;
		arg = arg->var;
		if (!VarMatchesPattern(arg, pattern)) return false;
		break;

	// We get array as an argument. We are interested in the type of it's element.
//	case RULE_ARRAY_ARG:
//		if (arg->mode == MODE_ELEMENT) return false;
//		if (atype->variant != TYPE_ARRAY) return false;
//		while(atype->element->variant == TYPE_ARRAY) atype = atype->element;
//		if (pattern->type != NULL) {
//			if (!TypeIsSubsetOf(atype->element, pattern->type)) return false;
//			if (!VarMatchesPattern(arg, pattern)) return false; 
//			printf("");
//		}
//		break;

	case RULE_ARG:
		if (arg->mode == MODE_DEREF) return false;
		if (!in_tuple && FlagOn(arg->submode, SUBMODE_REG)) return false; 
//		if (FlagOn(arg->submode, SUBMODE_REF)) return false;
		if (!VarMatchesPattern(arg, pattern)) return false;
		break;

	case RULE_ANY:
		break;

	default: break;
	}

	// If there is macro argument number %A-%Z specified in rule argument, we set or check it here

	if (pattern->arg_no != 0) {

		// For array element variable store array into the macro argument

		pvar = arg;
//		if (arg->mode == MODE_ELEMENT && !VarIsStructElement(arg)) {
//			pvar = arg->adr;
//		}

		j = pattern->arg_no-1;
		if (MACRO_ARG[j] == NULL) {
			MACRO_ARG[j] = pvar;
		} else {
			if (MACRO_ARG[j] != pvar) return false;
		}

		// Set the index items

//		if (pattern->index != NULL) {
//			if (!ArgMatch(pattern->index, arg->var)) return false;
//		}
	}
	return true;
}


//GLOBAL Var * G_MATCH_ARG[MACRO_ARG_CNT];

GLOBAL Bool RULE_MATCH_BREAK;

void EmptyMacros()
{
	UInt8 n;
	for(n=0; n<MACRO_ARG_CNT; n++) {
		MACRO_ARG[n] = NULL;
	}
}

Bool RuleMatch(Rule * rule, Instr * i)
{
	Bool match;

	if (i->op != rule->op) return false;

	EmptyMacros();

	match = ArgMatch(&rule->arg[0], i->result, false) 
		&& ArgMatch(&rule->arg[1], i->arg1, false) 
		&& ArgMatch(&rule->arg[2], i->arg2, false);

	if (match) {
		if (RULE_MATCH_BREAK) {
			RULE_MATCH_BREAK = true;
		}
	}
	return match;
}

Var * VarNextLocal(Var * scope, Var * local)
{
	while(true) {
		local = local->next;
		if (local == NULL) break;
		if (local->scope == scope) break;
	}
	return local;
}

Var * VarFirstLocal(Var * scope)
{
	return VarNextLocal(scope, VARS);
}

Var * NextArg(Var * proc, Var * arg, VarSubmode submode)
{
	Var * var = arg->next;
	while(var != NULL && (var->mode != MODE_ARG || var->scope != proc || FlagOff(var->submode, submode))) var = var->next;
	return var;
}

Var * FirstArg(Var * proc, VarSubmode submode)
{
	return NextArg(proc, proc, submode);
}

Var * FindArg(Var * macro, Var * var, Var ** args)
/*
Purpose:
	Find function (macro) argument or structure member.
*/
{

	Var * arg, * arr;
	UInt16 n;

	if (var != NULL) {

		// If this is element reference and either array or index is macro argument,
		// create new array element referencing actual array and index.
		
		if (var->mode == MODE_ELEMENT) {
			arr = FindArg(macro, var->adr, args);
			arg = FindArg(macro, var->var, args);	// index

			if (arr != var->adr || arg != var->var) {
				var = VarNewElement(arr, arg);
			}
		} else {

			// Positional macro argument (1 - 26)
			if (var->mode == MODE_ARG && var->name == NULL) {
				return args[var->idx-1];
			}

			if (macro != NULL) {
				for(n = 0, arg = FirstArg(macro, SUBMODE_ARG_IN); arg != NULL; arg = NextArg(macro, arg, SUBMODE_ARG_IN), n++) {
					if (arg == var) return args[n];
				}
			}
		}
	}
	return var;
}

/*

When parsing macro, any assignment to temporary variable other than specified by argument is executed, instead of
generating it.

*/

void GenMacro(InstrBlock * code, Var * macro, Var ** args)
/*
Purpose:
	Generate instructions based on macro.
	Any variable, that has same name as macro name is replaced by one of variables specified in arguments.
Argument:
	macro	Variable containing the macro to expand.
	args	Macro arguments (according to macro header).
*/{
	Instr * i;
	InstrOp op;
	Var * result, * arg1, * arg2, * r, * lab, * tmp_lab;

	Bool local_result;

	lab = tmp_lab = NULL;

	for(i = code->first; i != NULL; i = i->next) {
		op = i->op;
		local_result = false;
		// Macro may contain NOP instruction, we do not generate it to result
		if (op == INSTR_LINE) {
		} else if (op != INSTR_VOID) {

			// Labels defined in macro are all local.
			// If there is label in macro, generate temporary macro insted of it.

			result = i->result;
			if (result != NULL) {

				local_result = result->mode == MODE_ARG && result->idx == ('Z' - 'A' + 1);

				if (result->mode != MODE_ARG && (i->op == INSTR_LABEL || IS_INSTR_JUMP(i->op))) {

					if (local_result) {
						result = FindArg(macro, i->arg1, args);
					}

					if (result == lab) {
						result = tmp_lab;
					} else if (tmp_lab == NULL) {
						tmp_lab = VarNewTmpLabel();
						lab = i->result;
						result = tmp_lab;
					}
				} else {
					if (!local_result) {
						result = FindArg(macro, result, args);
					}
				}
			}
			arg1 = FindArg(macro, i->arg1, args);
			arg2 = FindArg(macro, i->arg2, args);

			// Try to evaluate constant instruction to prevent genrating excess instructions.

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
					MACRO_ARG[25] = arg1;
				} else {
					SyntaxError("failed to evaluate constant");
				}
			} else {
				Gen(op, result, arg1, arg2);
			}

		}
	}
}

Var * VarReg(Var * var)
/*
Purpose:
	Return register that is aliased by this variable or NULL.
*/
{
	Var * reg;

	reg = var;
	while(reg != NULL && (reg->mode == MODE_VAR || reg->mode == MODE_ARG) && reg->adr != NULL) {
		if (FlagOn(reg->submode, SUBMODE_REG)) return reg;
		reg = reg->adr;
		if (reg->mode == MODE_TUPLE) return reg;
	}
	return var;
}

Bool InstrTranslate(Instr * i, Bool * p_modified)
{
	Rule * rule;
	if (i->op == INSTR_LINE) {
		Gen(INSTR_LINE, i->result, i->arg1, i->arg2);
	} else if (EmitRule(i)) {
		Gen(i->op, i->result, i->arg1, i->arg2);
	} else {
		// Find translating rule
		for(rule = RULES[i->op]; rule != NULL; rule = rule->next) {
			if (RuleMatch(rule, i)) break;
		}

		if (rule != NULL) {
			GenMacro(rule->to, NULL, MACRO_ARG);
			*p_modified = true;
		} else {
			return false;
		}
	}
	return true;
}

void ProcTranslate(Var * proc)
/*
Purpose:
	Translate generic instructions to instructions directly translatable to processor instructions.
*/
{
	Instr * i, * first_i, * next_i;	//, * to;
	InstrBlock * blk;
	Bool modified, untranslated;
	UInt8 step = 0;
	UInt32 ln;
	Var * a = NULL;
	Instr i2;

	// As first step, we translate all variables on register address to actual registers

	for(blk = proc->instr; blk != NULL; blk = blk->next) {
		for(i = blk->first; i != NULL; i = i->next) {
			if (i->op == INSTR_LINE) continue;
			i->result = VarReg(i->result);
			i->arg1   = VarReg(i->arg1);
			i->arg2   = VarReg(i->arg2);
		}
	}

	// We perform as many translation steps as necessary
	// Some translation rules may use other translation rules, so more than one step may be necessary.
	// Translation ends either when there has not been any modification in last step or after
	// defined number of steps (to prevent infinite loop in case of invalid set of translation rules).

	do {
		modified = false;
		untranslated = false;
		ln = 1;
		for(blk = proc->instr; blk != NULL; blk = blk->next) {

			// The translation is done by using procedures for code generating.
			// We detach the instruction list from block and set the block as destination for instruction generator.
			// In this moment, the code generating stack must be empty anyways.

			first_i = blk->first;
			blk->first = blk->last = NULL;
			CODE = blk;
			i = first_i;

			while(i != NULL) {

				if (!InstrTranslate(i, &modified)) {

					// If this is commutative instruction, try the other order of rules

					if (i->op == INSTR_AND || i->op == INSTR_OR || i->op == INSTR_XOR || i->op == INSTR_ADD || i->op == INSTR_MUL) {
						i2.op = i->op;
						i2.result = i->result;
						i2.arg1 = i->arg2;
						i2.arg2 = i->arg1;
						if (InstrTranslate(&i2, &modified)) goto next;
					}

					// No emit rule nor rule for translating instruction found,
					// try to simplify the instruction by first calculating the element value to temporary variable
					// and using the variable instead.

					if (VarIsArrayElement(i->arg1)) {
						a = VarNewTmp(100, i->arg1->type);		//== adr->type->element
						GenLet(a, i->arg1);
						Gen(i->op, i->result, a, i->arg2);
						modified = true;
					} else if (VarIsArrayElement(i->arg2)) {
						a = VarNewTmp(101, i->arg1->type);		//== adr->type->element
						GenLet(a, i->arg2);
						Gen(i->op, i->result, i->arg1, a);
						modified = true;
					} else if (VarIsArrayElement(i->result)) {
						if (i->op != INSTR_LET || (i->arg1->mode != MODE_VAR && i->arg1->mode != MODE_ARG)) {
							a = VarNewTmp(102, i->result->type);
							Gen(i->op, a, i->arg1, i->arg2);
							GenLet(i->result, a);
							modified = true;
						}

					//==== We were not able to find translation for the instruction, emit it as it is
					//     TODO: This is an error, as we are not going to find any translation for the instruction next time.
					} else {
						Gen(i->op, i->result, i->arg1, i->arg2);
						untranslated = true;
					}
				}
next:
				next_i = i->next;
				InstrFree(i);
				i = next_i;
				ln++;
			}
		} // block

		if (VERBOSE) {
			// Do not print unmodified step (it would be same as previous step already printed)
			if (modified) {
				printf("========== Translate (step %d) ============\n", step+1);
				PrintProc(proc);
			}
		}
		step++;
	} while(modified && step < MAX_RULE_UNROLL);

}

/****************************************************************

 Print tokens

****************************************************************/

void PrintVarNameNoScope(Var * var)
{
	printf("%s", var->name);
	if (var->idx > 0) {
		printf("%ld", var->idx-1);
	}
}

void PrintVarName(Var * var)
{
	if (var->scope != NULL && var->scope != &ROOT_PROC && var->scope->name != NULL && !VarIsLabel(var)) {
		PrintVarName(var->scope);
		printf(".");
	}
	PrintVarNameNoScope(var);
}

void PrintVarVal(Var * var)
{
	Type * type;
	Var * index;

	if (var == NULL) return;

	if (var->mode == MODE_DEREF) {
//	if (FlagOn(var->submode, SUBMODE_REF)) {
		printf("@");
		var = var->var;
	}

	if (var->name == NULL) {
		if (var->mode == MODE_ARG) {
			printf("#%ld", var->idx-1);
		} else if (var->mode == MODE_ELEMENT) {
			PrintVarVal(var->adr);
			if (var->adr->type->variant == TYPE_STRUCT) {
				printf(".");
				PrintVarNameNoScope(var->var);
			} else {
				printf("(");
				index = var->var;
				while(index->mode == MODE_ELEMENT) {
					PrintVarVal(index->adr);
					printf(",");
					index = index->var;
				}
				PrintVarVal(index);
				printf(")");
			}
		} else {
			if (var->mode == MODE_RANGE) {
				PrintVarVal(var->adr); printf(".."); PrintVarVal(var->var);
			} else if (var->mode == MODE_TUPLE) {
				printf("(");
				PrintVarVal(var->adr);
				printf(",");
				PrintVarVal(var->var);
				printf(")");
			} else {
				switch(var->type->variant) {
				case TYPE_INT: printf("%ld", var->n); break;
				case TYPE_STRING: printf("'%s'", var->str); break;
				default: break;
				}
			}
		}
	} else {
		PrintVarName(var);

		if (var->adr != NULL) {
			if (var->adr->mode == MODE_TUPLE) {
				printf("@");
				PrintVarVal(var->adr);
			}
		}
	}

	type = var->type;
	if (type != NULL) {
		if (type->variant == TYPE_INT && !VarIsConst(var)) {
			if (type->range.min == 0 && type->range.max == 255) {
			} else {
//				printf(":%ld..%ld", type->range.min, type->range.max);
			}
		}
	}

}

void PrintVarArgs(Var * var)
{
	Var * arg;
	printf("(");
	for(arg = var->next; arg != NULL; arg = arg->next) {
		if (arg->mode == MODE_ARG && arg->scope == var) {
			printf(" %s", arg->name);
		}
	}
	printf(")");
}

void PrintVar(Var * var)
{
	Type * type;

	if (var->mode == MODE_DEREF) {
		printf("@");
		var = var->var;
	}
//	if (FlagOn(var->submode, SUBMODE_REF)) {
//		printf("@");
//	}

	if (var->mode == MODE_ELEMENT) {
		PrintVarName(var->adr);
		printf("(");
		PrintVar(var->var);
		printf(")");
	} else if (var->mode == MODE_CONST) {
		printf("%ld", var->n);
		return;
	} else {

		PrintVarName(var);

		if (var->adr != NULL) {
			printf("@");
			PrintVarVal(var->adr);
		}

		type = var->type;
		if (type != NULL) {
			if (type->variant == TYPE_PROC) {
				printf(":proc");
				PrintVarArgs(var);
			} else if (type->variant == TYPE_MACRO) {
				printf(":macro");
				PrintVarArgs(var);
			}
		}

		if (VarIsConst(var)) {
			printf(" = %ld", var->n);
		} else {
			type = var->type;
			if (type != NULL) {
				if (type->variant == TYPE_INT) {
					if (type->range.min == 0 && type->range.min == 255) {
					} else {
						printf(":%ld..%ld", type->range.min, type->range.max);
					}
				}
			}
		}
	}
	printf("  R%ld W%ld\n", var->read, var->write);
}

void InstrPrintInline(Instr * i)
{
	Var * inop;
	Bool r = false, a1 = false;

	if (i->op == INSTR_LINE) {
		PrintColor(BLUE);
		printf(";%s(%d) %s", i->result->name, i->line_no, i->line);
		PrintColor(RED+GREEN+BLUE);
	} else if (i->op == INSTR_LABEL) {
		PrintVarVal(i->result);
		printf("@");
	} else {
		inop = InstrFindCode(i->op);
		printf("   %s", inop->name);
	
		if (i->result != NULL) {
			printf(" ");
			PrintVarVal(i->result);
			r = true;
		}

		if (i->arg1 != NULL) {
			if (r) {
				printf(", ");
			} else {
				printf(" ");
			}

			PrintVarVal(i->arg1);
		}

		if (i->arg2 != NULL) {
			printf(", ");
			PrintVarVal(i->arg2);
		}
	}
}

void InstrPrint(Instr * i)
{
	InstrPrintInline(i);
	printf("\n");
}


void CodePrint(InstrBlock * blk)
{
	Instr * i;
	UInt32 n = 1;
	while (blk != NULL) {
		if (blk->label != NULL) {
			printf("    ");
			PrintVarVal(blk->label);
			printf("@");
		}
		if (blk->seq_no != 0) {
			printf("   (#%d)\n", blk->seq_no);
		} else {
			printf("\n");
		}
		for(i = blk->first; i != NULL; i = i->next, n++) {
			printf("%3ld| ", n);
			InstrPrint(i);
		}

		blk = blk->next;
	}
}

void PrintProc(Var * proc)
{
	CodePrint(proc->instr);
}

//$I
void InstrInit()
{
	UInt16 op;

	ROOT_PROC_TYPE.variant = TYPE_PROC;
//	ROOT_PROC_TYPE.members = NULL;

	memset(&ROOT_PROC, 0, sizeof(ROOT_PROC));
	ROOT_PROC.name = "root";
	ROOT_PROC.idx  = 0;
	ROOT_PROC.type = &ROOT_PROC_TYPE;
	ROOT_PROC.instr = NULL;

	SCOPE = &ROOT_PROC;

	for(op=0; op<INSTR_CNT; op++) {
		RULES[op] = NULL;
		LAST_RULE[op] = NULL;
		EMIT_RULES[op] = NULL;
		LAST_EMIT_RULE[op] = NULL;
	}

	CODE = NULL;

	// Alloc instruction block for root procedure.
	
	IBLOCK_STACK_SIZE = 0;
	InstrBlockPush();
	IBLOCK_STACK_SIZE = 0;

	CURRENT_LINE_NO = 0;

}

