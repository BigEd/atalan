/*

Emit phase

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"

#if defined(__Windows__)
    #include <windows.h>
#endif

FILE * G_OUTPUT;

extern Rule * EMIT_RULES[INSTR_CNT];
extern Var   ROOT_PROC;

UInt8 G_COLOR;
UInt8 G_PREV_OP = 0;

UInt8 PrintColor(UInt8 color)
/*
Purpose:
	Change the color of printed text.
*/
{
	UInt8 old_color = G_COLOR;
#ifdef __Windows__
	HANDLE hStdout; 
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
	SetConsoleTextAttribute(hStdout, color);
#endif
	G_COLOR = color;
	return old_color;
}

void Print(char * text)
{
	if (text != NULL) {
		printf("%s", text);
	}
}

void PrintRepeat(char * text, UInt16 cnt)
{
	while(cnt-- > 0) {
		Print(text);
	}
}

void PrintHeader(char * text)
/*
Purpose:
	Print header to output.
*/
{
	UInt16 len, half_len;
	UInt8 color;
	if (text == NULL) text = "";

	len = StrLen(text);
	if (len > 70) {
		len = 2;
	} else {
		len = 70 - len;
	}

	color = PrintColor(RED+GREEN);
	half_len = len / 2;
	PrintRepeat("=", half_len);
	Print(" ");
	Print(text);
	Print(" ");
	PrintRepeat("=", len - half_len);
	Print("\n");
	PrintColor(color);

}

char * G_BUF;		// buffer to output

void EmitOpenBuffer(char * buf)
{
	G_BUF = buf;
}

void EmitCloseBuffer()
{
	*G_BUF++ = 0;
}

void EmitByte(char c)
{
	if (G_BUF != NULL) {
		*G_BUF++ = c;
	} else {
		if (Verbose(NULL)) {
			printf("%c", c);
		}
		putc(c, G_OUTPUT);
	}
}

void EmitChar(char c)
{
	EmitByte(c);
}

