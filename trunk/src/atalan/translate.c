/*

Translator

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

Translate compiler instructions to processor instructions by applying rewriting rules.

*/

#include "language.h"

#define MAX_RULE_UNROLL 20

GLOBAL Var * RULE_PROC;

GLOBAL Bool RULE_MATCH_BREAK;

GLOBAL Var * MACRO_ARG_VAR[MACRO_ARG_CNT];
GLOBAL Var * MACRO_ARG[MACRO_ARG_CNT];

/*
Translation is done using rules. 
*/

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

Var * VarMacroArg(UInt8 i)
{
	return MACRO_ARG_VAR[i];
}

void EmptyRuleArgs()
{
	UInt8 n;
	for(n=0; n<MACRO_ARG_CNT; n++) {
		MACRO_ARG[n] = NULL;
	}
}

static Bool ArgMatch(RuleArg * pattern, Var * arg, Bool in_tuple);

Bool RuleMatch(Rule * rule, Instr * i)
{
	Bool match;

	if (i->op != rule->op) return false;

	EmptyRuleArgs();

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
//		return ArgMatch(pattern->arr, arg->adr, false); 
		pattern = pattern->arr;
		arg = arg->adr;
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

Rule * InstrRule(Instr * instr)
/*
Purpose:
	Find rule that emits code for this instruction.
	May be used to test, whether specified instruction may be emitted or not.
*/
{
	Rule * rule;
	
	rule = EMIT_RULES[instr->op];
	if (instr->op == INSTR_LINE) return rule;

	for(; rule != NULL; rule = rule->next) {
		if (RuleMatch(rule, instr)) break;
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


Bool InstrTranslate(Instr * i, Bool * p_modified)
{
	Var rule_proc;
	Rule * rule;

	if (i->op == INSTR_LINE) {
		Gen(INSTR_LINE, i->result, i->arg1, i->arg2);
	} else if (InstrRule(i)) {
		Gen(i->op, i->result, i->arg1, i->arg2);
	} else {
		// Find translating rule
		for(rule = RULES[i->op]; rule != NULL; rule = rule->next) {
			if (RuleMatch(rule, i)) {
				break;
			}
		}

		if (rule != NULL) {
			rule_proc.instr = rule->to;
			GenMacro(&rule_proc, MACRO_ARG);
			*p_modified = true;
		} else {
			return false;
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
	InstrBlock * blk;
	Bool modified, untranslated;
	UInt8 step = 0;
	UInt32 ln;
	Var * a = NULL, * var, * item;
	Instr i2;

//	printf("============ Registers1 ============\n");
//	PrintProc(proc);

	// As first step, we translate all variables on register address to actual registers

	for(blk = proc->instr; blk != NULL; blk = blk->next) {
		for(i = blk->first; i != NULL; i = i->next) {
			if (i->op == INSTR_LINE) continue;
			i->result = VarReg(i->result);
			i->arg1   = VarReg(i->arg1);
			i->arg2   = VarReg(i->arg2);
		}
	}

//	printf("============ Registers ============\n");
//	PrintProc(proc);

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
			GenSetDestination(blk, NULL);
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
					// Try opposite branch if this one is not implemented
					} else if (IS_INSTR_BRANCH(i->op)) {
						i2.op = OpRelSwap(i->op); i2.result = i->result; i2.arg1 = i->arg2; i2.arg2 = i->arg1;
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

void TranslateInit()
{
	UInt16 op;
	Var * var;
	UInt8 i;
	Type * type;

	type = TypeAlloc(TYPE_PROC);
	RULE_PROC = VarAlloc(MODE_VAR, NULL, 0);
	RULE_PROC->type = type;

	for(i=0; i<MACRO_ARG_CNT; i++) {
		var = VarAllocScope(RULE_PROC, MODE_ARG, NULL, i+1);
		MACRO_ARG_VAR[i] = var;
	}

	for(op=0; op<INSTR_CNT; op++) {
		RULES[op] = NULL;
		LAST_RULE[op] = NULL;
		EMIT_RULES[op] = NULL;
		LAST_EMIT_RULE[op] = NULL;
	}

}
