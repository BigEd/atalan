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

//#define CHR_INDENT 2		// ASCII Start Of text
//#define CHR_OUTDENT 3		// ASCII End Of text

#define CHR_INDENT '{'		// ASCII Start Of text
#define CHR_OUTDENT '}' 	// ASCII End Of text

#define UNDEFINED_INDENT 65535
Cell * SrcLoad(UInt8 * name);


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

	LineNo     line_no;		// line, at which the block started
} BlockStyle;


char PROJECT_DIR[MAX_PATH_LEN];
char SYSTEM_DIR[MAX_PATH_LEN];
char FILE_DIR[MAX_PATH_LEN];			// directory where the current file is stored
char FILENAME[MAX_PATH_LEN];

GLOBAL Var * MODULES;					// variable with all modules loaded into application (contained in scope)

GLOBAL ParseState * PREV_LEXER;

GLOBAL Cell *  SRC_FILE;					// current source file
SrcLineBlock * SRC_LINE_BLOCK;
UInt16 SRC_LINE_INDEX;
SrcLine * SRC_LINE;
SrcLine * PREV_SRC_LINE;
UInt8 * LINE;				// actual text of SRC_LINE. It is used to shorten the code (so we do not need to say SRC_LINE->txt in lexer).
GLOBAL LineNo LINE_NO;
GLOBAL LinePos  LINE_LEN;
GLOBAL LinePos  LINE_POS;				// index of next character in line
static LineIndent     LINE_INDENT;
static Int16    PREV_CHAR;

GLOBAL LinePos  TOKEN_POS;
GLOBAL Lexer LEX;
GLOBAL Token TOK;						// current token
GLOBAL Token TOK_NO_SPACES;				// if the current token was not preceded by whitespaces, it is copied here
static BlockStyle LEXBLK[64];		// Lexer blocks (indent, parentheses, line block)
GLOBAL UInt8      BLK_TOP;
GLOBAL char NAME[256];
GLOBAL char NAME2[256];
GLOBAL BigInt NUMERATOR;
GLOBAL BigInt DENOMINATOR;

void PrintLex()
{
	UInt16 i;
	char * c;

	for(i=0; i<=BLK_TOP; i++) {
		switch(LEXBLK[i].end_token) {
		case TOKEN_EOL:  c = "LINE"; break;
		case TOKEN_OUTDENT: c = "INDENT"; break;
		case TOKEN_CLOSE_P: c = "("; break;
		case TOKEN_EOF: c = "FILE"; break;
		case TOKEN_BLOCK_END: c = "END"; break;
		default: c = "?"; break;
		}
		if (i==0) c = "ROOT";

		printf("%6s %4d T%dS%d\n", c, LEXBLK[i].line_no, LEXBLK[i].indent/256, LEXBLK[i].indent % 256); 
	}
}

Bool NextLine()
{
	if (SRC_LINE_INDEX == LINES_PER_BLOCK-1) {
next_block:
		if (SRC_LINE_BLOCK != NULL) {
			SRC_LINE_BLOCK = SRC_LINE_BLOCK->next;
		}
		if (SRC_LINE_BLOCK == NULL) return false;
		SRC_LINE_INDEX = 0;
		LINE_NO = SRC_LINE_BLOCK->line_no;
	} else {
		SRC_LINE_INDEX++;
		LINE_NO++;
	}
	PREV_SRC_LINE = SRC_LINE;
	SRC_LINE = &SRC_LINE_BLOCK->lines[SRC_LINE_INDEX];
	LINE = SRC_LINE->txt;
	if (LINE == NULL) goto next_block;
	LINE_LEN = StrLen(LINE);
	LINE_POS = 0;
	return true;
}

SrcLine * PeekNextLine()
{
	SrcLineBlock * blk;
	SrcLine * line = NULL;
	if (SRC_LINE_BLOCK != NULL) {
		if (SRC_LINE_INDEX == LINES_PER_BLOCK-1) {
			blk = SRC_LINE_BLOCK;
			if (blk != NULL) {
				blk = blk->next;
			}
			if (blk != NULL) {
				line = &blk->lines[0];
			}
		} else {
			line = &SRC_LINE_BLOCK->lines[SRC_LINE_INDEX+1];
			if (line->txt == NULL) line = NULL;
		}
	}

	return line;
}

