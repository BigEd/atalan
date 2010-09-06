/*

Parser

Read tokens from 'lexer.c' and generate instructions using 'instr.c'.

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

/*
Syntax:

	{  }   means at the specified place, block is expected (in whatever form)
	[  ]   optional part
	[  ]*  zero or more repeates of the part
	  |    option
	"sk"   verbatim text
	<rule> reference to other rule

*/

#include "language.h"

// How many vars in a row are processed before error
#define MAX_VARS_COMMA_SEPARATED 100

GLOBAL Bool  SYSTEM_PARSE;  // if set to true, we are parsing system information and line tokens do not get generated

void ParseAssign(VarMode mode, VarSubmode submode, Type * to_type);
UInt16 ParseSubExpression(Type * result_type);
void ParseCall(Var * proc);
Type * ParseType();
Var * ParseArrayElement(Var * arr, Bool ref);
Var * ParseStructElement(Var * arr);

// This variable is set to true, when we parse expression inside condition.
// It modifies parsing behaviour concernign and, or and not.

UInt8 G_CONDITION_EXP;

#define STR_NO_EOL 1
void ParseString(UInt32 flags);
void ParseExpressionType(Type * result_type);

// All rules share one common scope, so they do not mix with normal scope of program.

Var * RULE_SCOPE;

extern Var * LAST_VAR;

void ParseVariable(Var ** p_var)
{
	Var * var;
	var = VarFind2(LEX.name, 0);
	*p_var = var;
	if (var == NULL) {
		SyntaxError("Unknown variable");
	} else {
		NextToken();
	}
}

void ParseArgList(VarMode mode, Type * to_type)
/*
Purpose:
	Parse block with list of arguments.
	  [">" | "<"] assign
	Arguments are added to current context with submode SUBMODE_ARG_*.

	This method is used when parsing function or macro.
*/
{
	VarSubmode submode = SUBMODE_EMPTY;

 	EnterBlockWithStop(TOKEN_EQUAL);			// TOKEN_EQUAL

	while (TOK != TOKEN_ERROR && !NextIs(TOKEN_BLOCK_END)) {

		submode = SUBMODE_ARG_IN;
		if (NextIs(TOKEN_LOWER)) {
			submode = SUBMODE_ARG_IN;
		}
		if (NextIs(TOKEN_HIGHER)) {
			submode = SUBMODE_ARG_OUT;
		}

		if (TOK == TOKEN_ID) {
			ParseAssign(mode, submode, to_type);
			NextIs(TOKEN_EOL);
		} else {
			SyntaxError("Expected variable name");
		}
	}
}

Type * ParseIntType()
{
	Type * type = NULL;
	Var * var;
	if (TOK == TOKEN_INT) {
		type = TypeAlloc(TYPE_INT);
		type->range.min = LEX.n;
		NextToken();
		if (TOK == TOKEN_DOTDOT) {
			NextToken();
			if (TOK == TOKEN_INT) {
				type->range.max = LEX.n;
				NextToken();
			}
		} else {
			type->range.max = type->range.min;
			type->range.min = 0;
		}

		if (type->range.min > type->range.max) {
			SyntaxError("range minimum bigger than maximum");
		}
	// Sme variable
	} else if (TOK == TOKEN_ID) {
		var = VarFind2(LEX.name, 0);
		if (var != NULL) {
			type = var->type;
			NextToken();
		} else {
			SyntaxError("$unknown variable");
		}
	} else {
		SyntaxError("Expected definition of integer type");
	}
	return type;
}

Type * ParseType2(VarMode mode)
/*
Purpose:
	Parse: <int> [".." <int>] | <var> | proc <VarList>
Input:
	for_var	Variable, for which we parse the type
			May be NULL
*/
{
	
	Var * var;
	Type * type = NULL;
	long last_n = 0;
	long i;
	Type * elmt;
	Bool id_required;

	if (NextIs(TOKEN_ENUM)) {
		type = TypeAlloc(TYPE_INT);
		type->range.flexible = true;
		if (TOK == TOKEN_INT) goto range;
		goto const_list;

	} else if (TOK == TOKEN_INT) {
		type = TypeAlloc(TYPE_INT);
range:
		type->range.min = LEX.n;
		NextToken();
		if (TOK == TOKEN_DOTDOT) {
			NextToken();
			if (TOK == TOKEN_INT) {
				type->range.max = LEX.n;
			}
		} else {
			type->range.max = type->range.min;
			type->range.min = 0;
		}

		if (type->range.min > type->range.max) {
			SyntaxError("range minimum bigger than maximum");
		} else {
			NextToken();
		}
const_list:
		// Parse type specific constants
		// There can be list of constants specified in block.
		// First thing in the block must be an identifier, so we try to open the block with this in mind.

//		if (TOK != TOKEN_COMMA && TOK != TOKEN_EQUAL && TOK != TOKEN_BLOCK_END) {
			EnterBlockWithStop(TOKEN_VOID);
		
			id_required = false;

			while (TOK != TOKEN_ERROR && !NextIs(TOKEN_BLOCK_END)) {

				while(NextIs(TOKEN_EOL));

				if (TOK == TOKEN_ID || TOK >= TOKEN_KEYWORD) {
					var = VarAlloc(MODE_CONST, LEX.name, 0);
					NextToken();
					if (NextIs(TOKEN_EQUAL)) {
						// Parse const expression
						if (TOK == TOKEN_INT) {
							last_n = LEX.n;
							NextToken();
						} else {
							SyntaxError("expected integer value");
						}
					} else {
						last_n++;
					}
					var->n = last_n;
					var->value_nonempty = true;

					TypeAddConst(type, var);

				} else {
					if (id_required) {
						SyntaxError("expected constant identifier");
					} else {
						ExitBlock();
						break;
					}
				}
				id_required = false;
				// One code may be ended either by comma or by new line
				if (NextIs(TOKEN_COMMA)) id_required = true;
				NextIs(TOKEN_EOL);
			}
//			printf("done");
//		}

	// Procedure
	} else if (NextIs(TOKEN_PROC)) {
		type = TypeAlloc(TYPE_PROC);
		ParseArgList(MODE_ARG, type);

	// Macro
	} else if (NextIs(TOKEN_MACRO)) {

		type = TypeAlloc(TYPE_MACRO);
		ParseArgList(MODE_ARG, type);

	// Struct
	} else if (NextIs(TOKEN_STRUCT)) {
		type = TypeAlloc(TYPE_STRUCT);
		ParseArgList(MODE_VAR, type);

	// Array
	} else if (NextIs(TOKEN_ARRAY)) {		
		type = TypeAlloc(TYPE_ARRAY);
		i = 0;

		if (TOK == TOKEN_OPEN_P) {
			EnterBlockWithStop(TOKEN_EQUAL);
			while (TOK != TOKEN_ERROR && !NextIs(TOKEN_BLOCK_END)) {
				elmt = ParseIntType();
				if (elmt != NULL) {
					if (i == MAX_DIM_COUNT) {
						SyntaxError("too many array indices");
						return NULL;
					}
					type->dim[i] = elmt;
					i++;
				}	
				NextIs(TOKEN_COMMA);
			};
		}
		
		// If no dimension has been defined, use flexible array.
		// This is possible only for constatnts now.

		if (TOK) {
			if (type->dim[0] == NULL) {
				elmt = TypeAlloc(TYPE_INT);
				elmt->range.flexible = true;
				elmt->range.min = 0;
				type->dim[0] = elmt;
			}
		}

		if (TOK) {
			if (NextIs(TOKEN_OF)) {
				type->element = ParseType();
			} else {
				type->element = TypeByte();
			}
		}

	} else if (NextIs(TOKEN_ADR2)) {		
		type = TypeAlloc(TYPE_ADR);
		if (NextIs(TOKEN_OF)) {
			type->element = ParseType();
		} else {
			type->element = TypeByte();
		}


	} else if (TOK == TOKEN_ID) {
		ParseVariable(&var);
		if (TOK != TOKEN_ERROR) {
			if (mode == MODE_TYPE) {
				type = TypeDerive(var->type);
			} else {
				type = var->type;
			}
		}
	}
	return type;
}

Type * ParseType()
{
	return ParseType2(MODE_VAR);
}

void ParseCommands();

/*
void PrintStack()
{
	long n;
	Var * var;

	for(n=0; n<TOP; n++) {
		var = STACK[n];
		if (var == NULL) {
			printf("<NULL> ");
		} else {
			if (var->name == NULL) {
				printf("%ld ", var->n);
			} else {
				printf((var->idx != 0)?"%s%ld ":"%s ", var->name, var->idx);
			}
		}
	}
	printf("\n");

}

#define MINIMAL_PRIORITY 0
#define MAXIMAL_PRIORITY 65535
*/

