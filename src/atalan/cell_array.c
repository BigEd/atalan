/*
Array value cells

Array cells represent values of type array.
The values have type describing index used to access the array and type of element.
Array is represented as set of instructions generating the array.

(c) 2012 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"

Var * NewArray(Type * type, InstrBlock * instr)
{
	Var * var;
	var = NewCell(INSTR_ARRAY);
	var->type = type;
	var->instr = instr;
	return var;
}
