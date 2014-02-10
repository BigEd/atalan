/*

Translator

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

Translate compiler instructions to processor instructions by applying rewriting rules.
Translator takes instruction after instruction in given procedure and tries to find suitable translation using
rules defined for current CPU.
*/

#include "language.h"

GLOBAL Var * RULE_FN_TYPE;						// This procedure is used to represent macro used when translating rule
GLOBAL Bool INSTR_MATCH_BREAK;

GLOBAL Var * MACRO_ARG_VAR[MACRO_ARG_CNT];		// Set of variables representing macro arguments
GLOBAL Var * MACRO_ARG[MACRO_ARG_CNT];

GLOBAL CompilerPhase MATCH_MODE;		// mode used to match rules (PHASE_TRANSLATE, PHASE_EMIT)

GLOBAL Bool VERBOSE_NOW;

/*
Translation is done using rules. 
There are two types of rules:

Instruction rules are directly translatable to processor instructions.
Normal rules translate compiler instruction to zero or more compiler instructions.

*/

GLOBAL RuleSet TRANSLATE_RULES;
GLOBAL RuleSet INSTR_RULES;

/*
================
Rule specificity
================

The rules are sorted by their specificity.
The most specific rule is the first in the list, less specific rules follow.
This ensures, that the more specific rules have bigger priority than less specific ones and will be preffered when translating.

Rule is more specific if it's arguments are more specific.

1. using actual CPU register or actual variable
2. using constant
3. using array or byte access
4. other possibilities

*/

Bool RuleArgIsRegister(RuleArg * l)
{
	if (l->variant == INSTR_VAR || l->variant == INSTR_VARIANT) return true;
	return false;
}

Bool RuleArgIsMoreSpecific(RuleArg * l, RuleArg * r)
/*
	Compare two rule arguments for specificity.
	Return true if l is more specific than r.
	Return false, if it is less specific or we are not able to decide which of the rules is more specific.
*/
{
	Bool r_is_reg;

	if (l == r) return false;
	if (l == NULL) return false;
	if (r == NULL) return true;

	r_is_reg = RuleArgIsRegister(r);
	if (RuleArgIsRegister(l)) {
		if (!r_is_reg) return true;
	}

	if (r_is_reg) return false;

	if (l->variant == INSTR_MATCH_VAL) {
		if (r->variant != INSTR_MATCH_VAL) return true;
	}

	// Accessing variable using byte or element is more specific than other variants
	if (l->variant == INSTR_BYTE || l->variant == INSTR_ELEMENT) {
		if (r->variant != INSTR_BYTE && l->variant != INSTR_ELEMENT) return true;

	}
	return false;
}

Bool RuleIsMoreSpecific(Rule * l, Rule * r)
/*
Purpose:
	Compare two rule filters for specificity.
	Return true if l is more specific than r.
	Return false, if it is less specific or we are not able to decide which of the rules is more specific.
*/
{
	UInt8 i;
	for(i=0; i<3; i++) {
		if (RuleArgIsMoreSpecific(l->arg[i], r->arg[i])) return true;
		if (RuleArgIsMoreSpecific(r->arg[i], l->arg[i])) return false;
	}
	return false;
}

void RuleSetInit(RuleSet * ruleset)
{
	UInt16 op;
	for(op=0; op<INSTR_CNT; op++) {
		ruleset->rules[op] = NULL;
	}
}

void RuleSetAddRule(RuleSet * ruleset, Rule * rule)
/*
Purpose:
	Add the rule to the rule set.
	The rules in the ruleset are sorted by specificity (the most specific rule is at the start of the list).
	The rules are hashed by operation.
*/
{
	InstrOp op = rule->op;
	Rule * prev_r, * r;

	if (!rule->to->first) {
		InternalError("Empty rule");
		return;
	}

	prev_r = NULL; r = ruleset->rules[op];

	while(r != NULL && !RuleIsMoreSpecific(rule, r)) {
		prev_r = r;
		r = r->next;
	}

	rule->next = r;

	if (prev_r == NULL) {
		ruleset->rules[op] = rule;
	} else {
		prev_r->next = rule;
	}

}

