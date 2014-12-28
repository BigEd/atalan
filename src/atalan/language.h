/*
TODO:
	- remove ParseIntType
	- remove INSTR_NAME
	- remove INSTR_SCOPE
	- range should include resolution? (default is 1, different for floats etc.)
	- remove NewRangeInt for NewRange (or Range?)
	- remove TYPE_ARRAY

	- we need two version of the Add - one that computes constants, one that uses variables too

	Array

	- use INSTR_ITEM instead of INSTR_ELEMENT (INSTR_ELEMENT will be INSTR_FIELD)
	- 

*/

#include "../common/common.h"
#include "bigint.h"

typedef struct CellTag Var;
typedef struct CellTag Type;				// Type is special version of Cell
typedef struct CellTag Cell;
typedef struct LocTag Loc;
typedef struct VarSetTag VarSet;
typedef struct RuleTag Rule;
typedef struct RuleArgTag RuleArg;
typedef struct SrcLineTag SrcLine;

#define COLOR_ERROR (RED+LIGHT)
#define COLOR_WARNING (RED+GREEN+LIGHT)
#define COLOR_LINE_POS (RED+GREEN+BLUE)
#define COLOR_HINTS  (RED+GREEN+BLUE)

#include "lexer.h"

Bool Verbose(Var * proc);

typedef enum {
	PHASE_PARSE = 1,
	PHASE_TRANSLATE = 2,
	PHASE_OPTIMIZE = 3,
	PHASE_EMIT = 4
} CompilerPhase;

extern CompilerPhase PHASE;

/*************************************************************

 Lexer

*************************************************************/

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
	TOKEN_PLUSMINUS = 177,		// ±
	TOKEN_POWER = '^',
//	TOKEN_MUL2 = 215, // × = 215
	TOKEN_BYTE_INDEX = TOKEN_DOLLAR,

	// Keyword tokens

	TOKEN_KEYWORD = 180,
	TOKEN_GOTO    = 180,
	TOKEN_IF,
	TOKEN_UNLESS,
	TOKEN_THEN,
	TOKEN_ELSE,
	TOKEN_FN,
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
	TOKEN_MUL2, // × = 215
	TOKEN_BITXOR,
	TOKEN_STRUCT,
	TOKEN_USE,
	TOKEN_REF,
	TOKEN_STEP,
	TOKEN_RETURN,
	TOKEN_SEQUENCE,
	TOKEN_ASSERT,
	TOKEN_EITHER,
	TOKEN_STRING_TYPE,
	TOKEN_LAST_KEYWORD = TOKEN_STRING_TYPE,

	// two character tokens
	TOKEN_LOWER_EQUAL,
	TOKEN_HIGHER_EQUAL,
	TOKEN_NOT_EQUAL,
	TOKEN_DOTDOT,
	TOKEN_RIGHT_ARROW,
	TOKEN_HORIZ_RULE

} Token;

#define KEYWORD_COUNT (TOKEN_LAST_KEYWORD - TOKEN_KEYWORD + 1)

typedef struct ParseStateTag ParseState;

struct ParseStateTag {
	ParseState * parent;
	Cell * file;
	SrcLine * line;
	SrcLine * prev_line;
	LineNo line_no;
	LinePos line_len;
	LinePos line_pos;
	Token   token;
	Int16   prev_char;
	Var *   var;			// variable defining the file (now only used to define module name)
	Var *   arguments;
};

typedef struct {
//	BigInt   n;
//	FILE * f;
	Bool   ignore_keywords;
} Lexer;


extern Var * MODULES;

extern char NAME[256];
extern char NAME2[256];

// This functions are used only by parser

void NextToken();
void NextStringToken();
void ExpectToken(Token tok);

Bool Spaces();
Bool NextCharIs(UInt8 chr);
Bool NextIs(Token tok);
Bool NextNoSpaceIs(Token tok);
void NewBlock(Token end_token, Token stop_token);
void EnterBlock();
void EnterBlockWithStop(Token stop_token);
void ExitBlock();


void LexInit();

Cell * ParseFn(Bool as_macro);

#define MAX_LINE_LEN 32767

extern Lexer LEX;
extern Token TOK;
extern Token TOK_NO_SPACES;				// if the current token was not preceded by whitespaces, it is copied here

//extern FILE * F;
//extern char   LINE[MAX_LINE_LEN+2];		// we reserve one extra byte for terminating EOL, one for 0
extern LineNo  LINE_NO;
extern UInt16  LINE_LEN;
extern UInt16  LINE_POS;
extern UInt16  TOKEN_POS;
//extern char * PREV_LINE;
extern Var *  SRC_FILE;					// current source file
extern char PROJECT_DIR[MAX_PATH_LEN];
extern char SYSTEM_DIR[MAX_PATH_LEN];
char FILE_DIR[MAX_PATH_LEN];			// directory where the current file is stored
char FILENAME[MAX_PATH_LEN];