/*******************************************************************

  Characters & lines

********************************************************************/
//$C
/*
static int ReadByte()
{
//	return fgetc(F);
	return 0;
}


static Bool ReadLine()
{
	int b, b2;
	LineIndent indent, prev_indent;
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
	// If there are some other blocks then TOKEN_OUTDENT or TOKEN_EOL, it is an error (missing closing parenthesis)
	if (LINE_LEN == 0) {
		for(top = BLK_TOP; top > 0; top--) {
			t = LEXBLK[top].end_token;
			LEXBLK[top].end_token = TOKEN_BLOCK_END;
			LEXBLK[top].stop_token = TOKEN_VOID;			// !!!! Do not emit stop tokens for automatically ended tokens
			if (t == TOKEN_EOF) break;
			if (t == TOKEN_CLOSE_P) {
				SyntaxError("missing closing parenthesis");
			}
		}
		return false;
	}

	// The indent on this line is smaller than indent of previous block

	top = BLK_TOP;
	if (LEXBLK[BLK_TOP].indent == UNDEFINED_INDENT) top--;
	if (indent < LEXBLK[top].indent) {
		prev_indent = LEXBLK[top].indent;
		while(top > 0) {
			if (LEXBLK[top].indent == indent) break;		// this is O.K., we found the same indent
			// We haven't encountered same indent and this indent is already bigger than our indent
			// We have no chance of finding the same indent
			top--;
		}
		if (top == 0) {
			if (tabs == 0 && (prev_indent % 256) == 0) {
				SyntaxError(">Invalid indent (previous indent is made of tabs, this one of spaces)");
			} else {
				SyntaxError(">Invalid indent");
			}
		}
	}

	if (LEXBLK[BLK_TOP].indent == UNDEFINED_INDENT) {
		LEXBLK[BLK_TOP].indent = indent;
		// If the indent is smaller or equal than previous indented block, 
		// this is empty indented block, and must be ended immediately.
		// We make sure following loop will end this block by making it's indent bigger than current indent.
		if (indent <= LEXBLK[BLK_TOP-1].indent) LEXBLK[BLK_TOP].indent++; //LEXBLK[BLK_TOP].end_token = TOKEN_BLOCK_END;
	}

	// End all indent blocks with indent smaller than actual indent
	for(top = BLK_TOP; top > 0 && (LEXBLK[top].end_token == TOKEN_BLOCK_END || LEXBLK[top].end_token == TOKEN_OUTDENT || LEXBLK[top].end_token == TOKEN_EOL) && LEXBLK[top].indent > indent; top--) {
		LEXBLK[top].end_token = TOKEN_BLOCK_END;
		LEXBLK[top].stop_token = TOKEN_VOID;		// we didn't stop on stop token, so we do not want to return it
	}

	return true;
}
*/
Bool Spaces()
/*
Purpose:
	Return true, if no non-space character directly follows current token.
	EOF is considered space too (in the logic, that no text follows the token directly.
*/
{
	UInt8 c;
	c = LINE[LINE_POS];
	return c == SPC || c == TAB || c == EOL;
}

/***********************************************

 Blocks

************************************************/
//$B

void NewBlock(Token end_token, Token stop_token)
{
	BLK_TOP++;
	LEXBLK[BLK_TOP].indent = LEXBLK[BLK_TOP-1].indent;
	if (end_token == TOKEN_OUTDENT) LEXBLK[BLK_TOP].indent = UNDEFINED_INDENT;
	LEXBLK[BLK_TOP].end_token   = end_token;
	LEXBLK[BLK_TOP].stop_token  = stop_token;
	LEXBLK[BLK_TOP].line_no     = LINE_NO;
}

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

	NewBlock(end, stop);

	// NextToken MUST be called AFTER new block  has been created 
	// (so it may possibly immediately exit that block, if it is empty)

	if (next_token) {
		NextToken();
		if (TOK == TOKEN_BLOCK_END && end == TOKEN_OUTDENT) {
			// This is empty indented block.
			// Lexer has moved to next line, and discovered, there is no indented block.
			// We need to return to that previous line so the next time when someone starts a block, we do not consume tokens we do not want to.
			NewBlock(TOKEN_BLOCK_END, TOKEN_EOL);
		}
	}
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
	// We may need to end block preceding this block, if the current token is block terminator.
	if (LEXBLK[BLK_TOP].end_token == TOK) {
		TOK = TOKEN_BLOCK_END;
		BLK_TOP--;
	}
}

/***********************************************

 Tokens

************************************************/
//$T

BLOCK_TYPE LexNestedBlock(BLOCK_TYPE parent_blk_type)
{
	if (LexSymbol("(")) {
		return BLOCK_BRACES;
	} else if (LexIndent()) {
		return BLOCK_INDENT;
	}

	if (parent_blk_type == BLOCK_INDENT || parent_blk_type == BLOCK_FILE) {
		return BLOCK_LINE;
	}
	return BLOCK_NONE;
}

