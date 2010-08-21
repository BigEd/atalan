/*********************************************************

Lexer

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*********************************************************/

#include "language.h"
#include <ctype.h>

typedef struct {
	Token  end_token;		// TOKEN_OUTDENT, TOKEN_CLOSE_P, TOKEN_EOL, TOKEN_BLOCK_END
	Token  stop_token;
	UInt16 indent;
} BlockStyle;

#define UNDEFINED_INDENT 65535

GLOBAL char   LINE[MAX_LINE_LEN];		// current buffer
GLOBAL char * PREV_LINE;				// previous line
GLOBAL LineNo  LINE_NO;
GLOBAL UInt16  LINE_LEN;
GLOBAL UInt16  LINE_POS;						// index of next character in line
GLOBAL UInt16  TOKEN_POS;

int     PREV_CHAR;

GLOBAL Lexer LEX;
GLOBAL Token TOK;						// current token
GLOBAL Token NEXT_TOK;					// if not TOKEN_VOID, return this token on next call
GLOBAL static BlockStyle BLK[64];		// Lexer blocks (indent, parentheses, line block)
GLOBAL UInt8      BLK_TOP;
static UInt16 BLOCK_END_CNT;

static char * keywords[] = {
	"goto", "if", "then", "else", "proc", "rule", "macro", "and", "or", "not",
	"while", "until", "where", "const", "enum", "array", "type", "file", "lo", "hi", "of",
	"for", "in", "out", "instr", "times", "adr", "debug", "mod", "xor"
};

int ReadByte()
{
	return fgetc(LEX.f);
}

static Bool ReadLine()
/*
Purpose:
	Read next line from current source file.
Result:
	Return true, if some line was loaded.
*/
{
	int b, b2;

	// Remeber current line as previous

	if (PREV_LINE != NULL) {
		free(PREV_LINE);
		PREV_LINE = NULL;
	}
	if (LINE_LEN > 0) {
		PREV_LINE = StrAlloc(LINE);
	}

	LINE_LEN = 0;
	b = EOF;

	if (PREV_CHAR != EOF) {
		b = PREV_CHAR;
		PREV_CHAR = EOF;
		goto have_char;
	}

	while (b != EOL && (b = ReadByte()) != EOF) {
have_char:
		// EOL found
		if (b == 10 || b == 13) {
			b2 = ReadByte();
			if (b2 != (b ^ (13 ^ 10))) {
				PREV_CHAR = b2;
			}
			b = EOL;
		}
		LINE[LINE_LEN++] = b;
		if (LINE_LEN == MAX_LINE_LEN) break;
	}
	LINE[LINE_LEN] = 0;
	LINE_POS = 0;
	LINE_NO++;
	return LINE_LEN > 0;
}

static int GetChar()
{
	if (LINE_POS == LINE_LEN) {
		if (!ReadLine()) return EOF;
	}
	return LINE[LINE_POS++];
}

static void UnGetChar(int c)
{
	LINE_POS--;
}

Bool Spaces()
/*
Purpose:
	Return true, if no non-space character directly follows current token.
	EOF is condidered space too (in the logic, that no text follows the token directly.
*/
{
	int c;
	c = GetChar();
	UnGetChar(c);
	return c == SPC || c == TAB || c == EOL || c == EOF;
}

UInt16 SkipSpaces()
/*
	Skip spaces and compute indent.
*/
{
	Int16 c;
	UInt16 tabs, spaces, indent;

retry:
	c = GetChar();
	tabs = 0; spaces = 0;
	do {
		switch(c) {
		case ' ': spaces++; break;
		case '\t':
			if (TOK == TOKEN_EOL && spaces>1) {
				SyntaxError("spaces before tab");
				return 0;
			}
			spaces = 0;
			tabs++;
			break;
		// Ignore completely empty lines, if there was nothing on the line
		// Otherwise reset indent (empty line can not modify indent)
		case EOL:
			if (TOK != TOKEN_EOL) goto done;
			goto retry;
		// Comment
		case ';':
			do {
				c = GetChar();		
			} while(c != EOL && c != EOF);

			// If previous token was EOL, this is line completely with comment, so swallow it including the EOL
			// Otherwise we return EOL token, because it belongs to some items already at line
			if (TOK == TOKEN_EOL) goto retry;
		default:
			goto done;
		}
		c = GetChar();
	} while(true);

done:
	// If there are no more tokens, return EOF
	// TODO: Return TOKEN_OUTDENT, if there is block open with indent

	UnGetChar(c);

	indent = tabs * 256 + spaces;
	return indent;
}

