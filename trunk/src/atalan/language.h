#include "../common/common.h"

typedef struct VarTag Var;
typedef struct LocTag Loc;
typedef struct VarSetTag VarSet;
typedef struct RuleTag Rule;
typedef struct RuleArgTag RuleArg;

Bool Verbose(Var * proc);

/*************************************************************

 Lexer

*************************************************************/

typedef UInt16 LineNo;
typedef UInt16 LinePos;

typedef enum {
	TOKEN_VOID = -2,
	TOKEN_EOF  = -1,
	TOKEN_ERROR = 0,
	TOKEN_BLOCK_END,
	TOKEN_OUTDENT,
	TOKEN_ID,
	TOKEN_INT,
	TOKEN_STRING,

	// Single character tokens (any ascii character is token)

	TOKEN_EOL = 10,
	TOKEN_ADR = '@',
	TOKEN_EQUAL = '=',
	TOKEN_HIGHER = '>',
	TOKEN_LOWER = '<',
	TOKEN_COMMA = ',',
	TOKEN_COLON = ':',
	TOKEN_PERCENT = '%',
	TOKEN_OPEN_P  = '(',
	TOKEN_CLOSE_P = ')',
	TOKEN_PLUS = '+',
	TOKEN_MINUS = '-',
	TOKEN_MUL = '*',
	TOKEN_DIV = '/',
	TOKEN_DOT = '.',
	TOKEN_HASH = '#',
	TOKEN_DOLLAR = '$',
	TOKEN_BYTE_INDEX = TOKEN_DOLLAR,

	// Keyword tokens

	TOKEN_KEYWORD = 128,
	TOKEN_GOTO    = 128,
	TOKEN_IF,
	TOKEN_UNLESS,
	TOKEN_THEN,
	TOKEN_ELSE,
	TOKEN_PROC,
	TOKEN_RULE,
	TOKEN_MACRO,
	TOKEN_AND,
	TOKEN_OR,
	TOKEN_NOT,
	TOKEN_SQRT,
	TOKEN_WHILE,
	TOKEN_UNTIL,
	TOKEN_WHERE,
	TOKEN_CONST,
	TOKEN_ENUM,
	TOKEN_ARRAY,
	TOKEN_TYPE2,			// this should be TOKEN_TYPE, but there was conflict with windows.h include on Windows
	TOKEN_FILE,
	TOKEN_LO,
	TOKEN_HI,
	TOKEN_OF,
	TOKEN_FOR,
	TOKEN_IN,
	TOKEN_OUT,
	TOKEN_PARAM,
	TOKEN_INSTR,
	TOKEN_TIMES,
	TOKEN_ADR2,
	TOKEN_DEBUG,
	TOKEN_MOD,
	TOKEN_BITNOT,
	TOKEN_BITAND,
	TOKEN_BITOR,
	TOKEN_BITXOR,
	TOKEN_STRUCT,
	TOKEN_USE,
	TOKEN_REF,
	TOKEN_STEP,
	TOKEN_RETURN,
	TOKEN_SCOPE,
	TOKEN_SEQUENCE,
	TOKEN_ASSERT,
	TOKEN_LAST_KEYWORD = TOKEN_ASSERT,

	// two character tokens
	TOKEN_LOWER_EQUAL,
	TOKEN_HIGHER_EQUAL,
	TOKEN_NOT_EQUAL,
	TOKEN_DOTDOT,
	TOKEN_RIGHT_ARROW

} Token;

#define KEYWORD_COUNT (TOKEN_LAST_KEYWORD - TOKEN_KEYWORD + 1)

typedef struct {
	FILE * file;
	char * line;
	char * prev_line;
	LineNo line_no;
	LinePos line_len;
	LinePos line_pos;
	Token   token;
	Int16   prev_char;
} ParseState;

typedef struct {
//	char   name[256];
	UInt32   n;
	FILE * f;
	Bool   ignore_keywords;
} Lexer;

Bool SrcOpen(char * name, Bool parse_options);
void SrcClose();
FILE * FindFile(char * name, char * ext, char * path);

extern char NAME[256];

// This functions are used only by parser

void NextToken();
void NextStringToken();
void ExpectToken(Token tok);

UInt16 SetBookmark();

Bool Spaces();
Bool NextCharIs(UInt8 chr);
Bool NextIs(Token tok);
void EnterBlock();
void EnterBlockWithStop(Token stop_token);
void ExitBlock();

void LexerInit();

#define MAX_LINE_LEN 32767