void RuleRegister(Rule * rule)
/*
Purpose:
	Register parsed rule.
	Rule is stored either in emit or normal rules set.
*/
{
	if (rule->to == NULL || rule->to->first == NULL) {
		SyntaxError("Empty rule");
	} else {

		if (rule->to->first->op == INSTR_EMIT) {
			RuleSetAddRule(&INSTR_RULES, rule);
		} else {
			RuleSetAddRule(&TRANSLATE_RULES, rule);
		}
	}
}

/***********************************************

  Rules garbage collector

************************************************/

void RuleArgMarkNonGarbage(RuleArg * rule)
{
	if (rule != NULL) {
		if (rule->variant == INSTR_MATCH || rule->variant == INSTR_MATCH_VAL) {
			TypeMark(rule->type);
		} else if (rule->variant == INSTR_TUPLE || rule->variant == INSTR_DEREF) {
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
			RuleArgMarkNonGarbage(rule->arg[i]);
		}
		rule = rule->next;
	}
}

void RuleSetGarbageCollect(RuleSet * ruleset)
{
	UInt8 op;
	for(op=0; op<INSTR_CNT; op++) {
		RulesMarkNonGarbage(ruleset->rules[op]);
	}
}

void RulesGarbageCollect()
{
	RuleSetGarbageCollect(&TRANSLATE_RULES);
	RuleSetGarbageCollect(&INSTR_RULES);
}

Var * VarRuleArg(UInt8 n)
/*
Purpose:
	Return n-th rule argument.
*/
{
	return MACRO_ARG_VAR[n];
}

Bool VarIsRuleArg(Var * var)
/*
Purpose:
	Test, if the variable is rule argument.
*/
{
	return var->scope == RULE_FN_TYPE;
}

void EmptyRuleArgs()
{
	UInt8 n;
	for(n=0; n<MACRO_ARG_CNT; n++) {
		MACRO_ARG[n] = NULL;
	}
}

static Bool ArgMatch(RuleArg * pattern, Var * arg, InstrOp parent_variant);

Bool RuleMatch(Rule * rule, Instr * i, CompilerPhase match_mode)
/*
Purpose:
	Return true, if the instruction matches specified rule.
*/
{
	Bool match;

	if (i->op != rule->op) return false;

	EmptyRuleArgs();
	MATCH_MODE = match_mode;

	match = ArgMatch(rule->arg[0], i->result, INSTR_NULL) 
		&& ArgMatch(rule->arg[1], i->arg1, INSTR_NULL) 
		&& ArgMatch(rule->arg[2], i->arg2, INSTR_NULL);

	if (match) {
		if (INSTR_MATCH_BREAK) {
			INSTR_MATCH_BREAK = true;
		}
	}
	return match;
}

Bool VarMatchesPattern(Var * var, RuleArg * pattern)
{
	Type * type = pattern->type;
	Type * vtype = var->type;
	Var * adr;	

	// Pattern expects reference to array with one or more indices
	if (pattern->index != NULL) {
		if (var->mode == INSTR_ELEMENT) {
			if (VarIsStructElement(var)) {
				// This is reference to structure
				// We may treat it as normal variable
//				Print("");
			} else {
				// 1D index
				if (!ArgMatch(pattern->index, var->var, INSTR_ELEMENT)) return false;
				return true;
			}
		} else {
			return false;
		}

	// Pattern does not expect any index, but we have some, so there is no match
	} else {
		adr = var;
		if (var->mode == INSTR_VAR && VarIsAlias(var)) {
			adr = var->adr;
		}
		if (VarIsArrayElement(adr)) return false;
	}
	if (type == NULL) return true;
	return IsSubset(var, type);
}

Bool VarIsOneOf(Var * var, Var * variants)
{
	Var * o;

	o = variants;
	if (variants->mode != INSTR_TUPLE) {
		if (variants->adr != NULL) o = variants->adr;
	}

	while(o->mode == INSTR_TUPLE) {
		if (IsEqual(var, o->adr)) return true;
		o = o->var;
	}
	return IsEqual(var, o);
}