extern char PLATFORM[64];

typedef enum {
	INSTR_NULL = 0,
	INSTR_VOID,		// no value at all
	INSTR_LET,		// var, val
	INSTR_IF,		// condition, jump

	INSTR_EQ,		// 4 must be even!!!.
	INSTR_NE,       // 5
	INSTR_LT,
	INSTR_GE,
	INSTR_GT,
	INSTR_LE,
	INSTR_OVERFLOW,
	INSTR_NOVERFLOW,

	INSTR_ADD,     // 12
	INSTR_SUB,     // 13
	INSTR_MUL,     // 14
	INSTR_DIV,     // 15
	INSTR_MOD,     // 16
	INSTR_SQRT,    // 17

	INSTR_AND,	   // 18
	INSTR_OR,      // 19
	INSTR_XOR,     // 20
	INSTR_NOT,     // 21

	INSTR_ROL,		// 22 bitwise rotate right
	INSTR_ROR,		// 23 bitwise rotate left

	INSTR_PROLOGUE,	//24
	INSTR_EPILOGUE, //25
	INSTR_EMIT,
	INSTR_VARDEF,
	INSTR_LABEL,

	INSTR_ALLOC,  //29
	INSTR_FN,     //30
	INSTR_RETURN,
	INSTR_ENDPROC,
	INSTR_CALL,   //33
	INSTR_VAR_ARG,
	INSTR_DATA,
	INSTR_FILE,
	INSTR_ALIGN,
	INSTR_ORG,				// set the destination address of compilation
	INSTR_HI,
	INSTR_LO,
	INSTR_PTR,
	INSTR_ARRAY_INDEX,		// generate index for array
	INSTR_LET_ADR,
	INSTR_DEBUG,
	INSTR_ASSERT_BEGIN,
	INSTR_ASSERT,
	INSTR_ASSERT_END,

	INSTR_LINE2,			// reference line in the source code
	INSTR_INCLUDE,

	INSTR_COMPILER,
	INSTR_CODE_END,			// end of BLK segment and start of data segment
	INSTR_DATA_END,			// end of data segment and start of variables segment
	INSTR_SRC_END,
	INSTR_DECL,

	// Following 'instructions' are used in expressions
	INSTR_VAR,				// 55 Variable (may be argument, input, output, ...)
	INSTR_INT,				// 56 Integer constant
	INSTR_ELEMENT,			// 57 <array> <index>     access structure element (left operand is array, right is name)
	INSTR_BYTE,				// 58 <var> <byte_index>  access byte of specified variable
	INSTR_RANGE,			// 59 x..y  (l = x, r = y) Used for slice array references
	INSTR_TUPLE,			// 60 INSTR_LIST <adr,var>  (var may be another tuple)
						    // Type of tuple may be undefined, or it may be structure of types of variables in tuple
	INSTR_DEREF,			// 61 dereference an address (var contains reference to dereferenced adr variable, type is type in [adr of type]. Byte if untyped adr is used.
	INSTR_ITEM,				// 62 access element in array
	INSTR_TYPE,				// 63
	INSTR_SCOPE,			// 64
	INSTR_BIT,              // 65
	INSTR_TEXT,				// 66 text constant
	INSTR_VARIANT,          // 67
	INSTR_NAME,			    // 68 constant depending on type (array, procedure)  TODO: Remove
	INSTR_ARRAY,			// 69 array constant
	INSTR_SEQUENCE,         // 70
	INSTR_EMPTY,			// No-value
	INSTR_MATCH,		    // l(adr) = argument-no, r(var) = type that must match   name:type
	INSTR_VAL,				// 73
	INSTR_ARRAY_TYPE,       // 74
	INSTR_FN_TYPE,          // 75
	INSTR_ANY,				// 76 Represents any possible value. There is only one cell like this.

	INSTR_USES,				// 77 Extern instruction declares use of variable inside function
	                        // it can have either left side maning the instruction is written or arg1 meaning the variable is read
	INSTR_POWER,			// 78
	INSTR_MEMORY,			// 79
	INSTR_SRC_FILE,			// 80
	INSTR_PRINT,			// 81
	INSTR_CODE,
	INSTR_CNT				//82
} InstrOp;

#include "errors.h"


/*********************************************************

 Variables & types

*********************************************************/
//$V

/*

TODO: Type represented by variable

Types are currently defined using separate structure.
We would like to represent types using variables.

For example int (range) should be represented using INSTR_RANGE.

INSTR_INT   Represent type consisting of single value. (Type for constant).
              This may be used for named constants ( zero: 0 ).

INSTR_RANGE   Range given by two variables (min..max). 
              Currently only constants are supported, but we may use anything.
              
*/

