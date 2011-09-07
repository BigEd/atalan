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

typedef enum {
	PHASE_TRANSLATE,
	PHASE_EMIT
} Phase;

GLOBAL Phase MATCH_MODE;		// mode used to match rules (PHASE_TRANSLATE, PHASE_EMIT)

/*
Translation is done using rules. 
*/

GLOBAL Rule * RULES[INSTR_CNT];
GLOBAL Rule * LAST_RULE[INSTR_CNT];

GLOBAL Rule * EMIT_RULES[INSTR_CNT];
GLOBAL Rule * LAST_EMIT_RULE[INSTR_CNT];

Bool RuleArgIsMoreSpecific(RuleArg * l, RuleArg * r)
/*
	Return true if l is more specific than r.
*/
{
	if (l == r) return false;
	if (l == NULL) return false;
	if (r == NULL) return true;

	if (l->variant == RULE_REGISTER) {
		if (r->variant != RULE_REGISTER) return true;
	}

	if (l->variant == RULE_CONST) {
		if (r->variant != RULE_CONST) return true;
	}

	// Accessing variable using byte or element is more specific than other variants
	if (l->variant == RULE_BYTE || l->variant == RULE_ELEMENT) {
		if (r->variant != RULE_BYTE && l->variant != RULE_ELEMENT) return true;

	}
	return false;
}

Bool RuleIsMoreSpecific(Rule * l, Rule * r)
/*
Purpose:
	Compare two rule filters for specificity.
	In case we are not able to decide, which rule is more specific, we return false.
*/
{
	UInt8 i;
	for(i=0; i<3; i++) {
		if (RuleArgIsMoreSpecific(&l->arg[i], &r->arg[i])) return true;
	}
	return false;
}

