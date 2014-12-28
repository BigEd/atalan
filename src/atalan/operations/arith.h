Cell * CellMin(Cell * v);
Cell * CellMax(Cell * v);

Cell * Add(Cell * l, Cell * r);
Cell * Sub(Cell * l, Cell * r);
Cell * Mul(Cell * l, Cell * r);
Cell * DivInt(Cell * l, Cell * r);
Cell * Mod(Cell * l, Cell * r);
Cell * Div(Cell * l, Cell * r);
Cell * Power(Cell * l, Cell * r);
Cell * Sqrt(Cell * l);

Cell * CellOp(InstrOp op, Cell * l, Cell * r);

//==== Evaluation of cells

Cell * AddEval(Cell * cell);
Cell * SubEval(Cell * cell);
Cell * MulEval(Cell * cell);
Cell * DivIntEval(Cell * cell);
Cell * ModEval(Cell * cell);
Cell * PowerEval(Cell * cell);
Cell * SqrtEval(Cell * cell);
