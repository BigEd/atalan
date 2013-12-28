/*
Function type cells

type	  Type of result
l         Type of argument element

(c) 2013 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"

Cell * NewFnType(Type * arg, Type * result)
{
	Var * var;
	var = NewCell(INSTR_FN_TYPE);
	var->type = result;
	var->l    = arg;
	return var;
}

#ifdef _DEBUG

Type * ResultType(Type * fn_type)
{
	ASSERT(fn_type->mode == INSTR_FN_TYPE);
	return fn_type->type;
}

Type * ArgType(Type * fn_type)
{
	ASSERT(fn_type->mode == INSTR_FN_TYPE);
	return fn_type->l;
}

#endif