/*

Variable defines name, type and address.


Maybe we should split it as defining:

INSTR_NAME   -  definition of named sell
INSTR_VAR    -  defines type, and address (value)

*/

#ifdef _DEBUG

Cell * VarAdr(Var * var);
Type * VarType(Var * var);
void VarSetAdr(Var * var, Cell * adr);
Cell * VarSymbol(Cell * var);

#else

#define VarAdr(var) (var)->adr
#define VarType(var) (var)->type
#define VarSymbol(var) (var)->symbol
#define VarSetAdr(var, a) (var)->_adr = a;
#endif

char * VarName(Var * var);
Bool VarIsNamed(Var * var, char * name);

void VarLet(Var * var, Var * val);
UInt8 VarArgIdx(Cell * var);
void VarCellPrint(Cell * cell);
void VarCellFree(Cell * cell);
Cell * VarEval(Cell * cell);

Cell * NewVarWithSymbol(Cell * scope, Cell * symbol, Type * type);
