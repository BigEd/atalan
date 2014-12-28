
typedef struct {
	Cell * type;		// type of values stored in this cell (this is more for future)
	Cell * adr;
	Cell * bank;		// bank could be any cell (even VOID as is default here)
	UInt32 size;		// the size will be computed from variable type, but can be specified by user (as skip in array)
	UInt32 cell_count;
	UInt16 flags;		// MEMORY_IN, MEMORY_OUT, MEMORY_SEQUENCE
} MemoryCell;

Cell * NewMemory(Cell * adr, Cell * bank, UInt32 size, UInt16 flags);


/*
SUBMODE_IN = 1,
SUBMODE_OUT = 2,		// even procedure may be marked as out. In such case, it has some side-effect.
SUBMODE_REG = 4,		// variable is stored in register
SUBMODE_IN_SEQUENCE = 8,
SUBMODE_ARG_IN  = 16,
SUBMODE_ARG_OUT = 32,
SUBMODE_OUT_SEQUENCE = 64,
*/



/*
   Name   --> Memory --> Type

*/
