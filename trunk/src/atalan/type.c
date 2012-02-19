/*

Type operations

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/


//TODO: Detect use of uninitialized array element. (Do not report error, if there is a chance, it was initialized).

#include "language.h"
#ifdef __Darwin__
#include "limits.h"
#endif

#define TYPE_CONST_COUNT 1024

extern char * TMP_NAME;

GLOBAL Type TVOID;
GLOBAL Type TINT;		// used for int constants
GLOBAL Type TSTR;
GLOBAL Type TLBL;
GLOBAL Type TBYTE;		//0..255
GLOBAL Type TSCOPE;
//GLOBAL Type TTUPLE;
GLOBAL Type * TUNDEFINED;
GLOBAL Type * TCONST[TYPE_CONST_COUNT];		// We keep the reference to array of constant integer types 0..1023
#define RESTRICTION 0

/*
Types are allocated in blocks.
We work with types in a very dynamic way during type inference, so we need the access quick.
Mark and sweep garbage collector is implemented for types.
*/

#define TYPE_BLOCK_CAPACITY 32

typedef struct TypeBlockTag  TypeBlock;

struct TypeBlockTag {
	TypeBlock * next;
	Type        types[TYPE_BLOCK_CAPACITY];
};

GLOBAL TypeBlock TYPES;

Bool RESOLVE_SEQUENCES = true;

/*
To make type allocation fast, we keep a list of free types.
Free type has class TYPE_VOID and it's base pointer points to next free type 
or NULL if this is the last type in the list.
*/

GLOBAL Type *    FREE_TYPE;

void TypeInitBlock(TypeBlock * tb)
{
	UInt8 i;
	Type * type;

	for(i=0, type = &tb->types[TYPE_BLOCK_CAPACITY-1]; i<TYPE_BLOCK_CAPACITY; i++, type--) {
		type->variant = TYPE_VOID;
		type->base   = FREE_TYPE;
		FREE_TYPE = type;
	}
}

Type * TypeAllocMem()
{
	Type * type;
	TypeBlock * tb;
	if (FREE_TYPE == NULL) {
		tb = MemAllocStruct(TypeBlock);
		TypeInitBlock(tb);
		tb->next = TYPES.next;
		TYPES.next = tb;
	}

	type = FREE_TYPE;
	FREE_TYPE = type->base;
	type->flags = 0;
	return type;
}

void TypeMark(Type * type)
{
	if (type != NULL) {
		SetFlagOn(type->flags, TypeUsed);
		TypeMark(type->base);
		if (type->variant == TYPE_ARRAY) {
			TypeMark(type->index);
			TypeMark(type->element);
		} else if (type->variant == TYPE_VARIANT || type->variant == TYPE_TUPLE) {
			TypeMark(type->left);
			TypeMark(type->right);
		}
	}
}

void TypeGarbageCollect()
{
	Var * var;
	Type * type;
	UInt8 i;
	TypeBlock * tb;

	// Mark all types as unused

	for(tb = &TYPES; tb != NULL; tb = tb->next) {
		for(i=0, type = tb->types; i<TYPE_BLOCK_CAPACITY; i++, type++) {
			SetFlagOff(type->flags, TypeUsed);
		}
	}

	// Mark all types used by variables, rules and instructions

	for (var = VARS; var != NULL; var = var->next) {
		TypeMark(var->type);
	}

	RulesGarbageCollect();

}

Type * TypeAlloc(TypeVariant variant)
{
	Type * type;
	type = TypeAllocMem();

	type->variant = variant;
	type->owner   = SCOPE;
	type->flexible = false;
	type->base = NULL;
	return type;
}

static IntLimit LIM_MIN = INTLIMIT_MIN;
static IntLimit LIM_MAX = INTLIMIT_MAX;

IntLimit * TypeMin(Type * type)
{
	if (type != NULL) {
		if (type->variant == TYPE_INT) {
			return &type->range.min;
		} else if (type->variant == TYPE_SEQUENCE) {
			switch(type->seq.op) {
			case INSTR_ADD:
			case INSTR_MUL:
				return TypeMin(type->seq.init);
			default:
				break;
			}
		}
	}
	return &LIM_MIN;
}

IntLimit * TypeMax(Type * type)
{
	if (type != NULL) {
		if (type->variant == TYPE_INT) {
			return &type->range.max;
		} else if (type->variant == TYPE_SEQUENCE) {
			switch(type->seq.op) {
			case INSTR_SUB:
			case INSTR_DIV:
			case INSTR_MOD:
				return TypeMax(type->seq.init);
			default:
				break;
			}

		}
	}
	return &LIM_MAX;
}


/*

Integer types.

*/

Type * TypeAllocConst(IntLimit n)
/*
Purpose:
	Alloc integer type for constant.
*/
{
	Type * type = NULL;
	if (n>=0 && n<TYPE_CONST_COUNT) {
		type = TCONST[n];
	}
	if (type == NULL) {
		type = TypeAlloc(TYPE_INT);
		type->range.flexible = false;
		type->range.min = type->range.max = n;
		if (n>=0 && n<TYPE_CONST_COUNT) {
			TCONST[n] = type;
		}
	}
	return type;
}

Type * TypeAllocInt(IntLimit min, IntLimit max)
{
	Type * type = TypeAlloc(TYPE_INT);
	type->range.flexible = false;
	type->range.min = min;
	type->range.max = max;
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
	Type * type = TypeAllocMem();
	memcpy(type, base, sizeof(Type));
	return type;
}

Type * TypeDerive(Type * base)
{
	Type * type = TypeCopy(base);
	type->base = base;
	type->owner   = SCOPE;
	return type;
}
/*
void TypeLet(Type * type, Var * var)
{
	Type * vtype;
	Var * arr;
	Int32 min, max;

	vtype = var->type;

	if (vtype == NULL) return;

	// Assigning array element sets the type to the type of the array element

	if (var->mode == INSTR_ELEMENT) {
		arr = var->adr;
		if (arr->type->variant == TYPE_ARRAY) {
			vtype = arr->type->element;
		} else {
			// Element variables on non-array variables are byte access to variable elements
			vtype = TypeByte();
		}
	}

	switch(vtype->variant)
	{
	case TYPE_INT:

		// When setting constant N to the variable, it's range is set to N..N
		// Variables with type like this are in fact constants.
		if (var->mode == INSTR_CONST) {
			min = max = var->n;
		} else {
			min = vtype->range.min;
			max = vtype->range.max;
		}
	
		if (type->variant == TYPE_UNDEFINED) {
			type->base = vtype;
			type->variant = TYPE_INT;
			type->range.min = min;
			type->range.max = max;
			type->owner = NULL;
		} else if (type->variant == TYPE_INT) {
			if (min < type->range.min) type->range.min = min;
			if (max > type->range.max) type->range.max = max;

		}
		break;
	default:
		break;
	}
}
*/

void TAdd(Int32 * x, Int32 tr) { *x = *x + tr; }
void TSub(Int32 * x, Int32 tr) { *x = *x - tr; }
void TMul(Int32 * x, Int32 tr) { *x = *x * tr; }
void TDiv(Int32 * x, Int32 tr) 
{ 
	// We may divide by zero.
	// In such case, no type transformation is performed
	if (tr != 0) {
		*x = *x / tr; 
	}
}

void TMod(Int32 * x, Int32 tr)  
{ 
	// For types like 0..255 etc., tr can be zero.
	// In such case, we do not modify the type.
	if (tr != 0) {
		*x = *x % tr;
	}
}

void TAnd(Int32 * x, Int32 tr) { *x = *x & tr; }
void TOr(Int32 * x, Int32 tr)  { *x = *x | tr; }
void TXor(Int32 * x, Int32 tr)  { *x = *x ^ tr; }

RangeTransform InstrFn(InstrOp op)
{
	RangeTransform r = NULL;
	switch(op) {
		case INSTR_DIV: r = &TDiv; break;
		case INSTR_MOD: r = &TMod; break;
		case INSTR_MUL: r = &TMul; break;
		case INSTR_ADD: r = &TAdd; break;
		case INSTR_SUB: r = &TSub; break;
		case INSTR_AND: r = &TAnd; break;
		case INSTR_OR:  r = &TOr; break;
		case INSTR_XOR: r = &TXor; break;
		default: break;
	}
	return r;
}
/*
void TypeTransform(Type * type, Var * var, InstrOp op)
{
	Type * vtype;
	Var * arr;
	RangeTransform r_fn;
	Int32 min, max, t;

	vtype = var->type;
	if (vtype == NULL) return;

	if (var->mode == INSTR_ELEMENT) {
		arr = var->adr;
		if (arr->type->variant == TYPE_ARRAY) {
			vtype = arr->type;
		} else {
			// Element variables on non-array variables are byte access to variable elements
			vtype = TypeByte();
		}
	}

	switch(type->variant) 
	{
	case TYPE_INT:

		r_fn = InstrFn(op);
		if (r_fn == NULL) {
			SyntaxError("type transformation function not found");
			return;
		}

		if (vtype->variant == TYPE_INT) {
			if (var->mode == INSTR_CONST) {
				min = max = var->n;
			} else {
				min = vtype->range.min;
				max = vtype->range.max;
				if (op == INSTR_DIV || op == INSTR_MOD || op == INSTR_SUB) {
					t = min;
					min = max;
					max = t;
				}
			}
			r_fn(&type->range.min, min);
			r_fn(&type->range.max, max);
			
		}
		break;
	default:
		break;
	}
}
*/
/*
UInt16 TypeItemCount(Type * type)
{
	UInt16 cnt;
	Type * idx;
	if (type->variant != TYPE_ARRAY) return 1;
	idx = type->dim[0];
	cnt = idx->range.max - idx->range.min + 1;
	return cnt;
}
*/

