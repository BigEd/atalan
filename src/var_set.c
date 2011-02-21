/*
 VarSet - Set of variables

This is collection of variables.

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"

void VarSetInit(VarSet * set)
{
	set->count = set->capacity = 0;
	set->arr   = NULL;
}

Var * VarSetFind(VarSet * set, Var * key)
{
	VarTuple * tuple;
	UInt16 cnt;
	for(cnt = set->count, tuple = set->arr; cnt>0; cnt--, tuple++) {
		if (tuple->key == key) return tuple->var;
	}
	return NULL;
}

void VarSetAdd(VarSet * set, Var * key, Var * var)
{
	UInt16 new_capacity;
	VarTuple * tuple;

	if (set->count == set->capacity) {
		new_capacity = set->capacity * 2;
		if (new_capacity == 0) new_capacity = 2;
		set->arr = (VarTuple *)realloc(set->arr, sizeof(VarTuple) * new_capacity);
		set->capacity = new_capacity;
	}

	tuple = &set->arr[set->count];
	tuple->key = key;
	tuple->var = var;
	set->count++;
}