UInt8 LexBlock()
{
	return LexNestedBlock(BLOCK_FILE);
}

Bool LexBlockSeparator(UInt8 blk_type)
{
	switch(blk_type) {
	case BLOCK_LINE: return  LexSymbol(",") || LexEOL();
	case BLOCK_BRACES: return LexSymbol(",");
	case BLOCK_INDENT: return LexEOL();
	}
	return false;
}

Bool LexBlockEnd(UInt8 blk_type)
{
	switch(blk_type) {
	case BLOCK_FILE: return LexEOF();
	case BLOCK_LINE: return  LexEOF() || LexEOL();
	case BLOCK_BRACES: return LexSymbol(")");
	case BLOCK_INDENT: return LexEOF() || LexOutdent();
	case BLOCK_NONE: return true;
	}
//	if (blk_type == 2) 
	return false;
}

Bool LexEOF()
{
	if (LINE_POS == LINE_LEN) {
		if (PeekNextLine() == NULL) {
			return true;
		}
	}
	return false;
}

Bool LexEOL()
{
	if (LINE_POS == LINE_LEN) {
		NextLine();
		return true;
	}
	return false;	
}

Bool LexSpaces()
{
	UInt8 c;
	Bool cnt = 0;
	c = LINE[LINE_POS];
	while(c == SPC || c == TAB) {
		cnt++;
		LINE_POS++;
		c = LINE[LINE_POS];
	}
	return cnt != 0;
}

Bool LexComment() 
/*** Syntax/Comments
Anything after ::;:: to the end of a line is comment.
*/
{
	LexSpaces();
	if (LINE[LINE_POS] == ';') {
		LINE_POS = LINE_LEN;
		return true;
	}
	return false;
}


Bool LexIndent()
{
	SrcLine * line;
	LexComment();
	if (LINE_POS != LINE_LEN) return false;
	line = PeekNextLine();
	if (line != NULL && *line->txt == CHR_INDENT) {
		NextLine();
		LINE_POS++;
		return true;
	}
	return false;
}

Bool LexOutdent()
{
	LexComment();
	if (LINE[LINE_POS] == CHR_OUTDENT) {
		LINE_POS++;
		return true;
	}
	return false;
}

Bool LexNum(Var ** p_i)
/*** Literals (1)
Numeric literals may be defined using the following notation:

:::::::::::::
65535       dec
$494949     hex
%0101010    bin
:::::::::::::

It is possible to separate parts of a numeric constant by apostrophe.

::::::::::::::::::::::::::::::
65'535
$ff'ff
%0101'0101'0101'1111
::::::::::::::::::::::::::::::

Decimal numbers may be defined using decimal dot.

:::::::::::::::
24.562
:::::::::::::::
*/
{
	BigInt numerator;
	BigInt denominator;
	UInt8 digit_count = 0;
	UInt8 c;
	UInt8 n;

	c = LINE[LINE_POS];
	// $fdab  hex number
	if (c == '$') {
		LINE_POS++;
		IntInit(&numerator, 0);
		IntInit(&denominator, 1);
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
			IntMulN(&numerator, 16);
			IntAddN(&numerator, n);
			digit_count++;
		} while(true);

		if (digit_count == 0) {
			LINE_POS--;
			return false;
		}
	// %10  bin number
	} else if (c == '%') {
		LINE_POS++;
		IntInit(&numerator, 0);
		IntInit(&denominator, 1);
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
			IntMulN(&numerator, 2);
			IntAddN(&numerator, n);
			digit_count++;
		} while(true);

		if (digit_count == 0) {
			LINE_POS--;
			return false;
		}

	// Decimal number (possibly with decimal dot)
	} else if (isdigit(c)) {
		LINE_POS++;
		IntInit(&numerator, 0);
		IntInit(&denominator, 1);
		while (isdigit(c) || c == '\'') {
			if (c != '\'') {
				IntMulN(&numerator, 10);
				IntAddN(&numerator, c - '0');
			}
			c = LINE[LINE_POS++];
		}

		if (c == '.' && isdigit(LINE[LINE_POS])) {
			c = LINE[LINE_POS++];
			while (isdigit(c) || c == '\'') {
				if (c != '\'') {
					IntMulN(&numerator, 10);
					IntAddN(&numerator, c - '0');
					IntMulN(&denominator, 10);
				}
				c = LINE[LINE_POS++];
			}
		}
		LINE_POS--;
	} else {
		return false;
	}
	if (IntEqN(&denominator, 1)) {
		*p_i = IntCell(&numerator);
	} else {
		*p_i = Div(IntCell(&numerator), IntCell(&denominator));
	}
	IntFree(&numerator);
	IntFree(&denominator);
	return true;
}


