/****************************************************************

 Print tokens

****************************************************************/
#include "language.h"

void PrintOptim(char * text)
{
	UInt8 color;
	color = PrintColor(GREEN);
	Print(":");
	Print(text);
	PrintColor(color);
}

void PrintIntCellNameUser(Var * var)
{
	Print(VarName(var));
}

void PrintQuotedCellName(Var * var)
{
	PrintChar('\'');
	PrintIntCellNameUser(var);
	PrintChar('\'');
}

void PrintCell(Var * var)
{
	UInt8 oc;
	char * s;
	InstrInfo * ii;
	if (var == NULL) return;

	ii = &INSTR_INFO[var->mode];
	if (ii->cell_print_fn != NULL) {
		ii->cell_print_fn(var);
		return;
	}

	if (var->mode == INSTR_DEREF) {
		Print("@");
		var = var->l;
	}

	if (var->mode == INSTR_ELEMENT) {
		PrintCell(var->l);
		if (var->l->mode == INSTR_TUPLE) {
			Print(".");
			PrintCell(var->r);
		} else {
			Print("#");
			PrintCell(var->r);
		}
	} else if (var->mode == INSTR_BYTE) {
		PrintCell(var->l);
		oc = PrintColor(GREEN+BLUE);
		Print("$");
		PrintColor(oc);
		PrintCell(var->r);

	} else if (var->mode == INSTR_TYPE) {
		PrintType(var->type);
	} else {
		if (var->mode == INSTR_RANGE) {
			PrintCell(var->l); Print(".."); PrintCell(var->r);
		} else if (var->mode == INSTR_TUPLE) {
			Print("(");
			PrintCell(var->l);
			Print(",");
			PrintCell(var->r);
			Print(")");
		} else {
			s = ii->symbol;

			// Unary operator
			if (ii->arg_type[2] == TYPE_VOID) {
				oc = PrintColor(GREEN+BLUE);
				Print(s);
				Print(" ");
				PrintColor(oc);
				PrintCell(var->l);
			} else {
				Print("(");
				PrintCell(var->l);
				oc = PrintColor(GREEN+BLUE);
				Print(" ");
				Print(s);
				Print(" ");
				//if (*s>='a' && *s<='z') { Print(" "); }
				PrintColor(oc);
				PrintCell(var->r);
				Print(")");
			}
		}
	}

	if (var->mode == INSTR_VAR) {
		PrintIntCellName(var);

		if (var->type->variant == TYPE_LABEL) {
			if (var->instr != NULL) {
				Print(" (#");
				PrintInt(var->instr->seq_no);
				Print(")");
			}
		}

		if (VarAdr(var) != NULL) {
			if (VarAdr(var)->mode == INSTR_TUPLE) {
				if (!VarIsReg(var)) {
					Print("@");
					PrintCell(VarAdr(var));
				}
			}
		}
	}

}

void PrintVarUser(Var * var)
{
	if (var->mode == INSTR_ELEMENT) {
		PrintIntCellNameUser(var->l); Print("("); PrintVarUser(var->r); Print(")");
	} else if (var->mode == INSTR_BYTE) {
		PrintIntCellNameUser(var->l);
		Print("$");
		PrintVarUser(var->r);
	} else if (var->mode == INSTR_TYPE) {
		PrintIntCellNameUser(var);
		PrintType(var->type);
	} else {
		PrintIntCellNameUser(var);
	}
}

void PrintVar(Var * var)
{
	Type * type;

	if (var == NULL) return;

	if (var->mode == INSTR_DEREF) {
		Print("@");
		var = var->l;
	}

	if (var->mode == INSTR_EMPTY) {
		Print("()");
	} else if (var->mode == INSTR_ELEMENT) {
		PrintIntCellName(var->l);
		Print("(");
		PrintVar(var->r);
		Print(")");
	} else if (var->mode == INSTR_BYTE) {
		PrintIntCellName(var->l);
		Print("$");
		PrintVar(var->r);
	} else if (var->mode == INSTR_INT) {
		PrintBigInt(&var->n);
		return;
	} else if (var->mode == INSTR_SEQUENCE) {
		Print("sequence "); PrintVar(var->seq.init); 
		if (var->seq.op == INSTR_ADD) {
			if (!IsEqual(var->seq.step, ONE)) {
			}
			Print(",..,");
			if (var->seq.compare_op != INSTR_LE) {
				Print(INSTR_INFO[var->seq.compare_op].symbol);
			}
			PrintVar(var->seq.limit);
		}

	} else if (var->mode == INSTR_ARRAY_TYPE) {
		Print("array (");
		PrintVar(IndexType(var));
		Print(") of ");
		PrintVar(ItemType(var));

	} else if (var->mode == INSTR_TYPE) {
		PrintType(var);

	} else if (var->mode == INSTR_FN) {
		if (IsMacro(var->type)) {
			Print("macro");
		}
		PrintVar(var->type);
	} else if (var->mode == INSTR_FN_TYPE) {
		PrintVar(ArgType(var));
		Print(" -> ");
		PrintVar(ResultType(var));
	} else if (var->mode == INSTR_VAR) {

		PrintIntCellName(var);

		if (VarAdr(var) != NULL) {
			Print("@");
			PrintCell(VarAdr(var));
		}

		type = VarType(var);
		if (type != NULL) {
			Print(":");
			PrintVar(type);
		}
	} else if (var->mode == INSTR_VARIANT) {
		PrintVar(var->l);
		Print("|");
		PrintVar(var->r);
	} else {
		PrintVar(var->l);
		Print(INSTR_INFO[var->mode].symbol);
		PrintVar(var->r);
	}
//	Print("  R%ld W%ld\n", var->read, var->write);
}