static Bool ArgMatch(RuleArg * pattern, Var * arg, InstrOp parent_variant)
{
	Type * atype;
	Var * pvar, * left, * right;
	UInt8 arg_no;
	InstrOp v;
	Var * var;

	if (pattern == NULL) return arg == NULL;

	v = pattern->variant;
	if (arg == NULL) return v == INSTR_NULL;

	var = arg;
	if (arg->mode == INSTR_VAR) {
		if (VarIsAlias(arg)) var = arg->adr;
	}

	atype = arg->type;

	// If this is rule argument and it has been previously set, it must match exactly

	arg_no = pattern->arg_no;
	if (arg_no != 0 && MACRO_ARG[arg_no-1] != NULL) {
		return MACRO_ARG[arg_no-1] == arg;	
	}

	switch(v) {

	case INSTR_ADD:
	case INSTR_SUB:

		if (arg->mode == INSTR_VAR || arg->mode == INSTR_INT) {
			// If we have variable and not arithmetic operation, try to match using subtraction or addition with 0
			// %A => %A-0 or %A+0
			// We do not allow this in emit mode, as it would convert comparison instructions implemented using subtraction.

			if (MATCH_MODE == PHASE_EMIT) return false;
			left  = arg;
			right = ZERO;
		} else {
			if (v != arg->mode) return false;
			left = arg->l;
			right = arg->r;
		}
		if (!ArgMatch(pattern->index, right, v)) return false;
		return ArgMatch(pattern->arr, left, v); 
		break;

	// <X>..<Y>
	case INSTR_RANGE:
		if (arg->mode != INSTR_RANGE) return false;		// pattern expects element, and variable is not an element
		if (!ArgMatch(pattern->index, arg->var, v)) return false;
		return ArgMatch(pattern->arr, arg->adr, v); 
		break;

	// In case of destination register, we ignore all flag registers, that are specified in pattern and not specified in argument.
	// I.e. instruction may affect more flags, than the source instruction requires.
	// All flags defined by source instruction (argument) must be part of pattern though.

	// <X>,<Y>
	case INSTR_TUPLE:
		if (arg->mode != INSTR_TUPLE) return false;
		if (!ArgMatch(pattern->index, arg->var, v)) return false;
		return ArgMatch(pattern->arr, arg->adr, v); 
		break;

	// <X>$<Y>
	case INSTR_BYTE:
		if (arg->mode != INSTR_BYTE) return false;		// pattern expects byte, and variable is not an byte
		if (!ArgMatch(pattern->index, arg->var, v)) return false;		
		return ArgMatch(pattern->arr, arg->adr, v);
		break;

	// <X>#<Y>
	case INSTR_ELEMENT:
		
		if (var->mode != INSTR_ELEMENT) return false;		// pattern expects element, and variable is not an element
		if (!ArgMatch(pattern->index, var->var, v)) return false;
		return ArgMatch(pattern->arr, var->adr, v);
		break;

	// const %A:type
	case INSTR_MATCH_VAL:
		if (!CellIsConst(arg)) return false;
		if (!VarMatchesPattern(arg, pattern)) return false;
		break;

	// Exact variable or constant.
	case INSTR_VAR:
		pvar = pattern->var;
		if (CellIsConst(pvar)) {
			if (!CellIsConst(arg)) return false;
			if (pvar->mode == INSTR_TEXT) {
				if (!StrEqual(pvar->str, arg->str)) return false;
			} else if (pvar->mode == INSTR_INT) {
				if (arg->mode != INSTR_INT || !IntEq(&pvar->n, &arg->n)) return false;
			}
		} else {
			if (pattern->var != NULL && !IsEqual(arg, pattern->var)) return false;
		}
		break;

	case INSTR_VARIANT:
		if (!VarIsOneOf(arg, pattern->var)) return false;
		break;

	case INSTR_MATCH:
//		if (arg->mode != INSTR_VAR) return false;
		if (FlagOn(arg->submode, SUBMODE_REG)) return false;
		if (parent_variant != INSTR_BYTE && !VarMatchesPattern(arg, pattern)) return false;
		break;
	
	// @%A
	case INSTR_DEREF:
		if (arg->mode != INSTR_DEREF) return false;
		arg = arg->var;
		if (!ArgMatch(pattern->arr, arg, v)) return false;
		break;

	case INSTR_NULL:
		break;

	default:
		if (pattern->variant != arg->mode) return false;
		if (!ArgMatch(pattern->index, arg->r, v)) return false;
		return ArgMatch(pattern->arr, arg->l, v); 
		break;
	}

	// If this is rule argument, we set it.

	if (arg_no != 0) {
		ASSERT(MACRO_ARG[arg_no-1] == NULL);
		MACRO_ARG[arg_no-1] = arg;
	}

	return true;
}