/*********************************************************************

  Parse Expression

*********************************************************************/
//$E

#define STACK_LIMIT 100
Var *  STACK[STACK_LIMIT];
UInt16 TOP;
Type * RESULT_TYPE;
int    G_TEMP_CNT;

Type   EXP_TYPE;			// Type returned by expression
							// Is modified as the expression gets generated

void ParseExpRoot();

void InstrBinary(InstrOp op)
/*
Purpose:
	Generate binary instruction as part of expression.
*/
{
	Var * result, * arg1, * arg2;
	Type * type;

	arg1 = STACK[TOP-2];
	arg2 = STACK[TOP-1];

	//TODO: Should not we free temporaries only after they were generated?

	VarFree(arg1);	//if (VarIsTmp(STACK[TOP-1])) G_TEMP_CNT--;
	VarFree(arg2);	//if (VarIsTmp(STACK[TOP-2])) G_TEMP_CNT--;

	// Todo: we may use bigger of the two
	if (RESULT_TYPE == NULL) {
		RESULT_TYPE = STACK[TOP-2]->type;
	}

	// Try to evaluate the instruction as constant.
	// If we succeed, no instruction is generated, we insted push the result on stack

	result = InstrEvalConst(op, arg1, arg2);
	if (result == NULL) {

		if (EXP_TYPE.variant == TYPE_UNDEFINED) {
			TypeLet(&EXP_TYPE, arg1);
		}

		TypeTransform(&EXP_TYPE, arg2, op);

//		type = RESULT_TYPE;
//		if (type != NULL && type->variant == TYPE_ARRAY) {
//			type = type->element;
//		}

		//TODO: Other than numeric types (
		type = TypeCopy(&EXP_TYPE);
		result = VarNewTmp(G_TEMP_CNT, type);
		G_TEMP_CNT++;
		Gen(op, result, arg1, arg2);
	}

	TOP--;
	STACK[TOP-1] = result;
//	PrintStack();
}

void InstrUnary(InstrOp op)
/*
Purpose:
	Generate unary instruction in expression.
*/
{
	Var * result;
	Var * top;

	long n1, r;
//	PrintStack();

	top = STACK[TOP-1];
	if (VarIsTmp(top)) G_TEMP_CNT--;

	// Todo: we may use bigger of the two

	if (RESULT_TYPE == NULL) {
		switch(op) {
		// HI and LO return always byte type
		case INSTR_HI:
		case INSTR_LO:
			RESULT_TYPE = TypeByte();
			break;
		default:
			RESULT_TYPE = top->type;
		}
	}
	if (VarIsConst(top)) {
		if (top->type->variant == TYPE_INT) {
			n1 = STACK[TOP-1]->n;
			switch(op) {
			case INSTR_HI: r = (n1 >> 8) & 0xff; break;
			case INSTR_LO: r = n1 & 0xff; break;
			default: break;
			}
			result = VarNewInt(r);
			goto done;
		}
	}
	result = VarNewTmp(G_TEMP_CNT, RESULT_TYPE);
	G_TEMP_CNT++;
	Gen(op, result, top, NULL);
done:
	STACK[TOP-1] = result;
}

void ParseParenthesis()
{
	EnterBlock();
	ParseExpRoot();
	if (!NextIs(TOKEN_BLOCK_END)) SyntaxError("missing closing ')'");
}

UInt8 ParseArgNo2()
/*
Parse %A - %Z and return it as number 1..26.
Return 0, if there is not such argument.
*/
{
	UInt8 arg_no = 0;
	UInt8 c;

	if (NextIs(TOKEN_PERCENT)) {
		if (TOK == TOKEN_ID) {
			c = LEX.name[0];
			if (LEX.name[1] == 0 && c >= 'A' && c <='Z') {
				arg_no = c - 'A' + 1;
			} else {
				SyntaxError("Rule argument mast be A..Z");
			}
		}
	}
	return arg_no;
}

UInt8 ParseArgNo()
{
	UInt8 arg_no = ParseArgNo2();
	if (arg_no > 0) NextToken();
	return arg_no;
}

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

	rtype = type;
	vtype = var->type;

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
		if (var->mode == MODE_CONST) {
			if (var->n < type->range.min) return false;
			if (var->n > type->range.max) return false;
		} else if (var->mode == MODE_VAR || var->mode == MODE_ARG || var->mode == MODE_ELEMENT) {
			if (vtype->range.min < type->range.min) return false;
			if (vtype->range.max > type->range.max) return false;
		} else {
			return false;
		}
	}

	return true;
}

void CheckArrayBound(UInt16 no, Var * arr, Type * idx_type, Var * idx, UInt16 bookmark)
/*
Purpose:
	Test, whether the array index fits array bounds.
	If not, report error.
*/
{
	if (idx_type != NULL) {
		if (!VarMatchType(idx, idx_type)) {
			if (idx->mode == MODE_CONST) {
				LogicWarning("array index is out of bounds", bookmark);
			} else {
				LogicWarning("array index may get out of bounds", bookmark);
			}
		}
	}
}

Var * ParseStructElement(Var * arr)
/*
Purpose:
	Parse access to structure element.
Syntax:  
	Member: "." <id>
*/
{
	Var * idx = NULL;
	Var * item;

	if (arr->type->variant == TYPE_STRUCT) {
		NextIs(TOKEN_DOT);
		if (TOK == TOKEN_ID) {
			item = VarFindScope(arr->type->owner, LEX.name, 0);
			if (item != NULL) {
				idx = VarNewElement(arr, item, false);
			} else {
				SyntaxError("$Structure does not contain member with name");
			}
			NextToken();
		} else {
			SyntaxError("Expected structure member identifier after '.'");
		}
	} else {
		SyntaxError("Variable has no members");
	}
	return idx;
}

Var * ParseArrayElement(Var * arr, Bool ref)
/*
Purpose:
	Parse access to array element.
	Parsed variable is of type element.
*/
{
	UInt16 top;
	Type * idx_type;
	Var * idx, * idx2, * item;
	UInt16 bookmark;

	// This is always () block
	EnterBlock();

//	NextIs(TOKEN_OPEN_P);

	top = TOP;

	// First dimension

	if (arr->type != NULL) {
		if (arr->type->variant == TYPE_ARRAY) {
			idx_type = arr->type->dim[0];
		} else {
			idx_type = TypeByte();
		}
	} else {
		idx_type = NULL;
	}

	bookmark = SetBookmark();
	ParseSubExpression(idx_type);	//Root();
	if (TOK) {
		idx = STACK[top];

		CheckArrayBound(0, arr, idx_type, idx, bookmark);

		// Second dimension

		idx_type = NULL;
		if (arr->type != NULL && arr->type->variant == TYPE_ARRAY) idx_type = arr->type->dim[1];

		if (NextIs(TOKEN_COMMA)) {
			if (idx_type != NULL) {
				TOP = top;
				bookmark = SetBookmark();
				ParseSubExpression(idx_type);
				idx2 = STACK[TOP-1];

				if (VarIsTmp(idx2)) G_TEMP_CNT--;
				CheckArrayBound(1, arr, idx_type, idx2, bookmark);

				idx = VarNewElement(idx, idx2, false);
			} else {
				SyntaxError("Array has only one dimension");
			}
		}

		if (!NextIs(TOKEN_BLOCK_END)) SyntaxError("missing closing ')'");
	}

	//ParseParenthesis();
	// Result is temporary variable created by InstrBinary, 
	// arg1 = array, arg2 = item

	//TODO: Try to find simmilar array access (same array, same index)

	item = VarNewElement(arr, idx, ref);

	return item;
}

