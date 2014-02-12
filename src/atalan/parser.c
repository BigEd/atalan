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
	[  ]*  zero or more repeats of the part
	  |    option
	"sk"   verbatim text
	<rule> reference to other rule
	~      there can not be space between previous and next syntactic token
*/

// A   Array Parser

#include "language.h"

// How many vars in a row are processed before error
#define MAX_VARS_COMMA_SEPARATED 100

Var *  STACK[STACK_LIMIT];
UInt16 TOP;
UInt16 VAR_LINE_POS;		// Position of top variable on stack. This is used when 

GLOBAL Bool  USE_PARSE;

Type * RESULT_TYPE;
Bool   EXP_IS_DESTINATION = false;		// Parsed expression is destination
Bool   EXP_IS_REF = false;				// Parsed expression will be used as a reference
Var *  EXP_EXTRA_SCOPE;					// Scope used by expression parsing to find extra variables.
UInt16 EXP_PARENTHESES;					// If > 0, we are parsing parenthesis inside expression
Bool   EXP_INSTR = false;				// parsing expression for instruction (store minus as variable)
Bool   EXP_DEFINES_VARS = false;
static Var * UNUSED_INSTR_SCOPE = NULL;

//TODO: Remove
Type   EXP_TYPE;			// Type returned by expression

LineNo  OP_LINE_NO;
LinePos OP_LINE_POS;				// Position of last parsed binary operator

Bool    PARSING_RULE = false;
Bool    PARSING_PATTERN = false;
Bool    PARSING_CONDITION = false;

// Is modified as the expression gets generated

void ParseExpRoot();

void ParseEnumItems(Type * type, UInt16 column_count);
void ParseAssign(InstrOp mode, VarSubmode submode, Type * to_type);
UInt16 ParseSubExpression(Type * result_type);
void ParseCall(Var * proc);
void ParseMacro(Var * macro);
Type * ParseType();
Var * ParseArrayElement(Var * arr);
Var * ParseStructElement(Var * arr);
Var * ParseFile();
void ParseIf(Bool negated, Var * result);

// This variable is set to true, when we parse expression inside condition.
// It modifies parsing behavior concerning and, or and not.

//UInt8 G_CONDITION_EXP;

#define STR_NO_EOL 1
void ParseString(InstrBlock * i, UInt32 flags);
void ParseExpressionType(Type * result_type);
void ParseExpression(Var * result);

// All rules share one common scope, so they do not mix with normal scope of program.

Var * RULE_SCOPE;

extern Var * LAST_VAR;

/*

Parser uses buffer of variables.
Usually, it is used as stack, buf sometimes it is used as a queue too.

*/

void BufEmpty()
{
	TOP = 0;
}

void BufPush(Var * var)
{
	STACK[TOP++] = var;
}

Var * BufPop()
{
	Var * var;
	TOP--;
	var = STACK[TOP];
	return var;
}

Var * ParseScope()
{
	Bool spaces;
	Var * var, * scope = NULL;
	do {
		if (scope != NULL) {
			var = VarFind(scope, NAME);
		} else {
			var = VarFind2(NAME);
		}

		if (var == NULL) break;
		NextToken();

		scope = var;
		spaces = Spaces();
		if (spaces || !NextIs(TOKEN_DOT)) break;
	} while(1);

	return scope;
}

Var * FindExpVar()
{
	Var * var = NULL;

	if (EXP_EXTRA_SCOPE != NULL) {
		var = VarFind(EXP_EXTRA_SCOPE, NAME);
	} 
	if (var == NULL) {
		var = VarFind2(NAME);
	}
	return var;
}

Var * ParseSimpleVariable()
/*
Purpose:
	Parse variable name.
	Use either extra context or current context.
Syntax:  var_name
*/
{
	Var * var = FindExpVar();
	if (var == NULL) {
		SyntaxError("Unknown variable");
	} else {
		NextToken();
	}

	return var;
}

Var * ParseVariable()
/*
Purpose:
	Parse variable name.

Syntax:  var_name [ ~ "." ~ var_name  ]*
*/
{
	Bool spaces;
	Var * var = NULL, * scope;
	do {
		scope = var;
		if (scope != NULL) {
			var = VarFind(scope, NAME);
		} else {
			if (EXP_EXTRA_SCOPE != NULL) {
				var = VarFind(EXP_EXTRA_SCOPE, NAME);
			} 
			if (var == NULL) {
				var = VarFind2(NAME);
			}
		}
		spaces = Spaces();
		if (var == NULL) {
			SyntaxError("Unknown variable");
		} else {
			NextToken();
		}
	} while(!spaces && NextIs(TOKEN_DOT));

	return var;
}

Var * ParseVariable2()
/*
Purpose:
	Parse variable name, possibly creating subelements.

Syntax:  var_name [ ~ "." ~ var_name  ]*
*/
{
	Bool spaces;
	Var * var = NULL, * scope;
	do {
		scope = var;
		if (scope != NULL) {
			var = VarFind(scope, NAME);
		} else {
			if (EXP_EXTRA_SCOPE != NULL) {
				var = VarFind(EXP_EXTRA_SCOPE, NAME);
			} 
			if (var == NULL) {
				var = VarFind2(NAME);
			}
		}
		spaces = Spaces();
		if (var == NULL) {
			var = NewVar(scope, NAME, NULL);			
			CellSetLocation(var, SRC_FILE, LINE_NO, TOKEN_POS);
			SetFlagOn(var->submode, SUBMODE_FRESH);
			if (ParsingSystem()) SetFlagOn(var->submode, SUBMODE_SYSTEM);

//			SyntaxError("Unknown variable");
		} else {
			NextToken();
		}
	} while(!spaces && NextIs(TOKEN_DOT));

	return var;
}

void ParseCommands();

/*********************************************************************

  Parse Expression

*********************************************************************/
//$E


void InstrBinary(InstrOp op)
/*
Purpose:
	Generate binary instruction as part of expression.
*/
{
	Var * result, * arg1, * arg2;

	arg1 = STACK[TOP-2];
	arg2 = STACK[TOP-1];

	result = CellOp(op, arg1, arg2);
	ASSERT(result != NULL);
	TOP--;
	STACK[TOP-1] = result;
	return;

	// Todo: we may use bigger of the two
	if (RESULT_TYPE == NULL) {
		RESULT_TYPE = STACK[TOP-2]->type;
	}

	// Try to evaluate the instruction as constant.
	// If we succeed, no instruction is generated, we instead push the result on stack

	result = InstrEvalConst(op, arg1, arg2);
	if (result == NULL) {
		if (EXP_INSTR) {
			result = NewOp(op, arg1, arg2);
		} else {
			result = NewTempVar(NULL);
			CellSetLocation(result, SRC_FILE, OP_LINE_NO, OP_LINE_POS);
			GenPos(op, result, arg1, arg2);
			OP_LINE_NO = 0;
			OP_LINE_POS = 0;
		}
	}

	TOP--;
	STACK[TOP-1] = result;
}