extern Lexer LEX;
extern Token TOK;
extern char   LINE[MAX_LINE_LEN+2];		// we reserve one extra byte for terminating EOL, one for 0
extern LineNo  LINE_NO;
extern UInt16  LINE_LEN;
extern UInt16  LINE_POS;
extern UInt16  TOKEN_POS;
extern char * PREV_LINE;
extern Var *  SRC_FILE;					// current source file
extern char PROJECT_DIR[MAX_PATH_LEN];
extern char SYSTEM_DIR[MAX_PATH_LEN];
char FILE_DIR[MAX_PATH_LEN];			// directory where the current file is stored
char FILENAME[MAX_PATH_LEN];
extern char PLATFORM[64];

typedef enum {
	INSTR_NULL = 0,
	INSTR_VOID,
	INSTR_LET,		// var, val

	INSTR_GOTO,
	INSTR_IFEQ,		// must be even!!!.
	INSTR_IFNE,
	INSTR_IFLT,
	INSTR_IFGE,
	INSTR_IFGT,
	INSTR_IFLE,
	INSTR_IFOVERFLOW,
	INSTR_IFNOVERFLOW,

	INSTR_PROLOGUE,
	INSTR_EPILOGUE,
	INSTR_EMIT,
	INSTR_VARDEF,
	INSTR_LABEL,
	INSTR_ADD,
	INSTR_SUB,
	INSTR_MUL,
	INSTR_DIV,
	INSTR_SQRT,

	INSTR_AND,
	INSTR_OR,

	INSTR_ALLOC,
	INSTR_PROC,
	INSTR_RETURN,
	INSTR_ENDPROC,
	INSTR_CALL,
	INSTR_VAR_ARG,
	INSTR_STR_ARG,			// generate str
	INSTR_DATA,
	INSTR_FILE,
	INSTR_ALIGN,
	INSTR_ORG,				// set the destination address of compilation
	INSTR_HI,
	INSTR_LO,
	INSTR_PTR,
	INSTR_ARRAY_INDEX,		// generate index for array
	INSTR_LET_ADR,
	INSTR_ROL,				// bitwise rotate right
	INSTR_ROR,				// bitwise rotate left
	INSTR_DEBUG,
	INSTR_MOD,
	INSTR_XOR,
	INSTR_NOT,
	INSTR_ASSERT_BEGIN,
	INSTR_ASSERT_END,

	INSTR_LINE,				// reference line in the source code
	INSTR_INCLUDE,
	INSTR_MULA,				// templates for 8 - bit multiply 
	INSTR_MULA16,           // templates for 8 - bit multiply 

	INSTR_COMPILER,
	INSTR_CODE_END,			// end of BLK segment and start of data segment
	INSTR_DATA_END,			// end of data segment and start of variables segment

	// Following 'instructions' are used in expressions
	INSTR_VAR,				// Variable (may be argument, input, output, ...)
	INSTR_CONST,			// Constant (depending on type)
	INSTR_ELEMENT,			// <array> <index>     access array or structure element (left operand is array, right is index)
	INSTR_BYTE,				// <var> <byte_index>  access byte of specified variable
	INSTR_RANGE,			// x..y  (l = x, r = y) Used for slice array references
	INSTR_TUPLE,			// INSTR_LIST <adr,var>  (var may be another tuple)
						    // Type of tuple may be undefined, or it may be structure of types of variables in tuple
	INSTR_DEREF,			// dereference an address (var contains reference to dereferenced adr variable, type is type in [adr of type]. Byte if untyped adr is used.
	INSTR_FIELD,			// access field of structure
	INSTR_TYPE,
	INSTR_SCOPE,
	INSTR_SRC_FILE,			//INSTR_SRC_FILE variable representing source file
							// scope   FILE that includes (uses) this file
							// name    filename
							// n       parse state

	INSTR_CNT
} InstrOp;


/*********************************************************

  Error reporting

**********************************************************/

extern UInt32 ERROR_CNT;
extern UInt32 LOGIC_ERROR_CNT;

void ErrArgClear();
void ErrArg(Var * var);
void SyntaxErrorBmk(char * text, UInt16 bookmark);
void SyntaxError(char * text);
void LogicWarning(char * text, UInt16 bookmark);
void LogicWarningLoc(char * text, Loc * loc);
void LogicError(char * text, UInt16 bookmark);
void LogicErrorLoc(char * text, Loc * loc);
void InternalError(char * text, ...);
void Warning(char * text);
void EndErrorReport();

void InitErrors();

/*********************************************************

 Variables & types

*********************************************************/
//$V

typedef struct TypeTag Type;
typedef struct InstrTag Instr;
typedef struct ExpTag Exp;
typedef struct InstrBlockTag InstrBlock;

