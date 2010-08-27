/*

Emit phase

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"

#if !defined(__UNIX__)
    #include <windows.h>
#endif

FILE * G_OUTPUT;

extern Rule * EMIT_RULES[INSTR_CNT];
extern Bool VERBOSE;
extern Var   ROOT_PROC;

void PrintColor(UInt8 color)
/*
Purpose:
	Change the color of printed text.
*/
{
#if !defined(__UNIX__)
	HANDLE hStdout; 
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
	SetConsoleTextAttribute(hStdout, color);
#endif
}

void EmitChar(char c)
{
	if (VERBOSE) {
		printf("%c", c);
	}
	putc(c, G_OUTPUT);
}

void EmitStr(char * str)
{
	char * s, c;

	if (str != NULL) {
		s = str;	
		while(c = *s++) {		
			if (c == '\'') {
				EmitChar('_');
				c = '_';
			}
			EmitChar(c);
		}
	}
}

void EmitInt(long n)
{
	char buf[32];
	int result = sprintf( buf, "%d", n );
	EmitStr(buf);
}

void EmitVarName(Var * var)
{
	EmitStr(var->name);
	if (var->idx != 0) EmitInt(var->idx-1);
}

void EmitVar(Var * var)
{
	if (var != NULL) {
		if (var->mode == MODE_SRC_FILE) {
			EmitStr(var->name);

		} else if (var->mode == MODE_ELEMENT) {
			if (VarIsStructElement(var)) {
				EmitVar(var->adr);
				EmitStr("+");
				EmitVar(var->var);
			} else {
				InternalError("don't know how to emit array element");
			}
		} else if (var->name != NULL) {
			if (var->mode == MODE_CONST && var->type != NULL && var->type->variant == TYPE_INT && var->type->owner != NULL) {
				EmitVarName(var->type->owner);
				EmitStr("__");
			} else if (var->scope != NULL && var->scope != &ROOT_PROC && var->scope->name != NULL && !VarIsLabel(var)) {
				EmitVarName(var->scope);
				EmitStr("__");
			}
			EmitVarName(var);
		} else {
			switch(var->type->variant) {
			case TYPE_INT: EmitInt(var->n);	break;
			case TYPE_STRING: EmitStr(var->str); break;
			default: break;
			}
		}
	}
}

extern Var * MACRO_ARG[26];

void EmitInstr2(Instr * instr, char * str)
{
	char * s, c;
	s = str;

	if (instr->op == INSTR_LINE) {
		PrintColor(BLUE);
	}

	while(c = *s++) {		
		if (c == '%') {
			c = *s++;
			// INSTR_LINE has special arguments

			if (c >='A' && c<='Z') {
				EmitVar(MACRO_ARG[c-'A']); continue;
			}

			switch(c) {
				case '0': 
					EmitVar(instr->result); continue;
				case '1': 
					if (instr->op != INSTR_LINE) {
						EmitVar(instr->arg1); 
					} else {
						EmitInt(instr->line_no);
					}
					continue;
				case '2': 
					if (instr->op != INSTR_LINE) {
						EmitVar(instr->arg2); 
					} else {
						EmitStr(instr->line);
					}
					continue;
				case '@': break;
				case 't': c = '\t'; break;
			}
		}
		EmitChar(c);
	}
	EmitChar(EOL);
	if (instr->op == INSTR_LINE) {
		PrintColor(RED+GREEN+BLUE);
	}
}

Rule * EmitRule(Instr * instr)
/*
Purpose:
	Find rule that emits code for this instruction.
	May be used to test, wheter specified instruction may be emitted or not.
*/
{
	Rule * rule;
	
	rule = EMIT_RULES[instr->op];
	if (instr->op == INSTR_LINE) return rule;

	for(; rule != NULL; rule = rule->next) {
		if (RuleMatch(rule, instr)) break;
	}
	return rule;
}

extern Bool RULE_MATCH_BREAK;

Bool EmitInstr(Instr * i)
{
	Rule * rule;
	Instr * to;

	rule = EmitRule(i);

	if (rule != NULL) {
		for(to = rule->to->first; to != NULL; to = to->next) {
			EmitInstr2(i, to->arg1->str);
		}
		return true;
	} else {
		InternalError("no rule for translating instruction");
		InstrPrint(i);
		return false;
	}
}

Bool EmitInstrBlock(InstrBlock * blk)
{
	Bool r = true;
	Instr * i;

	if (blk != NULL) {
		for(i = blk->first; r && i != NULL; i = i->next) {
			r = EmitInstr(i);
		}
	}
	return r;
}

Bool EmitProc(Var * proc)
{
	return EmitInstrBlock(proc->instr);
}

Bool EmitOpen(char * filename)
{
	char path[255];
	strcpy(path, PROJECT_DIR);
	strcat(path, filename);
	strcat(path, ".asm");
	G_OUTPUT = fopen(path, "w");
	return true;
}

void EmitClose()
{
	fclose(G_OUTPUT);
	G_OUTPUT = NULL;
}

void EmitLabels()
{
	Var * var, * ov;
	Instr instr;
	Type * type;
	UInt32 n;

	n = 1;
	for(var = VarFirst(); var != NULL; var = VarNext(var)) {
		type = var->type;

//		if (var->idx == 100) {
//			PrintVar(var);
//		}

//		if ((var->name != NULL) && (strcmp("i", var->name) == 0) && (var->scope->mode == MODE_SCOPE) && (var->scope->idx == 6)) {
//			PrintVar(var);
//		}

		if (type != NULL && type->variant == TYPE_ARRAY && var->mode == MODE_CONST) goto skip;
		if (
			   (var->adr != NULL && var->adr->scope != REGSET && (var->mode == MODE_VAR || var->mode == MODE_ARG) && (var->read > 0 || var->write > 0))
			|| (var->mode == MODE_CONST && var->read > 0 && var->name != NULL > 0)
		) {
			instr.op = INSTR_VARDEF;
			instr.result = var;
			instr.arg2 = NULL;
//			instr.arg1.type = TYPE_INT;

			if (var->mode != MODE_CONST) {
				//n = var->adr; 
				ov = var->adr;
			} else {
				n = var->n;
				ov = VarNewInt(n);
			}
//			instr.arg1.type = TYPE_VAR;
			instr.arg1 = ov;
			EmitInstr(&instr);
		}
skip:
		n++;
	}
}

void EmitProcedures()
{
	Var * var;
	Type * type;
	for(var = VarFirst(); var != NULL; var = VarNext(var)) {
		type = var->type;
		if (var->mode != MODE_TYPE && var->mode != MODE_ELEMENT && type != NULL && var->instr != NULL && type->variant == TYPE_PROC) {
			Instr vardef;
			MemEmptyVar(vardef);
			vardef.op = INSTR_PROC;
			vardef.result = var;
			EmitInstr(&vardef);
//			PrintProc(var);
			EmitProc(var);
			vardef.op = INSTR_ENDPROC;
			EmitInstr(&vardef);
		}
	}
}
