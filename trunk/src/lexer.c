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
	Token      stop_token;  // Stop token may alternatively end the block.
	                        // It is specified by caller (if not TOKEN_VOID). 
	                        // For example TOKEN_THEN, TOKEN_ELSE etc. Stop token will be returned by parser after block end.
	LineIndent indent;		// Indent of the block. For TOKEN_OUTDENT, any indent smaller than this will end the block.

} BlockStyle;


char PROJECT_DIR[MAX_PATH_LEN];
char SYSTEM_DIR[MAX_PATH_LEN];
char FILE_DIR[MAX_PATH_LEN];			// directory where the current file is stored
char FILENAME[MAX_PATH_LEN];

GLOBAL Var *  SRC_FILE;					// current source file

GLOBAL char   LINE[MAX_LINE_LEN+2];		// current buffer
GLOBAL char * PREV_LINE;				// previous line
GLOBAL LineNo  LINE_NO;
GLOBAL UInt16  LINE_LEN;
GLOBAL UInt16  LINE_POS;				// index of next character in line
static LineIndent     LINE_INDENT;
static int     PREV_CHAR;

GLOBAL UInt16  TOKEN_POS;
GLOBAL Lexer LEX;
GLOBAL Token TOK;						// current token
GLOBAL static BlockStyle BLK[64];		// Lexer blocks (indent, parentheses, line block)
GLOBAL UInt8      BLK_TOP;
GLOBAL char NAME[256];

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

	// Remember current line as previous
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
	// Character following whitespaces on empty line is either EOL, or semicolon (for comment lines).

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
			if (t == TOKEN_CLOSE_P) {
				SyntaxError("missing closing parenthesis");
			}
		}
		return false;
	}

	if (BLK[BLK_TOP].indent == UNDEFINED_INDENT) {
		BLK[BLK_TOP].indent = indent;
		// If the indent is smaller or equal than previous indented block, 
		// this is empty indented block, and must be ended immediately.
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
	EOF is considered space too (in the logic, that no text follows the token directly.
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

	// 2. Block may be enclosed in [ ]
	} else if (TOK == '[') {
		end = ']';

	// 3. Indented block will start at the end of line
	} else if (TOK == TOKEN_EOL) {
		end = TOKEN_OUTDENT;

	// 4. We may be at the end of some block, in such case new block is empty (ends immediately)
	} else if (TOK == TOKEN_BLOCK_END) {
		end = TOKEN_BLOCK_END;
		next_token = false;

	// 5. For other tokens, this is line block
	//    Line block may be alternatively ended by stop_token.
	} else {
		end  = TOKEN_EOL;
		stop = stop_token;
		next_token = false;

		// Block may be immediately terminated by stop token (may be empty)
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
	// (so it may possibly immediately exit that block, if it is empty)

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

static char * keywords[KEYWORD_COUNT] = {
	"goto", "if", "then", "else", "proc", "rule", "macro", "and", "or", "not", "sqrt",
	"while", "until", "where", "const", "enum", "array", "type", "file", "lo", "hi", "of",
	"for", "in", "out", "param", "instr", "times", "adr", "debug", "mod", "bitnot", "bitand", "bitor", "bitxor", "struct", "use", "ref", "step", "return",
	"scope", "sequence", "assert"
	
};

void NextStringToken()
/*
	Parse part of string.

	TOKEN_STRING	String part of string
	[				Beginning of embedded expression
	TOKEN_BLOCK_END	End of string
*/
{
	UInt16 n;
	UInt8 c;

	n = 0;
	do {
		if (n >= 254) {
			SyntaxError("string too long");
			return;
		}

		c = LINE[LINE_POS++];

		if (c == '[') {
			c = LINE[LINE_POS++];
			// [[
			if (c != '[') {
				// ["]
				if (c == '\"' && LINE[LINE_POS] == ']') {
					LINE_POS++;
					goto store;  // we must skip the test for string end
				} else {
					LINE_POS--;
					LINE_POS--;
					break;
				}
			}
		} else if (c == ']') {
			c = LINE[LINE_POS++];
			// ]]
			if (c != ']') {
				SyntaxError("unexpected closing ] in string");				
			}
		} else if (c == '\"') {
			LINE_POS--;
			break;
		}
store:
		NAME[n++] = c;
	} while(1);
	NAME[n] = 0;

	if (TOK != TOKEN_ERROR) {
		if (n != 0) {
			TOK = TOKEN_STRING;
		} else {
			if (LINE[LINE_POS] == '[') {
				LINE_POS++;
				TOK = '[';
			} else {
				LINE_POS++;
				TOK = TOKEN_BLOCK_END;
			}
		}
	}
}

void NextToken()
{
	Int16 c, c2, n;
	UInt16 top;
	UInt16 nest;

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
		// This is end of line, all line blocks in current file should be ended
		for(top = BLK_TOP; top > 0 && BLK[top].end_token != TOKEN_EOF; top--) {
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

	*NAME = 0;

	// Identifier
	if (isalpha(c) || c == '_' || c == '\'') {
		n = 0;
		// Identifier may be closed in ''
		c2 = 0; if (c == '\'') {
			c2 = c;
			c = LINE[LINE_POS++];
		}
		do {
			if (n == 255) {
				SyntaxError("identifier is too long");
				return;
			}
			NAME[n++] = c;
			c = LINE[LINE_POS++];
			if (c == c2) { LINE_POS++; break; }
		} while(c2 != 0 || isalpha(c) || isdigit(c) || c == '_' || c == '\'');
		NAME[n] = 0;
		LINE_POS--;

		TOK = TOKEN_ID;

		if (c2 != L'\'' && !LEX.ignore_keywords) {
			n = 0;
			for(n = 0; n < KEYWORD_COUNT; n++) {
				if (StrEqual(keywords[n], NAME)) {
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
	} else if (c == '%' && (LINE[LINE_POS]=='0' || LINE[LINE_POS]=='1')) {
		LEX.n = 0;
		do {
			c = LINE[LINE_POS++];
			if (c == '0' || c == '1') {
				n = c - '0';
			} else if (c == '\'') {
				continue;
			} else {
				LINE_POS--;
				break;
			}
			LEX.n *= 2;
			LEX.n += n;
		} while(true);
		TOK = TOKEN_INT;
		
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
	// String may contain subexpressions in []
	// ["] defines "
	// [[    [
	// ]]    ]

	} else if (c == '\"') {
		n = 0; nest = 0;
		do {
			if (n >= 254) {
				SyntaxError("string too long");
				return;
			}

			c2 = LINE[LINE_POS++];
str_chr:
			if (c2 == '[') {
				c2 = LINE[LINE_POS++];
				// [[
				if (c2 != '[') {
					// ["]
					if (c2 == '\"' && LINE[LINE_POS] == ']') {
						LINE_POS++;
						goto store;  // we must skip the test for string end
					} else {
						nest++;
						NAME[n++] = '[';
					}
				}
			} else if (c2 == ']') {
				c2 = LINE[LINE_POS++];
				// ]]
				if (c2 != ']') {
					if (nest == 0) {
						SyntaxError("unexpected closing ] in string");
					} else {
						nest--;
						NAME[n++] = ']';
						goto str_chr;
					}
				}
			}
			
			if (nest == 0 && c2 == '\"') break;
store:
			NAME[n++] = c2;
		} while(1);
		NAME[n] = 0;
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
			NAME[0] = c;
			NAME[1] = 0;
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

ParseState * ParseStateLabel()
{
	ParseState * s;

	s = MemAllocStruct(ParseState);
	s->file      = LEX.f;
	s->line      = StrAlloc(LINE);
	s->line_len  = LINE_LEN;
	s->line_no   = LINE_NO;
	s->line_pos  = LINE_POS;
	s->prev_line = PREV_LINE;
	s->token     = TOK;

	return s;
}

void ParseStateGoto(ParseState * s)
{
	if (s != NULL) {
		LEX.f     = s->file;
		strcpy(LINE, s->line);
		LINE_LEN  = s->line_len;
		LINE_NO   = s->line_no;
		LINE_POS  = s->line_pos;
		PREV_LINE = s->prev_line;
		TOK       = s->token;

		MemFree(s->line);
		MemFree(s);
	}
}

FILE * FindFile2(char * name, char * ext, char * base_dir)
{
//	char path[MAX_PATH_LEN];

	FILE * f = NULL;
//	if (*base_dir != 0) {
		strcpy(FILENAME, base_dir);
		strcat(FILENAME, name);
		strcat(FILENAME, ext);
	    f = fopen(FILENAME, "rb");
		if (f != NULL) {
			strcpy(FILE_DIR, base_dir);
		}
//	}
	return f;
}

void PlatformPath(char * path)
{
	char sep[2];
	sep[0] = DIRSEP;
	sep[1] = 0;
	strcpy(path, SYSTEM_DIR);
	strcat(path, "platform");
	strcat(path, sep);
	strcat(path, PLATFORM);
	strcat(path, sep);
}

FILE * FindFile(char * name, char * ext)
{
	FILE * f;
	char sep[2];
	char path[MAX_PATH_LEN];

	sep[0] = DIRSEP;
	sep[1] = 0;

	//%FILEDIR%

	f = FindFile2(name, ext, FILE_DIR);

	//%PROJDIR%/

	if (f == NULL) {
		f = FindFile2(name, ext, PROJECT_DIR);
	}

	// %SYSTEM%/platform/%PLATFORM%/

	if (f == NULL) {
		PlatformPath(path);
		f = FindFile2(name, ext, path);
	}

	// %SYSTEM%/processor/%name%/

	if (f == NULL) {
		strcpy(path, SYSTEM_DIR);
		strcat(path, "processor");
		strcat(path, sep);
		strcat(path, name);
		strcat(path, sep);
		f = FindFile2(name, ext, path);
	}

	// %SYSTEM%/

	if (f == NULL) {
		strcpy(path, SYSTEM_DIR);
		strcat(path, "module");
		strcat(path, sep);
		f = FindFile2(name, ext, path);
	}

	return f;
}

Bool SrcOpen(char * name)
/*
Purpose:
	Instruct lexer to use file with specified name as input.
	After parsing this file, parsing continues with current file.
*/
{
	int c;
	Var * file_var;
	FILE * f;
	char path[MAX_PATH_LEN];
	UInt16 path_len;
	char * filename;

	// When parsing system files, use SYSTEM folder
	if (!SYSTEM_PARSE) {
		strcpy(path, PROJECT_DIR);
	} else {
		strcpy(path, SYSTEM_DIR);
	}
	path_len = StrLen(path);
	filename = path + path_len;
	strcat(filename, name);
	strcat(filename, ".atl");

	// Check, that files are not cyclic dependent
	for(file_var = SRC_FILE; file_var != NULL; file_var = file_var->scope) {
		if (StrEqual(file_var->name, filename)) {
			ErrArg(file_var);
			ErrArg(SRC_FILE);
			SyntaxError("Modules [A] and [B] are trying to use each other.");
			return false;
		}
	}

	// If the file has been already loaded (variable with filename exists), 
	// ignore the load request (do not however report error)

	if (VarFind(filename, 0)) return false;

	// Create new block for the file 
	// File block is ended with TOKEN_EOF and starts with indent 0
	BLK_TOP++;
	BLK[BLK_TOP].end_token = TOKEN_EOF;
	BLK[BLK_TOP].indent    = 0;
	BLK[BLK_TOP].stop_token = TOKEN_VOID;

	// Reference to file is stored in variable of MODE_SRC_FILE

	file_var = VarAlloc(MODE_SRC_FILE, path + path_len, 0);
	file_var->n    = 0;
	file_var->scope = SRC_FILE;
	SRC_FILE = file_var;

	f = FindFile(name, ".atl");

	if (f != NULL) {

		file_var->parse_state = ParseStateLabel();

		PREV_CHAR = EOF;
		LEX.f     = f;
		LINE_NO   = 0;
		LINE_POS  = 0;
		LINE_LEN  = 0;
		LINE_INDENT = 0;
//		LEX.filename = name;
		//TODO: Remove LEX.filename
		PREV_LINE = NULL;
		TOK = TOKEN_EOL;

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
/*
Purpose:
	Finish using current file for parsing and continue with previous file.
*/
{
	if (LEX.f != NULL) {
		fclose(LEX.f);
		LEX.f = 0;
	}
	ParseStateGoto(SRC_FILE->parse_state);
	SRC_FILE = SRC_FILE->scope;
}

void LexerInit()
{
	*FILE_DIR = 0;

	BLK_TOP = 0;
	BLK[BLK_TOP].end_token = TOKEN_VOID;
	BLK[BLK_TOP].indent    = 0;
	BLK[BLK_TOP].stop_token = TOKEN_VOID;
	TOK = TOKEN_VOID;
}
