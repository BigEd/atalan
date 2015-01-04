/*

Compiler instructions management

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php


*/

#include "language.h"

extern Var * VARS;		// global variables
GLOBAL Instr NULL_INSTR;
GLOBAL Instr * InstrNull;

extern Var * MACRO_ARG[MACRO_ARG_CNT];

void InstrDecl(Instr * i);

InstrInfo INSTR_INFO[INSTR_CNT] = {
{ INSTR_NULL,      "",    "null", {TYPE_VOID, TYPE_VOID, TYPE_VOID }, 0, &VoidCellFree    , NULL,  &VoidEval },
{ INSTR_VOID,      "nop", "void", {TYPE_VOID, TYPE_VOID, TYPE_VOID }, 0, &VoidCellFree    , NULL,  &VoidEval },
{ INSTR_LET,       "let", "<-",   {TYPE_ANY,  TYPE_ANY,  TYPE_VOID }, 0, &VoidCellFree    , &PrintLet,  &VoidEval },
{ INSTR_IF,        "iff",  "if",  {TYPE_VOID, TYPE_ANY,  TYPE_LABEL}, 0, &VoidCellFree    , &PrintIf,  &VoidEval },			// if arg1 goto arg2

{ INSTR_EQ,        "eq", "=",     {TYPE_VOID, TYPE_ANY, TYPE_ANY}, INSTR_COMMUTATIVE_OPERATOR, &VoidCellFree, &PrintBinaryOp,  &EqEval },
{ INSTR_NE,        "ne", "<>",    {TYPE_VOID, TYPE_ANY, TYPE_ANY}, INSTR_COMMUTATIVE_OPERATOR, &VoidCellFree, &PrintBinaryOp,  &NeEval },
{ INSTR_LT,        "lt", "<",     {TYPE_VOID, TYPE_ANY, TYPE_ANY}, INSTR_OPERATOR,             &VoidCellFree, &PrintBinaryOp,  &LtEval },
{ INSTR_GE,        "ge", ">=",    {TYPE_VOID, TYPE_ANY, TYPE_ANY}, INSTR_OPERATOR,             &VoidCellFree, &PrintBinaryOp,  &GeEval },
{ INSTR_GT,        "gt", ">",     {TYPE_VOID, TYPE_ANY, TYPE_ANY}, INSTR_OPERATOR,             &VoidCellFree, &PrintBinaryOp,  &GtEval },
{ INSTR_LE,        "le", "<=",    {TYPE_VOID, TYPE_ANY, TYPE_ANY}, INSTR_OPERATOR,             &VoidCellFree, &PrintBinaryOp,  &LeEval },
{ INSTR_OVERFLOW,  "overflow", "over", {TYPE_ANY, TYPE_VOID, TYPE_VOID}, 0, &VoidCellFree    , NULL,  &VoidEval },
{ INSTR_NOVERFLOW, "noverflow", "not over", {TYPE_ANY, TYPE_VOID, TYPE_VOID}, 0, &VoidCellFree    , NULL,  &VoidEval },

{ INSTR_ADD,       "add", "+",    {TYPE_ANY, TYPE_ANY, TYPE_ANY}, INSTR_COMMUTATIVE_OPERATOR,  &VoidCellFree, &PrintBinaryOp,  &AddEval },
{ INSTR_SUB,       "sub", "-",    {TYPE_ANY, TYPE_ANY, TYPE_ANY}, INSTR_OPERATOR,              &VoidCellFree, &PrintBinaryOp,  &SubEval },
{ INSTR_MUL,       "mul", "*",    {TYPE_ANY, TYPE_ANY, TYPE_ANY}, INSTR_COMMUTATIVE_OPERATOR,  &VoidCellFree, &PrintBinaryOp,  &MulEval },
{ INSTR_DIV,       "div", "/",    {TYPE_ANY, TYPE_ANY, TYPE_ANY}, INSTR_OPERATOR,              &VoidCellFree, &PrintBinaryOp,  &DivIntEval },
{ INSTR_MOD,       "mod", "mod",  {TYPE_ANY, TYPE_ANY, TYPE_ANY}, INSTR_OPERATOR,            &VoidCellFree, &PrintBinaryOp,  &ModEval },
{ INSTR_SQRT,      "sqrt", "sqrt",{TYPE_ANY, TYPE_ANY, TYPE_VOID}, INSTR_OPERATOR,         &VoidCellFree, &PrintUnaryOp,   &SqrtEval },

{ INSTR_AND,       "and", "and",  {TYPE_ANY, TYPE_ANY, TYPE_ANY}, INSTR_COMMUTATIVE_OPERATOR,&VoidCellFree, &PrintBinaryOp,  &VoidEval },
{ INSTR_OR,        "or", "or",    {TYPE_ANY, TYPE_ANY, TYPE_ANY}, INSTR_COMMUTATIVE_OPERATOR,  &VoidCellFree, &PrintBinaryOp,  &VoidEval },
{ INSTR_XOR,       "xor", "xor",  {TYPE_ANY, TYPE_ANY, TYPE_ANY}, INSTR_COMMUTATIVE_OPERATOR,&VoidCellFree, &PrintBinaryOp,  &VoidEval },
{ INSTR_NOT,       "not", "not",  {TYPE_ANY, TYPE_ANY, TYPE_VOID}, INSTR_OPERATOR,           &VoidCellFree, &PrintUnaryOp,   &VoidEval },

{ INSTR_ROL,       "rotl", "<<",  {TYPE_ANY, TYPE_ANY, TYPE_ANY}, 0, &VoidCellFree    , &PrintBinaryOp,  &VoidEval },				// bitwise rotate right
{ INSTR_ROR,       "rotr", ">>",  {TYPE_ANY, TYPE_ANY, TYPE_ANY}, 0, &VoidCellFree    , &PrintBinaryOp,  &VoidEval },				// bitwise rotate left

{ INSTR_PROLOGUE,  "prologue", "",       {TYPE_VOID, TYPE_VOID, TYPE_VOID}, INSTR_OPTIONAL, &VoidCellFree    , &PrintInstr,  &VoidEval },
{ INSTR_EPILOGUE,  "epilogue", "",       {TYPE_VOID, TYPE_VOID, TYPE_VOID}, INSTR_OPTIONAL, &VoidCellFree    , &PrintInstr,  &VoidEval },
{ INSTR_EMIT,      "emit", "",           {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0, &VoidCellFree    , NULL,  &VoidEval },
{ INSTR_VARDEF,    "vardef", "",         {TYPE_ANY, TYPE_ANY, TYPE_VOID}, 0, &VoidCellFree    , &PrintInstr,  &VoidEval },
{ INSTR_LABEL,     "label", "label",     {TYPE_LABEL, TYPE_VOID, TYPE_VOID}, 0, &VoidCellFree    , &PrintInstr,  &VoidEval },

{ INSTR_ALLOC,     "alloc", "",          {TYPE_VOID, TYPE_ANY, TYPE_VOID}, 0, &VoidCellFree    , &PrintInstr,  &VoidEval },
{ INSTR_FN,        "fn",    "",          {TYPE_VOID, TYPE_ANY, TYPE_VOID}, 0, &VoidCellFree    , &PrintInstr,  &VoidEval },
{ INSTR_RETURN,    "return", "return",   {TYPE_VOID, TYPE_ANY, TYPE_VOID}, 0, &VoidCellFree    , &PrintInstr,  &VoidEval },
{ INSTR_ENDPROC,   "endfn", "",        {TYPE_VOID, TYPE_ANY, TYPE_VOID}, 0, &VoidCellFree    , &PrintInstr,  &VoidEval },
{ INSTR_CALL,      "call", "",           {TYPE_VOID, TYPE_ANY, TYPE_VOID}, 0, &VoidCellFree    , &PrintInstr,  &VoidEval },
{ INSTR_VAR_ARG,   "var_arg", "",        {TYPE_VOID, TYPE_ANY, TYPE_VOID}, INSTR_NON_CODE, &VoidCellFree    , &PrintInstr,  &VoidEval },

{ INSTR_DATA,      "data", "",           {TYPE_VOID, TYPE_ANY, TYPE_VOID}, INSTR_NON_CODE, &VoidCellFree    , &PrintInstr,  &VoidEval },
{ INSTR_FILE,      "file", "",           {TYPE_VOID, TYPE_STRING, TYPE_VOID}, INSTR_NON_CODE, &VoidCellFree    , NULL,  &VoidEval },
{ INSTR_ALIGN,     "align", "",          {TYPE_VOID, TYPE_ANY, TYPE_VOID}, 0, &VoidCellFree    , &PrintInstr,  &VoidEval },
{ INSTR_ORG,       "org", "",            {TYPE_VOID, TYPE_ANY, TYPE_VOID}, 0, &VoidCellFree    , &PrintInstr,  &VoidEval },				// set the destination address of compilation
{ INSTR_HI,        "hi", "hi",           {TYPE_ANY, TYPE_ANY, TYPE_VOID}, 0, &VoidCellFree    , NULL,  &VoidEval },
{ INSTR_LO,        "lo", "lo",           {TYPE_ANY, TYPE_ANY, TYPE_VOID}, 0, &VoidCellFree    , NULL,  &VoidEval },
{ INSTR_PTR,       "ptr", "",            {TYPE_VOID, TYPE_ANY, TYPE_VOID}, INSTR_NON_CODE, &VoidCellFree    , NULL,  &VoidEval },
{ INSTR_ARRAY_INDEX, "arrindex", "",     {TYPE_VOID, TYPE_ANY, TYPE_VOID}, INSTR_NON_CODE, &VoidCellFree    , NULL,  &VoidEval },		// generate index for array
{ INSTR_LET_ADR,   "let_adr", "=@",      {TYPE_ADR, TYPE_ANY, TYPE_VOID}, 0, &VoidCellFree    , NULL,  &VoidEval },
{ INSTR_DEBUG,     "debug", "",          {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0, &VoidCellFree    , &PrintInstr,  &VoidEval },

{ INSTR_ASSERT_BEGIN,"assert_begin", "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0, &VoidCellFree    , &PrintInstr,  &VoidEval },
{ INSTR_ASSERT,      "assert", "assert", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0, &VoidCellFree    , &PrintInstr,  &VoidEval },
{ INSTR_ASSERT_END,  "assert_end", "",   {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0, &VoidCellFree    , &PrintInstr,  &VoidEval },

{ INSTR_LINE2,      "line", "",          {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0, &VoidCellFree    , NULL,  &VoidEval },				// reference line in the source code
{ INSTR_INCLUDE,   "include", "",        {TYPE_VOID, TYPE_STRING, TYPE_VOID}, 0, &VoidCellFree    , NULL,  &VoidEval },

{ INSTR_COMPILER,  "compiler", "",       {TYPE_VOID, TYPE_ANY, TYPE_ANY}, 0, &VoidCellFree    , NULL,  &VoidEval },
{ INSTR_CODE_END,  "code_end", "",       {TYPE_VOID, TYPE_VOID, TYPE_VOID}, INSTR_OPTIONAL, &VoidCellFree    , &PrintInstr,  &VoidEval },		// end of BLK segment and start of data segment
{ INSTR_DATA_END,  "data_end", "",       {TYPE_VOID, TYPE_VOID, TYPE_VOID}, INSTR_OPTIONAL, &VoidCellFree    , &PrintInstr,  &VoidEval },		// end of data segment and start of variables segment
{ INSTR_SRC_END,   "src_end", "",        {TYPE_VOID, TYPE_VOID, TYPE_VOID}, INSTR_OPTIONAL, &VoidCellFree    , &PrintInstr,  &VoidEval },		// end of BLK segment and start of data segment
{ INSTR_DECL,      "decl",    "decl",    {TYPE_VOID, TYPE_ANY, TYPE_VOID}, 0, &VoidCellFree    , &PrintInstr,  &VoidEval },					// declare variable

{ INSTR_VAR,       "", "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0, &VarCellFree, &VarCellPrint,  &VarEval },			// Variable
{ INSTR_INT,       "", "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0, &IntCellFree, &IntCellPrint, &SelfEval },			// Integer constant
{ INSTR_ELEMENT,   "", ".", {TYPE_VOID, TYPE_ANY, TYPE_ANY}, 0, &VoidCellFree    , NULL,  &VoidEval },			// <array> <index>     access array or structure element (left operand is array, right is index)
{ INSTR_BYTE,      "", "$", {TYPE_VOID, TYPE_ANY, TYPE_ANY}, 0, &VoidCellFree    , NULL,  &VoidEval },			// <var> <byte_index>  access byte of specified variable
{ INSTR_RANGE,     "", "..", {TYPE_VOID, TYPE_ANY, TYPE_ANY}, 0, &VoidCellFree   , &PrintRange,  &RangeEval },			// x..y  (l = x, r = y) Used for slice array references
{ INSTR_TUPLE,     "", ",", {TYPE_VOID, TYPE_ANY, TYPE_ANY}, 0, &VoidCellFree    , NULL,  &VoidEval },			// { INSTR_LIST   (var may be another tuple)
{ INSTR_DEREF,     "", "@", {TYPE_VOID, TYPE_ANY, TYPE_VOID}, 0, &VoidCellFree    , NULL,  &VoidEval },			// dereference an address (var contains reference to dereferenced adr variable, type is type in [adr of type]. Byte if untyped adr is used.
{ INSTR_ITEM,      "", "#", {TYPE_VOID, TYPE_ANY, TYPE_VOID}, 0, &VoidCellFree    , &PrintBinaryOp,  &ItemEval },			// access field of structure
{ INSTR_TYPE,      "", "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0, &VoidCellFree    , NULL,  &VoidEval },
{ INSTR_SCOPE,     "", "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0, &VoidCellFree    , NULL,  &VoidEval },
{ INSTR_BIT,       "", "%", {TYPE_VOID, TYPE_ANY, TYPE_ANY}, 0, &VoidCellFree    , NULL,  &VoidEval },			// <var> <bit_index>  access bits of specified variable
{ INSTR_TEXT,      "text", "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0, &TextCellFree, &TextCellPrint, &SelfEval },		// Text constant
{ INSTR_VARIANT,   "variant", "", {TYPE_VOID, TYPE_ANY, TYPE_ANY}, 0, &VoidCellFree    , NULL,  &VoidEval },		// one of the two values (set)
{ INSTR_NAME,      "name", "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0, &VoidCellFree    , NULL,  &VoidEval },		//
{ INSTR_ARRAY,     "array", "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0, &VoidCellFree    , NULL,  &VoidEval },		// Array constant
{ INSTR_SEQUENCE,  "sequence", "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0, &VoidCellFree    , NULL,  &VoidEval },	// Sequence
{ INSTR_EMPTY,     "()", "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0, &VoidCellFree    , NULL,  &VoidEval },		// Empty
{ INSTR_MATCH,     ":", ":", {TYPE_VOID, TYPE_ANY, TYPE_ANY}, 0, &VoidCellFree    , &PrintMatch,  &VoidEval },		    // Match x:type
{ INSTR_VAL,	   "val", "", {TYPE_VOID, TYPE_ANY, TYPE_ANY}, 0, &VoidCellFree    , NULL,  &VoidEval },		    // Match const x:type
{ INSTR_ARRAY_TYPE,":array", "", {TYPE_VOID, TYPE_ANY, TYPE_ANY}, INSTR_IS_TYPE, &VoidCellFree    , NULL,  &VoidEval },
{ INSTR_FN_TYPE,   ":fn", "", {TYPE_VOID, TYPE_ANY, TYPE_ANY}, INSTR_IS_TYPE, &VoidCellFree    , NULL,  &VoidEval },
{ INSTR_ANY,       "", "?", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, INSTR_IS_TYPE, &VoidCellFree    , NULL,  &VoidEval },
{ INSTR_USES,      "uses", "?", {TYPE_ANY, TYPE_ANY, TYPE_VOID}, 0, &VoidCellFree    , NULL,  &VoidEval },
{ INSTR_POWER,     "power", "^", {TYPE_ANY, TYPE_ANY, TYPE_ANY}, INSTR_OPERATOR, &VoidCellFree    , &PrintBinaryOp,  &PowerEval },
{ INSTR_MEMORY,    "@", "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0, &VoidCellFree    , NULL,  &VoidEval },
{ INSTR_SRC_FILE,  "", "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0, &VoidCellFree    , &SrcFilePrint,  &VoidEval },			//{ INSTR_SRC_FILE variable representing source file
{ INSTR_PRINT,     "print",  "print", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0, &VoidCellFree    , NULL,  &VoidEval },		// print
{ INSTR_CODE,     "code",  "code", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0, &VoidCellFree    , &CodeCellPrint,  &VoidEval },		// instr
{ INSTR_SYMBOL,   "\'", "", {TYPE_VOID, TYPE_VOID, TYPE_VOID}, 0, &TextCellFree, &SymbolCellPrint, &SelfEval },		// Text constant
};


Cell * SelfEval(Cell * cell)
{
	return cell;
}

Cell * VoidEval(Cell * cell)
{
	return NULL;
}

void VoidCellFree(Cell * cell)
{

}

Var * VarGen(Var * t)
/*
Purpose:
	Alloc variable according to specified template.
*/
{
	Var * var = NULL;
	Var * name;
	Var * scope;
	if (t == NULL) return NULL;
	// %A.element
	if (t->mode == INSTR_ELEMENT) {
		scope = VarGen(t->l);
		name = NewVar(scope, t->r->str, NULL);
//		var = NewCell(INSTR_VAR);
//		name->var = var;
	} else if (CellIsConst(t)) {
		var = t;
	} if (VarIsRuleArg(t)) {
		var = MACRO_ARG[VarArgIdx(t)-1];
	}
	return var;
}

Type * TypeGen(Type * t)
{
	Type * type;
	Var * var, * idx;
	if (t == NULL) return t;

	type = t;
	if (t->mode == INSTR_ARRAY_TYPE) {
		type = NewArrayType(TypeGen(t->index), TypeGen(t->element));
		SetArrayStep(type, ArrayStep(t));
	} else {
		var = t;
		if (var->mode == INSTR_ELEMENT) {
			if (VarIsRuleArg(var->l)) {
				idx = MACRO_ARG[VarArgIdx(var->l)-1];
				type = idx->type;
			} else {
				type = var->type;
			}
			idx = var->r;
			if (StrEqual(idx->str, "idx")) {
				type = type->index;
			}
		} else {
			return var->type;
		}
	}
	return type;
}

Bool IsGoto(Instr * i)
{
	return i != NULL && i->op == INSTR_IF && IsEqual(i->arg1, ONE);
}

void InstrDecl(Instr * i)
{
	Type * type;
	Var * var, * proto;
	proto = i->arg1;
	type = TypeGen(proto->type);
	var = VarGen(i->arg1);
	var->type = type;
}


void InstrExecute(InstrBlock * blk)
{
	Instr * i;
	InstrInfo * ii;
	while(blk != NULL) {
		for(i = blk->first; i != NULL; i = i->next) {
			ii = &INSTR_INFO[i->op];
			ASSERT("TODO!!!!");
//			ii->intf->execute_fn(i);
		}
		blk = blk->next;
	}
}

/*

Instruction generating function always write to this block.
Blocks may be nested, because procedures may be nested.

*/

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
		eq = IsEqual(i->result, i2->result);
	}

	// Arguments must be same

	if (eq && INSTR_INFO[i->op].arg_type[ARG1] != TYPE_VOID) {
		eq = IsEqual(i->arg1, i2->arg1);
	}

	if (eq && INSTR_INFO[i->op].arg_type[ARG2] != TYPE_VOID) {
		eq = IsEqual(i->arg2, i2->arg2);
	}

	// Commutative instruction may be equivalent, if the arguments are switched
	if (!eq && FlagOn(INSTR_INFO[i->op].flags, INSTR_COMMUTATIVE)) {
		eq = IsEqual(i->arg1, i2->arg2) && IsEqual(i->arg2, i2->arg1);
	}
	
	return eq;
}

InstrOp InstrFind(char * name)
{
	UInt16 i;
	InstrInfo * ii = &INSTR_INFO[1];

	ASSERT(INSTR_NULL == 0);

	for(i=1; i<= INSTR_CNT; i++, ii++) {
		if (ii->name != NULL && StrEqual(name, ii->name)) {
			ASSERT(i == ii->op);
			return (InstrOp)i;
		}
	}

	return INSTR_NULL;
}

char * OpSymbol(InstrOp op)
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

Instr * InstrInsert(InstrBlock * blk, Instr * before, InstrOp op, Var * result, Var * arg1, Var * arg2)
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
	i->line = NULL;
	i->line_pos = 0;

	InstrAttach(blk, before, i, i);

	return i;
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
	case INSTR_LE: op = INSTR_GE; break;
	case INSTR_GE: op = INSTR_LE; break;
	case INSTR_GT: op = INSTR_LT; break;
	case INSTR_LT: op = INSTR_GT; break;
	default: break;
	}
	return op;
}

Var * InstrEvalConst(InstrOp op, Var * arg1, Var * arg2)
/*
Purpose:
	Try to evaluate instruction.
	Instruction may be evaluated if it's arguments are constant or some algebraic simplification may be applied.
Result:
	Return result as an variable or NULL, if instruction can not be evaluated.
*/
{
	Var * r = NULL;
	BigInt * n1, * n2;
	BigInt nr;

	/*
	==============================
	Optimization: Constant folding
	==============================

	Evaluate the expressions, whose operands are known to be constant at compile time.

	*/

	n1 = IntFromCell(arg1);
	n2 = IntFromCell(arg2);

	if (n1 != NULL && (arg2 == NULL || n2 != NULL)) {
		
		switch(op) {
			case INSTR_SQRT:
				if (IntHigherEq(n1, Int0())) {
					IntSqrt(&nr, n1);
				} else {
					SyntaxError("sqrt of negative number");
					// Error: square root of negative number
				}
				break;
			case INSTR_LO:
				IntAnd(&nr, n1, Int255());
				break;
			case INSTR_HI:
				IntSet(&nr, n1);
				IntDivN(&nr, 256);
				IntAndN(&nr, 0xff);
				break;
			case INSTR_DIV:
				if (!IntEqN(n1, 0)) {
					IntDiv(&nr, n1, n2);
				} else {
					SyntaxError("division by zero");
				}
				break;
			case INSTR_MOD:
				if (!IntEqN(n1, 0)) {
					IntMod(&nr, n1, n2);
				} else {
					SyntaxError("division by zero");
				}
				break;
			case INSTR_MUL:
				IntMul(&nr, n1, n2);
				break;
			case INSTR_ADD:
				IntAdd(&nr, n1, n2);
				break;
			case INSTR_SUB:
				IntSub(&nr, n1, n2);
				break;
			case INSTR_AND:
				IntAnd(&nr, n1, n2);
				break;
			case INSTR_OR:
				IntOr(&nr, n1, n2);
				break;
			case INSTR_XOR:
				IntXor(&nr, n1, n2);
				break;
			default: 
				return NULL;
				break;
		}
		r = IntCell(&nr);
		IntFree(&nr);
	}
	return r;
}

Var * InstrEvalAlgebraic(InstrOp op, Var * arg1, Var * arg2)
{
	/*
	=======================================
	Optimization: Algebraic simplifications
	=======================================

	::::::::::::::::::::::
	add x, a, 0  ->  let x, a
	add x, 0, a  ->  let x, a
	sub x, a, 0  ->  let x, a
	mul x, a, 1  ->  let x, a
	mul x, 1, a  ->  let x, a
	div x, a, 1  ->  let x, a
	::::::::::::::::::::::

	*/
	switch(op) {
	case INSTR_MUL:
		if (CellIsN(arg1, 1)) 
			return arg2;
		// continue to INSTR_DIV
	case INSTR_DIV:
		if (CellIsN(arg2, 1)) return arg1;
		break;

	case INSTR_ADD:
		if (CellIsN(arg1, 0)) return arg2;
		// continue to INSTR_SUB
	case INSTR_SUB:
		if (CellIsN(arg2, 0)) return arg1;
		break;

	case INSTR_SQRT:
		if (CellIsN(arg1, 1) || CellIsN(arg1, 0)) return arg1;
		break;
	default: break;
	}

	return InstrEvalConst(op, arg1, arg2);
}


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
	return VarUsesVar(i->arg1, i->result) || VarUsesVar(i->arg2, i->result);

//	return VarModifiesVar(i->result, i->arg1) || VarModifiesVar(i->result, i->arg2);
}


void ForEachBlock(InstrBlock * blk, ProcessBlockFn process_fn, void * info)
{
	while(blk != NULL) {
		process_fn(blk, info);
		blk = blk->next;
	}
}

Bool ProcInstrEnum(Var * proc, Bool (*fn)(Loc * loc, void * data), void * data)
{
	Instr * i;
	Instr * next_i;
	InstrBlock * blk;
	Loc loc;

	loc.proc = proc;

	for(blk = FnVarCode(proc); blk != NULL; blk = blk->next) {
		loc.blk = blk;
		loc.n = 1;
		for(i = blk->first; i != NULL; i = next_i) {
			next_i = i->next;
			loc.i = i;
			if (fn(&loc, data)) return true;
			loc.n++;
		}
	}
	return false;
}

InstrBlock * IfInstr(Instr * i)
{
	ASSERT(i->op == INSTR_IF);
	return i->arg2;
}
