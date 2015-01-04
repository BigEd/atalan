/***************************************************

 Error reporting

 Handle error reporting.
 
 Several classes of errors are recognized:

 - Syntax	 There is some syntactical problem in source code
 - Logic     Logic error in user program. For example type mismatch, index out of bounds etc.
 - Internal  Error, that really should not have happened

 Errors may be categorized by severity:

 - Error
 - Warning

 It is possible to specify arguments for error messages using ErrArg method.
 Last error argument has name A, the one before it B etc.
 Up to 26 arguments may be used and referenced in an error message.

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

SrcLine * BOOKMARK_LINE;
UInt16 BOOKMARK_LINE_POS;
Instr * ERR_INSTR;

#define STDERR stderr

#define MAX_ERR_ARG_COUNT 26
GLOBAL Var * ERR_ARGS[MAX_ERR_ARG_COUNT];
static UInt8 ERR_ARG_POS;
FILE * ERR_OLD_DESTINATION;
UInt8 ERR_OLD_COLOR;

void EndErrorReport()
{
	PrintEOL();
	PrintColor(ERR_OLD_COLOR);
	PrintDestination(ERR_OLD_DESTINATION);
}

static void ReportError(char * kind, char * text, Bookmark bookmark)
/*
	text	Text of error message
			If the first character is >, indent will be visualized and column will not be shown
*/
{
	UInt16 i, token_pos, indent, line_cnt, indent_len, end_pos, token_len;
	char c, * t;
	SrcLine * line;
	char * filename;
	char * line_txt;
	char buf[2048];
	char * o;
	Var * var;	
	Bool name = false;
	Bool show_indent = false;
	UInt8 used_args[26];		// list of variables used in error message (in the order of use)
	UInt8 arg_cnt = 0;
	Bool no_rep = false;
	UInt8 color;
	Bool will_continue = false;
	Bool to_here = false;
	UInt16 len;

	if (*text == '^') {
		to_here = true;
		text++;
	}

	if (*text == '$') {
		name = true;
		text++;
	}

	if (*text == '>') {
		show_indent = true;
		text++;
	}

	if (*text == '~') {
		will_continue = true;
		text++;
	}

	line = SRC_LINE;	//i = LINE_NO;
	token_pos = TOKEN_POS;
	if (bookmark != 0) {
		line = BOOKMARK_LINE;
		token_pos = BOOKMARK_LINE_POS;
	}

	i = LineNumber(line);

	// Line text
//	line = NULL;
//	if (bookmark == 0) {
//		line = SRC_LINE;
//	} else {
//		if (BOOKMARK_LINE_NO == LINE_NO) {
//			line = SRC_LINE;
//		} else if (BOOKMARK_LINE_NO == LINE_NO-1) {
//			line = PREV_SRC_LINE;
//		}
//	}

	line_txt = line->txt;
	token_len = 0;
	if (to_here) {
		for(end_pos = TOKEN_POS-1; end_pos>0 && line_txt[end_pos]==' ';) end_pos--;
		token_len = end_pos - token_pos + 1;
	}

	ERR_OLD_DESTINATION = PrintDestination(STDERR);

	// Choose error color based on kind
	color = COLOR_ERROR;
	if (*kind == 'W') color = COLOR_WARNING;
	line_cnt = 0;
	ERR_OLD_COLOR = PrintColor(color);

	filename = LineFileName(line);

	if (filename != NULL) {
		PrintFmt("%s(%d) %s: ", filename, i, kind);
	} else {
		PrintFmt("%s: ", kind);
	}

	indent = 4;

	// Format error message
	t = text; o = buf;
	do {
		c = *t++;
		if (c == '[') {

			// Print buffer we have up to now
			*o++ = 0;
			Print(buf);
			o = buf;

			no_rep = false;
			c = *t++;
			if (c == '-') { c = *t++; no_rep = true; }

			// Get argument and print it's property
			if (c>='A' && c<='Z') {
				c = c - 'A';
				if (c >= ERR_ARG_POS) {
					c = ERR_ARG_POS + MAX_ERR_ARG_COUNT - c;
				} else {
					c = ERR_ARG_POS - c;
				}
				var = ERR_ARGS[c-1];

				// Remember used arguments (in order of use).
				// Prevent duplicates.
				if (!no_rep) {
					for(i=0; i<arg_cnt; i++) if (used_args[i] == c) break;
					if (i == arg_cnt) {
						used_args[arg_cnt++] = c;
					}
				}

				// Output variable name
				if (var->mode != INSTR_VAR) {
					PrintVar(var); //PrintBigInt(&var->n);
				} else {
					PrintQuotedCellName(var);
				}

			} else if (c == '$') {
				Print("\'");
				Print(NAME);
				Print("\'");
			} else if (c == '*') {
				Print("\'");
				Print(OpSymbol(ERR_INSTR->op));
				Print("\'");
			}

			c = *t++;
			if (c != ']') {
				InternalError("Invalid format of argument in error message %s", text);
			}
		} else if (c == '\n' || c == 0) {
			*o++ = 0;
			Print(buf);
			Print("\n");
			o = buf;

			if (line_cnt != 0) {
				PrintRepeat(" ", indent);
			}

			line_cnt++;
		} else {
			*o++ = c;
		}
	} while (c != 0);

//	*o++ = 0;

//	fprintf(STDERR, "%s", buf);

	if (name) PrintFmt(" \'%s\'", NAME);
//	fprintf(STDERR, "\n");

	if (arg_cnt > 0) {
		for(i=0; i<arg_cnt; i++) {
			c = used_args[i];
			var = ERR_ARGS[c-1];
			if (var->line != NULL) {
				PrintRepeat(" ", indent);
				PrintFmt("'%s' was declared at line %s(%d)\n", VarName(var), LineFileName(line), LineNumber(line));
			}
		}

		PrintEOL();
	}

	// Print line with error and position of the error on line

	indent_len = 0;
	if (line_txt != NULL && *line_txt != 0) {

		PrintEOL();
		PrintColor(COLOR_LINE_POS);

		while(*line_txt == SPC || *line_txt == TAB) {
			if (show_indent) {
				if (*line_txt == SPC) Print(".");
				if (*line_txt == TAB) Print("->|");
			}
			line_txt++;
			indent_len++;
			if (token_pos > 0) token_pos--;
		}

		// Print line. If it does contain end new line, print one extra new line.
		Print(line_txt);
		len = StrLen(line_txt);
		if (line_txt[len-1] != '\n') {
			PrintEOL();
		}

		if (!show_indent) {
			if (token_pos > 0) {
				for(i=0; i<token_pos; i++) {
					c = line_txt[i];
					if (c != 9) c = 32;
					PrintChar(c);
				}
				// There can be some spaces or tabs before at the token pos
				while((c = line_txt[i]) == SPC || c == TAB) {
					PrintChar(c);
					i++;
				}
				Print("^");
				i++;
				while(token_len>1) {
					Print("^");
					token_len--;
				}
			}
		}
		PrintEOL();
		PrintColor(color);
	}

	if (!will_continue) EndErrorReport();

}

