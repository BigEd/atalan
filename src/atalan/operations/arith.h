Var * CellMin(Var * v);
Var * CellMax(Var * v);

Var * Add(Var * l, Var * r);
Var * Sub(Var * l, Var * r);
Var * Mul(Var * l, Var * r);
Var * DivInt(Var * l, Var * r);
Var * Mod(Var * l, Var * r);
Var * Div(Var * l, Var * r);

Var * CellOp(InstrOp op, Var * left, Var * right);

Int16 CellCompare(Var * left, Var * right);

Bool IsHigher(Var * left, Var * right);
Bool IsLower(Var * left, Var * right);
Bool IsHigherEq(Var * left, Var * right);
Bool IsLowerEq(Var * left, Var * right);