Bool InstrRuleIsDefined(InstrOp op)
{
	return INSTR_RULES.rules[op] != NULL;

}

Rule * RuleSetFindRule(RuleSet * ruleset, InstrOp op, Var * result, Var * arg1, Var * arg2)
/*
Purpose:
	Find rule that emits code for this instruction.
	May be used to test, whether specified instruction may be emitted or not.
*/
{
	Instr i;
	Rule * rule;
	
	rule = ruleset->rules[op];
	if (op == INSTR_LINE) return rule;

	i.op = op; i.result = result; i.arg1 = arg1; i.arg2 = arg2;
	for(; rule != NULL; rule = rule->next) {
		if (RuleMatch(rule, &i, PHASE_TRANSLATE)) break;
	}
	return rule;
}


Rule * InstrRule(Instr * instr)
/*
Purpose:
	Find rule that emits code for this instruction.
	May be used to test, whether specified instruction may be emitted or not.
*/
{
	Rule * rule;
	
	rule = INSTR_RULES.rules[instr->op];
	if (instr->op == INSTR_LINE) return rule;

	for(; rule != NULL; rule = rule->next) {
		if (RuleMatch(rule, instr, PHASE_EMIT)) {		
			break;
		}
	}

	return rule;
}

Rule * InstrRule2(InstrOp op, Var * result, Var * arg1, Var * arg2)
{
	Instr i;
	i.op = op;
	i.result = result;
	i.arg1 = arg1;
	i.arg2 = arg2;
	return InstrRule(&i);
}

Rule * TranslateRule(InstrOp op, Var * result, Var * arg1, Var * arg2)
{
	Instr i;
	Rule * rule;
	i.op = op;
	i.result = result;
	i.arg1 = arg1;
	i.arg2 = arg2;
	for(rule = TRANSLATE_RULES.rules[op]; rule != NULL; rule = rule->next) {
		if (RuleMatch(rule, &i, PHASE_TRANSLATE)) {
			return rule;
		}
	}
	return NULL;
}

void GenMatchedRule(Rule * rule)
{
	Var rule_proc;
	Var rule_type;
	Var * args[MACRO_ARG_CNT];

	rule_type.mode = INSTR_FN;	
	rule_type.submode = SUBMODE_MACRO;
	rule_type.type = VOID;
	rule_type.instr = rule->to;

	rule_proc.mode = INSTR_VAR;
	rule_proc.name = NULL;
	rule_proc.idx = 0;
	rule_proc.type = &rule_type;

	MemMove(args, MACRO_ARG, sizeof(args));
	GenMacro(&rule_proc, args);
}

Bool InstrTranslate(InstrOp op, Var * result, Var * arg1, Var * arg2, UInt8 mode)
{
	Rule * rule;

	if (FlagOn(mode, TEST_ONLY)) {
		if (op == INSTR_LINE) return true;
		if (InstrRule2(op, result, arg1, arg2) != NULL) return true;
		rule = TranslateRule(op, result, arg1, arg2);
		return rule != NULL;
	} else {

		if (op == INSTR_LINE) {
			GenInternal(INSTR_LINE, result, arg1, arg2);
		} else if ((rule = InstrRule2(op, result, arg1, arg2))) {
			if (VERBOSE_NOW) {
				Print("     "); EmitInstrOp(op, result, arg1, arg2);
			}
			GenRule(rule, result, arg1, arg2);
		} else if ((rule = TranslateRule(op, result, arg1, arg2))) {
			GenMatchedRule(rule);
		} else {
			return false;
		}
		return true;
	}
}