void ParseOperand()
{
	Var * var = NULL, * item = NULL, * proc;
	Bool ref = false;
	Bool type_match;
	UInt8 arg_no;
	Bool spaces;

	if (TOK == TOKEN_OPEN_P) {
		ParseParenthesis();
	} else {
		// @id denotes reference to variable
		if (TOK == TOKEN_ADR) {
			NextToken();
			ref = true;
			goto id;
		} else if (arg_no = ParseArgNo2()) {
			var = VarMacroArg(arg_no-1);
			goto indices;

		} else if (TOK == TOKEN_INT) {
			var = VarNewInt(LEX.n);
			NextToken();
		} else if (TOK == TOKEN_ID) {
id:
			var = VarFind2(LEX.name, 0);

			//TODO: We should try to search for the scoped constant also in case the resulting type
			//      does not conform to requested result type

			if (var != NULL) {
				type_match = VarMatchType(var, RESULT_TYPE);
			}

			// Try to find using result scope (support for associated constants)
			if (var == NULL || !type_match) {
				if (RESULT_TYPE != NULL) {
					item = VarFindScope(RESULT_TYPE->owner, LEX.name, 0); 
					if (item != NULL) var = item;
				}
			}

			if (var == NULL) {
				SyntaxError("$unknown variable");
				//TODO: Try to search in all scopes and list found places
				//TODO: Try to search using edit distance
				return;
			}

			// Assign address
			if (RESULT_TYPE != NULL && RESULT_TYPE->variant == TYPE_ADR && var->type->variant != TYPE_ADR) {
				NextToken();
				//TODO: Check type of the adress
				//      Create temporary variable and generate letadr
				STACK[TOP] = var;
				TOP++;					
				InstrUnary(INSTR_LET_ADR);
				return;
			}

			// Function call
			if (var->type != NULL && var->type->variant == TYPE_PROC) {
				proc = var;
				NextToken();
				ParseCall(proc);

				// Output arguments of procedure are stored on stack
				var = FirstArg(proc, SUBMODE_ARG_OUT);
				if (var != NULL) {
					do {
						STACK[TOP] = var;
						TOP++;					
					} while (var = NextArg(proc, var, SUBMODE_ARG_OUT));
				} else {
					SyntaxError("PROC does not return any result");
				}
				return;
			}
indices:
			spaces = Spaces();
			NextToken();
			if (!spaces) {
				if (NextIs(TOKEN_DOT)) {
					if (var->type->variant == TYPE_STRUCT) {
						var = ParseStructElement(var);
					} else {
						if (TOK == TOKEN_ID) {
							item = VarFindScope(var, LEX.name, 0);

							// If the element has not been found, try to match some built-in elements

							if (item == NULL) {
								if (var->type->variant == TYPE_INT) {
									if (StrEqual(LEX.name, "min")) {
										item = VarNewInt(var->type->range.min);
									} else if (StrEqual(LEX.name, "max")) {
										item = VarNewInt(var->type->range.max);
									}
								}
							}

							if (item != NULL) {
								var = item;
								NextToken();
							} else {
								SyntaxError("$unknown item");
							}
						} else {
							SyntaxError("variable name expected after .");
						}
					}

				// Access to array may be like
				// 
				} else if (TOK == TOKEN_OPEN_P) {

					item = ParseArrayElement(var, ref);

					if (VarIsTmp(STACK[TOP-1])) G_TEMP_CNT--;
					STACK[TOP-1] = item;

					return;
				}
			}// else {
			//	NextToken();
			//}
		} else {
//			SyntaxError("expected operand");
			return;
		}
		STACK[TOP] = var;
		TOP++;
	}
}

void ParseUnary()
{
	if (NextIs(TOKEN_HI)) {
		ParseOperand();
		InstrUnary(INSTR_HI);
	} else if (NextIs(TOKEN_LO)) {
		ParseOperand();
		InstrUnary(INSTR_LO);
	}  else if (NextIs(TOKEN_NOT)) {
		ParseOperand();
		InstrUnary(INSTR_NOT);
	} else {
		ParseOperand();
	}
}

void ParseMulDiv()
{
	ParseUnary();
retry:
	if (NextIs(TOKEN_MUL)) {
		ParseUnary();
		if (TOK) {
			InstrBinary(INSTR_MUL);
		}
		goto retry;
	} else if (NextIs(TOKEN_DIV)) {
		ParseUnary();
		if (TOK) {
			InstrBinary(INSTR_DIV);
		}
		goto retry;
	}  else if (NextIs(TOKEN_MOD)) {
		ParseUnary();
		if (TOK) {
			InstrBinary(INSTR_MOD);
		}
		goto retry;
	}
}

void ParsePlusMinus()
{
	ParseMulDiv();
retry:
	if (NextIs(TOKEN_PLUS)) {
		ParseMulDiv();
		if (TOK) {
			InstrBinary(INSTR_ADD);
		}
		goto retry;
	} else if (NextIs(TOKEN_MINUS)) {
		ParseMulDiv();
		if (TOK) {
			InstrBinary(INSTR_SUB);
		}
		goto retry;
	}
}

void ParseBinaryAnd()
{
retry:
	ParsePlusMinus();
	if (!G_CONDITION_EXP && NextIs(TOKEN_AND)) {
		ParsePlusMinus();
		if (TOK) {
			InstrBinary(INSTR_AND);
		}
		goto retry;
	}
}

void ParseBinaryOr()
{
retry:
	ParseBinaryAnd();
	if (!G_CONDITION_EXP && NextIs(TOKEN_OR)) {
		ParseBinaryAnd();
		if (TOK) {
			InstrBinary(INSTR_OR);
		}
		goto retry;
	} else if (NextIs(TOKEN_XOR)) {
		ParseBinaryAnd();
		if (TOK) {
			InstrBinary(INSTR_XOR);
		}
		goto retry;
	}
}

void ParseExpRoot()
{
	ParseBinaryOr();
}

typedef struct {
	Type    type;			// inferred type of expression
	Type *  result;			// expected type of resulting value
							// For example type of variable, into which the expression result 
							// gets assigned.
	UInt16 top;				// top of the stack when the parsing started
} ExpState;

UInt16 ParseSubExpression(Type * result_type)
/*
Purpose:
	Subexpression must be parsed, when we parse expression as part of parsing some complex expression.
	For example array indexes of function call arguments.

	At this moment, stack may contain some temporary results and expression type is partially
	evaluated. We must save this state and restore it after evaluation.
Result:
	Number of variables generated.
	Caller is responsible for consuming the generated variables from stack (by popping them).
*/
{
	ExpState state;
	memcpy(&state.type, &EXP_TYPE, sizeof(Type));
	state.result = RESULT_TYPE;
	state.top    = TOP;

	RESULT_TYPE = result_type;
	EXP_TYPE.variant = TYPE_UNDEFINED;

	ParseExpRoot();

	memcpy(&EXP_TYPE, &state.type, sizeof(Type));
	RESULT_TYPE = state.result;

	return TOP - state.top;
}

void ParseExpressionType(Type * result_type)
{
	RESULT_TYPE = result_type;
	TOP = 0;
	EXP_TYPE.variant = TYPE_UNDEFINED;
	ParseExpRoot();
}

void ParseExpression(Var * result)
/*
Parse expression, performing evaluation if possible.
If result mode is MODE_CONST, no code is to be generated.
*/
{
	if (result == NULL) {
		RESULT_TYPE = NULL;
	} else {
		if (result->mode == MODE_ELEMENT) {
			RESULT_TYPE = result->adr->type->element;
		} else {
			RESULT_TYPE = result->type;
		}
	}
	TOP = 0;
	EXP_TYPE.variant = TYPE_UNDEFINED;
	ParseExpRoot();

	// When we parse very simple expressions, no instruction gets generated
	// Calling code would typically call generating let instruction.
	// We simulate it for the type here, so the type returned from expression
	// parsing is correct.

	if (TOK) {
		if (EXP_TYPE.variant == TYPE_UNDEFINED) {
			TypeLet(&EXP_TYPE, STACK[0]);
		}
	}
}

void ExpectExpression(Var * result)
{
	ParseExpression(result);
	if (TOK) {
		if (TOP == 0) {
			SyntaxError("expected expression");
		}
	}
}

Bool  G_NOT;

void ParseCondition();

GLOBAL Block   G_BLOCKS[100];
GLOBAL Block * G_BLOCK;

void BeginBlock(Token command)
/*
Purpose:
	This method is called, when we start processing program flow control command.
*/
{
	Block * blk;
	Var * loop_label;
	blk = G_BLOCK+1;
	blk->command = command;

	// Do not generate label for if, as we are not going to repeat the if, so it would be never jumped anyways
	loop_label = NULL;
	if (command != TOKEN_IF) {
		loop_label = VarNewTmpLabel();
	}

	blk->body_label = NULL;
	blk->loop_label = loop_label;
	blk->t_label = NULL;
	blk->f_label = NULL;
	blk->not     = false;

	G_BLOCK = blk;

//	EnterBlock();		// instruct parser to parse block
//	ParseCommands();
//	NextIs(TOKEN_BLOCK_END);	// Block must end with TOKEN_END_BLOCK
}