typedef struct TypeInfoTag TypeInfo;
typedef struct InstrTag Instr;
typedef struct ExpTag Exp;
typedef struct CellTag InstrBlock;


#include "cell/cell_memory.h"
#include "cell/cell_sequence.h"
#include "cell/cell_src_file.h"
#include "cell/cell_code.h"

typedef enum {
	TYPE_VOID = 0,
	TYPE_INT,
	TYPE_STRING,
	TYPE_ARRAY,
	TYPE_LABEL,		// label in code (all labels share same type
	TYPE_ADR,		// address (or reference)
	TYPE_TYPE,
	TYPE_ANY
} TypeVariant;

typedef enum {
	JUMP_IF    = 0,
	JUMP_LOOP  = 1,
	JUMP_LOOP_EXIT = 2,
	JUMP_LOOP_ENTRY = 4     // may be combined with loop exit
} JumpType;

#define MACRO_ARG_CNT 26

extern SrcLine * PREV_SRC_LINE;
extern SrcLine * SRC_LINE;

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
	SUBMODE_PARAM    = 1024,
	SUBMODE_MACRO    = 2048,
	SUBMODE_LOCKED   = 4096,		// variable cannot be assigned multiple times

	// General
	SUBMODE_SYSTEM = 128,				// This is system variable (defined either by system or platform)
	SUBMODE_USER_DEFINED = 256,			// Type of this variable has been explicitly defined by user (programmer)
	SUBMODE_USED_AS_TYPE = 512,
	SUBMODE_FRESH = SUBMODE_USED_AS_TYPE,

	// INSTR_SRC_FILE
	SUBMODE_MAIN_FILE = 4,	// this flag is set for main source file (asm is not included for main file, because it is generated by compiler)

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

#define VarUsedInInterupt  32		// this procedure is used from interrupt
#define VarProcAddress     64		// procedure address is required (this means, we are not allowed to inline it)

#define VarUsed            16		// for register allocation

#define VarLabelDefined    32

typedef unsigned int VarIdx;
typedef UInt8 VarFlags;

struct VarCellTag {
	char *	name;
	VarIdx  idx;			// two variables with same name but different index may exist
	// 0 means no index, 1 means index 1 etc.
	// variable name "x1" is automatically converted to x,1
	Cell *	adr;	 // Address of variable in memory. For INSTR_TYPE, this means alignment of variable of this type.
	Type *  type;	 // Type of variable
	Cell *  subscope;
};

struct OpCellTag {
	Var * l;
	Var * r;
};

struct CellTag {

	InstrOp      mode;
	VarSubmode   submode;

	VarFlags  flags;

	// Variable identification (name,idx,scope)

	// INSTR_CODE, other
	union {

		struct {
			// INSTR_VAR
			char *	name2;
			VarIdx  idx;			// two variables with same name but different index may exist
									// 0 means no index, 1 means index 1 etc.
									// variable name "x1" is automatically converted to x,1
			// Location
			SrcLine * line;			// file in which the variable has been defined
			LinePos   line_pos;		// position on line at which the variable has been declared

			Var  *  scope;			// scope, in which this variable has been declared
			Var  *  next_in_scope;  // in future, this will be replaced by 'next'
			Var  *  subscope;		// first variable in subscope of this variable

			union {
				Cell *	_adr;	 // Address of variable in memory. For INSTR_TYPE, this means alignment of variable of this type.
								 // INSTR_ELEMENT	Array to which this variable belongs
								 // INSTR_TUPLE      First variable of tuple
				Var * l;
			};

			union {
				Type *  type;	 // Type of variable
				Var * m;
			};

			union {
				BigInt  n;				    // INSTR_INT
				char * str;				    // INSTR_TEXT
				MemoryCell mem;				// INSTR_MEMORY
				InstrBlock * instr;		    // INSTR_ARRAY, INSTR_FN  instructions for procedure or array initialization
				Cell * r;
				SrcFileCell src_file;
				SequenceCell seq;
				struct {					// INSTR_TYPE
					TypeVariant  variant;	// int, struct, proc, array
					Bool         is_enum;	// INSTR_INT  is enum
					Cell *        possible_values;	// for each type, there can be possible set of value defined
					// TYPE_ARRAY, TYPE_ADR
					struct {
						Type * index;
						Type * element;
					};
				};
			};
		}; // other

		//===== INSTR_CODE
		struct {
			InstrBlock * next;			// Blocks are linked in chain, so we can traverse them as required.
			// This is normally in order, in which the blocks were parsed.
			UInt32 seq_no;				// Block sequence number. It is used to determine order of blocks when detecting loops.
			// It is also useful when debugging the compiler, as we can quickly locate an instruction by block number and
			// sequence number of the caller.

			InstrBlock * to;			// this block continues (jumps) to this block (if to == NULL, we leave the routine after the last instruction in the block)
			InstrBlock * cond_to;		// last instruction conditionally jumps to this block if the condition if true