typedef enum {
	TYPE_VOID = 0,
	TYPE_INT,
	TYPE_STRUCT,
	TYPE_PROC,
	TYPE_MACRO,
	TYPE_STRING,
	TYPE_ARRAY,
	TYPE_LABEL,		// label in code (all labels share same type
	TYPE_ADR,		// address (or reference)
	TYPE_VARIANT,
	TYPE_TUPLE,
	TYPE_UNDEFINED,
	TYPE_SCOPE,
	TYPE_SEQUENCE,	// numeric sequence
	TYPE_ANY
} TypeVariant;

/*
TYPE_SEQUENCE

Step type is used in type inferring.
It describes how is a variable modified in loop, when type can not be directly defined.
It is partial type. It cannot be used to define.

*/

typedef struct {
	InstrOp op;			// step_type INSTR_ADD, INSTR_SUB, INSTR_MUL, INSTR_DIV, ...
	Type * step;		// type of argument (step value)
	Type * init;		// initial value of the step
	InstrOp compare_op;
	Type * limit;		// limit value of step (for ADD, MUL this is top value, for SUB, DIV this is bottom value)
} TypeSequence;

typedef Int16 Relation;

typedef Int32 IntLimit;
#define INTLIMIT_MIN (-2147483647 - 1)		// To prevent error in some compilers parser
#define INTLIMIT_MAX 2147483647L


// min + N * mul  (<max)
typedef struct {
	Bool flexible;		// range has been fixed by user
	IntLimit min;
	IntLimit max;
//	UInt32 mul;
} Range;

#define MAX_DIM_COUNT 2
#define MACRO_ARG_CNT 26

#define TypeUsed 1

struct TypeTag {
	TypeVariant  variant;	// int, struct, proc, array
	UInt16       flags;
	Bool         flexible;	// (read: inferenced)
	Bool		 is_enum;	// INSTR_INTEGER is enum
	Type * base;			// type, on which this type is based (may be NULL)
	Var * owner;			// original owner of this type
	union {
		Range range;
		// TYPE_ARRAY, TYPE_ADR
		struct {
			Type * dim[MAX_DIM_COUNT];		// array dimension types (integer ranges)
			Type * element;
			UInt16 step;					// Index will be multiplied by this value. Usually it is same as element size.
		};

		TypeSequence seq;
	};
};

/*

INSTR_DEREF

DEREF variables represent dereference of address variable.
Only address variables may be dereferenced.
For every adr variable, there may be at most one dereference variable.

If the dereferenced variable is of type ADR OF X, deref variable is of type X.

*/

typedef enum {
	SUBMODE_EMPTY = 0,

	// INSTR_VAR
	SUBMODE_IN = 1,
	SUBMODE_OUT = 2,		// even procedure may be marked as out. In such case, it has some side-effect.
	SUBMODE_REG = 4,		// variable is stored in register
	SUBMODE_IN_SEQUENCE = 8,
	SUBMODE_ARG_IN  = 16,
	SUBMODE_ARG_OUT = 32,
	SUBMODE_OUT_SEQUENCE = 64,

	// General
	SUBMODE_SYSTEM = 128,				// This is system variable (defined either by system or platform)
	SUBMODE_USER_DEFINED = 256,			// Type of this variable has been explicitly defined by user (programmer)

	// INSTR_SRC_FILE
	SUBMODE_MAIN_FILE = 4,	// this flag is set for main source file (asm is not included for main file, because it is generated by compiler)

	// INSTR_CONST
	SUBMODE_PARAM    = SUBMODE_IN,
	SUBMODE_UNDEFINED = SUBMODE_OUT		// undefined constant or option (neither constant nor option may be IN or OUT)
} VarSubmode;

//REF
//		Value of ptr is address
//
//
//Element is variable, that represents value in array i.e.  a(1)
//For multiple indices, elements are nested? a(1,2)  -> elmt(elmt(a, 1),2)
//Elements are practically always temporary.


#define VarLive            1
#define VarDead			   0		// this is in fact just 0 state of VarLive, it cannot be tested
#define VarUninitialized   2
#define VarLoop            4		// loop variable (incremented during loop)
#define VarLoopDependent   8		// variable is dependent (even transitively) on some loop variable
#define VarProcessed       16		// used when detecting, whether procedure is used or not

#define VarProcInterrupt   32		// this procedure is used from interrupt
#define VarProcAddress     64		// procedure address is required (this means, we are not allowed to inline it)

#define VarUsed            16		// for register allocation

