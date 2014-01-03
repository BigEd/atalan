#include "language.h"

void ExpectExpression(Var * result);
void ParseEnumItems(Type * type, UInt16 column_count);
void ParseAssign(InstrOp mode, VarSubmode submode, Type * to_type);
Var * ParseVariable();
BigInt * ParseIntConstExpression(Var * result);

Type * ParseType();
Type * ParseSubtype();
UInt8 ParseArgNo();

Bool PARSE_INLINE;

void ParseArgList(VarSubmode mode, Type * to_type)
/*
Purpose:
	Parse block with list of arguments.
	  [">" | "<"] assign
	Arguments are added to current context with submode SUBMODE_ARG_*.

	This method is used when parsing procedure or macro argument declaration or structure declaration.
*/
{
	VarSubmode submode = SUBMODE_EMPTY;
	Var * var, * adr;
	Bool out_part = false;

 	EnterBlockWithStop(TOKEN_EQUAL);			// TOKEN_EQUAL

	while (OK && !NextIs(TOKEN_BLOCK_END)) {

		if (!out_part && NextIs(TOKEN_RIGHT_ARROW)) {
			out_part = true;
		}

		submode = mode;

		if (out_part) {
			submode = SUBMODE_ARG_OUT;
		} else {

			if (NextIs(TOKEN_LOWER)) {
				submode = SUBMODE_ARG_IN;
			}
			if (NextIs(TOKEN_HIGHER)) {
				submode = SUBMODE_ARG_OUT;
			}
		}

		// Variables preceded by @ define local variables used in the procedure.
		if (NextIs(TOKEN_ADR)) {
			adr = ParseVariable();
			ifok {
				var = NewVarInScope(adr->type, to_type);
				var->adr  = adr;
				NextIs(TOKEN_EOL);
				continue;
			}
		}

		if (TOK == TOKEN_ID) {
			ParseAssign(INSTR_VAR, submode, to_type);
			NextIs(TOKEN_COMMA);
			NextIs(TOKEN_EOL);
		} else {
			SyntaxError("Expected variable name");
		}
	}
}


/*
      TUPLE
      /   \
	 V1   TUPLE
	      /   \	    
		 V2  TUPLE
		      /  \
			 V3  V4

*/

Var * ParseStruct(Token end_token)
{
	Var * list = NULL;
	Var * var, * type;
	Var * last_tuple = NULL;
	UInt32 cnt = 0;

	var = NULL;
	EnterBlockWithStop(TOKEN_VOID);

	while (OK && !NextIs(TOKEN_BLOCK_END)) {

		if (TOK == end_token) {
			if (list == NULL) list = VOID;
			ExitBlock();
			break;
		}

		if (LexId(NAME2)) {
			if (NextIs(TOKEN_COLON)) {
				type = ParseType();
			} else {
				type = ANY;
			}
			var = NewVar(NO_SCOPE, NAME2, type);
		} else {
			SyntaxError("Expected item name");
		}

		if (var != NULL) {
			if (cnt == 0) {
				list = var;
			} else if (cnt == 1) {
				list = NewTuple(list, var);
				last_tuple = list;
			} else {
				last_tuple->r = NewTuple(last_tuple->r, var);
				last_tuple = last_tuple->r;
			}
			cnt++;
		}

		if (NextIs(TOKEN_COMMA)) {
			
		} else {
			ExitBlock();
			break;
		}
	}

	return list;
}

void ParseCommandBlock();

Var * ParseFnType()
{
	Var * arg, * result = ANY;
	Var * type;
	Var * fn, * scope;

	InstrBlock * body;

	arg = ParseStruct(TOKEN_RIGHT_ARROW);

	if (NextIs(TOKEN_RIGHT_ARROW)) {
		ParseDefExpression();
		result = BufPop();
		if (result == NULL) result = VOID;
	}

	type = NewFnType(arg, result);

	fn = NewFn(type, NULL);
	fn->scope = SCOPE;
	scope = InScope(fn);
	
	GenBegin();
	ParseCommandBlock();
	body = GenEnd();
	ReturnScope(scope);

	iferr return NULL;

	if (body->first != NULL) {
		fn->instr = body;
		type = fn;
	}

	return type;
}


