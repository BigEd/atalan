extern Var * ZERO;
extern Var * ONE;
extern Var * MINUS_ONE;

Var * IntCellN(long n);
Var * IntCell(BigInt * n);

Bool CellIsConst(Var * var);
Bool CellRange(Var * var, Var ** p_min, Var ** p_max);
Bool CellIsIntConst(Var * var);
Bool CellIsN(Var * var, Int32 n);

BigInt * IntFromCell(Var * var);