#define VarLabelDefined    32

typedef unsigned int VarIdx;
typedef char * Name;

typedef UInt8 VarFlags;

struct VarTag {

	// Variable identification (name,idx,scope)
	Name	name;
	VarIdx  idx;	 // two variables with same name but different index may exist
					 // 0 means no index, 1 means index 1 etc.
					 // variable name "x1" is automatically converted to x,1
	Var  *  scope;	 // scope, in which this variable has been declared
	InstrOp	mode;
	VarSubmode submode;

	VarFlags  flags;
	Var *	adr;	 // Address of variable in memory. For INSTR_TYPE, this means alignment of variable of this type.
					 // INSTR_ELEMENT	Array to which this variable belongs
					 // INSTR_TUPLE      First variable of tuple

	Type *  type;	 // Pointer to type variable (such variable must have INSTR_TYPE)
					 // INSTR_ELEMENT:  Element type of array

	int     value_nonempty;
	// TODO: Replace value_nonempty just with flag VarDefined
	union {
		long	n;				// for const, or function default argument (other variants of value must be supported - array, struct, etc.)
		InstrBlock * instr;		// instructions for procedure or array initialization
		char * str;
		Var * var;
		ParseState * parse_state;	// INSTR_SRC_FILE
	};

	Var *   file;			// file in which the variable has been defined
	LineNo  line_no;		// line of number, on which the variable has been defined
	LinePos line_pos;		// position on line at which the variable has been declared

	Var *   current_val;	// current value assigned during optimization
							// TODO: Maybe this may be variable value.
	Instr * src_i;
	Exp *   dep;
	UInt16	read;			// how many times some instruction reads this variable (if 0, this is unused)
	UInt16	write;			// how many times some instruction writes this variable (if 1 this is constant)
	Var  *  next;			// next variable in chain
};

/*

Variable address
================

Address of variable may be:

INSTR_CONST          Integer defining location of variable in main memory.
MOVE_VAR            This variable is alias for the variable specified in adr.
INSTR_TUPLE          List of variables. One bigger variable may be defined as list of smaller variables.

INSTR_LABEL          Address.
                    Address alone may have address, which specifies memory bank, in which the address should be located.
					Address may be named. (For example labels).

*/

#define MAX_ARG_COUNT 128

/*
INSTR_LINE is special instruction, which does not affect execution of program.
It marks place, where next line in source code begins.
All instructions after INSTR_LINE until next INSTR_LINE instruction were generated as a result
of parsing the line specified in instruction.

To save memory, INSTR_LINE argument are not pointers to ordinary variables.
result     pointer to INSTR_SRC_FILE variable, that defines source file
arg1       integer line number.
arg2       pointer to text of line


*/

typedef struct MemBlockTag MemBlock;

struct MemBlockTag {
	UInt32 adr;
	UInt32 size;
};

typedef struct MemHeapTag MemHeap;

struct MemHeapTag {
	UInt32 count;
	UInt32 capacity;
	MemBlock * block;
};

void HeapInit(MemHeap * heap);
void HeapCleanup(MemHeap * heap);
void HeapAddBlock(MemHeap * heap, UInt32 adr, UInt32 size);
void HeapRemoveBlock(MemHeap * heap, UInt32 adr, UInt32 size);
Bool HeapAllocBlock(MemHeap * heap, UInt32 size, UInt32 * p_adr);
void HeapAddType(MemHeap * heap, Type * type);

void HeapUnitTest();
void HeapPrint(MemHeap * heap);

#define DATA_SEGMENT          0x1000000
#define DATA_SEGMENT_CAPACITY 0x1000000

// Variables that do not fit into defined variable space are stored in data segment.
// Data segment is allocated directly after BLK segment, however we reserve dynamic space for it at
// specified address.


/*************************************************************

  Type

*************************************************************/

void TypeInit();		// initialize the Type subsytem

Type * TypeAlloc(TypeVariant variant);
Type * TypeAllocInt(Int32 min, Int32 max);
Type * TypeDerive(Type * base);
Type * TypeCopy(Type * base);

Type * TypeByte();
Type * TypeLongInt();
Type * TypeScope();
Type * TypeTuple();

Type * TypeAdrOf(Type * element);

UInt16 TypeItemCount(Type * type);
void TypeLimits(Type * type, Var ** p_min, Var ** p_max);

//void TypeLet(Type * type, Var * var);
typedef void (*RangeTransform)(Int32 * x, Int32 tr);
//void TypeTransform(Type * type, Var * var, InstrOp op);