Type * ParseIntType()
{
	Type * type = TUNDEFINED;
	Var * var;
	Bookmark bookmark;
	UInt8 arg_no = 0;

	// When parsing rule, type may be preceded by %name:
	if (ParsingPattern()) {
		arg_no = ParseArgNo();
		if (arg_no > 0) {
			if (!NextIs(TOKEN_COLON)) {
				SyntaxError("Argument name must be followed by :");
			}
		}
	}

	bookmark = SetBookmark();
	ExpectExpression(NULL);
	ifok {
		var = BufPop();

		// If we parsed rule argument
		if (var->mode == INSTR_ELEMENT || VarIsRuleArg(var)) {
			type = var;
			goto done;
		// Integer type may be defined using predefined type definition or be defined as type of other variable
		} else if (var->mode == INSTR_TYPE || var->mode == INSTR_VAR) {
			type = var->type;

			if (type->mode == INSTR_TYPE) {
				if (type->variant == TYPE_TYPE) {
					type = var->type_value;
					SetFlagOn(var->submode, SUBMODE_USED_AS_TYPE);
				}

				if (type->variant != TYPE_INT) {
					SyntaxErrorBmk("Expected integer type", bookmark);
				}
			}
			goto done;
		} else if (var->mode == INSTR_INT || var->mode == INSTR_RANGE) {
			type = var;
		} else {
			//TODO: If simple integer variable, use it as type range
			SyntaxErrorBmk("expected type or constant expression", bookmark);
		}

		if (NextIs(TOKEN_DOTDOT)) {
			bookmark = SetBookmark();
			ExpectExpression(NULL);
			ifok {
				var = BufPop();
				type = NewRange(type, var);
			}
		} else {
		}

		if (type->mode == INSTR_RANGE && !IsLowerEq(type->adr, type->var)) {
			SyntaxErrorBmk("range minimum bigger than maximum", bookmark);
		}
	}
done:

	ifok {
		if (arg_no != 0) {
			var = NewCell(INSTR_MATCH);
			var->l = VarRuleArg(arg_no-1);
			var->r = type;
			type = var;
//			type = TypeAllocArg(arg_no, type);
		}
	}
	return type;
}

void ParseEnumStruct(Type * type)
{
	Var * var;
	UInt16 column_count = 0;

 	EnterBlockWithStop(TOKEN_EQUAL);			// TOKEN_EQUAL

	while (OK && !NextIs(TOKEN_BLOCK_END)) {

		if (TOK == TOKEN_ID) {
			ParseAssign(INSTR_VAR, SUBMODE_EMPTY, type);
			NextIs(TOKEN_COMMA);
			NextIs(TOKEN_EOL);
		} else {
			SyntaxError("Expected variable name");
		}
	}

	// Convert parsed fields to constant arrays
	FOR_EACH_LOCAL(type, var)
		var->type = TypeArray(type, var->type);		
//		var->type->step = TypeSize(var->type->element);
		var->mode = INSTR_INT;
		column_count++;
	NEXT_LOCAL

	NextIs(TOKEN_HORIZ_RULE);

	ParseEnumItems(type, column_count);
}

