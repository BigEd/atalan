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
	Var * var, * cpu_type, * cpu_array, * step;
	if (!IsCpuType(type)) {

		// For array:
		//   - item must be subset of CPU type item
		//   - index must be subset of CPU type index, item must be subset of item

		if (type->mode == INSTR_ARRAY_TYPE) {
			FOR_EACH_LOCAL(CPU->SCOPE, var)
				if (var->mode == INSTR_VAR && var->type->mode == INSTR_TYPE && var->type->possible_values->mode == INSTR_ARRAY_TYPE) {
					cpu_array = var->type->possible_values;
					if (IsSubset(type, cpu_array)) {
						cpu_type = NewArrayType(IndexType(type), ItemType(cpu_array));
						step = ArrayStep(type);
						if (step == NULL) {
							step = IntCellN(TypeSize(ItemType(cpu_type)));
						}
						return cpu_type;
					}
				}
			NEXT_LOCAL

		} else {
			FOR_EACH_LOCAL(CPU->SCOPE, var)
				if (var->mode == INSTR_VAR && var->type->mode == INSTR_TYPE) {
					if (IsSubset(type, var->type->possible_values)) return var;
				}
			NEXT_LOCAL
		}
	}
	return type;
}

void TranslateScope(Var * proc, Bool verbose, Bool always)
{
	Var * var;
	Type * type;

	FOR_EACH_LOCAL(proc, var)
		if (var->mode == INSTR_VAR) {

			TranslateScope(var, verbose, always);

			if (always || VarIsUsed(var)) {
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

void TranslateTypes2(Var * proc, Bool always)
{
	Bool verbose;

	verbose = Verbose(proc);
	if (verbose) {
		PrintHeader(2, proc->name);
		PrintProc(proc);
	}

	TranslateScope(proc, verbose, always);
}

void TranslateTypes(Var * proc)
{
	TranslateTypes2(proc, false);
}
