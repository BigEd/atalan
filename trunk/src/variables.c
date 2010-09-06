/*
Table of variables

There is one global table of varibles.

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"


GLOBAL Var * VARS;		// global variables

// Last allocated variable.
GLOBAL Var * LAST_VAR;
GLOBAL Var * SCOPE;		// current scope
GLOBAL UInt32 TMP_IDX;
GLOBAL UInt32 TMP_LBL_IDX;
GLOBAL Var * MACRO_ARG_VAR[MACRO_ARG_CNT];

GLOBAL Var * REGSET;
GLOBAL Var * REG[64];		// Array of registers (register is variable with address in REGSET, submode has flag SUBMODE_REG)
GLOBAL UInt8 REG_CNT;		// Count of registers

char * TMP_NAME = "_";
char * TMP_LBL_NAME = "_lbl";
char * SCOPE_NAME = "_s";
UInt32 SCOPE_IDX;


void VarInit()
{
	UInt8 i;
	Var * var;

	VARS = NULL;
	LAST_VAR = NULL;

	TMP_IDX = 0;
	TMP_LBL_IDX = 0;
	SCOPE_IDX = 0;

	for(i=0; i<MACRO_ARG_CNT; i++) {
		var = VarAlloc(MODE_ARG, NULL, i+1);
		MACRO_ARG_VAR[i] = var;
	}

	REG_CNT = 0;
}

Var * VarNewElement(Var * arr, Var * idx, Bool ref)
/*
Purpose:
	Alloc new reference to array element.
Argument:
	ref		Array is accessed using reference.
*/
{
	Var * item;

	// Try to find same element

	Var * var;
	for (var = VARS; var != NULL; var = var->next) {
		if (var->mode == MODE_ELEMENT) {
			if (var->adr == arr && var->var == idx && (ref == FlagOn(var->submode, SUBMODE_REF)) ) return var;
		}
	}

	item = VarAlloc(MODE_ELEMENT, NULL, 0);
	if (ref) item->submode = SUBMODE_REF;
	item->adr  = arr;

	// Type of array element variable is type of array element
	// We may attempt to address individual bytes of non-array variable as an aray
	// in such case the type of the lement is byte.
	if (arr->type != NULL) {
		if (arr->type->variant == TYPE_ARRAY) {
			item->type = arr->type->element;
		} else if (arr->type->variant == TYPE_STRUCT) {
			item->type = idx->type;
			item->submode |= (idx->submode & (SUBMODE_IN|SUBMODE_OUT|SUBMODE_REG));
		} else {
			item->type = TypeByte();
		}
	} else {
	}
	item->var  = idx;
	// If this is element from in or out variable, it is in or out too
	item->submode |= (arr->submode & (SUBMODE_IN|SUBMODE_OUT|SUBMODE_REG));
	return item;
}


Var * VarMacroArg(UInt8 i)
{
	return MACRO_ARG_VAR[i];
}

/*
*/

void SetScope(Var * new_scope)
{
	SCOPE = new_scope;
}

void EnterSubscope(Var * new_scope)
{
	new_scope->scope = SCOPE;
	new_scope->instr = 0;
	SCOPE = new_scope;
}

void ExitScope()
{
	SCOPE = SCOPE->scope;
}

void EnterLocalScope()
{
	Var * var;
	SCOPE_IDX++;
	var = VarAlloc(MODE_SCOPE, SCOPE_NAME, SCOPE_IDX);
	EnterSubscope(var);
}

Var * VarFirst()
{
	return VARS;
}

Var * VarFindType(char * name, VarIdx idx, Type * type)
{
	Var * var;
	for (var = VARS; var != NULL; var = var->next) {
		if (var->idx == idx && var->type == type && StrEqual(name, var->name)) break;
	}
	return var;
}

Var * VarNewTmp(long idx, Type * type)
{
	UInt32 range;

	//TODO: Should check, that the variable found has appropriate type
	Var * var;
	if (type != NULL) {
		if (type->variant == TYPE_INT) {
			range = type->range.max - type->range.min;
			if (range > 255) {
				idx += 1000;
			} else if (range > 65535) {
				idx += 2000;
			}
		} else if (type->variant == TYPE_ADR) {
			idx += 3000;
		}
	} else {
		idx += 4000;
	}

	// We always create new temporary variable.
	// Usuall algorithm for reusing temporary variables in expressions does not work
	// for us very well, because we need the temporary variables have many different types
	// (not just some, like byte, word, long, but ranges like 13..26 etc.).

//	var = VarFind(TMP_NAME, idx);
	var = NULL;
	if (var == NULL) {
		TMP_IDX++;
		var = VarAlloc(MODE_VAR, NULL, TMP_IDX /*idx*/);
		var->name = TMP_NAME;
		var->type = type;
	}
	return var;
}