void TypeAddConst(Type * type, Var * var);
Bool TypeIsSubsetOf(Type * type, Type * master);
Bool TypeIsBool(Type * type);

UInt32 TypeSize(Type * type);
UInt32 TypeAdrSize();
UInt32 TypeStructAssignOffsets(Type * type);

void ArraySize(Type * type, Var ** p_dim1, Var ** p_dim2);

//--- Proc type
void ProcTypeFinalize(Type * proc);


Bool VarMatchType(Var * var, Type * type);
Bool VarMatchesType(Var * var, Type * type);

void TypeInfer(Var * proc);

extern Type TVOID;
extern Type TINT;		// used for int constants
extern Type TSTR;
extern Type TLBL;
extern Type * TUNDEFINED;

#define NO_SCOPE ((Var *)1)

void VarInit();
void InitCPU();

Var * VarFirst();
#define VarNext(v) (v)->next

Var * InScope(Var * new_scope);
void ReturnScope(Var * prev);

void EnterLocalScope();
//void EnterSubscope(Var * new_scope);
void ExitScope();

Var * VarNewInt(long n);
Var * VarNewStr(char * str);
Var * VarNewLabel(char * name);
Var * FindOrAllocLabel(char * name, UInt16 idx);

void VarLetStr(Var * var, char * str);

Var * VarNewTmp(long idx, Type * type);
Var * VarNewTmpLabel();
Var * VarAlloc(InstrOp mode, Name name, VarIdx idx);
Var * VarAllocScope(Var * scope, InstrOp mode, Name name, VarIdx idx);
Var * VarAllocScopeTmp(Var * scope, InstrOp mode, Type * type);
Var * VarFind(Name name, VarIdx idx);
Var * VarFindScope(Var * scope, char * name, VarIdx idx);
Var * VarFindScope2(Var * scope, char * name);
Var * VarFind2(char * name);
//Var * VarFindInProc(char * name, VarIdx idx);
Var * VarProcScope();
Var * VarFindTypeVariant(Name name, VarIdx idx, TypeVariant type_variant);

Var * VarFindInt(Var * scope, UInt32 n);
Var * VarMacroArg(UInt8 i);

Bool VarIsConst(Var * var);
Bool VarIsIntConst(Var * var);
Bool VarIsN(Var * var, Int32 n);
Bool VarIsLabel(Var * var);
Bool VarIsArray(Var * var);
Bool VarIsTmp(Var * var);
Bool VarIsStructElement(Var * var);
Bool VarIsArrayElement(Var * var);
Bool VarIsReg(Var * var);
Var * VarReg(Var * var);
Bool VarIsFixed(Var * var);
Bool VarIsLocal(Var * var, Var * scope);
Bool VarIsInArg(Var * var);
Bool VarIsOutArg(Var * var);
Bool VarIsArg(Var * var);
Bool VarIsEqual(Var * left, Var * right);


Var * VarField(Var * var, char * fld_name);
void VarLet(Var * var, Var * val);

#define InVar(var)  FlagOn((var)->submode, SUBMODE_IN)
#define OutVar(var) FlagOn((var)->submode, SUBMODE_OUT)

UInt32 VarByteSize(Var * var);

void VarResetRegUse();

TypeVariant VarType(Var * var);
long VarParamCount(Var * var);

void VarGenerateArrays();
void VarToLabel(Var * var);

Var * VarNewType(TypeVariant variant);

Var * FirstArg(Var * proc, VarSubmode submode);
Var * NextArg(Var * proc, Var * arg, VarSubmode submode);

Var * VarFirstLocal(Var * scope);
Var * VarNextLocal(Var * scope, Var * local);

Var * VarNewElement(Var * arr, Var * idx);
Var * VarNewByteElement(Var * arr, Var * idx);
Var * VarNewDeref(Var * var);
Var * VarNewRange(Var * min, Var * max);
Var * VarNewTuple(Var * left, Var * right);
Var * VarNewOp(InstrOp op, Var * left, Var * right);

Var * VarEvalConst(Var * var);

void VarResetUse();

void VarEmitAlloc();

#define FOR_EACH_VAR(v) for(v = VARS; v != NULL; v = v->next) {
#define NEXT_VAR }

#define FOR_EACH_LOCAL(SCOPE, VAR) 	for(VAR = VarFirstLocal(SCOPE); VAR != NULL; VAR = VarNextLocal(SCOPE, VAR)) {
#define NEXT_LOCAL }

#define FOR_EACH_ARG(SCOPE, VAR) 	for(VAR = VarFirstLocal(SCOPE); VAR != NULL; VAR = VarNextLocal(SCOPE, VAR)) { if (VarIsArg(VAR)) {
#define NEXT_ARG } }