void ParseBlock()
{
	EnterBlock();		// To each command block there is appropriate lexer block
	ParseCommands();
	NextIs(TOKEN_BLOCK_END);	// Block must end with TOKEN_END_BLOCK
}

void EndBlock()
{
	G_BLOCK--;
}

void ParseCondParenthesis()
{
	EnterBlock();
	ParseCondition();
	if (!NextIs(TOKEN_BLOCK_END)) SyntaxError("missing closing ')'");
}

InstrOp RelInstrFromToken()
{
	InstrOp op;

	switch(TOK) {
	case TOKEN_EQUAL:        op = INSTR_IFEQ; break;
	case TOKEN_NOT_EQUAL:    op = INSTR_IFNE; break;
	case TOKEN_LOWER:        op = INSTR_IFLT; break;
	case TOKEN_HIGHER:       op = INSTR_IFGT; break;
	case TOKEN_LOWER_EQUAL:  op = INSTR_IFLE; break;
	case TOKEN_HIGHER_EQUAL: op = INSTR_IFGE; break;
	default: op = INSTR_VOID;
	}
	return op;
}

void ParseRel()
/*
	relop: "=" | "<>" | "<" | "<=" | ">" | ">="
	rel:  <exp> <relop> <exp> [<relop> <exp>]*
*/
{
	Var * v1;
	InstrOp op;

	if (TOK == TOKEN_OPEN_P) {
		ParseCondParenthesis();
	} else {
		G_CONDITION_EXP = true;
		ParseExpression(NULL);
		G_CONDITION_EXP = false;
		v1 = STACK[0];

		while ((op = RelInstrFromToken()) != INSTR_VOID) {

			// For normal operation, we jump to false label when the condition does NOT apply
			// For example for if:
			// if <cond>
			//     <block>
			//
			// must skip the <block>.			

			if (!G_BLOCK->not) op = OpNot(op);
			NextToken();
			G_CONDITION_EXP = true;
			ParseExpression(v1);
			G_CONDITION_EXP = false;
			if (TOK != TOKEN_ERROR) {
				if (G_BLOCK->f_label == NULL) {
					G_BLOCK->f_label = VarNewTmpLabel();
				}
				Gen(op, G_BLOCK->f_label, v1, STACK[0]);
				VarFree(v1);
				v1 = STACK[0];		//
									//VarFree(STACK[0]);
			}
		}
		VarFree(v1);
	}
}

void ParseAnd()
{
	// if x <> 2 and x <> 3 and x <> 4 then "x"
	//
	// should be translated as
	//
	// if x <> 2
	//    if x <> 3
	//       if x <> 4
	//          "x" 

retry:
	ParseRel();
	if (NextIs(TOKEN_AND)) {
		if (G_BLOCK->t_label != NULL) {
			GenLabel(G_BLOCK->t_label);
			G_BLOCK->t_label = NULL;
		}
		goto retry;
	}
}

void ParseCondition()
{
	// if x=1 or x=2 or x=3 then "x"
	//
	// should be translated as
	//
	//   if x = 1 goto body
	//   if x = 2 goto body
	//   if x = 3 goto body
	//   goto exit
	//body@
	//   "[x]"
	//exit@
	//
	//
	// 1. Because of normal if, the first condiion gets translated like:
	//
	//   if not x = 1 goto f1		(false)
	//   "[x]"
	//f1@
	//
	// 2. We need to invert the condition back:
	//
	//   if not x = 1 goto f1		(false)
	//   goto @body
	//@f1

	Var * body_label = NULL;

retry:
	ParseAnd();
	if (NextIs(TOKEN_OR)) {

		// If the condition was more complex and generated true label,
		// the true label would point to this jump

		if (G_BLOCK->t_label != NULL) {
			GenLabel(G_BLOCK->t_label);
			G_BLOCK->t_label = NULL;
		}

		if (body_label == NULL) body_label = VarNewTmpLabel();

		GenGoto(body_label);

		if (G_BLOCK->f_label != NULL) {
			GenLabel(G_BLOCK->f_label);
			G_BLOCK->f_label = NULL;
		}
		goto retry;
	}
	GenLabel(body_label);

}

void ParseLabel(Var ** p_label)
{
	
	Var * var = NULL;

	ExpectToken(TOKEN_ID);
	if (TOK == TOKEN_ID) {
		var = VarFind2(LEX.name, 0);
		if (var == NULL) {
			var = VarNewLabel(LEX.name);
//			SyntaxError("Unknown label");
		}
		NextToken();
	}
	*p_label = var;
	
}

void ParseGoto()
{
	
	Var * var;
	ParseLabel(&var);
	if (TOK != TOKEN_ERROR) {
		if (VarIsLabel(var)) {
			GenGoto(var);
		} else {
			var = VarNewElement(var, VarNewInt(0), true);
			GenGoto(var);
		}
	}
	
}

void ParseIf()
/*
Syntax:
	If: "if" <commands> ["then"] <commands>  ["else" "if" <cond>]* ["else" <commands>]
*/
{	
	BeginBlock(TOKEN_IF);		// begin if block
retry:
	NextToken();				// skip if
	ParseCondition();
	if (TOK == TOKEN_ERROR) return;

	// If condition referenced true label (which is not necessary, if it didn't contain AND or OR),
	// generate it here

	if (G_BLOCK->t_label != NULL) {
		GenLabel(G_BLOCK->t_label);
	}

	// There may be optional THEN after IF
	NextIs(TOKEN_THEN);

		EnterBlockWithStop(TOKEN_ELSE);
		ParseCommands();
		NextIs(TOKEN_BLOCK_END);	// Block must end with TOKEN_END_BLOCK
		if (NextIs(TOKEN_ELSE)) {
			
			// End current branch with jump after the end of if
			if (G_BLOCK->loop_label == NULL) {
				G_BLOCK->loop_label = VarNewTmpLabel();
			}
			GenGoto(G_BLOCK->loop_label);
			GenLabel(G_BLOCK->f_label);			// previous branch will jump here

			// else if
			if (TOK == TOKEN_IF) {
				G_BLOCK->f_label = NULL;		// expression will generate new labels if necesary
				G_BLOCK->t_label = NULL;
				goto retry;
			// else
			} else {
//				GenLabel(G_BLOCK->f_label);		// jump to else case here
				ParseBlock();
			}
		// No else
		} else {
			GenLabel(G_BLOCK->f_label);
		}

		// This is complete end of 'IF'
		if (G_BLOCK->loop_label != NULL) {
			GenLabel(G_BLOCK->loop_label);
		}
//	}
	EndBlock();
}

void ParseRange(Var ** p_min, Var ** p_max)
{
	Type * type;
	Var * min, * max;

	min = NULL; max = NULL;
	type = NULL;

	ParseExpression(NULL);
	min = STACK[0];
	if (NextIs(TOKEN_DOTDOT)) {
		ParseExpression(NULL);
		max = STACK[0];
	} else {
		// If there are multiple values on stack, we may use the second value as loop maximal value
		if (TOP > 1) {
			max = STACK[1];
			//TODO: Free other variables on stack
		} else {
			if (min->mode == MODE_CONST) {
				max = min;
				min = VarNewInt(0);
			} else {
				if (min->mode == MODE_TYPE) {
					type = min->type;
				} else if (min->mode == MODE_VAR || min->mode == MODE_ARG) {
					type = min->type;
				}
		
				if (type->variant != TYPE_INT) {
					SyntaxError("Expected integer type or variable $");
				} else {
					TypeLimits(type, &min, &max);
				}
			}
		}
	}
	*p_min = min;
	*p_max = max;
}

Type * TypeAllocRange(Var * min, Var * max)
/*
Purpose:
	Create integer type, that will be able to contain range specified by the two variables.
*/
{
	Int32 nmin = 0, nmax = 0;
	Int32 mmin = 0, mmax = 0;
	Int32 l;
	Type * type = NULL;

	if (min->mode == MODE_CONST) {
		if (min->type->variant == TYPE_INT) {
			nmin = min->n;
		} else {
			SyntaxError("Range minimum is not integer type");
		}
	} else if (min->mode == MODE_VAR || min->mode == MODE_ARG) {
		if (min->type->variant == TYPE_INT) {
			nmin = min->type->range.min;
			nmax = min->type->range.max;
		} else {
			SyntaxError("Range minimum is not integer type");
		}
	}

	if (max->mode == MODE_CONST) {
		if (max->type->variant == TYPE_INT) {
			if (max->n > nmax) nmax = max->n;
		} else {
			SyntaxError("Range maximum is not integer type");
		}
	} else if (max->mode == MODE_VAR) {
		if (max->type->variant == TYPE_INT) {
			l = max->type->range.max;
			if (l > nmax) nmax = l;
		} else {
			SyntaxError("Range maximum is not integer type");
		}
	}

	if (TOK) {
		type = TypeAllocInt(nmin, nmax);
	}
	return type;
}

