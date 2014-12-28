/*
Ranges

Ranges represent continuous set of values beginning at minimal value and ending at maximal value.
Step may be defined.

0..7             = 0,1,2,3,4,5,6,7
10..100 step 10  = 10,20,30,40,50,60,70,80,90,100


l = min
r = max
m = step

(c) 2013 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "../language.h"

Cell * NewRange(Cell * min, Cell * max)
{
	Cell * var = NewOp(INSTR_RANGE, min, max);

	return var;
}

Cell * NewRangeOpenRight(Cell * min, Cell * max)
{
	return NewRange(min, Sub(max, ONE));
}

Cell * NewRangeOpenLeft(Cell * min, Cell * max)
{
	return NewRange(Add(min, ONE), max);
}

Cell * NewRangeInt(BigInt * min, BigInt * max)
{
	return NewRange(IntCell(min), IntCell(max));
}

Cell * NewRangeIntN(Int32 min, Int32 max)
{
	Type * type;
	BigInt imin, imax;

	IntInit(&imin, min);
	IntInit(&imax, max);

	type = NewRangeInt(&imin, &imax);
	IntFree(&imin); IntFree(&imax);
	return type;
}

Cell * NewBitRange(UInt32 bit_count)
{
	Type * rt;
	BigInt n;

	IntInit(&n, 0);

	while(bit_count > 0) {
		IntMulN(&n, 2);
		IntAddN(&n, 1);
		bit_count--;
	}

	rt = NewRangeInt(Int0(), &n);

	return rt;
}

void PrintRange(Cell * cell)
{
	PrintCell(cell->l);
	Print("..");
	PrintCell(cell->r);
}

Cell * RangeEval(Cell * cell)
{
	return NewRange(Eval(cell->l), Eval(cell->r));
}