Bool LexString(UInt8 * p_str)
/*** Literals (2)
Text literals are enclosed in double quotes.
Special characters may be enclosed in square brackets.

::::::::::::::::
"This is text."
"I said: ["]Hello!["]"
:::::::::::::::::::

The following escape sequences are supported:
::::::::::::::::::::
["]  "
[[   [
]]   ]
::::::::::::::::
*/
{
	UInt8 n, nest, c2;

	if (LINE[LINE_POS] == '\"') {
		n = 0; nest = 0;
		LINE_POS++;
		do {
			if (n >= 254) {
				SyntaxError("string too long");
				return false;
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
						p_str[n++] = '[';
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
						p_str[n++] = ']';
						goto str_chr;
					}
				}
			}

			if (nest == 0 && c2 == '\"') break;
	store:
			p_str[n++] = c2;
		} while(1);
		p_str[n] = 0;
		return true;
	}

	// End of line
	return false;
}

Bool LexInt(Var ** p_i)
{
	return LexNum(p_i);
}

Bool LexText(Var ** p_i)
{
	UInt8 buf[1024];
	*p_i = NULL;
	if (LexString(buf)) {
		*p_i = TextCell((char *)buf);
		return true;
	}
	return false;
}

Bool LexId(char * p_name)
/* 
### Identifiers
Identifiers must start with a letter and may contain numbers, underlines and apostrophes.
Identifier may be enclosed in apostrophes. In such case, it may contain any character 
except apostrophe or newline. 

Example:
:::::::::::::
name
x1 x2
x'pos
'RH-'
'else'		; this is identifier, even if else is keyword
x x' x''  ; three different identifiers
'*'
:::::::::::::
*/{
	UInt8 c, n, c2;

	c = LINE[LINE_POS];
	if (isalpha(c) || c == '_' || c == '\'') {
		LINE_POS++;
		n = 0;
		// Identifier may be closed in ''
		c2 = 0; if (c == '\'') {
			c2 = c;
			c = LINE[LINE_POS++];
		}
		do {
			if (n == 255) {
				SyntaxError("identifier is too long");
				return false;
			}
			p_name[n++] = c;
			c = LINE[LINE_POS++];
			if (c == 0) break;
			if (c == c2) { LINE_POS++; break; }
		} while(c2 != 0 || isalpha(c) || isdigit(c) || c == '_' || c == '\'');
		p_name[n] = 0;
		LINE_POS--;
		return true;
	}
	return false;
}

UInt32 UCase(UInt32 c)
{
	if (c >= 'a' && c<='z') return c-('a'+'A');
	return c;
}

Bool LexWord(UInt8 * text)
{
	UInt8 n;
	UInt32 c1, c2;

	LexSpaces();
	n = LINE_POS;
	while(*text != 0) {
		c1 = *text++;
		if (*text == SPC) {
			if (!LexSpaces()) goto no_match;
		}
		c2 = LINE[LINE_POS++];
		if (UCase(c1) != UCase(c2)) goto no_match;
	}
	return true;
no_match:
	LINE_POS = n;
	return false;	
}

Bool LexPeekSymbol(UInt8 * text)
{
	Bool r;
	LinePos pos = LINE_POS;
	r = LexSymbol(text);
	LINE_POS = pos;
	return r;
}

Bool LexSymbol(UInt8 * text)
{
	return LexWord(text);
}

Bool LexSymbol2(UInt8 * text, UInt8 * text2)
{
	return LexWord(text) || LexWord(text2);
}

Bool LexPrefix(UInt8 * text)
{
	LinePos l = LINE_POS;
	if (LexSymbol(text) && !LexSpaces()) return true;
	LINE_POS = l;
	return false;
}

static char * keywords[KEYWORD_COUNT] = {
	"goto", "if", "unless", "then", "else", "fn", "rule", "macro", "and", "or", "not", "sqrt",
	"while", "until", "where", "const", "enum", "array", "type", "file", "lo", "hi", "of",
	"for", "in", "out", "param", "instr", "times", "adr", "debug", "mod", "bitnot", "bitand", "bitor", "\xd7", "bitxor", "struct", "use", "ref", "step", "return",
	"sequence", "assert", "either", "string"
	
};

UInt16 LexStringPart(UInt8 * text, UInt8 max_len)
/*
Purpose:
	Parse part of string.
	Stop on [ or end of string (so the next text is [ or ") or when string contains max_len characters.
Special Sequences:
	[[			Represents [
	]]			Represents ]
	[unparsed]  The part in square braces should be parsed by caller

Result:
	Return number of characters in resulting string.
	If 0, there was no constant text.

*/
{
	UInt16 n;
	UInt8 c;

	n = 0;
	while (n<max_len) {
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
				return n;
			}
		} else if (c == '\"') {
			LINE_POS--;
			break;
		}
