/*

Compiler instructions management

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php


*/

#include "language.h"
#define MAX_RULE_UNROLL 20
/*

Instruction generating function always write to this block.
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

void InstrDetach(InstrBlock * blk, Instr * first, Instr * last)
{
	Instr * next;

	if (blk != NULL && first != NULL) {

		next = last->next;
		if (first->prev != NULL) {
			first->prev->next = next;
		} else {
			blk->first = next;
		}

		if (next != NULL) {
			next->prev = first->prev;
		} else {
			blk->last = first->prev;
		}

		first->prev = NULL;
		last->next  = NULL;
	}
}

void InstrAttach(InstrBlock * blk, Instr * before, Instr * first, Instr * last)
{

	if (before == NULL) {
		first->prev = blk->last;
		if (blk->last != NULL) {
			blk->last->next = first;
		}
		blk->last = last;
		if (blk->first == NULL) blk->first = first;
		last->next = NULL;
	} else {
		first->prev = before->prev;
		last->next = before;

		if (before->prev != NULL) {
			before->prev->next = first;
		} else {
			blk->first = first;
		}
		before->prev = first;
	}
}

void InstrMoveCode(InstrBlock * to, Instr * before, InstrBlock * from, Instr * first, Instr * last)
/*
Purpose:
	Move piece of code (list of instructions) from one block to another.
*/
{
	InstrDetach(from, first, last);
	InstrAttach(to, before, first, last);
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
		if (i->op == INSTR_LINE) {
//			free(i->line);
		}
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

	// use Detach
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
	i->op = op;
	i->result = result;
	i->arg1 = arg1;
	i->arg2 = arg2;

	InstrAttach(blk, before, i, i);
}

InstrOp OpNot(InstrOp op)
{
	return op ^ 1;
}

