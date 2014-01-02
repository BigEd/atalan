Bool IsEqual(Var * left, Var * right);
Bool IsHigherEq(Var * left, Var * right);
Bool IsLowerEq(Var * left, Var * right);
Bool CellIsLower(Var * left, Var * right);

Var * Max(Var * l, Var * r);
Var * Min(Var * l, Var * r);
Var * Min4(Var * m1, Var * m2, Var * m3, Var * m4);
Var * Max4(Var * m1, Var * m2, Var * m3, Var * m4);

Bool IsSubset(Type * type, Type * master);
Bool TypeIsEqual(Type * left, Type * right);
