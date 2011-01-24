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

void VarReplaceVar(Var ** p_var, Var * from, Var * to)
{
	Var * var;

	var = *p_var;

	if (var == NULL) return;

	if (var == from) {
		*p_var = to;
		return;
	}

	if (var->mode == MODE_ELEMENT || var->mode == MODE_TUPLE || var->mode == MODE_RANGE) {
		VarReplaceVar(&var->var, from, to);
		VarReplaceVar(&var->adr, from, to);
	} else if (var->mode == MODE_DEREF) {
		VarReplaceVar(&var->var, from, to);
	}
}

void InstrReplaceVar(InstrBlock * block, Var * from, Var * to)
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
			if (i->op != INSTR_LINE) {
				VarReplaceVar(&i->result, from, to);		
				VarReplaceVar(&i->arg1, from, to);		
				VarReplaceVar(&i->arg2, from, to);		
				//if (i->result == from) i->result = to;
				//if (i->arg1   == from) i->arg1 = to;
				//if (i->arg2   == from) i->arg2 = to;
			}
		}
	}
}

void LinkBlocks(Var * proc)
{
	InstrBlock * nb, * blk, * dst;
	Instr * i;
	InstrOp op;
	Var * label;

	for(blk = proc->instr; blk != NULL; blk = blk->next) {
		blk->to      = NULL;
		blk->cond_to = NULL;
		blk->callers = NULL;
		blk->from    = NULL;
	}

	// Create caller lists for blocks for goto and if instructions

	for(nb = proc->instr; nb != NULL; nb = nb->next) {

		op = INSTR_VOID;
		i = nb->last;
		if (i != NULL) op = i->op;

		// Continue with instruction in next block 
		// This is not true for block ending with GOTO

		dst = nb->next;
		if (dst != NULL && op != INSTR_GOTO) {
			nb->to = nb->next;
			dst->from = nb;
		}

//		if (op == INSTR_VOID) continue;

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
}

void GenerateBasicBlocks(Var * proc)
/*
Purpose:
	Split the procedure blocks to basic blocks.
	Blocks are linked in chain using next.
*/
{
	Instr * i, * i2;
	InstrOp op;
	Var * label;
	InstrBlock * blk, * next_blk, * nb;

//	printf("************* Before Blocks **************\n");
//	PrintProc(proc);

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
				// Label at the beginning of the block, which already has a label
				if (nb->label == NULL) {
					nb->label = label;
					label->instr = nb;
					i = nb->first->next;
					InstrDelete(nb, nb->first);		// we do not need the label instruction anymore
				}

				// If there is another label(s) after this label, replace the use of second label by the first label
				// to prevent creation of empty blocks.
				while (i != NULL && i->op == INSTR_LABEL) {
					i2 = i;
					InstrReplaceVar(blk, i->result, nb->label);
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

	LinkBlocks(proc);

//	printf("************* Blocks **************\n");
//	PrintProc(proc);

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

Instr * FirstInstr(InstrBlock * blk)
/*
Purpose:
	Return first non-line instruction in the block.
*/{
	Instr * i;
	for (i = blk->first; i != NULL && i->op == INSTR_LINE; i = i->next);
	return i;
}

void OptimizeJumps(Var * proc)
/*
Purpose:
	Optimize jumps.

	1. Sequence like:

	    ifeq x,y,l2
	    jmp l1
	    label l2

	   will be translated to

	   ifne x,y,l1

*/
{
	InstrBlock * blk, * blk_to;
	Instr * i, * cond_i;
	Var * label;

	blk = proc->instr;
	while (blk != NULL) {
		blk_to = blk->to;
		if (blk_to != NULL) {
			i = FirstInstr(blk_to);		//for (i = blk_to->first; i != NULL && i->op == INSTR_LINE; i = i->next);
			cond_i = blk->last;

			if (i != NULL && i->op == INSTR_GOTO) {
				if (blk_to->next == blk->cond_to) {
					if (IS_INSTR_BRANCH(cond_i->op)) {
						cond_i->op = OpNot(cond_i->op);
						cond_i->result = i->result;
						blk->cond_to = i->result->instr;

						InstrBlockFree(blk_to);
					}
				}
			}

			// Jump to jump
			if (cond_i != NULL && IS_INSTR_JUMP(cond_i->op)) {
retry:
				label = cond_i->result;
				blk_to = label->instr;
				
				i = FirstInstr(blk_to);	//for(i = blk_to->first; i != NULL && i->op == INSTR_LINE; i = i->next);
				if (i != NULL && i->op == INSTR_GOTO && i->result != cond_i->result) {
					cond_i->result = i->result;
					goto retry;
				}
			}
		}
		blk = blk->next;
	}

	// Recreate destinations
	LinkBlocks(proc);
}

void DeadCodeElimination(Var * proc)
{
	InstrBlock * blk, * prev_blk;

	LinkBlocks(proc);

	// First block of procedure is not dead, as it is an entry point to the procedure

	prev_blk = proc->instr;
	if (prev_blk != NULL) blk = prev_blk->next;

	while (blk != NULL) {
		if (blk->from == NULL && blk->callers == NULL) {
			if (blk->label == NULL || (blk->label->read == 0 && blk->label->write == 0)) {
				//TODO: We should solve alignment using some other means (probably block, or label, should have alignment)
				if (blk->first == NULL || (blk->first->op != INSTR_ALIGN && blk->first->op != INSTR_ARRAY_INDEX)) {
					prev_blk->next = blk->next;
					InstrBlockFree(blk);
					MemFree(blk);
					blk = prev_blk;
				}
			}
		}
		prev_blk = blk;
		blk = blk->next;
	}

}
