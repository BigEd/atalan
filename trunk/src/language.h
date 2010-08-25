#include "common.h"
#include <stdio.h>

typedef struct VarTag Var;

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
	TOKEN_INSTR,
	TOKEN_TIMES,
	TOKEN_ADR2,
	TOKEN_DEBUG,
	TOKEN_MOD,
	TOKEN_XOR,

	// two character tokens
	TOKEN_LOWER_EQUAL,
	TOKEN_HIGHER_EQUAL,
	TOKEN_NOT_EQUAL,
	TOKEN_DOTDOT,
	TOKEN_RIGHT_ARROW

} Token;

typedef struct {
	char   name[256];
	UInt32   n;
	char * filename;
	FILE * f;
	Bool   ignore_keywords;
} Lexer;

Bool SrcOpen(char * name);
void SrcClose();


// This functions are used only by parser

void NextToken();
void ExpectToken(Token tok);

UInt16 SetBookmark();

Bool Spaces();
Bool NextIs(Token tok);
void EnterBlock();
void EnterBlockWithStop(Token stop_token);
void ExitBlock();

void LexerInit();

#define MAX_LINE_LEN 2048

extern Lexer LEX;
extern Token TOK;
extern char   LINE[MAX_LINE_LEN+2];		// we reserve one extra byte for terminating EOL, one for 0
extern LineNo  LINE_NO;
extern UInt16  LINE_LEN;
extern UInt16  LINE_POS;
extern UInt16  TOKEN_POS;
extern char * PREV_LINE;
extern Var *  SRC_FILE;					// current source file
extern char PROJECT_DIR[256];
extern char SYSTEM_DIR[256];

/*********************************************************

  Error reporting

**********************************************************/

extern UInt32 ERROR_CNT;
extern UInt32 LOGIC_ERROR_CNT;

void SyntaxError(char * text);
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
typedef struct InstrBlockTag InstrBlock;
//typedef struct InstrEnumTag InstrEnum;

typedef enum {
	TYPE_VOID = 0,
	TYPE_INT,
	TYPE_STRUCT,
	TYPE_PROC,
	TYPE_MACRO,
	TYPE_STRING,
	TYPE_ARRAY,
	TYPE_LABEL,		// label in code (all labels share same type
	TYPE_ADR,		// adress (or reference) to 
	TYPE_UNDEFINED
} TypeVariant;

typedef struct {
	Bool flexible;		// range has been fixed by user
	long min;
	long max;
} Range;

#define MAX_DIM_COUNT 2
#define MACRO_ARG_CNT 26

struct TypeTag {
	TypeVariant  variant;	// int, struct, proc, array
	Bool         flexible;	// (read: inferenced)
	Type * base;			// type, on which this type is based (may be NULL)
	Var * owner;			// original owner of this type
	union {
		Range range;
		// TYPE_ARRAY
		struct {
			Type * dim[MAX_DIM_COUNT];		// array dimension types (integer ranges)
			Type * element;
		};
	};
};

//TODO: Maybe we do not need the MODE_ARG, but MODE_VAR with SUBMODE_ARG_* ?

typedef enum {
	MODE_UNDEFINED = 0,
	MODE_ARG  = 1,		// Argument to function, macro or array
	MODE_CONST = 4,		// constant
	MODE_VAR   = 5,		// variable
	MODE_TYPE  = 6,		// type
	MODE_ELEMENT = 7,	// Array element (references array and index) - type is type of array
	MODE_SCOPE  = 8,	// Local scope
	MODE_LABEL = 9,
	MODE_SRC_FILE = 10	// variable representing source file
						// scope   FILE that includes this file
						// name    filename
						// n       current line number
} VarMode;

typedef enum {
	SUBMODE_EMPTY = 0,
	SUBMODE_IN = 1,			// MODE_VAR, MODE_ARG
	SUBMODE_OUT = 2,
	SUBMODE_REG = 4,		// variable is stored in register
	SUBMODE_REF = 8,
	SUBMODE_ARG_IN  = 16,
	SUBMODE_ARG_OUT = 32
} VarSubmode;