void TypeAddConst(Type * type, Var * var)
/*
Purpose:
	Add specified variable as associated constant to type.
*/
{

	if (type->variant == TYPE_INT) {
		// Register type of this constant as specified type
		var->type  = type;
		var->scope = var->type->owner;

		if (!var->value_nonempty) {
			if (type->range.flexible) {
				type->range.max++;
				var->n = type->range.max;
				var->value_nonempty = true;
			} else {
				SyntaxError("it is necessary to define constant value explicitly for this type");
			}
		} else {
			if (var->n < type->range.min) {
				if (type->range.flexible) {
					type->range.min = var->n;
				} else {
					SyntaxError("constant out of available range");
				}
			}
			if (var->n > type->range.max) {
				if (type->range.flexible) {
					type->range.max = var->n;
				} else {
					SyntaxError("constant out of available range");
				}
			}
		}
	}
}

void TypeLimits(Type * type, Var ** p_min, Var ** p_max)
/*
Purpose:
	Return integer type limits as two variables.
*/
{
	Var * min, * max;
	min = VarNewInt(type->range.min);
	max = VarNewInt(type->range.max);
	*p_min = min;
	*p_max = max;
}

Bool TypeIsSubsetOf(Type * type, Type * master)
/*
Purpose:
	Return true, if first type is subset of second type.
*/
{
	if (type == master) return true;
	if (type == NULL || master == NULL) return false;
	if (master->variant == TYPE_VARIANT) {
		return TypeIsSubsetOf(type, master->left) || TypeIsSubsetOf(type, master->right);
	} 
	if (type->variant != master->variant) return false;
	if (type->variant == TYPE_INT) {
		if (type->range.max > master->range.max) return false;
		if (type->range.min < master->range.min) return false;
	} else if (type->variant == TYPE_ARRAY) {
		return TypeIsSubsetOf(type->index, master->index) && TypeIsSubsetOf(type->element, master->element);
	}
	return true;
}

Bool TypeEqual(Type * left, Type * right)
{
	return TypeIsSubsetOf(left, right) && TypeIsSubsetOf(right, left);
}

UInt32 TypeAdrSize()
{
	//TODO: should be platform defined
	return 2;
}

Type * TypeByte()
{
	return &TBYTE;
}

Type * TypeLongInt()
{
	return &TINT;
}

Type * TypeScope()
{
	return &TSCOPE;
}
/*
Type * TypeTuple()
{
	return &TTUPLE;
}
*/

Type * TypeTuple(Type * left, Type * right)
{
	Type * type = TypeAlloc(TYPE_TUPLE);
	type->left  = left;
	type->right = right;
	return type;
}

Type * TypeArray(Type * index, Type * element)
{
	Type * type = TypeAlloc(TYPE_ARRAY);
	type->index = index;
	type->element = element;
	return type;
}

Var * NextItem(Var * scope, Var * arg, VarSubmode submode)
{
	Var * var = arg->next;
	while(var != NULL && (var->mode != INSTR_VAR || var->scope != scope || (submode != 0 && FlagOff(var->submode, submode)))) var = var->next;
	return var;
}

Var * FirstItem(Var * scope, VarSubmode submode)
{
	return NextItem(scope, scope, submode);
}

UInt32 TypeStructSize(Var * var);

UInt8 ConstByteSize(Int32 n)
{
	UInt8 size;
	if (n <= 255) size = 1;
	else if (n <= 65535) size = 2;
	else if (n <= 0xffffff) size = 3;
	else size = 4;		// we currently do not support bigger numbers than 4 byte integers
	return size;
}

UInt32 ArrayIndexCount(Type * index)
{
	if (index == 0) return 0;
	if (index->variant == TYPE_INT) {
		return index->range.max - index->range.min + 1;
	} else if (index->variant == TYPE_TUPLE) {
		return ArrayIndexCount(index->left) * ArrayIndexCount(index->right);
	}
	return 0;
}

UInt32 TypeSize(Type * type)
{
	UInt32 size;
	Int32 lrange;
	size = 0;
	if (type != NULL) {
		switch(type->variant) {
		case TYPE_INT:

			lrange = type->range.min;
			if (lrange > 0) lrange = 0;

			size = ConstByteSize(type->range.max - lrange);
			
			break;

		case TYPE_ADR:
			size = TypeAdrSize();
			break;

		case TYPE_STRUCT:
			size = TypeStructSize(type->owner);
			break;

		case TYPE_ARRAY:
			size = TypeSize(type->element) * ArrayIndexCount(type->index);
			break;
		default: break;
		}
	}
	return size;
}

UInt32 TypeStructSize(Var * var)
/*
Purpose:
	Compute size of structure variable in bytes.
*/
{
	UInt32 size = 0;
	Var * item;
	item = FirstItem(var, 0);
	while(item != NULL) {
		if (item->mode == INSTR_VAR) {
			size += TypeSize(item->type);
		}
		item = NextItem(var, item, 0);
	}
	return size;
}

UInt32 TypeStructAssignOffsets(Type * type)
/*
Purpose:
	Assign offsets to elements of structure.
*/
{
	UInt32 offset = 0;
	Var * item;
	item = FirstItem(type->owner, 0);
	while(item != NULL) {
		if (item->mode == INSTR_VAR) {
			if (item->adr == NULL) {
				item->adr = VarNewInt(offset);
				offset += TypeSize(item->type);
			}
		}
		item = NextItem(type->owner, item, 0);
	}
	return offset;			// offset now contains total size of structure
}

void TypeInit()
{

	FREE_TYPE = NULL;
	TypeInitBlock(&TYPES);


	TUNDEFINED = TypeAllocMem();
	TUNDEFINED->variant = TYPE_UNDEFINED;

	TINT.variant = TYPE_INT;
	TINT.range.min = -(long)2147483648L;
	TINT.range.max = 2147483647L;
	TINT.base      = NULL;
	TINT.owner   = NULL;

	TBYTE.variant = TYPE_INT;
	TBYTE.range.min = 0;
	TBYTE.range.max = 255;
	TBYTE.base      = NULL;
	TBYTE.owner     = NULL;

	TSTR.variant = TYPE_STRING;
	TSTR.base      = NULL;
	TSTR.owner     = NULL;

	TLBL.variant = TYPE_LABEL;
	TLBL.range.min = -(long)2147483648L;
	TLBL.range.max = 2147483647L;
	TLBL.base      = NULL;
	TLBL.owner     = NULL;

	TSCOPE.variant = TYPE_SCOPE;
	TSCOPE.base    = NULL;
	TSCOPE.owner   = NULL;

}

Bool TypeIsInt(Type * type)
{
	return type != NULL && type->variant == TYPE_INT;
}

Bool TypeIsIntConst(Type * type)
{
	if (type == NULL) return false;
	if (type->variant != TYPE_INT) return false;
	return type->range.min == type->range.max;
}

//TODO: Merge the two functions
//      Difference betwen TYPE_PROC handling

Bool VarMatchType(Var * var, Type * type)
/*
Purpose:
	Return 0, if the variable matches the type.
*/
{
	Type * vtype;
	Type * rtype;
	Var * result;

	if (var == NULL) return false;
	if (type == NULL) return true;
	if (type->variant == TYPE_UNDEFINED) return true;

	rtype = type;
	vtype = var->type;

	if (vtype->variant == TYPE_UNDEFINED) return true;
	if (vtype == NULL) return true;		// variable with no type specified matches everything

	if (vtype->variant == TYPE_PROC) {
		// Find the result of the function
	
		result = FirstArg(var, SUBMODE_ARG_OUT);

		if (result == NULL) return false;		// this is function with no argument

		rtype = result->type;
	}

	// Integer type
	if (type->variant == TYPE_INT) {
		if (vtype->variant != TYPE_INT) return false;
		if (var->mode == INSTR_CONST) {
			if (var->n < type->range.min) return false;
			if (var->n > type->range.max) return false;
		} else if (var->mode == INSTR_VAR || var->mode == INSTR_ELEMENT) {
			if (vtype->range.min < type->range.min) return false;
			if (vtype->range.max > type->range.max) return false;
		} else {
			return false;
		}
	}

	return true;
}

