#include "common.h"
#include <stdio.h>

typedef struct VarTag Var;

Bool Verbose(Var * proc);

/*************************************************************

 Lexer

*************************************************************/

#define EOL 10
#define TAB 9
#define SPC 32

typedef UInt16 LineNo;

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

	// Keyword tokens

	TOKEN_KEYWORD = 128,
	TOKEN_GOTO    = 128,
	TOKEN_IF,
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
	UInt16 line_len;
	UInt16 line_pos;
	Token   token;
} ParseState;

typedef struct {
//	char   name[256];
	UInt32   n;
	FILE * f;
	Bool   ignore_keywords;
} Lexer;

Bool SrcOpen(char * name);
void SrcClose();
FILE * FindFile(char * name, char * ext);

extern char NAME[256];

// This functions are used only by parser

void NextToken();
void NextStringToken();
void ExpectToken(Token tok);

UInt16 SetBookmark();

Bool Spaces();
Bool NextIs(Token tok);
void EnterBlock();
void EnterBlockWithStop(Token stop_token);
void ExitBlock();

void LexerInit();

#define MAX_LINE_LEN 500000

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

void PlatformPath(char * path);

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
void LogicError(char * text, UInt16 bookmark);
void InternalError(char * text, ...);
void Warning(char * text);

void InitErrors();

/*********************************************************

 Variables & types

*********************************************************/
//$V

/*

====================
Positional arguments
====================

mode = MODE_ARG, name = "", index = 0..25  (a..z)

Undefined name is used for positional argument, because otherwise arguments have name.

*/


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
	TYPE_UNDEFINED,
	TYPE_SCOPE
} TypeVariant;

typedef struct {
	Bool flexible;		// range has been fixed by user
	Int32 min;
	Int32 max;
} Range;

#define MAX_DIM_COUNT 2
#define MACRO_ARG_CNT 26

struct TypeTag {
	TypeVariant  variant;	// int, struct, proc, array
	Bool         flexible;	// (read: inferenced)
	Bool		 is_enum;	// MODE_INTEGER is enum
	Type * base;			// type, on which this type is based (may be NULL)
	Var * owner;			// original owner of this type
	union {
		Range range;
		// TYPE_ARRAY
		struct {
			Type * dim[MAX_DIM_COUNT];		// array dimension types (integer ranges)
			Type * element;
			UInt16 step;					// Index will be multiplied by this value. Usually it is same as element size.
		};
	};
};

//TODO: Maybe we do not need the MODE_ARG, but MODE_VAR with SUBMODE_ARG_* ?
//TODO: MODE_SCOPE is just MODE_VAR with type TYPE_SCOPE

typedef enum {
	MODE_UNDEFINED = 0,
	MODE_ARG  = 1,		// Argument to function, macro or array
	MODE_CONST = 4,		// constant
	MODE_VAR   = 5,		// variable
	MODE_TYPE  = 6,		// type
	MODE_ELEMENT = 7,	// Array element (references array and index) - type is type of array
	MODE_SCOPE  = 8,	// Local scope
	MODE_LABEL = 9,
	MODE_SRC_FILE = 10,	// variable representing source file
						// scope   FILE that includes this file
						// name    filename
						// n       parse state
	MODE_TUPLE = 11,	// <adr,var>  (var may be another tuple)
						// Type of tuple may be undefined, or it may be structure of types of variables in tuple
	MODE_RANGE = 12,	// x..y  (adr = x, var = y) Used for slice array references
	MODE_DEREF = 13		// dereference an address (var contains reference to dereferenced adr variable, type is type in [adr of type]. Byte if unspecified adr is used.
} VarMode;

/*

MODE_DEREF

DEREF variables represent dereference of address variable.
Only address variables may be dereferenced.
For every adr variable, there may be at most one dereference variable.

If the dereferenced variable is of type ADR OF X, deref variable is of type X.

*/


