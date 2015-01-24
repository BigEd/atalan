/*

Type operations

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"
#ifdef __Darwin__
#include "limits.h"
#endif

GLOBAL Type TLBL;
GLOBAL Var * TBYTE;		//0..255

/*
void TypeGarbageCollect()
{
	Var * var;
	Type * type;
	UInt8 i;
	TypeBlock * tb;

	// Mark all types as unused

	for(tb = &TYPES; tb != NULL; tb = tb->next) {
		for(i=0, type = tb->types; i<TYPE_BLOCK_CAPACITY; i++, type++) {
//			SetFlagOff(type->flags, TypeUsed);
		}
	}

	// Mark all types used by variables, rules and instructions

	for (var = VARS; var != NULL; var = var->next) {
		TypeMark(var->type);
	}

	RulesGarbageCollect();

}
*/

Type * TypeAlloc(TypeVariant variant)
{
	Type * type;
	type = NewCell(INSTR_TYPE);
	type->submode = 0;
	type->type = NULL;
	type->variant = variant;
	return type;
}

Type * TypeAdrOf(Type * element)
/*
Purpose:
	Alloc type as "adr of <element>".
	If the element is not specified, it is "adr of <memory>".
*/
{
	Type * type = TypeAlloc(TYPE_ADR);
	if (element == NULL) {
		if (CPU->MEMORY == NULL) {
			InitCPU();
		}
		element = CPU->MEMORY;
	}
	type->element = element;
	return type;
}

Type * TypeCopy(Type * base)
{
	Type * type = NewCell(INSTR_TYPE);
	memcpy(type, base, sizeof(Type));
	return type;
}

Type * TypeDerive(Type * base)
{
	Type * type = TypeCopy(base);
	type->type = base;
	return type;
}


void TypeAddConst(Type * type, Var * var)
/*
Purpose:
	Add specified variable as associated constant to type.
	The added variable must be of type INSTR_VAR.
*/
{
	Var * c;
	ASSERT(type->mode == INSTR_TYPE);
	ASSERT(var->mode == INSTR_VAR);
	c = var->type;
	ASSERT(c->mode == INSTR_INT);
	
	type->possible_values = Union(type->possible_values, var->type);
}


UInt32 TypeAdrSize()
{
	//TODO: should be platform defined
	return 2;
}

Var * TypeByte()
{
	return TBYTE;
}

void TypeInit()
{

	TBYTE = NewRange(ZERO, IntCellN(255));

	TLBL.mode    = INSTR_TYPE;
	TLBL.variant = TYPE_LABEL;
//	TLBL.range.min = -(long)2147483648L;
//	TLBL.range.max = 2147483647L;
	TLBL.type     = NULL;

}

Bool TypeIsInt2(Type * type)
{
	return type != NULL && (type->mode == INSTR_INT || type->mode == INSTR_RANGE || (type->mode == INSTR_TYPE && type->variant == TYPE_INT));
}

Bool TypeIsInt(Type * type)
{
	return type != NULL && type->variant == TYPE_INT;
}


void PrintTypeNoBrace(Type * type)
{
	if (type == NULL) { Print("NULL"); return; }
//	switch(type->variant) {
//	case TYPE_TUPLE:
//		PrintType(type->left); Print(" ,"); PrintType(type->right);
//		break;
//	default:
		PrintType(type);
//	}
}

void PrintType(Type * type)
{
	if (type == NULL) { Print("NULL"); return; }
	switch(type->variant) {
	case TYPE_INT:
		Print("int ");
		break;

	case TYPE_TYPE:
		Print("type ");
		break;

	case TYPE_ADR:
		Print("adr of ");
		PrintType(type->element);
		break;

		default:
		break;
	}

	if (type->possible_values != NULL) {
		PrintVar(type->possible_values);
	}
}

extern char * TMP_NAME;

void PrintVars(Var * proc)
{
	Var * var;
	Type * type;

	FOR_EACH_LOCAL(proc, var)
		if (var->mode == INSTR_SCOPE) {
			PrintVars(var);
		} else {
			if (var->mode == INSTR_VAR && VarName(var) != NULL && !VarIsTmp(var) && FlagOff(var->submode, SUBMODE_SYSTEM)) {
				type = var->type;
				if (type != NULL && type->variant == TYPE_LABEL) continue;
				PrintFmt("%s: ", VarName(var));
				PrintCell(VarType(var));
				Print("\n");
			}
		}
	NEXT_LOCAL
}


Type * FindType(Loc * loc, Var * var, Bool report_errors);

UInt16 g_fb_level;
