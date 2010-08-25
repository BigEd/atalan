/*********************************************************

 Lexer

 (c) 2010 Rudolf Kudla 
 Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*********************************************************/

// C characters & lines
// B blocks
// T tokens
// F files

#include "language.h"
#include <ctype.h>

typedef UInt16 LineIndent;

#define UNDEFINED_INDENT 65535

/*

Lexer traces block nesting using stack of BlockStyle structures.
Parser calls function EnterBlock.
At that moment, type of block is detected based on current token.
Block type is stored on the block stack.

*/

typedef struct {

	Token      end_token;   // Token, which will end the block. 
	                        // TOKEN_OUTDENT, TOKEN_CLOSE_P, TOKEN_EOL, TOKEN_BLOCK_END, TOKEN_EOF
	Token      stop_token;  // Stop token may alternativelly end the block.
	                        // It is specified by caller (if not TOKEN_VOID). 
	                        // For example TOKEN_THEN, TOKEN_ELSE etc. Stop token will be returned by parser after block end.
	LineIndent indent;		// Indent of the block. For TOKEN_OUTDENT, any indent smaller than this will end the block.

} BlockStyle;

char PROJECT_DIR[256];						// directory wh
char SYSTEM_DIR[256];

GLOBAL Var *  SRC_FILE;					// current source file
GLOBAL char   LINE[MAX_LINE_LEN+2];		// current buffer
GLOBAL char * PREV_LINE;				// previous line
GLOBAL LineNo  LINE_NO;
GLOBAL UInt16  LINE_LEN;
GLOBAL UInt16  LINE_POS;						// index of next character in line
GLOBAL UInt16  TOKEN_POS;
static LineIndent     LINE_INDENT;

static int     PREV_CHAR;

GLOBAL Lexer LEX;
GLOBAL Token TOK;						// current token
GLOBAL static BlockStyle BLK[64];		// Lexer blocks (indent, parentheses, line block)
GLOBAL UInt8      BLK_TOP;

/*******************************************************************

  Characters & lines

********************************************************************/
//$C

static int ReadByte()
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
	LineIndent indent;
	UInt16 tabs, spaces;
	Bool mixed_spaces;
	UInt16 top;
	Token t;

	// Remeber current line as previous
	// Previous line may be used for error reporting and generating lines into emitted code

	if (PREV_LINE != NULL) {
		free(PREV_LINE);
		PREV_LINE = NULL;
	}
	if (LINE_LEN > 0) {
		PREV_LINE = StrAlloc(LINE);
	}

next_line:
	LINE_LEN = 0;
	b = EOF;

	if (PREV_CHAR != EOF) {
		b = PREV_CHAR;
		PREV_CHAR = EOF;
		goto have_char;
	}

	while ((b = ReadByte()) != EOF) {
have_char:
		// EOL found
		if (b == 10 || b == 13) {
			b2 = ReadByte();
			if (b2 != (b ^ (13 ^ 10))) {
				PREV_CHAR = b2;
			}
			break;
		}
		LINE[LINE_LEN++] = b;
		if (LINE_LEN == MAX_LINE_LEN) break;
	}

	// Terminate the line with EOL
	// Do not do this if we didn't read any characters

	if (b != EOF || LINE_LEN > 0) {
		LINE[LINE_LEN++] = EOL;
	}
	LINE[LINE_LEN] = 0;
	LINE_NO++;

	// Compute indent of the line.
	// Indent is defined by spaces and tabs at the beginning of the line.
	// Spaces and tabs can not be mixed, there must be first tabs, then spaces.
	// If there is exactly one space before a TAB character, it is ignored.
	// Two and more spaces are reported as errors.

	mixed_spaces = false; tabs = 0; spaces = 0;

	for(LINE_POS = 0; LINE_POS < LINE_LEN; LINE_POS++) {
		b = LINE[LINE_POS];
		if (b == SPC) {
			spaces++;
		} else if (b == TAB) {
			// One space before TAB will be ignored. We suppose, that TAB size is at least 2,
			// so one space should not cause any misalignment.
			if (spaces > 1) mixed_spaces = true;
			spaces = 0;
			tabs++;
		} else {
			break;
		}
	}

	// If this is empty line, read next line.
	// Character following whitespaces on empty line is either EOL, or semicolon (for coment lines).

	if (b == EOL || b == ';') goto next_line;

	indent = tabs * 256 + spaces;

	LINE_INDENT = indent;

	if (mixed_spaces) {
		SyntaxError("spaces before tab");
	}

	// No character has been read, this is end of file
	// We have to end all blocks until TOKEN_EOF block
	// If there are some other blocks than TOKEN_OUTDENT or TOKEN_EOL, it is an error (missing closing parenthesis)
	if (LINE_LEN == 0) {
		for(top = BLK_TOP; top > 0; top--) {
			t = BLK[top].end_token;
			BLK[top].end_token = TOKEN_BLOCK_END;
			if (t == TOKEN_EOF) break;
			if (t != TOKEN_EOL && t != TOKEN_OUTDENT) {
				SyntaxError("missing closing parenthesis");
			}
		}
		return false;
	}

	if (BLK[BLK_TOP].indent == UNDEFINED_INDENT) {
		BLK[BLK_TOP].indent = indent;
		// If the indent is smaller or equal than previous indented block, 
		// this is empty indented block, and must be ended immediatelly.
		// We make sure following loop will end this block by making it's indent bigger than current indent.
		if (indent <= BLK[BLK_TOP-1].indent) BLK[BLK_TOP].indent++; //BLK[BLK_TOP].end_token = TOKEN_BLOCK_END;
	}

	// End all indent blocks with indent smaller than actual
	for(top = BLK_TOP; top > 0 && (BLK[top].end_token == TOKEN_BLOCK_END || BLK[top].end_token == TOKEN_OUTDENT) && BLK[top].indent > indent; top--) {
		BLK[top].end_token = TOKEN_BLOCK_END;
	}

	return true;
}