			InstrBlock * from;			// we may come to this block from here

			InstrBlock * callers;		// list of blocks calling this block (excluding from)
			InstrBlock * next_caller;	// next caller in the chain

			JumpType     jump_type;		// whether this is end of loop or some other type of branch

			Cell * label;				// label that starts the block
			Bool  processed;
			Type * itype;				// type computed in this block for variable when inferring types
			Instr * first, * last;		// first and last instruction of the block
			void * analysis_data;
		};
	};


	Instr * src_i;
	Exp *   dep;
	UInt16	read;			// how many times some instruction reads this variable (if 0, this is unused)
	UInt16	write;			// how many times some instruction writes this variable (if 1 this is constant)

	UInt16  set_index;		// index in current set

};

/*
TODO:

Const type variable

Type may be implemented as variable.

type	- parent type of the type
var     - set of possible values (for example range 3..4, etc.)

subscope - fields defining the type properties (for example index variable for array - we would prefer to specify index in some other way, but name # may be appropriate)

*/


/*

Variable address
----------------

Address of variable may be:

INSTR_INT          Integer defining location of variable in main memory.
MOVE_VAR            This variable is alias for the variable specified in adr.
INSTR_TUPLE          List of variables. One bigger variable may be defined as list of smaller variables.

INSTR_LABEL          Address.
                    Address alone may have address, which specifies memory bank, in which the address should be located.
					Address may be named. (For example labels).

*/

/*
Variables are managed in blocks.

Unused variable has INSTR_NULL and is in special scope.
Unused variables are kept in a list using next_in_scope.

*/

#define VAR_BLOCK_CAPACITY 10

typedef struct CellBlockTag CellBlock;

struct CellBlockTag {
	CellBlock * next;
	Var vars[VAR_BLOCK_CAPACITY];
};

extern CellBlock * CELL_BLOCKS;
extern CellBlock * G_BLK;
extern UInt16 G_VAR_I;

#define FOR_EACH_VAR(name) for(G_BLK = CELL_BLOCKS; G_BLK != NULL; G_BLK = G_BLK->next) for(name = &G_BLK->vars[0], G_VAR_I = 0; G_VAR_I < VAR_BLOCK_CAPACITY; G_VAR_I++, name++) {
#define NEXT_VAR }

#define MAX_ARG_COUNT 128

/*
For every cell type, we define cell interface, which provides set of functions applicable to this cell.
*/
//$I
/*
#define CELL_FREE void CellFree(Cell * cell)
#define CELL_PRINT void CellPrint(Cell * cell)
#define CELL_EXECUTE void CellExecute(Instr * i);

//typedef struct CellInterfaceTag CellInterface;

#define CELL_INTERFACE(NAME) CellInterface NAME ## CellInterface = {
#define END_CELL_INTERFACE };

typedef struct {
	void (*cell_free_fn)(Cell * cell);
	void (*cell_print_fn)(Cell * cell);
	Cell * (*eval_fn)(Cell * cell);
} CellInterface;

CellInterface GenericCellInterface;

CellInterface SrcFileCellInterface;
*/

// These functions may be used as default implementations of cell interface functions.
void VoidCellFree(Cell * cell);
void DefaultCellPrint(Cell * cell);
Cell * VoidEval(Cell * cell);
Cell * SelfEval(Cell * cell);


Cell * Eval(Cell * cell);

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

UInt8 IntByteSize(BigInt * n);

Type * TypeAlloc(TypeVariant variant);

Type * TypeDerive(Type * base);
Type * TypeCopy(Type * base);

Type * TypeByte();
Type * TypeArray(Type * index, Type * element);

Type * TypeAdrOf(Type * element);

typedef void (*RangeTransform)(BigInt * dest, BigInt * x, BigInt * tr);

void TypeAddConst(Type * type, Var * var);

Bool TypeIsInt2(Type * type);

Bool TypeIsConst(Type * type);
Bool TypeIsInt(Type * type);
Bool CellIsIntConst(Type * type);


UInt32 TypeAdrSize();
UInt32 TypeStructAssignOffsets(Type * type);

void PrintType(Type * type);

//--- Proc type

void TypeInfer(Var * proc);

extern Type TLBL;

#include "cell/cell_int.h"
#include "cell/cell_text.h"
#include "cell/cell_var.h"

//#include "cell/relational/cell_eq.h"
//#include "cell/relational/cell_ne.h"

//#include "cell/arithmetic/cell_add.h"

#include "cell/cell_array_type.h"
#include "cell/cell_fn_type.h"
#include "cell/cell_fn.h"
#include "cell/cell_tuple.h"
#include "cell/cell_range.h"
#include "cell/cell_item.h"

/**********************************************

 Variables

***********************************************/