Bool InstrTranslate2(InstrOp op, Var * result, Var * arg1, Var * arg2, UInt8 mode)
{
	if (InstrTranslate(op, result, arg1, arg2, mode)) return true;

	// If this is commutative instruction, try the other order of rules

	if (FlagOn(INSTR_INFO[op].flags, INSTR_COMMUTATIVE)) {
		if (InstrTranslate(op, result, arg2, arg1, mode)) return true;

	// Try opposite branch if this one is not implemented
	} //else if (IS_INSTR_BRANCH(op)) {
//		if (InstrTranslate(OpRelSwap(op), result, arg2, arg1, mode)) return true;
//	}

	return false;
}

void VarInitType(Var * var, Type * type)
{
	if (type == NULL) type = ANY;
	MemEmpty(var, sizeof(Var));
	var->mode = INSTR_VAR;
	var->type = type;
}

Type * TypeBiggerType(InstrOp op, Type * type)
/*
Purpose:
	Find bigger type supported by compiler.
*/
{
	Rule * rule;
	Type * result_type;	
	Type * found_type = NULL;
	rule = TRANSLATE_RULES.rules[op];

	for(; rule != NULL; rule = rule->next) {
		if (rule->arg[0]->variant == INSTR_MATCH) {
			result_type = rule->arg[0]->type;
			if (IsSubset(type, result_type) && !IsSubset(result_type, type)) {
				if (found_type == NULL || IsSubset(result_type, found_type)) found_type = result_type;
			}
		}
	}
	return found_type;
}