store:
		text[n++] = c;
	};
	text[n] = 0;
	return n;
}

void NextToken()
{
	UInt16 top;
	UInt16 nest;
	UInt8 c, c2, c3, n;
	Bool spaces = false;

	FAILURE("To Remove");
	TOK_NO_SPACES = TOKEN_VOID;

	// If there are some ended blocks, return TOKEN_BLOCK_END and exit the block
	// For blocks ended with stop token, return the stop_token instead
retry:
	if (LEXBLK[BLK_TOP].end_token == TOKEN_BLOCK_END) {
		TOK = LEXBLK[BLK_TOP].stop_token;
		if (TOK == TOKEN_VOID) TOK = TOKEN_BLOCK_END;
		BLK_TOP--;
		return;
	}

	// If it is necessary to read next line, read the line
	// Line reading may have closed some blocks, so we go to routine start

	if (LINE_POS == LINE_LEN) {

		NextLine();

		// This is end of line, all line blocks in current file should be ended
		for(top = BLK_TOP; top > 0 && LEXBLK[top].end_token == TOKEN_EOL || LEXBLK[top].end_token == TOKEN_BLOCK_END; top--) {
			if (LEXBLK[top].end_token == TOKEN_EOL) {
				LEXBLK[top].end_token = TOKEN_BLOCK_END;
				LEXBLK[top].stop_token = TOKEN_VOID;
			}
		}


		goto retry;
	}

	// Skip spaces
	// These spaces are in the middle of the line, so we simply skip them, as they do not affect indent

	while((c = LINE[LINE_POS]) == SPC || c == TAB) { spaces = true; LINE_POS++; }

	// We have first character of the next token.
	// Remember it's position on line

	// Comment (skip characters to the end of line, EOL is still used)
	if (c == ';') {
		LINE_POS = LINE_LEN-1;
		c = EOL;
	}

	TOKEN_POS = LINE_POS;
	LINE_POS++;

	// --- is special block separating indented blocks
	// Current indented block should have been ended due to smaller indent, so we should return the end of block,
	// then this token should be returned (but no other end of blocks!)
/*
	if (LINE[LINE_POS] == '-' && LINE[LINE_POS+1] == '-' && LINE[LINE_POS+2] == '-') {
		LINE_POS += 3;
		while(LINE[LINE_POS] == '-') LINE_POS++;
//		LEXBLK[BLK_TOP].end_token = TOKEN_BLOCK_END;
//		LEXBLK[BLK_TOP].stop_token = TOKEN_HORIZ_RULE;
//		TOK = TOKEN_BLOCK_END;
		return;
	}
*/
	*NAME = 0;

	if (c == '-' && LINE[LINE_POS] == '-' && LINE[LINE_POS+1] == '-') {
		LINE_POS += 2;
		while(LINE[LINE_POS] == '-') LINE_POS++;
		TOK = TOKEN_HORIZ_RULE;
	// Identifier
	} else if (isalpha(c) || c == '_' || c == '\'') {
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
		IntInit(&NUMERATOR, 0);
		IntInit(&DENOMINATOR, 1);
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
			IntMulN(&NUMERATOR, 16);
			IntAddN(&NUMERATOR, n);
		} while(true);
		TOK = TOKEN_INT;

	// %10  bin number
	} else if (c == '%' && (LINE[LINE_POS]=='0' || LINE[LINE_POS]=='1')) {
		IntInit(&NUMERATOR, 0);
		IntInit(&DENOMINATOR, 1);
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
			IntMulN(&NUMERATOR, 2);
			IntAddN(&NUMERATOR, n);
		} while(true);
		TOK = TOKEN_INT;
		
	// Decimal number
	} else if (isdigit(c)) {
		IntInit(&NUMERATOR, 0);
		IntInit(&DENOMINATOR, 1);
		while (isdigit(c) || c == '\'') {
			if (c != '\'') {
				IntMulN(&NUMERATOR, 10);
				IntAddN(&NUMERATOR, c - '0');
			}
			c = LINE[LINE_POS++];
		}
		TOK = TOKEN_INT;

		if (c == '.' && isdigit(LINE[LINE_POS])) {
			c = LINE[LINE_POS++];
			while (isdigit(c) || c == '\'') {
				if (c != '\'') {
					IntMulN(&NUMERATOR, 10);
					IntAddN(&NUMERATOR, c - '0');
					IntMulN(&DENOMINATOR, 10);
				}
				c = LINE[LINE_POS++];
			}
		}
		LINE_POS--;

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

		TOK = 0;

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
		}

		if (TOK == 0 && c2 != 0) {
			c3 = LINE[LINE_POS++];
			if (c == 0xE2 && c2 == 0x89 && c3 == 0xA4) {
				TOK = TOKEN_LOWER_EQUAL;
			} else if (c == 0xE2 && c2 == 0x89 && c3 == 0xA5) {
				TOK = TOKEN_HIGHER_EQUAL;
			} else if (c == 0xE2 && c2 == 0x89 && c3 == 0xA0) {
				TOK = TOKEN_NOT_EQUAL;
			} else if (c == 0xE2 && c2 == 0x88 && c3 == 0x9A) {
				TOK = TOKEN_SQRT;
			} else if (c == 0xE2 && c2 == 0x86 && c3 == 0x92) {
				TOK = TOKEN_RIGHT_ARROW;
			} else if (c == '-' && c2 == '-' && c3 == '-') {
				TOK = TOKEN_HORIZ_RULE;
			}

			if (TOK == 0) {
				LINE_POS--;
			}			
		}

		if (TOK == 0) {
			TOK = c;
			NAME[0] = c;
			NAME[1] = 0;
			LINE_POS--;
		}
	}

	// Stop token will be returned on next call, now end block and return TOKEN_BLOCK_END
	if (TOK == LEXBLK[BLK_TOP].stop_token) {
		TOK = TOKEN_BLOCK_END;
		LEXBLK[BLK_TOP].end_token = TOKEN_BLOCK_END;

	// Block end token is replaced by block end
	} else if (TOK == LEXBLK[BLK_TOP].end_token) {
		BLK_TOP--;
		TOK = TOKEN_BLOCK_END;
	}

	if (!spaces) TOK_NO_SPACES = TOK;
}