Bool Spaces()
/*
Purpose:
	Return true, if no non-space character directly follows current token.
	EOF is condidered space too (in the logic, that no text follows the token directly.
*/
{
	int c;
	c = LINE[LINE_POS];
	return c == SPC || c == TAB || c == EOL || c == EOF;
}

/***********************************************

 Blocks

************************************************/
//$B

void EnterBlockWithStop(Token stop_token)
{
	Token end  = TOKEN_EOL;
	Token stop = TOKEN_VOID;

	Bool next_token = true;

	// 1. Block may be enclosed in ( )
	if (TOK == TOKEN_OPEN_P) {
		end = TOKEN_CLOSE_P;

	// 2. Indented block will start at the end of line
	} else if (TOK == TOKEN_EOL) {
		end = TOKEN_OUTDENT;

	// 3. We may be at the end of some block, in such case new block is empty (ends immediatelly)
	} else if (TOK == TOKEN_BLOCK_END) {
		end = TOKEN_BLOCK_END;
		next_token = false;

	// 4. For other tokens, this is line block
	//    Line block may be alternativelly ended by stop_token.
	} else {
		end  = TOKEN_EOL;
		stop = stop_token;
		next_token = false;

		// Block may be immediatelly terminated by stop token (may be empty)
		// In such case, do not even create the block
		if (TOK == stop_token) {
			LINE_POS = TOKEN_POS;		// make sure the token is parsed again
			TOK = TOKEN_BLOCK_END;
			return;
		}
	}

	BLK_TOP++;
	BLK[BLK_TOP].indent = BLK[BLK_TOP-1].indent;
	if (end == TOKEN_OUTDENT) BLK[BLK_TOP].indent = UNDEFINED_INDENT;
	BLK[BLK_TOP].end_token   = end;
	BLK[BLK_TOP].stop_token  = stop;

	// NextToken MUST be called AFTER new block  has been created 
	// (so it may possibly immediatelly exit that block, if it is empty)

	if (next_token) NextToken();
}

void EnterBlock()
{
	EnterBlockWithStop(TOKEN_VOID);
}

void ExitBlock()
/*
Purpose:
	Exit block manually (from code) even if there has not been ending token.
*/
{
	BLK_TOP--;
}

/***********************************************

 Tokens

************************************************/
//$T

static char * keywords[] = {
	"goto", "if", "then", "else", "proc", "rule", "macro", "and", "or", "not",
	"while", "until", "where", "const", "enum", "array", "type", "file", "lo", "hi", "of",
	"for", "in", "out", "instr", "times", "adr", "debug", "mod", "xor"
};