typedef enum {
	SUBMODE_EMPTY = 0,
	SUBMODE_IN = 1,			// MODE_VAR, MODE_ARG
	SUBMODE_OUT = 2,
	SUBMODE_REG = 4,		// variable is stored in register
	SUBMODE_IN_SEQUENCE = 8,
	SUBMODE_ARG_IN  = 16,
	SUBMODE_ARG_OUT = 32,
	SUBMODE_OUT_SEQUENCE = 64,

	// MODE_SRC_FILE
	SUBMODE_MAIN_FILE = 4,	// this flag is set for main source file (asm is not included for main file, because it is generated by compiler)

	// MODE_CONST
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
	VarMode	mode;
	VarSubmode submode;

	VarFlags  flags;
	Var *	adr;	 // Address of variable in memory. For MODE_TYPE, this means alignment of variable of this type.
					 // MODE_ELEMENT	Array to which this variable belongs
					 // MODE_TUPLE      First variable of tuple

	Type *  type;	 // Pointer to type variable (such variable must have MODE_TYPE)
					 // MODE_ELEMENT:  Element type of array

	int     value_nonempty;
	// TODO: Replace value_nonempty just with flag VarDefined
	union {
		long	n;				// for const, or function default argument (other variants of value must be supported - array, struct, etc.)
		InstrBlock * instr;		// instructions for procedure or array initialization
		char * str;
		Var * var;
		ParseState * parse_state;	// MODE_SRC_FILE
	};
	UInt32 seq_no;
	Var *   current_val;	// current value asigned during optimalization
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

MODE_CONST          Integer definining location of variable in main memory.
MOVE_VAR,MODE_ARG   This variable is alias for the variable specified in adr.
MODE_TUPLE          List of variables. One bigger variable may be defined as list of smaller variables.

MODE_LABEL          Address.
                    Address alone may have adress, which specifies memory bank, in which the adress should be located.
					Address may be named. (For example labels).

*/

#define MAX_ARG_COUNT 128

typedef enum {
	INSTR_VOID = 0,
	INSTR_LET,		// var, val

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
	INSTR_GOTO,
	INSTR_ADD,
	INSTR_SUB,
	INSTR_MUL,
	INSTR_DIV,
	INSTR_SQRT,

	INSTR_AND,
	INSTR_OR,

	INSTR_ALLOC,
	INSTR_PROC,
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

	INSTR_LINE,				// reference line in the source code
	INSTR_INCLUDE,
	INSTR_MULA,				// templates for 8 - bit multiply 
	INSTR_MULA16,           // templates for 8 - bit multiply 

	INSTR_REF,				// this directive is not translated to any code, but declares, that some variable is used
	INSTR_DIVCARRY,
	INSTR_COMPILER,
	INSTR_CODE_END,			// end of CODE segment and start of data segment
	INSTR_DATA_END,			// end of data segment and start of variables segment

	// Following 'instructions' are used in expressions
	INSTR_VAR,
	INSTR_ELEMENT,			// access array element (left operand is array, right is index)
	INSTR_LIST,				// create list of two elements
	INSTR_DEREF,
	INSTR_FIELD,			// access field of structure

	INSTR_CNT
} InstrOp;


/*
INSTR_LINE is special instruction, which does not affect execution of program.
It marks place, where next line in source code begins.
All instructions after INSTR_LINE until next INSTR_LINE instruction were generated as a result
of parsing the line specified in instruction.

To save memory, INSTR_LINE argument are not pointers to ordinary variables.
result     pointer to MODE_SRC_FILE variable, that defines source file
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
// Data segment is allocated directly after CODE segment, however we reserve dynamic space for it at
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

UInt16 TypeItemCount(Type * type);
void TypeLimits(Type * type, Var ** p_min, Var ** p_max);

void TypeLet(Type * type, Var * var);
typedef void (*RangeTransform)(Int32 * x, Int32 tr);
void TypeTransform(Type * type, Var * var, InstrOp op);

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

extern Type TVOID;
extern Type TINT;		// used for int constants
extern Type TSTR;
extern Type TLBL;


void VarInit();
void VarInitRegisters();

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
Var * VarAlloc(VarMode mode, Name name, VarIdx idx);
Var * VarAllocScope(Var * scope, VarMode mode, Name name, VarIdx idx);
Var * VarAllocScopeTmp(Var * scope, VarMode mode, Type * type);
Var * VarFind(Name name, VarIdx idx);
Var * VarFindScope(Var * scope, char * name, VarIdx idx);
Var * VarFind2(char * name, VarIdx idx);
//Var * VarFindInProc(char * name, VarIdx idx);
Var * VarProcScope();
Var * VarFindMode(Name name, VarIdx idx, VarMode mode);

Var * VarFindInt(Var * scope, UInt32 n);
Var * VarMacroArg(UInt8 i);

Bool VarIsConst(Var * var);
Bool VarIsLabel(Var * var);
Bool VarIsArray(Var * var);
Bool VarIsTmp(Var * var);
Bool VarIsStructElement(Var * var);
Bool VarIsArrayElement(Var * var);
Bool VarIsReg(Var * var);
Var * VarReg(Var * var);
Bool VarIsFixed(Var * var);


#define InVar(var)  FlagOn((var)->submode, SUBMODE_IN)
#define OutVar(var) FlagOn((var)->submode, SUBMODE_OUT)

UInt32 VarByteSize(Var * var);

typedef UInt8 RegIdx;
extern Var * REG[64];		// Array of registers (register is variable with address in REGSET, submode has flag SUBMODE_REG)
extern RegIdx REG_CNT;		// Count of registers

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
Var * VarNewDeref(Var * var);
Var * VarNewRange(Var * min, Var * max);
Var * VarNewTuple(Var * left, Var * right);

void VarResetUse();

void VarFree(Var * var);

void VarEmitAlloc();

#define FOR_EACH_VAR(v) for(v = VARS; v != NULL; v = v->next) {
#define NEXT_VAR }

void PrintVar(Var * var);
void PrintVarName(Var * var);
void PrintScope(Var * scope);

void ProcUse(Var * proc, UInt8 flag);

void ProcessUsedProc(void (*process)(Var * proc));

typedef struct {
	Var * key;
	Var * var;
} VarTuple;

typedef struct {
	VarTuple * arr;
	UInt16     count;
	UInt16     capacity;
} VarSet;

void VarSetInit(VarSet * set);
Var * VarSetFind(VarSet * set, Var * key);
void VarSetAdd(VarSet * set, Var * key, Var * var);

/***********************************************************

  Instructions

***********************************************************/

#define IS_INSTR_BRANCH(x) ((x)>=INSTR_IFEQ && (x)<=INSTR_IFNOVERFLOW)
#define IS_INSTR_JUMP(x) (IS_INSTR_BRANCH(x) || (x) == INSTR_GOTO)

InstrOp OpNot(InstrOp op);

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

	//--- dest
	Var * result;

	union {
		Var * arg1;
		UInt16 line_no;
	};
	union {
		Var * arg2;
		char * line;
	};

	Instr * next_use[3];		// next use of result, arg1, arg2
//	UInt8 flags2[3];		// live

	Instr * next, * prev;
};