Bool VarIsArray(Var * var)
{
	return var->type != NULL && var->type->variant == TYPE_ARRAY;
}

Bool VarIsTmp(Var * var)
{
	return var->name == TMP_NAME;
}

Bool VarIsStructElement(Var * var)
{
	return var->adr->type->variant == TYPE_STRUCT;
}

Bool VarIsArrayElement(Var * var)
{
	return var->adr->type->variant == TYPE_ARRAY;
}


Var * VarFindInt(Var * scope, UInt32 n)
{
	Var * var;

	FOR_EACH_VAR(var)
		if (var->scope == scope && var->mode == MODE_CONST && var->n == n) return var;
	NEXT_VAR
	return NULL;
}

Var * VarNewInt(long n)
/*
Purpose:
	Alloc new integer constant variable.
*/
{
	Var * var;

	FOR_EACH_VAR(var)
		if (var->mode == MODE_CONST && var->name == NULL && var->type == &TINT && var->n == n) return var;
	NEXT_VAR

	var = VarAlloc(MODE_CONST, NULL, 0);
	var->type = &TINT;
	var->value_nonempty = true;
	var->n = n;
	return var;
}

Var * VarNewStr(char * str)
{
	Var * var;
	var = VarAlloc(MODE_CONST, NULL, 0);
	var->type = &TSTR;
	var->value_nonempty = true;
	var->str = StrAlloc(str);
	return var;
}

Var * VarNewLabel(char * name)
{
	Var * var;
	var = VarAlloc(MODE_LABEL, name, 0);
	var->type = &TLBL;
	return var;
}

void VarToLabel(Var * var)
{
	var->type = &TLBL;
	var->mode = MODE_VAR;
}

Var * VarNewTmpLabel()
{
	Var * var;
	TMP_LBL_IDX++;
	var = VarNewLabel(NULL);
	var->name = TMP_LBL_NAME;
	var->idx  = TMP_LBL_IDX;
	return var;
}

Var * VarAlloc(VarMode mode, char * name, VarIdx idx)
/*
Purpose:
	Alloc new variable.
*/
{
	Var * var;
	var = MemAllocStruct(Var);

	var->mode = mode;
	var->name  = StrAlloc(name);
	var->idx   = idx;
	var->scope = SCOPE;
	var->adr  = NULL;
	var->next  = NULL;

	if (VARS == NULL) {
		VARS = var;
	} else {
		LAST_VAR->next = var;
	}
	LAST_VAR = var;
	return var;
}

Var * VarFindScope(Var * scope, char * name, VarIdx idx)
/*
Purpose:
	Find variable in specified scope, and parent scopes.
	If scope is NULL, only global variables will be searched.
*/
{
	Var * var;
	for (var = VARS; var != NULL; var = var->next) {
		if (var->scope == scope && var->mode != MODE_UNDEFINED) {
			if (var->idx == idx && StrEqual(name, var->name)) break;
		}
	}
	return var;
}

Var * VarFind2(char * name, VarIdx idx)
/*
Purpose:
	Find variable in current scope.
*/
{
	Var * var = NULL;
	Var * s;
	for (s = SCOPE; s != NULL; s = s->scope) {
		var = VarFindScope(s, name, idx);
		if (var != NULL) break;
	}
	return var;
}


Var * VarFind(char * name, VarIdx idx)
{
	Var * var;
	for (var = VARS; var != NULL; var = var->next) {
		if (var->idx == idx && StrEqual(name, var->name)) break;
	}
	return var;
}


TypeVariant VarType(Var * var)
{
	if (var == NULL) return TYPE_VOID;
	if (var->type == NULL) return TYPE_VOID;
	return var->type->variant;
}

Bool VarIsLabel(Var * var)
{
	return var->type != NULL && var->type->variant == TYPE_LABEL;
}

//TODO: VarIsConst
Bool VarIsConst(Var * var)
{
	if (var == NULL) return false;
	if (var->mode == MODE_ELEMENT) return false;
	return var->mode != MODE_ARG && ((var->mode == MODE_CONST || (var->adr == NULL && var->value_nonempty) || var->name == NULL));
}


Var * VarNewType(TypeVariant variant)
{
	Var * var;
//	Type * type;
	var = VarAlloc(MODE_TYPE, NULL, 0);
	var->type = TypeAlloc(variant);
	return var;
}

Int16 TypeDim(Type * type)
{
	UInt16 d = 0;
	Type * dim;
	for(d=0; d<MAX_DIM_COUNT; d++) {
		dim = type->dim[d];
		if (dim == NULL) break;
	}
	return d;
}

