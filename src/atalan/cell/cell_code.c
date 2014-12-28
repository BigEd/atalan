/*
Code cells

(c) 2014 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

Code cell represents basic code block.
It is list of instructions.

*/

#include "../language.h"

InstrBlock * NewCode()
{
	InstrBlock * code = NewCell(INSTR_CODE);
	code->next = NULL;
	code->to = code->cond_to = code->from = code->callers = code->next_caller = NULL;
	code->jump_type = JUMP_IF;
	code->label = NULL;

	code->first = code->last = NULL;

	code->analysis_data = NULL;
	code->processed = false;
	code->itype = NULL;

	code->n = 0;
	return code;
}

void InstrBlockFree(InstrBlock * blk)
{
	Instr * i, * next;

	if (blk != NULL) {
		for(i = blk->first; i != NULL; i = next) {
			next = i->next;
			InstrFree(i);
		}
		blk->first = blk->last = NULL;
	}
}

UInt32 CodeInstrCount(InstrBlock * blk)
/*
Purpose:
	Return number of instructions in the block.
*/
{
	Instr * i;
	UInt32 n = 0;
	if (blk != NULL) {
		for(i = blk->first; i != NULL; i = i->next) n++;
	}
	return n;
}

InstrBlock * CodeLastBlock(InstrBlock * code)
{
	if (code != NULL) {
		while(code->next != NULL) code = code->next;
	}
	return code;
}

void CodeCellPrint(Cell * cell)
{
	UInt8 color = PrintColor(RED+GREEN+LIGHT);
	PrintInt(cell->seq_no); Print("@");
	PrintColor(color);
}
