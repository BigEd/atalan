/*

Emit phase

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

This module is responsible for emitting compiled code to appropriate output.
Emit supports two basic function EmitByte & EmitStr, which is the point through which all other functions go.

*/

#include "language.h"

FILE * G_OUTPUT;

extern Rule * EMIT_RULES[INSTR_CNT];
extern Var   ROOT_PROC;

/*
Removing null operations
========================
When emitting output source code, we sometimes generate texts like X+0, Y-0 etc.
We do not want this artifacts become part of generated source code, as it unnecessarily clutters it.
We therefore postpone emitting characters like + and - and if they are followed by integer 0, we do not output the sequence.

G_PREV_OP is variable, which remembers the previous operator.
*/

UInt8 G_PREV_OP = 0;
static SrcLine * EMIT_LINE;

/*
Emit to Buffer
==============
Emit can generate it's output either to a file or to specified memory buffer.
The buffer is used for example to format assembler command
*/

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
			PrintChar(c);	
		}
		if (G_OUTPUT != NULL) {
			putc(c, G_OUTPUT);
		}
	}
}

void EmitChar(char c)
{
	if (G_PREV_OP != 0) {
		EmitByte(G_PREV_OP);
		G_PREV_OP = 0;
	}

	if (c == '-' || c == '+') {
		G_PREV_OP = c;
	} else {
		EmitByte(c);
	}
}

void EmitStr(char * str)
{
	char * s, c;
//	UInt16 len;

	if (str != NULL) {

		if (G_BUF != NULL) {
			s = str;	
			while(c = *s++) {		
				EmitChar(c);
			}
		} else {
			// We must flush previous char if it is remembered
			EmitChar('-');
			G_PREV_OP = 0;

	//		len = StrLen(str);
	//		if (str[len-1] == '-') {
	//		}
			if (Verbose(NULL)) {
				Print(str);
			}
			if (G_OUTPUT) {
				fputs(str, G_OUTPUT);
			}
		}
	}
}

void EmitInt(long n)
{
	char buf[32];

	// Swallow operators like -0,  +0
	if ((G_PREV_OP == '-' || G_PREV_OP == '+') && n == 0) {
		G_PREV_OP = 0;
		return;
	}
	sprintf( buf, "%d", n );
	EmitStr(buf);
}

void EmitBigInt(BigInt * n)
{
	Int32 i;
	i = IntN(n);
	EmitInt(i);
}

void EmitStrConst(char * str)
/*
Purpose:
	Emit string constant.
	This code is currently MADS specific to handle emit of single quotes.
	Quotes are emitted like:  'ahaha', 39, 'sksksks'
TODO:
	Define escapes for string generation.
	'  -> "', 39, '"
	
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

void EmitIntCellName(Var * var)
/*
Purpose:
	Emit name of variable.
	Variable name may contain non-alphanumeric characters, so we must translate them to support ordinary assemblers.
	Identifiers starting with digit are prefixed by _N.
	Non-alphanumeric characters are replaced by xNN, when NN are two hexadecimal digits representing ascii code of the character.
*/
{
	char * s, c;

	s = VarName(var);
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
//	if (var->idx != 0) EmitInt(var->idx-1);
}

void EmitVar(Var * var, UInt8 format)
{
	Bool non_keyword = true;

	if (var != NULL) {
//		if (var->mode == INSTR_SRC_FILE) {
//			EmitStr(var->name);
//		} else 
		if (var->mode == INSTR_ELEMENT) {
//			if (VarIsStructElement(var)) {
//				EmitVar(var->adr, format);
//				EmitStr("+");
//				EmitVar(var->var, format);
//			} else {
				InternalError("don't know how to emit element");
//			}
		} else if (var->mode == INSTR_DEREF) {
			EmitVar(var->l, format);
		} else if (var->mode == INSTR_BYTE) {
			InternalError("don't know how to emit byte array element");
		} else if (var->mode == INSTR_TEXT) {
			if (format == 1) {
				EmitStrConst(var->str); 
			} else {
				EmitStr(var->str);
			}
		} else if (var->mode == INSTR_INT) {
			EmitBigInt(&var->n);
		} else if (var->mode == INSTR_VAR) {
			// *** Module parameters (4)
			// When parameter name is emmited, it is prefixed with PARAM_ prefix
			if (VarIsParam(var)) {
				EmitStr("PARAM_");
				EmitStr("__");
			} else if (var->scope != NULL && var->scope != &ROOT_PROC && var->scope != CPU->SCOPE && VarName(var->scope) != NULL && !VarIsLabel(var)) {
				EmitIntCellName(var->scope);
				EmitStr("__");
			} else {
				non_keyword = true;
				// For variables (excluding registers), emit extra underscore at the beginning to prevent name clash with assembler built-in keywords and register names
				if (!VarIsReg(var)) {
					EmitStr("_");
				}
			}
			EmitIntCellName(var);			
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
	Var * step;
	BigInt bn;
	Var * pn;
	s = str;

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
					if (StrEqualPrefix(s, "count", 5)) {
						VarCount(var, &bn);
						EmitBigInt(&bn);
						IntFree(&bn);
						s+= 5;
						continue;
					} else if (StrEqualPrefix(s, "size", 4)) {
						n = VarByteSize(var);
						EmitInt(n);
						s+= 4;
						continue;
					} else if (StrEqualPrefix(s, "elemsize", 8) || StrEqualPrefix(s, "item.size", 9)) {
						s += 8;
						if (VarIsArray(var)) {
							n = TypeSize(ItemType(var->type));
						} else {
							n = 0;
						}
						EmitInt(n);
						continue;
					} if (StrEqualPrefix(s, "step", 4)) {
						s += 4;
						step = ONE;
						if (VarIsArray(var)) {
							step = ArrayStep(var->type);
						}
						EmitVar(step, 0);
						continue;

					} else if (StrEqualPrefix(s, "index.min", 9)) {
						s += 9;
						if (VarIsArray(var)) {
							pn = CellMin(IndexType(var->type));
						} else {
							pn = ZERO;
						}
						EmitVar(pn, 0);
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
					EmitVar(instr->arg1, format); 
					continue;
				case '2': 
					EmitVar(instr->arg2, format); 
					continue;
				case '@': break;
				case 't': c = '\t'; break;
			}
		}
		EmitChar(c);
	}
}

