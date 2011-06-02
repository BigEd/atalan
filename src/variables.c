/*
Table of variables

There is one global table of variables.

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

/*
Registers are special type of variable.
Because we use them a lot, we create an array of references to registers here.
*/

GLOBAL Var * REGSET;
GLOBAL Var * REG[64];		// Array of registers (register is variable with address in REGSET, submode has flag SUBMODE_REG)
GLOBAL RegIdx REG_CNT;		// Count of registers

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

	TMP_IDX = 1;
	TMP_LBL_IDX = 0;
	SCOPE_IDX = 0;

	for(i=0; i<MACRO_ARG_CNT; i++) {
		var = VarAlloc(MODE_ARG, NULL, i+1);
		MACRO_ARG_VAR[i] = var;
	}

	REG_CNT = 0;
}

Var * VarNewRange(Var * min, Var * max)
{
	Var * var = VarAlloc(MODE_RANGE, NULL, 0);

	var->adr = min;
	var->var = max;

	return var;
}

Var * VarNewTuple(Var * left, Var * right)
{
	Var * var;

	for (var = VARS; var != NULL; var = var->next) {
		if (var->mode == MODE_TUPLE && var->adr == left && var->var == right) return var;
	}

	var = VarAlloc(MODE_TUPLE, NULL, 0);
	var->type = TypeTuple();
	var->adr = left;
	var->var = right;
	return var;
}

Var * VarNewDeref(Var * adr)
{
	Var * var;
	for (var = VARS; var != NULL; var = var->next) {
		if (var->mode == MODE_DEREF && var->var == adr) return var;
	}

	var = VarAlloc(MODE_DEREF, NULL, 0);
	var->var = adr;
	if (adr->type != NULL && adr->type->variant == TYPE_ADR) {
		var->type = adr->type->element;
	}
	return var;
}

Var * VarNewByteElement(Var * arr, Var * idx)
/*
Purpose:
	Alloc new reference to specified byte of variable.
Argument:
	ref		Array is accessed using reference.
*/
{
	Var * item;

	// Try to find same element

	Var * var;
	for (var = VARS; var != NULL; var = var->next) {
		if (var->mode == MODE_BYTE) {
			if (var->adr == arr && var->var == idx) return var;
		}
	}

	item = VarAlloc(MODE_BYTE, NULL, 0);
	item->adr = arr;
	item->var = idx;
	item->type = TypeByte();
	return item;
}

Var * VarNewElement(Var * arr, Var * idx)
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
			if (var->adr == arr && var->var == idx /*&& (ref == FlagOn(var->submode, SUBMODE_REF))*/ ) return var;
		}
	}

	item = VarAlloc(MODE_ELEMENT, NULL, 0);
//	if (ref) item->submode = SUBMODE_REF;
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

Var * InScope(Var * new_scope)
{
	Var * scope;
	scope = SCOPE;
	SCOPE = new_scope;
	return scope;
}

void ReturnScope(Var * prev)
{
	SCOPE = prev;
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
	var->type = TypeScope();
	var->scope = SCOPE;
	SCOPE = var;
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
	Var * var;
	var = VarAllocScopeTmp(NULL, MODE_VAR, type);
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
	Var * adr;
	if (var == NULL || var->mode != MODE_ELEMENT) return false;
	adr = var->adr;
//	if (adr == NULL) return false;			// TODO: ELEMENT with NULL adr?
	if (adr->mode == MODE_DEREF) return true;
	return adr->type != NULL && adr->type->variant == TYPE_ARRAY;
//	return var != NULL && var->mode == MODE_ELEMENT && var->adr != NULL && var->adr->type != NULL && (var->adr->type->variant == TYPE_ARRAY);
}
/*
Bool VarIsOut(Var * var)
{
	if (var == NULL) return false;
	if (FlagOn(var->submode, SUBMODE_OUT)) return true;
	if (var->mode == MODE_ELEMENT) {
		return VarIsOut(var->adr);
	}
	return false;
}
*/
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

	//TODO: Integer constants may be in special 'const' scope
	var = VarAlloc(MODE_CONST, NULL, 0);
	var->scope = NULL;
	var->type = &TINT;
	var->value_nonempty = true;
	var->n = n;
	return var;
}

