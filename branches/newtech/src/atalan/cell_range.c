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

#include "language.h"

Var * NewRange(Var * min, Var * max)
{
	Var * var = NewOp(INSTR_RANGE, min, max);

	return var;
}

Var * NewRangeOpenRight(Var * min, Var * max)
{
	return NewRange(min, Sub(max, ONE));
}

Var * NewRangeOpenLeft(Var * min, Var * max)
{
	return NewRange(Sub(min, ONE), max);
}

Var * NewRangeInt(BigInt * min, BigInt * max)
{
	return NewRange(IntCell(min), IntCell(max));
}

Var * NewRangeIntN(Int32 min, Int32 max)
{
	Type * type;
	BigInt imin, imax;

	IntInit(&imin, min);
	IntInit(&imax, max);

	type = NewRangeInt(&imin, &imax);
	IntFree(&imin); IntFree(&imax);
	return type;
}

Var * NewBitRange(UInt32 bit_count)
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