extern Bool INSTR_MATCH_BREAK;

Bool EmitInstrInline(Instr * i)
{
	Rule * rule;
	Instr * to;

	rule = InstrRule(i);

	if (rule != NULL) {
		for(to = rule->to->first; to != NULL; to = to->next) {
			EmitInstr2(i, to->arg1->str);
		}
		return true;
	} else {
		InternalError("CPU does not support instruction");
		InstrPrint(i);
 		return false;
	}
}

Bool EmitInstr(Instr * i)
{
	Rule * rule;
	Instr * to;

	rule = InstrRule(i);

	if (rule != NULL) {
		for(to = rule->to->first; to != NULL; to = to->next) {
			EmitInstr2(i, to->arg1->str);
			EmitChar(EOL);
		}
	} else {
		if (FlagOff(INSTR_INFO[i->op].flags, INSTR_OPTIONAL)) {
			InternalError("CPU does not support instruction");
			InstrPrint(i);
			return false;
		}
	}
	return true;
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
	UInt32 n;

	if (blk != NULL) {
		for(i = blk->first, n=1; r && i != NULL; i = i->next, n++) {
			r = EmitInstr(i);
		}
	}
	return r;
}

Bool EmitProc(Var * proc)
{
	Bool result = true;
	InstrBlock * blk;

	EMIT_LINE = NULL;

	for(blk = FnVarCode(proc); blk != NULL; blk = blk->next) {

		// If block is labeled, Emit label instruction
		if (blk->label != NULL) {
			EmitInstrOp(INSTR_LABEL, blk->label, NULL, NULL);
		}

		result = EmitInstrBlock(blk);
		if (!result) break;

		//TODO: RETURN should be part of procedure
		if (blk->to == NULL && (blk->last == NULL || !IsGoto(blk->last))) {
			if (proc != &ROOT_PROC) {
				EmitInstrOp(INSTR_RETURN, NULL, proc, NULL);
			}
		}
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
	Var * var, * adr;
	Type * type;

	FOR_EACH_VAR(var)
		if (var->mode == INSTR_VAR) {
			type = var->type;

//			if (type != NULL && type->variant == TYPE_ARRAY && var->mode == INSTR_INT) continue;
			if (VarIsReg(var)) continue;

			adr = VarAdr(var);
			if ( (adr != NULL && !VarIsReg(adr) && var->mode == INSTR_VAR && (var->read > 0 || var->write > 0))
			  || (CellIsIntConst(var) && (var->read > 0  || FlagOn(var->submode, SUBMODE_PARAM)) && VarName(var) != NULL)
			) {

				if (adr != NULL && adr->mode == INSTR_INT && IntN(&adr->n) >= DATA_SEGMENT) {
					continue;
				}

				EmitInstrOp(INSTR_VARDEF, var, adr, NULL);
			}
		}
	NEXT_VAR
}

void EmitProcedures()
{
	Var * var;
	Instr vardef;

	FOR_EACH_VAR(var)
		if (IsFnVar(var) && IsFnImplemented(var->type)) {
			if (var->read > 0) {
				MemEmptyVar(vardef);
				vardef.op = INSTR_FN;
				vardef.arg1 = var;
				EmitInstr(&vardef);
	//			PrintProc(var);
				EmitProc(var);
				vardef.op = INSTR_ENDPROC;
				EmitInstr(&vardef);
			}
		}
	NEXT_VAR
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
	char name[MAX_PATH_LEN], path[MAX_PATH_LEN];
	UInt16 len;
	
	MemEmptyVar(i);
	i.op = INSTR_INCLUDE;
	FOR_EACH_LOCAL(MODULES, var)

		// We do not try to generate asm include file for main file, as the compiler generates it
		//  I.e.
		//    When compiling 'x.atl', compiler emits 'x.asm'.
		//    Including this file would create a loop.

		if (FlagOff(var->submode, SUBMODE_MAIN_FILE)) {

			StrCopy(name, VarName(var));
			len = StrLen(name);
			name[len-4] = 0;
			f = FindFile(name, ".asm", path);

			if (f != NULL) {
				fclose(f);
				i.arg1 = TextCell(FILENAME);
				EmitInstr(&i);
			}
		}
	NEXT_VAR
}

void Emit(char * filename)
{
	Var * var;

	EmitOpen(filename);

	VarUse();

	if (Verbose(NULL)) {

		PrintHeader(1, "Variables");

		FOR_EACH_VAR(var)
			if (var->write >= 1 || var->read >= 1) {
				if (var->mode == INSTR_VAR && !VarIsReg(var) && !VarIsLabel(var)) {
					PrintVar(var); PrintEOL();
				}
			}
		NEXT_VAR

		PrintHeader(1, "Output");
	} // verbose

	EmitLabels();

	//TODO: ProcessUsedProc
//	if (!EmitProc(&ROOT_PROC)) goto failure;
	EmitProc(&ROOT_PROC);
	EmitProcedures();
	EmitAsmIncludes();	
	EmitInstrOp(INSTR_CODE_END, NULL, NULL, NULL);
	VarEmitAlloc();
	EmitInstrOp(INSTR_SRC_END, NULL, NULL, NULL);
	EmitClose();
}