void NextToken()
{
	Int16 c, c2, n;
	UInt16 top;

	// If there are some ended blocks, return TOKEN_BLOCK_END and exit the block
	// For blocks ended with stop token, return the stop_token instead
retry:
	if (BLK[BLK_TOP].end_token == TOKEN_BLOCK_END) {
		TOK = BLK[BLK_TOP].stop_token;
		if (TOK == TOKEN_VOID) TOK = TOKEN_BLOCK_END;
		BLK_TOP--;
		return;
	}

	// If it is necessary to read next line, read the line
	// Line reading may have closed some blocks, so we go to routine start

	if (LINE_POS == LINE_LEN) {
		// This is end of line, all line blocks should be ended
		for(top = BLK_TOP; top > 0; top--) {
			if (BLK[top].end_token == TOKEN_EOL) {
				BLK[top].end_token = TOKEN_BLOCK_END;
				BLK[top].stop_token = TOKEN_VOID;
			}
		}
		ReadLine();
		goto retry;
	}

	// Skip spaces
	// These spaces are in the middle of the line, so we simply skip them, as they do not affect indent

	while((c = LINE[LINE_POS]) == SPC || c == TAB) LINE_POS++;

	// We have first character of the next token.
	// Remember it's position on line

	// Comment (skip characters to the end of line, EOL is still used)
	if (c == ';') {
		LINE_POS = LINE_LEN-1;
		c = EOL;
	}

	TOKEN_POS = LINE_POS;
	LINE_POS++;

	*LEX.name = 0;

	// Identifier
	if (isalpha(c) || c == '_' || c == '\'') {
		n = 0;
		// Identifier may be closed in ''
		c2 = 0; if (c == '\'') c2 = c;
		do {
			if (n == 255) {
				SyntaxError("identifier is too long");
				return;
			}
			LEX.name[n++] = c;
			c = LINE[LINE_POS++];
			if (c == c2) { LINE_POS++; break; }
		} while(isalpha(c) || isdigit(c) || c == '_' || c == '\'');
		LEX.name[n] = 0;
		LINE_POS--;

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
			c = LINE[LINE_POS++];
			if (isdigit(c)) {
				n = c - '0';
			} else if (c >= 'a' && c<='f') {
				n = c - ('a' - 10);
			} else if (c >= 'A' && c<='F') {
				n = c - ('A' - 10);
			} else if (c == '\'') {
				continue;
			} else {
				LINE_POS--;
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
	// Decimal number
	} else if (isdigit(c)) {
		LEX.n = 0;
		while (isdigit(c) || c == '\'') {
			if (c != '\'') {
				LEX.n *= 10;
				LEX.n += c - '0';
			}
			c = LINE[LINE_POS++];
		}
		LINE_POS--;
		TOK = TOKEN_INT;

	// String
	} else if (c == '\"') {
		n = 0;
		do {
			if (n == 255) {
				SyntaxError("string too long");
				return;
			}
			c2 = LINE[LINE_POS++];
			if (c2 == '\"') break;
			LEX.name[n++] = c2;
		} while(1);
		LEX.name[n] = 0;
		TOK = TOKEN_STRING;

	// End of line
	} else if (c == EOL) {
		TOK = TOKEN_EOL;
		return;

	// Symbol
	} else {

		c2 = LINE[LINE_POS++];
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
			LINE_POS--;
		}
	}

	// Stop token will be returned on next call, now end block and return TOKEN_BLOCK_END
	if (TOK == BLK[BLK_TOP].stop_token) {
		TOK = TOKEN_BLOCK_END;
		BLK[BLK_TOP].end_token = TOKEN_BLOCK_END;

	// Block end token is replaced by block end
	} else if (TOK == BLK[BLK_TOP].end_token) {
		BLK_TOP--;
		TOK = TOKEN_BLOCK_END;
	}

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


/***********************************************

 Source Files

************************************************/
//$B

Bool SrcOpen(char * name)
{
	int c;
	Var * file_var;
	char path[256];
	UInt16 path_len;
	PREV_CHAR = EOF;

	// Create new block for the file 
	// File block is ended with TOKEN_EOF and starts with indent 0
	BLK_TOP++;
	BLK[BLK_TOP].end_token = TOKEN_EOF;
	BLK[BLK_TOP].indent    = 0;
	BLK[BLK_TOP].stop_token = TOKEN_VOID;

	TOK = TOKEN_EOL;

	if (!SYSTEM_PARSE) {
		strcpy(path, PROJECT_DIR);
	} else {
		strcpy(path, SYSTEM_DIR);
	}
	path_len = StrLen(path);
	strcat(path, name);
	strcat(path, ".atl");

	// Reference to file is stored in variable of MODE_SRC_FILE

	file_var = VarAlloc(MODE_SRC_FILE, path + path_len, 0);
	file_var->n    = 0;
	file_var->scope = SRC_FILE;
	SRC_FILE = file_var;

    LEX.f = fopen(path, "rb");
	if (LEX.f != NULL) {

		LINE_NO   = 0;
		LINE_POS  = 0;
		LINE_LEN  = 0;
		LINE_INDENT = 0;
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
		InternalError("could not open file: %s",path);
		return false;
    }
	return true;
}

void SrcClose()
{
	SRC_FILE = SRC_FILE->scope;
	if (LEX.f != NULL) {
		fclose(LEX.f);
		LEX.f = 0;
	}
}

void LexerInit()
{
	BLK_TOP = 0;
	BLK[BLK_TOP].end_token = TOKEN_VOID;
	BLK[BLK_TOP].indent    = 0;
	BLK[BLK_TOP].stop_token = TOKEN_VOID;
}