void ParseFor()
/*
Syntax:
	for: "for" <var> [":" <range>] ["where" cond] ["until" cond | "while" cond]

*/
{
	Var * var, * where_t_label;
	char name[256];
	Var * min, * max;
	Type * type;
	InstrBlock * cond, * where_cond, * body;
	Int32 n;

	var = NULL; min = NULL; max = NULL; cond = NULL; where_cond = NULL;
	where_t_label = NULL;

	EnterLocalScope();

	if (NextIs(TOKEN_FOR)) {

		if (TOK == TOKEN_ID) {
			
			// Copy the name of loop variable, so we can get the next token

			strcpy(name, LEX.name);
			NextToken();

			// for i ":" <range>
			if (NextIs(TOKEN_COLON)) {
				ParseRange(&min, &max);
				if (TOK) {
					type = TypeAllocRange(min, max);
					if (TOK) {
						var = VarAlloc(MODE_VAR, name, 0);
						var->type = type;
					}
				}
			// for i (range is not specified, this is reference to global variable or type)
			} else {
				var = VarFind2(name, 0);
				if (var != NULL) {
					if (var->type->variant == TYPE_INT) {
						TypeLimits(var->type, &min, &max);
					} else {
						SyntaxError("$Loop variable must be integer");
					}
				} else {
					SyntaxError("$Loop variable not found");
				}
			}
		} else {
			SyntaxError("Expected loop variable name");
		}
	}

	if (TOK == TOKEN_ERROR) return;

	BeginBlock(TOKEN_FOR);
	
	// WHERE can be used only if there was FOR

	if (var != NULL) {
		if (NextIs(TOKEN_WHERE)) {
			G_BLOCK->f_label = G_BLOCK->loop_label;
			InstrBlockPush();
			ParseCondition();
			if (G_BLOCK->t_label != NULL) {
				GenLabel(G_BLOCK->t_label);
				G_BLOCK->t_label = NULL;
			}

			where_cond = InstrBlockPop();
			G_BLOCK->f_label = NULL;
			if (TOK == TOKEN_ERROR) goto done;
		}
	}

	if (TOK == TOKEN_UNTIL || TOK == TOKEN_WHILE) {
		if (TOK == TOKEN_UNTIL) {
			G_BLOCK->not = true;
		}
		NextToken();

		InstrBlockPush();
		ParseCondition();
		if (G_BLOCK->t_label != NULL) {
			GenLabel(G_BLOCK->t_label);
		}
		cond = InstrBlockPop();

		if (TOK == TOKEN_ERROR) goto done;
	}

	/*
		<i> = min
		goto loop_label		; only if there is condition (otherwise we expect at least one occurence)
	body_label@
	;WHERE
		<where_condition>  f_label = loop_label | t_label
	where_t_label@
		<body>
	loop_label@		
		<condition>
	t_label@
		add <i>,<i>,1
		ifle body_label,<i>,max
	f_label@
	*/

	// Parse body

	InstrBlockPush();
	ParseBlock();
	body = InstrBlockPop();
	if (TOK == TOKEN_ERROR) return;

	// Variable initialization

	if (var != NULL) {
		Gen(INSTR_LET, var, min, NULL);
	}

	if (cond != NULL) {
		GenGoto(G_BLOCK->loop_label);
	}

	// Body consists of where_cond & body

	G_BLOCK->body_label = VarNewTmpLabel();
	GenLabel(G_BLOCK->body_label);

	if (where_cond != NULL) {
		GenBlock(where_cond);
	}

	GenBlock(body);

	if (cond != NULL || where_cond != NULL) {
		GenLabel(G_BLOCK->loop_label);
	}

	// Insert condition
	if (cond != NULL) {
		GenBlock(cond);
		if (var == NULL) {
			GenGoto(G_BLOCK->body_label);
		}
	}

	if (var != NULL) {
		Gen(INSTR_ADD, var, var, VarNewInt(1));		// STEP

		// We prefer comparison usign equality.
		// On many architectures, this is faster than <=, because it has to be done using 2 instructions (carry clear, zero set)
		if (max->mode == MODE_CONST) {

			n = max->n;
			if ((n == 0xff || n == 0xffff || n == 0xffffff) && n == var->type->range.max) {
				n = 0;
			} else {
				n++;
			}


//			if (var->type->range.max == 255 && max->n == 255) {
//				max = VarNewInt(0);
//			} else {
			max = VarNewInt(n);
//			}
			Gen(INSTR_IFNE, G_BLOCK->body_label, var, max);	//TODO: Overflow
		} else {
			Gen(INSTR_IFLE, G_BLOCK->body_label, var, max);	//TODO: Overflow
		}
	}

	if (cond != NULL) {
		GenLabel(G_BLOCK->f_label);
	}
done:
	EndBlock();
	VarFree(min);
	VarFree(max);
	ExitScope();
}
/*
void ParseLoop()
{

	InstrBlock * cond, * body;

	//		goto loop_label
	//f_label@
	//body_label@
	//		<body>
	//loop_label@
	//		<condition>
	//false_label@

	if (TOK == TOKEN_UNTIL || TOK == TOKEN_WHILE) {
		BeginBlock(TOK);
		if (TOK == TOKEN_WHILE) {		//TOKEN_WHILE
			G_BLOCK->not = true;
		}
		NextToken();

		//-----
		InstrBlockPush();
		ParseCondition();
		cond = InstrBlockPop();
		if (TOK == TOKEN_ERROR) return;

		InstrBlockPush();
		ParseBlock();
		body = InstrBlockPop();
		if (TOK == TOKEN_ERROR) return;

		// Some loops do not have body, we do not generate goto & loop_label for them

		if (body != NULL) {
			GenGoto(G_BLOCK->loop_label);
		}

		if (G_BLOCK->f_label != NULL) {
			GenLabel(G_BLOCK->f_label);
		}

		GenBlock(body);

		if (body != NULL) {
			GenLabel(G_BLOCK->loop_label);
		}
		GenBlock(cond);

		if (G_BLOCK->t_label != NULL) {
			GenLabel(G_BLOCK->t_label);
		}
		EndBlock();
	}
}
*/
/*
void ParseExpression(Var * result)
{
	
	Var * var, * op, * res;
	Var * args[3];
	TypeVariant vara;

	long priority, cnt;

	STACK[0] = NULL;	// zero pointer is bound
	TOP = 0;		// current top stack

	do {
		NextToken();
		var = NULL;

		// If there is an operator with missing argument on top of stack, we should try
		// parsing the expression, even if there is EOL

		if (TOK == TOKEN_EOL) {
			op = STACK[TOP];
			if (VarType(op) == TYPE_MACRO) {
				continue;
			}
		} else if (TOK == TOKEN_INT) {
			var = VarNewInt(LEX.n);
		} else {
			var = VarFind(LEX.name, 0);
		}

		// Get priority of the variable.
		// If the variable contains macro, it has priority defined by macro (address field)
		// Other variables get maximal priority (32000).
		// If we have no variable, it gets lowest priority, which will force emitting of all
		// operators on stack.

		priority = MINIMAL_PRIORITY;
		if (var != NULL) {
			priority = MAXIMAL_PRIORITY;
			if (VarType(var) == TYPE_MACRO) {
				priority = var->adr;
			}
		}		

		vara = VarType(var);

		while (TOP > 3
			   && (op = STACK[TOP-1])!=NULL
			   && op->type->variant == TYPE_MACRO
			   && op->adr >= priority
		){

		    //cnt = VarParamCount(op);
			op = STACK[TOP-1];
		    // Macros without instructions are just discarded

			cnt = VarParamCount(op);

			// For macros without arguments, just remove the macro
			if (cnt == 0) {
				if (op->instr->op == INSTR_BEGIN) {
				}
				STACK[TOP-1] = STACK[TOP];
				TOP--; 
				continue; 
			} else if (cnt == 3) {
				// When emitting last operator of expression, use result variable.
				// Alloc new temprary variable otherwise.
				if (priority == 0 && TOP == 3) {
					res = result;
				} else {
					res = VarNewTemp(result->type);
				}

				// Create array of operands and apply the macro
				args[0] = STACK[TOP-2];
				args[1] = STACK[TOP];
				args[2] = res;
				GenMacro(op, args);

				// Pop used variables from stack and store the reference to result there
				TOP--;
				TOP--;
				STACK[TOP] = res;
			}
		}

		TOP++;
		STACK[TOP] = var;

		PrintStack();

	} while(var != NULL);

	
}
*/