extern Var * ANY;			// any possible value (for type, this can also mean undefined)
extern Var * VOID;			// no value

void VarInit();
void InitCPU();

Var * NewArray(Type * type, InstrBlock * instr);

// Name cell

Var * ForEachCell(Bool (*cell_fn)(Var * cell, void * data), void * data);

//===== Scope

#define NO_SCOPE ((Var *)1)

Var * InScope(Var * new_scope);
void ReturnScope(Var * prev);

void EnterSubscope();
void ExitSubscope();

void CellSetScope(Var * var, Var * scope);

//===== Cell allocation

Var * NewCell(InstrOp mode);
Var * NewCellInScope(InstrOp mode, Var * scope);
Var * CellCopy(Var * var);

Type * CellType(Var * cell);
void CellSetLocation(Var * cell, SrcLine * line, LinePos line_pos);



//---- labels
Var * VarNewLabel(char * name);
Var * FindOrAllocLabel(char * name);
Var * VarNewTmpLabel();



Var * VarFind(Var * scope, char * name);
Var * VarFind(Var * scope, char * name);
Var * VarFindScope2(Var * scope, char * name);
Var * VarFind2(char * name);
Var * VarFindLabel(char * name);

//===== Variables

Var * NewVarInScope(Type * scope, Var * type);
Var * NewVar(Var * scope, char * name, Type * type);
Var * NewVarWithIndex(Var * scope, char * name, UInt16 idx, Type * type);
Var * NewTempVar(Type * type);

Var * VarProcScope();

Bool VarIsZeroNonzero(Var * var, Var ** p_zero, Var ** p_non_zero);

Var * VarRuleArg(UInt8 i);


Bool VarIsParam(Var * var);

Var * NewVariant(Var * left, Var * right);

Bool VarIsLabel(Var * var);
Bool VarIsArray(Var * var);

Bool CellIsValue(Var * var);
Bool CellIsStatic(Var * var);

Bool VarIsTmp(Var * var);
Bool VarIsStructElement(Var * var);
Bool VarIsArrayElement(Var * var);
Bool VarIsReg(Var * var);
Var * VarReg(Var * var);
Bool VarIsFixed(Var * var);
Bool VarIsAlias(Var * var);

Bool VarIsLocal(Var * var, Var * scope);
Bool VarIsUsed(Var * var);


Bool VarIsArg(Var * var);

Bool VarIsRuleArg(Var * var);

void VarCount(Var * var, BigInt * cnt);
void VarRange(Var * var, BigInt ** p_min, BigInt ** p_max);
Bool VarIdentical(Var * left, Var * right);

Var * VarFindAssociatedConst(Var * var, char * name);

Var * VarField(Var * var, char * fld_name);

#define InVar(var)  FlagOn((var)->submode, SUBMODE_IN)
#define OutVar(var) FlagOn((var)->submode, SUBMODE_OUT)

void VarResetRegUse();

long VarParamCount(Var * var);

void VarGenerateArrays();
void VarToLabel(Var * var);

Var * VarNewElement(Var * arr, Var * idx);
Var * VarNewByteElement(Var * arr, Var * idx);
Var * VarNewBitElement(Var * arr, Var * idx);
Var * VarNewDeref(Var * var);

//===== Text

Var * TextCell(char * str);
void VarInitStr(Var * var, char * str);


//===== Op
Var * NewOp(InstrOp op, Var * left, Var * right);
Bool CellIsOp(Var * cell);
void PrintBinaryOp(Cell * cell);
void PrintUnaryOp(Cell * cell);

//===== Sequence
Var * NewSequence(Var * init, Var * step, InstrOp step_op, Var * limit, InstrOp compare_op);
Bool  SequenceRange(Type * type, Var ** p_min, Var ** p_max);
Var * ResolveSequence(Var * cell);


Var * VarEvalConst(Var * var);

void VarResetUse();

void VarEmitAlloc();

#define FOR_EACH_LOCAL(SCOPE, VAR) 	for(VAR = (SCOPE)->subscope; VAR != NULL; VAR = VAR->next_in_scope) {
#define NEXT_LOCAL }


void PrintVar(Var * var);
void PrintIntCellName(Var * var);
void PrintVarUser(Var * var);
void PrintQuotedCellName(Var * var);
void PrintScope(Var * scope);
void PrintUserScope(Var * scope);
void PrintVars(Var * proc);

void ProcUse(Var * proc, UInt8 flag);

void ProcessUsedProc(void (*process)(Var * proc));

#include "operations/compare.h"
#include "operations/properties.h"
#include "operations/set.h"
#include "operations/arith.h"
#include "operations/bool.h"

typedef struct {
	Var * key;
	Var * var;
} VarTuple;

typedef struct VarSetTag {
	VarTuple * arr;
	UInt16     count;
	UInt16     capacity;
};

#define VarSetCount(set) ((set)->count)