//REF
//		Value of ptr is address
//
//
//Element is variable, that represents value in array i.e.  a(1)
//For multiple indices, elements are nested? a(1,2)  -> elmt(elmt(a, 1),2)
//Elements are practically always temporary.


#define VarLive            1
#define VarUninitialized   2
#define VarLoop            4		// loop variable (incremented during loop)
#define VarLoopDependent   8		// variable is dependent (even transitively) on some loop variable

typedef unsigned int VarIdx;
typedef char * Name;

struct VarTag {

	// Variable identification (name,idx,scope)

	Name	name;
	VarIdx  idx;	 // two variables with same name but different index may exist
					 // 0 means no index, 1 means index 1 etc.
					 // variable name "x1" is automatically converted to x,1
	Var  *  scope;	 // procedure, in which this variable is declared
					 // it may be named scope, instead
	VarMode	mode;
	VarSubmode submode;

	UInt8   flags;
	Var *	adr;	 // Address of variable in memory. For MODE_TYPE, this means alignment of variable of this type.
					 // MODE_ELEMENT	Array to which this variable belongs

	Type *  type;	 // Pointer to type variable (such variable must have MODE_TYPE)
					 // MODE_ELEMENT:  Element type of array

	int     value_nonempty;
	union {
		long	n;				// for const, or function default argument (other variants of value must be supported - array, struct, etc.)
		InstrBlock * instr;		// instructions for procedure or array initi
		char * str;
		Var * var;
		Instr * lab_adr;		// label address
		FILE * f;				// MODE_SRC_FILE
	};
	UInt32 seq_no;
	Var *   current_val;	// current value asigned during optimalization
							// TODO: Maybe this may be variable value.
	Instr * src_i;
	UInt16	read;			// how many times some instruction reads this variable (if 0, this is unused)
	UInt16	write;			// how many times some instruction writes this variable (if 1 this is constant)
	Var  *  next;			// next variable in chain
};

