#include "language.h"

void PrintArgs(Cell * list)
{
	Cell * en, * c, * v;
	FOR_EACH_ITEM(en, c, list)
		if (c->mode == INSTR_TEXT) {
			Print(c->str);
		} else if (c->mode == INSTR_INT) {
			if (IsEqual(c, IntCellN(128))) {
				PrintEOL();
			}
		} else if (c->mode == INSTR_VAR) {
			v = Eval(c);
			PrintCell(v);
		}
	NEXT_ITEM(en, c)
}

//InstrEvalConst
//InstrEvalAlgebraic
//VarEvalConst
//TypeEval

Cell * Eval(Cell * cell)
{
	InstrInfo * ii;
	if (cell == NULL) return NULL;
	ii = &INSTR_INFO[cell->mode];	
	return ii->eval_fn(cell);
}

Cell * Interpret(Cell * proc, Cell * args)
{
	InstrBlock * blk;
	Instr * i;
	Cell * r = NULL;
	Cell * v;

	blk = FnVarCode(proc);
next:
	while(blk != NULL) {
		for(i = blk->first; i != NULL; i = i->next) {
			switch(i->op) {
			case INSTR_ASSERT:
				PrintCell(i->arg1);
				exit(0);
				break;
			case INSTR_LET:
				v = Eval(i->arg1);
				ifok {
					VarLet(i->result, v);
				}
				break;

			case INSTR_IF:
				v = Eval(i->arg1);
				if (!IsEqual(v, FALSE)) {
					blk = IfInstr(i);
					goto next;
				}
				break;
			case INSTR_PRINT:
				PrintArgs(i->arg1);
				break;
			}
		}
		blk = blk->next;
	}
	return r;
}