void VarFree(Var * var)
{
	if (var != NULL) {
		if (VarIsTmp(var)) G_TEMP_CNT--;
	}
}

Var * ParseFile()
{
	Var * item = NULL;
	Bool block = false;

	if (TOK == TOKEN_OPEN_P) {
		EnterBlock();
		block = true;
	}

	if (TOK == TOKEN_STRING) {
		item = VarNewStr(StrAlloc(LEX.name));
	} else {
		SyntaxError("expected string specifying file name");
	}

	if (block) {
		ExpectToken(TOKEN_BLOCK_END);
	} else {
		NextToken();
	}
	return item;
}

UInt32 ParseArrayConst(Var * var)
/*
Purpose:
	Parse array constant.
Arguments:
	var		Array variable for which the constant is parsed.
*/
{
	UInt32 i, rep;
	Var * item;
	Type * item_type;

	InstrBlockPush();
	i = 0;

	item_type = var->type->element;

	EnterBlock();

	while(!NextIs(TOKEN_BLOCK_END)) {

		// Skip any EOLs (we may use them to separate subarrays?)
		if (NextIs(TOKEN_EOL)) continue;

		// Items may be separated by comma too (though it is optional)
		if (i > 0) {
			if (NextIs(TOKEN_COMMA)) {
				// Skip any EOLs after comma
				while (NextIs(TOKEN_EOL));
			}
		}


		// FILE "filename"

		if (NextIs(TOKEN_FILE)) {
			item = ParseFile();
			if (TOK) {
				Gen(INSTR_FILE, NULL, item, NULL);
				continue;
			} else {
				break;
			}
		}

		//TODO: Here can be either the type or integer constant or address
		ParseExpressionType(item_type);
		item = STACK[0];

		rep = 1;
		if (NextIs(TOKEN_TIMES)) {

			if (item->type->variant == TYPE_INT) {
				rep = item->n;
			} else {
				SyntaxError("repeat must be defined using integer");
				break;
			}
			ParseExpressionType(item_type);
			item = STACK[0];
		}

		while(rep--) {
			// Generate reference to variable
			if (item->type->variant == TYPE_ARRAY) {
				Gen(INSTR_PTR, NULL, item, NULL);
				i += TypeAdrSize();		// address has several bytes
			} else {
				Gen(INSTR_DATA, NULL, item, NULL);
				i++;
			}
		}
	}
	var->instr = InstrBlockPop();
	return i;
}

void ArraySize(Type * type, Var ** p_dim1, Var ** p_dim2)
{
	Type * dim;
	UInt32 size;

	*p_dim1 = NULL;
	*p_dim2 = NULL;
	if (type->variant == TYPE_ARRAY) {
		dim = type->dim[0];
		*p_dim1 = VarNewInt(dim->range.max - dim->range.min + 1);
		dim = type->dim[1];
		if (dim != NULL) {
			*p_dim2 = VarNewInt(dim->range.max - dim->range.min + 1);
		}
	} else if (type->variant == TYPE_STRUCT) {
		size = TypeSize(type);
		*p_dim1 = VarNewInt(size);
	}
}


Bool VarIsImplemented(Var * var)
{
	Rule * rule;
	Instr i;
	TypeVariant v;

	// If the variable has no type, it will not be used in instruction,
	// so it is considered implemented.

	if (var->type == NULL) return true;

	// Type declarations do not need to be implementd
	// (we think of them as being implemented by compiler).

	if (var->mode == MODE_TYPE) return true;

	// Macros and procedures are considered imp

	v = var->type->variant;
	if (v == TYPE_MACRO || v == TYPE_PROC || v == TYPE_LABEL) return true;

	// Register variables are considered implemented.
	if (var->adr != NULL && var->adr->scope == REGSET) return true;


	memset(&i, 0, sizeof(i));
	i.op = INSTR_ALLOC;
	i.result = var;
	ArraySize(var->type, &i.arg1, &i.arg2);
	rule = EmitRule(&i);
	return rule != NULL;
}

void ParseAssign(VarMode mode, VarSubmode submode, Type * to_type)
/*
Purpose:
	Parse variable assignment/declaration.
	Lexer contains name of the first defined variable.
*/
{
	Bool is_assign;
	Bool flexible;
	UInt16 cnt, j, i, stack;
	Var * var,  * item;
	Var * vars[MAX_VARS_COMMA_SEPARATED];
	Type * type = NULL;
	UInt16 bookmark;

	is_assign = false;

	if (TOK != TOKEN_ID) {
		SyntaxError("expected identifier");
		return;
	}

	bookmark = SetBookmark();

	// Comma separated list of identifiers
	cnt = 0;
	do {
		var = NULL;

		// Either find an existing variable or create new one
		if (to_type == NULL) {
			var = VarFind2(LEX.name, 0);
		}
		//TODO: Type with same name already exists
		if (var == NULL) {			
			var = VarAlloc(MODE_UNDEFINED, LEX.name, 0);
			// We need to prevent the variable from finding itself in case it has same name as type from outer scope
			// This is done by assigning it mode MODE_UNDEFINED (search ignores such variables).
			// Real mode is assigned when the variable type is parsed.
			var->submode = submode;
		}
		NextToken();
		ErrArg(var);

		//===== Array index like ARR(x, y)

		if (mode != MODE_CONST && mode != MODE_ARG && mode != MODE_TYPE && !Spaces()) {
			if (TOK == TOKEN_OPEN_P) {
				if (var->mode != MODE_UNDEFINED) {
					var = ParseArrayElement(var, false);
				} else {
					SyntaxErrorBmk("Array variable [A] is not declared", bookmark);
				}
			} else if (TOK == TOKEN_DOT) {
				var = ParseStructElement(var);
			}

		}

		//===== Address
		if (TOK == TOKEN_ADR) {
			// If there are spaces after the @, this is label definition
			if (Spaces()) {
				GenLabel(var);
				NextToken();
				is_assign = true;
			} else {
				NextToken();
				if (TOK == TOKEN_INT) {
					var->adr = VarNewInt(LEX.n);
					NextToken();
				} else if (TOK == TOKEN_ID) {
					var->adr = VarFindScope(REGSET, LEX.name, 0);
					if (var->adr == NULL) {
						SyntaxError("$undefined regset");
					} else {
						NextToken();
					}
				} else {
					SyntaxError("expected integer or register set name");
				}
			}
		}

		vars[cnt] = var;
		cnt++;
		// this is to check if there is not too many exprs
		if (cnt>=MAX_VARS_COMMA_SEPARATED) {
			SyntaxError("too many comma separated identifiers");
		}
	} while (NextIs(TOKEN_COMMA));

	// This is definitelly a type!!!
	// Assignment does not allow type specified.

	if (NextIs(TOKEN_COLON)) {

		is_assign = true;

		// Parsing may create new constants, arguments atc. so we must enter subscope, to assign the
		// type elements to this variable
		EnterSubscope(var);
		bookmark = SetBookmark();
		type = ParseType2(mode);
		ExitScope();
	}

	// Set the parsed type to all new variables (we do this, even if a type was not parsed)

	for(j = 0; j<cnt; j++) {
		var = vars[j];
		if (var->mode == MODE_UNDEFINED) {
			var->mode = mode;

			if (type != NULL) {
				var->type = type;

				// Definition of named constant assigned to type (name:xxx = 34)
				if (var->mode == MODE_CONST) {
					if (var->type != NULL) {
						TypeAddConst(var->type, var);
					}
				} else {
					if (!VarIsImplemented(var)) {
						LogicError("Type not supported by platform", bookmark);
					}
				}
			}
		}
	}

	// If there is assigment part, generate instruction
	// (it may be pruned later, when it is decided the variable is not used, or is constant)

	if (NextIs(TOKEN_EQUAL)) {

		is_assign = true;
		stack = 0; TOP = 0;

		for(j = 0; j<cnt; j++) {
			var = vars[j];
			type = var->type;

			// Procedure or macro is defined using parsing code
			if (type != NULL && (type->variant == TYPE_PROC || type->variant == TYPE_MACRO)) {
				EnterSubscope(var);
				InstrBlockPush();
				ParseBlock();
				var->instr = InstrBlockPop();
				ExitScope();
			} else {

				// Initialization of array
				// Array is initialized as list of constants.

				if (type != NULL && type->variant == TYPE_ARRAY && var->mode == MODE_CONST) {
					flexible = type->dim[0]->range.flexible;
					i = ParseArrayConst(var);
					if (flexible) {
						type->dim[0]->range.max = i-1;
					}

				// Normal assignment
				} else {

					if (TOK == TOKEN_STRING) {
						// We may assign strings to array references
						if (var->mode == MODE_ELEMENT || var->mode == MODE_VAR) {

							//TODO: Read the variable from definition of format procedure
							item = VarFind("_arr", 0);

							// Call format routine (set adrees argument)
							Gen(INSTR_LET_ADR, item, var, NULL);
							Gen(INSTR_FORMAT, NULL, NULL, NULL);
							ParseString(STR_NO_EOL);
						} else {
							SyntaxError("string may be assigned only to variable");
						}
					} else {

						if (j == 0 || NextIs(TOKEN_COMMA)) {
							bookmark = SetBookmark();
							ExpectExpression(var);
						}

						if (TOK) {

							// Expression may return multiple values, use them
							for(stack = 0; stack < TOP; stack++) {

								if (stack != 0) {
									j++;
									if (j < cnt) {
										var = vars[j];
										type = var->type;
									} else {
										SyntaxError("unused return value");
										break;
									}
								}

								item = STACK[stack];

								if (var->mode == MODE_CONST) {
									var->n = item->n;
									var->value_nonempty = item->value_nonempty;
									// Set the type based on the constant
									// TODO: Maybe we should create IntType(n,n) - i.e. variable with only one possible value?
									if (var->type == NULL) {
										var->type = TypeAllocInt(item->n, item->n);
									}
								} else {
									// Resulting variable is temporary.
									// There must be instruction generated by expression parser, which assigns result
									// of the expression to this variable.
									// It must be last instruction generated.
									// We just replace the result in this instruction with result of assign.
									// This eliminates unnecessary temporary variable usage.

									// The variable does not have a type, therefore we must use inferencing

									if (var->type == NULL) {

										// Variable is initialized by constant, set the type to n..n
										if (item->mode == MODE_CONST) {
											if (item->type->variant == TYPE_INT) {
												var->type = TypeAllocInt(item->n, item->n);
											} else {
												SyntaxError("unexpected type for assignment");
											}
										} else {

											if (VarIsTmp(item)) {
												type = &EXP_TYPE;
											} else {
												type = item->type;
											}

											var->type = TypeCopy(type);
										}
										var->type->flexible = true;
									} else {
										// Variables with flexible type will have their type expanded.
										if (var->type->flexible) {
											TypeLet(var->type, item);
										} else {
											if (!VarMatchType(item, var->type)) {
												LogicWarning("value does not fit into variable", bookmark);
											}
										}
									}

									if (VarIsTmp(item)) {
										GenLastResult(var);
									} else {
										Gen(INSTR_LET, var, item, NULL);
									}
								}
								VarFree(item);
							}
						}
					}
				}
			}
		}
	} // idx


	if (TOK != TOKEN_ERROR && !is_assign && mode != MODE_ARG) {
		SyntaxError("expects : or =");
	}
}

