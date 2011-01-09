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
void ExpectExpression(Var * result);
Var * PopTop();
void ParseCall(Var * proc);
void ParseMacro(Var * macro);
Type * ParseType();
Var * ParseArrayElement(Var * arr);
Var * ParseStructElement(Var * arr);
Var * ParseFile();

// This variable is set to true, when we parse expression inside condition.
// It modifies parsing behaviour concernign and, or and not.

//UInt8 G_CONDITION_EXP;

#define STR_NO_EOL 1
void ParseString(UInt32 flags);
void ParseExpressionType(Type * result_type);

// All rules share one common scope, so they do not mix with normal scope of program.

Var * RULE_SCOPE;

extern Var * LAST_VAR;

void ParseVariable(Var ** p_var)
{
	Bool spaces;
	Var * var = NULL, * scope;
	do {
		scope = var;
		if (scope != NULL) {
			var = VarFindScope(scope, LEX.name, 0);
		} else {
			var = VarFind2(LEX.name, 0);
		}
		spaces = Spaces();
		if (var == NULL) {
			SyntaxError("Unknown variable");
		} else {
			NextToken();
		}
	} while(!spaces && NextIs(TOKEN_DOT));

	*p_var = var;
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
			if (var->mode == MODE_CONST) {
				if (var->type->variant == TYPE_INT) {
					type = TypeAlloc(TYPE_INT);
					type->range.min = 0;
					type->range.max = var->n;
				} else {
					SyntaxError("Expected integer constant");
				}
			} else {
				type = var->type;
			}
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
	Type * type = NULL, * variant_type = NULL;
	long last_n = 0;
	long i;
	Type * elmt;
	Bool id_required;

next:
	if (NextIs(TOKEN_ENUM)) {
		type = TypeAlloc(TYPE_INT);
		type->range.flexible = true;
		type->is_enum        = true;
		if (TOK == TOKEN_INT) goto range;
		goto const_list;

	} else if (TOK == TOKEN_INT || TOK == TOKEN_MINUS) {
		type = TypeAlloc(TYPE_INT);
range:
		ExpectExpression(NULL);
		if (TOK) {
			var = PopTop();
			if (var->mode == MODE_CONST) {
				type->range.min = var->n;
			} else {
				SyntaxError("expected constant expression");
			}
		}

//		type->range.min = LEX.n;
//		NextToken();
		if (TOK == TOKEN_DOTDOT) {
			NextToken();
			ExpectExpression(NULL);
			if (TOK) {
				var = PopTop();
				if (var->mode == MODE_CONST) {
					type->range.max = var->n;
				} else {
					SyntaxError("expected constant expression");
				}
			}
		} else {
			type->range.max = type->range.min;
			type->range.min = 0;
		}

		if (type->range.min > type->range.max) {
			SyntaxError("range minimum bigger than maximum");
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
				// For integer type, constants may be defined
				if (type->variant == TYPE_INT) goto const_list;
			} else {
				type = var->type;
			}
		}
	}

	if (variant_type != NULL) {
		variant_type->dim[1] = type;
		type = variant_type;
	}

	if (NextIs(TOKEN_OR)) {
		variant_type = TypeAlloc(TYPE_VARIANT);
		variant_type->dim[0] = type;
		goto next;
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
			case INSTR_SQRT: r = (UInt32)sqrt(n1); break;
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

	if (arr->mode == MODE_ELEMENT && arr->adr->mode == MODE_SCOPE) {
		NextIs(TOKEN_DOT);
		if (TOK == TOKEN_ID) {
			item = VarFindScope(arr->adr, LEX.name, 0);
			if (item != NULL) {
				if (item->type->variant == TYPE_ARRAY) {
					idx = VarNewElement(item, arr->var);
//					idx->type = idx->type->element;
				} else {
					SyntaxError("$Variable is not an array.");
				}
			} else {
				SyntaxError("$Scope does not contain member with name");
			}
			NextToken();
		}

	} else if (arr->type->variant == TYPE_STRUCT) {
		NextIs(TOKEN_DOT);
		if (TOK == TOKEN_ID) {
			item = VarFindScope(arr->type->owner, LEX.name, 0);
			if (item != NULL) {
				idx = VarNewElement(arr, item);
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

Var * ParseArrayElement(Var * arr)
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
//	Bool prev_cond;
	VarMode mode = MODE_ELEMENT;

	// Array element access uses always () block
	EnterBlock();

//	NextIs(TOKEN_OPEN_P);

	top = TOP;

	// First dimension (or first element of tuple)

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
//	prev_cond = G_CONDITION_EXP;
//	G_CONDITION_EXP = false;

	// It may be (..<n>), use min as default
	if (TOK == TOKEN_DOTDOT) {
		mode = MODE_RANGE;
		idx  = VarNewInt(idx_type->range.min);
		
	} else {

		ParseSubExpression(idx_type);
		if (TOK) {
			idx = STACK[top];
			CheckArrayBound(0, arr, idx_type, idx, bookmark);
		}
	}

	TOP = top;
	bookmark = SetBookmark();

	// <min>..<max>
	if (NextIs(TOKEN_DOTDOT)) {
		mode = MODE_RANGE;
		if (TOK == TOKEN_COMMA || TOK == TOKEN_BLOCK_END) {
			idx2 = VarNewInt(idx_type->range.max);
		} else {
			ParseSubExpression(idx_type);
			idx2 = STACK[top];
			if (TOK) {
				CheckArrayBound(1, arr, idx_type, idx2, bookmark);
			}
		}
		idx = VarNewRange(idx, idx2);
	}

	if (TOK) {
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

				idx = VarNewElement(idx, idx2);
			} else {
				SyntaxError("Array has only one dimension");
			}
		}

		if (!NextIs(TOKEN_BLOCK_END)) SyntaxError("missing closing ')'");
	}
	//G_CONDITION_EXP = prev_cond;

	//ParseParenthesis();
	// Result is temporary variable created by InstrBinary, 
	// arg1 = array, arg2 = item

	item = VarNewElement(arr, idx);


	TOP = top;

	return item;
}

void ParseOperand()
{
	Var * var = NULL, * item = NULL, * proc;
	Bool ref = false;
	Bool type_match;
	UInt8 arg_no;
	Bool spaces;
	Type * type;

	if (TOK == TOKEN_OPEN_P) {
		ParseParenthesis();
	} else {
		// file "slssl"
		if (TOK == TOKEN_FILE) {

			NextToken();

			// This will be constant variable with temporary name, array of bytes
			item = ParseFile();
			if (TOK) {
				type = RESULT_TYPE;
				if (type == NULL) {
					type = TypeAlloc(TYPE_ARRAY);
				}
				var = VarNewTmp(0, type);
				var->mode = MODE_CONST;

				InstrBlockPush();
				Gen(INSTR_FILE, NULL, item, NULL);
				var->instr = InstrBlockPop();
			}
		// @id denotes reference to variable
		} else if (TOK == TOKEN_ADR) {
			NextToken();
			var = VarFind2(LEX.name, 0);
			if (var != NULL) {
				var = VarNewDeref(var);
			} else {
				SyntaxError("$unknown variable");
			}
			goto no_id;
		} else if (arg_no = ParseArgNo2()) {
			var = VarMacroArg(arg_no-1);
			goto indices;

		} else if (TOK == TOKEN_INT) {
			var = VarNewInt(LEX.n);
			NextToken();
		} else if (TOK == TOKEN_ID) {

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
no_id:
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
			if (var->type != NULL && (var->type->variant == TYPE_PROC || var->type->variant == TYPE_MACRO)) {
				proc = var;
				NextToken();
				if (var->type->variant == TYPE_PROC) {
					ParseCall(proc);
				} else {
					ParseMacro(proc);
				}

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

					item = ParseArrayElement(var);

//					if (VarIsTmp(STACK[TOP-1])) G_TEMP_CNT--;
					STACK[TOP] = item;
					TOP++;
					return;
				}
			}// else {
			//	NextToken();
			//}
		} else {
			SyntaxError("expected operand");
			return;
		}
		STACK[TOP] = var;
		TOP++;
	}
}