#define FOR_EACH_IN_ARG(SCOPE, VAR) 	for(VAR = VarFirstLocal(SCOPE); VAR != NULL; VAR = VarNextLocal(SCOPE, VAR)) { if (VarIsInArg(VAR)) {
#define NEXT_IN_ARG } }

#define FOR_EACH_OUT_ARG(SCOPE, VAR) 	for(VAR = VarFirstLocal(SCOPE); VAR != NULL; VAR = VarNextLocal(SCOPE, VAR)) { if (VarIsOutArg(VAR)) {
#define NEXT_OUT_ARG } }

void PrintVar(Var * var);
void PrintVarName(Var * var);
void PrintVarUser(Var * var);
void PrintQuotedVarName(Var * var);
void PrintScope(Var * scope);

void ProcUse(Var * proc, UInt8 flag);

void ProcessUsedProc(void (*process)(Var * proc));

typedef struct {
	Var * key;
	Var * var;
} VarTuple;

typedef struct VarSetTag {
	VarTuple * arr;
	UInt16     count;
	UInt16     capacity;
};

void VarSetInit(VarSet * set);
Var * VarSetFind(VarSet * set, Var * key);
void VarSetAdd(VarSet * set, Var * key, Var * var);
Var * VarSetRemove(VarSet * set, Var * key);
void VarSetEmpty(VarSet * set);
void VarSetCleanup(VarSet * set);

Bool ProcIsInterrupt(Var * proc);

/***********************************************************

  CPU

***********************************************************/

#define MAX_CPU_REG_COUNT 64

typedef UInt8 RegIdx;

typedef struct {
	Var * SCOPE;
	Var * REG[MAX_CPU_REG_COUNT];		// Array of registers
	RegIdx REG_CNT;		// Count of registers
	Type * MEMORY;		// Array (adr) of cpu_word
} CPUType;

extern CPUType * CPU;

/***********************************************************

  Instructions

***********************************************************/

#define IS_INSTR_BRANCH(x) ((x)>=INSTR_IFEQ && (x)<=INSTR_IFNOVERFLOW)
#define IS_INSTR_JUMP(x) (IS_INSTR_BRANCH(x) || (x) == INSTR_GOTO)

InstrOp OpNot(InstrOp op);
InstrOp OpRelSwap(InstrOp op);

#define FlagExpProcessed 1

/*
 Expression represents tree of expressions.
 op defines operation used to compute the result of the expression.
 INSTR_VAR represents expression representing value of variable or reference to variable.
*/

struct ExpTag {
	UInt8     flags;
	InstrOp   op;			// operation
	union {
		Exp * arg[2];		// op != INSTR_VAR
		Var * var;			// op == INSTR_VAR
	};
};

/*

Compiler instruction.

We use three-address instructions. in the form  result = arg1 op arg2.

*/

struct InstrTag {
	InstrOp op;
	Rule *  rule;		// after translation, this is pointer to rule defining the operator (may be NULL for INSTR_LINE)

	//--- dest
	Var * result;

	//TODO: Merge the union to arg1,arg2 & line_no, line
	union {
		Var * arg1;
		UInt16 line_no;
	};
	union {
		Var * arg2;
		char * line;
	};

	// Position on line, on which is the token that generated the instruction.
	// If 0, it means the position is not specified (previous token should be used)

	LinePos  line_pos;

	// Type of result computed by this instruction
	// This type may differ from type defined in instruction result variable 
	// (it may be it's subset).
	// For example in case of LET x, 10 instruction, type in result_type will be 10..10, even if type of
	// x variable may be 0..255.

	// TODO: type may be union with next_use (they are not used at the same time)

	Type * type[3];				// 0 result type, 1 arg1 type 2 arg2 type

	UInt8    flags;
	Instr * next_use[3];		// next use of result, arg1, arg2

	Instr * next, * prev;
};

// Instruction flags
#define InstrRestriction 1		// result type in the instruction is actually an restriction

struct LocTag {
	Var * proc;
	InstrBlock * blk;
	Instr * i;
};

typedef struct {
	Loc defs[64];
	UInt16 count;
} Defs;

void DefsAdd(Defs * defs, InstrBlock * blk, Instr * i);
void DefsInit(Defs * defs);

void ReachingDefs(Var * proc, Var * var, Loc * loc, Defs * defs);

#define InstrInvariant 1
#define InstrLoopDep 2