void VarSetInit(VarSet * set);
Var * VarSetFind(VarSet * set, Var * key);
void VarSetAdd(VarSet * set, Var * key, Var * var);
Var * VarSetRemove(VarSet * set, Var * key);
void VarSetEmpty(VarSet * set);
void VarSetCleanup(VarSet * set);
VarTuple * VarSetItem(VarSet * set, UInt16 index);

void VarSetPrint(VarSet * set);

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

#define IS_INSTR_BRANCH(x) ((x)==INSTR_IF)
#define IS_INSTR_JUMP(x) ((x) == INSTR_IF)
#define IS_RELATIONAL_OP(x) ((x)>=INSTR_EQ && (x)<= INSTR_NOVERFLOW)

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
	Rule *  rule;		// after translation, this is pointer to rule defining the operator
	Var * result;
	Var * arg1;
	Var * arg2;

	// Position on line, on which is the token that generated the instruction.
	// If 0, it means the position is not specified (previous token should be used)

	SrcLine * line;
	LinePos  line_pos;

	// Type of result computed by this instruction
	// This type may differ from type defined in instruction result variable 
	// (it may be it's subset).
	// For example in case of LET x, 10 instruction, type in result_type will be 10..10, even if type of
	// x variable may be 0..255.

	// TODO: type may be union with next_use (they are not used at the same time)

	Type * type[3];				// 0 result type, 1 arg1 type 2 arg2 type
	Type * result_index_type;

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
	UInt32 n;			// sequence number of instruction in block
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
/*
struct InstrBlockTag {

	InstrBlock * next;			// Blocks are linked in chain, so we can traverse them as required.
								// This is normally in order, in which the blocks were parsed.
	UInt32 seq_no;				// Block sequence number. It is used to determine order of blocks when detecting loops.
								// It is also useful when debugging the compiler, as we can quickly locate an instruction by block number and
								// sequence number of the caller.

	InstrBlock * to;			// this block continues (jumps) to this block (if to == NULL, we leave the routine after the last instruction in the block)
	InstrBlock * cond_to;		// last instruction conditionally jumps to this block if the condition if true

	InstrBlock * from;			// we may come to this block from here

	InstrBlock * callers;		// list of blocks calling this block (excluding from)
	InstrBlock * next_caller;	// next caller in the chain

	JumpType     jump_type;		// whether this is end of loop or some other type of branch

	Cell * label;				// label that starts the block
	Bool  processed;
	Type * itype;				// type computed in this block for variable when inferring types
	Instr * first, * last;		// first and last instruction of the block
	void * analysis_data;
};
*/

void InstrInit();
void InstrPrint(Instr * i);
void InstrPrintInline(Instr * i);
void InstrFree(Instr * i);

char * OpSymbol(InstrOp op);

#define PrintInferredTypes 1

void PrintCell(Cell * cell);
void PrintCell(Var * var);
void PrintProc(Var * proc);
void PrintProcFlags(Var * proc, UInt32 flags);
void PrintBlockHeader(InstrBlock * blk);
void PrintInstrLine(UInt32 n);
void PrintCode(InstrBlock * blk, UInt32 flags);

InstrOp InstrFind(char * name);

// When we generate instructions, it is always done to defined code block
// Code blocks are managed using those procedures.


void InstrMoveCode(InstrBlock * to, Instr * after, InstrBlock * from, Instr * first, Instr * last);
Instr * InstrDelete(InstrBlock * blk, Instr * i);
Instr * InstrInsert(InstrBlock * blk, Instr * before, InstrOp op, Var * result, Var * arg1, Var * arg2);
void InstrAttach(InstrBlock * blk, Instr * before, Instr * first, Instr * last);
void InstrInsertRule(InstrBlock * blk, Instr * before, InstrOp op, Var * result, Var * arg1, Var * arg2);

InstrBlock * LastBlock(InstrBlock * block);

Bool IsGoto(Instr * i);
InstrBlock * IfInstr(Instr * i);

typedef void (*ProcessBlockFn)(InstrBlock * block, void * info);

void ForEachBlock(InstrBlock * blk, ProcessBlockFn process_fn, void * info);
Bool ProcInstrEnum(Var * proc, Bool (*fn)(Loc * loc, void * data), void * data);

extern Var * SCOPE;		// currently parsed variable (procedure, macro)
//extern Var * REGSET;	// enumerator with register sets

void InstrVarUse(InstrBlock * code, InstrBlock * end);
void VarUse();

Var * InstrEvalConst(InstrOp op, Var * arg1, Var * arg2);
Var * InstrEvalAlgebraic(InstrOp op, Var * arg1, Var * arg2);

void InstrBlockReplaceVar(InstrBlock * block, Var * from, Var * to);
void ProcReplaceVar(Var * proc, Var * from, Var * to);