void EnterBlockWithStop(Token stop_token, Token first_token)
{
	Token end_token = TOKEN_EOL;
	Token prev_end_token;

	Bool next_token = true;

	if (TOK == TOKEN_OPEN_P) {
		end_token = TOKEN_CLOSE_P;
		stop_token = TOKEN_VOID;
	} else if (TOK == TOKEN_EOL) {
		end_token = TOKEN_OUTDENT;
	} else if (TOK == TOKEN_BLOCK_END) {
		// We currently have an end of block
		// This may be in situation, where we have line block followed by indent block.
		// In such situation, the line end block should be postponed after the end of
		// indent block.
		
		//  const { enum
		//     { id = 1
		//  }}

		prev_end_token = BLK[BLK_TOP + 1].end_token;
		if (prev_end_token == TOKEN_EOL) {
			if (NEXT_TOK == TOKEN_VOID) {
				BLK_TOP++;			// return state before the block has been ended
				BLK[BLK_TOP].end_token = TOKEN_BLOCK_END;	// mark block as once closed
				TOK = TOKEN_EOL;
				end_token = TOKEN_OUTDENT;
			} else {
				BLOCK_END_CNT++;
				return;
			}
		} else if (prev_end_token == TOKEN_CLOSE_P) {

			// We are at the ending of parenthesised block, if there would be new block
			// now, it would have to be BEFORE the parenthesized block has ended.
			// Therefore we just return TOKEN_BLOCK_END.

			BLOCK_END_CNT++;
			return;
		} else {
			SyntaxError("Unexpected token BLOCK_END in EnterBlock");
		}
//		BLOCK_END_CNT++;
//		return;
	} else {
		next_token = false;
		// Block may be immediatelly terminated by stop token (may be empty)
		// In such case, do not even create the block
		if (TOK == stop_token) {
			NEXT_TOK = TOK;
			TOK = TOKEN_BLOCK_END;
			return;
		}
	}

	BLK_TOP++;
	BLK[BLK_TOP].indent = BLK[BLK_TOP-1].indent;
	if (end_token == TOKEN_OUTDENT) BLK[BLK_TOP].indent = UNDEFINED_INDENT;
	BLK[BLK_TOP].end_token  = end_token;
	BLK[BLK_TOP].stop_token  = stop_token;

	// NextToken MUST be called AFTER new block  has been created 
	// (so it may possibly immediatelly exit that block, if it is empty)

	if (next_token) NextToken();
}

void EnterBlock()
{
	EnterBlockWithStop(TOKEN_VOID, TOKEN_VOID);
}

void ExitBlock()
/*
Purpose:
	Exit block manually (from code) even if there has not been ending token.
*/
{
	BLK_TOP--;
}

