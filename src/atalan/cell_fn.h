#define IsMacro(X) FlagOn((X)->submode, SUBMODE_MACRO)

Cell * NewFn(Type * type, InstrBlock * instr);

Bool IsFnVar(Var * var);
Bool IsFnImplemented(Var * fn_cell);
