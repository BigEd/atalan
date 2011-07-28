/*

Compiler instructions management

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php


*/

#include "language.h"

GLOBAL Var * INSTRSET;			// enumerator with instructions
extern Var * VARS;		// global variables
GLOBAL Instr NULL_INSTR;
GLOBAL Instr * InstrNull;

InstrInfo INSTR_INFO[INSTR_CNT] = {
	{ INSTR_NULL, "null", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_VOID, "void", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_LET, "<-", {TYPE_ANY, TYPE_ANY, TYPE_VOID}, 0 },

	{ INSTR_GOTO, "", {TYPE_LABEL, TYPE_ANY, TYPE_ANY}, 0 },
	{ INSTR_IFEQ, "=", {TYPE_LABEL, TYPE_ANY, TYPE_ANY}, 0 },
	{ INSTR_IFNE, "<>", {TYPE_LABEL, TYPE_ANY, TYPE_ANY}, 0 },
	{ INSTR_IFLT, "<=", {TYPE_LABEL, TYPE_ANY, TYPE_ANY}, 0 },
	{ INSTR_IFGE, ">=", {TYPE_LABEL, TYPE_ANY, TYPE_ANY}, 0 },
	{ INSTR_IFGT, ">", {TYPE_LABEL, TYPE_ANY, TYPE_ANY}, 0 },
	{ INSTR_IFLE, "<", {TYPE_LABEL, TYPE_ANY, TYPE_ANY}, 0 },
	{ INSTR_IFOVERFLOW, "over", {TYPE_LABEL, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_IFNOVERFLOW, "not over", {TYPE_LABEL, TYPE_VOID, TYPE_VOID}, 0 },

	{ INSTR_PROLOGUE, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_EPILOGUE, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_EMIT, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_VARDEF, "", {TYPE_ANY, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_LABEL, "", {TYPE_LABEL, TYPE_VOID, TYPE_VOID}, 0 },

	{ INSTR_ADD, "+", {TYPE_ANY, TYPE_ANY, TYPE_ANY}, INSTR_COMMUTATIVE },
	{ INSTR_SUB, "-", {TYPE_ANY, TYPE_ANY, TYPE_ANY}, 0 },
	{ INSTR_MUL, "*", {TYPE_ANY, TYPE_ANY, TYPE_ANY}, INSTR_COMMUTATIVE },
	{ INSTR_DIV, "/", {TYPE_ANY, TYPE_ANY, TYPE_ANY}, 0 },
	{ INSTR_SQRT, "sqrt", {TYPE_ANY, TYPE_ANY, TYPE_VOID}, 0 },
	{ INSTR_AND, "bitand", {TYPE_ANY, TYPE_ANY, TYPE_ANY}, INSTR_COMMUTATIVE },
	{ INSTR_OR, "bitor", {TYPE_ANY, TYPE_ANY, TYPE_ANY}, INSTR_COMMUTATIVE },

	{ INSTR_ALLOC, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_PROC, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_RETURN, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_ENDPROC, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_CALL, "", {TYPE_PROC, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_VAR_ARG, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_STR_ARG, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },			// generate str

	{ INSTR_DATA, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_FILE, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_ALIGN, "", {TYPE_VOID, TYPE_ANY, TYPE_VOID}, 0 },
	{ INSTR_ORG, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },				// set the destination address of compilation
	{ INSTR_HI, "hi", {TYPE_ANY, TYPE_ANY, TYPE_VOID}, 0 },
	{ INSTR_LO, "lo", {TYPE_ANY, TYPE_ANY, TYPE_VOID}, 0 },
	{ INSTR_PTR, "", {TYPE_VOID, TYPE_ANY, TYPE_VOID}, 0 },
	{ INSTR_ARRAY_INDEX, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },		// generate index for array
	{ INSTR_LET_ADR, "=@", {TYPE_ADR, TYPE_ANY, TYPE_VOID}, 0 },
	{ INSTR_ROL, "<<", {TYPE_ANY, TYPE_ANY, TYPE_ANY}, 0 },				// bitwise rotate right
	{ INSTR_ROR, ">>", {TYPE_ANY, TYPE_ANY, TYPE_ANY}, 0 },				// bitwise rotate left
	{ INSTR_DEBUG, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_MOD, "mod", {TYPE_ANY, TYPE_ANY, TYPE_ANY}, 0 },
	{ INSTR_XOR, "bitxor", {TYPE_ANY, TYPE_ANY, TYPE_ANY}, INSTR_COMMUTATIVE },
	{ INSTR_NOT, "bitnot", {TYPE_ANY, TYPE_ANY, TYPE_VOID}, 0 },
	{ INSTR_ASSERT_BEGIN, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_ASSERT, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_ASSERT_END, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },

	{ INSTR_LINE, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },				// reference line in the source code
	{ INSTR_INCLUDE, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_MULA, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },				// templates for 8 - bit multiply 
	{ INSTR_MULA16, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },           // templates for 8 - bit multiply 

	{ INSTR_COMPILER, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_CODE_END, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },			// end of BLK segment and start of data segment
	{ INSTR_DATA_END, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },			// end of data segment and start of variables segment

	{ INSTR_VAR, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },				// Variable (may be argument, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 }, input, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 }, output, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 }, ...)
	{ INSTR_CONST, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },			// Constant (depending on type)
	{ INSTR_ELEMENT, "#", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },			// <array> <index>     access array or structure element (left operand is array, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 }, right is index)
	{ INSTR_BYTE, "$", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },				// <var> <byte_index>  access byte of specified variable
	{ INSTR_RANGE, "..", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },			// x..y  (l = x, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 }, r = y) Used for slice array references
	{ INSTR_TUPLE, ",", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },			// { INSTR_LIST <adr, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },var>  (var may be another tuple)
	{ INSTR_DEREF, "@", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },			// dereference an address (var contains reference to dereferenced adr variable, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 }, type is type in [adr of type]. Byte if untyped adr is used.
	{ INSTR_FIELD, ".", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },			// access field of structure
	{ INSTR_TYPE, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_SCOPE, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },
	{ INSTR_SRC_FILE, "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0 },			//{ INSTR_SRC_FILE variable representing source file
};


/*

Instruction generating function always write to this block.
Blocks may be nested, because procedures may be nested.

*/

InstrBlock * InstrBlockAlloc()
{
	return MemAllocStruct(InstrBlock);
}


void InstrBlockFree(InstrBlock * blk)
{
	Instr * i, * next;

	if (blk != NULL) {
		for(i = blk->first; i != NULL; i = next) {
			next = i->next;
			InstrFree(i);
		}
		blk->first = blk->last = NULL;
	}
}

Bool InstrEquivalent(Instr * i, Instr * i2)
/*
Purpose:
	Return true, if two instructions are equivalent (executing them has same effect).
*/
{
	Bool eq;

	if (i == NULL || i2 == NULL) return false;
	if (i->op != i2->op) return false;
	
	eq = true;

	if (eq && INSTR_INFO[i->op].arg_type[RESULT] != TYPE_VOID) {
		eq = VarIsEqual(i->result, i2->result);
	}

	// Arguments must be same

	if (eq && INSTR_INFO[i->op].arg_type[ARG1] != TYPE_VOID) {
		eq = VarIsEqual(i->arg1, i2->arg1);
	}

	if (eq && INSTR_INFO[i->op].arg_type[ARG2] != TYPE_VOID) {
		eq = VarIsEqual(i->arg2, i2->arg2);
	}

	// Commutative instruction may be equivalent, if the arguments are switched
	if (!eq && FlagOn(INSTR_INFO[i->op].flags, INSTR_COMMUTATIVE)) {
		eq = VarIsEqual(i->arg1, i2->arg2) && VarIsEqual(i->arg2, i2->arg1);
	}
	
	return eq;
}

Var * InstrFind(char * name)
{
	Var * var;

	if (INSTRSET == NULL) {
		INSTRSET  = VarFindScope(&ROOT_PROC, "instrs", 0);
	}

	var = VarFindScope(INSTRSET, name, 0);
	return var;
}
// Code used for reading

Var * InstrFindCode(UInt16 code)
{
	Var * var;
	var = VarFindInt(INSTRSET, code);
	return var;
}

char * OpName(InstrOp op)
{
	return INSTR_INFO[op].symbol;
}

void InstrDetach(InstrBlock * blk, Instr * first, Instr * last)
{
	Instr * next;

	if (blk != NULL && first != NULL) {

		next = last->next;
		if (first->prev != NULL) {
			first->prev->next = next;
		} else {
			blk->first = next;
		}

		if (next != NULL) {
			next->prev = first->prev;
		} else {
			blk->last = first->prev;
		}

		first->prev = NULL;
		last->next  = NULL;
	}
}

void InstrAttach(InstrBlock * blk, Instr * before, Instr * first, Instr * last)
{
	if (first == NULL) return;
	if (before == NULL) {
		first->prev = blk->last;
		if (blk->last != NULL) {
			blk->last->next = first;
		}
		blk->last = last;
		if (blk->first == NULL) blk->first = first;
		last->next = NULL;
	} else {
		first->prev = before->prev;
		last->next = before;

		if (before->prev != NULL) {
			before->prev->next = first;
		} else {
			blk->first = first;
		}
		before->prev = first;
	}
}

void InstrMoveCode(InstrBlock * to, Instr * before, InstrBlock * from, Instr * first, Instr * last)
/*
Purpose:
	Move piece of code (list of instructions) from one block to another.
*/
{
	InstrDetach(from, first, last);
	InstrAttach(to, before, first, last);
}

void InstrFree(Instr * i)
{
	if (i != NULL) {
		if (i->op == INSTR_LINE) {
//			free(i->line);
		}
		free(i);
	}
}


Instr * InstrDelete(InstrBlock * blk, Instr * i)
/*
Purpose:
	Delete the instruction from specified block.
	Instruction MUST be part of the specified block.
	Return pointer to next instruction after the deleted instruction.
*/
{
	Instr * next = NULL;

	// use Detach
	if (blk != NULL && i != NULL) {

		next = i->next;

		if (i->prev != NULL) {
			i->prev->next = next;
		} else {
			blk->first = next;
		}

		if (next != NULL) {
			i->next->prev = i->prev;
		} else {
			blk->last = i->prev;
		}

		InstrFree(i);
	}
	return next;
}

void InstrInsert(InstrBlock * blk, Instr * before, InstrOp op, Var * result, Var * arg1, Var * arg2)
/*
Purpose:
	Insert instruction before specified instruction in specified block.
	If before instruction is NULL, instruction is inserted to the end of block.
*/
{
	Instr * i = MemAllocStruct(Instr);
	i->op = op;
	i->result = result;
	i->arg1 = arg1;
	i->arg2 = arg2;

	InstrAttach(blk, before, i, i);
}

InstrOp OpNot(InstrOp op)
{
	return op ^ 1;
}

InstrOp OpRelSwap(InstrOp op)
/*
Purpose:
	Change orientation of non commutative relational operators
	This is different from NOT operation.
*/{

	switch(op) {
	case INSTR_IFLE: op = INSTR_IFGE; break;
	case INSTR_IFGE: op = INSTR_IFLE; break;
	case INSTR_IFGT: op = INSTR_IFLT; break;
	case INSTR_IFLT: op = INSTR_IFGT; break;
	default: break;
	}
	return op;
}

Var * InstrEvalConst(InstrOp op, Var * arg1, Var * arg2)
/*
Purpose:
	Try to evaluate instruction.
	Instruction may be evaluated if it's arguments are constant.
Result:
	Return evaluated variable as constant or NULL, if instruction can not be evaluated.
*/
{
	Var * r = NULL;

	// Multiplication of A by 1 is same as assigning A
	if (op == INSTR_MUL) {
		if (VarIsN(arg1, 1)) return arg2;
		if (VarIsN(arg2, 1)) return arg1;
	}

	if (VarIsConst(arg1) && (arg2 == NULL || VarIsConst(arg2))) {
		
		if (arg1->type == NULL || arg1->type->variant != TYPE_INT) return NULL;

		switch(op) {
			case INSTR_SQRT:
				if (arg1->n >= 0) {
					r = VarNewInt((UInt32)sqrt(arg1->n));
				} else {
					// Error: square root of negative number
				}
				break;
			case INSTR_LO:
				r = VarNewInt(arg1->n & 0xff);
				break;
			case INSTR_HI:
				r = VarNewInt((arg1->n >> 8) & 0xff);
				break;
			case INSTR_DIV:
				r = VarNewInt(arg1->n / arg2->n);
				break;
			case INSTR_MOD:
				r = VarNewInt(arg1->n % arg2->n);
				break;
			case INSTR_MUL:
				r = VarNewInt(arg1->n * arg2->n);
				break;
			case INSTR_ADD:
				r = VarNewInt(arg1->n + arg2->n);
				break;
			case INSTR_SUB:
				r = VarNewInt(arg1->n - arg2->n);
				break;
			case INSTR_AND:
				r = VarNewInt(arg1->n & arg2->n);
				break;
			case INSTR_OR:
				r = VarNewInt(arg1->n | arg2->n);
				break;
			case INSTR_XOR:
				r = VarNewInt(arg1->n ^ arg2->n);
				break;
			default: break;
		}
	}
	return r;
}


//GLOBAL Var * G_MATCH_ARG[MACRO_ARG_CNT];



/****************************************************************

 Generating instructions

****************************************************************/


Int32 ByteMask(Int32 n)
/*
Purpose:
	Return power of $ff ($ff, $ffff, $ffffff or $ffffffff) bigger than specified number.
*/
{
	Int32 nmask;
	nmask = 0xff;
	while(n > nmask) nmask = (nmask << 8) | 0xff;
	return nmask;
}


void GenArrayInit(Var * arr, Var * init)
/*
Purpose:
	Generate array initialization loop.
Arguments:
	arr		Reference to array or array element or array slice
*/
{
	Type * type, * src_type, * idx1_type;
	Var * idx, * src_idx, * label, * range, * stop, * label_done;
	Int32 nmask;
	Int32 stop_n;
	Var * min1, * max1, * src_min, * src_max;
	Var * dst_arr;

	src_idx = NULL;
	type = arr->type;
	label = VarNewTmpLabel();
	idx1_type = NULL;

	if (type->variant == TYPE_ARRAY) {
		idx1_type = type->dim[0]	;
	}

	if (arr->mode == INSTR_ELEMENT) {

		// If this is array of array, we may need to initialize index variable differently
		
		if (arr->adr->mode == INSTR_ELEMENT) {
			idx1_type = arr->adr->type->dim[0];
		} else if (arr->adr->mode == INSTR_VAR) {
			idx1_type = arr->adr->type->dim[0];
		}

		range = arr->var;
		if (range->mode == INSTR_RANGE) {
			min1 = range->adr;
			max1 = range->var;
		} else {
			min1 = arr->var;
			max1 = VarNewInt(arr->adr->type->dim[0]->range.max);
		}
		dst_arr = arr->adr;
	} else {
		min1 = VarNewInt(type->dim[0]->range.min);
		max1 = VarNewInt(type->dim[0]->range.max);
		dst_arr = arr;
	}

	idx = VarNewTmp(0, idx1_type);

	src_type = init->type;
	
	// This is copy instruction (source is array)
	if (src_type->variant == TYPE_ARRAY) {		
		src_min = VarNewInt(src_type->dim[0]->range.min);
		src_max = VarNewInt(src_type->dim[0]->range.max + 1);
		src_idx = VarNewTmp(0, src_type->dim[0]);
		init = VarNewElement(init, src_idx);
		label_done = VarNewTmpLabel();
	}

	if (max1->mode == INSTR_CONST) {
		stop_n = max1->n;
	
		nmask = ByteMask(stop_n);
		if (nmask == stop_n) {
			stop_n = 0;
		} else {
			stop_n++;
		}
		stop = VarNewInt(stop_n);
	} else {
		stop = max1;
	}

	GenLet(idx, min1);
	if (src_idx != NULL) {
		GenLet(src_idx, src_min);
	}
	GenLabel(label);
	GenLet(VarNewElement(dst_arr, idx), init);
	if (src_idx != NULL) {
		Gen(INSTR_ADD, src_idx, src_idx, VarNewInt(1));
		Gen(INSTR_IFEQ, label_done, src_idx, src_max);
	}
	Gen(INSTR_ADD, idx, idx, VarNewInt(1));
	Gen(INSTR_IFNE, label, idx, stop);
	if (src_idx != NULL) {
		GenLabel(label_done);
	}
}

/****************************************************************

 Print tokens

****************************************************************/

void PrintVarNameNoScope(Var * var)
{
	printf("%s", var->name);
	if (var->idx > 0) {
		printf("%ld", var->idx-1);
	}
}

void PrintVarName(Var * var)
{
	UInt8 oc;

	if (VarIsReg(var)) {
		oc = PrintColor(GREEN+BLUE);
		PrintVarNameNoScope(var);
		PrintColor(oc);
	} else {
		if (var->scope != NULL && var->scope != &ROOT_PROC && var->scope->name != NULL && !VarIsLabel(var)) {
			PrintVarName(var->scope);
			printf(".");
		}
		PrintVarNameNoScope(var);
	}
}

void PrintQuotedVarName(Var * var)
{
	PrintChar('\'');
	PrintVarUser(var);
	PrintChar('\'');
}


void PrintVarVal(Var * var)
{
	Type * type;
	Var * index;
	UInt8 oc;

	if (var == NULL) return;

	if (var->mode == INSTR_DEREF) {
//	if (FlagOn(var->submode, SUBMODE_REF)) {
		printf("@");
		var = var->var;
	}

	if (var->name == NULL) {
		if (VarIsArg(var)) {
			printf("#%ld", var->idx-1);
		} else if (var->mode == INSTR_ELEMENT) {
			PrintVarVal(var->adr);
			if (var->adr->type->variant == TYPE_STRUCT) {
				printf(".");
				PrintVarNameNoScope(var->var);
			} else {
				printf("(");
				index = var->var;
				while(index->mode == INSTR_ELEMENT) {
					PrintVarVal(index->adr);
					printf(",");
					index = index->var;
				}
				PrintVarVal(index);
				printf(")");
			}
		} else if (var->mode == INSTR_BYTE) {
			PrintVarVal(var->adr);
			oc = PrintColor(GREEN+BLUE);
			Print("$");
			PrintColor(oc);
			PrintVarVal(var->var);

		} else {
			if (var->mode == INSTR_RANGE) {
				PrintVarVal(var->adr); printf(".."); PrintVarVal(var->var);
			} else if (var->mode == INSTR_TUPLE) {
				printf("(");
				PrintVarVal(var->adr);
				printf(",");
				PrintVarVal(var->var);
				printf(")");
			} else {
				switch(var->type->variant) {
				case TYPE_INT: printf("%ld", var->n); break;
				case TYPE_STRING: printf("'%s'", var->str); break;
				default: break;
				}
			}
		}
	} else {
		PrintVarName(var);

		if (var->type->variant == TYPE_LABEL) {
			if (var->instr != NULL) {
				Print(" (#");
				PrintInt(var->instr->seq_no);
				Print(")");
			}
		}

		if (var->adr != NULL) {
			if (var->adr->mode == INSTR_TUPLE) {
				if (!VarIsReg(var)) {
					printf("@");
					PrintVarVal(var->adr);
				}
			}
		}
	}

	type = var->type;
	if (type != NULL) {
		if (type->variant == TYPE_INT && !VarIsConst(var)) {
			if (type->range.min == 0 && type->range.max == 255) {
			} else {
//				printf(":%ld..%ld", type->range.min, type->range.max);
			}
		}
	}

}

void PrintVarArgs(Var * var)
{
	Var * arg;
	printf("(");
	FOR_EACH_LOCAL(var, arg)
		if (VarIsArg(arg)) {
			printf(" %s", arg->name);
		}
	NEXT_LOCAL
	printf(")");
}

void PrintVarNameUser(Var * var)
{
	Print(var->name);
	if (var->idx > 0) {
		PrintInt(var->idx-1);
	}
}

void PrintVarUser(Var * var)
{
	if (var->mode == INSTR_ELEMENT) {
		PrintVarNameUser(var->adr); Print("("); PrintVarUser(var->var); Print(")");
	} else if (var->mode == INSTR_BYTE) {
		PrintVarNameUser(var->adr);
		Print("$");
		PrintVarUser(var->var);
	} else {
		PrintVarNameUser(var);
	}
}

void PrintVar(Var * var)
{
	Type * type;

	if (var->mode == INSTR_DEREF) {
		printf("@");
		var = var->var;
	}
//	if (FlagOn(var->submode, SUBMODE_REF)) {
//		printf("@");
//	}

	if (var->mode == INSTR_ELEMENT) {
		PrintVarName(var->adr);
		Print("(");
		PrintVar(var->var);
		Print(")");
	} else if (var->mode == INSTR_BYTE) {
		PrintVarName(var->adr);
		Print("$");
		PrintVar(var->var);
	} else if (var->mode == INSTR_CONST) {
		printf("%ld", var->n);
		return;
	} else {

		PrintVarName(var);

		if (var->adr != NULL) {
			printf("@");
			PrintVarVal(var->adr);
		}

		type = var->type;
		if (type != NULL) {
			if (type->variant == TYPE_PROC) {
				printf(":proc");
				PrintVarArgs(var);
			} else if (type->variant == TYPE_MACRO) {
				printf(":macro");
				PrintVarArgs(var);
			}
		}

		if (VarIsConst(var)) {
			printf(" = %ld", var->n);
		} else {
			type = var->type;
			if (type != NULL) {
				if (type->variant == TYPE_INT) {
					if (type->range.min == 0 && type->range.min == 255) {
					} else {
						printf(":%ld..%ld", type->range.min, type->range.max);
					}
				}
			}
		}
	}
	printf("  R%ld W%ld\n", var->read, var->write);
}

void InstrPrintInline(Instr * i)
{
	Var * inop;
	Bool r = false, a1 = false;

	if (i->op == INSTR_LINE) {
		PrintColor(BLUE+LIGHT);
		printf(";%s(%d) %s", i->result->name, i->line_no, i->line);
		PrintColor(RED+GREEN+BLUE);
	} else if (i->op == INSTR_LABEL) {
		PrintVarVal(i->result);
		Print("@");
	} else {
		inop = InstrFindCode(i->op);
		printf("   %s", inop->name);
	
		if (i->result != NULL) {
			Print(" ");
			PrintVarVal(i->result);
			r = true;
		}

		if (i->arg1 != NULL) {
			if (r) {
				Print(", ");
			} else {
				Print(" ");
			}

			PrintVarVal(i->arg1);
		}

		if (i->arg2 != NULL) {
			Print(", ");
			PrintVarVal(i->arg2);
		}
	}
}

void InstrPrint(Instr * i)
{
	InstrPrintInline(i);
	printf("\n");
}


void CodePrint(InstrBlock * blk)
{
	Instr * i;
	UInt32 n;
	while (blk != NULL) {
		n = 1;
		printf("#%ld/  ", blk->seq_no);
		if (blk->label != NULL) {
			printf("    ");
			PrintVarVal(blk->label);
			printf("@");
		}
		printf("\n");
		for(i = blk->first; i != NULL; i = i->next, n++) {
			printf("%3ld| ", n);
			InstrPrint(i);
		}

		blk = blk->next;
	}
}

void PrintProc(Var * proc)
{
	CodePrint(proc->instr);
}

//$I
void InstrInit()
{
	InstrOp op;

	MemEmptyVar(NULL_INSTR);
	NULL_INSTR.op = INSTR_NULL;
	InstrNull = &NULL_INSTR;

	for(op=INSTR_NULL; op < INSTR_CNT; op++) {
		ASSERT(INSTR_INFO[op].op == op);
	}
}

Bool InstrIsSelfReferencing(Instr * i)
/*
Purpose:
	Return true, if the instruction references itself.
	Self referencing instructions are instructions like:

	add x, x, 1 
	
	I.e. instructions, where result is one of the arguments.

	We must take tuples into consideration:

	add (a,c,z,n), (a,c), 4

	is self referencing too.

	add x(a), a, 3
*/
{
	if (i == NULL || i->result == NULL) return false;
	return VarModifiesVar(i->result, i->arg1) || VarModifiesVar(i->result, i->arg2);
}
