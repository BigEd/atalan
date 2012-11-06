/*

Global optimization routines

(c) 2012 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"


void MarkBlockAsUnprocessed(InstrBlock * block)
{
	InstrBlock * blk;
	for(blk = block; blk != NULL; blk = blk->next) {
		blk->processed = false;
		blk->analysis_data = NULL;
	}
}


void OptimizeDataFlow(Var * proc, ProcessBlockFn block_fn, void * info, void * first_block_info)
{
}

void OptimizeDataFlowBack(Var * proc, ProcessBlockFn block_fn, void * info)
/*
Purpose:
	Perform analysis of whole procedure block by block.
	Start from the end of the procedure and continue to the start of the procedure.
	Only block, whose both possible followers were already processed are processed.
*/
{
	InstrBlock * blk;
	Bool change;

	blk = proc->instr;
	MarkBlockAsUnprocessed(blk);

	change = true;
	while(change) {
		change = false;
		for(blk = proc->instr; blk != NULL; blk = blk->next) {
			if (!blk->processed) {
				if ((blk->to == NULL || blk->to->processed) && (blk->cond_to == NULL || blk->cond_to->processed || blk->cond_to == blk)) {
					block_fn(blk, info);
					blk->processed = true;
					change = true;
				}
			}
		}
	}
}