Bool NextCharIs(UInt8 chr)
{
	if (LINE[LINE_POS] != chr) return false;
	LINE_POS++;
	return true;
}

Bool NextIs(Token tok)
{
	if (TOK != tok) return false;
	NextToken();
	return true;
}

Bool NextNoSpaceIs(Token tok)
{
	if (TOK_NO_SPACES != tok) return false;
	NextToken();
	return true;
}

void ExpectToken(Token tok)
{
	ifok {
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

Bool ParsingSystem()
{
	return SRC_FILE == NULL || FlagOn(SRC_FILE->type->submode, SUBMODE_SYSTEM);
}

void ParseStatePush()
{
	ParseState * s;
	s = MemAllocStruct(ParseState);
	s->file      = SRC_FILE;
	s->line      = SRC_LINE;
	s->prev_line = PREV_SRC_LINE;
	s->line_len  = LINE_LEN;
	s->line_no   = LINE_NO;
	s->line_pos  = LINE_POS;
	s->token     = TOK;
	s->prev_char = PREV_CHAR;
	s->parent    = PREV_LEXER;
	PREV_LEXER   = s;
}

void ParseStatePop()
{
	ParseState * s = PREV_LEXER;
	if (s != NULL) {
		PREV_LEXER = s->parent;
		SRC_FILE   = s->file;
		SRC_LINE   = s->line;
		PREV_SRC_LINE = s->prev_line;
		LINE_LEN  = s->line_len;
		LINE_NO   = s->line_no;
		LINE_POS  = s->line_pos;
		TOK       = s->token;
		PREV_CHAR = s->prev_char;
		MemFree(s);
	}
}

FILE * FindFile2(char * base_dir, char * name, char * ext)
{
	FILE * f = NULL;
	strcpy(FILENAME, base_dir);
	strcat(FILENAME, name);
	if (ext != NULL) strcat(FILENAME, ext);
	f = fopen(FILENAME, "rb");
	if (f != NULL) {
		strcpy(FILE_DIR, base_dir);
	}
	return f;
}

void SystemSubdir(char * path, char * subdir)
{
	char sep[2];
	sep[0] = DIRSEP;
	sep[1] = 0;

	strcpy(path, SYSTEM_DIR);
	strcat(path, subdir);
	strcat(path, sep);

}

FILE * FindFile(char * name, char * ext, char * path)
{
	FILE * f;
	char sep[2];

	sep[0] = DIRSEP;
	sep[1] = 0;

	*path = 0;

	//%FILEDIR%

	strcpy(path, FILE_DIR);
	f = FindFile2(path, name, ext);

	//%PROJDIR%/

	if (f == NULL) {
		strcpy(path, PROJECT_DIR);
		f = FindFile2(path, name, ext);
	}

	// %SYSTEM%/platform/%PLATFORM%/

	if (f == NULL) {
		SystemSubdir(path, "platform");
		if (*PLATFORM != 0) {
			strcat(path, PLATFORM);
		} else {
			strcat(path, name);
		}
		strcat(path, sep);
		f = FindFile2(path, name, ext);
		if (f != NULL) {
			if (*PLATFORM == 0) {
				strcpy(PLATFORM, name);
			}
		}
	}

	// %SYSTEM%/cpu/%name%/

	if (f == NULL) {
		SystemSubdir(path, "cpu");
		strcat(path, name);
		strcat(path, sep);
		f = FindFile2(path, name, ext);
	}

	// %SYSTEM%/

	if (f == NULL) {
		SystemSubdir(path, "module");
		f = FindFile2(path, name, ext);
	}

	return f;
}

// *** Module parameters (1)
// When using module using USE command, programmer may specify comma separated list of parameters in the form 'name: value'.
// Parsed parameters are stored as constants in source file variable scope.
// Type of parameter is not known at this moment, so we store whatever value is parsed (integer or string or identifier).

void ParseModuleParameters(BLOCK_TYPE parent_blk_type, Bool SkipOnly)
/*
Purpose:
	Parse module arguments.
Syntax:
	{ name ":" value ["," name ":" value]* }
*/
{
	char opt_name[256];
	Var * param, * val;
	BLOCK_TYPE blk_type;

	blk_type = LexNestedBlock(parent_blk_type);
	while(OK && !LexBlockEnd(blk_type)) {
		if (LexId(opt_name)) {
			if (LexSymbol(":")) {
				// TODO: Parse expression
				if (LexNum(&val) || LexText(&val)) {
					if (!SkipOnly) {
						param = NewVar(SRC_FILE, opt_name, val);
					}
				}
			}
		}
		LexBlockSeparator(blk_type);
	}
}

/*

+----------------+
| INSTR_VAR      |
|                |
|                |
+----------------+
        |
        |     +----------------+        +----------------+
		|     | INSTR_SRC_FILE |        | SrcLineBlock   |
		+---->|                |------->|                |
		      |                |        |                |
		      +----------------+        +----------------+
                                           |next
										   v
                                        +----------------+
		                                | SrcLineBlock2  |
		                                |                |
		                                |                |
		                                +----------------+
										   |next
										   v
										  ...
*/


Bool SrcOpen(char * name, UInt16 blk_type, Bool parse_options)
/*
Purpose:
	Instruct lexer to use file with specified name as input.
	After parsing this file, parsing continues with current file.
*/
{
	Var * file_var;

	// When parsing system files, use SYSTEM folder
	// Build the file name to compare for duplicity.

	file_var = SrcLoad(name);
	if (file_var == NULL) {
		if (parse_options) ParseModuleParameters(blk_type, true);
		return false;
	} else {

		// Create new block for the file 
		// File block is ended with TOKEN_EOF and starts with indent 0
		BLK_TOP++;
		LEXBLK[BLK_TOP].end_token = TOKEN_EOF;
		LEXBLK[BLK_TOP].indent    = 0;
		LEXBLK[BLK_TOP].stop_token = TOKEN_VOID;

		// Reference to file is stored in variable of INSTR_SRC_FILE


//		if (StrEqualPrefix(path, SYSTEM_DIR, StrLen(SYSTEM_DIR))) {
//			SetFlagOn(file_var->submode, SUBMODE_SYSTEM);
//		}

		if (parse_options) {
			ParseModuleParameters(blk_type, false);
		}

		ParseStatePush();
		SRC_FILE = file_var;
		SRC_LINE_BLOCK = SRC_FILE->type->src_file.lines;
		SRC_LINE = &SRC_LINE_BLOCK->lines[0];
		SRC_LINE_INDEX = 0;
		LINE_NO = 1;
		PREV_SRC_LINE = NULL;
		LINE = SRC_LINE->txt;
		LINE_LEN = StrLen(LINE);
		LINE_POS = 0;
	}
	return true;
}

void SrcClose()
/*
Purpose:
	Finish using current file for parsing and continue with previous file.
*/
{
	Token tok;
	tok = TOK;
	ParseStatePop();
	iferr {
		TOK = tok;
	}
}

void LexInit()
{
	*FILE_DIR = 0;
	PREV_LEXER = NULL;
	BLK_TOP = 0;
	LEXBLK[BLK_TOP].end_token = TOKEN_VOID;
	LEXBLK[BLK_TOP].indent    = 0;
	LEXBLK[BLK_TOP].stop_token = TOKEN_VOID;
	TOK = TOKEN_VOID;

	MODULES = NewCell(INSTR_SCOPE);

}

#define MAX_INDENT_NESTING 128

Cell *  SrcLoad(UInt8 * name)
{
	Cell * file = NULL, * file_var = NULL;
	SrcLineBlock * block = NULL, * block2;
	SrcLine * line;
	UInt8 buf[MAX_INDENT_NESTING+MAX_LINE_LEN+2];
	UInt16 indent_stack[MAX_INDENT_NESTING];
	UInt8 indent_level = 0;
	UInt16 indent;
	UInt8 * p, b;
	Bool mixed_spaces;
	UInt16 tabs, spaces,len;
	UInt16 line_idx = LINES_PER_BLOCK;
	UInt32 line_no = 0;
	Bool end_of_file;
	FILE * f;
	char path[MAX_PATH_LEN];
	char * filename;

	f = FindFile(name, ".atl", path);

	len = StrLen(path);
	filename = path + len;
	strcat(path, name);
	strcat(filename, ".atl");

	if (f == NULL) {
		InternalError("could not open file: %s", path);
		return NULL;
	}

	// Check, that files are not cyclic dependent
	for(file_var = SRC_FILE; file_var != NULL; file_var = file_var->scope) {
		if (VarIsNamed(file_var, filename)) {
			ErrArg(file_var);
			ErrArg(SRC_FILE);
			SyntaxError("Modules [A] and [B] are trying to use each other.");
			goto done;
		}
	}
	
	// If the file has been already loaded (variable with filename exists), 
	// ignore the load request (do not however report error)

	if (VarFind(MODULES, filename)) {
		goto done;
	}

	file = NewCell(INSTR_SRC_FILE);
	file->src_file.line_count = 0;
	file->src_file.lines = NULL;

	indent_stack[0] = 0;
	end_of_file = false;
	do {
		buf[MAX_INDENT_NESTING] = 0;
		if (fgets(buf+MAX_INDENT_NESTING, sizeof(buf), f) == NULL) {
			end_of_file = true;
		}

		p = buf+MAX_INDENT_NESTING;

		// Remove whitespaces at the end of line
		len = StrLen(p);
		while(len>0 && p[len-1]==EOL || p[len-1]==SPC || p[len-1]==13) len--;
		p[len] = 0;

		if (line_no == 0) {
			if (*p == 239) {
				p+=3;
			}
		}
		line_no++;

		// Compute indent of the line.
		// Indent is defined by spaces and tabs at the beginning of the line.
		// Spaces and tabs can not be mixed, there must be first tabs, then spaces.
		// If there is exactly one space before a TAB character, it is ignored.
		// Two and more spaces are reported as errors.

		mixed_spaces = false; tabs = 0; spaces = 0;

		while(*p != 0) {
			b = *p;
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
			p++;
		}

		// For empty line, we do not do any special processing (even if it contained some tabs or spaces)
		// Mixed indent is ignored too.
		// One notable exception if empty line generated at the end of file.
		// We process that to make sure that for each indent there is corresponding outdent.

		if (!end_of_file && *p == 0) goto add_line;

		if (mixed_spaces) {
			SyntaxError(">Spaces before tab");
		}
		indent = tabs * 256 + spaces;

		// Indent is either bigger than current indent, which means we have one extra indent level,
		if (indent > indent_stack[indent_level]) {
			indent_level++;
			indent_stack[indent_level] = indent;
			p--; *p = CHR_INDENT;
		// Or smaller or equal, in which case we must find the same indent on stack.
		} else {
			while(indent_level>0 && indent < indent_stack[indent_level]) {
				indent_level--;
				p--; *p = CHR_OUTDENT;
			}

			if (indent != indent_stack[indent_level]) {
				if (tabs == 0 && (indent_stack[indent_level] % 256) == 0) {
					SyntaxError(">Invalid indent (previous indent is made of tabs, this one of spaces)");
				} else {
					SyntaxError(">Invalid indent");
				}						
			}
		}

		// Now we have pointer to line data and indent level.
		// We can add the line to list of lines...
add_line:
		if (line_idx == LINES_PER_BLOCK) {
			block2 = MemAllocStruct(SrcLineBlock);
			block2->line_no = line_no;
			block2->file = file;
			line_idx = 0;
			if (block == NULL) {
				file->src_file.lines = block2;
			} else {
				block->next = block2;
			}
			block = block2;
		}

		line = &block->lines[line_idx];
		line->block = block;
		line->txt = StrAlloc(p);
		line_idx++;
	} while(!end_of_file);

	file_var = NewVar(MODULES, filename, file);
	file->src_file.file_var = file_var;

done:
	fclose(f);
	return file_var;
}