Bool InstrTranslate3(InstrOp op, Var * result, Var * arg1, Var * arg2, UInt8 mode)
{
	Var * a, * a_l, * a_r;
	Var  tmp1,  tmp2,  tmp_r;
	Bool has1, has2, has_r;
	Type * result_type = NULL;
	Type * type;

	if (InstrTranslate2(op, result, arg1, arg2, mode)) return true;


	// It is not possible to translate the instruction directly.
	// We will try to simplify the translated instruction by taking one of it's complex arguments
	// and replacing it by simple variable.
	// The variable will be assigned the value using LET instruction before it gets used in the instruction.
	// We are thus replacing one instruction by set of two or more instructions.

	has1 = has2 = has_r = false;

	if (CellIsOp(arg1)) {

		a_l = arg1->l;
		a_r = arg1->r;

		// Let r = (x * 4) - y
		if (CellIsOp(a_l)) {
			//TODO: Infer the type.
			type = ANY; if (result != NULL) type = result->type;
			if (type == ANY) if (a_l->l->mode == INSTR_VAR) type = a_l->l->type;
			if (type == ANY) if (a_l->r->mode == INSTR_VAR) type = a_l->r->type;
			VarInitType(&tmp1, type);
			if (InstrTranslate3(INSTR_LET, &tmp1, a_l, NULL, mode | TEST_ONLY)) {
				has1 = true;
			}
		} else if (a_l->mode == INSTR_ELEMENT) {
			type = ItemType(a_l->l->type);
			VarInitType(&tmp1, type);
			if (InstrTranslate3(INSTR_LET, &tmp1, a_l, NULL, mode | TEST_ONLY)) {
				has1 = true;
			}

		}

		// Let r = x - (y * 2)

		if (CellIsOp(a_r)) {
			type = ANY; if (result != NULL) type = result->type;
			if (type == ANY) if (a_r->l->mode == INSTR_VAR) type = a_r->l->type;
			if (type == ANY) if (a_r->r->mode == INSTR_VAR) type = a_r->r->type;
			VarInitType(&tmp2, type);
			if (InstrTranslate3(INSTR_LET, &tmp2, a_r, NULL, mode | TEST_ONLY)) {
				has2 = true;
			}
		}

		if (mode == GENERATE) {
			if (has1) {
				a_l = NewTempVar(tmp1.type);
				InstrTranslate3(INSTR_LET, a_l, arg1->l, NULL, GENERATE);
			}

			if (has2) {
				a_r = NewTempVar(tmp2.type);
				InstrTranslate3(INSTR_LET, a_r, arg1->r, NULL, GENERATE);
			}

			if (has1 || has2) {
				arg1 = NewOp(arg1->mode, a_l, a_r);
				goto found_translation2;
			}
		}
	}

	// First we perform simple test detecting, if one of the arguments is more complex, then what we will
	// use for replacing it (simple variable).

	has_r = (result != NULL && result->mode != INSTR_VAR);
	has1 = (arg1 != NULL && arg1->mode != INSTR_VAR);
	has2 = (arg2 != NULL && arg2->mode != INSTR_VAR);

	if (has_r) { VarInitType(&tmp_r, result->type); }
	if (has1)  { VarInitType(&tmp1, arg1->type); }
	if (has2)  { VarInitType(&tmp2, arg2->type); }

	if (result != NULL) {
		result_type = result->type;
	}

	if (has1) {
		// When the instruction is assignment, it may happen, that assigning the argument to temporary variable 
		// would generate basically the same instruction as is the translated instruction.
		// In that case, we do not try to test it again and instead consider the argument as non-extractable.
		// Translation would otherwise end in an infinite loop.
		if (op == INSTR_LET && !has_r && TypeIsEqual(result->type, tmp1.type)) {
			has1 = false;
		} else {
			has1 = InstrTranslate3(INSTR_LET, &tmp1, arg1, NULL, mode | TEST_ONLY);
		}
	}
	
	if (has2) {
		if (op == INSTR_LET && !has_r && TypeIsEqual(result->type, tmp2.type)) {
			has2 = false;
		} else {
			has2 = InstrTranslate3(INSTR_LET, &tmp2, arg2, NULL, mode | TEST_ONLY);
		}
	}

	if (has_r) {
		if (op == INSTR_LET && arg1->mode == INSTR_VAR && TypeIsEqual(arg1->type, tmp_r.type)) {
			has_r = false;
		} else {
			has_r = InstrTranslate3(INSTR_LET, result, &tmp_r, NULL, mode | TEST_ONLY);
		}
	}

	if (has1 && InstrTranslate2(op, result, &tmp1, arg2, mode | TEST_ONLY)) {
		has2 = has_r = false;
	} else if (has2 && InstrTranslate2(op, result, arg1, &tmp2, mode | TEST_ONLY)) {
		has1 = has_r = false;
	} else if (has_r && InstrTranslate2(op, &tmp_r, arg1, arg2, mode | TEST_ONLY)) {
		has1 = has2 = false;
	} else if (has1 && has2 && InstrTranslate2(op, result, &tmp1, &tmp2, mode | TEST_ONLY)) {
		has_r = false;
	} else if (has_r && has1 && InstrTranslate2(op, &tmp_r, &tmp1, arg2, mode | TEST_ONLY)) {
		has2 = false;		
	} else if (has_r && has2 && InstrTranslate2(op, &tmp_r, arg1, &tmp2, mode | TEST_ONLY)) {
		has1 = false;
	} else if (has_r && has1 && has2 && InstrTranslate2(op, &tmp_r,&tmp1, &tmp2, mode | TEST_ONLY)) {

	} else {

		// No translation of the instruction was found.
		// Try, if it is possible to use instruction returning result with bigger range and then cast that
		// bigger result to smaller variable.
		// The instruction must have a result, of course.

		if (result != NULL && FlagOff(mode, BIGGER_RESULT)) {
			if (result->mode == INSTR_VAR && result->type->variant == TYPE_INT) {
				result_type = result->type; 
				while((result_type = TypeBiggerType(op, result_type)) != NULL) {			
					VarInitType(&tmp_r, result_type);
					if (InstrTranslate3(INSTR_LET, result, &tmp_r, NULL, mode | TEST_ONLY)) {
						if (InstrTranslate3(op, &tmp_r, arg1, arg2, mode | BIGGER_RESULT | TEST_ONLY)) {
							has_r = true;
							goto found_translation;
						}
					}
				}
			}
		}
		return false;
	}
found_translation:
	if (mode == GENERATE) {
		if (has1) {
			a = NewTempVar(arg1->type);
			InstrTranslate3(INSTR_LET, a, arg1, NULL, GENERATE);
			arg1 = a;
		}
		if (has2) {
			a = NewTempVar(arg2->type);
			InstrTranslate3(INSTR_LET, a, arg2, NULL, GENERATE);
			arg2 = a;
		}
		if (has_r) {
			a = result;
			result = NewTempVar(result_type);
		}
found_translation2:
		InstrTranslate2(op, result, arg1, arg2, GENERATE);

		if (has_r) {
			InstrTranslate3(INSTR_LET, a, result, NULL, GENERATE);
		}
	}
	return true;
}