Bool VarMatchesType(Var * var, Type * type)
/*
Purpose:
	Test, if the variable matches the type.
	This is pattern matcher, so type will not match, if it's byte count does not match.
*/
{
	Type * vtype = var->type;

	if (type == vtype) return true;
	// If pattern has no defined type, it fits
	if (type == NULL) return true;

//	if (vtype->variant == TYPE_UNDEFINED) return true;		// anything matches undefined type

	switch(type->variant) {
	case TYPE_UNDEFINED:
		return false;

	case TYPE_VARIANT:
		return VarMatchesType(var, type->left) || VarMatchesType(var, type->right);

	case TYPE_INT:
		if (var->mode == INSTR_TUPLE) {
			return false;
		}

		if (vtype != NULL) {
			// If variable is constant, the check is different
			if (var->mode == INSTR_CONST) {
				if (vtype->variant != TYPE_INT) return false;
				if (var->n < type->range.min) return false;
				if (var->n > type->range.max) return false;
			} else if (var->mode == INSTR_ELEMENT) {
				// Specified variable is element, but the type is not array
				if (type->variant != TYPE_ARRAY) return false;
			} else {
				if (vtype->variant != TYPE_INT) return false;
				if (type->range.max < vtype->range.max) return false;
				if (type->range.min > vtype->range.min) return false;

				// If the size of matched type is 
				if (TypeSize(type) > TypeSize(vtype)) {
					return false;
				}
			}
		}
		break;

	case TYPE_ARRAY:
		if (vtype == NULL) return false;
		if (vtype->variant != TYPE_ARRAY) return false;

		// Match first index, second index, return type

		return TypeIsSubsetOf(vtype->index, type->index)
//			&& TypeIsSubsetOf(vtype->dim[1], type->dim[1])
			&& TypeIsSubsetOf(vtype->element, type->element);
		
	case TYPE_ADR:
		return vtype != NULL && vtype->variant == TYPE_ADR;

	case TYPE_PROC:

		if (vtype->variant != TYPE_PROC) return false;

		// Interrupt routines types will be based on some other type
		if (vtype->base == NULL && type->base == NULL) return true;

		// Procedure type is only same, if it is exactly the same
		return false;

	case TYPE_STRUCT:
		if (vtype != NULL) {
			if (vtype->variant != TYPE_STRUCT) return false;
		}
		default:
		break;
	}

	return true;
}

void PrintTypeNoBrace(Type * type)
{
	if (type == NULL) { Print("NULL"); return; }
	switch(type->variant) {
	case TYPE_TUPLE:
		PrintType(type->left); Print(" ,"); PrintType(type->right);
		break;
	default:
		PrintType(type);
	}
}

void PrintType(Type * type)
{
	if (type == NULL) { Print("NULL"); return; }
	switch(type->variant) {
	case TYPE_INT:		
		PrintInt(type->range.min); Print(".."); PrintInt(type->range.max);
		break;
	case TYPE_SEQUENCE:
		Print("seq "); PrintType(type->seq.init); Print(" + "); PrintType(type->seq.step);
		break;

	case TYPE_ADR:
		Print("adr of ");
		PrintType(type->element);
		break;

	case TYPE_ARRAY:
		Print("array (");
		PrintTypeNoBrace(type->index);
		Print(") of ");
		PrintType(type->element);
		break;

	case TYPE_PROC:
		Print("proc");
		break;

	case TYPE_VARIANT:
		PrintType(type->left); Print(" or "); PrintType(type->right);
		break;

	case TYPE_TUPLE:
		Print("("); PrintTypeNoBrace(type); Print(")");
		break;

	case TYPE_MACRO:
		Print("proc");
		break;
		default:
		break;
	}
}

void PrintVars(Var * proc)
{
	Var * var;
	Type * type;

	FOR_EACH_LOCAL(proc, var)
		if (var->mode == INSTR_SCOPE) {
			PrintVars(var);
		} else {
			if (var->name != NULL && var->name != TMP_NAME && FlagOff(var->submode, SUBMODE_SYSTEM) && var->mode == INSTR_VAR) {
				type = var->type;
				if (type != NULL && type->variant == TYPE_LABEL) continue;
				PrintFmt("%s: ", var->name);
				PrintType(var->type);
				Print("\n");
			}
		}
	NEXT_LOCAL
}

Type * IntTypeEval(InstrOp op, Type * left, Type * right)
{
	RangeTransform r_fn;
	Int32 min, max, t;
	Type * rt = NULL;

	// When assigning the value, resulting type is simply the same as original value
	switch(op) {

	case INSTR_LO:
	case INSTR_HI:
		rt = TypeByte();		//TypeAllocInt(0, 255);
		break;

	default:
		if (right != NULL && right->variant == TYPE_INT) {
			r_fn = InstrFn(op);
			if (r_fn != NULL) {
				rt = TypeAllocInt(left->range.min, left->range.max);
				min = right->range.min;
				max = right->range.max;
				if (op == INSTR_DIV || op == INSTR_MOD || op == INSTR_SUB) {
					t = min;
					min = max;
					max = t;
				}
				r_fn(&rt->range.min, min);
				r_fn(&rt->range.max, max);
			}
		}
	}
	return rt;
}

Type * SeqTypeEval(InstrOp op, Type * left, Type * right)
{
	Type * rt = NULL;
	RangeTransform r_fn;

	if (right->variant == TYPE_INT) {
		switch(op) {
		case INSTR_SUB:
		case INSTR_ADD:
			if (left->seq.init != NULL) {
				rt = TypeAlloc(TYPE_SEQUENCE);
				rt->seq.compare_op = left->seq.compare_op;
				rt->seq.op         = left->seq.op;
				rt->seq.step       = left->seq.step;
				rt->seq.limit      = left->seq.limit;
			
				r_fn = InstrFn(op);
			
				rt->seq.init       = IntTypeEval(op, left->seq.init, right);
			}
			break;
		}
	}
	//TODO: Only if there is same operation and step
	return rt;
}


Type * AdrTypeEval(InstrOp op, Type * left, Type * right)
{
	Type * rt = NULL;

	switch(op) {

	case INSTR_LO:
	case INSTR_HI:
		rt = TypeByte();
		break;
		default:
		break;
	}
	return rt;
}

Type * TypeEval(InstrOp op, Type * left, Type * right)
{
	Type * rt = NULL;

	if (left != NULL) {

		if (op == INSTR_LET_ADR) {
			rt = TypeAdrOf(left);
		} else if (op == INSTR_LET) {
			rt = left;
		} else {

			switch(left->variant) {
			case TYPE_INT:
				rt = IntTypeEval(op, left, right);
				break;
			case TYPE_SEQUENCE:
				rt = SeqTypeEval(op, left, right);
				break;
			case TYPE_ADR:
				rt = AdrTypeEval(op, left, right);
				break;
			default:
				ASSERT("Unknown type eval.");
			}
		}
	}

	return rt;
}

#define TYPE_IS_UNDEFINED(t)  (t == NULL)

Bool  SequenceRange(Type * type, IntLimit * min, IntLimit * max)
{
	IntLimit init, step, limit, max_over, step_min;
	InstrOp op;

	if (type != NULL && type->variant == TYPE_SEQUENCE) {
		if (TypeIsInt(type->seq.step) && TypeIsInt(type->seq.init) && TypeIsInt(type->seq.limit)) {
			init = type->seq.init->range.min;
			step = type->seq.step->range.max;
			step_min = type->seq.step->range.min;
			limit = type->seq.limit->range.max;

			if (type->seq.op == INSTR_ADD) {

				max_over = (limit - init) % step;
				op = type->seq.compare_op;

				// If the comparison is exact, we know the sequence stops even in case of IFNE
				// In other case, the sequence is infinite, as the condition does not 'catch' any value of sequence
				if (op == INSTR_IFNE && TypeIsIntConst(type->seq.step) && TypeIsIntConst(type->seq.init) && TypeIsIntConst(type->seq.limit) && max_over == 0) {
					limit -= step;  // compared value will not be part of the sequence
				} else if (op == INSTR_IFLE) {
				} else if (op == INSTR_IFLT) {

					// We only make the limit smaller, if the step is constant
					if (step == step_min) {
						limit = ((limit - init - 1) / step_min) * step_min + init;
						max_over = 0;
					}
				} else {
					goto done;
				}

				*min = init;
				*max = limit + max_over;
				return true;
			}
		}
	}
done:
	return false;
}

Type * ResolveSequence(Type * type)
/*
Purpose:
	Try to combine sequence type into integer type.
*/
{
	IntLimit min, max;

	if (SequenceRange(type, &min, &max)) {
		return TypeAllocInt(min, max);
	}
	return NULL;
}