/*
For instructions, where arg1 or arg2 = result, source points to instruction, that previously set the result.

1.		ror x,x,1
2.		ror x,x,1   source = 1
3.		ror x,x,1   source = 2
4.      let a,x

*/

struct InstrBlockTag {

	InstrBlock * next;			// Blocks are linked in chain, so we can traverse them as required.
								// This is normally in order, in which the blocks were parsed.
	UInt32 seq_no;				// Block sequence number. It is used to determine order of blocks when detecting loops.

	InstrBlock * to;			// this block continues (jumps) to this block (if to == NULL, we leave the routine after the last instruction in the block)
	InstrBlock * cond_to;		// last instruction conditionally jumps to this block if the condition if true

	InstrBlock * from;			// we may come to this block from here

	InstrBlock * callers;		// list of blocks calling this block (excluding from)
	InstrBlock * next_caller;	// next caller in the chain

	InstrBlock * loop_end;

	Var * label;				// label that starts the block
	Bool  processed;
	Type * type;				// type computed in this block for variable when inferring types
	Instr * first, * last;		// first and last instruction of the block
};

InstrBlock * InstrBlockAlloc();

void InstrInit();
void InstrPrint(Instr * i);
void InstrPrintInline(Instr * i);
void InstrFree(Instr * i);

char * OpName(InstrOp op);

void PrintVarVal(Var * var);
void PrintProc(Var * proc);

Var * InstrFind(char * name);

// When we generate instructions, it is always done to defined code block
// Code blocks are managed using those procedures.


void InstrBlockFree(InstrBlock * blk);
UInt32 InstrBlockInstrCount(InstrBlock * blk);

void InstrMoveCode(InstrBlock * to, Instr * after, InstrBlock * from, Instr * first, Instr * last);
Instr * InstrDelete(InstrBlock * blk, Instr * i);
void InstrInsert(InstrBlock * blk, Instr * before, InstrOp op, Var * result, Var * arg1, Var * arg2);


extern Var * SCOPE;		// currently parsed variable (procedure, macro)
//extern Var * REGSET;	// enumerator with register sets

void CodePrint(InstrBlock * blk);

void InstrVarUse(InstrBlock * code, InstrBlock * end);
void VarUse();

Var * InstrEvalConst(InstrOp op, Var * arg1, Var * arg2);

UInt16 SetBookmarkLine(Loc * loc);
UInt16 SetBookmarkVar(Var * var);

void InstrReplaceVar(InstrBlock * block, Var * from, Var * to);
void ProcReplaceVar(Var * proc, Var * from, Var * to);

Bool InstrEquivalent(Instr * i, Instr * i2);
Bool InstrIsSelfReferencing(Instr * i);

// Instructions generating

void GenerateInit();
void GenSetDestination(InstrBlock * blk, Instr * i);
void GenBegin();
InstrBlock * GenEnd();

void GenInternal(InstrOp op, Var * result, Var * arg1, Var * arg2);
void Gen(InstrOp op, Var * result, Var * arg1, Var * arg2);
void GenRule(Rule * rule, Var * result, Var * arg1, Var * arg2);
void GenLet(Var * result, Var * arg1);
void GenLine();
void GenLabel(Var * var);
void GenGoto(Var * var);
void GenBlock(InstrBlock * blk);
void GenMacro(Var * macro, Var ** args);
void GenLastResult(Var * var, Var * item);
void GenArrayInit(Var * arr, Var * init);
void GenPos(InstrOp op, Var * result, Var * arg1, Var * arg2);

#define RESULT 0
#define ARG1   1
#define ARG2   2

#define INSTR_COMMUTATIVE 1

typedef struct {
	InstrOp  op;
	char * symbol;
	TypeVariant arg_type[3];		// 0 = result, 1 = arg1, 2 = arg2
	UInt8   flags;
} InstrInfo;

extern InstrInfo INSTR_INFO[INSTR_CNT];

/***********************************************************

  Translate

***********************************************************/

extern Var * RULE_PROC;

typedef enum {
	RULE_UNDEFINED,
	RULE_ANY = 0,		// any argument
	RULE_VARIABLE,		// (type) variable of specified type
	RULE_CONST,			// (type) constant matching specified type
	RULE_REGISTER,		// (var)  actual variable (typically used for register)
	RULE_VALUE,			// (n)    actual value
	RULE_DEREF,			// dereference of variable (dereferenced type is always adr)
	RULE_ARG,			// argument (type of argument is defined, may be NULL)
	RULE_ELEMENT,		// array element - var defines pattern for array, index defines pattern for index
	RULE_TUPLE,
	RULE_RANGE,
	RULE_BYTE,

	RULE_DESTINATION

} RuleArgVariant;

