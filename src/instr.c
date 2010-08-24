/*

Compiler instructions management

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php


*/

#include "language.h"

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
	CODE->first = CODE->last = NULL;
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

void Gen(InstrOp op, Var * result, Var * arg1, Var * arg2)
/*
Purpose:
	Generate function into specified code.
*/
{
	Var * var;
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

	// For commutative or relational operations make sure the constant is the other operator
	// This simplifies further code processign.

	if (op == INSTR_ADD || op == INSTR_MUL || op == INSTR_OR || op == INSTR_AND || op == INSTR_XOR || IS_INSTR_BRANCH(op)) {
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

	InstrInsert(CODE, NULL, op, result, arg1, arg2);
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
		Gen(INSTR_LABEL, var, NULL, NULL);
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


static Bool ArgMatch(RuleArg * pattern, Var * arg);

Bool VarMatchesType(Var * var, RuleArg * pattern)
{
	Type * type = pattern->type;
	Type * vtype = var->type;

	// Pattern expects reference to array with one or more indeces
	if (pattern->index != NULL) {
		if (var->mode != MODE_ELEMENT) return false;		// this should be a(x) and it is not array reference
		if (!ArgMatch(pattern->index, var->var)) return false;
		//TODO: Match type of array item (we suppose byte not)
		return true;

	// Pattern does not expect any index, but we have some
	} else {
		if (var->mode == MODE_ELEMENT) return false;
	}

	if (type == vtype) return true;

	// If pattern has no defined type, it fits
	if (type == NULL) return true;


	if (type->variant == TYPE_INT) {
		if (vtype != NULL) {
			// If variable is constant, the check is different
			if (var->mode == MODE_CONST) {
				if (vtype->variant == TYPE_INT) {
					if (var->n < type->range.min) return false;
					if (var->n > type->range.max) return false;
				}
			} else if (var->mode == MODE_ELEMENT) {
				// Specified variable is element, but the type is not array
				if (type->variant != TYPE_ARRAY) return false;
			} else {
				if (vtype->variant != TYPE_INT) return false;
				if (type->range.max < vtype->range.max) return false;
				if (type->range.min > vtype->range.min) return false;
			}
		}
	} else if (type->variant == TYPE_ARRAY) {
		if (vtype == NULL) return false;
		if (vtype->variant != TYPE_ARRAY) return false;

		// Match first index, second index, return type

		return TypeIsSubsetOf(vtype->dim[0], type->dim[0])
			&& TypeIsSubsetOf(vtype->dim[1], type->dim[1])
			&& TypeIsSubsetOf(vtype->element, type->element);
		
	} else if (type->variant == TYPE_ADR) {
		if (vtype != NULL) {
			if (vtype->variant != TYPE_ADR) return false;
		}
	} else if (type->variant == TYPE_PROC) {

		// Interrupt routines types will be based on some other type
		if (vtype->base == NULL && type->base == NULL) return true;

		// Procedure type is only same, if it is exactly the same
		return false;

	}
	return true;
}

static Bool ArgMatch(RuleArg * pattern, Var * arg)
{
	Type * atype;
	Var * pvar;
	UInt8 j;

	if (arg == NULL) return pattern->variant == RULE_ANY;
	atype = arg->type;
	
	switch(pattern->variant) {
	case RULE_CONST:	// var
		if (!VarIsConst(arg)) return false;
		if (!VarMatchesType(arg, pattern)) return false;
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
		if (!VarMatchesType(arg, pattern)) return false;
		break;
	
	case RULE_DEREF:
		if (arg->mode == MODE_CONST) return false;
		if (FlagOff(arg->submode, SUBMODE_REF)) return false;
		if (!VarMatchesType(arg, pattern)) return false;
		break;

	case RULE_ARG:
		if (FlagOn(arg->submode, SUBMODE_REG)) return false; 
		if (!VarMatchesType(arg, pattern)) return false;
		break;

	case RULE_ANY:
		break;

	default: break;
	}

	// If there is macro argument number %A-%Z specified in rule argument, we set or check it here

	if (pattern->arg_no != 0) {

		// For element variable store array into the macro argument

		pvar = arg;
		if (arg->mode == MODE_ELEMENT) {
			pvar = arg->adr;
		}

		j = pattern->arg_no-1;
		if (MACRO_ARG[j] == NULL) {
			MACRO_ARG[j] = pvar;
		} else {
			if (MACRO_ARG[j] != pvar) return false;
		}

		// Set the index items

		if (pattern->index != NULL) {
			if (!ArgMatch(pattern->index, arg->var)) return false;
		}
	}
	return true;
}


//GLOBAL Var * G_MATCH_ARG[MACRO_ARG_CNT];

GLOBAL Bool RULE_MATCH_BREAK;

Bool RuleMatch(Rule * rule, Instr * i)
{
	UInt8 n;
	Bool match;

	if (i->op != rule->op) return false;

	for(n=0; n<MACRO_ARG_CNT; n++) {
		MACRO_ARG[n] = NULL;
	}


	match = ArgMatch(&rule->arg[0], i->result) 
		&& ArgMatch(&rule->arg[1], i->arg1) 
		&& ArgMatch(&rule->arg[2], i->arg2);

	if (match) {
		if (RULE_MATCH_BREAK) {
			RULE_MATCH_BREAK = true;
		}
	}
	return match;
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
				var = VarNewElement(arr, arg, FlagOn(var->submode, SUBMODE_REF));
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

Bool ProcTranslate(Var * proc)
{
	Instr * i;	//, * to;
	Rule * rule;
	InstrBlock * blk;
//	Var * arg[3];
	Bool modified;
	UInt8 step = 0;
	UInt32 ln;
	Var * a = NULL;

	if (proc->instr == NULL) return true;

	CODE = NULL;

	do {
		modified = false;
		InstrBlockPush();

		for(ln = 1, i = proc->instr->first; i != NULL; i = i->next, ln++) {

			// We only preprocess instructions, that do not have emit rule yet.
		
//			if (step == 0 && ln == 2) {
//				RULE_MATCH_BREAK = true;
//			}

			if (i->op == INSTR_LINE) {
				Gen(INSTR_LINE, i->result, i->arg1, i->arg2);
			} else if (!EmitRule(i)) {

				for(rule = RULES[i->op]; rule != NULL; rule = rule->next) {
					if (RuleMatch(rule, i)) break;
				}

				if (rule != NULL) {
					GenMacro(rule->to, NULL, MACRO_ARG);
					modified = true;
				} else {

					// No emit rule nor rule for translating instruction found,
					// try to simplify the instruction.

					if (i->arg1 != NULL && i->arg1->mode == MODE_ELEMENT) {
						a = VarNewTmp(100, i->arg1->type);		//== adr->type->element
						Gen(INSTR_LET, a, i->arg1, NULL);
						Gen(i->op, i->result, a, i->arg2);
						modified = true;
					} else {
						Gen(i->op, i->result, i->arg1, i->arg2);
					}
				}
			// Emit rule exists, just copy the instructions
			} else {
				Gen(i->op, i->result, i->arg1, i->arg2);
			}
		}
		blk = InstrBlockPop();
		InstrBlockFree(proc->instr);
		proc->instr = blk;

		if (VERBOSE) {
			printf("=========== Step %d ===============\n", step+1);
			PrintProc(proc);
		}
		step++;
	} while(modified && step < 3);

	return modified;
}

/****************************************************************

 Print tokens

****************************************************************/

void PrintVarName(Var * var)
{
	if (var->scope != NULL && var->scope != &ROOT_PROC && var->scope->name != NULL && !VarIsLabel(var)) {
		PrintVarName(var->scope);
		printf(".");
	}
	printf("%s", var->name);
	if (var->idx > 0) {
		printf("%ld", var->idx-1);
	}
}


void PrintVarVal(Var * var)
{
	Type * type;
	Var * index;

	if (var == NULL) return;

	if (FlagOn(var->submode, SUBMODE_REF)) {
		printf("@");
	}

	if (var->name == NULL) {
		if (var->mode == MODE_ARG) {
			printf("#%ld", var->idx-1);
		} else if (var->mode == MODE_ELEMENT) {
			PrintVarVal(var->adr);
			printf("(");
			index = var->var;
			while(index->mode == MODE_ELEMENT) {
				PrintVarVal(index->adr);
				printf(",");
				index = index->var;
			}
			PrintVarVal(index);
			printf(")");
		} else {
			switch(var->type->variant) {
			case TYPE_INT: printf("%ld", var->n); break;
			case TYPE_STRING: printf("'%s'", var->str); break;
			default: break;
			}
		}
	} else {
		PrintVarName(var);
	}

	type = var->type;
	if (type != NULL) {
		if (type->variant == TYPE_INT && !VarIsConst(var)) {
			if (type->range.min == 0 && type->range.max == 255) {
			} else {
				printf(":%ld..%ld", type->range.min, type->range.max);
			}
		}
	}

}

void PrintVar(Var * var)
{
	Type * type;
	Var * arg;

	if (FlagOn(var->submode, SUBMODE_REF)) {
		printf("@");
	}

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
			} else if (type->variant == TYPE_MACRO) {
				printf(":macro");
			}
			printf("(");
			for(arg = var->next; arg != NULL; arg = arg->next) {
				if (arg->mode == MODE_ARG && arg->scope == var) {
					printf(" %s", arg->name);
				}
			}
			printf(")");
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

void InstrPrint(Instr * i)
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
	printf("\n");
}


void CodePrint(InstrBlock * blk)
{
	Instr * i;
	UInt32 n;
	if (blk != NULL) {
		for(n = 1, i = blk->first; i != NULL; i = i->next, n++) {
			printf("%3ld| ", n);
			InstrPrint(i);
		}
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

