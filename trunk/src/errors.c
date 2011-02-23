/***************************************************

 Error reporting

 Handle error reporting.
 
 Several classes of errors are recognized:

 - Syntax	 There is some syntactical problem in source code
 - Logic     Logic error in user program. For example type mismatch, index out of bounds etc.
 - Internal  Error, that really should not have happened

 Errors may be categorized by severity, too:

 - Error
 - Warning

 It is possible to specify arguments for error messages using ErrArg method.
 Last error argument has name A, the one before it B etc.
 Up to 26 arguments may be used.

 For example:

 ErrArg(var_a); ErrArg(var_b);
 SyntaxError("Value [A] conflicts with [B].");

****************************************************/

#include <stdio.h>
#include <stdarg.h>
#include "language.h"

GLOBAL UInt32 ERROR_CNT;
GLOBAL UInt32 LOGIC_ERROR_CNT;

#define MAX_STORE 16

UInt16 BOOKMARK_LINE_NO;
UInt16 BOOKMARK_LINE_POS;

#define STDERR stderr

#define MAX_ERR_ARG_COUNT 26
GLOBAL Var * ERR_ARGS[MAX_ERR_ARG_COUNT];
static UInt8 ERR_ARG_POS;

static void ReportError(char * kind, char * text, UInt16 bookmark)
{
	UInt16 i, token_pos, indent, line_cnt;
	char c, * t, * n;
	char * line;
	char buf[2048];
	char * o;
	Var * var;
	
	Bool name = false;
	if (*text == '$') {
		name = true;
		text++;
	}

	i = LINE_NO;
	token_pos = TOKEN_POS;
	if (bookmark != 0) {
		i = BOOKMARK_LINE_NO;
		token_pos = BOOKMARK_LINE_POS;
	}

	line_cnt = 0;
	PrintColor(RED);
	if (SRC_FILE != NULL) {
		indent = fprintf(STDERR, "%s(%d) %s error: ", SRC_FILE->name, i, kind);
	} else {
		indent = fprintf(STDERR, "%s error: ", kind);
	}

	// Format error message
	t = text; o = buf;
	do {
		c = *t++;
		if (c == '[') {
			c = *t++;
			// Get argument and print it's property
			if (c>='A' && c<='Z') {
				c = c - 'A';
				if (c > ERR_ARG_POS) {
					c = ERR_ARG_POS + MAX_ERR_ARG_COUNT - c;
				} else {
					c = ERR_ARG_POS - c;
				}
				var = ERR_ARGS[c-1];
			}

			// Output variable name
			n = var->name;
			*o++ = '\'';
			while(*n != 0) *o++ = *n++;
			*o++ = '\'';

			c = *t++;
			if (c != ']') {
				InternalError("Invalid format of argument in error message %s", text);
			}
		} else if (c == '\n' || c == 0) {
			*o++ = 0;

			if (line_cnt != 0) {
				for(i=0; i<indent; i++) fprintf(STDERR, " ");
			}
			fprintf(STDERR, "%s\n", buf);
			line_cnt++;
			o = buf;
		} else {
			*o++ = c;
		}
	} while(c != 0);

//	*o++ = 0;

//	fprintf(STDERR, "%s", buf);

	if (name) fprintf(STDERR, " \'%s\'", NAME);
	fprintf(STDERR, "\n\n");

	// Print line with error and position of the error on line

	line = NULL;
	if (bookmark == 0) {
		line = LINE;
	} else {
		if (BOOKMARK_LINE_NO == LINE_NO) {
			line = LINE;
		} else if (BOOKMARK_LINE_NO == LINE_NO-1) {
			line = PREV_LINE;
		}
	}

	if (line != NULL) {

		while(*line == SPC || *line == TAB) {
			line++;
			if (token_pos > 0) token_pos--;
		}

		fprintf(STDERR, "%s", line);

		if (token_pos > 0) {
			for(i=0; i<token_pos; i++) {
				c = line[i];
				if (c != 9) c = 32;
				fprintf(STDERR, "%c", c);
			}
			// There can be some spaces ot tabs before at the token pos
			while((c = line[i]) == SPC || c == TAB) {
				fprintf(STDERR, "%c", c);
				i++;
			}
			fprintf(STDERR, "^\n");
		}
	}
	PrintColor(RED+GREEN+BLUE);
}

void SyntaxErrorBmk(char * text, UInt16 bookmark)
{
	ReportError("Syntax", text, bookmark);
	TOK = TOKEN_ERROR;
	ERROR_CNT++;
}

void SyntaxError(char * text)
{
	SyntaxErrorBmk(text, 0);
}

void LogicWarning(char * text, UInt16 bookmark)
{
	ReportError("Logic", text, bookmark);
	LOGIC_ERROR_CNT++;
}

void LogicError(char * text, UInt16 bookmark)
{
	ReportError("Logic", text, bookmark);
	ERROR_CNT++;
}

void InternalError(char * text, ...)
{
	va_list argp;
	PrintColor(RED+LIGHT);
	fprintf(STDERR, "Internal error: ");
	va_start(argp, text);
	vfprintf(STDERR, text, argp);
	va_end(argp);
	if (LINE_NO)
		fprintf(STDERR," (line %d)",LINE_NO);
	fprintf(STDERR,"\n");
	TOK = TOKEN_ERROR;
	ERROR_CNT++;
	PrintColor(RED+GREEN+BLUE);
}

void Warning(char * text)
{
	fprintf(STDERR, "Warning: %s\n", text);
}

UInt16 SetBookmark()
{
	BOOKMARK_LINE_NO = LINE_NO;
	BOOKMARK_LINE_POS = TOKEN_POS;
	return 1;
}

UInt16 SetBookmarkLine(Instr * i)
{
	while(i != NULL && i->op != INSTR_LINE) i = i->prev;
	if (i == NULL) return 0;
	LINE_NO = i->line_no;
	BOOKMARK_LINE_NO  = i->line_no;
	SRC_FILE = i->result;
	strcpy(LINE, i->line);
	BOOKMARK_LINE_POS = 0;
	return 1;
}

void ErrArgClear()
{
	ERR_ARG_POS = 0;
}

void ErrArg(Var * var)
{
	ERR_ARGS[ERR_ARG_POS++] = var;
	if (ERR_ARG_POS == MAX_ERR_ARG_COUNT) ERR_ARG_POS = 0;
}

void InitErrors()
{
	ERROR_CNT = 0;
	LOGIC_ERROR_CNT = 0;
	MemEmptyVar(ERR_ARGS);
	ERR_ARG_POS = 0;
}