void VarGenerateArrays()
/*
Purpose:
	Generate instructions allocating space for arrays.
*/
{
	Var * var, *cnst, *cnst2, *type_var;
	Type * type, * dim;
	UInt32 size;	//, i;
	UInt8 d;

	// Generate initialized arrays

	FOR_EACH_VAR(var)
		if (type = var->type) {
			if (type->variant == TYPE_ARRAY) {
				if ((var->mode == MODE_VAR || var->mode == MODE_CONST) && var->instr != NULL) {
					
					// Make array aligned (it type defines address, it is definition of alignment)
					type_var = type->owner;
					if (type_var->adr != NULL) {
						Gen(INSTR_ALIGN, NULL, type_var->adr, NULL);
					}
					// Label & initializers
					GenLabel(var);
					GenBlock(var->instr);
				}
			}
		}
	NEXT_VAR

	// Generate array indexes

	FOR_EACH_VAR(var)
		if (var->mode == MODE_VAR || var->mode == MODE_CONST) {
			type = var->type;
			if (type != NULL && type->variant == TYPE_ARRAY) {
				d = TypeDim(type);
				if (d == 2) {
					dim = type->dim[0];
					cnst = VarNewInt(dim->range.max - dim->range.min + 1);
					dim = type->dim[1];
					cnst2 = VarNewInt(dim->range.max - dim->range.min + 1);
					Gen(INSTR_ARRAY_INDEX, var, cnst, cnst2);
				}
			}
		}
	NEXT_VAR

	// Generate empty arrays

	FOR_EACH_VAR(var)
		type = var->type;
		if (type != NULL) {
			if (var->mode == MODE_VAR && type->variant == TYPE_ARRAY && var->adr == NULL && var->instr == NULL) {

				size = 1;	// size of basic element (byte by default)
				for(d=0; d<MAX_DIM_COUNT; d++) {			
					dim = type->dim[d];
					if (dim == NULL) break;
					size *= dim->range.max - dim->range.min + 1;
				}
				if (d == 2) {
					dim = type->dim[0];
					cnst = VarNewInt(dim->range.max - dim->range.min + 1);
					dim = type->dim[1];
					cnst2 = VarNewInt(dim->range.max - dim->range.min + 1);
					GenLabel(var);		// use the variable as label - this will set the address part of the variable
					Gen(INSTR_ALLOC, var, cnst, cnst2);
				} else {
					cnst = VarNewInt(size);
					GenLabel(var);		// use the variable as label - this will set the address part of the variable
					Gen(INSTR_ALLOC, var, cnst, NULL);
				}
			}
		}
	NEXT_VAR
}

void VarResetUse()
{
	Var * var;

	FOR_EACH_VAR(var)
		if (var->type == NULL || var->type->variant != TYPE_PROC) {
			var->read = 0;
			var->write = 0;
			var->flags = var->flags & (~(VarUninitialized|VarLoop|VarLoopDependent));
		}
	NEXT_VAR
}

void VarInitRegisters()
/*
Purpose:
	Initialize array of registers.
	All variables that have adress in REGSET are registers.
*/
{
	Var * var;
	FOR_EACH_VAR(var)
		if (var->adr != NULL && var->adr->scope == REGSET) {
			SetFlagOn(var->submode, SUBMODE_REG);
			REG[REG_CNT++] = var;
		}
	NEXT_VAR
}

Bool ProcIsInterrupt(Var * proc)
{
	Type * base;
	base = proc->type;
	if (base != NULL) {
		while(base->base != NULL) base = base->base;
		if (base->owner == INTERRUPT) return true;
	}
	return false;
}

void ProcUse(Var * proc, UInt8 flag)
/*
Purpose:
	Mark all used procedures starting with specified root procedure.
Arguments:
	flag		VarProcInterrupt  This procedure is used from interrupt routine
				VarProcAddress	  Address of this procedure is used
*/
{
	Instr * i;
	if (proc->instr == NULL) return;
	if (FlagOn(proc->flags, VarProcessed)) return;

	proc->read++;
	SetFlagOn(proc->flags, flag | VarProcessed);

	if (ProcIsInterrupt(proc)) {
		flag |= VarProcInterrupt;
	}

	for(i = proc->instr->first; i != NULL; i = i->next) {
		if (i->op == INSTR_CALL) {
			ProcUse(i->result, flag);
		} else {
			if (i->op != INSTR_LINE) {
				if (VarType(i->arg1) == TYPE_PROC) {
					ProcUse(i->arg1, flag | VarProcAddress);
				}
				if (VarType(i->arg2) == TYPE_PROC) {
					ProcUse(i->arg2, flag | VarProcAddress);
				}
			}
		}
	}
}