void SyntaxErrorBmk(char * text, Bookmark bookmark)
{
	ReportError("Syntax error", text, bookmark);
	TOK = TOKEN_ERROR;
	ERROR_CNT++;
}

void SyntaxError(char * text)
{
	SyntaxErrorBmk(text, 0);
}

void LogicWarning(char * text, Bookmark bookmark)
{
	ReportError("Warning", text, bookmark);
	LOGIC_ERROR_CNT++;
}

void LogicWarningLoc(char * text, Loc * loc)
{
	Bookmark bookmark = SetBookmarkLoc(loc);
	LogicWarning(text, bookmark);
}

void LogicErrorLoc(char * text, Loc * loc)
{
	Bookmark bookmark = SetBookmarkLoc(loc);
	LogicError(text, bookmark);
}

void LogicError(char * text, Bookmark bookmark)
{
	ReportError("Logic error", text, bookmark);
	ERROR_CNT++;
}

void InternalError(char * text, ...)
{
	UInt8 old_color;
	char buffer[256];
	va_list argp;
	old_color = PrintColor(COLOR_ERROR);
	Print("Internal error: ");
	va_start(argp, text);
	vsprintf(buffer, text, argp);
	va_end(argp);
	Print(buffer);
	if (LINE_NO) {
		Print("(line "); PrintInt(LINE_NO); Print(")");
	}
	PrintEOL();
	TOK = TOKEN_ERROR;
	ERROR_CNT++;
	PrintColor(old_color);
}

void InternalErrorLoc(char * text, Loc * loc)
{
	Bookmark bookmark = SetBookmarkLoc(loc);
	ReportError("Internal error", text, bookmark);
	TOK = TOKEN_ERROR;
	ERROR_CNT++;
}

void RuntimeError(char * text)
{
	ReportError("Runtime error", text, 0);
	TOK = TOKEN_ERROR;
	ERROR_CNT++;
}

void Warning(char * text)
{
	fprintf(STDERR, "Warning: %s\n", text);
}

Bookmark SetBookmark()
{
	BOOKMARK_LINE = SRC_LINE;
	BOOKMARK_LINE_POS = TOKEN_POS;
	return 1;
}

InstrBlock * PrevBlk(InstrBlock * first, InstrBlock * blk)
{
	InstrBlock * b;

	b = first;
	while(b != NULL && b->next != blk) b = b->next;
	return b;
}

Bookmark SetBookmarkLoc(Loc * loc)
{
	Instr * i = loc->i;
	InstrBlock * blk = loc->blk;

	ERR_INSTR = i;

	do {

		while(i != NULL && i->line == NULL) i = i->prev;
		if (i != NULL) break;

		blk = PrevBlk(loc->proc->instr, blk);
		if (blk == NULL) return 0;
		i = blk->last;
	} while (true);

	BOOKMARK_LINE = i->line;
	BOOKMARK_LINE_POS = i->line_pos;

	return 1;
}

Bookmark SetBookmarkVar(Var * var)
{
	BOOKMARK_LINE = var->line;
	BOOKMARK_LINE_POS = var->line_pos;
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
