InstrBlock * NewCode();

Instr * FirstInstr(InstrBlock * blk);
Instr * LastInstr(InstrBlock * blk);
void InstrBlockFree(InstrBlock * blk);
UInt32 CodeInstrCount(InstrBlock * blk);
InstrBlock * CodeLastBlock(InstrBlock * code);
void CodeCellPrint(Cell * cell);
