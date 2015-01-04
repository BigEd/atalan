/*
Symbols

Symbols represent name defined in source code.

(c) 2015 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

Cell * NewSymbol(char * name);
Cell * NewSymbolWithIndex(char * str, VarIdx idx);
char * SymbolName(Cell * symbol);
Bool  SymbolIsNamed(Cell * symbol, char * name);
Bool  SymbolIsTmp(Cell * symbol);

VarIdx SymbolIdx(Cell * symbol);
void SymbolCellPrint(Cell * cell);
