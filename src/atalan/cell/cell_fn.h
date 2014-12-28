#define IsMacro(X) FlagOn((X)->submode, SUBMODE_MACRO)

Cell * NewFn(Type * type, InstrBlock * instr);

Bool IsFnVar(Var * var);
Bool IsFnImplemented(Var * fn_cell);

#define FOR_EACH_IN_ARG(SCOPE, VAR) 	FOR_EACH_ITEM(en, VAR, ArgType(SCOPE->type->type))
#define NEXT_IN_ARG(VAR) NEXT_ITEM(en, VAR)

#define FOR_EACH_OUT_ARG(SCOPE, VAR) 	FOR_EACH_ITEM(en, VAR, ResultType(SCOPE->type->type))
#define NEXT_OUT_ARG(VAR) NEXT_ITEM(en, VAR)

typedef Bool (*VarFilter)(Var * var);
void ProcLocalVars(Var * proc, VarSet * set, VarFilter filter_fn);

InstrBlock * FnVarCode(Var * proc);
void FnSetCode(Cell * fn, InstrBlock * code);

Bool VarIsInArg(Var * fn_var, Var * var);
Bool VarIsOutArg(Var * fn_var, Var * var);

