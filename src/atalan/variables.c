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

/*
We have theoretical option of supporting mltiple CPUs simultaneously (this the CPUS array).
Current CPU used is stored in CPU variable.
*/

CPUType CPUS[1];			// currently, there is only one supported CPU
GLOBAL CPUType * CPU;		// current CPU (in case we use multiple CPUs in the future)

char * TMP_NAME = "_";
char * TMP_LBL_NAME = "_lbl";
char * SCOPE_NAME = "_s";
UInt32 SCOPE_IDX;

void VarInit()
{

	VARS = NULL;
	LAST_VAR = NULL;

	TMP_IDX = 1;
	TMP_LBL_IDX = 0;
	SCOPE_IDX = 0;

	// Alloc rule procedure and rule arguments (rule arguments are local arguments of RULE_PROC)

	CPU = &CPUS[0];
}

Var * VarFindOp(InstrOp op, Var * left, Var * right)
/*
Purpose:
	Find variable created as combination of two other variables.
Argument:
	ref		Array is accessed using reference.
*/
{
	Var * var;
	for (var = VARS; var != NULL; var = var->next) {
		if (var->mode == op && var->adr == left && var->var == right) return var;
	}
	return NULL;
}

Var * VarNewRange(Var * min, Var * max)
{
	Var * var = VarAlloc(INSTR_RANGE, NULL, 0);

	var->adr = min;
	var->var = max;

	return var;
}

Var * VarNewTuple(Var * left, Var * right)
/*
Purpose:
	Create new tuple from the two variables.
	If the right variable is NULL, left is returned.
*/
{
	Var * var;

	if (right == NULL) return left;
	if (left == NULL) return right;

	var = VarFindOp(INSTR_TUPLE, left, right);
	if (var == NULL) {
		var = VarAllocScope(NO_SCOPE, INSTR_TUPLE, NULL, 0);
		var->type = TypeTuple(left->type, right->type);
		var->adr = left;
		var->var = right;
	}
	return var;
}

Var * VarNewDeref(Var * adr)
{
	Var * var;
	for (var = VARS; var != NULL; var = var->next) {
		if (var->mode == INSTR_DEREF && var->var == adr) return var;
	}

	var = VarAlloc(INSTR_DEREF, NULL, 0);
	var->var = adr;
	if (adr->type != NULL && adr->type->variant == TYPE_ADR) {
		var->type = adr->type->element;
	}
	return var;
}