void EmitStr(char * str)
{
	char * s, c;

	if (str != NULL) {
		s = str;	
		while(c = *s++) {		
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

void EmitStrConst(char * str)
/*
Purpose:
	Emit string constant.
	This code is currently MADS specific to handle emit of single quotes.
	Quotes are emitted like:  'ahaha', 39, 'sksksks'
*/
{
	char * s, c;
	Bool in_quotes = false;
	Bool empty = true;

	if (str != NULL) {
		s = str;	
		while(c = *s++) {		
			if (c == '\'') {
				if (in_quotes) {
					EmitStr("\'");
					in_quotes = false;
				}
				if (!empty) EmitStr(",");
				EmitStr("39");
				empty = false;
			} else {
				if (!in_quotes) {
					if (!empty) EmitStr(",");
					EmitStr("c\'");
					in_quotes = true;
				}
				EmitByte(c);
				empty = false;
			}
		}
		if (in_quotes) {
			EmitStr("\'");
		}
	}
}

void EmitHex(UInt8 c)
{
	char h[16] = "0123456789abcdef";
	EmitChar(h[(c >> 4) & 0xf]);
	EmitChar(h[c & 0xf]);
}

void EmitVarName(Var * var)
/*
Purpose:
	Emit name of variable.
	Variable name may contain non-alphanumeric characters, so we must translate them to support ordinary assemblers.
	Identifiers starting with digit are prefixed by _N.
	Non-alphanumeric characters are replaced by xNN, when NN are two hexadecimal digits representing ascii code of the character.
*/
{
	char * s, c;

	s = var->name;
	if (s != NULL) {
		// If identifier starts with number, we prefix _N
		c = *s;
		if (c >='0' && c <= '9') {
			EmitChar('_');
			EmitChar('N');
		}

		while(c = *s++) {
			if (c == '\'') {
				EmitChar('_');
				c = '_';
			} if (c == '_' || (c >= 'a' && c <= 'z') || (c>='A' && c<='Z') || (c>='0' && c<='9')) {
				EmitChar(c);
			} else {
				EmitChar('x');
				EmitHex(c);
			}
		}
	}

	// If variable has index, append the index
	if (var->idx != 0) EmitInt(var->idx-1);
}

void EmitVar(Var * var, UInt8 format)
{
	if (var != NULL) {
		if (var->mode == MODE_SRC_FILE) {
			EmitStr(var->name);

		} else if (var->mode == MODE_ELEMENT) {
			if (VarIsStructElement(var)) {
				EmitVar(var->adr, format);
				EmitStr("+");
				EmitVar(var->var, format);
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
			case TYPE_STRING: 
				if (format == 1) {
					EmitStrConst(var->str); 
				} else {
					EmitStr(var->str);
				}
				break;
			default: break;
			}
		}
	}
}

extern Var * MACRO_ARG[26];

void EmitInstr2(Instr * instr, char * str)
{
	Var * var;
	UInt8 format = 0;
	char * s, c;
	UInt32 n;
	s = str;

	if (instr->op == INSTR_LINE) {
		PrintColor(BLUE);
	}

	while(c = *s++) {		
		if (c == '%') {
			c = *s++;
			if (c == '\'') {
				format = 1;
				c = *s++;
			}
			
			if (c >='A' && c<='Z') {
				var = MACRO_ARG[c-'A'];
				// Variable properties
				if (*s == '.') {
					s++;
					if (StrEqualPrefix(s, "elemsize", 8)) {					// TODO: elem.size
						s += 8;
						if (var->type->variant == TYPE_ARRAY) {
							n = TypeSize(var->type->element);
						} else {
							n = 0;
						}
						EmitInt(n);
						continue;
					} else if (StrEqualPrefix(s, "index.min", 9)) {
						s += 9;
						if (var->type->variant == TYPE_ARRAY) {
							n = var->type->dim[0]->range.min;
						} else {
							n = 0;
						}
						EmitInt(n);
						continue;
					}
					s--;
				}
				EmitVar(var, format); continue;
			}

			switch(c) {
				case '0': 
					EmitVar(instr->result, format); continue;
				case '1': 
					if (instr->op != INSTR_LINE) {
						EmitVar(instr->arg1, format); 
					} else {
						EmitInt(instr->line_no);
					}
					continue;
				case '2': 
					if (instr->op != INSTR_LINE) {
						EmitVar(instr->arg2, format); 
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

Rule * EmitRule2(InstrOp op, Var * result, Var * arg1, Var * arg2)
{
	Instr i;
	i.op = op;
	i.result = result;
	i.arg1 = arg1;
	i.arg2 = arg2;
	return EmitRule(&i);
}

extern Bool RULE_MATCH_BREAK;

Bool EmitInstr(Instr * i)
{
	Rule * rule;
	Instr * to;

	if (i->op == INSTR_REF) return true;

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

Bool EmitInstrOp(InstrOp op, Var * result, Var * arg1, Var * arg2)
{
	Instr i;
	MemEmptyVar(i);
	i.op = op;
	i.result = result;
	i.arg1   = arg1;
	i.arg2   = arg2;
	return EmitInstr(&i);
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
	Bool result = true;
	InstrBlock * blk;

	for(blk = proc->instr; blk != NULL; blk = blk->next) {

		// If block is labeled, Emit label instruction
		if (blk->label != NULL) {
			EmitInstrOp(INSTR_LABEL, blk->label, NULL, NULL);
		}

		result = EmitInstrBlock(blk);
		if (!result) break;
	}
	return result;
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
/*
Purpose:
	Emit definition of constants and variables whose address has been defined.
*/
{
	Var * var, * ov, * adr;
	Instr instr;
	Type * type;
	UInt32 n;

	for(var = VarFirst(), n = 1; var != NULL; var = VarNext(var), n++) {
		type = var->type;

		if (type != NULL && type->variant == TYPE_ARRAY && var->mode == MODE_CONST) continue;
		if (var->scope == REGSET) continue;

		adr = var->adr;
		if (
			   (adr != NULL && !VarIsReg(adr) && adr->scope != REGSET && (var->mode == MODE_VAR || var->mode == MODE_ARG) && (var->read > 0 || var->write > 0))
			|| (var->mode == MODE_CONST && var->read > 0 && var->name != NULL > 0)
		) {

			if (adr != NULL && adr->mode == MODE_CONST && adr->n >= DATA_SEGMENT) continue;

			instr.op = INSTR_VARDEF;
			instr.result = var;
			instr.arg2 = NULL;

			if (var->mode != MODE_CONST) {
				//n = var->adr; 
				ov = var->adr;
			} else {
				n = var->n;
				ov = VarNewInt(n);
			}
			instr.arg1 = ov;
			EmitInstr(&instr);
		}
	}
}

void EmitProcedures()
{
	Var * var;
	Type * type;
	Instr vardef;

	for(var = VarFirst(); var != NULL; var = VarNext(var)) {
		type = var->type;
		if (var->mode != MODE_TYPE && var->mode != MODE_ELEMENT && type != NULL && var->instr != NULL && type->variant == TYPE_PROC) {
			if (var->read > 0) {
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
}

void EmitAsmIncludes()
/*
Purpose:
	Try to find corresponding .asm file for every used .atl file.
	If it is found, generate include instruction to output file.
*/
{
	Var * var;
	Instr i;
	FILE * f;
	char name[MAX_PATH_LEN];
	UInt16 len;

	MemEmptyVar(i);
	i.op = INSTR_INCLUDE;
	for(var = VarFirst(); var != NULL; var = VarNext(var)) {
		if (var->mode == MODE_SRC_FILE) {
			if (FlagOff(var->submode, SUBMODE_MAIN_FILE)) {

				strcpy(name, var->name);
				len = StrLen(name);
				name[len-4] = 0;
				f = FindFile(name, ".asm");

				if (f != NULL) {
					fclose(f);
					i.result = VarNewStr(FILENAME);
					EmitInstr(&i);
				}
			}
		}
	}
}
