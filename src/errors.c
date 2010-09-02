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
	UInt16 i, token_pos;
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

	// Format error message
	t = text; o = buf;
	while((c = *t++) != 0) {
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
		} else {
			*o++ = c;
		}
	}
	*o++ = 0;

	fprintf(STDERR, "%s(%d) %s error:", SRC_FILE->name, i, kind);
	fprintf(STDERR, "%s", buf);

	if (name) fprintf(STDERR, " \'%s\'", LEX.name);
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
			token_pos--;
		}

		fprintf(STDERR, "%s", line);
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

void SyntaxError(char * text)
{
	ReportError("syntax", text, 0);
	TOK = TOKEN_ERROR;
	ERROR_CNT++;
}

void LogicWarning(char * text, UInt16 bookmark)
{
	ReportError("logic", text, bookmark);
	LOGIC_ERROR_CNT++;
}

void LogicError(char * text, UInt16 bookmark)
{
	ReportError("logic", text, bookmark);
	ERROR_CNT++;
}

void InternalError(char * text, ...)
{
	va_list argp;
	fprintf(STDERR, "Internal error: ");
	va_start(argp, text);
	vfprintf(STDERR, text, argp);
	va_end(argp);
	if (LINE_NO)
		fprintf(STDERR," (line %d)",LINE_NO);
	fprintf(STDERR,"\n");
	TOK = TOKEN_ERROR;
	ERROR_CNT++;
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