Bool InstrEquivalent(Instr * i, Instr * i2);
Bool InstrIsSelfReferencing(Instr * i);

// Instruction generating

void GenerateInit();
void GenRegisterRule(Rule * rule);
void GenSetDestination(InstrBlock * blk, Instr * i);
InstrBlock * GenBegin();
InstrBlock * GenEnd();		//TODO: GenEnd should return void
InstrBlock * GenNewBlock();

void GenFromLine(SrcLine * line, InstrOp op, Var * result, Var * arg1, Var * arg2);
void GenInternal(InstrOp op, Var * result, Var * arg1, Var * arg2);
void Gen(InstrOp op, Var * result, Var * arg1, Var * arg2);
void GenRule(Rule * rule, Var * result, Var * arg1, Var * arg2);
void GenLet(Var * result, Var * arg1);
void GenLabel(Var * var);
void GenGoto(Var * var);
void GenBlock(InstrBlock * blk);
void GenMacro(Var * macro, Var ** args);
//void GenMacroParse(Var * macro, Var ** args);
void GenLastResult(Var * var, Var * item);

void GenPos(InstrOp op, Var * result, Var * arg1, Var * arg2);
void GenLetPos(Var * result, Var * arg1);


// Cell/instruction information

#define RESULT 0
#define ARG1   1
#define ARG2   2

#define INSTR_COMMUTATIVE 1
#define INSTR_NON_CODE    2		// non-executable instruction generating data
#define INSTR_OPTIONAL    4		// instruction must not be translated
#define INSTR_OPERATOR    8
#define INSTR_IS_TYPE     16    // type is always NON_CODE

#define INSTR_COMMUTATIVE_OPERATOR (INSTR_OPERATOR + INSTR_COMMUTATIVE)

typedef struct {
	InstrOp  op;
	char * name;
	char * symbol;
	TypeVariant arg_type[3];		// 0 = result, 1 = arg1, 2 = arg2
	UInt8   flags;

	void (*cell_free_fn)(Cell * cell);
	void (*cell_print_fn)(Cell * cell);
	Cell * (*eval_fn)(Cell * cell);

//	CellInterface * intf;

} InstrInfo;

extern InstrInfo INSTR_INFO[INSTR_CNT];

void TranslateTypes(Var * proc);
void TranslateTypes2(Var * proc, Bool always);
Type * CpuType(Type * type);

/***********************************************************

  Translate

***********************************************************/

extern Var * RULE_FN_TYPE;

struct RuleArgTag {
	InstrOp variant;
	UInt8          arg_no;		// argument index (1..26)  used for variable & const
	union {
	   Var * var;
	   Type * type;
	   RuleArg * arr;
	};
	RuleArg  * index;		        // pointer to index for array (NULL if there is no index)
							        // Rule arg is allocated in this case
							        // This type must be INSTR_MATCH or INSTR_VAR, variable must be array
};

struct RuleTag {
	Rule * next;
	SrcLine *  line;			// file in which the rule has been defined

	InstrOp op;
	Cell * arg[3];
//	RuleArg * arg[3];
	InstrBlock * to;
	Var * flags;			// for instruction rule, this is variable with flag or flags (tuple) that are modified, when this instruction is executed
	UInt8 cycles;			// How many cycles the instruction uses.
	Var * fn;				// function used for this rule. This fn will contain local variables of the rule.
};


/*
Rules are stored in rule sets. We have three sets of rules:

 1. macro rules are used when parsing code
 2. Translate rules are used when translating instructions generated by parser to instruction rules
 3. Instruction rules (which represent actual processor instructions)

*/

typedef struct {
	Rule * rules[INSTR_CNT];
} RuleSet;

void RuleSetInit(RuleSet * ruleset);
void RuleSetAddRule(RuleSet * ruleset, Rule * rule);
Rule * RuleSetFindRule(RuleSet * ruleset, InstrOp op, Var * result, Var * arg1, Var * arg2);
void TranslateRules();

void GenMatchedRule(Rule * rule);

void RuleRegister(Rule * rule);
//Bool RuleMatch(Rule * rule, Instr * i);

#define GENERATE 0
#define TEST_ONLY 1
#define BIGGER_RESULT 2

void ProcTranslate(Var * proc);
Bool InstrTranslate3(InstrOp op, Var * result, Var * arg1, Var * arg2, UInt8 mode);

void CheckValues(Var * proc);
void TranslateInit();

Rule * TranslateRule(InstrOp op, Var * result, Var * arg1, Var * arg2);

// Garbage collector

void RulesGarbageCollect();

/*************************************************************

 Parser

*************************************************************/
typedef struct BlockTag Block;

struct BlockTag {
	Token command;		// TOK_IF, TOK_WHILE
	Var * body_label;
	Var * loop_label;
	Var * t_label;		// label, to which the condition jumps if it is true
	Var * f_label;		// label, to which the condition jumps if it is false
	Bool  not;			// set to true, if the condition should be negated
};

