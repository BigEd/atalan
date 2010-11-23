#include "language.h"

void HeapInit(MemHeap * heap)
{
	heap->capacity = 0;
	heap->count    = 0;
	heap->block    = NULL;
}

void HeapCleanup(MemHeap * heap)
{
	MemFree(heap->block);
}

void HeapAddBlock(MemHeap * heap, UInt32 adr, UInt32 size)
/*
Purpose:
	Add new block to heap.
	Nothing is added if such block already exists.
	Blocks will be merged.
*/
{
	MemBlock * mbl;
	UInt32 end, hend;
	UInt32 cnt;

	// Try to append the block to some existing block
	// We may even merge three blocks together

	mbl = heap->block;
	end = adr + size;
	for (cnt = heap->count; size > 0 && cnt > 0; cnt--) {
		hend = mbl->adr + mbl->size;

		// mbl->adr....hend
		//          adr........end
		if (adr >= mbl->adr && adr < end) {
			if (end > hend) {
				mbl->size = end - mbl->adr;
			}
			return;
		}
		mbl++;
	}

	if (heap->count == heap->capacity) {
		cnt = (heap->capacity==0)?4:heap->capacity*2;
		heap->block = (MemBlock *)realloc(heap->block, sizeof(MemBlock) * cnt);
		heap->capacity = cnt;
	}

	mbl = &heap->block[heap->count];
	mbl->adr = adr;
	mbl->size = size;
	heap->count++;
}

Bool HeapAllocBlock(MemHeap * heap, UInt32 size, UInt32 * p_adr)
/*
Purpose:
	Alloc block of specified size from current heap.
*/
{
	UInt32 cnt;
	MemBlock * best_mbl, * mbl;
	Bool found = false;

	// We try to find smallest block bigger or equal to requested size

	best_mbl = NULL;
	mbl = heap->block;
	for (cnt = heap->count; size > 0 && cnt > 0; cnt--) {
		if (mbl->size >= size && (best_mbl == NULL || best_mbl->size > mbl->size)) {
			best_mbl = mbl;
			if (mbl->size == size) break;		// if size of the block is same as requested, we do not need to search further
		}
		mbl++;
	}

	if (best_mbl != NULL) {
		*p_adr = best_mbl->adr;
		best_mbl->adr += size;
		best_mbl->size -= size;
		// Remove block, if it is empty
		if (best_mbl->size == 0) {
			mbl = &heap->block[heap->count - 1];
			if (mbl != best_mbl) {
				best_mbl->adr  = mbl->adr;
				best_mbl->size = mbl->size;
			}
			heap->count--;
		}
		found = true;
	}


	return found;
}
