/*
Memory

Memory represent actual location of a variable in computer memory.

(c) 2014 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "../language.h"

Cell * NewMemory(Cell * adr, Cell * bank, UInt32 size, UInt16 flags)
{
	Cell * cell = NewCellInScope(INSTR_MEMORY, NO_SCOPE);
	cell->mem.adr = adr;
	cell->mem.bank = bank;
	cell->mem.flags = flags;
	cell->mem.size = size;
	cell->mem.type = NULL;
	return cell;
}