Var * ParseInstrArg3()
{
	Var * var = NULL;
	ParseExpression(NULL);
	if (TOK != TOKEN_ERROR) {
		var = STACK[0];
		if (VarIsTmp(var)) G_TEMP_CNT--;
	}
	return var;

}

InstrOp ParseInstrOp()
/*
Purpose:
	Parse instrunction operator name.
*/
{
	Var * inop;
	InstrOp op = INSTR_VOID;

	if (TOK == TOKEN_ID || TOK >= TOKEN_KEYWORD) {
		inop = InstrFind(LEX.name);
		if (inop != NULL) {
			op = inop->n;
			NextToken();
		} else {
			SyntaxError("$Unknown instruction");
		}
	} else {
		SyntaxError("Expected instruction name");
	}

	return op;
}


void ParseInstr()
/*
Syntax: <instr_name> <result> <arg1> <arg2>
*/
{
	Var * arg[3];
	UInt8 n, arg_no;
	InstrOp op;
	Var * label;
	char inc_path[MAX_PATH_LEN];

	op = ParseInstrOp();
	if (TOK != TOKEN_ERROR) {
		n = 0;
	// Include has special handling
	// We need to make the file relative to current file dir and check the existence of the file

		if (op == INSTR_INCLUDE) {

			if (TOK == TOKEN_STRING) {
				PathMerge(inc_path, FILE_DIR, LEX.name);
				arg[n++] = VarNewStr(inc_path);
				NextToken();
			} else {
				SyntaxError("expectd name of include file");
			}

		// Branching instruction has label as first argument
		// 
		} else if (IS_INSTR_JUMP(op) || op == INSTR_LABEL || op == INSTR_CALL) {
			if (TOK == TOKEN_ID) {
				label = VarFind2(LEX.name, 0);
				if (label == NULL) {
					label = VarNewLabel(LEX.name);
				}
				NextToken();
				NextIs(TOKEN_COMMA);
				arg[0] = label;
				n++;
			} else if (arg_no = ParseArgNo()) {
				arg[0] = VarMacroArg(arg_no-1);
				NextIs(TOKEN_COMMA);
				n++;
			} else {
				SyntaxError("expected label identifier");
			}
		}

		while(n<3 && TOK != TOKEN_ERROR) {
			arg[n++] = ParseInstrArg3();
			if (!NextIs(TOKEN_COMMA)) break;
		}

		while(n<3) arg[n++] = NULL;

		if (TOK != TOKEN_ERROR) {
			Gen(op, arg[0], arg[1], arg[2]);
		}
	}	
}

void ParseInstr2()
{	
	EnterBlock(TOKEN_VOID);
	while(TOK != TOKEN_ERROR && !NextIs(TOKEN_BLOCK_END)) {
		ParseInstr();
		NextIs(TOKEN_EOL);
	};
}

void ParseRuleArg2(RuleArg * arg)
{
	Var * var = NULL;
	RuleArg * idx;

//	if (LEX.line_no == 213) {
//		arg_no = 0;
//	}

	if (TOK == TOKEN_ID) {
		arg->variant = RULE_REGISTER;
		ParseVariable(&arg->var);
		return;
	} else if (arg->arg_no = ParseArgNo()) {
		arg->variant = RULE_ARG;
	} else if (NextIs(TOKEN_ADR)) {
		arg->variant = RULE_DEREF;
		arg->arg_no  = ParseArgNo();
	} else if (NextIs(TOKEN_CONST)) {
		arg->variant = RULE_CONST;
		arg->arg_no  = ParseArgNo();
	} else if (TOK == TOKEN_INT) {
		arg->variant = RULE_VALUE;
		arg->var  = VarNewInt(LEX.n);
		NextToken();
		return;
	}

	// Parese type after the argument (if present)
	if (NextIs(TOKEN_COLON)) {
		if (arg->variant == RULE_ANY) arg->variant = RULE_VARIABLE;
		arg->type =	ParseType();
	}

	if (NextIs(TOKEN_OPEN_P)) {
		idx = arg;
		do {
			idx->index = MemAllocStruct(RuleArg);
			ParseRuleArg2(idx->index);
			idx = idx->index;
		} while(NextIs(TOKEN_COMMA));

		if (TOK != TOKEN_ERROR && !NextIs(TOKEN_CLOSE_P)) {
			SyntaxError("expected closing brace");
		}
	}
}

void ParseRule()
/*
<instr> "=" ["#" <instr>]+  | "emit"+
*/
{
	InstrOp op;	
	UInt8 i;
	Rule * rule;
//	Var * scope;

	op = ParseInstrOp();
	if (TOK == TOKEN_ERROR) return;

	rule = MemAllocStruct(Rule);
	rule->op = op;
	rule->line_no = LINE_NO;
	rule->file    = SRC_FILE;

	// Parse three parameters

	for(i=0; i<3 && TOK != TOKEN_EQUAL && TOK != TOKEN_ERROR; i++) {
		ParseRuleArg2(&rule->arg[i]);
		NextIs(TOKEN_COMMA);
	}

	// TODO: Rule should use parse block to parse code, TOKEN_INSTR should be part of parse code

	if (NextIs(TOKEN_EQUAL)) {

//		scope = SCOPE;
//		SCOPE = RULE_SCOPE;
//		EnterLocalScope();

		if (NextIs(TOKEN_INSTR)) {
			InstrBlockPush();
			ParseInstr2(&rule->to);
			rule->to = InstrBlockPop();	
		} else {

			// Emitting rule
			if (TOK == TOKEN_STRING) {
				InstrBlockPush();
				do {
					Gen(INSTR_EMIT, NULL, VarNewStr(LEX.name), NULL);
					NextToken();
				} while (TOK == TOKEN_STRING);
				rule->to = InstrBlockPop();
			} else {
				SyntaxError("Expected instruction or string");
			}
		}
//		ExitScope();
//		SCOPE = scope;
	}

	if (TOK != TOKEN_ERROR) {
		RuleRegister(rule);
	}
	
}

