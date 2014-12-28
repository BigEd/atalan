/*
Array type cells

type	  Type of array element
l         Type of index element
r         Step (for memory specific array implementations)
adr       Memory alignment

(c) 2012 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "../language.h"

Cell * NewArrayType(Type * index, Type * item)
{
	Var * var;
	var = NewCell(INSTR_ARRAY_TYPE);
	var->type = item;
	var->l    = index;
	return var;
}

Type * TypeArray(Type * index, Type * item)
{
	Type * type = NewArrayType(index, item);
//	type->step = TypeSize(element);
	return type;
}

#ifdef _DEBUG

Type * ItemType(Type * arr_type)
{
	ASSERT(arr_type->mode == INSTR_ARRAY_TYPE);
	return arr_type->type;
}

Type * IndexType(Type * arr_type)
{
	ASSERT(arr_type->mode == INSTR_ARRAY_TYPE);
	return arr_type->l;
}

Var * ArrayStep(Type * arr_type)
{
	ASSERT(arr_type->mode == INSTR_ARRAY_TYPE);
	return arr_type->r;
}

void SetArrayStep(Type * arr_type, Var * step)
{
	ASSERT(arr_type->mode == INSTR_ARRAY_TYPE);
	arr_type->r = step;
}
#endif

Var * ArrayTypeField(Var * arr_type, char * fld_name)
{
	Cell * fld = NULL;
	if (StrEqual(fld_name, "index")) {
		fld = IndexType(arr_type);
	} else if (StrEqual(fld_name, "item")) {
		fld = ItemType(arr_type);		
	} else if (StrEqual(fld_name, "step")) {
		fld = ArrayStep(arr_type);
	}
	return fld;
}