void ParseInit();
Bool Parse(char * name, UInt16 blk_type, Bool main_file, Bool parse_options);
void ProcCheck(Var * proc);

void ReportSimilarNames(char * name);

Bool ParsingSystem();
Bool ParsingRule();
Bool ParsingPattern();

void BufEmpty();
void BufPush(Var * var);
Var * BufPop();

Type * ParseType3();
Type * ParseType2(InstrOp mode);
Type * ParseTypeInline();

void ParseExpression(Var * result);
void ParseExpressionType(Type * result_type);
Cell * ParseDefExpression();
Bool ParseArg(Var ** p_var);

#define STACK_LIMIT 100
Var *  STACK[STACK_LIMIT];
UInt16 TOP;
extern LinePos OP_LINE_POS;				// Position of last parsed binary operator

/*************************************************************

Analytics

*************************************************************/

typedef Bool (*AnalyzeBlockFn)(Var * proc, InstrBlock * block, void * info);
/*
Purpose:
	Perform data flow analysis for specified block.
Arguments:
	block		Block to analyze
	info		Global information information shared between all blocks
Result:
	Return true, if there was some change in the block information, false otherwise.
*/

void DataFlowAnalysis(Var * proc, AnalyzeBlockFn block_fn, void * info);

typedef UInt8 * LiveSet;		// VarLive, VarDead, VarUndefined

void LiveVariableAnalysis(Var * proc);
void FreeLiveVariableAnalysis(Var * proc);

/*************************************************************

 Optimize phase

*************************************************************/

void OptimizeDataFlowBack(Var * proc, AnalyzeBlockFn block_fn, void * info);

void ResetValues();

Bool VarUsesVar(Var * var, Var * test_var);
Bool VarModifiesVar(Var * var, Var * test_var);
Int16 VarTestReplace(Var ** p_var, Var * from, Var * to);
Int16 VarReplace(Var ** p_var, Var * from, Var * to);

Int16 InstrTestReplaceVar(Instr * i, Var * from, Var * to);
Int16 InstrReplaceVar(Instr * i, Var * from, Var * to);

void ResetValue(Var * res);
void VarSetSrcInstr(Var * var, Instr * i);
Var * SrcVar(Var * var);

Bool InstrUsesVar(Instr * i, Var * var);
Bool InstrReadsVar(Instr * i, Var * var);
Bool InstrSpill(Instr * i, Var * var);

UInt8 VarIsLiveInBlock(Var * proc, InstrBlock * block, Var * var);
void MarkBlockAsUnprocessed(InstrBlock * block);


void ProcOptimize(Var * proc);
void GenerateBasicBlocks(Var * proc);
void MarkLoops(Var * proc);

Bool OptimizeLive(Var * proc);
Bool OptimizeLive2(Var * proc);

Bool OptimizeValues(Var * proc);
Bool OptimizeVarMerge(Var * proc);
Bool OptimizeLoops(Var * proc);

void OptimizeJumps(Var * proc);
void DeadCodeElimination(Var * proc);
Bool OptimizeMergeBranchCode(Var * proc);

void ProcClearProcessed(Var * proc);
void AllocateVariables(Var * proc);

void OptimizeProcInline(Var * proc);

void LoopPreheader(Var * proc, InstrBlock * header, Loc * loc);

void OptimizeLoopShift(Var * proc);

void InstrExecute(InstrBlock * blk);

/*************************************************************

 Emit phase

*************************************************************/


void PrintOptim(char * text);

Bool InstrRuleIsDefined(InstrOp op);
Rule * InstrRule(Instr * instr);
Rule * InstrRule2(InstrOp op, Var * result, Var * arg1, Var * arg2);

void Emit(char * filename);
Bool EmitInstr(Instr * code);
Bool EmitInstrOp(InstrOp op, Var * result, Var * arg1, Var * arg2); 
Bool EmitInstrInline(Instr * i);
void EmitOpenBuffer(char * buf);
void EmitCloseBuffer();
void EmitChar(char c);

extern Var   ROOT_PROC;

//extern Bool VERBOSE;
void InitPlatform();
extern Var * INTERRUPT;
extern Var * CPU_TYPE;
extern Var * MACRO_PRINT;		// Print macro
extern Var * MACRO_FORMAT;		// Format macro
extern Var * MACRO_ASSERT_PRINT;		// Assert procedure
extern Var * MACRO_ASSERT;		// Assert procedure
extern MemHeap VAR_HEAP;		// variable heap (or zero page heap), this is heap from which variables are preferably allocated
extern Bool  ASSERTS_OFF;		// do not generate asserts into output code

#define COLOR_OPTIMIZE (GREEN+LIGHT)

Cell * Interpret(Cell * proc, Cell * args);
