/*
Type value cells

type      Pointer to type, from which this type is derived
variant   Variant of the type (TYPE_INT, TYPE_ARRAY, ...)

(c) 2012 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"

Var * VarNewType(Type * type)
{
	Var * var;
	var = NewCell(INSTR_VAR);
	var->type = TypeType(NULL);
	var->type_value = type;
	return var;
}

Bool VarIsType(Var * var)
{
	return var != NULL && (var->mode == INSTR_TYPE || var->type->variant == TYPE_TYPE);
}