Var * VarNewOp(InstrOp op, Var * arr, Var * idx)
/*
Purpose:
	Alloc new reference to array element.
Argument:
	ref		Array is accessed using reference.
*/
{
	Var * item;

	// Try to find same element

	Var * var = VarFindOp(op, arr, idx);
	if (var != NULL) return var;
/*
	for (var = VARS; var != NULL; var = var->next) {
		if (var->mode == op) {
			if (var->adr == arr && var->var == idx) return var;
		}
	}
*/
	item = VarAlloc(op, NULL, 0);
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

Var * VarNewElement(Var * arr, Var * idx)
{
	return VarNewOp(INSTR_ELEMENT, arr, idx);
}

Var * VarNewByteElement(Var * arr, Var * idx)
/*
Purpose:
	Alloc new reference to specified byte of variable.
Argument:
	ref		Array is accessed using reference.
*/
{	
	//TODO: VarNewByteElement may create simple integer when used with two integer constants

	Var * item = VarNewOp(INSTR_BYTE, arr, idx);
	item->type = TypeByte();
	return item;
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
	var = VarAlloc(INSTR_SCOPE, SCOPE_NAME, SCOPE_IDX);
	var->type = TypeScope();
	var->scope = SCOPE;
	SCOPE = var;
}

Var * VarFirst()
{
	return VARS;
}

Var * VarNewTmp(Type * type)
{
	Var * var;
	var = VarAllocScopeTmp(NULL, INSTR_VAR, type);
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
	if (var == NULL || var->mode != INSTR_ELEMENT) return false;
	adr = var->adr;
//	if (adr == NULL) return false;			// TODO: ELEMENT with NULL adr?
	if (adr->mode == INSTR_DEREF) return true;
	return adr->type != NULL && adr->type->variant == TYPE_ARRAY;
//	return var != NULL && var->mode == INSTR_ELEMENT && var->adr != NULL && var->adr->type != NULL && (var->adr->type->variant == TYPE_ARRAY);
}
/*
Bool VarIsOut(Var * var)
{
	if (var == NULL) return false;
	if (FlagOn(var->submode, SUBMODE_OUT)) return true;
	if (var->mode == INSTR_ELEMENT) {
		return VarIsOut(var->adr);
	}
	return false;
}
*/

Var * VarFindInt(Var * scope, UInt32 n)
{
	Var * var;

	FOR_EACH_VAR(var)
		if (var->scope == scope && var->mode == INSTR_CONST && var->n == n) return var;
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
		if (var->mode == INSTR_CONST && var->name == NULL && var->type == &TINT && var->n == n) return var;
	NEXT_VAR

	//TODO: Integer constants may be in special 'const' scope
	var = VarAlloc(INSTR_CONST, NULL, 0);
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
	var = VarAlloc(INSTR_CONST, NULL, 0);
	VarLetStr(var, str);
	return var;
}

Var * VarNewLabel(char * name)
{
	Var * var;
	var = VarAlloc(INSTR_VAR, name, 0);
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
	var->mode = INSTR_VAR;
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


Var * VarAllocScope(Var * scope, InstrOp mode, Name name, VarIdx idx)
{
	Var * var;
	if (scope == NULL) scope = SCOPE;
	if (scope == NO_SCOPE) scope = NULL;

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

Var * VarAllocScopeTmp(Var * scope, InstrOp mode, Type * type)
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

Var * VarClone(Var * scope, Var * var)
{
	Var * copy;

	copy = MemAllocStruct(Var);
	MemMove(copy, var, sizeof(Var));
	var->scope = scope;
	return copy;
}

Var * VarAlloc(InstrOp mode, char * name, VarIdx idx)
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
	Find variable in specified scope.
	If scope is NULL, only global variables will be searched.
*/
{
	Var * var;
	for (var = VARS; var != NULL; var = var->next) {
		if (var->scope == scope && var->mode != INSTR_VOID) {
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
	PrintFmt("===== %s =======\n", scope->name);
	for (var = VARS; var != NULL; var = var->next) {
		if (var->scope == scope) {
			PrintVar(var);
		}
	}
}

Var * VarFindScope2(Var * scope, char * name)
{
	Var * var = NULL;
	Var * s;
	for (s = SCOPE; s != NULL; s = s->scope) {
		var = VarFindScope(s, name, 0);
		if (var != NULL) break;
		// For procedures whose type has been defined using predefined type, try to find arguments from this type
		if (s->type->variant == TYPE_PROC && s->type->owner != NULL && s->type->owner != s) {
			var = VarFindScope(s->type->owner, name, 0);
			if (var != NULL) break;
		}
	}
	return var;
}

Var * VarFind2(char * name)
/*
Purpose:
	Find variable in current scope.
*/
{
	return VarFindScope2(SCOPE, name);
/*
	Var * var = NULL;
	Var * s;
	for (s = SCOPE; s != NULL; s = s->scope) {
		var = VarFindScope(s, name, 0);
		if (var != NULL) break;
	}
	return var;
*/
}

Var * VarProcScope()
{
	Var * s;
	for (s = SCOPE; s != NULL; s = s->scope) {
		if (s->type != NULL && (s->type->variant == TYPE_PROC || s->type->variant == TYPE_MACRO)) break;
	}
	return s;
}

Var * VarFind(char * name, VarIdx idx)
{
	Var * var;
	for (var = VARS; var != NULL; var = var->next) {
		if (var->idx == idx && StrEqual(name, var->name)) break;
	}
	return var;
}

Var * VarFindTypeVariant(Name name, VarIdx idx, TypeVariant type_variant)
{
	Var * var;
	for (var = VARS; var != NULL; var = var->next) {
		if (var->idx == idx && StrEqual(name, var->name) && (type_variant == TYPE_UNDEFINED || (var->type != NULL && var->type->variant == type_variant))) break;
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
	return var->mode == INSTR_CONST;
}

Bool VarIsIntConst(Var * var)
{
	return var != NULL && var->mode == INSTR_CONST && var->type->variant == TYPE_INT;
}

Bool VarIsN(Var * var, Int32 n)
{
	return VarIsIntConst(var) && var->n == n;
}

Var * VarNewType(TypeVariant variant)
{
	Var * var;
//	Type * type;
	var = VarAlloc(INSTR_TYPE, NULL, 0);
	var->type = TypeAlloc(variant);
	return var;
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
	Type * type;
	UInt32 size;	//, i;
	Var * dim1, * dim2;

	// Generate empty arrays

	FOR_EACH_VAR(var)
		type = var->type;
		if (type != NULL) {
			if (var->mode == INSTR_VAR && type->variant == TYPE_ARRAY && var->adr == NULL && var->instr == NULL) {

				if (VarIsUsed(var)) {
					
					size = TypeSize(type);
/*
					size = 1;	// size of basic element (byte by default)
					for(d=0; d<MAX_DIM_COUNT; d++) {			
						dim = type->dim[d];
						if (dim == NULL) break;
						size *= dim->range.max - dim->range.min + 1;
					}
*/

					// Make array aligned (it type defines address, it is definition of alignment)
					type_var = type->owner;
					if (type_var->adr != NULL) {
						EmitInstrOp(INSTR_ALIGN, NULL, type_var->adr, NULL);
					}

					ArraySize(type, &dim1, &dim2);

					if (dim2 != NULL) {
						EmitInstrOp(INSTR_LABEL, var, NULL, NULL);		// use the variable as label - this will set the address part of the variable
						EmitInstrOp(INSTR_ALLOC, var, dim1, dim2);
					} else {
						cnst = VarNewInt(size);
						EmitInstrOp(INSTR_LABEL, var, NULL, NULL);		// use the variable as label - this will set the address part of the variable
						EmitInstrOp(INSTR_ALLOC, var, cnst, NULL);
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
			if ((var->mode == INSTR_VAR || var->mode == INSTR_CONST) && var->instr != NULL && var->adr == NULL) {		
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
		if (var->mode == INSTR_VAR || var->mode == INSTR_CONST) {
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
			if ((var->mode == INSTR_VAR || var->mode == INSTR_CONST) && var->instr != NULL && var->adr != NULL && VarIsUsed(var)) {
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

void InitCPU()
/*
Purpose:
	Initialize array of registers.
	All variables that have adress in REGSET are registers.
*/
{
	Var * var;

	if (CPU->MEMORY == NULL) {
		CPU->REG_CNT = 0;

		if (CPU->SCOPE == NULL) {
			InternalError("CPU scope not found");
		}

		FOR_EACH_LOCAL(CPU->SCOPE, var)
			// Only variables without address are registers.
			// The variables with address are register sets.
			if (var->mode == INSTR_VAR && var->adr == NULL && var->type->variant == TYPE_INT) {
				SetFlagOn(var->submode, SUBMODE_REG);
				CPU->REG[CPU->REG_CNT++] = var;
			}
		NEXT_LOCAL

		var = VarFindScope(CPU->SCOPE, "memory", 0);
		if (var != NULL) {
			CPU->MEMORY = var->type;
		} else {
			InternalError("CPU.memory was not defined");
		}
	}
}

void VarResetRegUse()
{
	RegIdx i;
	for(i = 0; i<CPU->REG_CNT; i++) {
		SetFlagOff(CPU->REG[i]->flags, VarUsed);
	}
}

Bool VarIsReg(Var * var)
/*
Purpose:
	Return true, if this variable is register or is stored in register(s).	
*/
{
	if (var == NULL) return false;

	if (var->mode == INSTR_VAR) {
		if (FlagOn(var->submode, SUBMODE_REG)) return true;
		return VarIsReg(var->adr);		// variable address may be register
	} else if (var->mode == INSTR_TUPLE) {
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
		if (var->mode == INSTR_ELEMENT) {
			return 1;		//TODO: Compute size in a better way
		} else if (var->mode == INSTR_BYTE) {
			return 1;
		} else if (var->mode == INSTR_CONST) {
			return ConstByteSize(var->n);
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
	Var * var, * label;
	UInt16 bmk;
	Loc loc;
	UInt16 n;

	loc.proc = proc;

	proc->read++;

	if (proc->instr == NULL) return;
	if (FlagOn(proc->flags, VarProcessed)) return;

//	PrintProc(proc);

	SetFlagOn(proc->flags, flag | VarProcessed);

	if (ProcIsInterrupt(proc)) {
		flag |= VarProcInterrupt;
	}

	// Mark all defined labels (those defined with label instruction)

	for(blk = proc->instr; blk != NULL; blk = blk->next) {
		if (blk->label != NULL) {
			SetFlagOn(blk->label->flags, VarLabelDefined);
		}
		for(i = blk->first; i != NULL; i = i->next) {
			if (i->op == INSTR_LABEL) {
				SetFlagOn(i->result->flags, VarLabelDefined);
			}
		}
	}

	for(blk = proc->instr; blk != NULL; blk = blk->next) {
		for(i = blk->first, n=1; i != NULL; i = i->next, n++) {
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
					label = i->result;
					if (label != NULL && label->type->variant == TYPE_LABEL) {
						if (FlagOff(label->flags, VarLabelDefined)) {

							loc.blk = blk;
							loc.i   = i;
							bmk = SetBookmarkLine(&loc);

							var = VarFindTypeVariant(label->name, label->idx, TYPE_LABEL);
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

Var * VarReplaceVar(Var * var, Var * from, Var * to)
/*
Purpose:
	Replace one variable by another.
	Variable may be used for example in array indexes, tuples etc.
	Replacement is performed 'in place'.
*/
{
	Var * l, * r;
	if (var == NULL) return NULL;
	if (var == from) return to;

	if (var->mode == INSTR_ELEMENT || var->mode == INSTR_TUPLE || var->mode == INSTR_RANGE) {
		l = VarReplaceVar(var->adr, from, to);
		r = VarReplaceVar(var->var, from, to);
		if (l != var->adr || r != var->var) var = VarNewOp(var->mode, l, r);

	} else if (var->mode == INSTR_DEREF) {
		l = VarReplaceVar(var->var, from, to);
		if (l != var->var) var = VarNewDeref(l);
	}
	return var;
}

void InstrReplaceVar(InstrBlock * block, Var * from, Var * to)
/*
Purpose:
	Replace use of variable 'from' with variable 'to' in specified block.
	When this procedure ends, 'from' variable is no more referenced in the block.
*/
{
	Instr * i;
	InstrBlock * nb;
	for(nb = block; nb != NULL; nb = nb->next) {
		for (i = nb->first; i != NULL; i = i->next) {
			if (i->op != INSTR_LINE) {
				i->result = VarReplaceVar(i->result, from, to);
				i->arg1   = VarReplaceVar(i->arg1, from, to);
				i->arg2   = VarReplaceVar(i->arg2, from, to);
			}
		}
	}
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
	if (var->mode == INSTR_VAR) return true;
	if (var->mode == INSTR_ELEMENT && var->var->mode == INSTR_CONST) return true;		// access to constant array element
	return false;
}

Bool VarModifiesVar(Var * var, Var * test_var)
{
	Bool uses = false;
	if (var != NULL && test_var != NULL) {
		if (var->mode == INSTR_CONST || test_var->mode == INSTR_CONST) return false;

		if (var == test_var) {
			uses = true;
		} else {
			
			if (test_var->mode == INSTR_TUPLE) {
				return VarModifiesVar(var, test_var->adr) || VarModifiesVar(var, test_var->var);
			} else if (test_var->mode == INSTR_VAR && test_var->adr != NULL) {
				return VarModifiesVar(var, test_var->adr);
			}

			if (var->mode == INSTR_TUPLE) {
				return VarModifiesVar(var->adr, test_var) || VarModifiesVar(var->var, test_var);
			}

//			if (var->mode == INSTR_DEREF) {
//				uses = VarUsesVar(var->var, test_var);
//			} else if (var->mode == INSTR_ELEMENT || var->mode == INSTR_BYTE || var->mode == INSTR_TUPLE) {
//				uses = VarUsesVar(var->var, test_var) || VarUsesVar(var->adr, test_var);
//			} else if (var->adr != NULL) {
//				if (var->adr->mode != INSTR_CONST) {
//					return VarUsesVar(var->adr, test_var);
//				}
//			}
		}
	}
	return uses;	
}

Bool VarUsesVar(Var * var, Var * test_var)
/*
Purpose:
	Return true, if the specified variable uses tested variable.
	It may either be same, or use the tested variable as index into array etc.
*/
{
	Bool uses = false;
	if (var != NULL && test_var != NULL) {

		if (test_var->mode == INSTR_CONST) return false;

		if (var == test_var) {
			return true;
		} else {

			if (test_var->mode == INSTR_TUPLE) {
				return VarUsesVar(var, test_var->adr) || VarUsesVar(var, test_var->var);
			} else {

				if (var->mode == INSTR_DEREF) {
					return VarUsesVar(var->var, test_var);
				} else if (var->mode == INSTR_ELEMENT || var->mode == INSTR_BYTE || var->mode == INSTR_TUPLE) {
					return VarUsesVar(var->var, test_var) || VarUsesVar(var->adr, test_var);
				} else if (var->adr != NULL) {
					if (var->adr->mode != INSTR_CONST) {
						return VarUsesVar(var->adr, test_var);
					}
				}
			}
		}
		if (test_var->mode == INSTR_VAR && test_var->adr != NULL) {
			return VarUsesVar(var, test_var->adr);
		}
	}
	return false;	
}

Var * VarReg(Var * var)
/*
Purpose:
	Return register that is aliased by this variable or NULL.
*/
{
	Var * reg;

	reg = var;
	while(reg != NULL && reg->mode == INSTR_VAR) {
		if (FlagOn(reg->submode, SUBMODE_REG)) return reg;
		reg = reg->adr;
		if (reg != NULL && reg->mode == INSTR_TUPLE) return reg;
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
	while(var != NULL && (var->mode != INSTR_VAR || var->scope != arg->scope || FlagOff(var->submode, submode))) var = var->next;
	return var;
}

Var * FirstArg(Var * proc, VarSubmode submode)
{
	Var * var;
	Var * owner;

	// We may call the procedure using address stored in a variable, but we need to parse the arguments using
	// procedure itself.
	if (proc->type->variant == TYPE_ADR) {
		proc = proc->type->element->owner;
	}

	// If this procedure type is defined using shared definition, use the definition to fing the arguments

	owner = proc->type->owner;
	if (owner != NULL && owner != proc) {
		proc = owner;
	}
	var = proc->next;
	while(var != NULL && (var->mode != INSTR_VAR || var->scope != proc || FlagOff(var->submode, submode))) var = var->next;
	return var;
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

	if (fld == NULL) {
		fld = VarFindScope(var, fld_name, 0);
	}
	return fld;
}

Var * VarEvalConst(Var * var)
{
	Var * r = var;
	IntLimit a, idx;

	if (var != NULL) {
		if (var->mode == INSTR_BYTE) {
			if (VarIsIntConst(var->adr) && VarIsIntConst(var->var)) {
				a = var->adr->n;
				idx = var->var->n;
				a = (a >> (idx * 8)) & 0xff;
				r = VarNewInt(a);
			}
		}
	}
	return r;
}


void VarLet(Var * var, Var * val)
/*
Purpose:
	Set the variable var to specified value.
*/
{
	if (var != NULL && val != NULL) {
		if (val->type->variant == TYPE_INT) {
			var->n = val->n;
			var->value_nonempty = true;
		}
	}
}

Bool VarIsInArg(Var * var)
{
	return FlagOn(var->submode, SUBMODE_ARG_IN);
}

Bool VarIsOutArg(Var * var)
{
	return FlagOn(var->submode, SUBMODE_ARG_OUT);
}

Bool VarIsArg(Var * var)
{
	return FlagOn(var->submode, SUBMODE_ARG_IN | SUBMODE_ARG_OUT);
}

Bool VarIsRuleArg(Var * var)
{
	return var->scope == RULE_PROC;
}

Bool VarIsEqual(Var * left, Var * right)
{
	if (left == NULL || right == NULL) return false;
	if (left == right) return true;
	if (left->mode == INSTR_VAR && left->adr != NULL) return VarIsEqual(left->adr, right);
	if (right->mode == INSTR_VAR && right->adr != NULL) return VarIsEqual(left, right->adr);

	if (left->mode == right->mode) {
		if (left->mode == INSTR_TUPLE) {
			return VarIsEqual(left->adr, right->adr) && VarIsEqual(left->var, right->var);
		}
	}

	return false;
}

Var * VarFindAssociatedConst(Var * var, char * name)
{
	if (var == NULL) return NULL;
	return VarFindScope(var->type->owner, name, 0);
}
