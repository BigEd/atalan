Bool IsEqual(Cell * l, Cell * r);
Bool IsNotEqual(Cell * l, Cell * r);
Bool IsLowerEq(Cell * l, Cell * r);
Bool IsLower(Cell * l, Cell * r);
Bool IsHigher(Cell * l, Cell * r);
Bool IsHigherEq(Cell * l, Cell * r);

Int16 CellCompare(Cell * l, Cell * r);

Cell * Max(Cell * l, Cell * r);
Cell * Min(Cell * l, Cell * r);
Cell * Min4(Cell * m1, Cell * m2, Cell * m3, Cell * m4);
Cell * Max4(Cell * m1, Cell * m2, Cell * m3, Cell * m4);

Bool IsSubset(Type * type, Type * master);
Bool TypeIsEqual(Type * left, Type * right);

Cell * EqEval(Cell * cell);
Cell * NeEval(Cell * cell);
Cell * LtEval(Cell * cell);
Cell * LeEval(Cell * cell);
Cell * GtEval(Cell * cell);
Cell * GeEval(Cell * cell);
