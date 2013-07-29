/*

Translate types

(c) 2013 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

Translate variable types to CPU types.

*/

#include "language.h"

Bool IsCpuType(Type * type)
{
	return type != NULL && type->mode == INSTR_TYPE && type->scope != NULL && type->scope->type == CPU_TYPE;
}

Type * CpuType(Type * type)
/*
Purpose:
	Return CPU type that is able to represent all values of the specified type.
*/
{
	Var * var;
	if (!IsCpuType(type)) {
		FOR_EACH_LOCAL(CPU->SCOPE, var)
			if (var->mode == INSTR_VAR && var->type->mode == INSTR_TYPE) {
				if (IsSubset(type, var->type->possible_values)) return var;
			}
		NEXT_LOCAL
	}
	return type;
}

void TranslateTypes(Var * proc)
{
	Var * var;
	Type * type;
	Bool verbose;

	verbose = Verbose(proc);
	if (verbose) {
		PrintHeader(2, proc->name);
		PrintProc(proc);
	}

	FOR_EACH_LOCAL(proc, var)
		if (var->mode == INSTR_VAR) {
			if (var->write > 0 || var->read > 0) {
				type = var->type;
				var->type = CpuType(type);
				if (verbose) {
					PrintVar(var);
					PrintEOL();
				}
			}
		}
	NEXT_LOCAL
}