#define COLOR_KEYWORD GREEN

void PrintKeyword(char * txt)
{
	UInt8 color;
	color = PrintColor(COLOR_KEYWORD);
	Print(txt);
	PrintColor(color);
}

void InstrPrintInline(Instr * i)
{
//	Var * inop;
	Bool r = false, a1 = false;

/*
	if (i->op == INSTR_LINE) {
		PrintColor(BLUE+LIGHT);
		PrintFmt("%s(%d) %s", i->result->name, i->line_no, i->line);
		PrintColor(RED+GREEN+BLUE);
	} else 
*/
	Print("   ");
	if (i->op == INSTR_IF) {
		PrintKeyword("if");
	} else {
		PrintKeyword(INSTR_INFO[i->op].name);
	}

	if (i->result != NULL) {
		Print(" ");
		PrintCell(i->result);
		r = true;
	}

	if (i->arg1 != NULL) {
		if (r) {
			if (i->op == INSTR_LET) {
				PrintKeyword(" = ");
			} else {
				PrintKeyword(", ");
			}
		} else {
			Print(" ");
		}

		PrintCell(i->arg1);
	}

	if (i->arg2 != NULL) {
		if (i->op == INSTR_IF) {
			PrintKeyword(" goto ");
		} else {
			PrintKeyword(", ");
		}
		PrintCell(i->arg2);
	}
}

void InstrPrint(Instr * i)
{
	InstrPrintInline(i);
	Print("\n");
}

void PrintBlockHeader(InstrBlock * blk)
{
	PrintFmt("#%ld/  ", blk->seq_no);
	if (blk->label != NULL) {
		Print("    ");
		PrintCell(blk->label);
		Print("@");
	}
	Print("----------\n");
}

void PrintInstrLine(UInt32 n)
{
	PrintFmt("%3ld| ", n);
}

void PrintInferType(Type * type)
{
	UInt8 old_color;
	old_color = PrintColor(COLOR_OPTIMIZE);
	if (type == NULL) {
		Print("???");
	} else {
		PrintVar(type);
	}
	PrintColor(old_color);
}

void PrintCode(InstrBlock * blk, UInt32 flags)
{
	Instr * i;
	UInt32 n;
	InstrInfo * ii;
	SrcLine * line;
	UInt8 color;

	line = NULL;

	while (blk != NULL) {
		n = 1;
		PrintBlockHeader(blk);
		for(i = blk->first; i != NULL; i = i->next, n++) {

			if (i->line != NULL && line != i->line) {
				line = i->line;
				Print("   | ");
				color = PrintColor(BLUE);
				PrintFmt("%s(%d) %s\n", LineFileName(line), LineNumber(line), line->txt);
				PrintColor(color);
			}

			ii = &INSTR_INFO[i->op];

			PrintInstrLine(n);
			InstrPrintInline(i);
			
			if (FlagOn(flags, PrintInferredTypes)) {
				Print("  ");
				if (i->op == INSTR_LET) {
					PrintInferType(i->type[0]);
				} else if (i->op == INSTR_IF) {
					Print("if "); PrintInferType(i->type[1]);
				}
			}
			PrintEOL();
		}

		blk = blk->next;
	}
}

void PrintProc(Var * proc)
{
	PrintProcFlags(proc, 0);
}

void PrintProcFlags(Var * proc, UInt32 flags)
{
	PrintCode(FnVarCode(proc), flags);
}
