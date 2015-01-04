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

#include "../language.h"

GLOBAL UInt32 TMP_IDX;

Cell * VarAdr(Var * var)
{
	ASSERT(var != NULL);
	ASSERT(var->mode == INSTR_VAR);
	return var->_adr;
}

void VarSetAdr(Var * var, Cell * adr)
{
	ASSERT(var != NULL);
	ASSERT(var->mode == INSTR_VAR);
	var->_adr = adr;
}

Type * VarType(Var * var)
{
	ASSERT(var != NULL);
	ASSERT(var->mode == INSTR_VAR);
	return var->type;
}

void VarSetType(Var * var, Type * type)
{
	if (type == NULL) type = VOID;
	var->type = type;
}

Cell * VarSymbol(Cell * var)
{
	ASSERT(var != NULL);
	ASSERT(var->mode == INSTR_VAR);
	return var->symbol;
}

void VarSetSymbol(Cell * var, Cell * symbol)
{
	if (symbol == NULL) symbol = VOID;
	var->symbol = symbol;
}

UInt8 VarArgIdx(Cell * var)
{
	return SymbolIdx(var->symbol);
}

Var * NewVarInScope(Type * scope, Var * type)
{
	Var * var = NewCellInScope(INSTR_VAR, scope);
	VarSetType(var, type);
	return var;
}

Cell * NewVarWithSymbol(Cell * scope, Cell * symbol, Type * type)
{
	Cell * var = NewVarInScope(scope, type);
	VarSetSymbol(var, symbol);
	return var;
}

Var * NewVarWithIndex(Var * scope, char * name, UInt16 idx, Type * type)
{
	Var * var = NewVarInScope(scope, type);
	var->symbol = NewSymbolWithIndex(name, idx);
	return var;
}


Var * NewVar(Var * scope, char * name, Type * type)
{
	return NewVarWithIndex(scope, name, 0, type);
}

Bool VarIsNamed(Var * var, char * name)
{
	return var != NULL && var->mode == INSTR_VAR && SymbolIsNamed(var->symbol, name);
}


Var * NewTempVar(Type * type)
/*
Purpose:
	Alloc variable in specified scope scope.
*/
{
	Cell * symbol;
	TMP_IDX++;
	symbol = NewSymbolWithIndex(NULL, TMP_IDX);
	return NewVarWithSymbol(NULL, symbol, type);
}

Bool VarIsTmp(Var * var)
{
	return SymbolIsTmp(VarSymbol(var));
}

Bool VarIsUsed(Var * var)
{
	if (var == NULL) return false;
	ASSERT(var->mode == INSTR_VAR);
	return var->read > 0 || var->write > 0;
}

Var * VarFindInStruct(Var * var, char * name)
{
	Var * found;
	if (var == NULL) return NULL;
	if (var->mode == INSTR_VAR && VarIsNamed(var, name)) return var;
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
		} else if (scope->mode == INSTR_VAR) {
			var = VarFindInStruct(VarType(scope), name);
		}

		if (var == NULL) {
			FOR_EACH_LOCAL(scope, var)
				if (var->mode == INSTR_VAR && VarIsNamed(var, name)) return var;
			NEXT_LOCAL
		}
	}

	return var;
}

char * VarName(Var * var)
/*
Purpose:
	Return name of the specified variable.
	Only one name at the moment may be used.
	Next call to the function will render the name invalid.
*/
{
	if (var == NULL) return "";
	ASSERT(var->mode == INSTR_VAR);
	return SymbolName(VarSymbol(var));
}

Var * VarFindLabel(char * name)
{
	Var * var;
	FOR_EACH_VAR(var)
		if (var->mode == INSTR_VAR && VarIsNamed(var, name) && (var->type != NULL && var->type->mode == INSTR_TYPE && var->type->variant == TYPE_LABEL)) break;
	NEXT_VAR
	return var;
}

Bool VarIdentical(Var * left, Var * right)
{
	if (left == NULL || right == NULL) return false;
	if (left == right) return true;

	// Variable may be alias (i.e. may be specified by 

	if ((left->submode & (SUBMODE_IN | SUBMODE_OUT | SUBMODE_IN_SEQUENCE | SUBMODE_OUT_SEQUENCE)) != (right->submode & (SUBMODE_IN | SUBMODE_OUT | SUBMODE_IN_SEQUENCE | SUBMODE_OUT_SEQUENCE))) return false;

	while (left->mode == INSTR_VAR && VarAdr(left) != NULL  && VarAdr(left)->mode == INSTR_VAR) left = VarAdr(left);
	while (right->mode == INSTR_VAR && VarAdr(right) != NULL && VarAdr(right)->mode == INSTR_VAR) right = VarAdr(right);

	if (left == right) return true;

	return false;
}

Bool VarIsArray(Var * var)
{
	return var->mode == INSTR_VAR && var->type != NULL && var->type->mode == INSTR_ARRAY_TYPE;
}

Bool VarIsAlias(Var * var)
{
	return var != NULL && var->mode == INSTR_VAR && VarAdr(var) != NULL && !CellIsConst(VarAdr(var));
}

void VarLet(Var * var, Var * val)
/*
Purpose:
	Set the variable var to specified value.
	The variable must be of type INSTR_VAR and the value is set to address.
*/
{
	if (var != NULL && val != NULL) {
		ASSERT(var->mode == INSTR_VAR);

		if (val->mode == INSTR_VAR) {
			val = val->_adr;
		}

		var->_adr = val;
	}
}

Cell * VarEval(Cell * cell)
{
	Cell * r = VarAdr(cell);
	if (r == NULL) r = VarType(cell);
	return r;
}

void PrintCellNameNoScope(Var * var)
{
	Print(VarName(var));
}

void PrintIntCellName(Var * var)
{
	UInt8 oc;

	if (VarIsReg(var)) {
		oc = PrintColor(GREEN+BLUE);
		PrintCellNameNoScope(var);
		PrintColor(oc);
	} else if (VarIsArg(var)) {
		Print("%"); PrintChar(VarArgIdx(var)-1+'A');
	} else {
		if (var->scope != NULL && var->scope != &ROOT_PROC && !VarIsTmp(var->scope) && !VarIsLabel(var)) {
			PrintIntCellName(var->scope);
			Print(".");
		}
		PrintCellNameNoScope(var);
	}
}

void VarCellPrint(Cell * cell)
{
	PrintIntCellName(cell);
}

void VarCellFree(Cell * cell)
{
}
