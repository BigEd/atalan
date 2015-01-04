/*
=======
If cell
=======

If cell represents conditional expression.
The first argument is expression, that represents condition.
If it evaluates to 0, the condition is false. 1 represents true.

(c) 2015 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

Cell * NewIf(Cell * condition, Cell * label);
void PrintIf(Cell * cell);