struct RuleArgTag {
	RuleArgVariant variant;
	UInt8          arg_no;		// argument index (1..26)  used for variable & const
	union {
	   Var * var;
	   Type * type;
	   RuleArg * arr;
	   UInt8   type_arg_no;		// number of macro argument defining a type
	};
	RuleArg  * index;		        // pointer to index for array (NULL if there is no index)
							        // Rule arg is allocated in this case
							        // This type must be RULE_VARIABLE or RULE_REGISTER
							        // Type of that variable must be TYPE_ARRAY
};

struct RuleTag {
	Rule * next;
	Var *  file;			// file in which the rule has been defined
	LineNo line_no;			// line in the source code, on which the rule was defined

	InstrOp op;
	RuleArg arg[3];
	InstrBlock * to;
	Var * flags;			// for instruction rule, this is variable with flag or flags (tuple) that are modified, when this instruction is executed
};

void RuleRegister(Rule * rule);
Bool RuleMatch(Rule * rule, Instr * i);

void ProcTranslate(Var * proc);
void CheckValues(Var * proc);
void TranslateInit();

// Garbage collector

void RulesGarbageCollect();
void TypeMark(Type * type);

/*************************************************************

 Parser

*************************************************************/
typedef struct BlockTag Block;
extern Bool  SYSTEM_PARSE;  // if set to true, we are parsing system information and line tokens do not get generated

struct BlockTag {
	Token command;		// TOK_IF, TOK_WHILE
	Var * body_label;
	Var * loop_label;
	Var * t_label;		// label, to which the condition jumps if it is true
	Var * f_label;		// label, to which the condition jumps if it is false
	Bool  not;			// set to true, if the condition should be negated
};

void ParseInit();
Bool Parse(char * name, Bool main_file, Bool parse_options);
void ProcCheck(Var * proc);

void BufEmpty();
void BufPush(Var * var);

#define STACK_LIMIT 100
Var *  STACK[STACK_LIMIT];
UInt16 TOP;
extern LinePos OP_LINE_POS;				// Position of last parsed binary operator

/*************************************************************

 Optimize phase

*************************************************************/

Bool VarUsesVar(Var * var, Var * test_var);
Bool VarModifiesVar(Var * var, Var * test_var);
Int16 VarTestReplace(Var ** p_var, Var * from, Var * to);
Int16 VarReplace(Var ** p_var, Var * from, Var * to);

Bool InstrUsesVar(Instr * i, Var * var);
Bool InstrSpill(Instr * i, Var * var);

UInt8 VarIsLiveInBlock(Var * proc, InstrBlock * block, Var * var);
void MarkBlockAsUnprocessed(InstrBlock * block);


void ProcOptimize(Var * proc);
void GenerateBasicBlocks(Var * proc);
Bool OptimizeLive(Var * proc);
Bool OptimizeValues(Var * proc);
Bool OptimizeVarMerge(Var * proc);
void OptimizeLoops(Var * proc);

void OptimizeJumps(Var * proc);
void DeadCodeElimination(Var * proc);
Bool OptimizeMergeBranchCode(Var * proc);

void ProcClearProcessed(Var * proc);
void AllocateVariables(Var * proc);

void OptimizeProcInline(Var * proc);

/*************************************************************

 Emit phase

*************************************************************/

void PrintHeader(char * text);
void PrintOptim(char * text);

Rule * InstrRule(Instr * instr);
Rule * InstrRule2(InstrOp op, Var * result, Var * arg1, Var * arg2);

Bool EmitOpen(char * filename);
void EmitClose();
Bool EmitInstr(Instr * code);
Bool EmitInstrOp(InstrOp op, Var * result, Var * arg1, Var * arg2); 
Bool EmitProc(Var * proc);
void EmitLabels();
void EmitProcedures();
void EmitAsmIncludes();
void EmitOpenBuffer(char * buf);
void EmitCloseBuffer();
void EmitChar(char c);

extern Var   ROOT_PROC;

//extern Bool VERBOSE;
void InitPlatform();
extern Var * INTERRUPT;
extern Var * MACRO_PRINT;		// Print macro
extern Var * MACRO_FORMAT;		// Format macro
extern Var * MACRO_ASSERT_PRINT;		// Assert procedure
extern Var * MACRO_ASSERT;		// Assert procedure
extern MemHeap VAR_HEAP;		// variable heap (or zero page heap), this is heap from which variables are preferably allocated
extern Var * VARS;				// list of variables
extern Bool  ASSERTS_OFF;		// do not generate asserts into output code

