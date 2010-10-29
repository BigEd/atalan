/*

Basic block optimalization routines

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

In computing, a basic block is a portion of the code within a program with certain desirable 
properties that make it highly amenable to analysis. Compilers usually decompose programs into 
their basic blocks as a first step in the analysis process. 
Basic blocks form the vertices or nodes in a control flow graph.

The code in a basic block has one entry point, meaning no code within it is the destination 
of a jump instruction anywhere in the program, and it has one exit point, meaning only 
the last instruction can cause the program to begin executing code in 
a different basic block. Under these circumstances, whenever the first instruction 
in a basic block is executed, the rest of the instructions 
are necessarily executed exactly once, in order.

(http://en.wikipedia.org/wiki/Basic_block)

*/

#include "language.h"


InstrBlock * SplitBlock(InstrBlock * block, Instr * i)
/*
Purpose:
	Split block at specified intruction.
	Return new created block.
*/
{
	Instr * prev;
	InstrBlock * nb;

	nb = MemAllocStruct(InstrBlock);
	nb->first = i;
	nb->last  = block->last;
	nb->next  = block->next;

	prev = i->prev;
	block->last = prev;
	block->next = nb;

	if (prev != NULL) {
		prev->next = NULL;
	}

	i->prev = NULL;

	return nb;
}

void ReplaceVar(InstrBlock * block, Var * from, Var * to)
/*
Purpose:
	Replace use of variable 'from' with variable 'to' in specified block.
	When this procedure ends, 'from' variable is no more referenced in the block.
*/
{
	Instr * i;
	InstrBlock * nb;
	for(nb = block; nb != NULL; nb = nb->next) {
		for (i = nb->first; i != NULL; i = i->next) {
			if (i->result == from) i->result = to;
			if (i->arg1   == from) i->arg1 = to;
			if (i->arg2   == from) i->arg2 = to;
		}
	}
}

void GenerateBasicBlocks(Var * proc)
/*
Purpose:
	Split the procedure blocks to basic blocks.
	Blocks are linked in chain using next.
*/
{
	InstrBlock * nb, * dst;
	Instr * i, * i2;
	InstrOp op;
	Var * label;
	InstrBlock * blk, * next_blk;

	blk = proc->instr;
	while(blk != NULL) {
		next_blk = blk->next;
		nb = blk;

		for (i = blk->first; i != NULL; i = i->next) {
	retry:
			op = i->op;
			// Label starts new basic block
			if (op == INSTR_LABEL) {
				if (i->prev != NULL) {
					nb = SplitBlock(nb, i);
				}
				label = i->result;
				nb->label = label;
				label->instr = nb;
				i = nb->first->next;
				InstrDelete(nb, nb->first);		// we do not need the label instruction anymore

				// If there is another label(s) after this label, replace the use of second label by the first label
				// to prevent creation of empty blocks.
				while (i != NULL && i->op == INSTR_LABEL) {
					i2 = i;
					ReplaceVar(blk, i->result, nb->label);
					i = i->next;
					InstrDelete(nb, i2);
				}
				if (i == NULL) {
					break;
				}
				goto retry;
			// Jump ends basic block
			} else if (IS_INSTR_JUMP(op)) {
				if (i->next != NULL) {
					i = i->next;
					nb = SplitBlock(nb, i);
					goto retry;
				}
			}
		}
		blk = next_blk;
	}

	for(blk = proc->instr; blk != NULL; blk = blk->next) {
		blk->to      = NULL;
		blk->cond_to = NULL;
		blk->callers = NULL;
		blk->from    = NULL;
	}

	// Create caller lists for blocks for goto and if instructions

	for(nb = proc->instr; nb != NULL; nb = nb->next) {
		// Continue with instruction in next block 
		// (this is not true for block ending with GOTO, but that is handled in following condition)

		dst = nb->next;
		if (dst != NULL) {
			nb->to = nb->next;
			dst->from = nb;
		}

		i = nb->last;
		if (i == NULL) continue;
		op = i->op;

		// If this is conditional jump or jump, register it as caller to destination block

		if (IS_INSTR_JUMP(op)) {
			label = i->result;
			dst = label->instr;

			if (op == INSTR_GOTO) {
				nb->to = dst;
			} else {
				nb->cond_to = dst;
			}
			nb->next_caller = dst->callers;
			dst->callers = nb;
		}
	}

	printf("************* Blocks **************\n");
	PrintProc(proc);

	// TODO: Dead code blocks
/*
	for(nb = block; nb != NULL;) {
		dst = nb->next;
		if (dst != NULL) {
			i = nb->last;
			if (i->op == INSTR_GOTO) {
				label = i->result;
				nb->to = NULL;
				// There are no callers (we are not coming from previous block and there are no callers)
				if (dst->callers == NULL) {
					nb = dst->next;
					InstrBlockFree(dst);
					continue;
				}
			}
		}
		nb = nb->next;
	}
*/
	//TODO: Blocks with labels, that are not jumped to (data labels) may be merged together
}