Type * TypeUnion(Type * left, Type * right)
/*
Purpose:
	Combine types so, that resulting type may contain values from left or from the right type.
	This may be used when a variable is initialized in multiple code branches.
*/
{
	Type * type = NULL, * t;
	Int32 min, max;

	if (left == right) return left;

	// Make sure, that if there is NON-NULL type, it is the left one
	if (left == NULL) {
		left = right;
		right = NULL;
	}

	// In case there is only one type or no type, return the one type or NULL
	if (right == NULL) return left;

	// If any of the types is undefined, result is undefined
	if (left->variant == TYPE_UNDEFINED) return left;
	if (right->variant == TYPE_UNDEFINED) return right;

	// If one of the types is sequence, make sure it is the left one
	if (right->variant == TYPE_SEQUENCE) {
		t = left;
		left = right;
		right = t;
	}

	if (left->variant == TYPE_SEQUENCE) {

		// Sequence & sequence
		if (right->variant == TYPE_SEQUENCE) {

			// same operator
			if (right->seq.op == left->seq.op) {
				type = TypeAlloc(TYPE_SEQUENCE);
				type->seq.op = right->seq.op;
				type->seq.init = TypeUnion(left->seq.init, right->seq.init);
				type->seq.step = TypeUnion(left->seq.step, right->seq.step);
				return type;
			}

		// Step is on the left side

		} else if (right->variant == TYPE_INT) {
			if (left->seq.init == NULL) {
				type = TypeCopy(left);
				type->seq.init = right;
				return type;
				//return ResolveSequence(type);
			}

			type = ResolveSequence(left);
			if (type != NULL) {
				return TypeUnion(type, right);
			}
/*
			if (left->seq.op == INSTR_ADD && left->seq.step->range.min >=0) {
				type = TypeAlloc(TYPE_INT);
				type->range.max = INT_MAX;
				type->range.min = right->range.min;
				return type;
			}
*/
		}
	}

	if (right == NULL) return right;

	switch (left->variant) {
	case TYPE_INT:
		if (right->variant == TYPE_INT) {
			if (left->range.max >= right->range.min || right->range.max >= left->range.min) {
				min =  left->range.min; if (right->range.min < min) min = right->range.min;
				max = right->range.max; if ( left->range.max > max) max =  left->range.max;
				if (left->range.min == min && left->range.max == max) {
					type = left;
				} else if (left->range.min == min && left->range.max == max) {
					type = right;
				} else {
					type = TypeAllocInt(min, max);
				}
			}
		}
		break;
	case TYPE_VARIANT:
		if (TypeIsSubsetOf(right, left->left) || TypeIsSubsetOf(right, left->right)) return left;
		break;
	default:
		break;
	}

	// Default case is VARIANT type
	if (type == NULL) {
		type = TypeAlloc(TYPE_VARIANT);
		type->left  = left;
		type->right = right;
	}
	return type;
}

Type * TypeExpandRange(Type * type, IntLimit min, IntLimit max)
{
	if (type->range.min < min) min = type->range.min;
	if (type->range.max > max) max = type->range.max;

	if (min != type->range.min || max != type->range.max) {		
		type = TypeAllocInt(min, max);
	}
	return type;
}

Type * TypeExpand(Type * left, Type * right)
/*
Purpose:
	Combine types so, that resulting type may contain values from left or from the right type.
	This may be used when a variable is initialized in multiple code branches.
*/
{
	Type * type = NULL;
	Int32 min, max;

	if (left == NULL) return right;
	if (right == NULL) return left;

	if (left->variant == TYPE_UNDEFINED) return right;
	if (right->variant == TYPE_UNDEFINED) return left;

	switch (left->variant) {
	case TYPE_INT:
		if (right->variant == TYPE_INT) {
			type = TypeExpandRange(left, right->range.min, right->range.max);
//			min = left->range.min; 			
//			if (right->range.min < min) min = right->range.min;
//			max = right->range.max;
//			if (left->range.max > max) max = left->range.max;

//			if (min != left->range.min || max != left->range.max) {		
//				type = TypeAllocInt(min, max);
//			}
		} else if (right->variant == TYPE_VARIANT) {
			type = TypeExpand(left, right->left);
			type = TypeExpand(type, right->right);
		} else if (right->variant == TYPE_SEQUENCE) {
			if (SequenceRange(right, &min, &max)) {
				type = TypeExpandRange(left, min, max);
			}
		}
		break;
		default:
		break;
	}

	if (type == NULL) type = left;

	return type;
}


//$R

Type * TypeRestrictRange(Type * type, IntLimit rmin, IntLimit rmax)
{
	Type * r = type;
	IntLimit min, max;

	switch(type->variant) {
	case TYPE_INT:
		min = type->range.min;
		max = type->range.max;

		// <    > type range
		// (    ) restriction range
		//  ----  resulting type range

		// 1.  (   <    >     )      => void type
		if (rmin <= min && rmax >= max) {
			r = &TVOID;
		// 1. <---(    )--->
		} else if (min < rmin && rmax < max) {
			r = TypeUnion(TypeAllocInt(min, rmin-1), TypeAllocInt(rmax+1, max));
		// 2. <------>  (     )
		} else if (max < rmin) {
			r = type;
		// 4.  (      )  <----->
		} else if (rmax < min) {
			r = type;
		// 3.  <---(     >   )
		} else if (min < rmin) {
			r = TypeAllocInt(min, rmin-1);
		// 5.  (     <  )----->
		} else if (rmax >= min) {
			r = TypeAllocInt(rmax+1, max);
		}
		break;

	case TYPE_TUPLE:
		r = TypeUnion(TypeRestrictRange(type->left, rmin, rmax), TypeRestrictRange(type->right, rmin, rmax));
		break;
	default:
		break;
	}

	return r;
}

Type * TypeRestrict(Type * type, Type * restriction)
{
	Type * r = type;

	if (restriction == NULL || restriction->variant == TYPE_UNDEFINED) return r;

	switch(restriction->variant) {
	case TYPE_INT:
		r = TypeRestrictRange(type, restriction->range.min, restriction->range.max);
		break;
	case TYPE_TUPLE:
		r = TypeUnion(TypeRestrict(type, restriction->left), TypeRestrict(type, restriction->right));
		break;
	default:
		break;
	}
	return r;
}

Type * TypeRestrictOp(Type * type, Type * restriction, InstrOp op)
{
	Type * rt, * left, * right;
	IntLimit min, max, init, step;

	// If no restriction is defined, the type is returned unrestricted
	if (restriction == NULL || restriction->variant == TYPE_UNDEFINED) return type;

	rt = type;

	// For variant type, we apply both variants as restrictions and use union of them.
	if (restriction->variant == TYPE_VARIANT) {
		left = TypeRestrictOp(rt, restriction->left, op);
		right = TypeRestrictOp(rt, restriction->right, op);
		rt = TypeUnion(left, right);
		goto done;
	}

	// Compute minimal and maximal value defined by restriction.
	// If the value is not known, extreme value is returned.
	IntSet(&min, TypeMin(restriction));
	IntSet(&max, TypeMax(restriction));
/*
	if (restriction->variant == TYPE_INT) {
		IntSet(&min, TypeMin(restriction));
		IntSet(&max, TypeMax(restriction));
	} else if (restriction->variant == TYPE_SEQUENCE) {
		switch(restriction->seq.op) {
		case INSTR_SUB:
		case INSTR_DIV:
		case INSTR_MOD:
			IntSet(&max, TypeMax(restriction->seq.init));
			break;
		case INSTR_ADD:
		case INSTR_MUL:
			IntSet(&min, TypeMin(restriction->seq.init));
			break;
		}
	} else {
		goto done;
	}
*/
	if (type != NULL) {
		if (type->variant == TYPE_SEQUENCE) {
			switch(op) {
			case INSTR_IFNE:
//				if (TypeIsIntConst(type->seq.step) && TypeIsIntConst(type->seq.init)) {
//					init = type->seq.init->range.min;
//					step = type->seq.step->range.max;
//					if (type->seq.op == INSTR_ADD) {
//						rt = TypeAllocInt(init, max-1);
//					}
//				} else {
					if (type->seq.compare_op != op) {
						type->seq.compare_op = op;
						type->seq.limit = TypeAllocInt(max, max);
						type = ResolveSequence(type);			// type =
					}
//				}
				break;
			case INSTR_IFLT:
				max--;
				//continue to IFLE
			case INSTR_IFLE:
				if (TypeIsInt(type->seq.step) && TypeIsInt(type->seq.init)) {
					init = type->seq.init->range.min;
					step = type->seq.step->range.max;			// maximal step
					if (type->seq.op == INSTR_ADD) {
						rt = TypeAllocInt(init, max + step);	// we may overstep maximal value by step
					}
				} else {
					if (type->seq.compare_op != op) {
						type->seq.compare_op = op;
						type->seq.limit = TypeAllocInt(max, max);
					}
				}
				break;
			case INSTR_IFGT:
				break;

			case INSTR_IFGE:
				break;

			case INSTR_IFTYPE:
			case INSTR_IFNTYPE:
				break;
			default:
				ASSERT(false);	// unknown operator
			}

		} else if (type->variant == TYPE_INT) {

			rt = NULL;
			switch(op) {

			case INSTR_IFNE:
				rt = TypeRestrict(type, restriction);
				break;

			// For Eq, resulting type is the range of source variable
			case INSTR_IFEQ:
				rt = TypeRestrict(restriction, type);
//				rt = restriction;
				break;

			case INSTR_IFLE: IntAdd(&min, IntMax(&min, &max), Int1()); IntSetMax(&max); break;	// remove anything bigger than 
			case INSTR_IFLT: IntSet(&min, IntMax(&min, &max)); IntSetMax(&max); break;
			case INSTR_IFGE: IntSetMin(&min); IntSub(&max, IntMin(&min, &max), Int1()); break;
			case INSTR_IFGT: IntSetMin(&min); IntSet(&max, IntMin(&min, &max)); break;

			default: ;
			}

			if (rt == NULL) {
				rt = TypeRestrictRange(type, min, max);
			}
		}
	}
done:
	return rt;
}

