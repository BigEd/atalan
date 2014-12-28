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

Type * ParseIntType()
{
	Type * type = ANY;
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

//			if (type->mode == INSTR_TYPE) {
//				if (type->variant == TYPE_TYPE) {
//					type = var->type_value;
//					SetFlagOn(var->submode, SUBMODE_USED_AS_TYPE);
//				}

//				if (type->variant != TYPE_INT) {
//					SyntaxErrorBmk("Expected integer type", bookmark);
//				}
//			}
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

		if (type->mode == INSTR_RANGE && !IsLowerEq(type->l, type->r)) {
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

//	if (NextIs(TOKEN_SEQUENCE)) {
//		ParseExpression(NULL);
//		index = BufPop();
//	} else 
/*	if (LexWord("type")) {
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
*/
	//# "fn" args
	if (LexWord("fn")) {
		type = ParseFn(false);
	//# "macro" args
	} else if (LexWord("macro")) {

		type = ParseFn(true);

	// String
	} else if (LexWord("string")) {
		type = TypeAlloc(TYPE_STRING);

	// Array
	} else if (LexWord("array")) {		
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
					t->r = NewTuple(t->r, elmt);
					t = t->r;
				} else {
					t = NewTuple(index, elmt);
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
			if (LexWord("step")) {
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
			if (LexWord("of")) {
				item = ParseSubtype();
			} else {
				item = TypeByte();
			}
		}

		type = NewArrayType(index, item);
		SetArrayStep(type, step);

	} else if (LexWord("adr")) {
		elmt = NULL;
		if (LexWord("of")) {
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
				if (IsEqual(min->l, min->r)) return min->l;
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

	ifok {
		if (type != NULL && type->variant == TYPE_INT && !type->is_enum) {
			if (TOK != TOKEN_OR) {
				type = ParseConstList(type);
			}
		}
	}
done:
	ifok {
		if (variant_type != NULL) {
			variant_type->r = type;
			type = variant_type;
		}

		if (NextIs(TOKEN_OR)) {
			variant_type = NewOp(INSTR_VARIANT, type, NULL);
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