extern InstrBlock * BLK;

void ProcTranslate(Var * proc)
/*
Purpose:
	Translate generic instructions to instructions directly translatable to processor instructions.
*/
{
	Instr * i, * first_i, * next_i;
	InstrBlock * first_blk, * blk;
	UInt8 step = 0;
	Bool in_assert;
	UInt8 color;
	Loc loc;

	if (!IsFnImplemented(proc->type)) return;

	loc.proc = proc;
	VERBOSE_NOW = false;
	if (Verbose(proc)) {
		VERBOSE_NOW = true;
		PrintHeader(2, VarName(proc));
	}

	first_blk = FnVarInstr(proc);

	// As first step, we translate all variables stored on register addresses to actual registers

	for(blk = first_blk; blk != NULL; blk = blk->next) {
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

	in_assert = false;

	for(blk = first_blk; blk != NULL; blk = blk->next) {

		if (Verbose(proc)) {
			PrintBlockHeader(blk);
		}

		loc.blk = blk;
		loc.n = 1;

		first_i = blk->first;
		blk->first = blk->last = NULL;
		GenSetDestination(blk, NULL);
		i = first_i;

		while(i != NULL) {

			loc.i = i;
			if (ASSERTS_OFF) {
				if (i->op == INSTR_ASSERT_BEGIN) {
					in_assert = true;
					goto next;
				} else if (i->op == INSTR_ASSERT_END) {
					in_assert = false;
					goto next;
				} else {
					if (in_assert) goto next;
				}
			}

			if (VERBOSE_NOW) {
				color = PrintColor(RED+BLUE);
				PrintInstrLine(loc.n); InstrPrint(i);
				PrintColor(color);
			}

			if (!InstrTranslate3(i->op, i->result, i->arg1, i->arg2, GENERATE)) {
				if (FlagOff(INSTR_INFO[i->op].flags, INSTR_OPTIONAL)) {
					InternalErrorLoc("Unsupported instruction", &loc); 
					InstrPrint(i);
				}
			}
next:
			next_i = i->next;
			i = next_i;
			loc.n++;
		}

		// Free the instructions
		i = first_i;
		while (i!=NULL) {
			next_i = i->next;
			InstrFree(i);
			i = next_i;
		}
	
	} // block
}

void TranslateInit()
{
	Var * var;
	UInt8 i;
	Type * type;

	// Create RULE procedure and allocate it's arguments

	type = NewFnType(VOID, VOID);
	RULE_FN_TYPE = NewTempVar(type);

	for(i=0; i<MACRO_ARG_CNT; i++) {
		var = NewVarWithIndex(RULE_FN_TYPE, NULL, i+1, NULL);
		var->submode = SUBMODE_ARG_IN;
		MACRO_ARG_VAR[i] = var;
	}

	RuleSetInit(&TRANSLATE_RULES);
	RuleSetInit(&INSTR_RULES);
}