Type * TypeRestrictBlk(Type * type, Var * var, InstrBlock * blk, Bool neg)
{
	Type * rt, * vt;
	Instr * i;
	InstrOp op;
	Var * var2;

	rt = type;

	// 1. Undefined type may not be restricted
	if (type == NULL || type->variant == TYPE_UNDEFINED) goto done;

	if (blk == NULL) goto done;
	i = blk->last;
	if (i == NULL) goto done;
	op = i->op;
	if (!IS_INSTR_BRANCH(i->op)) goto done;

	// Select appropriate variable
	if (i->arg1 == var) {
		var2 = i->arg2;
		vt = i->type[ARG2];
	} else if (i->arg2 == var) {
		var2 = i->arg1;
		op = OpRelSwap(op);
		vt = i->type[ARG1];
	} else {
		goto done;
	}

	// If this is path not taken, swap the operation
	if (neg) {
		op = OpNot(op);
	}

	//TODO: We may use the inferred type from instruction
	if (vt == NULL) vt = var2->type;

	rt = TypeRestrictOp(type, vt, op);

done:
	return rt;
}


UInt32 paths;		// number of paths
UInt32 undefined;
Bool   looped;		//self referencing and looped (without found bound)

Type * FindType(Loc * loc, Var * var, Bool report_errors);

UInt16 g_fb_level;

//#define TRACE_INFER 1

Type * TypeAllocIntRange(Type * min, Type * max)
{
	Type * type = TUNDEFINED;
	if (TypeIsInt(min) && TypeIsInt(max)) {
		type = TypeAllocInt(min->range.min, max->range.max);
	}
	return type;
}

Bool VarIdentical(Var * left, Var * right)
{
	if (left == NULL || right == NULL) return false;
	if (left == right) return true;

	// Variable may be alias (i.e. may be specified by 

	if ((left->submode & (SUBMODE_IN | SUBMODE_OUT | SUBMODE_IN_SEQUENCE | SUBMODE_OUT_SEQUENCE)) != (right->submode & (SUBMODE_IN | SUBMODE_OUT | SUBMODE_IN_SEQUENCE | SUBMODE_OUT_SEQUENCE))) return false;

	while (left->adr != NULL && left->mode == INSTR_VAR && left->adr->mode == INSTR_VAR) left = left->adr;
	while (right->adr != NULL && right->mode == INSTR_VAR && right->adr->mode == INSTR_VAR) right = right->adr;

	if (left == right) return true;

	return false;
}

Type * FindTypeCall(Var * proc, Var * var)
/*
Purpose:
	Find type of variable.
*/
{
	Var * arg;
	FOR_EACH_OUT_ARG(proc, arg)
		if (arg == var) return arg->type;
	NEXT_OUT_ARG
	return NULL;
}

Type * FindTypeBlock(Loc * loc, Var * var, Type * index_type, InstrBlock * blk, Instr * instr)
/*
Purpose:
	Find type of variable var used in instruction at location loc in specified block beginning at instruction instr.
Result:
	NULL             Specified variable is not modified in this block (or any block that jumps to this block).
	                 This may happen for example for loops, where the variable is not modified in the loop.
                     When NULL is combined with other type, result is the non-null type.
	TYPE_UNDEFINED   Undefined type means, that the type is somehow modified, but we do not know exactly how.
	                 (We cannot infer the type).
					 When UNDEFINED type is combined with any other type, result is UNDEFINED type.
*/
{
	Instr * i;
	InstrBlock * caller;
	Type * type, * type2, * old_type;
	Var * var2;
	UInt16 caller_count;

	if (blk == NULL) return NULL;

#ifdef TRACE_INFER
	PrintRepeat("  ", g_fb_level); Print("#"); PrintInt(blk->seq_no); Print("\n");
#endif
	// If the block has been already processed, return the remembered result.
	// In case of loop, the result will be NULL, which means this branch does not alter the type of the variable in any way.

	if (blk->processed) goto done2;

	blk->type = NULL;

	if (instr == NULL) {
		i = blk->last;
		blk->processed = true;
	} else {
		i = instr->prev;
	}

	type = NULL;

	// Definition of the variable may be in this block
	for(; i != NULL; i = i->prev) {
		if (i->op == INSTR_LINE) continue;

		if (i->op == INSTR_CALL) {
			type = FindTypeCall(i->result, var);
			if (type != NULL) goto done;
		}

		if (i->result == NULL) continue;

		if (VarIdentical(i->result,var)) {

			type = i->type[RESULT];
			if (type != NULL && FlagOff(i->flags, InstrRestriction)) goto done;

			// We have found the same instruction again.
			// This means, it is part of some loop.
			if (i == loc->i) {
				looped = true;
				if (InstrIsSelfReferencing(i)) {

					if (!VarIdentical(i->arg2, i->result)) {
						var2 = i->arg2;
						if (i->type[ARG2] != NULL) {
							type = TypeAlloc(TYPE_SEQUENCE);
							type->seq.op = i->op;
							type->seq.step = i->type[ARG2];
							type->seq.init = NULL;
							type->seq.compare_op = INSTR_VOID;
							type->seq.limit = NULL;
							goto sub1;		//continue;
						}
					}
					type = TUNDEFINED;	// we are not able to deduct the type of the instruction now
					goto done;
				}
			}

//			if (type == NULL || FlagOn(i->flags, InstrRestriction)) type = TUNDEFINED;
			type = TUNDEFINED;
sub1:
			#ifdef TRACE_INFER
				PrintRepeat("  ", g_fb_level); Print("instr:"); PrintType(type); Print("\n");
			#endif
			goto done;
		// For array, we check, that A(x) fits B(y) so, that A = B and x contains y
		} else if (var->mode == INSTR_ELEMENT) {
			if (i->result->mode == INSTR_ELEMENT && var->adr == i->result->adr) {
				// Now we need to find the type of result index & type of var index
				if (TypeIsSubsetOf(index_type, i->result_index_type)) {
					type = i->type[RESULT];
					if (type != NULL && FlagOff(i->flags, InstrRestriction)) goto done;
				}
			}
		}
	}


	// We are in starting block and there is no definition.
	// This is use of undefined variable. (Except for procedure input arguments, in variables and global variables).
	
	if (blk->from == NULL && blk->callers == NULL) {
		// If we are at the beginning of the procedure and this is an input argument, we can use the type of variable as an argument.
		// Input register variables are considered defined here too.
		if (VarIsInArg(var) || InVar(var)) {
			type = var->type;
		} else {
			undefined++;
			type = TUNDEFINED;
		}
	}


	old_type = type;
	caller_count = 0;
	if (blk->from != NULL) {
		caller_count++;
#ifdef TRACE_INFER
		g_fb_level++;
#endif
		type2 = FindTypeBlock(loc, var, index_type, blk->from, NULL);
#ifdef TRACE_INFER
		g_fb_level--;
#endif

		// This is just speed optimization, if the type is undefined, we do not need to continue processing
		if (type2 != NULL) {
			if (type2->variant == TYPE_UNDEFINED) {
				type = type2;
				goto done;
			}
			type2 = TypeRestrictBlk(type2, var, blk->from, true);
			type = TypeUnion(type, type2);
			type = TypeRestrictBlk(type, var, blk->from, true);
		}
	}

	for(caller = blk->callers; caller != NULL; caller = caller->next_caller) {
		caller_count++;
//		if (caller_count > 1) paths++;
#ifdef TRACE_INFER
		g_fb_level++;
#endif
		type2 = FindTypeBlock(loc, var, index_type, caller, NULL);
#ifdef TRACE_INFER		
		g_fb_level--;
#endif
		if (type2 != NULL) {
			if (type2->variant == TYPE_UNDEFINED) {
				type = type2;
				goto done;
			}
			type2 = TypeRestrictBlk(type2, var, caller, false);
			type = TypeUnion(type, type2);
			type = TypeRestrictBlk(type, var, caller, false);
		}
	}

	if (caller_count > 0) paths += caller_count - 1;

done:
	blk->type = type;
done2:
	return blk->type;
}