void NextToken()
{
	Int16 c, c2, n;
	UInt16 indent;

	// If there are some stacked block ends, return next of them
	if (BLOCK_END_CNT > 0) {
stock_block_end:
		BLOCK_END_CNT--;
		TOK = TOKEN_BLOCK_END;
		return;
	}

	// If we have some token in stock, return this token
	if (NEXT_TOK != TOKEN_VOID) {
		TOK = NEXT_TOK;
		NEXT_TOK = TOKEN_VOID;
		return;
	}

	*LEX.name = 0;
	TOKEN_POS = LINE_POS;

	//TODO: Should not be block_end before next token?

	if (BLK[BLK_TOP].end_token == TOKEN_VOID) {
		TOK = BLK[BLK_TOP].stop_token;
		BLK_TOP--;
		return;
	}

	if (BLK[BLK_TOP].end_token == TOKEN_BLOCK_END) {
		if (BLK[BLK_TOP].stop_token != TOKEN_VOID) {
			BLK[BLK_TOP].end_token = TOKEN_VOID;
		} else {
			BLK_TOP--;
		}
		TOK = TOKEN_BLOCK_END;
		return;
	}

	indent = SkipSpaces();

	// We only solve indent at new line or end of file

	if (TOK == TOKEN_EOL) {
		if (BLK[BLK_TOP].end_token == TOKEN_OUTDENT) {
			if (BLK[BLK_TOP].indent == UNDEFINED_INDENT) {
				// End current block & other blocks too

				if (indent <= BLK[BLK_TOP-1].indent) {
					BLOCK_END_CNT++;
					BLK_TOP--;
					goto outdent;
				}
				BLK[BLK_TOP].indent = indent;
			} else {
outdent:
				// indent decreases agains current indent
				while (indent < BLK[BLK_TOP].indent && BLK[BLK_TOP].end_token == TOKEN_OUTDENT) {
					BLK_TOP--;
					BLOCK_END_CNT++;
				}
				if (BLOCK_END_CNT > 0) goto stock_block_end;
			}
		}
	}

	c = GetChar();		// non-space char

	if (c == EOF && BLK[BLK_TOP].end_token == TOKEN_OUTDENT) {
		indent = 0;
		goto outdent;
	}

	if (isalpha(c) || c == '_' || c == '\'') {
		n = 0;
		c2 = 0; if (c == '\'') c2 = c;
		do {
			if (n == 255) {
				SyntaxError("identifier too long");
				return;
			}
			LEX.name[n++] = c;
			c = GetChar();
			if (c == c2) break;
		} while(isalpha(c) || isdigit(c) || c == '_' || c == '\'');
		LEX.name[n] = 0;
		if (c != c2) UnGetChar(c);

		TOK = TOKEN_ID;

		if (c2 != L'\'' && !LEX.ignore_keywords) {
			n = 0;
			for(n = 0; n < sizeof(keywords)/sizeof(char *); n++) {
				if (StrEqual(keywords[n], LEX.name)) {
					TOK = TOKEN_KEYWORD + n;
					break;
				}
			}
		}

	// $fdab  hex number
	} else if (c == '$') {
		LEX.n = 0;
		do {
			c = GetChar();
			if (isdigit(c)) {
				n = c - '0';
			} else if (c >= 'a' && c<='f') {
				n = c - ('a' - 10);
			} else if (c >= 'A' && c<='F') {
				n = c - ('A' - 10);
			} else if (c == '\'') {
				continue;
			} else {
				UnGetChar(c);
				break;
			}
			LEX.n *= 16;
			LEX.n += n;
		} while(true);
		TOK = TOKEN_INT;

	// %10  bin number
/*	} else if (c == '%') {
		LEX.n = 0;
		do {
			c = GetChar();
			if (c == '0' || c == '1') {
				n = c - '0';
			} else if (c == '\'') {
				continue;
			} else {
				UnGetChar(c);
				break;
			}
			LEX.n *= 2;
			LEX.n += n;
		} while(true);
		TOK = TOKEN_INT;
*/		
	} else if (isdigit(c) /*|| c == '-'*/) {
		LEX.n = 0;
		while (isdigit(c) || c == '\'') {
			if (c != '\'') {
				LEX.n *= 10;
				LEX.n += c - '0';
			}
			c = GetChar();
		}
		UnGetChar(c);
		TOK = TOKEN_INT;

	// String
	} else if (c == '\"') {
		n = 0;
		do {
			if (n == 255) {
				SyntaxError("string too long");
				return;
			}
			c2 = GetChar();
			if (c2 == '\"') break;
			LEX.name[n++] = c2;
		} while(1);
		LEX.name[n] = 0;
		TOK = TOKEN_STRING;

	} else {
//symbol:
		c2 = GetChar();
		if (c == '.' && c2 == '.') {
			TOK = TOKEN_DOTDOT;
		} else if (c == '<' && c2 == '=') {
			TOK = TOKEN_LOWER_EQUAL;
		} else if (c == '>' && c2 == '=') {
			TOK = TOKEN_HIGHER_EQUAL;
		} else if (c == '<' && c2 == '>') {
			TOK = TOKEN_NOT_EQUAL;
		} else if (c == '-' && c2 == '>') {
			TOK = TOKEN_RIGHT_ARROW;
		} else {
			TOK = c;
			LEX.name[0] = c;
			LEX.name[1] = 0;
			UnGetChar(c2);
		}
	}

	// Stop token will be returned on next call, now end block and return TOKEN_BLOCK_END
	if (TOK == BLK[BLK_TOP].stop_token) {
		NEXT_TOK = TOK;
		TOK = TOKEN_BLOCK_END;
		BLK_TOP--;

	// Block end token is replaced by block end
	} else if (TOK == BLK[BLK_TOP].end_token) {
		TOK = TOKEN_BLOCK_END;
		BLK_TOP--;
	}

}

Bool SrcOpen(char * name)
{
	int c;
	
	PREV_CHAR = EOF;

	BLK_TOP++;
	BLK[BLK_TOP].end_token = TOKEN_EOF;
	BLK[BLK_TOP].indent    = 0;

	TOK = TOKEN_EOL;
	NEXT_TOK = TOKEN_VOID;

    LEX.f = fopen(name, "rb");
	if (LEX.f != NULL) {
		LINE_NO   = 0;
		LINE_POS  = 0;
		LINE_LEN  = 0;
		LEX.filename = name;
		PREV_LINE = NULL;

		// Skip UTF-8 BOM

		c = ReadByte();
		if (c == 239) {
			c = ReadByte();
			c = ReadByte();
		} else {
			PREV_CHAR = c;
		}
	} else {
		BLK_TOP--;
        InternalError("open file ");
		return false;
    }
	return true;
}

Bool NextIs(Token tok)
{
	if (TOK != tok) return false;
	NextToken();
	return true;
}


void ExpectToken(Token tok)
{
	if (TOK != TOKEN_ERROR) {
		if (tok == TOKEN_ID) {
			LEX.ignore_keywords = true;
		}
		NextToken();
		LEX.ignore_keywords = false;

		if (TOK != tok) {			
			SyntaxError("unexpected token");
		}
	}
}

void SrcClose()
{
	if (LEX.f != NULL) {
		fclose(LEX.f);
		LEX.f = 0;
		BLK_TOP = 1;
	}
}

void LexerInit()
{
	BLK_TOP = 1;
	NEXT_TOK = TOKEN_VOID;
	BLK[BLK_TOP].end_token = TOKEN_EOF;
	BLK[BLK_TOP].indent    = 0;
	BLK[BLK_TOP].stop_token = TOKEN_VOID;
}
