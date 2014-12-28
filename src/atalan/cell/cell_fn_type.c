/*
Function type cells

type	  Type of result
l         Type of argument element

(c) 2013 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "../language.h"

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

Bool VarUseReg(Var * var)
/*
Purpose:
	Return true, if this variable is register and mark it as used if it is.
*/
{
	if (var == NULL) return false;

	if (var->mode == INSTR_VAR) {
		if (FlagOn(var->submode, SUBMODE_REG)) {
			SetFlagOn(var->flags, VarUsed);
			return true;
		}
	} else if (var->mode == INSTR_TUPLE) {
		return VarUseReg(var->l) || VarUseReg(var->r);
	}
	return false;
}

Var * RegAlloc(UInt32 byte_size)
/*
Purpose:
	Find register of specified size and mark it as used if it was found.
*/
{
	RegIdx i;
	Var * reg;
	UInt32 used_reg_size = 0xffff;	// best register size
	UInt32 reg_size;
	Var * used_reg = NULL;

	for(i = 0; i < CPU->REG_CNT; i++) {
		reg = CPU->REG[i];
		if (reg->mode == INSTR_VAR && FlagOff(reg->flags, VarUsed)) {
			if (IsEqual(reg->type->r, ONE)) continue;					// exclude flag registers
			reg_size = VarByteSize(reg);
			if (reg_size < byte_size) continue;		// exclude registers with different size
			if (OutVar(reg) || InVar(reg)) continue;			// out registers can not be used to replace variables
//			if (reg->var != NULL) continue;
			if (reg_size < used_reg_size) {
				used_reg = reg;
				used_reg_size = reg_size;
			}
		}
	}
	if (used_reg != NULL) {
		VarUseReg(used_reg);
	}
	return used_reg;
}

static void RegFree(Var * reg)
{
	SetFlagOff(reg->flags, VarUsed);	
}

static void FnAssignRegisterArguments(Cell * proc_type, Cell * args)
/*
Purpose:
	Finalize layout of procedure type arguments.
	Try to assign registers to arguments.

	As submode, specify either SUBMODE_ARG_IN or SUBMODE_ARG_OUT.
*/
{
	Var * en, * arg, * reg, * tmp;
	UInt32 var_size;

	// *** Register Arguments (1)
	// Try to assign as many arguments to registers as possible.
	// If argument does not fit into one register, try to use two or more smaller registers to pass it.


	// Input variables
	// 1. Mark all registers already used by arguments
	// TODO: We should generate temporaries & initialization for this registers

	VarResetRegUse();
	FOR_EACH_ITEM(en, arg, args)
		VarUseReg(VarAdr(arg));
	NEXT_ITEM(en, arg)

	// 2. Try to assign registers to variables

	FOR_EACH_ITEM(en, arg, args)
		if (VarAdr(arg) == NULL) {
			var_size = VarByteSize(arg);
			if (var_size != 0) {
				// Try to find free register
				reg = RegAlloc(var_size);

				//If appropriate register was not found, and the requested size is 2, 
				//try to use two byte registers and use tuple instead
				if (reg == NULL && var_size == 2) {
					reg = RegAlloc(1);
					if (reg != NULL) {
						tmp = RegAlloc(1);
						if (tmp != NULL) {
							VarUseReg(tmp);
							reg = NewTuple(reg, tmp);
						} else {
							RegFree(reg); 
							reg = NULL;
						}
					}
				}
				VarSetAdr(arg, reg);		// if we failed to assign, this is NULL anyways
			}
		}
	NEXT_ITEM(en, arg)
}

void FnTypeFinalize(Var * proc_type)
{
	FnAssignRegisterArguments(proc_type, ArgType(proc_type));
	FnAssignRegisterArguments(proc_type, ResultType(proc_type));
}


void ProcAddLocalVars(Var * proc, VarSet * set, VarFilter filter_fn)
/*
Purpose:
	Return all local variables from procedure.
	Variables from subscopes are added (for example 'for' variables).
	Variables from other procedures are not added.
*/
{
	Var * var;

	FOR_EACH_LOCAL(proc, var)
		if (var->mode == INSTR_SCOPE) {
			if (var != CPU->SCOPE) {
				ProcAddLocalVars(var, set, filter_fn);
			}
		} else {
			// Unused variables and labels are not part of result
			if ((var->write > 0 || var->read > 0)) {
				if (var->mode == INSTR_VAR) {
					if (filter_fn(var)) {
						VarSetAdd(set, var, NULL);
					}
				}
			}
		}
	NEXT_LOCAL
}

void ProcLocalVars(Var * proc, VarSet * set, VarFilter filter_fn)
{
	VarSetEmpty(set);
	ProcAddLocalVars(proc, set, filter_fn);
}