/*
For instructions, where arg1 or arg2 = result, source points to instruction, that previously set the result.

1.		ror x,x,1
2.		ror x,x,1   source = 1
3.		ror x,x,1   source = 2
4.      let a,x

*/

struct InstrBlockTag {

	InstrBlock * next;			//  Blocks are linked in chain, so we can traverse them as required.
	UInt32 seq_no;

	InstrBlock * to;			// this block continues (jumps) to this block (if to == NULL, this is last block in the procedure)
	InstrBlock * cond_to;		// last instruction conditionaly jumps to this block

	InstrBlock * from;			// we may come to this block from here

	InstrBlock * callers;		// list of blocks calling this block (excluding prev)
	InstrBlock * next_caller;	// next caller in the chain

	InstrBlock * loop_end;

	Var * label;				// label that starts the block
	Bool  processed;
	Instr * first, * last;		// first and last instruction of the block
};

void InstrInit();
void InstrPrint(Instr * i);
void InstrPrintInline(Instr * i);

void PrintVarVal(Var * var);
void PrintProc(Var * proc);

//void InstrFree(Instr * i);

Var * InstrFind(char * name);

// When we generate instructions, it is always done to defined code block
// Code blocks are managed using those procedures.

void InstrBlockPush();
InstrBlock * InstrBlockPop();