Type * FindType(Loc * loc, Var * var, Bool report_errors)
/*
Purpose:
	Find type of variable at specified location.
Result:
	NULL if type was not found or is undefined
*/
{
	Type * type = NULL;
	Type * arr_type;
	Type * index_type;
	Type * left, * right;

	if (var == NULL) return NULL;
	if (var->mode == INSTR_CONST) {
		type = var->type;
		//TODO: Use type from constant (if it exists)
		if (type->variant == TYPE_INT) {
			if (type->range.min != var->n || type->range.max != var->n) {
				type = TypeAllocInt(var->n, var->n);
				type->flexible = false;
			}
		}
	} else if (var->mode == INSTR_RANGE) {
		left = FindType(loc, var->adr, report_errors);
		right = FindType(loc, var->var, report_errors);
		type = TypeAllocIntRange(left, right);
		
	} else if (var->mode == INSTR_TUPLE) {
		left = FindType(loc, var->adr, report_errors);
		right = FindType(loc, var->var, report_errors);
		type = TypeTuple(left, right);

	} else if (var->mode == INSTR_BYTE) {
		type = TypeByte();
	} else if (var->mode == INSTR_TYPE) {
		type = var->type;
	} else {
		if (var->type->variant == TYPE_PROC) {
			type = var->type;
		} else {

#ifdef TRACE_INFER
		g_fb_level = 0;
#endif
			paths = 1;
			looped = false;
			undefined = 0;
			MarkBlockAsUnprocessed(loc->proc->instr);
			index_type = NULL;
			if (VarIsArrayElement(var)) {
				index_type = FindType(loc, var->var, false);
			}
			type = FindTypeBlock(loc, var, index_type, loc->blk, loc->i);

			// Type has not been specified in previous code
			if (type->variant == TYPE_UNDEFINED) {

				if (var->mode == INSTR_ELEMENT) {
					arr_type = var->adr->type;
					if (VarIsArrayElement(var)) {
						type = arr_type->element;
					//Using address to access array element
					} else if (arr_type->variant == TYPE_ADR) {
						if (arr_type->element->variant == TYPE_ARRAY) {
							// adr of array of type
							type = arr_type->element->element;
						} else {
							Print("");
						}
					}
				}
			}

			// Let-adr instruction generates address of specified variable.
			// The address will be valied even in case the variable has not been initialized yet, therefore
			// we will not report 'uninitialized variable'.
			if (loc->i->op == INSTR_LET_ADR) {
				type = TypeAdrOf(type);
			} else {
				// Type was not found. This means, the variable has not been assigned yet (or at least at some path).
				// In such case, we are not able to infer the type (or infer it completely).
				// User will be asked to specify the type for the variable.
				// It also means, we may be using undefined variable!

				if (type->variant == TYPE_UNDEFINED) {

					if (report_errors) {
						// Argument to let adr 
						if (undefined > 0) {
							// Input register does not have to be explicitly initialized.
							// Global variables are supposed to be initialized before entering the procedure.
							if (!InVar(var) && (var->mode == INSTR_VAR) && VarIsLocal(var, loc->proc)) {
								ErrArg(var);
								if (paths > 1) {
									LogicWarningLoc("Possible use of uninitialized variable [A].\nThere exists a path where it is not initialized before it is used here.", loc);
								} else {
									LogicWarningLoc("Use of uninitialized variable [A].", loc);
								}
							}
						} else if (looped) {
							LogicWarningLoc("looped variable", loc);
						}
					}
				}
			}
		}
	}

	if (type != NULL && type->variant == TYPE_UNDEFINED) type = NULL;
	return type;
}

void ReplaceConst(Var ** p_var, Type * type)
/*
Purpose:
	If the possible type of variable is range where min = max, replace the variable with constant.
*/
{
	Var * var;
	var = *p_var;
	if (var != NULL && var->mode != INSTR_CONST && !InVar(var)) {
		if (type != NULL && type->variant == TYPE_INT && type->range.min == type->range.max) {
			*p_var = VarNewInt(type->range.min);
		}
	}
}

void ReportUnusedVars(Var * proc)
/*
Purpose:
	Report variables declared in this procedure that are not used.
*/
{
	Var * var;
	Type * type;

	FOR_EACH_LOCAL(proc, var)
		if (var->name != NULL && var->name != TMP_NAME && FlagOff(var->submode, SUBMODE_SYSTEM) && var->mode == INSTR_VAR) {
			type = var->type;
			if (type != NULL && type->variant == TYPE_LABEL) continue;
			if (var->read == 0) {
				ErrArg(var);
				if (var->write == 0) {
					LogicWarning("Unreferenced variable [-A].", SetBookmarkVar(var));
				} else {
					if (!OutVar(var)) {
						LogicWarning("Unread variable [-A].", SetBookmarkVar(var));
					}
				}
			}
		}
	NEXT_LOCAL
}

Bool DistributeRestrictionBlk(Loc * loc, Var * var, Type * restriction, InstrBlock * blk, Instr * instr)
/*
Purpose:
	Spread information about restriction of specified variable.
	The type specified defines restriction.
Result:
	Returns true, if something was modified.
*/
{
	Instr * i;
//	InstrBlock * caller;
	Type * type;
	Bool modified = false;

	if (blk == NULL) return false;

	// If the block has been already processed, return the remembered result.
	// In case of loop, the result will be NULL, which means this branch does not alter the type of the variable in any way.

	if (blk->processed) return false;

	if (instr == NULL) {
		i = blk->last;
		blk->processed = true;
	} else {
		i = instr->prev;
	}

	// Definition of the variable may be in this block
	for(; i != NULL; i = i->prev) {
		if (i->op == INSTR_LINE) continue;
		if (i->result == NULL) continue;

		if (VarIdentical(i->result, var)) {

			// We have found the same instruction again.
			// This means, it is part of some loop.
			if (i == loc->i) {
				if (InstrIsSelfReferencing(i)) {
					looped = true;

					type = TUNDEFINED;	// we are not able to deduct the type of the instruction now
					goto done;
				}
			}

			type = i->type[RESULT];
			if (type == NULL || type->variant == TYPE_UNDEFINED) {
				i->type[RESTRICTION] = restriction;
				SetFlagOn(i->flags, InstrRestriction);
				modified = true;
			} else {
				// type may be already restriction
				// then we came from different branch here and the restrictions should combine
			}
			goto done;
		}
	}

	// We are in starting block and we haven't found the variable.
	// It must be either input argument or global variable.
	// In other case, this would be use of undefined variable, that is however handled elsewhere.
	
	if (blk->from == NULL && blk->callers == NULL) {
		type = TypeRestrict(var->type, restriction);
		if (type != var->type) {
			var->type = type;
			modified = true;
		}
	}


done:
	blk->processed = true;
	return modified;
}

Bool PropagateConstraint(Loc * loc, Var * var, Type * restriction, InstrBlock * blk, Instr * instr)
{
	paths = 1;
	looped = false;
	undefined = 0;
	MarkBlockAsUnprocessed(loc->proc->instr);

	return DistributeRestrictionBlk(loc, var, restriction, blk, instr);
}

Bool ProcInstrEnum(Var * proc, Bool (*fn)(Loc * loc, void * data), void * data)
{
	Instr * i;
	Instr * next_i;
	InstrBlock * blk;
	Loc loc;

	loc.proc = proc;

	for(blk = proc->instr; blk != NULL; blk = blk->next) {
		loc.blk = blk;
		loc.n = 1;
		for(i = blk->first; i != NULL; i = next_i) {
			next_i = i->next;
			if (i->op != INSTR_LINE) {
				loc.i = i;
				if (fn(&loc, data)) return true;
			}
			loc.n++;
		}
	}
	return false;
}

typedef struct {
	Bool modified;
	Bool modified_blocks;
	Bool final_pass;
} InferData;

void VarConstraints(Loc * loc, Var * var, InferData * d)
{
	Var * idx;
	Type * ti;

	// Index of array access must match the type specified in array
	if (VarIsArrayElement(var)) {
		idx = var->var;
		if (idx->mode == INSTR_VAR || idx->mode == INSTR_CONST) {
			ti = FindType(loc, idx, d->final_pass);
			// Type of the index is undefined, this is restriction
			if (ti == NULL || ti->variant == TYPE_UNDEFINED) {
				if (PropagateConstraint(loc, idx, var->adr->type->index, loc->blk, loc->i)) {
					d->modified = true;
				}
			}
		}
	}
}