Type * ParseConstList(Type * type)
{
	Bool id_required;
	Var * var;
	BigInt last_n, * c;

	IntInit(&last_n, 0);

	EnterBlockWithStop(TOKEN_VOID);
		
	id_required = false;

	while (OK && !NextIs(TOKEN_BLOCK_END)) {

		while(NextIs(TOKEN_EOL));

		if (LexId(NAME2)) {
			var = NewVar(type, NAME2, NULL);
			if (NextIs(TOKEN_EQUAL)) {
				SyntaxError("Unexpected equal");
			}

			if (NextIs(TOKEN_COLON)) {
				c = ParseIntConstExpression(type);
				ifok {
					IntModify(&last_n, c);
				}
			} else {
				IntAddN(&last_n, 1);
			}

			var->type = IntCell(&last_n);


			// ????
//			if (type->owner != SCOPE) {
//				type = TypeDerive(type);
//			}

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
	IntFree(&last_n);
	return type;
}

Type * ParseType3()
{
	Type * type = NULL, * variant_type = NULL;
	Type * elmt, * t;
	Var * index, * item, * step;

	//# "type" restrict_type

	if (NextIs(TOKEN_SEQUENCE)) {
		ParseExpression(NULL);
		index = BufPop();
	} else if (NextIs(TOKEN_TYPE2)) {
		variant_type = ParseType2(INSTR_VAR);
		type = TypeAlloc(TYPE_TYPE);
		type->possible_values = variant_type;
	//# "enum" ["struct"]
	} else if (NextIs(TOKEN_ENUM)) {
		type = TypeAlloc(TYPE_INT);
		type->is_enum        = true;
		if (NextIs(TOKEN_STRUCT)) {
			ParseEnumStruct(type);
		} else {
			type = ParseConstList(type);
		}

	//# "proc" args
	} else if (NextIs(TOKEN_PROC)) {
		type = TypeAlloc(TYPE_PROC);
		ParseArgList(SUBMODE_ARG_IN, type);
		ifok {
			ProcTypeFinalize(type);
		}
	//# "macro" args
	} else if (NextIs(TOKEN_MACRO)) {

		type = ParseFnType();
		if (type != NULL) {
			SetFlagOn(type->submode, SUBMODE_MACRO);
		}

	// Struct
	} else if (NextIs(TOKEN_STRUCT)) {
		type = TypeAlloc(TYPE_STRUCT);
		ParseArgList(SUBMODE_EMPTY, type);

	// String
	} else if (NextIs(TOKEN_STRING_TYPE)) {
		type = TypeAlloc(TYPE_STRING);

	// Array
	} else if (NextIs(TOKEN_ARRAY)) {		
		index = NULL; t = NULL;
		item = NULL;
		step = NULL;


		if (TOK == TOKEN_OPEN_P) {
			EnterBlockWithStop(TOKEN_EQUAL);
			while (OK && !NextIs(TOKEN_BLOCK_END)) {
				elmt = ParseIntType();
				if (index == NULL) {
					index = elmt;
				} else if (t != NULL) {
					t->right = TypeTuple(t->right, elmt);
					t = t->right;
				} else {
					t = TypeTuple(index, elmt);
					index = t;
				}
				NextIs(TOKEN_COMMA);
			};
		}
		
		// If no dimension has been defined, use flexible array.
		// This is possible only for constants now.

		if (index == NULL) index = ANY;

		// Element STEP may be defined
		ifok {
			if (NextIs(TOKEN_STEP)) {
				ExpectExpression(NULL);
				ifok {
					step = STACK[0];
//					} else {
//						SyntaxError("Expected integer constant");
//					}
				}
			}
		}

		ifok {
			if (NextIs(TOKEN_OF)) {
				item = ParseSubtype();
			} else {
				item = TypeByte();
			}
		}

		type = NewArrayType(index, item);
		SetArrayStep(type, step);

	} else if (NextIs(TOKEN_ADR2)) {
		elmt = NULL;
		if (NextIs(TOKEN_OF)) {
			elmt = ParseSubtype();
		}
		type = TypeAdrOf(elmt);
	}
	return type;
}

Type * ParseType2(InstrOp mode)
/*
Purpose:
	Parse: <int> [".." <int>] | <var> | proc <VarList>
Input:
	mode	Type of variable for which we parse.
*/
{
	
	Var * var;
	Type * type = NULL, * variant_type = NULL;
	Var * min, * max;
	BigInt * imin, * imax;

	Bookmark bookmark;

next:
	type = ParseType3();
	iferr return NULL;

	if (type == NULL) {
		bookmark = SetBookmark();
		ParseExpressionType(NULL);
		if (OK && TOP != 0) {

			min = BufPop();
			if (min->mode == INSTR_RANGE) {
				if (IsEqual(min->adr, min->var)) return min->adr;
				return min;
			}

			max = NULL;
			if (NextIs(TOKEN_DOTDOT)) {
				ExpectExpression(NULL);
				ifok {
					max = BufPop();
				}
			//# <int>
			} else if (CellIsIntConst(min)) {
				type = min;
				goto done;
			}

			type = NULL;
			if (max == NULL) {
				var = min;
				if (var->mode == INSTR_TYPE) {
					type = var->type;
				} else if (var->mode == INSTR_VAR && var->type->mode == INSTR_TYPE && var->type->variant == TYPE_TYPE) {
					type = var->type->possible_values;
					SetFlagOn(var->submode, SUBMODE_USED_AS_TYPE);
				}

				// This is directly type
				if (type != NULL) {
					// For integer type, constants may be defined
					if (type->variant == TYPE_INT) {
//						type = ParseIntRange(type);
						goto const_list;
					}
					goto done;
				}
				max = var;		
			}

			if (IsEqual(min, max)) {
				type = min;
				goto done;
			} else {
				imin = IntFromCell(min);
				imax = IntFromCell(max);
				if (imin != NULL && imax != NULL) {
					if (IntHigher(imin, imax)) {
						SyntaxErrorBmk("range minimum bigger than maximum", bookmark);
						goto done;
					}

					type = NewRange(min, max);

				} else {
					SyntaxErrorBmk("expected type constant expression", bookmark);
				}
			}
		//# "text"
		} else if (LexString(NAME2)) {
			type = TextCell(NAME2);
			goto done;

		}
	}

const_list:
	// Parse type specific constants
	// There can be list of constants specified in block.
	// First thing in the block must be an identifier, so we try to open the block with this in mind.
	// We try to parse constants only for integer types (in future, we may try other numeric or string types)

	if (type != NULL && type->variant == TYPE_INT && !type->is_enum) {
		if (TOK != TOKEN_OR) {
			type = ParseConstList(type);
		}
	}
done:
	ifok {
		if (variant_type != NULL) {
			variant_type->right = type;
			type = variant_type;
		}

		if (NextIs(TOKEN_OR)) {
			variant_type = TypeAlloc(TYPE_VARIANT);
			variant_type->left = type;
			goto next;
		}
	}
	return type;
}

Type * ParseType()
{
	PARSE_INLINE = false;
	return ParseType2(INSTR_VAR);
}

BigInt * ParseIntConstExpression(Var * result)
{
	BigInt * bi = NULL;
	Var * var;

	ParseExpression(result);
	ifok {
		var = BufPop();
		bi = IntFromCell(var);
		if (bi == NULL) {
			SyntaxError("expected constant expression");
		}
	}
	return bi;
}

Type * ParseTypeInline() 
/*
Syntax: "+" full_type | "(" full_type ")" | normal_type |  identifier | int ".." exp | "-" int ".." exp
*/{
	Type * type = NULL;
	Var * var;
	UInt16 bookmark;

	PARSE_INLINE = true;

	if (TOK == TOKEN_OPEN_P || TOK == TOKEN_PLUS) {
		type = ParseType2(INSTR_TYPE);
	} else {
		type = ParseType3();
		if (!TOK) return NULL;
		if (type != NULL) return type;

		if (ParseArg(&var)) {
			type = var;
		} else if (TOK == TOKEN_ID) {
			bookmark = SetBookmark();
			var = ParseVariable();
			ifok {
				if (var->mode == INSTR_TYPE) {
					type = var->type;
				} else if (var->mode == INSTR_VAR && var->type->mode == INSTR_TYPE && var->type->variant == TYPE_TYPE) {
					type = var->type->possible_values;
				} else {
					ErrArg(var);
					SyntaxErrorBmk("Variable [A] does not define type.", bookmark);
				}
			}
		} else if (TOK == TOKEN_INT || TOK == TOKEN_MINUS) {
			ParseExpression(NULL);
			ifok {
				type = BufPop();
				if (NextIs(TOKEN_DOTDOT)) {
					ParseExpression(NULL);
					ifok {
						type = NewRange(type, BufPop());
					}
				}
			}
		}
	}
	return type;
}

Type * ParseSubtype()
{
	if (PARSE_INLINE) {
		return ParseTypeInline();
	} else {
		return ParseType();
	}
}