void InstrBlockFree(InstrBlock * blk);

// Instructions generating

void InternalGen(InstrOp op, Var * result, Var * arg1, Var * arg2);
void Gen(InstrOp op, Var * result, Var * arg1, Var * arg2);
void GenLet(Var * result, Var * arg1);
void GenLine();
void GenLabel(Var * var);
void GenGoto(Var * var);
void GenBlock(InstrBlock * blk);
void GenMacro(InstrBlock * code, Var * macro, Var ** args);
void GenLastResult(Var * var);
void GenArrayInit(Var * arr, Var * init);

Instr * InstrDelete(InstrBlock * blk, Instr * i);
void InstrInsert(InstrBlock * blk, Instr * before, InstrOp op, Var * result, Var * arg1, Var * arg2);

extern Var * SCOPE;		// currently parsed variable (procedure, macro)
extern Var * REGSET;	// enumerator with register sets

void CodePrint(InstrBlock * blk);

void InstrVarUse(InstrBlock * code, InstrBlock * end);
void VarUse();

Var * InstrEvalConst(InstrOp op, Var * arg1, Var * arg2);

UInt16 SetBookmarkLine(Instr * i);

void InstrReplaceVar(InstrBlock * block, Var * from, Var * to);
void ProcReplaceVar(Var * proc, Var * from, Var * to);

/***********************************************************

  Rules

***********************************************************/

typedef struct RuleTag Rule;
typedef struct RuleArgTag RuleArg;

//TODO: Check ARG type

typedef enum {
	RULE_ANY = 0,		// any argument
	RULE_VARIABLE,		// (type) variable of specified type
	RULE_CONST,			// (type) constant matching specified type
	RULE_REGISTER,		// (var)  actual variable (typically used for register)
	RULE_VALUE,			// (n)    actual value
	RULE_DEREF,			// dereference of variable (dereferenced type is always adr)
	RULE_ARG,			// argument (type of argument is defined, may be NULL)
	RULE_ELEMENT,		// array element - var defines pattern for array, index defines pattern for index
	RULE_TUPLE,
	RULE_RANGE
//	RULE_ARRAY_ARG      // argument that should be array of specified type
} RuleArgVariant;

typedef enum {
	RULE_ARRAY = 0,
	RULE_DIM,      // second dimension in array
	RULE_ITEM      // structure access
} RuleIndexVariant;

struct RuleArgTag {
	RuleArgVariant variant;
	UInt8          arg_no;		// argument index (1..26)  used for variable & const
	union {
	   Var * var;
	   Type * type;
	   RuleArg * arr;
	};
//	UInt8      index_variant;		// '(' first, '.' structure, ',' second index in array
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
};

void RuleRegister(Rule * rule);
Bool RuleMatch(Rule * rule, Instr * i);

void ProcTranslate(Var * proc);
void CheckValues(Var * proc);

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
Bool Parse(char * name, Bool main_file);
void ProcCheck(Var * proc);

/*************************************************************

 Optimize phase

*************************************************************/

Bool VarUsesVar(Var * var, Var * test_var);
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

void ProcClearProcessed(Var * proc);
void AllocateVariables(Var * proc);

void ProcInline(Var * proc);

/*************************************************************

 Emit phase

*************************************************************/

#define BLUE 1
#define GREEN 2
#define RED 4
#define LIGHT 8
UInt8 PrintColor(UInt8 color);
void PrintHeader(char * text);

Rule * EmitRule(Instr * instr);
Rule * EmitRule2(InstrOp op, Var * result, Var * arg1, Var * arg2);

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

//extern Bool VERBOSE;
extern Var * INTERRUPT;
extern Var * MACRO_PRINT;		// Print macro
extern Var * MACRO_FORMAT;		// Format macro
extern Var * MACRO_ASSERT;		// Assert procedure
extern MemHeap VAR_HEAP;		// variable heap (or zero page heap), this is heap from which variables are preferably allocated
extern Var * VARS;				// list of variables