Bool InstrConstraints(Loc * loc, void * data)
{
	Var * result;
	InstrOp op;
	Instr * i;
	Type * tr, * tl;
	InferData * d = (InferData *)data;

	i = loc->i;
	result = i->result;
				
	if (i->type[RESULT] != NULL) {
		if (i->arg1 != NULL && i->type[ARG1] == NULL) {
			if (i->type[ARG2] != NULL) {
				// Restriction given by instruction
				tl = i->type[RESULT];
				tr = i->type[ARG2];
				op = INSTR_VOID;
				switch(i->op) {
				case INSTR_ADD: op = INSTR_SUB;	break;
				case INSTR_SUB: op = INSTR_ADD; break;
				case INSTR_MUL: op = INSTR_DIV; break;
				case INSTR_DIV: op = INSTR_MUL; break;
				default:
					break;
				}
				if (op != INSTR_VOID) {
					tr = TypeEval(op, tl, tr);
					if (PropagateConstraint(loc, i->arg1, tr, loc->blk, i)) {
						d->modified = true;
					}
				}
			}
		}

		if (i->arg2 != NULL && i->type[ARG2] == NULL) {
		}
	}

	VarConstraints(loc, i->result, d);
	VarConstraints(loc, i->arg1, d);
	VarConstraints(loc, i->arg2, d);
	return false;
}


void CheckIndex(Loc * loc, Var * var)
// Check array indexes
// Array may have one or two indexes
// Index may be simple, or it can be range
{
	Type * type;
	Var * idx;

	if (VarIsArrayElement(var)) {
		idx = var->var;
		type = FindType(loc, var->var, true);
		if (type != NULL) {
			if (!TypeIsSubsetOf(type, var->adr->type->index)) {
				if (type->variant == TYPE_INT) {
					ErrArg(VarNewInt(type->range.max));
					ErrArg(VarNewInt(type->range.min));
					ErrArg(var->adr);
					LogicWarningLoc("Index of array [A] out of bounds.\nThe index range is [B]..[C].", loc);
				} else {
					ErrArg(var->adr);
					LogicErrorLoc("Cannot infer type of index of array [A].", loc);
				}
			}
		} else {
			// failed to compute index, what does it means?
		}
	}

}


#define MIN1 TypeMin(i->type[ARG1])
#define MAX1 TypeMax(i->type[ARG1])
#define MIN2 TypeMin(i->type[ARG2])
#define MAX2 TypeMax(i->type[ARG2])

Bool TypeIsComplete(Type * type)
{
	if (type == NULL) return false;
	if (type->variant == TYPE_SEQUENCE) {
		return type->seq.init != NULL && type->seq.step != NULL && type->seq.limit != NULL && type->seq.op != INSTR_VOID;
	}
	return true;
}

Bool InstrInferType(Loc * loc, void * data)
{
	Var * result;
	Instr * i;
	Type * tr, * ti;
	InferData * d = (InferData *)data;
	Bool taken, not_taken, not;

	i = loc->i;

	if (loc->blk->seq_no == 8 && loc->n == 15) {
		Print("");
	}

	if (i->result != NULL && (i->type[RESULT] == NULL || FlagOn(i->flags, InstrRestriction))) {

		// If the result of the instruction is array index, try to infer type of the index.
		// The type may be later used when looking for the type of some instruction argument.

		if (i->result_index_type == NULL && VarIsArrayElement(i->result)) {
			i->result_index_type = FindType(loc, i->result->var, d->final_pass);
			if (i->result_index_type != NULL) d->modified = true;
		}

		if (i->arg1 != NULL && i->type[ARG1] == NULL) {
			i->type[ARG1] = FindType(loc, i->arg1, d->final_pass);
			if (i->type[ARG1] != NULL) d->modified = true;
		}

		if (i->arg2 != NULL && i->type[ARG2] == NULL) {
			i->type[ARG2] = FindType(loc, i->arg2, d->final_pass);
			if (i->type[ARG2] != NULL) d->modified = true;
		}

		// In some cases, we may find out, that the type at this place does allow only one value.
		// That means, we may replace the variable with the constant.
		ReplaceConst(&i->arg1, i->type[ARG1]);
		ReplaceConst(&i->arg2, i->type[ARG2]);

		// For comparisons, we may check whether the condition is not always true or always false
		if (IS_INSTR_BRANCH(i->op)) {
			if (i->type[ARG1] != NULL && i->type[ARG2] != NULL) {
				taken = false;
				not_taken = false;
				not = false;
				switch (i->op) {
				case INSTR_IFNE:
					not = true;
				case INSTR_IFEQ:
					// We know for sure the condition is true, if both values are in fact same integer constants
					taken = TypeIsIntConst(i->type[ARG1]) && TypeIsIntConst(i->type[ARG2]) && IntEq(MIN1, MIN2);
					not_taken = IntLower(MAX1, MIN2) || IntHigher(MIN1, MAX2);
					break;

				case INSTR_IFGE:
					not = true;
				case INSTR_IFLT:
					taken = IntLower(MAX1, MIN2);
					not_taken = IntHigherEq(MIN1, MAX2);
					break;

				case INSTR_IFLE:
					not = true;
				case INSTR_IFGT:
					taken = IntHigher(MIN1, MAX2);
					not_taken = IntLowerEq(MAX1, MIN2);
					break;
					default:
					break;

				case INSTR_IFNTYPE:
					not = true;
				case INSTR_IFTYPE:
					if (TypeIsComplete(i->type[ARG1]) && TypeIsComplete(i->type[ARG2])) {
						if (TypeEqual(i->type[ARG1], i->type[ARG2])) {
							taken = true;
						} else {
							not_taken = true;
						}
					}
					break;
				}

				if (not) {
					if (taken) {
						not_taken = true;
						taken = false;
					} else if (not_taken) {
						taken = true;
						not_taken = false;
					}
				}

				if (taken) {
					i->op = INSTR_GOTO;
					i->arg1 = i->arg2 = NULL;
					d->modified_blocks = true;
					return true;
				} else if (not_taken) {
					i->result->write--;
					InstrDelete(loc->blk, i);
					d->modified_blocks = true;
					return true;
				}
			}
		} else {

			result = i->result;

			tr = TypeEval(i->op, i->type[ARG1], i->type[ARG2]);

			// Type was evaluated, test, whether there is not an error while assigning it
			if (tr != NULL /*&& !InstrIsSelfReferencing(i)*/) {
				if (FlagOn(result->submode, SUBMODE_USER_DEFINED) || (result->mode == INSTR_ELEMENT && FlagOn(result->adr->submode, SUBMODE_USER_DEFINED))) {

					// We allow assigning values to arrays, so we must allow this operation in type checker

					ti = result->type;

					// We initialize array with list of elements
					// TODO: Parser should probably create element of array borders, to distinguish it from
					//       assigning arrays.

					if (i->op == INSTR_LET && ti->variant == TYPE_ARRAY && tr->variant != TYPE_ARRAY) {
						ti = ti->element;
					}

					if (!TypeIsSubsetOf(tr, ti)) {
						if (d->final_pass) {
							if (tr->variant == TYPE_INT && ti->variant == TYPE_INT) {
								ErrArg(result);
								ErrArg(VarNewInt(ti->range.max));
								ErrArg(VarNewInt(ti->range.min));
								ErrArg(VarNewInt(tr->range.max));
								ErrArg(VarNewInt(tr->range.min));
								if (TypeIsIntConst(tr)) {
									LogicWarningLoc("The value [A] does not fit into variable", loc);
								} else {
									LogicWarningLoc("Result of expression does not fit the target variable.\nThe range of result is [A]..[B]. The range of variable is [C]..[D].", loc);
								}
							} else {
								LogicWarningLoc("Value does not fit into variable", loc);								
							}
						}
					}
				}

			// If the resulting type is defined and we failed to compute the type, we may try to
			// deduce the type 'backwards'.
			} else {
			}

			if (tr != NULL) {
				i->type[RESULT] = tr;
				SetFlagOff(i->flags, InstrRestriction);
				d->modified = true;
			}
		}
	}

	if (d->final_pass) {
		if (loc->blk->seq_no == 8 && loc->n == 15) {
			Print("");
		}
		CheckIndex(loc, i->result);
		CheckIndex(loc, i->arg1);
		CheckIndex(loc, i->arg2);
	}

	return false;
}

/*

Type inferencer tries to determine type for result of every instruction (that is for every instruction that has result).

1. Type was deduced
2. Type was not deduced, but the destination variable has been defined by user (use the user defined type)
3. Type was not deduced, destination variable is immediately assigned to variable with defined type

*/