void InstrUnary(InstrOp op)
/*
Purpose:
	Generate unary instruction in expression.
*/
{
	Var * result;
	Var * top;

	BigInt * n1, r;
//	PrintStack();

	top = STACK[TOP-1];

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

	n1 = IntFromCell(top);

	if (n1 != NULL) {
		n1 = &STACK[TOP-1]->n;
		switch(op) {
		case INSTR_HI: 
			IntSet(&r, n1);
			IntDivN(&r, 256);
			IntAndN(&r, 0xff);

			//r = (n1 >> 8) & 0xff; 
			break;
		case INSTR_LO: 
			IntSet(&r, n1);
			IntAndN(&r, 0xff);
			//r = n1 & 0xff; 
			break;
		case INSTR_SQRT: 
			IntSqrt(&r, n1);
//			r = (UInt32)sqrt(n1); 
			break;
		default: goto unknown_unary; break;
		}
		result = IntCell(&r);
		goto done;
	}
unknown_unary:
	result = NewTempVar(RESULT_TYPE);
	Gen(op, result, top, NULL);
done:
	STACK[TOP-1] = result;
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
			c = NAME[0];
			if (NAME[1] == 0 && c >= 'A' && c <='Z') {
				arg_no = c - 'A' + 1;
			} else {
				SyntaxError("Rule argument must be A..Z");
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

Bool ParseArg(Var ** p_var)
{
	UInt8 arg_no;
	*p_var = NULL;
	arg_no = ParseArgNo();
	if (arg_no == 0) return false;
	*p_var = VarRuleArg(arg_no-1);
	return true;
}

Var * ParseArrayIdx(Type * atype);

Var * ParseArrayItem(Var * arr)
/*
Purpose:
	Parse index after $, % or #.
*/
{
	UInt8 arg_no;
	Var * item = NULL;

	NextToken();
	if (LexInt(&item)) {
	} else if (TOK == TOKEN_ID) {
		if (VarIsArray(arr)) {
			item = VarFindAssociatedConst(IndexType(arr->type), NAME);
		}
		if (item != NULL) {
			NextToken();
		} else {
			item = ParseSimpleVariable();
		}
	} else if (arg_no = ParseArgNo2()) {		//TODO: Use ParseArgNo
		item = VarRuleArg(arg_no-1);
		NextToken();
	} else if (TOK == TOKEN_OPEN_P) {
		item = ParseArrayIdx(arr->type);
	} else {
		SyntaxError("Expected constant or variable name");
	}
	return item;
}

Var * ParseSpecialArrays(Var * arr)
/*
Syntax:
   <arr>$<idx> | <arr>#<idx> | <arr>%<idx>
*/
{
	// Special arrays are parsed using character parsing, as that prevents $ from being used as hexadecimal integer prefix
	Var * item, * var;
	var = NULL;

	if (NextCharIs('$')) {
		item = ParseArrayItem(arr);
		ifok {
			var = VarNewByteElement(arr, item);
		}
	} else if (NextCharIs('%')) {
		item = ParseArrayItem(arr);
		ifok {
			var = VarNewBitElement(arr, item);
		}
	} else if (NextCharIs('#')) {
		item = ParseArrayItem(arr);
		ifok {
			var = VarNewElement(arr, item);
		}
	}
	return var;
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

	// Try to find local variable in local scope of array variable.
	
	if (arr->mode == INSTR_ELEMENT /*&& arr->adr->mode == INSTR_SCOPE*/) {
		NextIs(TOKEN_DOT);
		if (TOK == TOKEN_ID) {
			item = VarFind(arr->adr, NAME);
			if (item != NULL) {
				if (item->type->variant == TYPE_ARRAY) {
					idx = VarNewElement(item, arr->var);
//					idx->type = idx->type->element;
				} else {
					SyntaxError("$Variable is not an array.");
				}
			} else {
				if (arr->adr->mode == INSTR_SCOPE) {
					SyntaxError("$Scope does not contain member with name");
				}
			}
//			NextToken();
		}

	}
	
	if (idx == NULL) {
		if (arr->type->mode == INSTR_TYPE && arr->type->variant == TYPE_STRUCT) {
			NextIs(TOKEN_DOT);
			if (TOK == TOKEN_ID) {
				item = VarFind(arr->type, NAME);
				if (item != NULL) {
					idx = VarNewElement(arr, item);
				} else {
					SyntaxError("Structure does not contain member with name [$]");
				}
	//			NextToken();
			} else {
				SyntaxError("Expected structure member identifier after '.'");
			}
		} else {
			if (NextIs(TOKEN_DOT)) {
				ASSERT(TOK == TOKEN_ID || TOK >= TOKEN_KEYWORD && TOK<=TOKEN_LAST_KEYWORD);
				item = VarFind(arr, NAME);
				if (item == NULL) {
					item = NewVar(arr, NAME, NULL);
					SetFlagOn(item->submode, SUBMODE_FRESH);
					CellSetLocation(item, SRC_FILE, LINE_NO, TOKEN_POS);
					NextToken();
					return item;
//TODO: Check nonexistent element
//					SyntaxError("Variable [A] does not contain member [$]");
				}
			}
		}
	}

	ifok {
		item = ParseSpecialArrays(idx);
		if (item != NULL) {
			idx = item;
		} else {
			NextToken();
		}
	}
	return idx;
}

Var * ParseArrayIdx(Type * atype)
{
	UInt16 top;
	Type * idx_type, * t;
	Var * idx, * idx2;
	UInt16 bookmark;
	TypeVariant tv;

	top = TOP;

	tv = TYPE_VOID;

	// First dimension (or first element of tuple)
	idx_type = NULL;

	if (atype != NULL) {
		tv = atype->variant;

		if (tv == TYPE_ARRAY) {
			idx_type = t = atype->index;
			if (t->variant == TYPE_TUPLE) idx_type = t->left;
		} else if (tv == TYPE_ADR) {
			TODO("Better implementation");
//			idx_type = &TINT;		// Access to n-th element of an  array specified by address. In this case, the size of index is not bound.
		} else if (tv == TYPE_SCOPE) {
			idx_type = NULL;
		} /*else {
			// This is default case for accessing bytes of variable
			// It should be replaced by x$0 x$1 syntax in the future.
			idx_type = TypeByte();
		}
*/
	} //else {
//		idx_type = NULL;
//	}

	idx = idx2 = NULL;

	// Syntax: arr ~ "#" <int> | <id> | "("  ")"
	if (NextIs(TOKEN_HASH)) {
		if (LexInt(&idx)) {
			goto done_idx;
		} else if (TOK == TOKEN_ID) {
			idx = ParseVariable();
			goto done_idx;
		} else if (TOK == TOKEN_OPEN_P) {
			// If there is opening brace, continue to parsing index
		} else {
			SyntaxError("Expected integer, variable or () after #");
		}
	}

	// Array element access uses always () block
	EnterBlock();

	bookmark = SetBookmark();

	// Syntax a()  represents whole array
	if (tv == TYPE_ARRAY && TOK == TOKEN_BLOCK_END) {
		idx = idx_type;
		goto done;
	}

	// It may be (..<n>), or even () use min as default
	if (TOK == TOKEN_DOTDOT) {
		idx  = CellMin(idx_type);
	} else {
		ParseSubExpression(idx_type);
		ifok {
			idx = STACK[top];
		}
	}

	TOP = top;
	bookmark = SetBookmark();

	// <min>..<max>
	if (NextIs(TOKEN_DOTDOT)) {
		if (TOK == TOKEN_COMMA || TOK == TOKEN_BLOCK_END) {
			if (tv == TYPE_ARRAY) {
				idx2 = CellMax(idx_type);
			}
		} else {
			ParseSubExpression(idx_type);
			idx2 = STACK[top];
		}
		if (idx2 != NULL) {
			idx = NewRange(idx, idx2);
		}
	}

	ifok {
		
		while (NextIs(TOKEN_COMMA)) {

			// (idx1, (idx2, idx3))

			idx_type = NULL;
			if (t->variant == TYPE_TUPLE) {
				idx_type = t = t->right;
				if (t->variant == TYPE_TUPLE) {
					idx_type = t->left;
				}
			}

			if (idx_type != NULL) {
				TOP = top;
				bookmark = SetBookmark();
				ParseSubExpression(idx_type);
				idx2 = STACK[TOP-1];
				idx = NewTuple(idx, idx2);
			} else {
				SyntaxError("Too many indexes specified");
			}
		}

	}
done:

	ifok {
		if (!NextIs(TOKEN_BLOCK_END)) SyntaxError("missing closing ')'");
	}
	TOP = top;
done_idx:
	return idx;
}

Var * ParseArrayElement(Var * arr)
/*
Purpose:
	Parse access to array element.
	Parsed variable is of type element.
Syntax: arr "#" idx | arr "(" idx ")" | arr "()"
*/
{
	UInt16 top;
	Type * idx_type, * atype, * t;
	Var * idx, * idx2, * item;
	UInt16 bookmark;
	TypeVariant tv;

	top = TOP;

	atype = arr->type;
	tv = TYPE_VOID;

	// First dimension (or first element of tuple)
	t = idx_type = NULL;

	if (atype != NULL) {
		tv = atype->variant;

		if (tv == TYPE_ARRAY) {
			idx_type = t = atype->index;
			if (t->variant == TYPE_TUPLE) idx_type = t->left;
		} else if (tv == TYPE_ADR) {
			TODO("Better implementation");
//			idx_type = &TINT;		// Access to n-th element of an  array specified by address. In this case, the size of index is not bound.
		} else if (tv == TYPE_SCOPE) {
			idx_type = NULL;
		}
	} //else {
//		idx_type = NULL;
//	}

	idx = idx2 = NULL;

	// Syntax: arr ~ "#" <int> | <id> | "("  ")"
	if (NextIs(TOKEN_HASH)) {
		if (LexInt(&idx)) {
			goto done_idx;
		} else if (TOK == TOKEN_ID) {
			idx = ParseVariable();
			goto done_idx;
		} else if (TOK == TOKEN_OPEN_P) {
			// If there is opening brace, continue to parsing index
		} else {
			SyntaxError("Expected integer, variable or () after #");
		}
	}

	// Array element access uses always () block
	EnterBlock();

	bookmark = SetBookmark();

	// Syntax a()  represents whole array
	if (tv == TYPE_ARRAY && TOK == TOKEN_BLOCK_END) {
		idx  = idx_type;
		goto done;
	}

	// It may be (..<n>), or even () use min as default
	if (TOK == TOKEN_DOTDOT) {
		idx  = CellMin(idx_type);
	} else {
		ParseSubExpression(idx_type);
		ifok {
			idx = STACK[top];
		}
	}

	TOP = top;
	bookmark = SetBookmark();

	// <min>..<max>
	if (NextIs(TOKEN_DOTDOT)) {
		if (TOK == TOKEN_COMMA || TOK == TOKEN_BLOCK_END) {
			if (tv == TYPE_ARRAY) {
				idx2 = CellMax(idx_type);
			}
		} else {
			ParseSubExpression(idx_type);
			idx2 = STACK[top];
		}
		if (idx2 != NULL) {
			idx = NewRange(idx, idx2);
		}
	}

	ifok {
		
		while (NextIs(TOKEN_COMMA)) {

			// (idx1, (idx2, idx3))

			if (t != NULL) {
				idx_type = NULL;
				if (t->variant == TYPE_TUPLE) {
					idx_type = t = t->right;
					if (t->variant == TYPE_TUPLE) {
						idx_type = t->left;
					}
				}
			}

			if (t == NULL || idx_type != NULL) {
				TOP = top;
				bookmark = SetBookmark();
				ParseSubExpression(idx_type);
				idx2 = STACK[TOP-1];
				idx = NewTuple(idx, idx2);
			} else {
				if (t != NULL) {
					SyntaxError("Too many indexes specified");
				}
			}
		}

	}
done:

	ifok {
		if (!NextIs(TOKEN_BLOCK_END)) SyntaxError("missing closing ')'");
	}
/*
	if (tv == TYPE_SCOPE) {
		// scope(idx).array
		if (NextIs(TOKEN_DOT)) {
		}
	}
*/
	TOP = top;

done_idx:
	item = VarNewElement(arr, idx);

	return item;
}


typedef struct {
	Var * min[5];
	Int16 min_dist;
	UInt16 cnt;
} SimmilarNames;

void SimmilarNamesInit(SimmilarNames * names)
{
	names->min_dist = 3;
	names->cnt = 0;
}

void SimmilarNamesAdd(SimmilarNames * names, char * name,  Var * v)
{
	Int16 dist;
	if (v->name != NULL && v->idx == 0) {
		dist = StrEditDistance(name, v->name);
		if (dist < 2) {
			if (dist < names->min_dist) {
				names->min[0] = v;
				names->min_dist = dist;
				names->cnt = 1;
			} else if (dist == names->min_dist) {
				if (names->cnt < 5) names->min[names->cnt] = v;
				names->cnt++;
			}
		}
	}
}

void ReportSimilarNames(char * name)
{
	SimmilarNames names;
	Var * v;
	UInt16 i;
	Var * scope;
	UInt8 proc_cnt;
	Bool printed = false;
	UInt16 len;
	UInt8 color;


	v = NULL;

	// Find names with lowest edit distance.
	// There may be up to 5 such names.
	// We are first trying to find the names within scope.

	len = StrLen(name);

	color = PrintColor(COLOR_HINTS);

	// Do not try to suggest simmilar names, if the variable consists of only one character

	if (len > 1) {
		SimmilarNamesInit(&names);
		for(scope = SCOPE; scope != NULL; scope = scope->scope) {
			FOR_EACH_LOCAL(scope, v)
				SimmilarNamesAdd(&names, name, v);
			NEXT_LOCAL
		}

		if (names.cnt > 0) {
			Print("Did you mean ");
			if (names.cnt > 5) names.cnt = 5;
			for(i=0; i<names.cnt; i++) {
				if (i>0) {
					if (i == names.cnt-1) {
						Print(" or ");
					} else {
						Print(" ,");
					}
				}
				PrintQuotedCellName(names.min[i]);
			}
			Print("?\n");
			printed = true;
		}
	}

	// We are trying to locate same variable in different scope
	SimmilarNamesInit(&names);
	FOR_EACH_VAR(v)
		if (!VarIsArg(v)) {
			SimmilarNamesAdd(&names, name, v);
		}
	NEXT_VAR

	if (names.min_dist == 0) {
		if (names.cnt > 0) {
			if (names.cnt > 5) names.cnt = 5;
			if (printed) PrintEOL();
			printed = false;
			proc_cnt = 0;
			for(i=0; i<names.cnt; i++) {
				v = names.min[i];
				if (v->line_no != 0) {

					if (!printed) {
						Print("Variable with the same name was declared in different scope:"); PrintEOL(); PrintEOL();
						printed = true;
					}

					Print("   "); Print(v->file->name); Print("("); PrintInt(v->line_no); Print(")"); 
					// Find some scope with name

					scope = v->scope;
					while(scope != NULL && scope->name == NULL) scope = scope->scope;
					if (scope != NULL) {
						if (scope->type->mode == INSTR_FN) {
							Print("  in procedure '"); 
						} else if (scope->type->variant == TYPE_SCOPE) {
							Print("  in scope '"); 
						}
						Print(scope->name); Print("'");
						proc_cnt++;
					}
					PrintEOL();
				}
			}

			if (proc_cnt > 0) {
				PrintEOL();
				Print("Did you mean to declare the variable as global?");
			}
		}
	}
	PrintColor(color);

}

void ParseOperand()
{
	Var * var = NULL, * item = NULL, * proc, * arg;
	Bool ref = false;
	Bool type_match;
	UInt8 arg_no;
	Type * type;
	UInt32 arg_cnt;
	Var * en;

	type_match = false;

	if (NextIs(TOKEN_OPEN_P)) {
		if (NextIs(TOKEN_CLOSE_P)) {
			BufPush(VOID);
		} else {
			NewBlock(TOKEN_CLOSE_P, TOKEN_VOID);
			EXP_PARENTHESES++;
			ParseExpRoot();
			if (!NextIs(TOKEN_BLOCK_END)) SyntaxError("missing closing ')'");			
			EXP_PARENTHESES--;
		}
	} else {
		// file "slssl"
		if (NextIs(TOKEN_FILE)) {
			// This will be constant variable with temporary name, array of bytes
			item = ParseFile();
			ifok {
				type = RESULT_TYPE;
				if (type == NULL) {
					type = NewArrayType(NULL, NULL);
				}
				var = NewTempVar(type);
				var->mode = INSTR_INT;

				GenBegin();
				Gen(INSTR_FILE, NULL, item, NULL);
				var->instr = GenEnd();
			}
		// @id denotes reference to variable
		} else if (NextIs(TOKEN_ADR)) {
			if (arg_no = ParseArgNo2()) {
				var = VarRuleArg(arg_no-1);
			} else {
				var = FindExpVar();
			}

			if (var != NULL) {
				var = VarNewDeref(var);
			} else {
				SyntaxError("$unknown variable");
			}
			goto no_id;
		} else if (arg_no = ParseArgNo2()) {
			var = VarRuleArg(arg_no-1);
			goto indices;

		} else if (LexNum(&var)) {
		} else if (TOK == TOKEN_ID) {

			var = FindExpVar();

			//TODO: We should try to search for the scoped constant also in case the resulting type
			//      does not conform to requested result type

			if (var != NULL) {

				// Out-only variables may not be in expressions unless this is destination expression
				if (!EXP_IS_DESTINATION && !EXP_IS_REF) {
					if (var->type->mode != INSTR_FN && OutVar(var) && !InVar(var)) {
						ErrArg(var);
						LogicError("Variable [A] may be only written", 0);
					}
				}

				type_match = IsSubset(var, RESULT_TYPE);
			}

			// Try to find using result scope (support for associated constants)
			if (var == NULL || !type_match) {
				if (RESULT_TYPE != NULL) {
					item = VarFind(RESULT_TYPE, NAME); 
					if (item != NULL) var = item;
				}
			}

			if (var == NULL) {
				if (!EXP_DEFINES_VARS) {
					SyntaxError("~unknown name [$]");
					ReportSimilarNames(NAME);
					EndErrorReport();
				}

				// We are going to make the compilation continue by creating fake variable as if the
				// programmer has defined it.

				var = NewVar(NULL, NAME, ANY);
				SetFlagOn(var->submode, SUBMODE_FRESH);

				goto indices;

			} else {
				OP_LINE_POS = TOKEN_POS + 1;
			}


no_id:
			// Procedure call
			if (var->type->mode == INSTR_FN /*|| (var->type->variant == TYPE_ADR && var->type->element != NULL && var->type->element->variant == TYPE_PROC)*/) {
				if (RESULT_TYPE != NULL && RESULT_TYPE->variant == TYPE_ADR) {
					// this is address of procedure
				} else {
					proc = var;
					NextToken();
					if (IsMacro(var->type)) {
						ParseMacro(proc);
						return;
					} else {
						ParseCall(proc);
					}

					// *** Register Arguments (5)
					// After the procedure has been called, we must store values of all output register arguments to temporary variables.
					// This prevents trashing the value in register by some following computation.

					arg_cnt = 0;

					FOR_EACH_ITEM(en, arg, ResultType(var->type->type))

						var = arg;
						if (VarIsReg(arg)) {
							var = NewTempVar(arg->type);
							GenLet(var, arg);
						}
						BufPush(var);		
						arg_cnt++;
					NEXT_ITEM(en, arg)

					if (arg_cnt == 0) {
						SyntaxError("PROC does not return any result");
					}
					return;
				}
			}
indices:
			item = ParseSpecialArrays(var);
			if (item != NULL) {
				var = item;
			} else {
				NextToken();
			}

retry_indices:
//			while(item = ParseSpecialArrays(var)) var = item;

			if (NextNoSpaceIs(TOKEN_DOT)) {
				//TODO: Why is this?
				if (VarIsArg(var)) {
					var = VarNewElement(var, TextCell(NAME));
					item = ParseSpecialArrays(var);
					if (item != NULL) {
						var = item;
					} else {
						NextToken();
					}
				} else {
					if (var->type->variant == TYPE_STRUCT || var->mode == INSTR_ELEMENT) {
						var = ParseStructElement(var);
						goto retry_indices;
					} else {
						if (TOK == TOKEN_ID) {
							item = VarFind(var, NAME);
							// If the item is not part of variable scope, try to find it in type
							if (item == NULL) {
								item = VarFind(var->type, NAME);
								if (item != NULL) {
									// If the found item is array, we use the variable as an index to the array
									if (item->type->variant == TYPE_ARRAY) {
										item = VarNewElement(item, var);
									}
								}
							}

							// If the element has not been found, try to match some built-in elements

							if (item == NULL) {
								item = VarField(var, NAME);
							}

							if (item != NULL) {
								var = item;
								goto indices;	//NextToken();
							} else {
								SyntaxError("$unknown item");
							}
						} else {
							SyntaxError("variable name expected after .");
						}
					}
				}

			// Access to array may be like
			// 
			} else if (TOK_NO_SPACES == TOKEN_OPEN_P) {
				item = ParseArrayElement(var);
				var = item;
				goto retry_indices;
			}
		} else {
			return;
		}

		if (NextIs(TOKEN_COLON)) {
			if (var != NULL) {
				if (FlagOn(var->submode, SUBMODE_FRESH)) {
					ParseExpression(NULL);
					ifok {
						var->type = BufPop();
					}
				} else {
					SyntaxError("Type redefinition is not allowed.");
				}
			} else {
				SyntaxError("There must be variable name to the left of double colon.");
			}
		}

		// Assign address
		if (RESULT_TYPE != NULL && RESULT_TYPE->variant == TYPE_ADR && var->type->variant != TYPE_ADR) {
			//TODO: Check type of the adress
			//      Create temporary variable and generate letadr
			BufPush(var);				
			InstrUnary(INSTR_LET_ADR);
			return;
		}
//done:
		BufPush(var);
	}
}

Bool NextOpIs(Token tok)
{
	Bool r;
	LinePos line_pos;
	LineNo line_no;

	line_pos = TOKEN_POS;
	line_no  = LINE_NO;
	r = NextIs(tok);
	if (r) {
		OP_LINE_NO  = line_no;
		OP_LINE_POS = line_pos+1;
	}
	return r;
}

Bool ParseRelOp(InstrOp * pOp)
{
	InstrOp op;
	if      (NextIs(TOKEN_EQUAL))        op = INSTR_EQ;
	else if (NextIs(TOKEN_NOT_EQUAL))    op = INSTR_NE; 
	else if (NextIs(TOKEN_LOWER))        op = INSTR_LT; 
	else if (NextIs(TOKEN_HIGHER))       op = INSTR_GT; 
	else if (NextIs(TOKEN_LOWER_EQUAL))  op = INSTR_LE; 
	else if (NextIs(TOKEN_HIGHER_EQUAL)) op = INSTR_GE; 
	else return false;
	*pOp = op;
	return true;
}

void ParseSequence()
{
	Var * var[3];
	UInt16 var_cnt;
	Var * limit, * step, * seq;
	Var * step2, * geom_step;

	InstrOp step_op = INSTR_VOID;
	InstrOp compare_op = INSTR_VOID;

	limit = NULL; step = NULL;
	var_cnt = 0;
	ifok {
		do {		
			if (TOK == TOKEN_DOTDOT) break;
			ParseOperand();
			var[var_cnt] = BufPop();
			var_cnt++;
			if (!NextIs(TOKEN_COMMA)) break;
			iferr break;
		} while(var_cnt<3);

		if (NextIs(TOKEN_DOTDOT)) {
			if (!NextIs(TOKEN_COMMA)) {
				SyntaxError("Expected comma before sequence limit");
				return;
			}
			ParseOperand();
			limit = BufPop();
			ifok {
				step = ONE;
				step_op = INSTR_ADD;
				compare_op = INSTR_LE;
				
				if (var_cnt >= 1) {
					if (IsLower(limit, var[0])) {
						step_op = INSTR_SUB;
						compare_op = INSTR_GE;
					}
				}

				if (var_cnt >= 2) {
					if (step_op == INSTR_ADD) {
						step = Sub(var[1], var[0]);
						if (IsLowerEq(step, ZERO)) {
							SyntaxError("Invalid sequence.");
						}
					} else {
						step_op = INSTR_SUB;
						compare_op = INSTR_GE;
						step = Sub(var[0], var[1]);
					}
				}

				if (var_cnt == 3) {
					if (step_op == INSTR_ADD) {
						step2 = Sub(var[2], var[1]);
						geom_step = Div(step2, step);
						step = geom_step;
						step_op = INSTR_MUL;
					}
				}
				
				if (var_cnt > 3) {
					SyntaxError("Multiple variables in sequence not supported yet.");
				}
			}

		} else {
			SyntaxError("Expected .. in sequence definition");
		}

		ifok {
			seq = NewSequence(var[0], step, step_op, limit, compare_op);
			BufPush(seq);
		}
	}
}

void ParseUnary()
{
	Var * var;
	if (NextOpIs(TOKEN_MINUS)) {
		// Unary minus before X is interpreted as 0 - X
		BufPush(ZERO);
		ParseOperand();
		InstrBinary(INSTR_SUB);
	} else if (NextOpIs(TOKEN_HI)) {
		ParseOperand();
		InstrUnary(INSTR_HI);
	} else if (NextOpIs(TOKEN_LO)) {
		ParseOperand();
		InstrUnary(INSTR_LO);
	}  else if (NextOpIs(TOKEN_BITNOT)) {
		ParseOperand();
		InstrUnary(INSTR_NOT);
	} else if (NextOpIs(TOKEN_SQRT)) {
		ParseOperand();
		InstrUnary(INSTR_SQRT);
	} else if (NextOpIs(TOKEN_PLUSMINUS)) {
		ParseOperand();
		var = BufPop();
		BufPush(NewRange(Sub(ZERO, var), var));
	} else if (NextIs(TOKEN_SEQUENCE)) {
		ParseSequence();
	} else {
		ParseOperand();
	}
}

void ParseMulDiv()
{
	ParseUnary();
retry:
	if (NextOpIs(TOKEN_MUL)) {
		ParseUnary();
		ifok {
			InstrBinary(INSTR_MUL);
		}
		goto retry;
	} else if (NextOpIs(TOKEN_DIV)) {
		ParseUnary();
		ifok {
			InstrBinary(INSTR_DIV);
		}
		goto retry;
	}  else if (NextOpIs(TOKEN_MOD)) {
		ParseUnary();
		ifok {
			InstrBinary(INSTR_MOD);
		}
		goto retry;
	}
}

void ParsePlusMinus()
{
	LinePos pos;

	ParseMulDiv();
retry:
	pos = TOKEN_POS;
	if (NextOpIs(TOKEN_PLUS)) {
		ParseMulDiv();
		ifok {
			InstrBinary(INSTR_ADD);
		}
		goto retry;
	} else if (NextOpIs(TOKEN_MINUS)) {
		ParseMulDiv();
		ifok {
			InstrBinary(INSTR_SUB);
		}
		goto retry;
	}
}

void ParseBinaryAnd()
{
	ParsePlusMinus();
retry:
	if (NextOpIs(TOKEN_BITAND)) {
		ParsePlusMinus();
		ifok {
			InstrBinary(INSTR_AND);
		}
		goto retry;
	}
}

void ParseBinaryOr()
{
	ParseBinaryAnd();
retry:
	if (NextOpIs(TOKEN_BITOR)) {
		ParseBinaryAnd();
		ifok {
			InstrBinary(INSTR_OR);
		}
		goto retry;
	} else if (NextOpIs(TOKEN_BITXOR)) {
		ParseBinaryAnd();
		ifok {
			InstrBinary(INSTR_XOR);
		}
		goto retry;
	}
}

void ParseRangeOp()
{
	Var * var;
	ParseBinaryOr();
	if (NextOpIs(TOKEN_DOTDOT)) {
		ParseBinaryOr();
		var = NewRange(STACK[TOP-2], STACK[TOP-1]);
		TOP--;
		STACK[TOP-1] = var;
	} else if (NextOpIs(TOKEN_PLUSMINUS)) {
		ParseBinaryOr();
		var = NewRange(Sub(STACK[TOP-2], STACK[TOP-1]), Add(STACK[TOP-2], STACK[TOP-1]));
		TOP--;
		STACK[TOP-1] = var;
	}
}

void ParseTuple()
{
	Var * var;
	ParseRangeOp();
retry:
	if (EXP_PARENTHESES > 0) {
		if (NextOpIs(TOKEN_COMMA)) {
			ParseTuple();
			ifok {
				var = NewTuple(STACK[TOP-2], STACK[TOP-1]);
				TOP--;
				STACK[TOP-1] = var;
			}
			goto retry;
		}
	}
}

void ParseRel2()
{
	InstrOp op;
	ParseTuple();
	if (ParseRelOp(&op)) {
		ParseTuple();
		ifok {
			InstrBinary(op);
		}

	// Type guard

	// *** Type Assert (1)
	// We may assert the type inferencer deduced the correct type of a variable.
	// This is done using ::assert var:type:: syntax.

	// Type Assert is implemented using special INSTR_MATCH operator.

	} else if (NextIs(TOKEN_COLON)) {
		ParseTuple();
		ifok {
			InstrBinary(INSTR_MATCH);
		}
	}
}

void ParseLogAnd()
{
	ParseRel2();
	while(NextOpIs(TOKEN_AND)) {
		ParseRel2();
		ifok {
			InstrBinary(INSTR_AND);
		}
	}
}

void ParseLogOr()
{
	ParseLogAnd();
	while(NextOpIs(TOKEN_OR)) {
		ParseLogAnd();
		ifok {
			InstrBinary(INSTR_OR);
		}
	}
}

void ParseExpIf()
{
	Var * var;
	Bool negated = false;

	if (PARSING_CONDITION || ParsingRule()) {
		ParseLogOr();
		return;
	}
	// IF <condition> THEN <true_val> ELSE <false_val>

	if (NextIs(TOKEN_UNLESS)) {
		negated = true;
	}

	if (negated || NextIs(TOKEN_IF)) {
		// 1. create temporary variable
		// 2. parse condition
		var = NewTempVar(NULL);
		ParseIf(negated, var);
		BufPush(var);	
	} else {
		ParseTuple();
	}
}

void ParseExpRoot()
{
	ParseExpIf();
}

typedef struct {
	Type    type;			// inferred type of expression
	Type *  result;			// expected type of resulting value
							// For example type of variable, into which the expression result 
							// gets assigned.
	UInt16 top;				// top of the stack when the parsing started
	UInt16 parentheses;
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
	state.parentheses = EXP_PARENTHESES;

	RESULT_TYPE = result_type;
	EXP_TYPE.mode = INSTR_ANY;
	EXP_PARENTHESES = 0;

	ParseExpRoot();

	memcpy(&EXP_TYPE, &state.type, sizeof(Type));
	RESULT_TYPE = state.result;
	EXP_PARENTHESES = state.parentheses;
	return TOP - state.top;
}

void ParseExpressionType(Type * result_type)
{
	RESULT_TYPE = result_type;
	TOP = 0;
	EXP_TYPE.mode = INSTR_ANY;
	ParseExpRoot();
}

void ParseDefExpression()
{
	EXP_DEFINES_VARS = true;
	ParseExpression(NULL);
	EXP_DEFINES_VARS = false;
}

void ParseExpression(Var * result)
/*
Parse expression, performing evaluation if possible.
If result mode is INSTR_INT, no code is to be generated.
*/
{
	Type * type;

	if (result == NULL) {
		RESULT_TYPE = NULL;
	} else {
		type = result->type;

		if (result->mode == INSTR_ELEMENT) {
			type = result->adr->type;
			if (VarIsArray(result->adr)) {
				RESULT_TYPE = type->element;
			} else if (type->variant == TYPE_ADR) {
				type = type->element;		// adr of array(index) of type
				if (type->mode == INSTR_ARRAY_TYPE) {
					RESULT_TYPE = ItemType(type);
				} else {
					RESULT_TYPE = type;
				}
			} else {
			}
		} else if (type != NULL && type->mode == INSTR_ARRAY_TYPE) {
			RESULT_TYPE = ItemType(type);
		} else {
			RESULT_TYPE = result->type;
		}
	}
	TOP = 0;
	EXP_TYPE.mode = INSTR_ANY;
	EXP_PARENTHESES = 0;
	ParseExpRoot();

	// When we parse very simple expressions, no instruction gets generated
	// Calling code would typically call generating let instruction.
	// We simulate it for the type here, so the type returned from expression
	// parsing is correct.

	ifok {
		if (EXP_TYPE.mode == INSTR_ANY) {
			if (TOP > 0) {
//				TypeLet(&EXP_TYPE, STACK[0]);
			}
		}
	}
}

Var * ParseCondExpression()
{
	Var * var;
	PARSING_CONDITION = true;
	ParseExpression(NULL);
	PARSING_CONDITION = false;
	var = BufPop();
	return var;
}

Var * ParseArray()
/*
Purpose:
	Parse array.
	Array elements are separated by comma.
*/
{
	Var * arr = NULL;
	Var * list, * item, * var;
	UInt32 count;
	Type * item_type;
//	BigInt min, max;
	UInt32 var_count;
	BigInt icnt;

	// Set the parentheses mode on.
	// This nesures, we are parse comma operator as part of expresion parsing.
	EXP_PARENTHESES = 1;
	ParseExpressionType(NULL);
	ifok {
		arr = STACK[0];
		if (arr->mode == INSTR_TUPLE) {

			// Generate array using tuples
			list = arr;

			// Detect, if the array is constant or if it contains some variables
			var_count = 0;
			item = list;
			do {
				if (item->mode == INSTR_TUPLE) {
					var = item->adr;
					item = item->var;
				} else {
					var = item;
					item = NULL;
				}
				if (!CellIsConst(var)) var_count++;
			} while(item != NULL);

//			arr = VarAllocScopeTmp(NULL, INSTR_INT, NULL);
			item_type = TypeByte();  //TODO: Detect correct type of array

			count = 0;
			GenBegin();
			item = list;
			do {
				if (item->mode == INSTR_TUPLE) {
					var = item->adr;
					item = item->var;
				} else {
					var = item;
					item = NULL;
				}
				Gen(INSTR_DATA, NULL, var, NULL);
				count++;
			} while(item != NULL);

			IntInit(&icnt, count);
			arr = NewArray(TypeArray(NewRangeInt(Int0(), &icnt), item_type), GenEnd());
			IntFree(&icnt);
		}
	}
	return arr;
}

void ExpectExpression(Var * result)
{
	ParseExpression(result);
	ifok {
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

void ParseBlock(Token stop, Var * result)
{
	EnterBlockWithStop(stop);		// To each command block there is appropriate lexer block
	if (result == NULL) {
		ParseCommands();
	} else {
		ParseExpression(result);
		NextIs(TOKEN_EOL);
		GenLet(result, BufPop());
	}
	NextIs(TOKEN_BLOCK_END);	// Block must end with TOKEN_END_BLOCK
}

void ParseCommandBlock()
{
	ParseBlock(TOKEN_VOID, NULL);
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

void GenRel(InstrOp op, Var * left, Var * right)
{
	if (!G_BLOCK->not) op = OpNot(op);

	if (G_BLOCK->f_label == NULL) {
		G_BLOCK->f_label = VarNewTmpLabel();
	}
	Gen(op, G_BLOCK->f_label, left, right);
}


void ParseRel()
/*
	relop: "=" | "<>" | "<" | "<=" | ">" | ">="
	rel:  <exp> <relop> <exp> [<relop> <exp>]*
*/
{
	Var * left, * right;
	InstrOp op;
	Type * type;

	if (TOK == TOKEN_OPEN_P) {
		ParseCondParenthesis();
	} else {
		ParseExpression(NULL);
		ifok {
			left = STACK[0];

			// var <relop> var [<relop> var]
			if (ParseRelOp(&op)) {
				do {

					// For normal operation, we jump to false label when the condition does NOT apply
					// For example for if:
					// if <cond>
					//     <block>
					//
					// must skip the <block>.			

					ParseExpression(left);
					right = STACK[0];
					ifok {
						GenRel(op, left, right);
						left = right;
					}
				} while (ParseRelOp(&op));

			// Type guard
			} else if (NextIs(TOKEN_COLON)) {

				type = ParseType();
				right = NewCell(INSTR_TYPE);
				right->type = type;
				GenRel(INSTR_TYPE, left, right);

			// No relation operator follows the expression, this must be test of boolean variable
			} else {

				right = VarFindAssociatedConst(left, "true");
				if (right != NULL) {
					op = INSTR_EQ;
				} else {
					right = VarFindAssociatedConst(left, "false");
					if (right != NULL) op = INSTR_NE;
				}
				if (op != INSTR_VOID) {
					GenRel(op, left, right);
				} else {
					SyntaxError("variable is not of boolean type");
				}
			}
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

void SimpleIf(InstrOp op, Var * result, Var * arg1, Var * arg2)
{
	BeginBlock(TOKEN_IF);
	ParseAnd();
	if (G_BLOCK->t_label != NULL) {
		GenLabel(G_BLOCK->t_label);
	}
	Gen(op, result, arg1, arg2);
	if (G_BLOCK->f_label != NULL) {
		GenLabel(G_BLOCK->f_label);
	}
	EndBlock();
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
	Var * tmp;
	Var * body_label = NULL;
	Token tok;
	if (NextIs(TOKEN_EITHER)) {
		tmp = NewTempVar(NewRange(ZERO, ONE));
		GenLet(tmp, ZERO);
		SimpleIf(INSTR_LET, tmp, ONE, NULL); 
		if (NextIs(TOKEN_OR)) {
			SimpleIf(INSTR_XOR, tmp, tmp, ONE);
		}
		GenRel(INSTR_EQ, tmp, ONE);

	} else {	
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

}

void ParseLabel(Var ** p_label)
{
// Labels are global in procedure

	Var * var = NULL;

	if (ParseArg(&var)) {

	} else {
		ExpectToken(TOKEN_ID);
		if (TOK == TOKEN_ID) {
			var = FindOrAllocLabel(NAME);
			NextToken();
		}
	}
	*p_label = var;
	
}

void ParseGoto()
{
	Var * var;
	ParseLabel(&var);
	ifok {
		if (!VarIsLabel(var) && !VarIsArg(var)) {
			var = VarNewDeref(var);
		}
		GenGoto(var);
	}
	
}

void ParseIf(Bool negated, Var * result)
/*
Purpose:
	Parse command if.
	"IF" cond ["THEN"] commands ["ELSE IF" <cond> ["THEN"] commands]* ["ELSE"] commands
*/
{
	Var * cond, * label, * neg;
	
	cond = ParseCondExpression();

	iferr return;
	if (NextIs(TOKEN_GOTO)) {
		ParseLabel(&label);
		Gen(INSTR_IF, NULL, cond, label);
		return;
	}

	neg = Not(cond);
	label =  VarNewTmpLabel();
	// There may be optional THEN
	NextIs(TOKEN_THEN);

	Gen(INSTR_IF, NULL, neg, label);
	ParseBlock(TOKEN_ELSE, NULL);
	GenLabel(label);
}

void ParseFor()
/*
Syntax:
	for: ["for" <var> [":" <range>][#<index>][in <array>] ["where" where_cond]] ["until" cond | "while" cond]

*/
{
	Var * var, * where_t_label;
	char name[256];
	char idx_name[256];
	Var * min, * max, * step, * arr, * idx;
	Type * type;
	Var * where_cond;
	InstrBlock * cond_code, * where_cond_code, * body;
	LinePos idx_tok_pos;
//	Int32 n, nmask;
	LinePos token_pos;
	BigInt * n, nmask;
	BigInt t1, t2, t3;
	Bool higher;
	Var * cond;
	Var * body_label = VarNewTmpLabel();
	Bool cond_first = false;


	var = NULL; idx = NULL; min = NULL; max = NULL; cond = NULL; where_cond = NULL; step = NULL; arr = NULL;
	where_t_label = NULL;

	*idx_name = 0;
	EnterSubscope();

	// Parse "for" part.
	// We may also call this function when the loop begins just with "while" or "until".
	if (NextIs(TOKEN_FOR)) {
		GenLine();
		if (NextIs(TOKEN_HASH)) goto indexed;

		if (LexId(name)) {

			if (NextIs(TOKEN_HASH)) {
indexed:
				idx_tok_pos = TOKEN_POS;
				if (LexId(idx_name)) {
					if (NextIs(TOKEN_IN)) {
						goto parse_in;
					} else {
						SyntaxError("There should be in after index variable");
					}
				} else {
					SyntaxError("Expected index variable name");
				}
			}

			token_pos = TOKEN_POS;
			// for i ":" <range>
			if (NextIs(TOKEN_COLON)) {

				var = NewVar(NULL, name, NULL);
				CellSetLocation(var, SRC_FILE, LINE_NO, token_pos);

				ParseExpression(NULL);
//				ParseRange(&min, &max);
				ifok {
					type = BufPop();
					TypeLimits(type, &min, &max);
//					type = NewRange(min, max);
					ifok {

						if (CellIsConst(min) && CellIsConst(max)) {
							SetFlagOn(var->submode, SUBMODE_USER_DEFINED);
						}
//						var->type = type;
					}
				}
				idx = var;

			// For in, we create two local variables.
			// One is local (unnamed) index variable that we use to iterate.
			// The second is named as user specified and represents arr#index.
			} else if (NextIs(TOKEN_IN)) {
parse_in:
				arr = ParseArray();
				ifok {
					if (arr->type->mode == INSTR_ARRAY_TYPE) {
						type = IndexType(arr->type);
						TypeLimits(type, &min, &max);

						if (*idx_name != 0) {
							idx = NewVar(NULL, idx_name, type);
							CellSetLocation(idx, SRC_FILE, LINE_NO, idx_tok_pos);
							SetFlagOn(idx->submode, SUBMODE_USER_DEFINED);
						} else {
							idx = NewTempVar(type);						
						}
						var = NewVar(NULL, name, ItemType(arr->type));
						var->adr = VarNewElement(arr, idx);

						CellSetLocation(var, SRC_FILE, LINE_NO, token_pos);
						SetFlagOn(var->submode, SUBMODE_USER_DEFINED);


					} else {
						SyntaxError("Expression after IN must be array");
					}
				}
				
			// for i (range is not specified, this is reference to global variable or type)
			} else {
				SyntaxError("Expected : or in");
			}

		} else {
			SyntaxError("Expected loop variable name");
		}
	}

	iferr return;

	BeginBlock(TOKEN_FOR);
	
	// STEP can be only used if we have the loop variable defined
	// Default step is 1.

	if (var != NULL) {
		if (NextIs(TOKEN_STEP)) {
			ParseExpression(max);
			step = STACK[0];
		} else {
			step = ONE;
		}
	}

	// WHERE can be used only if there was FOR

	if (NextIs(TOKEN_WHERE)) {
		if (var != NULL) {
			G_BLOCK->f_label = G_BLOCK->loop_label;
			GenBegin();
			where_cond = ParseCondExpression();
//			ParseCondition();
			if (G_BLOCK->t_label != NULL) {
				GenLabel(G_BLOCK->t_label);
				G_BLOCK->t_label = NULL;
			}

			where_cond_code = GenEnd();
			G_BLOCK->f_label = NULL;
			iferr goto done;
		} else {
			SyntaxError("Where can only be used together with loop variable");
		}
	}

	if (NextIs(TOKEN_UNTIL)) {
		G_BLOCK->not = true;
		goto cond_parse;
	} else if (NextIs(TOKEN_WHILE)) {
cond_parse:

		GenBegin();
		GenLine();
		cond = ParseCondExpression();
		if (!G_BLOCK->not) cond = Not(cond);

		if (G_BLOCK->f_label == NULL) {
			G_BLOCK->f_label = VarNewTmpLabel();
		}

		GenInternal(INSTR_IF, NULL, cond, G_BLOCK->f_label);

//		if (G_BLOCK->t_label != NULL) {
//			GenLabel(G_BLOCK->t_label);
//		}
		cond_code = GenEnd();

		iferr goto done;
	}

	/*
	Case when UNTIL is not used:
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

	/*
	Case, when UNTIL is used:
		<i> = min
		goto loop_label		; only if there is condition (otherwise we expect at least one occurence)
	body_label@
		if !<condition> goto f_label
	;WHERE
		<where_condition>  f_label = loop_label | t_label
	where_t_label@
		<body>
	t_label@
		add <i>,<i>,1
	loop_label@
		ifgt f_label,<i>,max
	f_label@
	*/

	// Parse body

	GenBegin();
	GenLine();
	ParseCommandBlock();
	body = GenEnd();
	iferr return;

	// Variable initialization

	if (var != NULL) {
		GenInternal(INSTR_LET, idx, min, NULL);
	}

	if (cond != NULL || (min != NULL && !CellIsIntConst(min)) || (max != NULL && !CellIsIntConst(max))) {
		cond_first = true;
	}

	// Loop with condition, but without variable
	if (cond_first) {
		GenGoto(G_BLOCK->loop_label);
	}

	// Body consists of where_cond & body

	G_BLOCK->body_label = body_label;
	GenLabel(G_BLOCK->body_label);

	if (cond != NULL && var != NULL) {
		GenBlock(cond_code);
	}

	if (where_cond != NULL) {
		GenBlock(where_cond_code);
	}

	GenBlock(body);

	if (where_cond != NULL) {
		GenLabel(G_BLOCK->loop_label);
	}

	// Insert condition (we do not have index variable)
	if (cond != NULL && var == NULL) {
		if (where_cond == NULL) {
			GenLabel(G_BLOCK->loop_label);
		}
		GenBlock(cond_code);
//		if (var == NULL) {
			GenGoto(G_BLOCK->body_label);
//		}
	}

	if (var != NULL) {

		// Add the step to variable
		GenInternal(INSTR_LET, idx, NewOp(INSTR_ADD, idx, step), NULL);

		// 1. If max equals to byte limit (0xff, 0xffff, 0xffffff, ...), only overflow test is enough
		//    We must constant adding by one, as that would be translated to increment, which is not guaranteed
		//    to set overflow flag.

		if (max->mode == INSTR_INT) {
			n = &max->n;
			IntInit(&nmask, 0xff);
			while(IntHigher(n, &nmask)) {
				IntMulN(&nmask, 256);
				IntOrN(&nmask, 0xff);
//				nmask = (nmask << 8) | 0xff;
			}

			if (IntEq(n, &nmask) && (step->mode != INSTR_INT || IntHigherN(&step->n, 255))) {
				GenInternal(INSTR_NOVERFLOW, G_BLOCK->body_label, NULL, NULL);
				goto var_done;
			} else if (step->mode == INSTR_INT) {

				// 2. Min,max,step are constants, in such case we may use IFNE and calculate correct stop value
				if (min->mode == INSTR_INT) {

					//n = min->n + ((max->n - min->n) / step->n + 1) * step->n;
					//n = n & nmask;

					IntSub(&t1, &max->n, &min->n);
					IntDiv(&t3, &t1, &step->n);
					IntAddN(&t3, 1);
					IntFree(&t1);
					IntMul(&t1, &t3, &step->n);
					IntFree(&t3);
					IntAdd(&t2, &min->n, &t1);
					IntFree(&t1);
					IntAnd(&t3, &t2, &nmask);
					IntFree(&t2);
//					IntModify(&idx->type->range.max, &t3);		// set the computed limit value as max of the index variable
					max = IntCell(&t3);
					IntFree(&t3);
					idx->type = NewRange(min, max);
					GenInternal(INSTR_IF, NULL, NewOp(INSTR_NE, idx, max), G_BLOCK->body_label);	//TODO: Overflow
					goto var_done;
				// 3. max & step are constant, we may detect, that overflow will not occur
				} else {
					IntSub(&t1, &nmask, &max->n);
					higher = IntHigherEq(&t1, &step->n);
					IntFree(&t1);
					if (higher) goto no_overflow;
				}
			}
		}

		// Alloc f_label if necessary
		if (G_BLOCK->f_label == NULL) {
			G_BLOCK->f_label = VarNewTmpLabel();
		}

		// If step is 1, it is not necessary to test the overflow
		if (!IsEqual(step, ONE)) {
			GenInternal(INSTR_OVERFLOW, G_BLOCK->f_label, NULL, NULL);
		}
no_overflow:

		if (cond_first) {
			GenLabel(G_BLOCK->loop_label);
		}
		// We use > comparison as in the case step is <> 1, it may step over the limit without touching it.
		// Also user may modify the index variable (although this should be probably discouraged when for is used).

		GenInternal(INSTR_IF, NULL, NewOp(INSTR_GE, max, idx), G_BLOCK->body_label);	//TODO: Overflow
//		GenInternal(INSTR_GE, G_BLOCK->body_label, max, idx);
	}
var_done:

	if (G_BLOCK->f_label != NULL) {
		GenLabel(G_BLOCK->f_label);
	}
done:
	EndBlock();
	ExitSubscope();
}

Var * ParseFile()
{
	Var * item = NULL;
	Bool block = false;
	FILE * f;
	char path[MAX_PATH_LEN];

	if (TOK == TOKEN_OPEN_P) {
		EnterBlock();
		block = true;
	}

	if (LexString(NAME2)) {
		strcpy(path, FILE_DIR);
		strcat(path, NAME2);
		f = fopen(path, "rb");
		if (f != NULL) {
			fclose(f);
			item = TextCell(path);
		} else {
			SyntaxError("File not found");
		}
	} else {
		SyntaxError("expected string specifying file name");
	}

	ifok {
		if (block) {
			ExpectToken(TOKEN_BLOCK_END);
		} else {
//			NextToken();
		}
	}
	return item;
}

UInt32 ParseArrayConst(Type * type, Bool nested, Type ** p_item_type)
/*
Purpose:
	Parse array constant.
Arguments:
	type		Type of element, that should be parsed.
*/
{
	UInt32 i, rep;
	Var * item;
	Type * item_type;
	UInt16 bookmark;
	UInt32 item_size;
	Bool inexact_element;
	BigInt * bi;

	Var * element_type = NULL;

	i = 0;
	inexact_element = false;
	if (p_item_type == NULL) inexact_element = true;

	item_type = type->element;

	// In case of array of arrays simple string is understood as the whole array
	if (nested) {
		if (LexString(NAME2)) {
			item = TextCell(NAME2);
			item_size = StrLen(NAME2);
			Gen(INSTR_DATA, NULL, item, NULL);
			return item_size;
		}
	}

 	EnterBlock();

	EXP_IS_REF = true;

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
			ifok {
				Gen(INSTR_FILE, NULL, item, NULL);
				inexact_element = true;
				continue;
			} else {
				break;
			}
		}

		//TODO: Here can be either the type or integer constant or address
		bookmark = SetBookmark();
		if (LexString(NAME2)) {
			item = TextCell(NAME2);
			item_size = StrLen(NAME2);
			inexact_element = true;
		} else {
			ParseExpressionType(item_type);
			item = STACK[0];
			item_size = 1;
		}

		rep = 1;
		if (NextIs(TOKEN_TIMES)) {
			bi = IntFromCell(item);
			if (bi != NULL) {
				//TODO: Check, that repeat is not too big
				rep = IntN(bi);
			} else {
				SyntaxError("repeat must be defined using integer");
				break;
			}
			bookmark = SetBookmark();
			ParseExpressionType(item_type);
			item = STACK[0];
		}

		if (item->mode == INSTR_TEXT) {
			//TODO: Convert string - possibly using translating array
		} else if (item->mode == INSTR_INT) {
			if (item->type->variant != TYPE_ARRAY) {
				if (!IsSubset(item, item_type)) {
					LogicError("value does not fit into array", bookmark);
					continue;
				}

				if (!inexact_element) {
					element_type = Union(element_type, item);
				}

			}
		}

		while(rep--) {
			// Generate reference to variable
			if (item->type->variant == TYPE_ARRAY) {
				Gen(INSTR_PTR, NULL, item, NULL);
				item_size = TypeAdrSize();		// address has several bytes
			} else {
				Gen(INSTR_DATA, NULL, item, NULL);
			}
			i += item_size;
		}
	}

	if (!inexact_element) {
		type->element = element_type;
	}

	EXP_IS_REF = false;
	return i;
}


/*************************************************************************

   Parsing Arrays

**************************************************************************/

//Constant array is stored as array of vars.
//
// Array of array is generated like this:
//   index:array of adr of item		// implemented based on platform
//   size:array of byte

//$A


typedef struct ArrParserTag ArrParser;

struct ArrParserTag {
	Var * arr;
	Type * elem_type;
	Var * sizes, * index_lo, * index_hi;
	UInt32 min_size, max_size;
};

void ArrParserInit(ArrParser * apar, Var * arr)
{
//	InstrBlock * sizes_i, * index_lo_i, * index_hi_i;
	Var * arr_const_type;

	apar->arr = arr;
	apar->elem_type = arr->type->element;

	if (apar->elem_type->variant == TYPE_ARRAY) {
//		index_lo_i = InstrBlockAlloc();
//		index_hi_i = InstrBlockAlloc();

//		apar->sizes = VarAllocScope(arr, INSTR_ARRAY, "size");

		arr_const_type = NewArray(NULL, InstrBlockAlloc());
		apar->sizes = NewVar(arr, "size", arr_const_type);

		arr_const_type = NewArray(TypeByte(), InstrBlockAlloc());
		apar->index_lo = NewVar(arr, "index_lo", arr_const_type);

		arr_const_type = NewArray(TypeByte(), InstrBlockAlloc());
		apar->index_hi = NewVar(arr, "index_hi", arr_const_type);

//		apar->sizes->instr = sizes_i;

//		apar->index_lo = VarAllocScope(arr, INSTR_ARRAY, "index_lo");
//		apar->index_lo->instr = index_lo_i;

//		apar->index_hi = VarAllocScope(arr, INSTR_ARRAY, "index_hi");
//		apar->index_hi->instr = index_hi_i;
	} else {
		arr->instr = InstrBlockAlloc();
	}

	apar->min_size = 0xffffff;
	apar->max_size = 0;
}

void ArrParserNext(ArrParser * apar, UInt32 idx)
{
	Var * item;
	UInt32 size;
	BigInt isize;

	if (apar->elem_type->variant == TYPE_ARRAY) {
		item = NewVarWithIndex(apar->arr, "e", idx, NULL);
		GenBegin();

		size = ParseArrayConst(apar->elem_type, true, NULL);

		if (size > apar->max_size) apar->max_size = size;
		if (size < apar->min_size) apar->min_size = size;
		item->instr = GenEnd();

		//TODO: Make min according to array min

		IntInit(&isize, size);
		item->type = TypeArray(NewRangeInt(Int0(), &isize), apar->elem_type);
		IntFree(&isize);

		InstrInsertRule(apar->sizes->instr, NULL, INSTR_DATA, NULL, IntCellN(size), NULL);
		InstrInsertRule(apar->index_lo->instr, NULL, INSTR_PTR, NULL, VarNewByteElement(item, ZERO), NULL);
		InstrInsertRule(apar->index_hi->instr, NULL, INSTR_PTR, NULL, VarNewByteElement(item, ONE), NULL);
	} else {
		ParseExpressionType(apar->elem_type);
		ifok {
			InstrInsertRule(apar->arr->instr, NULL, INSTR_DATA, NULL, STACK[0], NULL);
		}
	}
}

void ArrParserFinish(ArrParser * apar, UInt32 idx)
{
	Type * idx_type;

	if (apar->elem_type->variant == TYPE_ARRAY) {
		idx_type = NewRangeIntN(0, idx);
		apar->sizes->type = TypeArray(idx_type, NewRangeIntN(apar->min_size, apar->max_size));

		//TODO: Depending on address space
		apar->index_lo->type = TypeArray(idx_type, TypeByte());
		apar->index_hi->type = apar->index_lo->type;
	}
}

Var * ParseArrayC(Type * type)
{
	UInt32 size;
	Type * elem_type;
	UInt32 idx;
	ArrParser apar;
	Bool flexible;		// is index flexible
	Var * var;

	var = NewArray(type, NULL);

//	type = var->type;
	flexible = type->index->range.flexible;

	elem_type = type->element;

	if (elem_type->variant == TYPE_ARRAY) {

		ArrParserInit(&apar, var);

		EnterBlock();
		idx = 1;
		while(!NextIs(TOKEN_BLOCK_END)) {

			// Skip any EOLs (we may use them to separate subarrays?)
			if (NextIs(TOKEN_EOL)) continue;

			ArrParserNext(&apar, idx);

			idx++;
		}

		ArrParserFinish(&apar, idx);


	} else {
		GenBegin();
		// Make the array parser generate more specific type
		size = ParseArrayConst(type, false, &type->element);
		var->instr = GenEnd();
	}

	if (flexible) {
		type->index->range.max = size-1;
	}

	ASSERT(var->mode == INSTR_ARRAY);
	return var;
}

void ParseEnumItems(Type * type, UInt16 column_count)
{
	BigInt last_n;
	Bool id_required;
//	Var * var;
	Var * local, * first_local;
	UInt16 i, row;
	ArrParser * apar;
	Var * name;

	IntInit(&last_n, -1);

	first_local = type->subscope;
	
	// Initialize array parsers
	apar = (ArrParser*)MemAllocEmpty(sizeof(ArrParser) * column_count);
	local = first_local;
	for(i=0; i< column_count; i++) {
		ArrParserInit(&apar[i], local);
		local = local->next_in_scope;
	}
	row = 1;
	EnterBlock();
	while (OK && !NextIs(TOKEN_BLOCK_END)) {
		while(NextIs(TOKEN_EOL));

		// Parse item identifier
		if (LexId(NAME2)) {
			name = NewVar(NO_SCOPE, NAME2, NULL);
			if (NextIs(TOKEN_COLON)) {
			} else {

			}

			IntAddN(&last_n, 1);
			name->type = IntCell(&last_n);

			TypeAddConst(type, name);

			local = first_local;
			for(i=0; TOK && i< column_count; i++) {
				ArrParserNext(&apar[i], row);
				local = local->next_in_scope;
				NextIs(TOKEN_COMMA);
			}

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
		row++;
	}
}

Var * VarRangeSize(BigInt * min, BigInt * max)
{
	Var * var;
	BigInt bi;
	IntRangeSize(&bi, min, max);
	var = IntCell(&bi);
	IntFree(&bi);
	return var;
}

void ArraySize(Type * type, Var ** p_dim1, Var ** p_dim2)
{
	UInt32 size;
	Type * dim1, * dim2;


	*p_dim1 = NULL;
	*p_dim2 = NULL;
	if (type->variant == TYPE_ARRAY) {
		dim1 = type->index;
		dim2 = NULL;
		if (dim1->variant == TYPE_TUPLE) {
			dim2 = dim1->right;
			dim1 = dim1->left;
		}
		*p_dim1 = VarRangeSize(&dim1->range.min, &dim1->range.max);

		if (dim2 != NULL) {
			*p_dim2 = VarRangeSize(&dim2->range.min, &dim2->range.max);

		// Array of array
		} else {
			if (type->element != NULL && type->element->variant == TYPE_ARRAY) {
				dim1 = type->element->index;
				*p_dim2 = VarRangeSize(&dim1->range.min, &dim1->range.max);
			}
		}
	} else if (type->variant == TYPE_STRUCT) {
		size = TypeSize(type);
		*p_dim1 = IntCellN(size);
	}
}

Bool VarIsImplemented(Var * var)
{
	Rule * rule;
	Instr i;
	TypeVariant v;

	if (CellIsConst(var)) return true;

	// Macros and procedures are considered implemented

	if (var->mode == INSTR_FN || var->mode == INSTR_FN_TYPE) return true;

	// If the variable has no type, it will not be used in instruction,
	// so it is considered implemented.

	if (var->mode == INSTR_ANY) return true;

	v = var->type->variant;

	// Type declarations do not need to be implemented
	// (we think of them as being implemented by compiler).

	if (var->mode == INSTR_TYPE) return true;

	// Macros and procedures are considered imp

	if (v == TYPE_LABEL || v == TYPE_SCOPE || v == TYPE_TYPE) return true;

	// Register variables are considered implemented.
//	if (var->adr != NULL && var->adr->scope == CPU_SCOPE) return true;


	//TODO: We do not want to use array size
	ArraySize(var->type, &i.arg1, &i.arg2);
	rule = InstrRule2(INSTR_ALLOC, var, i.arg1, i.arg2);
	if (rule != NULL) return true;

	rule = TranslateRule(INSTR_DECL, NULL, var, NULL);
	if (rule != NULL) {
		InstrExecute(rule->to);
		return true;
	}

	return false;
}


Var * ParseAdr()
/*
Purpose:
	Parse address specified after the @ symbol in variable definition.
*/
{
	UInt16 cnt;

	Var * adr, * tuple, * item;

	NextToken();

	// (var,var,...)   tuple
	// int (concrete address)
	// variable (some variable)

	adr = NULL; tuple = NULL;

	//@
	if (TOK == TOKEN_OPEN_P) {
		EnterBlock();
		cnt = 0;
		do {
			item = ParseVariable();
			if (!TOK) break;
			cnt++;
			BufPush(item);
		} while(NextIs(TOKEN_COMMA));

		if (TOK && !NextIs(TOKEN_BLOCK_END)) {
			SyntaxError("expected closing parenthesis");
		}

		adr = NULL;
		while(cnt > 0) {
			TOP--;
			adr = NewTuple(STACK[TOP], adr);
			cnt--;
		}

	} else if (LexInt(&adr)) {

	} else if (TOK == TOKEN_ID) {
		adr = VarFind2(NAME);
		if (adr == NULL) {
			SyntaxError("undefined variable [$] used as address");
			NextToken();
		} else {
			NextToken();
dot:
			if (NextIs(TOKEN_DOT)) {
				if (LexId(NAME2)) {
					adr = VarFind(adr, NAME2);				
					goto dot;
				} else {
					SyntaxError("Expected variable name");
				}
			}

			if (adr->mode == INSTR_SCOPE) {
				SyntaxError("scope can not be used as address");
			} 
			// name(slice)
			if (TOK == TOKEN_OPEN_P) {
				adr = ParseArrayElement(adr);
			}
		}
	} else {
		SyntaxError("expected integer or register set name");
	}
	return adr;
}

void InsertRegisterArgumentSpill(Var * proc, Var * args, VarSubmode submode, Instr * i)
{
	Var * en, * arg, * tmp;

	FOR_EACH_ITEM(en, arg, args)
		if (VarIsReg(arg)) {
			tmp = NewVarInScope(proc, arg->type);
			ProcReplaceVar(proc, arg, tmp);

			if (submode == SUBMODE_ARG_IN) {
				InstrInsert(proc->instr, i, INSTR_LET, tmp, arg, NULL);
			} else {
				InstrInsert(proc->instr, i, INSTR_LET, arg, tmp, NULL);
			}
		}
	NEXT_ITEM(en, arg)
}

Bool CodeHasSideEffects(Var * scope, InstrBlock * code)
/*
Purpose:
	Return true, if the code has some side effects.
*/
{
	Instr * i;
	InstrBlock * blk;
	Var * var;

	for(blk = code; blk != NULL; blk = blk->next) {
		for(i = blk->first; i != NULL; i = i->next) {
			if (i->op == INSTR_CALL) {
				if (FlagOn(var->submode, SUBMODE_OUT)) return true;
			} else if (i->op == INSTR_LINE) {
			} else {
				var = i->result;
				if (var != NULL) {
					if (OutVar(var) || !VarIsLocal(var, scope)) return true;
				}
			}
		}
	}
	return false;
}

void ParseMacroBody(Var * proc)
{
	Var * lbl;
	Var * scope;

	if (proc->instr != NULL) {
		SyntaxError("Macro has already been defined");
		return;
	}

	scope = InScope(proc);
	GenBegin();
	ParseCommandBlock();

	// If there is a return statement in procedure, special label "_exit" is defined.

	lbl = VarFind(SCOPE, "_exit");
	GenLabel(lbl);

	proc->instr = GenEnd();
	if (CodeHasSideEffects(proc, proc->instr)) {
		SetFlagOn(proc->submode, SUBMODE_OUT);
	}
	ReturnScope(scope);

	// *** Register Arguments (2)
	// As the first thing in a procedure, we must spill all arguments that are passed in registers
	// to local variables. 
	// Otherwise some operations may trash the contents of an argument and it's value would become unavailable.
	// In the body of the procedure, we must use these local variables instead of register arguments.
	// Optimizer will later remove unnecessary spills.

	InsertRegisterArgumentSpill(proc, ArgType(proc->type->type), SUBMODE_ARG_IN, proc->type->instr->first);

	// *** Register Arguments (3)
	// At the end of a procedure, we load all values of output register arguments to appropriate registers.
	// To that moment, local variables are used to keep the values of output arguments, so we have
	// the registers available for use in the procedure body.

	InsertRegisterArgumentSpill(proc, ResultType(proc->type->type), SUBMODE_ARG_OUT, NULL);

}

void VarArrayOfStructToStructOfArrays(Var * var)
/*
Purpose:
	Change the type of specified variable from array of structures to structure of arrays.

	For every element in the structure, create array of the type of the element.
	For example:

	type point:struct
	   x:0..320
	   y:0..240

	points:array(idx) of point

	Is converted to:

	points:
	     x:array(idx) of 0..320
		 y:array(idx) of 0..240
*/
{
	Var * structure, * elmt, * sub;
	Type * idx, * subtype;

	idx = var->type->index;
	structure = var->type->element;

	FOR_EACH_LOCAL(structure, elmt)
		subtype = TypeArray(idx, elmt->type);
		sub = NewVar(var, elmt->name, subtype);

		if (!VarIsImplemented(sub)) {
			ErrArg(structure);
			ErrArg(elmt);
			SyntaxError("Platform does not support array of [B] because of structure member [A].");
		}
	NEXT_LOCAL
}

void ParseElements(Var * var)
{
	EnterBlockWithStop(TOKEN_EQUAL);
	while(!NextIs(TOKEN_BLOCK_END)) {
		// Skip any EOLs (we may use them to separate subarrays?)
		if (NextIs(TOKEN_EOL)) continue;
		ParseAssign(INSTR_VAR, SUBMODE_EMPTY, NULL);
		while(NextIs(TOKEN_EOL));
	}

}

#define LOCAL_SCOPE 0
#define GLOBAL_SCOPE 1

void ParseAssign(InstrOp mode, VarSubmode submode, Type * to_type)
/*
Purpose:
	Parse variable assignment/declaration.
	Lexer contains name of the first defined variable.
*/
{
	Bool is_assign, existed;
	UInt16 cnt, j, stack;
	Var * var,  * item, * adr, * scope, * idx, * min, * max;
	Var * vars[MAX_VARS_COMMA_SEPARATED];
	Type * type;
	UInt16 bookmark;
	Bool global_scope;
	UInt8 arg_no;
	BigInt ib;

	type = ANY;
	is_assign = false;
	existed   = true;
	global_scope = false;
	scope = NULL;

	// Force use of current scope
	// For example .X will try to find X in current scope, not in any other parent scope
	if (!Spaces()) {
		if (NextIs(TOKEN_DOT)) {
			scope = SCOPE;

		// It is necessary to use @x to address global variable.
		} else if (NextIs(TOKEN_ADR)) {
			global_scope = true;
		}
	}

	if (TOK != TOKEN_ID && TOK != TOKEN_PERCENT) {
		SyntaxError("expected identifier");
		return;
	}

	bookmark = SetBookmark();

	// Comma separated list of identifiers
	cnt = 0;
	do {
retry:
		if ((arg_no = ParseArgNo2()) != 0) {
			var = VarRuleArg(arg_no-1);
		} else if (!ParseArg(&var)) {
			// Either find an existing variable or create new one
			if (to_type == NULL) {
				if (scope == NULL) {
					if (EXP_EXTRA_SCOPE != NULL) {
						var = VarFind(EXP_EXTRA_SCOPE, NAME);
					} 
					if (var == NULL) {					
						var = VarFind2(NAME);
					}
				} else {
					var = VarFind(scope, NAME);
				}
			}
		}

		//TODO: Type with same name already exists
		if (var == NULL) {			

			// We need to prevent the variable from finding itself in case it has same name as type from outer scope
			// This is done by assigning it mode INSTR_VOID (search ignores such variables).
			// Real mode is assigned when the variable type is parsed.

			var = NewVar(scope, NAME, NULL);			
			CellSetLocation(var, SRC_FILE, LINE_NO, TOKEN_POS);
			if (ParsingSystem()) submode |= SUBMODE_SYSTEM;
			existed = false;
			var->submode = submode;
			SetFlagOn(var->submode, SUBMODE_FRESH);
		} else {
			if (var->mode == INSTR_SCOPE) {
				NextToken();
				scope = var;

				if (NextIs(TOKEN_DOT)) goto retry;
				goto no_dot;
			}
		}

		//?
//		NextToken();
		item = ParseSpecialArrays(var);
		if (item != NULL) {
			var = item;
			goto parsed;
		}

		NextToken();
	// Parse array and struct indices
no_dot:
		ErrArg(var);

		//===== Array index like ARR(x, y)

		if (mode != INSTR_NAME && mode != INSTR_TYPE && !Spaces()) {
			if (TOK == TOKEN_OPEN_P || TOK == TOKEN_HASH) {
				if (var->mode != INSTR_VOID) {
					var = ParseArrayElement(var);
				} else {
					SyntaxErrorBmk("Array variable [A] is not declared", bookmark);
				}
				ifok goto no_dot;
			} else if (TOK == TOKEN_DOT) {
				var = ParseStructElement(var);
				ifok goto no_dot;
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
				adr = ParseAdr();
				is_assign = true;
				if (var->adr == NULL) {
					var->adr = adr;
				} else {
					SyntaxError("Address of variable [A] has been already defined.");
				}
			}
		}
parsed:
		vars[cnt] = var;
		cnt++;
		// this is to check if there is not too many expressions
		if (cnt>=MAX_VARS_COMMA_SEPARATED) {
			SyntaxError("too many comma separated identifiers");
		}
	} while (NextIs(TOKEN_COMMA));

	// This is definitely a type!!!
	// Assignment does not allow type specified.

	if (NextIs(TOKEN_COLON)) {

		for(j = 0; j<cnt; j++) {
			var = vars[j];
			// We have found the variable and we have type explicitly defined, but not in current scope, so we may create it in current scope
			//TODO: We may need to copy more than one level of variable

			if (FlagOff(var->submode, SUBMODE_FRESH) && var->scope != SCOPE) {
				var = NewVar(scope, var->name, NULL);
				var->mode = INSTR_VOID;
				var->line_no = LINE_NO;
	//			var->line_pos = TOKEN_POS;
				var->file    = SRC_FILE;
				if (ParsingSystem()) submode |= SUBMODE_SYSTEM;
				existed = false;
				var->submode = submode;
				vars[j] = var;
			}
		}


		// Scope
		if (NextIs(TOKEN_SCOPE)) {
			mode = INSTR_SCOPE;
			type = TypeScope();
			is_assign = true;

			// If this is definition of CPU, immediately remember it
//			if (StrEqual(var->name, "CPU")) {
//				CPU->SCOPE = var;
//			}

		} else {
			is_assign = true;

			// Parsing may create new constants, arguments etc. so we must enter subscope, to assign the
			// type elements to this variable
			scope = InScope(vars[0]);
			bookmark = SetBookmark();
			type = ParseType2(mode);

			ifok {
				// CPU is defined as object with CPU type
				if (CPU_TYPE != NULL && type == CPU_TYPE) {
					CPU->SCOPE = var;
				}

				if (type == NULL || type->mode != INSTR_TYPE || type->variant == TYPE_SCOPE) {
					if (type == NULL || (type->mode != INSTR_FN_TYPE && type->mode != INSTR_FN)) {
						ParseElements(vars[0]);
					}
	//				type = TypeScope();
				}
				if (type == NULL) type = TypeScope();
			}

			ReturnScope(scope);
		}
	}

	// Set the parsed type to all new variables (we do this, even if a type was not parsed)
	if (!TOK) return;

	for(j = 0; j<cnt; j++) {
		var = vars[j];

		// If scope has not been explicitly defined, use current scope

		if (var->scope == NULL) {
			var->scope = SCOPE;
		}

		if (FlagOn(var->submode, SUBMODE_FRESH)) {

			var->mode = mode;
			SetFlagOff(var->submode, SUBMODE_FRESH);
			if (type->mode != INSTR_ANY) {
				var->type = type;
				SetFlagOn(var->submode, SUBMODE_USER_DEFINED);

				// Definition of named constant assigned to type (name:xxx = 34)
				if (var->mode == INSTR_NAME && FlagOff(submode, SUBMODE_PARAM)) {
					if (var->type->variant != TYPE_UNDEFINED) {
						TypeAddConst(var->type, var);
					}
				} else {
					if (!VarIsImplemented(var)) {
						// If this is array of structure, convert the variable to structure of arrays
						if (var->type->variant == TYPE_ARRAY && var->type->element->variant == TYPE_STRUCT) {
							VarArrayOfStructToStructOfArrays(var);
							//TODO: Error if subelement not supported
						} else {
							if (*PLATFORM != 0) {
//								LogicError("Type not supported by platform", bookmark);
							} else {
//								SyntaxError("Platform has not been specified");
							}
						}
					}
				}
			// If type has not been defined, but this is alias, use type of the aliased variable
			} else if (var->adr != NULL) {
				adr = var->adr;

				// We are parsing procedure or macro argument
				if (adr->mode == INSTR_VAR) {
					var->type = adr->type;
				} else if (adr->mode == INSTR_TUPLE) {
					is_assign = true;
				} else if (adr->mode == INSTR_ELEMENT) {
					is_assign = true;
					var->type = adr->type;
					idx = adr->var;

					// For array ranges, define type as array(0..<range_size>) of <array_element>

					if (idx->mode == INSTR_RANGE) {
						min = idx->adr; max = idx->var;
						if (min->mode == INSTR_INT && max->mode == INSTR_INT) {
							IntSub(&ib, &max->n, &min->n);
							var->type = TypeArray(NewRangeInt(Int0(), &ib), adr->adr->type->element);
							IntFree(&ib);
						} else {
							SyntaxError("Address can not use variable slices");
						}
					}

				}
			}
		} else {
			if (type->mode != INSTR_ANY) {
				ErrArg(var);
				SyntaxErrorBmk("Variable [A] already defined", bookmark);
			}
		}
	}

	// If there is assignment part, generate instruction
	// (it may be pruned later, when it is decided the variable is not used, or is constant)

	if (NextIs(TOKEN_EQUAL)) {

		is_assign = true;
		stack = 0; TOP = 0;

		for(j = 0; j<cnt; j++) {
			var = vars[j];
			type = var->type;

			ErrArgClear();
			ErrArg(var);

			if (CellIsConst(var) && existed) {
				SyntaxError("Assigning value to constant [A].");
				continue;
			} else if (var->mode == INSTR_TYPE) {
				SyntaxError("Assigning value to type [A].");
				continue;
			} else if (var->mode == INSTR_VAR && FlagOn(var->submode, SUBMODE_IN) && FlagOff(var->submode, SUBMODE_OUT)) {
				SyntaxError("Assigning value to read only register [A].");
				continue;
			}

			// Procedure or macro is defined using parsing code
			if (type->mode == INSTR_FN_TYPE) {
				// if (macro)
				ParseMacroBody(var);
				// else if
				//ParseProcBody(var);
				// endif
//			} else if (typev == TYPE_SCOPE) {
//				scope = InScope(var);
//				ParseCommandBlock();
//				ReturnScope(scope);
//			} else if (type->mode == INSTR_TYPE) {
//				scope = InScope(var);
//				var->type_value = ParseType2(INSTR_VAR);
//				ReturnScope(scope);
			} else {

				// Initialization of array
				// Array is initialized as list of constants.

				if (type->mode == INSTR_ARRAY_TYPE) {
					var->var = ParseArrayC(var->type);			// Warning: The type variable may get modified according to match parsed constant

				// Normal assignment
				} else {

					if (TOK == TOKEN_STRING) {
						// We may assign strings to array references
						if (var->mode == INSTR_ELEMENT || var->mode == INSTR_VAR) {
							if (MACRO_FORMAT != NULL) {
								// Call format routine (set address argument)
								GenBegin();
								GenMacro(MACRO_FORMAT, &var);
								ParseString(GenEnd(), STR_NO_EOL);
							} else {
								SyntaxError("printing into array not supported by the platform");
							}
						} else if (var->mode == INSTR_NAME) {
							var->mode = INSTR_TEXT;
							VarInitStr(var, NAME);
							NextToken();
						} else {
							SyntaxError("string may be assigned only to variable or to constant");
						}
					} else {

						if (j == 0 || NextIs(TOKEN_COMMA)) {
							bookmark = SetBookmark();
							ExpectExpression(var);
						}

						ifok {

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

								// Default value for procedure argument or parameter
								if (VarIsParam(var) || FlagOn(submode, SUBMODE_ARG_IN) || FlagOn(submode, SUBMODE_ARG_OUT)) {		// mode == INSTR_ARG
									var->var = item;
/*								} else if (var->mode == INSTR_VAR) {
									VarLet(var, item);
									// Set the type based on the constant
									if (typev == TYPE_UNDEFINED) {
										var->type = var;
									}
*/
								} else {
									// If the result is stored into temporary variable, we may direct the result directly to the assigned variable.
									// This can be done only if there is just one result.
									// For multiple results, we can not use this optimization, as it is not last instruction, what generated the result.
									if (TOP == 1 && VarIsTmp(item)) {
										GenLastResult(var, item);
									} else {
										GenLet(var, item);
									}
								}
							}
						}
					}
				}
			}
		}
	} else {
		// No equal sign, this must be call to procedure or macro (without return arguments used)
		var = vars[0];
		if (existed && !is_assign && var != NULL) {
			if (var->type->mode == INSTR_FN) {
				is_assign = true;
				if (IsMacro(var->type)) {
					ParseMacro(var);
				} else {
					ParseCall(var);
				}
			} else {
				SyntaxError("Expected function or macro name.");
			}
		}
	}

	// *** Module parameters (3)
	// When the module parameter declaration has been parsed, we try to find a value with same name specified as parameter value for this module
	// in use directive.
	// If the value has been found, it's value is set to parameter instead of value possibly parsed in declaration (parameter default value).

	for(j = 0; j<cnt; j++) {
		var = vars[j];
		if (VarIsParam(var)) {
			item = VarFindScope2(SRC_FILE, var->name);
			if (item != NULL) {
				VarLet(var, item);
			} else {
				if (var->var == NULL) {
					SyntaxError("Value of parameter [A] has not been specified.");
				}
			}
		}
	}

	ErrArgClear();

	ifok {
		if (!is_assign) {
			if (FlagOff(submode, SUBMODE_ARG_IN | SUBMODE_ARG_OUT)) {
				SyntaxError("expects : or =");
			}
		}
	}
}

Var * ParseInstrArg()
{
	Var * var = NULL;
	EXP_EXTRA_SCOPE = CPU->SCOPE;
	ParseExpression(NULL);
	EXP_EXTRA_SCOPE = NULL;
	ifok {
		var = STACK[0];
	}
	return var;

}

Var * ParseInstrLabel()
{
	Var * label = NULL;
	UInt8 arg_no;

	label = ParseVariable2();

	if (label != NULL) {

		if (FlagOn(label->submode, SUBMODE_FRESH)) {
			FlagOff(label->submode, SUBMODE_FRESH);
			label->type = &TLBL;
		}
	} else if (arg_no = ParseArgNo()) {
		label = VarRuleArg(arg_no-1);
//		NextIs(TOKEN_COMMA);
	} else {
		SyntaxError("expected label identifier");
	}
	return label;
}

RuleArg * ParseRuleArg2();

void ParseInstr()
/*
Syntax: <instr_name> <result> <arg1> <arg2>
*/
{
	Var * arg[3];
	UInt8 n;
//	UInt8 arg_no;
	InstrOp op;
//	Var * label, * scope;
	char inc_path[MAX_PATH_LEN];
	Var * inop;
	Type * type;
	Bool  had_comma;

	op = INSTR_VOID;

	EXP_INSTR = true;
	had_comma = false;

	if (NextIs(TOKEN_IF)) {
		arg[1] = ParseInstrArg();
		if (ParseRelOp(&op)) {
			arg[2] = ParseInstrArg();
			if (NextIs(TOKEN_GOTO)) {
				arg[0] = ParseInstrLabel();
			} else {
				SyntaxError("Expected goto");
			}
		} else {
			SyntaxError("Expected relational operator");
		}
	} else {
		if (LexId(NAME2)) {

			// This is instruction
			op = InstrFind(NAME2);
			if (op == INSTR_NULL) {
				inop = VarFind(CPU->SCOPE, NAME2);
				if (inop == NULL) {
					SyntaxError("Unknown instruction or macro [$]");
				} else {
					if (IsMacro(inop->type)) {
						EXP_EXTRA_SCOPE = CPU->SCOPE;
						ParseMacro(inop);
						EXP_EXTRA_SCOPE = NULL;
						return;
					} else {
						ErrArg(inop);
						SyntaxError("[A] must be instruction or macro");
					}
				}
			}
		} else {
			SyntaxError("Expected instruction or macro name or IF");
		}

		arg[0] = arg[1] = arg[2] = NULL;

		ifok {

			n = 0;
		// Include has special handling
		// We need to make the file relative to current file dir and check the existence of the file

			if (op == INSTR_DECL) {
				arg[1] = ParseInstrArg();
				NextIs(TOKEN_COLON);
				type = ParseTypeInline();
				arg[1]->type = type;
				n=3;
			} else if (op == INSTR_INCLUDE || op == INSTR_FILE) {

				if (LexString(NAME2)) {
					PathMerge(inc_path, FILE_DIR, NAME2);
					arg[n++] = TextCell(inc_path);
				} else {
					SyntaxError("expected name of include file");
				}

			// Branching instruction has label as first argument
			// 
			} else if (op == INSTR_CALL) {
				arg[1] = ParseInstrLabel();
				ifok {
					n++;
					goto next_arg;
				}
			} else if (IS_INSTR_JUMP(op) || op == INSTR_LABEL) {

				arg[0] = ParseInstrLabel();
				ifok {
					n++;
					goto next_arg;
				}
			}

			EXP_IS_DESTINATION = true;
			while(n<3 && OK) {
				// For first argument, we always try to parse, as currently some rules are written with the comma at the start
				if (n!=0 && INSTR_INFO[op].arg_type[n] == TYPE_VOID) {
					n++;
				} else {
					had_comma = false;
					arg[n++] = ParseInstrArg();
					EXP_IS_DESTINATION = false;
next_arg:
					if (!NextIs(TOKEN_COMMA)) break;
					had_comma = true;
				}
			}
			EXP_IS_DESTINATION = false;

			if (had_comma || NextIs(TOKEN_COMMA)) {
				SyntaxError("instruction does not take more arguments");
			}
		}
	}	

	ifok {
		if (op != INSTR_VOID) {
			Gen(op, arg[0], arg[1], arg[2]);
		}
	}

	EXP_INSTR = false;
}

void ParseInstr2()
{	
	EnterBlock(TOKEN_VOID);
	while(OK && !NextIs(TOKEN_BLOCK_END)) {
		ParseInstr();
		NextIs(TOKEN_EOL);
	};
}

/*
TODO: Rule Parsing

1. Unify INSTR_ constants and INSTR_ constants

2. Parse rule arguments using expression parser
   var:type     => INSTR_MATCH
   var:val type => INSTR_MATCH_VAL
   var@list     => one of constants
   .var         => reference to variable

What about element?
*/

Bool PARSING_CONDITION;

RuleArg * NewRuleArg()
{
	RuleArg * arg;
	arg = MemAllocStruct(RuleArg);
	return arg;
}

RuleArg * NewOpRule(InstrOp op, RuleArg * l, RuleArg * r)
{
	RuleArg * arg = NewRuleArg();
	arg->variant = op;
	arg->arr = l;
	arg->index = r;
	return arg;
}

RuleArg * ParseRuleElement();
RuleArg * ParseRuleRange();

/*
void ParseRuleBinary(RuleArg * arg, InstrOp variant)
{
	RuleArg * arr;
	arr = NewRuleArg();
	MemMove(arr, arg, sizeof(RuleArg));
	arg->variant = variant;
	arg->arr     = arr;
	arg->arg_no  = 0;

	arg->index = NewRuleArg();
//	ParseRuleElement(arg->index);
	ParseRuleArg2(arg->index);
}
*/

void ParseRuleType(RuleArg * arg)
{
	Var * type = NULL;
	if (arg == NULL) return;
	if (NextIs(TOKEN_COLON)) {
		if (arg->variant == INSTR_NULL) {
			arg->variant = INSTR_MATCH;
		}
		if (CPU->SCOPE != NULL) {
			type = VarFind(CPU->SCOPE, NAME);
			if (type != NULL) {
				type->possible_values = ParseTypeInline();
			}
		}
		if (type == NULL) {
			type =	ParseTypeInline(); 
		}
		arg->type = type;
	}
}
/*
RuleArg *  ParseRuleArgArray(RuleArg * arg)
{
	if (NextCharIs(TOKEN_BYTE_INDEX)) {
		NextToken();
		ParseRuleBinary(arg, INSTR_BYTE);
	} else if (NextCharIs(TOKEN_PERCENT)) {
		NextToken();
		ParseRuleBinary(arg, INSTR_BIT);
	} else {
		NextToken();
	}
}
*/
RuleArg *  ParseSimpleRuleArg(Bool from_deref)
{
	RuleArg * arg = NULL;
	Var * var;
	UInt8 arg_no;

	if (TOK == TOKEN_ID) {
		arg = NewRuleArg();
		arg->variant = INSTR_VAR;
		arg->var = ParseVariable();

	} else if (LexInt(&var)) {
		arg = NewRuleArg();
		arg->variant = INSTR_VAR;
		arg->var = var;

	} else if (arg_no = ParseArgNo2()) {
		arg = NewRuleArg();
		arg->arg_no = arg_no;
		if (NextCharIs(TOKEN_ADR)) {
			NextToken();
			arg->variant = INSTR_VARIANT;
			arg->var = ParseVariable();
		} else {
			NextToken();
			arg->variant = INSTR_MATCH;
			if (!from_deref) {
//				ParseRuleArgArray(arg);
			}
		}

	} else if (NextIs(TOKEN_CONST)) {
		arg = NewRuleArg();
		arg->variant = INSTR_MATCH_VAL;
		arg->arg_no  = ParseArgNo();

	} else if (NextIs(TOKEN_OPEN_P)) {
		arg = ParseRuleRange();
		if (OK && !NextIs(TOKEN_CLOSE_P)) {
			SyntaxError("expected closing brace");
		}
	}

	ParseRuleType(arg);
	return arg;
}

RuleArg *  ParseRuleUnary()
{
	RuleArg * arg = NULL;
	if (NextIs(TOKEN_ADR)) {
		arg->variant = INSTR_DEREF;
		arg->arr = ParseSimpleRuleArg(true);
		if (arg->arr->variant == INSTR_MATCH) {
//			arg = ParseRuleArgArray(arg);
		}
		ParseRuleType(arg->arr);
	} else {
		arg = ParseSimpleRuleArg(false);
	}
	return arg;
}

RuleArg *  ParseRuleElement()
{
	RuleArg * idx, * idx2;
	RuleArg * arg = ParseRuleUnary();
retry:
	if (NextIs(TOKEN_OPEN_P)) {

		// Parse indexes (there can be comma separated list of indexes)
		idx = NULL;
		do {
			idx2 = idx;
			idx = ParseRuleArg2();
			if (idx2 != NULL) {
				idx = NewOpRule(INSTR_TUPLE, idx2, idx);
			}

		} while(NextIs(TOKEN_COMMA));

		arg = NewOpRule(INSTR_ELEMENT, arg, idx);

		if (OK && !NextIs(TOKEN_CLOSE_P)) {
			SyntaxError("expected closing brace");
		}
		goto retry;
	}
	return arg;
}


RuleArg *  ParseRuleMulDiv()
{
	RuleArg * arg = ParseRuleElement();
retry:
	if (NextIs(TOKEN_MUL)) {
		arg = NewOpRule(INSTR_MUL, arg, ParseRuleElement());
		goto retry;
	} else if (NextIs(TOKEN_DIV)) {
		arg = NewOpRule(INSTR_DIV, arg, ParseRuleElement());
		goto retry;
	}
	return arg;
}

RuleArg *  ParseRuleAdd()
{
	RuleArg * arg = ParseRuleMulDiv();
retry:
	if (NextIs(TOKEN_MINUS)) {
		arg = NewOpRule(INSTR_SUB, arg, ParseRuleMulDiv());
		goto retry;
	} else if (NextIs(TOKEN_PLUS)) {
		arg = NewOpRule(INSTR_ADD, arg, ParseRuleMulDiv());
		goto retry;
	}
	return arg;
}

RuleArg * ParseRuleBAnd()
{
	RuleArg * arg = ParseRuleAdd();
retry:
	if (NextIs(TOKEN_BITAND)) {
		arg = NewOpRule(INSTR_AND, arg, ParseRuleAdd());
		goto retry;
	}
	return arg;
}

RuleArg * ParseRuleBOr()
{
	RuleArg * arg;
	arg = ParseRuleBAnd();
retry:
	if (NextIs(TOKEN_BITOR)) {
		arg = NewOpRule(INSTR_OR, arg, ParseRuleBAnd());
		goto retry;
	} else if (NextIs(TOKEN_BITXOR)) {
		arg = NewOpRule(INSTR_XOR, arg, ParseRuleBAnd());
		goto retry;
	}
	return arg;
}

RuleArg * ParseRuleArith()
{
	return ParseRuleBOr();
}

RuleArg * ParseRuleRange()
{
	RuleArg * arg;
	arg = ParseRuleArith();
	if (NextIs(TOKEN_DOTDOT)) {
		arg = NewOpRule(INSTR_RANGE, arg, ParseRuleArith());
	}
	return arg;
}

RuleArg * ParseRuleDeref()
{
	RuleArg * arg;
	RuleArg * idx, * idx2;

	arg = ParseRuleRange();
	// Tuples
	if (NextIs(TOKEN_OPEN_P)) {
		idx = ParseRuleArg2();
		if (NextIs(TOKEN_COMMA)) {
			// There should be at least one comma
			idx2 = ParseRuleArg2();
			arg = NewOpRule(INSTR_TUPLE, idx, idx2);
			NextIs(TOKEN_CLOSE_P);
		}
	}
	return arg;
}

RuleArg * ParseRuleRel()
{
	RuleArg * arg;
	InstrOp op;
	arg = ParseRuleDeref();
	if (PARSING_CONDITION) {
		while (ParseRelOp(&op)) {
			arg = NewOpRule(op, arg, ParseRuleDeref());
		}
	}
	return arg;
}

RuleArg * ParseRuleNot()
{
	RuleArg * arg;
	Bool n = false;
	if (PARSING_CONDITION) {
		if (NextIs(TOKEN_NOT)) {
			n = true;
		}
	}
	arg = ParseRuleRel();
	if (n) {
		arg = NewOpRule(INSTR_NOT, arg, NULL);
	}
	return arg;
}

RuleArg * ParseRuleAnd()
{
	RuleArg * arg;
	arg = ParseRuleNot();
	if (PARSING_CONDITION) {
		if (NextIs(TOKEN_AND)) {
			arg = NewOpRule(INSTR_AND, arg, ParseRuleRel());
		}
	}
	return arg;
}

RuleArg * ParseRuleOr()
{
	RuleArg * arg;
	arg = ParseRuleAnd();
	if (PARSING_CONDITION) {
		if (NextIs(TOKEN_OR)) {
			arg = NewOpRule(INSTR_OR, arg, ParseRuleAnd());
		}
	}
	return arg;
}

RuleArg *  ParseRuleArg2()
{
	return ParseRuleOr();
}

RuleArg *  ParseRuleArg()
{
	PARSING_CONDITION = false;
	return ParseRuleArg2();
}

RuleArg * ParseRuleConditionArg()
{
	PARSING_CONDITION = true;
	return ParseRuleArg2();
}


void ResolveRuleArg(Rule * rule, RuleArg * arg)
{
	UInt8 i;
	RuleArg * rarg;

	if (arg == NULL) return;
	if (arg->variant == INSTR_MATCH && arg->type == NULL) {
		for(i=0; i<3; i++) {
			rarg = rule->arg[i];
			if (rarg != NULL && rarg != arg && rarg->arg_no == arg->arg_no) {
				arg->variant = rarg->variant;
				arg->var = rarg->var;
				return;
			}
		}
	}
}

Bool ParsingRule()
{
	return PARSING_RULE;
}

Bool ParsingPattern()
{
	return PARSING_PATTERN;
}

void ParseRule()
/*
<instr> "=" "instr" <instr>+  | "emit"+
*/
{
	InstrOp op = INSTR_NULL;

	UInt8 i;
	Rule * rule;
	char buf[255];
	char *s, *d, c;
	Bool macro_rule = false;
	Var * n;

	rule = MemAllocStruct(Rule);
	rule->line_no = LINE_NO;
	rule->file    = SRC_FILE;

	PARSING_RULE = true;
	PARSING_PATTERN = true;

	if (TOK == TOKEN_ID || TOK >= TOKEN_KEYWORD && TOK<=TOKEN_LAST_KEYWORD) {
		op = InstrFind(NAME);
	}

	// If the rule does not start with instruction name, we try more general syntax in the form
	//   <var> "=" <var>
	
	EXP_EXTRA_SCOPE = CPU->SCOPE;

	if (op == INSTR_NULL) {
		if (NextIs(TOKEN_IF)) {
			rule->arg[1] = ParseRuleConditionArg();
			if (NextIs(TOKEN_GOTO)) {
				rule->arg[2] = ParseRuleArg();		// must be label!	
				rule->op = INSTR_IF;
			} else {
				SyntaxError("Expected goto");
			}

		} else {

			//TODO: In future, we will parse the rule in a more general way
			EXP_IS_DESTINATION = true;
			rule->arg[0] = ParseRuleArg();
			EXP_IS_DESTINATION = false;

			if (NextIs(TOKEN_EQUAL)) {
				rule->op = INSTR_LET;
				rule->arg[1] = ParseRuleArg();
			} else {
				SyntaxError("Expected equal");
			}
		}

		// Flags defined as @flags
		if (NextIs(TOKEN_ADR)) {
			rule->flags = ParseVariable();
		}

		EXP_EXTRA_SCOPE = NULL;
	} else {

		NextToken();

		// Parse three parameters
		rule->op = op;

		EXP_IS_DESTINATION = true;
		EXP_EXTRA_SCOPE = CPU->SCOPE;

		for(i=0; i<3 && TOK != TOKEN_EQUAL && OK; i++) {
			if (INSTR_INFO[op].arg_type[i] != TYPE_VOID) {
				rule->arg[i] = ParseRuleArg();
			}
			EXP_IS_DESTINATION = false;

			// Flags defined as @flags
			if (NextIs(TOKEN_ADR)) {
				rule->flags = ParseVariable();
				break;
			}

			if (i != 2) {
				NextIs(TOKEN_COMMA);
			}
		}
	}

	for(i=0; i<3; i++) {
		ResolveRuleArg(rule, rule->arg[i]);
	}

	// Number of cycles may be defined after hash '#3'
	if (TOK == TOKEN_HASH) {
		if (LexInt(&n)) {
			rule->cycles = (UInt8)IntN(IntFromCell(n));
		} else {
			SyntaxError("Expected cycle count as integer");
		}
		
	}

	PARSING_PATTERN = false;
	EXP_IS_DESTINATION = false;
	EXP_EXTRA_SCOPE = NULL;

	// TODO: Rule should use parse block to parse code, TOKEN_INSTR should be part of parse code

	if (NextIs(TOKEN_RIGHT_ARROW)) {

		if (NextIs(TOKEN_MACRO)) {
			macro_rule = true;
			GenBegin();
			ParseCommandBlock();
			rule->to = GenEnd();
		} else if (NextIs(TOKEN_INSTR)) {
			GenBegin();
			ParseInstr2();
			rule->to = GenEnd();
		// Emitting rule
		} else if (LexString(NAME2)) {
			GenBegin();
			do {

				// Rule strings may are preprocessed so, that %/ is replaced by current path.
				s = NAME2;
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

				GenInternal(INSTR_EMIT, NULL, TextCell(buf), NULL);

			} while (LexString(NAME2));
			rule->to = GenEnd();

		// General syntax rule. We set CPU scope to allow access to registers.
		} else {
			GenBegin();
			EXP_EXTRA_SCOPE = CPU->SCOPE;
			ParseCommandBlock();
			EXP_EXTRA_SCOPE = NULL;
			rule->to = GenEnd();
		}
	} else {
		SyntaxError("Expected ->.");
	}

	ifok {
		if (macro_rule) {
			GenRegisterRule(rule);
		} else {
			RuleRegister(rule);
		}
	}
	PARSING_RULE = false;
	
}

Var * VarSize(Var * var)
{
	Var * size = NULL, * arr;

	if (var->mode == INSTR_ELEMENT) {
		arr = var->adr;
		size = VarFind(arr, "size");
		if (size != NULL) {
			size = VarNewElement(size, var->var);
		}
	} else {
		if (var->type->variant == TYPE_ARRAY) {			
			size = VarRangeSize(&var->type->index->range.min, &var->type->index->range.max);
		}
	}
	return size;
}

void ParseString(InstrBlock * call, UInt32 flags)
/*
Purpose:
	Parse string constant.
	String may contain variables enclosed in square braces.
*/
{

/*
	String generates following sections of code:

	1. expressions used to calculate the string parameters 
	2. call to string output routine
	3. list of arguments
	4. EOL (optional)
	5. End of argument list
*/

	Var * var, * var2, * size_var, * size_tmp;
	Bool no_eol;
	UInt16 n;
	InstrBlock * args;

	do {

		// We need to create list of argument instructions now, but generate it later
		// Therefore we create instrblock and insert the argument instructions there.
		// Later it gets generated to current code.

		args =  InstrBlockAlloc();
		no_eol = false;
		LINE_POS = TOKEN_POS+1;

		while (OK) {
			NextStringToken();
			if (TOK == TOKEN_BLOCK_END) break;

			// Constant string argument
			if (TOK == TOKEN_STRING) {
				var = TextCell(NAME);
				InstrInsert(args, NULL, INSTR_VAR_ARG, NULL, var, NULL);
			// Expression argument (one or more expressions)
			} else {
				ASSERT(TOK == '[');
				EnterBlock();
				ParseExpression(NULL);
				ASSERT(TOK == TOKEN_BLOCK_END);

				for(n=0; n<TOP; n++) {

					var = STACK[n];

					if (VarIsArray(var)) {
						var2 = NewTempVar(TypeAdrOf(var->type));
						size_var = VarSize(var);
						Gen(INSTR_LET_ADR, var2, var, NULL);
						if (size_var->mode == INSTR_ELEMENT) {
							size_tmp = NewTempVar(size_var->adr->type->element);
							GenLet(size_tmp, size_var);
							size_var = size_tmp;
						}

						InstrInsert(args, NULL, INSTR_VAR_ARG, NULL, var2, size_var);
					} else {
						// If the parsed value is element, we need to store it to temporary variable first.
						// Otherwise the code to access the element would get generated into list of arguments.

						if (var->mode == INSTR_ELEMENT) {
							var2 = NewTempVar(var->adr->type->element);
							GenLet(var2, var);
							var = var2;
						}
						InstrInsert(args, NULL, INSTR_VAR_ARG, NULL, var, NULL);
					}
				}
			}
		}

		GenBlock(call);
		GenBlock(args);

		ifok {
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
			var2 = IntCellN(128);
			Gen(INSTR_DATA, NULL, var2, NULL);
		}

	} while (TOK == TOKEN_STRING);


	// Generate ending 0 byte
	var2 = ZERO;
	Gen(INSTR_DATA, NULL, var2, NULL);

}


UInt16 ParseArgs(Var * proc, Var * arg_type, VarSubmode submode, Var ** args)
/*
Purpose:
	Parse arguments passed to procedure or macro.
Arguments:
	proc     Procedure or macro for which we parse the arguments.
	submode  SUBMODE_ARG_IN if parsing input arguments, SUBMODE_ARG_OUT if parsing output arguments
	args     When specified, we store parsed argument values to this array.
*/
{
	Var * arg, * val, * tmp, * arg_enum;
	Bool no_next_args;
	UInt16 first, idx;

	// *** Register Arguments (4)
	// When calling a procedure, we first store values of register arguments into temporary variables and continue with evaluation of next argument.
	// This prevents trashing the register by some more complex operation performed when computing following arguments.
	// All values of register arguments are loaded directly before actual call is made.

	Var * reg_args[MAX_ARG_COUNT];		// temporary variables allocated for register arguments
	Var * reg_vals[MAX_ARG_COUNT];
	UInt8 reg_arg_cnt, i, arg_no;

	no_next_args = false;
	reg_arg_cnt = 0;
	arg_no = 0;

	TOP = 0;		// Currently ExpressionParser sets the TOP to 0, so we must do it too.
	                // In future, we may need to solve it in a better way (ExpressionParser should not do it).
	first = idx = TOP;
	arg_enum = arg = arg_type;
	if (arg->mode == INSTR_TUPLE) {
		arg = arg_enum->l;
	}

	if (arg != NULL) {
		EnterBlock();
		while(OK) {

			// If we do not need to parse another argument, exit the loop and the parse block
			if (arg == NULL) {
				if (!no_next_args) {
					if (!NextIs(TOKEN_BLOCK_END)) {
						ExitBlock();
					}
				}
				break;
			}

			// Parse next expression, if there are no arguments remaining on the stack and there has not been 
			// and end of argument block yet.
			if (!no_next_args) {
				if (TOP == idx) {
					if (!NextIs(TOKEN_BLOCK_END)) {
						TOP = first;
						idx = first;
						ParseExpression(arg);
						iferr break;		// TODO: consume line?, or to next comma?
					} else {
						no_next_args = true;
					}
				}
			}

			// Use value from stack or default value of an argument
			if (TOP > idx) {
				val = STACK[idx++];
			} else {
				val = arg->var;		// argument default value
			}

			if (val != NULL) {
				if (args != NULL) {
					args[arg_no] = val;
				} else {
					if (VarIsReg(arg)) {
						//TODO: If var is already tmp, we do not need to create new temporary here
						tmp = NewTempVar(arg->type);
						GenLetPos(tmp, val);
						val = tmp;
						reg_args[reg_arg_cnt] = arg;
						reg_vals[reg_arg_cnt] = val;
						reg_arg_cnt++;
					} else {
						if (VarIsTmp(val)) {
							GenLastResult(arg, val);
						} else {
							GenLet(arg, val);
						}
					}
				}
			} else {
				if (submode == SUBMODE_ARG_IN) {
					ErrArg(arg);
					ErrArg(proc);
					if (IsMacro(proc->type)) {
						SyntaxError("Missing argument [B] in use of macro [A]");
					} else {
						SyntaxError("Missing argument [B] in call of procedure [A]");
					}

				// Output arguments (in return) do not have to be specified all
				} else {
					break;
				}
			}
			if (arg_enum->mode == INSTR_TUPLE) {
				arg = arg_enum = arg_enum->r;
				if (arg_enum->mode == INSTR_TUPLE) arg = arg_enum->l;
			} else {
				arg = NULL;
			}
//			arg = NextArg(proc, arg, submode);
			NextIs(TOKEN_COMMA);		// Arguments may be optionally separated by comma
			arg_no++;
		}
	}

	if (idx < TOP) {
		SyntaxError("superfluous argument");
	}
	TOP = first;

	// Load register arguments
	if (!IsMacro(proc->type)) {
		ifok {
			for(i=0; i<reg_arg_cnt; i++) {
				GenLet(reg_args[i], reg_vals[i]);
			}
		}
	}

	return arg_no;
}

void ParseCall(Var * var)
{
	Var * proc = var;
	ParseArgs(proc, ArgType(var->type->type), SUBMODE_ARG_IN, NULL);
	Gen(INSTR_CALL, NULL, var, NULL);
}

void ParseReturn()
/*
Syntax: "return" arg*
*/
{
	Var * proc;
	Var * label;

	NextToken();
	proc = VarProcScope();
	ParseArgs(proc, ResultType(proc->type->type), SUBMODE_ARG_OUT, NULL);

	// Return is implemented as jump to end of procedure
	// Optimizer may later move return instead of jump (if there is no cleanup)

	label = FindOrAllocLabel("_exit");
	GenGoto(label);
}

void ParseMacro(Var * macro)
{
	Var * args[32];
	Var * arg, * var, * en;
	UInt16 arg_cnt;
	UInt16 in_cnt;

	in_cnt = ParseArgs(macro, ArgType(macro->type->type), SUBMODE_ARG_IN, args);

	arg_cnt = in_cnt;
	// We must generate temporary variables for results and store them to expression stack
	FOR_EACH_ITEM(en, arg, ResultType(macro->type->type))
		var = NewTempVar(arg->type);
		args[arg_cnt] = var;
		BufPush(var);
		arg_cnt++;
	NEXT_ITEM(en, arg)

	ifok {
		GenMacro(macro, args);
	}
}

void ParseDeclarations(InstrOp mode, VarSubmode submode)
/*
Purpose:
	Parse list of declarations of variables of specified mode and submode.
Syntax:
	Decl: { [<assign>]* }
*/
{
	EnterBlock();		
	while (OK && !NextIs(TOKEN_BLOCK_END)) {
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
	Parse(NAME, false, true);
}

void ParseUse()
/*
Syntax: { [file_ref] }
*/
{
	NextToken();		// skip TOKEN_USE
	EnterBlock();
	while (OK && !NextIs(TOKEN_BLOCK_END)) {
		ParseUseFile();
		NextIs(TOKEN_COMMA);
		while(NextIs(TOKEN_EOL));
	}
}

void AssertVar(Var * var)
{
	Var * name;
	char buf[200];
	InstrInfo * ii;
	char * p;

	if (var == NULL) return;
	p = buf; 
	*p++ = ' ';

	if (var->mode == INSTR_VAR && !VarIsReg(var) && !VarIsLabel(var) && var->name != NULL && !VarIsTmp(var)) {
		StrCopy(p, var->name); p += StrLen(var->name);
		StrCopy(p, " = "); p += 3;
		name = TextCell(buf);
		GenInternal(INSTR_VAR_ARG, NULL, name, NULL);
		GenInternal(INSTR_VAR_ARG, NULL, var, NULL);

	// Array access will be reported using syntax array#index = value.

	} else if (var->mode == INSTR_ELEMENT) {
		StrCopy(p, var->l->name); p += StrLen(var->l->name);
		*p++ = '#';
		if (var->r->mode == INSTR_INT) {
			p = IntFormat(p, &var->r->n);
		}
		StrCopy(p, " = "); p += 3;
		name = TextCell(buf);

		GenInternal(INSTR_VAR_ARG, NULL, name, NULL);
		if (InstrTranslate3(INSTR_VAR_ARG, NULL, var, NULL, TEST_ONLY)) {
			GenInternal(INSTR_VAR_ARG, NULL, var, NULL);
		} else {
			//TODO: Generate temporary variable with the result
		}

	} else {
		ii = &INSTR_INFO[var->mode];
		if (ii->arg_type[1] != TYPE_VOID) AssertVar(var->l);
		if (ii->arg_type[2] != TYPE_VOID) AssertVar(var->r);
	}
}

void ParseAssert()
{
	InstrBlock * cond_code, * args;
	Instr * i;
	char location[100];
	UInt16 bookmark;
	Var * cond, * label;

	if (TOK == TOKEN_STRING) {
		if (MACRO_ASSERT_PRINT != NULL) {
			GenBegin();
			GenMacro(MACRO_ASSERT_PRINT, NULL);
			ParseString(GenEnd(), 0); 
		} else {
			SyntaxError("This platform does not support output asserts");
		}
	} else {

		// We must remember block to be able to analyze the used variables

		Gen(INSTR_ASSERT_BEGIN, NULL, NULL, NULL);
		BeginBlock(TOKEN_IF);		// begin if block		
		G_BLOCK->not = true;

		GenBegin();
		bookmark = SetBookmark();
//		ParseCondition();
		cond = ParseCondExpression();
		iferr return;

		label =  VarNewTmpLabel();
		Gen(INSTR_IF, NULL, cond, label);

		cond_code = GenEnd();

		if (CodeHasSideEffects(SCOPE, cond_code)) {
			LogicWarning("assertion has side-effects", bookmark);
		}

		// Generate arguments for assert only if the ASSERT instruction has been defined by the platform

		if (!ASSERTS_OFF) {
			GenBegin();

			sprintf(location, "Error %s(%d): ", SRC_FILE->name, LINE_NO);
			Gen(INSTR_VAR_ARG, NULL, TextCell(location), NULL);
			for(i = cond_code->first; i != NULL; i = i->next) {
				if (i->op != INSTR_LINE) {
					AssertVar(i->arg1);
					AssertVar(i->arg2);
				}
			}
			Gen(INSTR_DATA, NULL, ZERO, NULL);

			args = GenEnd();
		}

		GenBlock(cond_code);

		// If condition referenced true label (which is not necessary, if it didn't contain AND or OR),
		// generate it here

		if (G_BLOCK->t_label != NULL) {
			GenLabel(G_BLOCK->t_label);
		}

		// Generate call to assert (variant of print instruction)
		if (!ASSERTS_OFF) {
			GenMacro(MACRO_ASSERT, NULL);
			GenBlock(args);
		}

		// generate file name and line number
		// generate list of used variables

		Gen(INSTR_ASSERT, NULL, NULL, NULL);
		GenLabel(G_BLOCK->f_label);
		GenLabel(label);
		Gen(INSTR_ASSERT_END, NULL, NULL, NULL);
		EndBlock();
	}
}

void ParseCommands()
{
	VarSubmode submode;

	while (TOK != TOKEN_BLOCK_END && TOK != TOKEN_EOF && OK && TOK != TOKEN_OUTDENT) {

		if (NextIs(TOKEN_IF)) {
			ParseIf(false, NULL);
		} else if (NextIs(TOKEN_UNLESS)) {
			ParseIf(true, NULL);
		} else if (NextIs(TOKEN_GOTO)) {
			ParseGoto();
		} else if (NextIs(TOKEN_RULE)) {
			ParseRule();
		} else if (NextIs(TOKEN_ASSERT)) {
			ParseAssert();
		} else if (PeekNext(TOKEN_WHILE)) {
			ParseFor();
		} else if (PeekNext(TOKEN_UNTIL)) {
			ParseFor();
		} else if (PeekNext(TOKEN_FOR)) {
			ParseFor();
		} else if (NextIs(TOKEN_DEBUG)) {
			Gen(INSTR_DEBUG, NULL, NULL, NULL);
		} else {
			switch(TOK) {

			// *** Module parameters (2)
			// Module parameters are declared in the module is the same way as constants, only prefixed with ::param:: keyword.
			// For example: ::param ORG:0..65535 = $2000

			case TOKEN_PARAM: 
				NextToken();
				ParseDeclarations(INSTR_NAME, SUBMODE_PARAM); break;
			case TOKEN_CONST: 
				NextToken();
				ParseDeclarations(INSTR_NAME, SUBMODE_EMPTY); break;
			case TOKEN_TYPE2:  
				NextToken();
				ParseDeclarations(INSTR_TYPE, SUBMODE_EMPTY); break;
			case TOKEN_IN:
				submode = SUBMODE_IN;
				NextToken();
				if (NextIs(TOKEN_SEQUENCE)) {
					submode |= SUBMODE_IN_SEQUENCE;
				}
				if (NextIs(TOKEN_OUT)) {
					submode |= SUBMODE_OUT;
				}
				ParseDeclarations(INSTR_VAR, submode); 
				break;
			case TOKEN_OUT:  
				submode = SUBMODE_OUT;
				NextToken();
				ParseDeclarations(INSTR_VAR, SUBMODE_OUT);	
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
				if (MACRO_PRINT != NULL) {
					GenBegin();
					if (IsMacro(MACRO_PRINT->type)) {
						GenMacroParse(MACRO_PRINT, NULL);
					} else {
						Gen(INSTR_CALL, NULL, MACRO_PRINT, NULL);
					}
					ParseString(GenEnd(), 0); 
				} else {
					PlatformError("Print is not supported by the platform");
				}
				break;

			case TOKEN_PERCENT:
				ParseAssign(INSTR_VAR, SUBMODE_EMPTY, NULL); 
				break;
			case TOKEN_ID:
			case TOKEN_DOT:
				ParseAssign(INSTR_VAR, SUBMODE_EMPTY, NULL); 
				break;

			case TOKEN_EOL:
				NextToken(); 
				break;
			default:         
				SyntaxError("unexpected token");
			}
		}
	}
}

extern UInt8      BLK_TOP;

Bool Parse(char * name, Bool main_file, Bool parse_options)
{
	Bool no_platform;

	no_platform = (*PLATFORM == 0);
	if (SrcOpen(name, parse_options)) {
		if (main_file) {
			SRC_FILE->submode = SUBMODE_MAIN_FILE;
		}
		ParseCommands();
		ifok {
			if (no_platform  && *PLATFORM != 0) {
				InitPlatform();
			}
			if (TOK != TOKEN_BLOCK_END) {
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
	G_BLOCK->command = TOKEN_FN;
	USE_PARSE = false;
	EXP_EXTRA_SCOPE = NULL;
	OP_LINE_POS = 0;
	OP_LINE_NO  = 0;
	UNUSED_INSTR_SCOPE = NULL;

}