void ParseString(UInt32 flags)
/*
Purpose:
	Parse string constant.
	String may contain variables enclosed in square braces.
*/
{
	char * start, * end, c;
	Var * var, * var2;
	Bool in_var;
	Bool no_eol;
//	Bool eol;

	do {
		no_eol = false;
		in_var = false;
		start = end = LEX.name;
		do {
			c = *end;
			if (c == L'[' || c == 0) {
				if (end != start) {
					*end = 0;
					var = VarNewStr(start);
					var2 = VarNewInt(StrLen(start));
					Gen(INSTR_STR_ARG, NULL, var, var2);
				}

				if (c == 0) break;
				// Find ending square bracket
				end++;
				start = end;
				while ((c = *end) && c != ']') end++;
				*end = 0;
				var = VarFind2(start, 0);
				if (var != NULL) {
					Gen(INSTR_VAR_ARG, NULL, var, NULL);
				} else {
					SyntaxError("Variable not found");
				}
				start = end+1;
			}
			end++;
		} while (TOK != TOKEN_ERROR);

		if (TOK != TOKEN_ERROR) {
			NextToken();
		}

//		eol = false;

		if (FlagOn(flags, STR_NO_EOL)) {
			no_eol = true;
		} else if (TOK == TOKEN_COMMA) {
			no_eol = true;
			NextToken();
		}

		// If not instructed otherwise, generate EOL
		if (!no_eol) {
			var2 = VarNewInt(128);
			Gen(INSTR_DATA, NULL, var2, NULL);
		}

		} while (TOK == TOKEN_STRING);


	// Generate ending 0 byte
	var2 = VarNewInt(0);
	Gen(INSTR_DATA, NULL, var2, NULL);

//	if (TOK != TOKEN_ERROR) {
//		NextToken();
//	}
}

Var * PopTop()
{
	Var * var;
	TOP--;
	var = STACK[TOP];
	if (VarIsTmp(var)) G_TEMP_CNT--;
	return var;
}

void ParseCall(Var * proc)
{
	Var * arg;
	arg = FirstArg(proc, SUBMODE_ARG_IN);
	if (arg != NULL) {
		EnterBlock();
		while(TOK != TOKEN_ERROR && !NextIs(TOKEN_BLOCK_END)) {
			if (arg == NULL) {
				ExitBlock();
				break;
			}
			ParseExpression(arg);
			Gen(INSTR_LET, arg, PopTop(), NULL);
//			if (VarIsTmp(STACK[0])) G_TEMP_CNT--;
//			TOP = 0;
			arg = NextArg(proc, arg, SUBMODE_ARG_IN);
		}
		if (arg != NULL) {
			SyntaxError("Missing argument in procedure call");
		}
	}
	Gen(INSTR_CALL, proc, NULL, NULL);
}

void ParseMacro(Var * macro)
{
	Var * arg;
	Var * args[32];
	UInt8 i;
	arg = FirstArg(macro, SUBMODE_ARG_IN);
	EnterBlock();
	i = 0;
	while(TOK != TOKEN_ERROR && !NextIs(TOKEN_BLOCK_END)) {
		if (arg == NULL) {
			ExitBlock();
			break;
		}
		ParseExpression(arg);
		//TODO: Use all the results from expression parsing
		args[i] = STACK[0];
		if (VarIsTmp(STACK[0])) G_TEMP_CNT--;
		arg = NextArg(macro, arg, SUBMODE_ARG_IN);
		i++;
	}
	if (arg != NULL) {
		SyntaxError("Missing argument in macro call");
	}
	GenMacro(macro->instr, macro, args);
}

void ParseId()
{
	Var * var;		// may be global?

	var = VarFind2(LEX.name, 0);
	if (var != NULL) {
		if (var->type != NULL) {
			switch(var->type->variant) {
				case TYPE_PROC:
					NextToken();
					ParseCall(var);
					return;
				case TYPE_MACRO:
					NextToken();
					ParseMacro(var);
					return;
				default: break;
			}
		}
	}
	ParseAssign(MODE_VAR, SUBMODE_EMPTY, NULL);
}

void ParseDeclarations(VarMode mode, VarSubmode submode)
/*
Purpose:
	Parse list of declarations of variables of specified mode and submode.
Syntax:
	Decl: { [<assign>]* }
*/
{
	NextToken();
	EnterBlock();		
	while (TOK != TOKEN_ERROR && !NextIs(TOKEN_BLOCK_END)) {
		ParseAssign(mode, submode, NULL);
		while(NextIs(TOKEN_EOL));
	}
}

void ParseUseFile()
{
	if (TOK != TOKEN_ID && TOK != TOKEN_STRING) {
		SyntaxError("Expected module name");
		return;
	}

	Parse(LEX.name, false);
	NextToken();
}

void ParseUse()
/*
Syntax: { [file_ref] }
*/
{
	NextToken();		// skip TOKEN_USE
	EnterBlock();
	while (TOK != TOKEN_ERROR && !NextIs(TOKEN_BLOCK_END)) {
		ParseUseFile();
		NextIs(TOKEN_COMMA);
		while(NextIs(TOKEN_EOL));
	}
}

void ParseRef()
{
}

void ParseCommands()
{

	while (TOK != TOKEN_BLOCK_END && TOK != TOKEN_EOF && TOK != TOKEN_ERROR && TOK != TOKEN_OUTDENT) {

		switch(TOK) {
		case TOKEN_CONST: 
			ParseDeclarations(MODE_CONST, SUBMODE_EMPTY); break;
		case TOKEN_TYPE2:  
			ParseDeclarations(MODE_TYPE, SUBMODE_EMPTY); break;
		case TOKEN_IN:    
			ParseDeclarations(MODE_VAR, SUBMODE_IN); break;
		case TOKEN_OUT:   
			ParseDeclarations(MODE_VAR, SUBMODE_OUT);	break;

		case TOKEN_USE:
			ParseUse();
			break;

		case TOKEN_INSTR:
			NextToken();
			ParseInstr2();
			break;

		case TOKEN_STRING: 
			Gen(INSTR_PRINT, NULL, NULL, NULL);
			ParseString(0); 
			break;
		case TOKEN_ID: 
			ParseId(); break;
		case TOKEN_RULE: 
			NextToken(); 
			ParseRule(); break;
		case TOKEN_GOTO: 
			ParseGoto(); break;
		case TOKEN_IF:   
			ParseIf(); break;
		case TOKEN_WHILE:
		case TOKEN_UNTIL: 
			ParseFor(); break;
		case TOKEN_FOR: 
			ParseFor(); break;
		case TOKEN_DEBUG: 
			NextToken(); 
			Gen(INSTR_DEBUG, NULL, NULL, NULL); break;
		case TOKEN_EOL:
			NextToken(); 
//			if (G_DEPTH > 0) return;
			break;
		default:         
			SyntaxError("unexpected token");
		}
	}
}

extern UInt8      BLK_TOP;

Bool Parse(char * name, Bool main_file)
{
	G_TEMP_CNT = 1;
	if (SrcOpen(name)) {
		if (main_file) {
			SRC_FILE->submode = SUBMODE_MAIN_FILE;
		}
		ParseCommands();
		if (TOK != TOKEN_ERROR) {
			if (TOK == TOKEN_BLOCK_END) {
//				if (BLK_TOP > 1) {
//					SyntaxError("Unended blocks at the end of file");
//				}
			} else {
				SyntaxError("Unexpected end of file");
			}
		}
		SrcClose();
	}

	return ERROR_CNT == 0;
}

void ParseInit()
{
	MemEmptyVar(G_BLOCKS);
	G_BLOCK = &G_BLOCKS[0];
	G_BLOCK->command = TOKEN_PROC;
	G_CONDITION_EXP = 0;
	SYSTEM_PARSE = true;
}