typedef enum {
	INSTR_VOID = 0,
	INSTR_LET,		// var, val

	INSTR_IFEQ,		// must be even!!!.
	INSTR_IFNE,
	INSTR_IFLT,
	INSTR_IFGE,
	INSTR_IFGT,
	INSTR_IFLE,

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
	INSTR_AND,
	INSTR_OR,

	INSTR_ALLOC,
	INSTR_PRINT,
	INSTR_FORMAT,
	INSTR_PROC,
	INSTR_ENDPROC,
	INSTR_CALL,
	INSTR_VAR_ARG,
	INSTR_STR_ARG,			// generate str
	INSTR_DATA,
	INSTR_FILE,
	INSTR_ALIGN,
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

/*************************************************************

  Type

*************************************************************/

Type * TypeAlloc(TypeVariant variant);
Type * TypeAllocInt(Int32 min, Int32 max);
Type * TypeDerive(Type * base);
Type * TypeCopy(Type * base);

Type * TypeByte();
Type * TypeLongInt();

UInt16 TypeItemCount(Type * type);
void TypeLimits(Type * type, Var ** p_min, Var ** p_max);

void TypeLet(Type * type, Var * var);
typedef void (*RangeTransform)(Int32 * x, Int32 tr);
void TypeTransform(Type * type, Var * var, InstrOp op);

void TypeAddConst(Type * type, Var * var);
Bool TypeIsSubsetOf(Type * type, Type * master);


void VarInit();
void VarInitRegisters();

Var * VarFirst();
#define VarNext(v) (v)->next

void SetScope(Var * new_scope);
void EnterLocalScope();
void EnterSubscope(Var * new_scope);
void ExitScope();

Var * VarNewInt(long n);
Var * VarNewStr(char * str);
Var * VarNewLabel(char * name);
Var * VarNewTmp(long idx, Type * type);
Var * VarNewTmpLabel();
Var * VarAlloc(VarMode mode, Name name, VarIdx idx);
Var * VarFind(Name name, VarIdx idx);
Var * VarFindScope(Var * scope, char * name, VarIdx idx);
Var * VarFind2(char * name, VarIdx idx);
Var * VarFindInt(Var * scope, UInt32 n);
Var * VarMacroArg(UInt8 i);

Bool VarIsConst(Var * var);
Bool VarIsLabel(Var * var);
Bool VarIsArray(Var * var);
Bool VarIsTmp(Var * var);

extern Var * REG[64];		// Array of registers (register is variable with address in REGSET, submode has flag SUBMODE_REG)
extern UInt8 REG_CNT;		// Count of registers

TypeVariant VarType(Var * var);
long VarParamCount(Var * var);

void VarGenerateArrays();
void VarToLabel(Var * var);

Var * VarNewType(TypeVariant variant);

UInt16 AdrSize();

Var * FirstArg(Var * proc, VarSubmode submode);
Var * NextArg(Var * proc, Var * arg, VarSubmode submode);


Var * VarNewElement(Var * arr, Var * idx, Bool ref);

void VarResetUse();

void VarFree(Var * var);

#define FOR_EACH_VAR(v) for(v = VARS; v != NULL; v = v->next) {
#define NEXT_VAR }

void PrintVar(Var * var);
void PrintVarName(Var * var);

/***********************************************************

  Instructions

***********************************************************/

#define IS_INSTR_BRANCH(x) ((x)>=INSTR_IFEQ && (x)<=INSTR_IFLE)
#define IS_INSTR_JUMP(x) (IS_INSTR_BRANCH(x) || (x) == INSTR_GOTO)

InstrOp OpNot(InstrOp op);

/*
Compiler instruction.
*/

struct InstrTag {
	InstrOp op;
	UInt8 flags[3];		// live

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

	Instr * next, * prev;
};


// Instr blocks are in loop.

struct InstrBlockTag {
	Instr * first, * last;
};

void InstrInit();
void InstrPrint(Instr * i);
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

void Gen(InstrOp op, Var * result, Var * arg1, Var * arg2);
void GenLabel(Var * var);
void GenGoto(Var * var);
void GenBlock(InstrBlock * blk);
void GenMacro(InstrBlock * code, Var * macro, Var ** args);
void GenLastResult(Var * var);

Instr * InstrDelete(InstrBlock * blk, Instr * i);
void InstrInsert(InstrBlock * blk, Instr * before, InstrOp op, Var * result, Var * arg1, Var * arg2);

extern Var * SCOPE;		// currently parsed variable (procedure, macro)
extern Var * REGSET;	// enumerator with register sets

void CodePrint(InstrBlock * blk);

void InstrVarUse(InstrBlock * code, Instr * from, Instr * to);
void VarUse();

Var * InstrEvalConst(InstrOp op, Var * arg1, Var * arg2);

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
	RULE_DEREF,			// dereference of variable
	RULE_ARG			// argument (type of argument is defined, may be NULL)
} RuleArgVariant;

struct RuleArgTag {
	RuleArgVariant variant;
	UInt8          arg_no;		// argument index (1..26)  used for variable & const
	union {
	Var * var;
	Type * type;
	Int32 n;
	};
	RuleArg  * index;		// pointer to index for array (NULL if there is no index)
							// Rule arg is allocated in this case
							// This type must be RULE_VARIABLE or RULE_REGISTER
							// Type of that variable must be TYPE_ARRAY
};

struct RuleTag {
	Rule * next;
	InstrOp op;
	RuleArg arg[3];
	InstrBlock * to;
};

void RuleRegister(Rule * rule);
Bool RuleMatch(Rule * rule, Instr * i);

Bool ProcTranslate(Var * proc);

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
Bool Parse(char * name);

/*************************************************************

 Optimize phase

*************************************************************/
void Optimize(Var * proc);

/*************************************************************

 Emit phase

*************************************************************/

#define BLUE 1
#define GREEN 2
#define RED 4
#define LIGHT 8
void PrintColor(UInt8 color);

Rule * EmitRule(Instr * instr);
Bool EmitOpen(char * filename);
void EmitClose();
Bool EmitInstr(Instr * code);
Bool EmitProc(Var * proc);
void EmitLabels();
void EmitProcedures();

extern Bool VERBOSE;