InstrOp OpRelSwap(InstrOp op)
/*
Purpose:
	Change orientation of non commutative relational operators
	This is different from NOT operation.
*/{

	switch(op) {
	case INSTR_IFLE: op = INSTR_IFGE; break;
	case INSTR_IFGE: op = INSTR_IFLE; break;
	case INSTR_IFGT: op = INSTR_IFLT; break;
	case INSTR_IFLT: op = INSTR_IFGT; break;
	default: break;
	}
	return op;
}


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
	// This simplifies further code processing.

	if (op == INSTR_ADD || op == INSTR_MUL || op == INSTR_OR || op == INSTR_AND || op == INSTR_XOR || IS_INSTR_BRANCH(op)) {
		if (op != INSTR_IFOVERFLOW && op != INSTR_IFNOVERFLOW) {
			if (arg1->mode == MODE_CONST) {
				var = arg1; arg1 = arg2; arg2 = var;

				op = OpRelSwap(op);

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
	Type * rtype, * atype;


	rtype = result->type;
	atype = arg1->type;

	// TODO: We should test for chain of ADR OF ADR OF ADR ....
	//       Error should be reported when assigning address of incorrect type

	if (rtype != NULL && atype != NULL && rtype->variant == TYPE_ADR && atype->variant != TYPE_ADR) {
		Gen(INSTR_LET_ADR, result, arg1, NULL);
	} else {
		Gen(INSTR_LET, result, arg1, NULL);
	}
}

void GenGoto(Var * label)
{
	if (label != NULL) {
		InternalGen(INSTR_GOTO, label, NULL, NULL);
	}
}

void GenLabel(Var * var)
{
	if (var != NULL) {
		if (var->type->variant == TYPE_UNDEFINED) {
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
	Instruction may be evaluated if it's arguments are constant.
Result:
	Return evaluated variable as constant or NULL, if instruction can not be evaluated.
*/
{
	Var * r = NULL;

	// Multiplication of A by 1 is same as assigning A
	if (op == INSTR_MUL) {
		if (VarIsN(arg1, 1)) return arg2;
		if (VarIsN(arg2, 1)) return arg1;
	}

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
/*
Purpose:
	Register parsed rule.
*/
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

void RuleArgMarkNonGarbage(RuleArg * rule)
{
	if (rule != NULL) {
		if (rule->variant == RULE_VARIABLE || rule->variant == RULE_CONST) {
			TypeMark(rule->type);
		} else if (rule->variant == RULE_TUPLE || rule->variant == RULE_DEREF) {
			RuleArgMarkNonGarbage(rule->arr);
		}
		RuleArgMarkNonGarbage(rule->index);
	}
}

void RulesMarkNonGarbage(Rule * rule)
{
	UInt8 i;
	while(rule != NULL) {
		for(i=0; i<2; i++) {
			RuleArgMarkNonGarbage(&rule->arg[i]);
		}
		rule = rule->next;
	}
}

void RulesGarbageCollect()
{
	UInt8 op;
	for(op=0; op<INSTR_CNT; op++) {
		RulesMarkNonGarbage(RULES[op]);
		RulesMarkNonGarbage(EMIT_RULES[op]);
	}
}

static Bool ArgMatch(RuleArg * pattern, Var * arg, Bool in_tuple);

Bool VarMatchesPattern(Var * var, RuleArg * pattern)
{
	Type * type = pattern->type;
	Type * vtype = var->type;

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

	case RULE_BYTE:
		if (arg->mode != MODE_BYTE) return false;		// pattern expects byte, and variable is not an byte
		if (!ArgMatch(pattern->index, arg->var, false)) return false;
		return ArgMatch(pattern->arr, arg->adr, false); 
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
		arg = arg->var;
		if (!VarMatchesPattern(arg, pattern)) return false;
		break;

	case RULE_ARG:
		if (arg->mode == MODE_DEREF || arg->mode == MODE_RANGE) return false;
		if (!in_tuple && FlagOn(arg->submode, SUBMODE_REG)) return false; 
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

Var * VarField(Var * var, char * fld_name)
/*
Purpose:
	Return property of variable.
	Following properties are supported:

	min
	max
	step
*/
{
	Var * fld = NULL;
	Type * type;
	TypeVariant vtype;

	type = var->type;
	vtype = type->variant;

	if (vtype == TYPE_INT) {
		if (StrEqual(fld_name, "min")) {
			fld = VarNewInt(type->range.min);
		} else if (StrEqual(fld_name, "max")) {
			fld = VarNewInt(type->range.max);
		}
	} else if (vtype == TYPE_ARRAY) {
		if (StrEqual(fld_name, "step")) {
			fld = VarNewInt(type->step);
		}
	}
	return fld;
}

Var * FindArg(Var * macro, Var * var, Var ** args, VarSet * locals)
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
		
		if (var->mode == MODE_DEREF) {
			arg = FindArg(macro, var->var, args, locals);
			if (arg != var->var) {
				var = VarNewDeref(arg);
			}
		} else if (var->mode == MODE_ELEMENT) {

			arr = FindArg(macro, var->adr, args, locals);

			if (arr != var->adr && var->var->mode == MODE_CONST && var->var->type->variant == TYPE_STRING) {
				var = VarField(arr, var->var->str);
			} else {
				arg = FindArg(macro, var->var, args, locals);	// index

				if (arr != var->adr || arg != var->var) {
					var = VarNewElement(arr, arg);
				}
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

				// This is local variable in macro
				if (var->scope == macro) {
					arg = VarSetFind(locals, var);
					if (arg == NULL) {
						arg = VarAllocScopeTmp(NULL, var->mode, var->type);
						VarSetAdd(locals, var, arg);
					}
					return arg;
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
	VarSet locals;

	Bool local_result;

	VarSetInit(&locals);

	lab = tmp_lab = NULL;

	for(i = code->first; i != NULL; i = i->next) {
		op = i->op;
		local_result = false;
		// Line instructions are not processed in any special way (TODO: We should mark them as macro generated)
		if (op == INSTR_LINE) {

		// Macro may contain NOP instruction, we do not generate it to result
		} else if (op != INSTR_VOID) {

			// Labels defined in macro are all local.
			// If there is label in macro, generate temporary label instead of it.

			result = i->result;
			if (result != NULL) {

				// %Z variable is used as forced local argument.
				local_result = result->mode == MODE_ARG && result->idx == ('Z' - 'A' + 1);

				if (result->mode != MODE_ARG && (i->op == INSTR_LABEL || IS_INSTR_JUMP(i->op))) {

					if (local_result) {
						result = FindArg(macro, i->arg1, args, &locals);
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
						result = FindArg(macro, result, args, &locals);
					}
				}
			}
			arg1 = FindArg(macro, i->arg1, args, &locals);
			arg2 = FindArg(macro, i->arg2, args, &locals);

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

Int32 ByteMask(Int32 n)
/*
Purpose:
	Return power of $ff ($ff, $ffff, $ffffff or $ffffffff) bigger than specified number.
*/
{
	Int32 nmask;
	nmask = 0xff;
	while(n > nmask) nmask = (nmask << 8) | 0xff;
	return nmask;
}


void GenArrayInit(Var * arr, Var * init)
/*
Purpose:
	Generate array initialization loop.
Arguments:
	arr		Reference to array or array element or array slice
*/
{
	Type * type, * src_type, * idx1_type;
	Var * idx, * src_idx, * label, * range, * stop, * label_done;
	Int32 nmask;
	Int32 stop_n;
	Var * min1, * max1, * src_min, * src_max;
	Var * dst_arr;

	src_idx = NULL;
	type = arr->type;
	label = VarNewTmpLabel();
	idx1_type = NULL;

	if (type->variant == TYPE_ARRAY) {
		idx1_type = type->dim[0]	;
	}

	if (arr->mode == MODE_ELEMENT) {

		// If this is array of array, we may need to initialize index variable differently
		
		if (arr->adr->mode == MODE_ELEMENT) {
			idx1_type = arr->adr->type->dim[0];
		} else if (arr->adr->mode == MODE_VAR) {
			idx1_type = arr->adr->type->dim[0];
		}

		range = arr->var;
		if (range->mode == MODE_RANGE) {
			min1 = range->adr;
			max1 = range->var;
		} else {
			min1 = arr->var;
			max1 = VarNewInt(arr->adr->type->dim[0]->range.max);
		}
		dst_arr = arr->adr;
	} else {
		min1 = VarNewInt(type->dim[0]->range.min);
		max1 = VarNewInt(type->dim[0]->range.max);
		dst_arr = arr;
	}

	idx = VarNewTmp(0, idx1_type);

	src_type = init->type;
	
	// This is copy instruction (source is array)
	if (src_type->variant == TYPE_ARRAY) {		
		src_min = VarNewInt(src_type->dim[0]->range.min);
		src_max = VarNewInt(src_type->dim[0]->range.max + 1);
		src_idx = VarNewTmp(0, src_type->dim[0]);
		init = VarNewElement(init, src_idx);
		label_done = VarNewTmpLabel();
	}

	if (max1->mode == MODE_CONST) {
		stop_n = max1->n;
	
		nmask = ByteMask(stop_n);
		if (nmask == stop_n) {
			stop_n = 0;
		} else {
			stop_n++;
		}
		stop = VarNewInt(stop_n);
	} else {
		stop = max1;
	}

	GenLet(idx, min1);
	if (src_idx != NULL) {
		GenLet(src_idx, src_min);
	}
	GenLabel(label);
	GenLet(VarNewElement(dst_arr, idx), init);
	if (src_idx != NULL) {
		Gen(INSTR_ADD, src_idx, src_idx, VarNewInt(1));
		Gen(INSTR_IFEQ, label_done, src_idx, src_max);
	}
	Gen(INSTR_ADD, idx, idx, VarNewInt(1));
	Gen(INSTR_IFNE, label, idx, stop);
	if (src_idx != NULL) {
		GenLabel(label_done);
	}
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
			if (RuleMatch(rule, i)) {
				break;
			}
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
	Var * a = NULL, * var, * item;
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

		for(blk = proc->instr; blk != NULL; blk = blk->next) {

			ln = 1;
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

					// Array assignment default
					if (i->op == INSTR_LET) {
						var = i->result;
						item = i->arg1;
						if (var->type->variant == TYPE_ARRAY || var->mode == MODE_ELEMENT && var->var->mode == MODE_RANGE || (var->mode == MODE_ELEMENT && item->type->variant == TYPE_ARRAY) ) {
							GenArrayInit(var, item);
							goto next;
						}
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

		if (Verbose(proc)) {
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
		} else if (var->mode == MODE_BYTE) {
			PrintVarVal(var->adr);
			Print("$");
			PrintVarVal(var->var);

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

		if (var->type->variant == TYPE_LABEL) {
			if (var->instr != NULL) {
				Print(" (#");
				PrintInt(var->instr->seq_no);
				Print(")");
			}
		}

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

void PrintVarNameUser(Var * var)
{
	Print(var->name);
	if (var->idx > 0) {
		PrintInt(var->idx-1);
	}
}

void PrintVarUser(Var * var)
{
	if (var->mode == MODE_ELEMENT) {
		PrintVarNameUser(var->adr); Print("("); PrintVarUser(var->var); Print(")");
	} else if (var->mode == MODE_BYTE) {
		PrintVarNameUser(var->adr);
		Print("$");
		PrintVarUser(var->var);
	} else {
		PrintVarNameUser(var);
	}
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
		Print("(");
		PrintVar(var->var);
		Print(")");
	} else if (var->mode == MODE_BYTE) {
		PrintVarName(var->adr);
		Print("$");
		PrintVar(var->var);
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
		Print("@");
	} else {
		inop = InstrFindCode(i->op);
		printf("   %s", inop->name);
	
		if (i->result != NULL) {
			Print(" ");
			PrintVarVal(i->result);
			r = true;
		}

		if (i->arg1 != NULL) {
			if (r) {
				Print(", ");
			} else {
				Print(" ");
			}

			PrintVarVal(i->arg1);
		}

		if (i->arg2 != NULL) {
			Print(", ");
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
	UInt32 n;
	while (blk != NULL) {
		n = 1;
		printf("#%ld/  ", blk->seq_no);
		if (blk->label != NULL) {
			printf("    ");
			PrintVarVal(blk->label);
			printf("@");
		}
		printf("\n");
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
	Type * type;

	type = TypeAlloc(TYPE_PROC);

	ROOT_PROC_TYPE.variant = TYPE_PROC;

	memset(&ROOT_PROC, 0, sizeof(ROOT_PROC));
	ROOT_PROC.name = "root";
	ROOT_PROC.idx  = 0;
	ROOT_PROC.type = &ROOT_PROC_TYPE;
	ROOT_PROC.instr = NULL;

	InScope(&ROOT_PROC);

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

