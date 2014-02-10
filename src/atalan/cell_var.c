/*
Variables

Variables are represented as cells with specified name, type, and possibly address.
They always belong to some scope.

* name
* scope
* type
* address

(c) 2012 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"

GLOBAL UInt32 TMP_IDX;
char * TMP_NAME = "_";

void VarSetType(Var * var, Type * type)
{
	if (type == NULL) type = VOID;
	var->type = type;
}

Var * NewVarInScope(Type * scope, Var * type)
{
	Var * var = NewCellInScope(INSTR_VAR, scope);
	VarSetType(var, type);
	return var;
}

Var * NewVarWithIndex(Var * scope, char * name, UInt16 idx, Type * type)
{
	Var * var = NewVarInScope(scope, type);
	var->name = StrAlloc(name);
	var->idx  = idx;
	return var;
}

Var * NewVar(Var * scope, char * name, Type * type)
{
	return NewVarWithIndex(scope, name, 0, type);
}

Var * NewTempVar(Type * type)
/*
Purpose:
	Alloc variable in specified scope scope.
*/
{
	TMP_IDX++;
	return NewVarWithIndex(NULL, TMP_NAME, TMP_IDX, type);
}

void CellSetLocation(Var * cell, Var * file, LineNo line_no, LinePos line_pos)
{
	cell->file    = SRC_FILE;
	cell->line_no = line_no;
	cell->line_pos = line_pos;
}

Bool VarIsTmp(Var * var)
{
	return var->name == TMP_NAME;
}

Bool VarIsUsed(Var * var)
{
	if (var == NULL) return false;
	ASSERT(var->mode == INSTR_VAR);
	return var->read > 0 || var->write > 0;
}

Bool VarIsNamed(Var * var, char * name)
{
	return var != NULL && var->mode == INSTR_VAR && StrEqual(name, var->name);
}

Var * VarFindInStruct(Var * var, char * name)
{
	Var * found;
	if (var == NULL) return NULL;
	if (VarIsNamed(var, name)) return var;
	if (var->mode == INSTR_TUPLE) {
		found = VarFindInStruct(var->l, name);
		if (found == NULL) found = VarFindInStruct(var->r, name);
		return found;
	}
	return NULL;
}

Var * VarFind(Var * scope, char * name)
/*
Purpose:
	Find variable with specified name in specified scope.
	Return NULL, if the variable does not exist.
	If scope is NULL, only global variables will be searched.
*/
{
	Var * var = NULL;
	if (scope != NULL) {

		if (scope->mode == INSTR_FN) {			
			var = VarFindInStruct(ArgType(scope->type), name);
			if (var == NULL) {
				var = VarFindInStruct(ResultType(scope->type), name);
			}
		}

		if (var == NULL) {
			FOR_EACH_LOCAL(scope, var)
				if (VarIsNamed(var, name)) return var;
			NEXT_LOCAL
		}
	}

	return var;
}

GLOBAL char VAR_NAME[128];

char * VarName(Var * var)
/*
Purpose:
	Return name of the specified variable.
	Only one name at the moment may be used.
	Next call to the function will render the name invalid.
*/
{
	VAR_NAME[0] = 0;
	if (var == NULL) return "";
	if (var->idx == 0) return var->name;
	sprintf(VAR_NAME, "%s%d", var->name, var->idx-1);
	return VAR_NAME;
}

Var * VarFindTypeVariant(char * name, TypeVariant type_variant)
{
	Var * var;
	FOR_EACH_VAR(var)
		if (var->mode == INSTR_VAR && StrEqual(name, var->name) && (type_variant == TYPE_UNDEFINED || (var->type != NULL && var->type->mode == INSTR_TYPE && var->type->variant == type_variant))) break;
	NEXT_VAR
	return var;
}

Bool VarIdentical(Var * left, Var * right)
{
	if (left == NULL || right == NULL) return false;
	if (left == right) return true;

	// Variable may be alias (i.e. may be specified by 

	if ((left->submode & (SUBMODE_IN | SUBMODE_OUT | SUBMODE_IN_SEQUENCE | SUBMODE_OUT_SEQUENCE)) != (right->submode & (SUBMODE_IN | SUBMODE_OUT | SUBMODE_IN_SEQUENCE | SUBMODE_OUT_SEQUENCE))) return false;

	while (left->adr != NULL && left->mode == INSTR_VAR && left->adr->mode == INSTR_VAR) left = left->adr;
	while (right->adr != NULL && right->mode == INSTR_VAR && right->adr->mode == INSTR_VAR) right = right->adr;

	if (left == right) return true;

	return false;
}

Bool VarIsArray(Var * var)
{
	return var->mode == INSTR_VAR && var->type != NULL && var->type->mode == INSTR_ARRAY_TYPE;
}

Bool VarIsAlias(Var * var)
{
	ASSERT(var->mode == INSTR_VAR);
	return var->adr != NULL && !CellIsConst(var->adr);
}
