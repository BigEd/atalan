Cell * NewRange(Cell * min, Cell * max);
Cell * NewRangeOpenRight(Cell * min, Cell * max);
Cell * NewRangeOpenLeft(Cell * min, Cell * max);
Cell * NewRangeInt(BigInt * min, BigInt * max);
Cell * NewRangeIntN(Int32 min, Int32 max);
Cell * NewBitRange(UInt32 bit_count);

void PrintRange(Cell * cell);
Cell * RangeEval(Cell * cell);
