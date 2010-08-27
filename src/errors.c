/***************************************************

 Error reporting

 Handle error reporting.
 
 Several classes of errors are recognized:

 - Syntax	 There is some syntactical problem in source code
 - Logic     Logic error in user program. For example type mismatch, index out of bounds etc.
 - Internal  Error, that really should not have happened

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

static void ReportError(char * kind, char * text, UInt16 bookmark)
{
	UInt16 i, token_pos;
	char c;
	char * line;

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

	fprintf(STDERR, "%s(%d) %s error: %s", LEX.filename, i, kind, text);
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

void InitErrors()
{
	ERROR_CNT = 0;
	LOGIC_ERROR_CNT = 0;
}