void ParseUnary()
{
	if (NextIs(TOKEN_MINUS)) {
		// Unary minus before X is interpreted as 0 - X
		STACK[TOP++] = VarNewInt(0);
		ParseOperand();
		InstrBinary(INSTR_SUB);
	} else if (NextIs(TOKEN_HI)) {
		ParseOperand();
		InstrUnary(INSTR_HI);
	} else if (NextIs(TOKEN_LO)) {
		ParseOperand();
		InstrUnary(INSTR_LO);
	}  else if (NextIs(TOKEN_BITNOT)) {
		ParseOperand();
		InstrUnary(INSTR_NOT);
	} else if (NextIs(TOKEN_SQRT)) {
		ParseOperand();
		InstrUnary(INSTR_SQRT);
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
	Var * var;
	ParsePlusMinus();
retry:
	if (TOK == TOKEN_BITAND) {
		var = STACK[TOP];
//		if (!G_CONDITION_EXP || !TypeIsBool(var->type)) {
			NextToken();
			ParsePlusMinus();
			if (TOK) {
				InstrBinary(INSTR_AND);
			}
			goto retry;
//		}
	}
}

void ParseBinaryOr()
{
	ParseBinaryAnd();
retry:
	if (NextIs(TOKEN_BITOR)) {
		ParseBinaryAnd();
		if (TOK) {
			InstrBinary(INSTR_OR);
		}
		goto retry;
	} else if (NextIs(TOKEN_BITXOR)) {
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
	Type * type;

	if (result == NULL) {
		RESULT_TYPE = NULL;
	} else {
		if (result->mode == MODE_ELEMENT) {
			type = result->adr->type;
			if (type->variant == TYPE_ARRAY) {
				RESULT_TYPE = type->element;
			} else if (type->variant == TYPE_ADR) {

			} else {
			}
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

void GenRel(InstrOp op, Var * left, Var * right)
{
	if (!G_BLOCK->not) op = OpNot(op);

	if (G_BLOCK->f_label == NULL) {
		G_BLOCK->f_label = VarNewTmpLabel();
	}
	Gen(op, G_BLOCK->f_label, left, right);
}

Var * VarFindAssociatedConst(Var * var, char * name)
{
	if (var == NULL) return NULL;
	return VarFindScope(var->type->owner, name, 0);
}

void ParseRel()
/*
	relop: "=" | "<>" | "<" | "<=" | ">" | ">="
	rel:  <exp> <relop> <exp> [<relop> <exp>]*
*/
{
	Var * left, * right;
	InstrOp op;

	if (TOK == TOKEN_OPEN_P) {
		ParseCondParenthesis();
	} else {
		ParseExpression(NULL);
		left = STACK[0];

		op = RelInstrFromToken();

		// No relation operator follows the expression, this must be test of boolean variable
		if (op == INSTR_VOID) {

			right = VarFindAssociatedConst(left, "true");
			if (right != NULL) {
				op = INSTR_IFEQ;
			} else {
				right = VarFindAssociatedConst(left, "false");
				if (right != NULL) op = INSTR_IFNE;
			}
			if (op != INSTR_VOID) {
				GenRel(op, left, right);
			} else {
				SyntaxError("variable is not of boolean type");
			}

		// var <relop> var [<relop> var]
		} else {
			do {

				// For normal operation, we jump to false label when the condition does NOT apply
				// For example for if:
				// if <cond>
				//     <block>
				//
				// must skip the <block>.			

				NextToken();
				ParseExpression(left);
				right = STACK[0];
				if (TOK != TOKEN_ERROR) {
					GenRel(op, left, right);
					left = right;
				}
				op = RelInstrFromToken();
			} while (op != INSTR_VOID);
		}
	}
}

void ParseNot()
{

	Bool not = false;
	while (NextIs(TOKEN_NOT)) not = !not;

	if (not) {
		not = G_BLOCK->not;
		G_BLOCK->not = !not;
		ParseRel();
		G_BLOCK->not = not;
	} else {
		ParseRel();
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
	Token tok;

retry:
	ParseNot();

	tok = TOKEN_AND;
	if (G_BLOCK->not) tok = TOKEN_OR;

	if ((!G_BLOCK->not && NextIs(TOKEN_AND)) || (G_BLOCK->not && NextIs(TOKEN_OR))) {
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
	Token tok;
retry:
	ParseAnd();
	// If the condition is negated (either using NOT or UNTIL), meaning of AND and OR is switched

	tok = TOKEN_OR;
	if (G_BLOCK->not) tok = TOKEN_AND;

	if (NextIs(tok)) {

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
// Labels are global in procedure

	Var * var = NULL;

	ExpectToken(TOKEN_ID);
	if (TOK == TOKEN_ID) {
		var = FindOrAllocLabel(LEX.name, 0);
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
			var = VarNewDeref(var);				//TODO: MODE_DEREF
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
	Var * min, * max, * step;
	Type * type;
	InstrBlock * cond, * where_cond, * body;
	Int32 n, nmask;

	var = NULL; min = NULL; max = NULL; cond = NULL; where_cond = NULL; step = NULL;
	where_t_label = NULL;

	EnterLocalScope();

	if (NextIs(TOKEN_FOR)) {

		GenLine();

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
	
	// STEP can be only used if there was FOR

	if (var != NULL) {
		if (NextIs(TOKEN_STEP)) {
			ParseExpression(max);
			step = STACK[0];
		}
	}

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
		InternalGen(INSTR_LET, var, min, NULL);
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

		// Default step is 1
		if (step == NULL) step = VarNewInt(1);
		
		// Add the step to variable
		InternalGen(INSTR_ADD, var, var, step);

		// 1. If max equals to byte limit (0xff, 0xffff, 0xffffff, ...), only overflow test is enough
		//    We must constant adding by one, as that would be translated to increment, which is not guaranteed
		//    to set overflow flag.

		if (max->mode == MODE_CONST) {
			n = max->n;
			nmask = 0xff;
			while(n > nmask) nmask = (nmask << 8) | 0xff;

			if (n == nmask && (step->mode != MODE_CONST || step->n > 255)) {
				InternalGen(INSTR_IFNOVERFLOW, G_BLOCK->body_label, NULL, NULL);
				goto var_done;
			} else if (step->mode == MODE_CONST) {

				// 2. Min,max,step are constants, in such case we may use IFNE and calculate correct stop value
				if (min->mode == MODE_CONST) {
					n = min->n + ((max->n - min->n) / step->n + 1) * step->n;
					n = n & nmask;
					max = VarNewInt(n);
					InternalGen(INSTR_IFNE, G_BLOCK->body_label, var, max);	//TODO: Overflow
					goto var_done;
				// 3. max & step are constant, we may detect, that overflow will not occur
				} else {
					if ((nmask - max->n) >= step->n) goto no_overflow;
				}
			}
		}

		// Alloc f_label if necessary
		if (G_BLOCK->f_label == NULL) {
			G_BLOCK->f_label = VarNewTmpLabel();
		}
		InternalGen(INSTR_IFOVERFLOW, G_BLOCK->f_label, NULL, NULL);
no_overflow:
		InternalGen(INSTR_IFLE, G_BLOCK->body_label, var, max);
	}
var_done:

	if (G_BLOCK->f_label != NULL) {
		GenLabel(G_BLOCK->f_label);
	}
done:
	EndBlock();
	VarFree(min);
	VarFree(max);
	ExitScope();
}

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

	if (TOK) {
		if (block) {
			ExpectToken(TOKEN_BLOCK_END);
		} else {
			NextToken();
		}
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

		// Array of array
		} else {
			if (type->element != NULL && type->element->variant == TYPE_ARRAY) {
				dim = type->element->dim[0];
				*p_dim2 = VarNewInt(dim->range.max - dim->range.min + 1);
			}
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
	if (v == TYPE_MACRO || v == TYPE_PROC || v == TYPE_LABEL || v == TYPE_SCOPE) return true;

	// Register variables are considered implemented.
	if (var->adr != NULL && var->adr->scope == REGSET) return true;


	memset(&i, 0, sizeof(i));
	i.op = INSTR_ALLOC;
	i.result = var;
	ArraySize(var->type, &i.arg1, &i.arg2);
	rule = EmitRule(&i);
	return rule != NULL;
}

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

	if (arr->mode == MODE_ELEMENT) {

		// If this is array of array, we may need to initialize index variable differently
		
		if (arr->adr->mode == MODE_ELEMENT) {
			idx1_type = arr->adr->type->dim[0];
		}

		range = arr->var;
		if (range->mode == MODE_RANGE) {
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

	if (max1->mode == MODE_CONST) {
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

	Gen(INSTR_LET, idx, min1, NULL);
	if (src_idx != NULL) {
		Gen(INSTR_LET, src_idx, src_min, NULL);
	}
	GenLabel(label);
	Gen(INSTR_LET, VarNewElement(dst_arr, idx), init, NULL);
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

void ParseAssign(VarMode mode, VarSubmode submode, Type * to_type)
/*
Purpose:
	Parse variable assignment/declaration.
	Lexer contains name of the first defined variable.
*/
{
	Bool is_assign, existed;
	Bool flexible;
	UInt16 cnt, j, i, stack;
	Var * var,  * item, * adr, * tuple, * scope, * idx, * min, * max;
	Var * vars[MAX_VARS_COMMA_SEPARATED];
	Type * type = NULL;
	TypeVariant typev;
	UInt16 bookmark;

	is_assign = false;
	existed   = true;
	scope = NULL;

	// Force use of current scope
	// For example .X will try to find X in current scope, not in any other parent scope
	if (!Spaces() && NextIs(TOKEN_DOT)) {
		scope = SCOPE;
	}

	if (TOK != TOKEN_ID) {
		SyntaxError("expected identifier");
		return;
	}

	bookmark = SetBookmark();

	// Comma separated list of identifiers
	cnt = 0;
	do {
retry:
		var = NULL;

		// Either find an existing variable or create new one
		if (to_type == NULL) {
			if (scope == NULL) {
				var = VarFind2(LEX.name, 0);
			} else {
//				PrintScope(scope);
				var = VarFindScope(scope, LEX.name, 0);
			}
		}
		//TODO: Type with same name already exists
		if (var == NULL) {			
			var = VarAllocScope(scope, MODE_UNDEFINED, LEX.name, 0);
			existed = false;
//			if (scope != NULL) {
//				PrintScope(scope);
//			}

			// We need to prevent the variable from finding itself in case it has same name as type from outer scope
			// This is done by assigning it mode MODE_UNDEFINED (search ignores such variables).
			// Real mode is assigned when the variable type is parsed.
			var->submode = submode;
//			if (scope != NULL) {
//				var->scope = scope;
//			}
		} else {
			if (var->mode == MODE_SCOPE) {
				NextToken();
				scope = var;

				if (NextIs(TOKEN_DOT)) {
					goto retry;
				} else {
					goto no_dot;
//					SyntaxError("Dot expected after scope name");
				}
			}
		}
		NextToken();
	// Parse array and struct indices
no_dot:
		ErrArg(var);

		//===== Array index like ARR(x, y)

		if (mode != MODE_CONST && mode != MODE_ARG && mode != MODE_TYPE && !Spaces()) {
			if (TOK == TOKEN_OPEN_P) {
				if (var->mode != MODE_UNDEFINED) {
					var = ParseArrayElement(var);
				} else {
					SyntaxErrorBmk("Array variable [A] is not declared", bookmark);
				}
				if (TOK) goto no_dot;
			} else if (TOK == TOKEN_DOT) {
				var = ParseStructElement(var);
				if (TOK) goto no_dot;
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

				// (var,var,...)   tuple
				// int (concrete address)
				// variable (some variable)

				adr = NULL; tuple = NULL;
				NextToken();
				//@
				if (TOK == TOKEN_OPEN_P) {
					EnterBlock();
					do {
						ParseVariable(&item);
						if (TOK) {
							if (adr == NULL) {
								adr = item;
							} else {
								if (tuple == NULL) {
									adr = tuple = VarNewTuple(adr, item);
								} else {
									tuple->var = VarNewTuple(tuple->var, item);
								}
							}
						}
					} while(NextIs(TOKEN_COMMA));
					if (!NextIs(TOKEN_BLOCK_END)) {
						SyntaxError("expected closing parenthesis");
					}
				} else if (TOK == TOKEN_INT) {
					adr = VarNewInt(LEX.n);
					NextToken();
				} else if (TOK == TOKEN_ID) {

					adr = VarFindScope(REGSET, LEX.name, 0);
					if (adr == NULL) {
						adr = VarFind2(LEX.name, 0);
						NextToken();
						if (adr == NULL) {
							SyntaxError("$undefined regset or variable used as address");
						} else {
dot:
							if (NextIs(TOKEN_DOT)) {
								if (TOK == TOKEN_ID) {
									adr = VarFindScope(adr, LEX.name, 0);
									NextToken();
									goto dot;
								} else {
									SyntaxError("Expected variable name");
								}
							}

							if (adr->mode == MODE_SCOPE) {
								SyntaxError("scope can not be used as address");
							} 
							// name(slice)
							if (TOK == TOKEN_OPEN_P) {
								adr = ParseArrayElement(adr);
							}
						}
					} else {
						NextToken();
					}
				} else {
					SyntaxError("expected integer or register set name");
				}
				var->adr = adr;
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

		// Scope
		if (NextIs(TOKEN_SCOPE)) {
			mode = MODE_SCOPE;
			type = TypeScope();
			is_assign = true;
		} else {
			is_assign = true;

			// Parsing may create new constants, arguments atc. so we must enter subscope, to assign the
			// type elements to this variable
			scope = SCOPE;
			SCOPE = var;
//			EnterSubscope(var);
			bookmark = SetBookmark();
			type = ParseType2(mode);
//			ExitScope();
			SCOPE = scope;
		}
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
			} else {
				// If type has not been defined, but this is alias, use type of the aliased variable
				adr = var->adr;
				if (adr != NULL) {
					if (adr->mode == MODE_VAR) {
						var->type = adr->type;
					} else if (adr->mode == MODE_TUPLE) {
						is_assign = true;
					} else if (adr->mode == MODE_ELEMENT) {
						is_assign = true;
						var->type = adr->type;
						idx = adr->var;

						// For array ranges, define type as array(0..<range_size>) of <array_element>

						if (idx->mode == MODE_RANGE) {
							min = idx->adr; max = idx->var;
							if (min->mode == MODE_CONST && max->mode == MODE_CONST) {
								var->type = TypeAlloc(TYPE_ARRAY);
								var->type->dim[0] = TypeAllocInt(0, max->n - min->n);
								var->type->element = adr->adr->type->element;
							} else {
								SyntaxError("Address can not use variable slices");
							}
						}

					}
				}
			}
		} else {
			if (type != NULL) {
				ErrArg(var);
				SyntaxError("Variable [A] already defined");
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
			typev = TYPE_UNDEFINED;
			if (type != NULL) typev = type->variant;

			ErrArgClear();
			ErrArg(var);

			if (var->mode == MODE_CONST && existed) {
				SyntaxError("Assigning value to constant [A].");
				continue;
			} else if (var->mode == MODE_TYPE) {
				SyntaxError("Assigning value to type [A].");
				continue;
			} else if (FlagOn(var->submode, SUBMODE_IN) && FlagOff(var->submode, SUBMODE_OUT)) {
				SyntaxError("Assigning value to read only register [A].");
				continue;
			}

			// Procedure or macro is defined using parsing code
			if (typev == TYPE_PROC || typev == TYPE_MACRO) {
				EnterSubscope(var);
				InstrBlockPush();
				ParseBlock();
				item = VarFindScope(SCOPE, "_exit", 32767);
				if (item != NULL) {
					GenLabel(item);
				}
				var->instr = InstrBlockPop();
				ExitScope();
			} else if (typev == TYPE_SCOPE) {
				EnterSubscope(var);
				ParseBlock();
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

							// Call format routine (set adrees argument)
							GenMacro(MACRO_FORMAT->instr, MACRO_FORMAT, &var);
							ParseString(STR_NO_EOL);
						} else if (var->mode == MODE_CONST) {
							VarLetStr(var, LEX.name);
							NextToken();
						} else {
							SyntaxError("string may be assigned only to variable or to constant");
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

									// Array assignment
									if (var->type->variant == TYPE_ARRAY || var->mode == MODE_ELEMENT && var->var->mode == MODE_RANGE || (var->mode == MODE_ELEMENT && item->type->variant == TYPE_ARRAY) ) {
										GenArrayInit(var, item);
									} else {
										if (VarIsTmp(item)) {
											GenLastResult(var);
										} else {
											Gen(INSTR_LET, var, item, NULL);
										}
									}
								}
								VarFree(item);
							}
						}
					}
				}
			}
		}
	} else {
		// No equal
		var = vars[0];
		if (existed && !is_assign && var != NULL && var->type != NULL) {
//			NextToken();
			switch(var->type->variant) {
				case TYPE_PROC:
					ParseCall(var);
					is_assign = true;
					break;
				case TYPE_MACRO:
					ParseMacro(var);
					is_assign = true;
					break;
				default: break;
			}
		}
	}
	ErrArgClear();

	if (TOK != TOKEN_ERROR) {
		if (!is_assign) {
			if (mode != MODE_ARG) {
				SyntaxError("expects : or =");
			}
		}
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
				arg[0] = label;
				n++;
				goto next_arg;	//NextIs(TOKEN_COMMA);
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
next_arg:
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

RuleArg * NewRuleArg()
{
	RuleArg * arg;
	arg = MemAllocStruct(RuleArg);
	return arg;
}

void ParseRuleArg2(RuleArg * arg)
{
	Var * var = NULL;
	RuleArg * idx, * arr,  * idx2;
//	RuleIndexVariant idx_var;

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

	// Parse type after the argument (if present)
	if (NextIs(TOKEN_COLON)) {
		if (arg->variant == RULE_ANY) arg->variant = RULE_VARIABLE;
		arg->type =	ParseType();
	}

	// Parse Range
	if (NextIs(TOKEN_DOTDOT)) {
		arr = NewRuleArg();
		MemMove(arr, arg, sizeof(RuleArg));
		arg->variant = RULE_RANGE;
		arg->arr     = arr;
		arg->arg_no  = 0;

		arg->index = NewRuleArg();
		ParseRuleArg2(arg->index);
		return;
	}

	// Parse array subscripts
	while (NextIs(TOKEN_OPEN_P)) {

		// Current argument will be changed to RULE_ELEMENT, so we must copy it to rule for array
		arr = NewRuleArg();
		MemMove(arr, arg, sizeof(RuleArg));
		arg->variant = RULE_ELEMENT;
		arg->arr     = arr;
		arg->arg_no  = 0;

		// Parse indexes (there can be comma separated list of indexes)
		idx = arg;
		do {
			idx->index = NewRuleArg();
			ParseRuleArg2(idx->index);

			if (!NextIs(TOKEN_COMMA)) break;

			idx2 = NewRuleArg();
			idx2->variant = RULE_TUPLE;
			idx2->arr     = idx->index;
			idx->index    = idx2;
			idx = idx2;

		} while(true);

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
	char buf[255];
	char *s, *d, c;

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

//		EnterBlock(TOKEN_VOID);

		if (NextIs(TOKEN_INSTR)) {
			InstrBlockPush();
			ParseInstr2(&rule->to);
			rule->to = InstrBlockPop();	
		} else {

			// Emitting rule
			if (TOK == TOKEN_STRING) {
				InstrBlockPush();
				do {

					// Rule strings may are preprocessed so, that %/ is replaced by current path.
					s = LEX.name;
					d = buf;
					do {
						c = *s++;
						if (c == '%' && s[0] == '/') {
							strcpy(d, FILE_DIR);
							d += strlen(FILE_DIR);
							s++;
						} else {
							*d++ = c;
						}
					} while (c != 0);

					Gen(INSTR_EMIT, NULL, VarNewStr(buf), NULL);

					NextToken();
				} while (TOK == TOKEN_STRING);
				rule->to = InstrBlockPop();
			} else {
				SyntaxError("Expected instruction or string");
			}
		}
//		NextIs(TOKEN_BLOCK_END);
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

				//TODO: Set the string as input source (enclose in parenthesis) - we may only set the position in line to token
				//TODO: Parse expression
				//TODO: Insert expression before the string (block for insertion should be passed to string parse routine)

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

}

Var * PopTop()
{
	Var * var;
	TOP--;
	var = STACK[TOP];
	if (VarIsTmp(var)) G_TEMP_CNT--;
	return var;
}

void ParseArgs(Var * proc, VarSubmode submode)
{
	Var * arg;
	Var * val;
	arg = FirstArg(proc, submode);
	if (arg != NULL) {
		EnterBlock();
		while(TOK != TOKEN_ERROR && !NextIs(TOKEN_BLOCK_END)) {
			if (arg == NULL) {
				ExitBlock();
				break;
			}
			ParseExpression(arg);

			if (TOP > 0) {
				val = PopTop();
				if (VarIsTmp(val)) {
					GenLastResult(arg);
				} else {
					Gen(INSTR_LET, arg, val, NULL);
				}
			} else {
				if (submode == SUBMODE_ARG_IN) {
					ErrArg(arg);
					ErrArg(proc);
					SyntaxError("Missing argument [A] in call of procedure [B]");

				// Output arguments (in return) do not have to be specified all
				} else {
					break;
				}
			}
			arg = NextArg(proc, arg, submode);
		}
	}
}

void ParseCall(Var * proc)
{
	ParseArgs(proc, SUBMODE_ARG_IN);
	Gen(INSTR_CALL, proc, NULL, NULL);
}

void ParseReturn()
{
	Var * proc;
	Var * label;

	NextToken();
	proc = VarProcScope();
	ParseArgs(proc, SUBMODE_ARG_OUT);

	// Return is implemented as jump to end of procedure
	// Optimizer may later move return insted of jump (if there is no cleanup)

	label = FindOrAllocLabel("_exit", 32767);
	GenGoto(label);
}

void ParseMacro(Var * macro)
{
	Var * arg;
	Var * args[32];
	UInt8 i;
	i = 0;
	arg = FirstArg(macro, SUBMODE_ARG_IN);
	if (arg != NULL) {
		EnterBlock();
		while(TOK != TOKEN_ERROR && !NextIs(TOKEN_BLOCK_END)) {
			if (arg == NULL) {
				ExitBlock();
				break;
			}
			ParseExpression(arg);

			if (TOP > 0) {
				args[i] = STACK[0];
			} else {
				ErrArg(arg);
				ErrArg(macro);
				SyntaxError("Missing argument [A] in call of macro [B]");
			}
			arg = NextArg(macro, arg, SUBMODE_ARG_IN);
			i++;
		}
	}
	GenMacro(macro->instr, macro, args);
}

void ParseId()
{
	Var * var;		// may be global?

	//TODO: Procedures & macros in scope or struct
//	ParseVariable(&var);
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
	VarSubmode submode;

	while (TOK != TOKEN_BLOCK_END && TOK != TOKEN_EOF && TOK != TOKEN_ERROR && TOK != TOKEN_OUTDENT) {

		switch(TOK) {
		case TOKEN_CONST: 
			NextToken();
			ParseDeclarations(MODE_CONST, SUBMODE_EMPTY); break;
		case TOKEN_TYPE2:  
			NextToken();
			ParseDeclarations(MODE_TYPE, SUBMODE_EMPTY); break;
		case TOKEN_IN:
			submode = SUBMODE_IN;
			NextToken();
			if (NextIs(TOKEN_OUT)) {
				submode |= SUBMODE_OUT;
			}
			ParseDeclarations(MODE_VAR, submode); 
			break;
		case TOKEN_OUT:   
			submode = SUBMODE_OUT;
			NextToken();
			ParseDeclarations(MODE_VAR, SUBMODE_OUT);	
			break;

		case TOKEN_USE:
			ParseUse();
			break;

		case TOKEN_RETURN:
			ParseReturn();
			break;

		case TOKEN_INSTR:
			NextToken();
			ParseInstr2();
			break;

		case TOKEN_STRING: 
			GenMacro(MACRO_PRINT->instr, MACRO_PRINT, NULL);
			ParseString(0); 
			break;

		case TOKEN_ID:
		case TOKEN_DOT:
			ParseAssign(MODE_VAR, SUBMODE_EMPTY, NULL); 
			break;

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
//	G_CONDITION_EXP = 0;
	SYSTEM_PARSE = true;
}

/*
void ProcCheck(Var * proc)
{

}
*/