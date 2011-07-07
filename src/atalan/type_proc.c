/*

Proc Type opearations

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"

Bool VarUseReg(Var * var)
/*
Purpose:
	Return true, if this variable is register and mark it as used if it is.
*/
{
	if (var == NULL) return false;

	if (var->mode == MODE_VAR || var->mode == MODE_ARG) {
		if (FlagOn(var->submode, SUBMODE_REG)) {
			SetFlagOn(var->flags, VarUsed);
			return true;
		}
	} else if (var->mode == MODE_TUPLE) {
		return VarUseReg(var->adr) || VarUseReg(var->var);
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

	for(i = 0; i < CPU->REG_CNT; i++) {
		reg = CPU->REG[i];
		if (FlagOff(reg->flags, VarUsed)) {
			if (reg->type->range.max == 1) continue;			// exclude flag registers
			if (byte_size != VarByteSize(reg)) continue;			// exclude registers with different size
			if (FlagOn(reg->submode, SUBMODE_OUT)) continue;	// out registers can not be used to replace variables
			if (reg->var != NULL) continue;
			VarUseReg(reg);
			return reg;
		}
	}
	return NULL;
}

void RegFree(Var * reg)
{
	SetFlagOff(reg->flags, VarUsed);	
}

/*
void ProcArguments(Var * proc)
Purpose:
	For specified procedure, try to assign parameters to registers.
{
	Var * arg, * reg, * tmp;
	Instr * i;
	UInt32 var_size;

	// Procedure without body will not be processed
	if (proc->instr == NULL) return;
	
	// Input variables
	// 1. Mark all registers already used by arguments
	// TODO: We should generate temporaries & initialization for this registers

	VarResetRegUse();
	for(arg = FirstArg(proc, SUBMODE_ARG_IN); arg != NULL; arg = NextArg(proc, arg, SUBMODE_ARG_IN)) {
		VarUseReg(arg->adr);
	}

	// 2. Try to assign registers to variables

	i = proc->instr->first;		// remember current first instruction, we will insert assignments from temporaries to registers here

	for(arg = FirstArg(proc, SUBMODE_ARG_IN); arg != NULL; arg = NextArg(proc, arg, SUBMODE_ARG_IN)) {
		if (arg->adr == NULL) {
			var_size = VarByteSize(arg);
			// Try to find free register
			reg = FindUnusedReg(var_size);

			//
			if (reg == NULL && var_size == 2) {
				reg = FindUnusedReg(1);
				if (reg != NULL) {
					VarUseReg(reg);
					tmp = FindUnusedReg(1);
					if (tmp != NULL) {
						VarUseReg(tmp);
						reg = VarNewTuple(reg, tmp);
					} else {
						SetFlagOff(reg->flags, VarUsed);
						reg = NULL;
//						tmp = VarAllocScopeTmp(proc, MODE_VAR, TypeByte());		// todo: alloc proper type
					}
				}
			}

			//TODO: If appropriate register was not found, and the requested size is 2, try to use two byte registers and use tuple instead
			if (reg != NULL) {
				arg->adr = reg;
				VarUseReg(reg);
				tmp = VarAllocScopeTmp(proc, MODE_VAR, arg->type);
				ProcReplaceVar(proc, arg, tmp);
				InstrInsert(proc->instr, i, INSTR_LET, tmp, arg, NULL);
			}
		}
	}

//	printf("=========== Args ==========\n");
//	PrintProc(proc);

}
*/

static void ProcAssignRegisterArguments(Var * proc, VarSubmode submode)
/*
Purpose:
	Finalize layout of procedure type arguments.
	Try to assign registers to arguments.

	As submode, specify either SUBMODE_ARG_IN or SUBMODE_ARG_OUT.
*/
{
	Var * arg, * reg, * tmp;
	UInt32 var_size;

	// *** Register Arguments (1)
	// Try to assign as many arguments to registers as possible.
	// If argument does not fit into one register, try to use two or more smaller registers to pass it.


	// Input variables
	// 1. Mark all registers already used by arguments
	// TODO: We should generate temporaries & initialization for this registers

	VarResetRegUse();
	for(arg = FirstArg(proc, submode); arg != NULL; arg = NextArg(proc, arg, submode)) {
		VarUseReg(arg->adr);
	}

	// 2. Try to assign registers to variables

	for(arg = FirstArg(proc, submode); arg != NULL; arg = NextArg(proc, arg, submode)) {
		if (arg->adr == NULL) {
			var_size = VarByteSize(arg);
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
						reg = VarNewTuple(reg, tmp);
					} else {
						RegFree(reg); 
						reg = NULL;
					}
				}
			}
			arg->adr = reg;		// if we failed to assign, this is NULL anyways
		}
	}
}

void ProcTypeFinalize(Type * proc_type)
{
	Var * proc;
	proc = proc_type->owner;

	ProcAssignRegisterArguments(proc, SUBMODE_ARG_IN);
	ProcAssignRegisterArguments(proc, SUBMODE_ARG_OUT);
}