Bool InstrInitInfer(Loc * loc, void * data)
{
	Instr * i = loc->i;
	i->type[0] = i->type[1] = i->type[2] = NULL;
	i->result_index_type = NULL;
	i->flags = 0;
	return false;
}

Bool LoopSteps(Instr * i, IntLimit * min_steps, IntLimit * max_steps)
/*
Purpose:
	Compute number of steps performed by loop depending on loop instruction.
*/
{
	Type * type;
	IntLimit min_span, max_span;

	// Compute number of repeats for this loop (returned as integer type)
	if (i->type[ARG1] != NULL && i->type[ARG2] != NULL) {
		type = i->type[ARG1];
		if (type->variant == TYPE_SEQUENCE) {
			if (TypeIsInt(type->seq.step) && TypeIsInt(type->seq.init) && TypeIsInt(type->seq.limit)) {

				max_span = (type->seq.limit->range.max - type->seq.init->range.min) + 1;
				min_span = (type->seq.limit->range.min - type->seq.init->range.max) + 1;

				if (type->seq.op == INSTR_ADD) {
					*min_steps = min_span / type->seq.step->range.max;
					*max_steps = max_span / type->seq.step->range.min;
					return true;
				}
			}
		}
	}
	return false;
}

Bool UseLoop(Var * proc, InstrBlock * header, InstrBlock * end, IntLimit min_steps, IntLimit max_steps)
{
	Bool modified = false;
	Loc loc;
	InstrBlock * stop = end->next;
	Instr * i;
	Type * type;
	IntLimit stop_min, stop_max;

	loc.proc = proc;
	loc.blk  = header;
	for(loc.blk = header; loc.blk != stop; loc.blk = loc.blk->next) {
		for(loc.i = loc.blk->first; loc.i != NULL; loc.i = loc.i->next) {
			i = loc.i;
			type = i->type[RESULT];
			if (type != NULL && type->variant == TYPE_SEQUENCE) {
				if (TypeIsInt(type->seq.step) && TypeIsInt(type->seq.init) && type->seq.limit == NULL) {
					if (type->seq.op == INSTR_ADD) {
						stop_min = type->seq.init->range.min + ((min_steps - 1) * type->seq.step->range.min);
						stop_max = type->seq.init->range.max + ((max_steps - 1) * type->seq.step->range.max);
						i->type[RESULT] = TypeAllocInt(stop_min, stop_max);
						modified = true;
					} else if (type->seq.op == INSTR_SUB) {
//						stop_max = type->seq.init->range.max - ((min_steps - 1) * type->seq.step->range.min);
						stop_min = type->seq.init->range.min - ((max_steps - 1) * type->seq.step->range.max);
						i->type[RESULT] = TypeAllocInt(stop_min, type->seq.init->range.max);
						modified = true;
					}
				}
			}
		}
	}
	return modified;
}

static Bool TypeInferLoops(Var * proc)
/*
Purpose:
	Try to compute number of steps of every loop in the procedure.
	If sucessfull, this information may be used to infer some more types.
*/ {
	InstrBlock * header;
	Instr * i;
	IntLimit min_steps, max_steps;

	Bool modified = false;
	Loc loc;

	loc.proc = proc;

	MarkLoops(proc);
	for(loc.blk = proc->instr; loc.blk != NULL; loc.blk = loc.blk->next) {
		header = NULL;
		if (loc.blk->jump_type == JUMP_LOOP) {
			header = loc.blk->cond_to;
			if (header == NULL) header = loc.blk->to;
		}

		// This is end of loop.
		// We may try to infer some information if we know maximal and/or minimal number of repeats
		if (header != NULL) {
//			Print("Loop "); PrintInt(header->seq_no); Print(".."); PrintInt(loc.blk->seq_no); PrintEOL();
			i = LastInstr(loc.blk);

			// Check sequence like
			//     ifeq ...  goto LAB
			//     ----
			//     goto loop
			//     ----
			//LAB@
			if (i == NULL && loc.blk->callers == NULL && loc.blk->from != NULL) {
				i = LastInstr(loc.blk->from);
				if (i != NULL && IS_INSTR_BRANCH(i->op)) {
					if (LoopSteps(i, &min_steps, &max_steps)) {
					}
				}
			} else if (i != NULL && IS_INSTR_BRANCH(i->op)) {
//				InstrPrint(i);
				if (LoopSteps(i, &min_steps, &max_steps)) {
					// we have the number of steps of this loop
					modified = UseLoop(proc, header, loc.blk, min_steps, max_steps);
				}
			}
		}
	}

	return modified;
}

static Bool InstrFreeIncomplete(Loc * loc, void * data)
{
	UInt8 n;
	Instr * i = loc->i;

	Type * type;
	for(n=0; n<=2; n++) {
		type = i->type[n];
		if (type != NULL && !TypeIsComplete(type)) i->type[n] = NULL;
	}
	return false;
}

void TypeInfer(Var * proc)
/*
Purpose:
	Try to infer types for all variables used in given procedure.
*/
{

	Instr * i;
	Type * tr, * tl;
	Var * var;
	Loc loc;
	UInt16 steps;
	InferData data;
	UInt32 n;

	Bool assert_begin;

	ProcInstrEnum(proc, &InstrInitInfer, NULL);

	if (Verbose(proc)) {
		PrintHeader(2, proc->name);
		PrintProc(proc);
	}

	// 1. For every instruction in the code try to infer the type of it's result
	// 2. Repeat this until no new result type was inferred

	data.final_pass = false;
retry:
	do {

		steps = 0;
		do {
			data.modified = false;
			data.modified_blocks = false;
			ProcInstrEnum(proc, &InstrInferType, &data);
			if (data.modified_blocks) {
				GenerateBasicBlocks(proc);
				DeadCodeElimination(proc);
//				PrintProc(proc);
			}
			steps++;
		} while (data.modified);

		steps = 0;
		do {
			data.modified = false;
			ProcInstrEnum(proc, &InstrConstraints, &data);
			steps++;
		} while(data.modified);

	} while (steps > 1);

	// We may deduce some information based on loops

	if (TypeInferLoops(proc)) {
		// Some of the types may be incomplete sequences, we need to clear these types so they will be evaluated again
		ProcInstrEnum(proc, &InstrFreeIncomplete, NULL);
		goto retry;
	}

	// Perform final pass, which will print error messages.
	data.final_pass = true;
	ProcInstrEnum(proc, &InstrInferType, &data);
	ProcInstrEnum(proc, &InstrConstraints, &data);


	// Extend the type of variables to handle
	// - Check variables, whose types can not be inferred here
	// - Check array indexes (this may already lead to argument inference algorithm)

	loc.proc = proc;
	for(loc.blk = proc->instr; loc.blk != NULL; loc.blk = loc.blk->next) {
		for(n = 1, loc.i = loc.blk->first; loc.i != NULL; loc.i = loc.i->next, n++) {
			i = loc.i;

			if (i->op == INSTR_LINE) continue;

			var = i->result;
			if (var != NULL && !VarIsLabel(var) && !VarIsArrayElement(var) && !VarIsArray(var)) {
				tr = i->type[RESULT];
				if (VarIsLocal(var, proc) && FlagOff(var->submode, SUBMODE_USER_DEFINED)) {
					if (tr != NULL && tr->variant != TYPE_UNDEFINED && FlagOff(i->flags, InstrRestriction)) {
						var->type = TypeExpand(var->type, i->type[RESULT]);
					} else {
//						if (var->type->variant == TYPE_UNDEFINED) {

							// For temporary variable, there is no reason to define 
							if (VarIsTmp(var)) {
								LogicErrorLoc("Cannot infer type of result of operator [*].", &loc);
							} else {
								ErrArg(var);
								LogicErrorLoc("Cannot infer type of variable [A].\nPlease define the type or use assert to give the compiler some more information.", &loc);
							}
//						}
					}
				} else if (tr == NULL) {
					tl = i->type[ARG1];
					if (tl != NULL) {
						if (tl->variant == TYPE_SEQUENCE) {
							//TODO: This test is probably never good
//							ErrArg(var);
//							LogicErrorLoc("Cyclic modification of variable [A].\nVariable will eventually go out of it's defined bounds.\n", &loc);
						}
					}
				}
			}
		}
	}

	if (Verbose(proc)) {
		PrintVars(proc);
	}

	ReportUnusedVars(proc);

	// Test, if there is some assert that is known at compile time to be always false.
	// Such assert reports error.

	for(loc.blk = proc->instr; loc.blk != NULL; loc.blk = loc.blk->next) {
		assert_begin = false;
		for(i = loc.blk->first; i != NULL; i = i->next) {
			if (i->op == INSTR_ASSERT_BEGIN) {
				assert_begin = true;
			} else if (i->op == INSTR_ASSERT) {
				if (assert_begin) {
					loc.i = i;
					LogicErrorLoc("Assert is always false.", &loc);
				}
			}
		}
	}
}