void RuleRegister(Rule * rule)
/*
Purpose:
	Register parsed rule.
*/
{
	InstrOp op = rule->op;
	Rule * r, * prev_r;
	if (!rule->to->first) InternalError("Empty rule");

	if (rule->to->first->op == INSTR_EMIT) {
		if (LAST_EMIT_RULE[op] != NULL) LAST_EMIT_RULE[op]->next = rule;
		if (EMIT_RULES[op] == NULL) EMIT_RULES[op] = rule;
		LAST_EMIT_RULE[op] = rule;
	} else {

		prev_r = NULL; r = RULES[op];

		while(r != NULL && !RuleIsMoreSpecific(rule, r)) {
			prev_r = r;
			r = r->next;
		}

		rule->next = r;

		if (prev_r == NULL) {
			RULES[op] = rule;
		} else {
			prev_r->next = rule;
		}

//		if (LAST_RULE[op] != NULL) LAST_RULE[op]->next = rule;
//		if (RULES[op] == NULL) RULES[op] = rule;
//		LAST_RULE[op] = rule;
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

static Bool ArgMatch(RuleArg * pattern, Var * arg, RuleArgVariant parent_variant);

Bool RuleMatch(Rule * rule, Instr * i, Phase match_mode)
/*
Purpose:
	Return true, if the instruction matches specified rule.
*/
{
	Bool match;

	if (i->op != rule->op) return false;

	EmptyRuleArgs();
	MATCH_MODE = match_mode;

	match = ArgMatch(&rule->arg[0], i->result, RULE_UNDEFINED) 
		&& ArgMatch(&rule->arg[1], i->arg1, RULE_UNDEFINED) 
		&& ArgMatch(&rule->arg[2], i->arg2, RULE_UNDEFINED);

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
		if (var->mode == INSTR_ELEMENT) {
			if (VarIsStructElement(var)) {
				// This is reference to structure
				// We may treat it as normal variable
//				printf("");
			} else {
				// 1D index
				if (!ArgMatch(pattern->index, var->var, RULE_ELEMENT)) return false;
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

Bool VarIsFlagReg(Var * var)
{
	return VarIsReg(var) && var->type->range.max == 1 && var->type->range.min == 0;
}

static Bool ArgMatch(RuleArg * pattern, Var * arg, RuleArgVariant parent_variant)
{
	Type * atype;
	Var * pvar;
	UInt8 j;
	RuleArgVariant v = pattern->variant;

	if (arg == NULL) return v == RULE_ANY;
	atype = arg->type;
	
	switch(v) {

	// <X>..<Y>
	case RULE_RANGE:
		if (arg->mode != INSTR_RANGE) return false;		// pattern expects element, and variable is not an element
		if (!ArgMatch(pattern->index, arg->var, v)) return false;
		return ArgMatch(pattern->arr, arg->adr, v); 
		break;

	// In case of destination register, we ignore all flag registers, that are specified in pattern and not specified in argument.
	// I.e. instruction may affect more flags, than the source instruction requires.
	// All flags defined by source instruction (argument) must be part of pattern though.

	// <X>,<Y>
	case RULE_TUPLE:
		if (arg->mode != INSTR_TUPLE) return false;
		if (!ArgMatch(pattern->index, arg->var, v)) return false;
		return ArgMatch(pattern->arr, arg->adr, v); 
		break;

	// <X>$<Y>
	case RULE_BYTE:
		if (arg->mode != INSTR_BYTE) return false;		// pattern expects byte, and variable is not an byte
		if (!ArgMatch(pattern->index, arg->var, v)) return false;		
		return ArgMatch(pattern->arr, arg->adr, v);
		break;

	// <X>(<Y>)
	case RULE_ELEMENT:
		if (arg->mode != INSTR_ELEMENT) return false;		// pattern expects element, and variable is not an element
		if (!ArgMatch(pattern->index, arg->var, v)) return false;
		return ArgMatch(pattern->arr, arg->adr, v); 
		break;

	// const %A:type
	case RULE_CONST:
		if (!VarIsConst(arg)) return false;
		if (!VarMatchesPattern(arg, pattern)) return false;
		break;

	// 1
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

	// Exact variable.
	case RULE_REGISTER:
		if (pattern->var != NULL && !VarIsEqual(arg, pattern->var)) return false;
		break;

	case RULE_VARIABLE:
		if (arg->mode != INSTR_VAR) return false;
		if (FlagOn(arg->submode, SUBMODE_REG)) return false;
		if (parent_variant != RULE_BYTE && !VarMatchesPattern(arg, pattern)) return false;
		break;
	
	case RULE_DEREF:
		if (arg->mode != INSTR_DEREF) return false;
		arg = arg->var;
		if (!VarMatchesPattern(arg, pattern)) return false;
		break;

	// %A:type
	case RULE_ARG:
		// In Emit phase, we need to exactly differentiate between single variable and byte offset.
		if (arg->mode == INSTR_DEREF || arg->mode == INSTR_RANGE || (arg->mode == INSTR_BYTE && MATCH_MODE == PHASE_EMIT) || arg->mode == INSTR_ELEMENT) return false;
		if (parent_variant != RULE_TUPLE && FlagOn(arg->submode, SUBMODE_REG)) return false; 
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
//		if (arg->mode == INSTR_ELEMENT && !VarIsStructElement(arg)) {
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
		if (RuleMatch(rule, instr, PHASE_EMIT)) break;
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
	} else if ((rule = InstrRule(i))) {
		GenRule(rule, i->result, i->arg1, i->arg2);
	} else {
		// Find translating rule
		for(rule = RULES[i->op]; rule != NULL; rule = rule->next) {
			if (RuleMatch(rule, i, PHASE_TRANSLATE)) {
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
	Bool modified, untranslated, in_assert;
	UInt8 step = 0;
	UInt32 n;
	Var * a = NULL, * var, * item;
	Instr i2;

//	printf("============ Registers1 ============\n");
//	PrintProc(proc);

//	if (StrEqual(proc->name, "cycle")) {
//		printf("");
//	}

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
		in_assert = false;

		for(blk = proc->instr; blk != NULL; blk = blk->next) {

			n = 1;
			// The translation is done by using procedures for code generating.
			// We detach the instruction list from block and set the block as destination for instruction generator.
			// In this moment, the code generating stack must be empty anyways.

			first_i = blk->first;
			blk->first = blk->last = NULL;
			GenSetDestination(blk, NULL);
			i = first_i;

			while(i != NULL) {

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

				if (!InstrTranslate(i, &modified)) {

					// If this is commutative instruction, try the other order of rules
					if (FlagOn(INSTR_INFO[i->op].flags, INSTR_COMMUTATIVE) /*.i->op == INSTR_AND || i->op == INSTR_OR || i->op == INSTR_XOR || i->op == INSTR_ADD || i->op == INSTR_MUL*/) {
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
						if (var->type->variant == TYPE_ARRAY || var->mode == INSTR_ELEMENT && var->var->mode == INSTR_RANGE || (var->mode == INSTR_ELEMENT && item->type->variant == TYPE_ARRAY) ) {
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
						if (i->op != INSTR_LET || i->arg1->mode != INSTR_VAR) {
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
				n++;
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
	RULE_PROC = VarAlloc(INSTR_VAR, NULL, 0);
	RULE_PROC->type = type;

	for(i=0; i<MACRO_ARG_CNT; i++) {
		var = VarAllocScope(RULE_PROC, INSTR_VAR, NULL, i+1);
		var->submode = SUBMODE_ARG_IN;
		MACRO_ARG_VAR[i] = var;
	}

	for(op=0; op<INSTR_CNT; op++) {
		RULES[op] = NULL;
		LAST_RULE[op] = NULL;
		EMIT_RULES[op] = NULL;
		LAST_EMIT_RULE[op] = NULL;
	}

}