void VarLetStr(Var * var, char * str)
{
	var->type = &TSTR;
	var->value_nonempty = true;
	var->str = StrAlloc(str);
}

Var * VarNewStr(char * str)
{
	Var * var;
	var = VarAlloc(MODE_CONST, NULL, 0);
	VarLetStr(var, str);
	return var;
}

Var * VarNewLabel(char * name)
{
	Var * var;
	var = VarAlloc(MODE_LABEL, name, 0);
	var->type = &TLBL;
	return var;
}

Var * FindOrAllocLabel(char * name, UInt16 idx)
{
	Var * var = NULL;
	Var * proc;
	Var * scope;

	proc = VarProcScope();
	var = VarFindScope(proc, name, idx);
	if (var == NULL) {
		scope = InScope(proc);
		var = VarNewLabel(name);
		var->idx = idx;
		ReturnScope(scope);
	}
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

Var * VarAllocScope(Var * scope, VarMode mode, Name name, VarIdx idx)
{
	Var * var;
	if (scope == NULL) scope = SCOPE;

	var = MemAllocStruct(Var);

	var->mode = mode;
	var->name  = StrAlloc(name);
	var->idx   = idx;
	var->scope = scope;
	var->adr  = NULL;
	var->next  = NULL;
	var->type  = TUNDEFINED;		// freshly allocated variable has undefined type (but not NULL!)

	if (VARS == NULL) {
		VARS = var;
	} else {
		LAST_VAR->next = var;
	}
	LAST_VAR = var;
	return var;

}

Var * VarAllocScopeTmp(Var * scope, VarMode mode, Type * type)
/*
Purpose:
	Alloc new temporary variable in specified scope.
*/
{
	Var * var;
	var = VarAllocScope(scope, mode, NULL, 0);
	var->name = TMP_NAME;
	var->idx = TMP_IDX;
	if (type == NULL) type = TUNDEFINED;
	var->type = type;
	TMP_IDX++;
	return var;
}

Var * VarAlloc(VarMode mode, char * name, VarIdx idx)
/*
Purpose:
	Alloc new variable.
*/
{
	return VarAllocScope(SCOPE, mode, name, idx);
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

void PrintScope(Var * scope)
/*
Purpose:
	Find variable in specified scope, and parent scopes.
	If scope is NULL, only global variables will be searched.
*/
{
	Var * var;
	printf("===== %s =======\n", scope->name);
	for (var = VARS; var != NULL; var = var->next) {
		if (var->scope == scope) {
			PrintVar(var);
		}
	}
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

Var * VarProcScope()
{
	Var * s;
	for (s = SCOPE; s != NULL; s = s->scope) {
		if (s->type != NULL && (s->type->variant == TYPE_PROC || s->type->variant == TYPE_MACRO)) break;
	}
	return s;
}
/*
Var * VarFindInProc(char * name, VarIdx idx)
Purpose:
	Find variable in current scope.
{
	Var * var = NULL;
	Var * s = SCOPE;
	do {
		var = VarFindScope(s, name, idx);
		if (var != NULL) break;
		if (s->type != NULL && s->type->variant == TYPE_PROC) break;
		s = s->scope;
	} while(s != NULL);

	return var;
}
*/

Var * VarFind(char * name, VarIdx idx)
{
	Var * var;
	for (var = VARS; var != NULL; var = var->next) {
		if (var->idx == idx && StrEqual(name, var->name)) break;
	}
	return var;
}

Var * VarFindMode(Name name, VarIdx idx, VarMode mode)
{
	Var * var;
	for (var = VARS; var != NULL; var = var->next) {
		if (var->mode == mode && var->idx == idx && StrEqual(name, var->name)) break;
	}
	return var;
}

TypeVariant VarType(Var * var)
{
	if (var == NULL) return TYPE_VOID;
	return var->type->variant;
}

Bool VarIsLabel(Var * var)
{
	return var->type->variant == TYPE_LABEL;
}

Bool VarIsConst(Var * var)
{
	if (var == NULL) return false;
	return var->mode == MODE_CONST;
}

Bool VarIsIntConst(Var * var)
{
	return var != NULL && var->mode == MODE_CONST && var->type->variant == TYPE_INT;
}

Bool VarIsN(Var * var, Int32 n)
{
	return VarIsIntConst(var) && var->n == n;
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

Bool VarIsUsed(Var * var)
{
	return var != NULL && (var->read > 0 || var->write > 0);
}

void VarEmitAlloc()
/*
Purpose:	
	Emit instructions allocating variables, that are not placed at specific location.
*/
{
	Var * var, *cnst, * type_var;
	Type * type, * dim;
	UInt32 size;	//, i;
	UInt8 d;
	Var * dim1, * dim2;

	// Generate empty arrays

	FOR_EACH_VAR(var)
		type = var->type;
		if (type != NULL) {
			if (var->mode == MODE_VAR && type->variant == TYPE_ARRAY && var->adr == NULL && var->instr == NULL) {

				if (VarIsUsed(var)) {
					size = 1;	// size of basic element (byte by default)
					for(d=0; d<MAX_DIM_COUNT; d++) {			
						dim = type->dim[d];
						if (dim == NULL) break;
						size *= dim->range.max - dim->range.min + 1;
					}

					// Make array aligned (it type defines address, it is definition of alignment)
					type_var = type->owner;
					if (type_var->adr != NULL) {
						EmitInstrOp(INSTR_ALIGN, NULL, type_var->adr, NULL);
					}

					ArraySize(type, &dim1, &dim2);

					if (dim2 != NULL) {
	//					dim = type->dim[0];
	//					cnst = VarNewInt(dim->range.max - dim->range.min + 1);
	//					dim = type->dim[1];
	//					cnst2 = VarNewInt(dim->range.max - dim->range.min + 1);
						EmitInstrOp(INSTR_LABEL, var, NULL, NULL);		// use the variable as label - this will set the address part of the variable
						EmitInstrOp(INSTR_ALLOC, var, dim1, dim2);
	//					Gen(INSTR_ALLOC, var, cnst, cnst2);
					} else {
						cnst = VarNewInt(size);
						EmitInstrOp(INSTR_LABEL, var, NULL, NULL);		// use the variable as label - this will set the address part of the variable
						EmitInstrOp(INSTR_ALLOC, var, cnst, NULL);
	//					GenLabel(var);		// use the variable as label - this will set the address part of the variable
	//					Gen(INSTR_ALLOC, var, cnst, NULL);
					}
				}
			}
		}
	NEXT_VAR
}

void VarGenerateArrays()
/*
Purpose:
	Generate instructions allocating space for arrays.
*/
{
	Var * var, *type_var;
	Type * type;
	Var * dim1, * dim2;

	// Generate initialized arrays, where location is not specified

	FOR_EACH_VAR(var)
		type = var->type;
		if (type->variant == TYPE_ARRAY) {
			if ((var->mode == MODE_VAR || var->mode == MODE_CONST) && var->instr != NULL && var->adr == NULL) {		
				if (VarIsUsed(var)) {
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
				if (VarIsUsed(var)) {
					ArraySize(type, &dim1, &dim2);
					if (dim2 != NULL) {
						Gen(INSTR_ARRAY_INDEX, var, dim1, dim2);
					}
				}
			}
		}
	NEXT_VAR

	// Generate initialized arrays at specified addresses

	FOR_EACH_VAR(var)
		type = var->type;
		if (type->variant == TYPE_ARRAY) {
			if ((var->mode == MODE_VAR || var->mode == MODE_CONST) && var->instr != NULL && var->adr != NULL && VarIsUsed(var)) {
				Gen(INSTR_ORG, NULL, var->adr, NULL);
				GenLabel(var);
				GenBlock(var->instr);
			}
		}
	NEXT_VAR

}

void VarResetUse()
{
	Var * var;

	FOR_EACH_VAR(var)
		if (var->type->variant != TYPE_PROC) {
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

void VarResetRegUse()
{
	RegIdx i;
	for(i = 0; i<REG_CNT; i++) {
		SetFlagOff(REG[i]->flags, VarUsed);
	}
}

Bool VarIsReg(Var * var)
/*
Purpose:
	Return true, if this variable is register or is stored in register(s).	
*/
{
	if (var == NULL) return false;

	if (var->mode == MODE_VAR || var->mode == MODE_ARG) {
		if (FlagOn(var->submode, SUBMODE_REG)) return true;
		return VarIsReg(var->adr);		// variable address may be register
	} else if (var->mode == MODE_TUPLE) {
		return VarIsReg(var->adr) || VarIsReg(var->var);
	}
	return false;
}

UInt32 VarByteSize(Var * var)
/*
Purpose:
	Return size of variable in bytes.
*/
{
	Type * type;
	if (var != NULL) {
		type = var->type;
		if (var->mode == MODE_ELEMENT) {
			return 1;		//TODO: Compute size in a better way
		}
		return TypeSize(type);
	}
	return 0;
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
	InstrBlock * blk;
	Var * var;
	UInt16 bmk;
	Loc loc;

	loc.proc = proc;

	proc->read++;

	if (proc->instr == NULL) return;
	if (FlagOn(proc->flags, VarProcessed)) return;

	SetFlagOn(proc->flags, flag | VarProcessed);

	if (ProcIsInterrupt(proc)) {
		flag |= VarProcInterrupt;
	}

	// Mark all defined labels (those defined with label instruction)

	for(blk = proc->instr; blk != NULL; blk = blk->next) {
		for(i = blk->first; i != NULL; i = i->next) {
			if (i->op == INSTR_LABEL) {
				SetFlagOn(i->result->flags, VarLabelDefined);
			}
		}
	}

	for(blk = proc->instr; blk != NULL; blk = blk->next) {
		for(i = blk->first; i != NULL; i = i->next) {
			if (i->op == INSTR_CALL) {
				ProcUse(i->result, flag);
//				// Procedure has side-effect, if it call a procedure with side effect
//					if (FlagOn(i->result->submode, SUBMODE_OUT)) {
//					SetFlagOn(proc->submode, SUBMODE_OUT);
//				}
			} else {
				if (i->op != INSTR_LINE) {
					if (VarType(i->arg1) == TYPE_PROC) {
						ProcUse(i->arg1, flag | VarProcAddress);
					}
					if (VarType(i->arg2) == TYPE_PROC) {
						ProcUse(i->arg2, flag | VarProcAddress);
					}
/*
					if (i->result != NULL) {
						if (!VarIsLocal(i->result, proc)) {
							SetFlagOn(proc->submode, SUBMODE_OUT);
						}
					}
*/
					if (i->result != NULL && i->result->mode == MODE_LABEL) {
						if (FlagOff(i->result->flags, VarLabelDefined)) {

							loc.blk = blk;
							loc.i   = i;
							bmk = SetBookmarkLine(&loc);

							var = VarFindMode(i->result->name, i->result->idx, MODE_LABEL);
							if (var != NULL) {
								ErrArg(var);
								SyntaxErrorBmk("Label [A] is defined in other procedure.\nIt is not possible to jump between procedures.", bmk);								
							} else {
								SyntaxErrorBmk("Label [A] is undefined", bmk);
							}
						}
					}
				}
			}
		}
	}
	SetFlagOff(proc->flags, VarProcessed);
}

void ProcReplaceVar(Var * proc, Var * from, Var * to)
/*
Purpose:
	Replace use of specific variable in a procedure by other variable.
*/
{
	InstrBlock * blk;

	for(blk = proc->instr; blk != NULL; blk = blk->next) {
		InstrReplaceVar(blk, from, to);
	}
}

Bool VarIsFixed(Var * var)
/*
Purpose:
	Test, that variable is on fixed location.
	This means normal variable, argument or reference to array element with constant index.
*/
{
	if (var->mode == MODE_VAR || var->mode == MODE_ARG) return true;
	if (var->mode == MODE_ELEMENT && var->var->mode == MODE_CONST) return true;		// access to constant array element
	return false;
}

Bool VarUsesVar(Var * var, Var * test_var)
/*
Purpose:
	Return true, if the specified variable uses tested variable.
	It may either be same, or use the tested variable as index into array etc.
*/
{
	Bool uses = false;
	if (var != NULL) {
		if (var == test_var) {
			uses = true;
		} else {
			if (var->mode == MODE_DEREF) {
				uses = VarUsesVar(var->var, test_var);
			} else if (var->mode == MODE_ELEMENT || var->mode == MODE_TUPLE) {
				uses = VarUsesVar(var->var, test_var) || VarUsesVar(var->adr, test_var);
			}
		}
	}
	return uses;	
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

Bool VarIsLocal(Var * var, Var * scope)
{
	while (var != NULL) {
		if (var->scope == scope) return true;
		var = var->scope;
	}
	return false;
}
