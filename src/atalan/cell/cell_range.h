/*
=====
Range
=====

Range cell represents interval of values higher or equal MIN value and lower or equal MAX value.
Step between two numbers may be specified. Default value of step is 1.

Order of values in the range is not specified.

::::::::::::
0..7             = 0,1,2,3,4,5,6,7
10..100 step 10  = 10,20,30,40,50,60,70,80,90,100
::::::::::::

::::::::::::
  range: type min, max, step -> x
		x >= min and x <= max and (x - min) mod step = 0
:::::::::::::

(c) 2015 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

Cell * NewRange(Cell * min, Cell * max);
Cell * NewRangeOpenRight(Cell * min, Cell * max);
Cell * NewRangeOpenLeft(Cell * min, Cell * max);
Cell * NewRangeInt(BigInt * min, BigInt * max);
Cell * NewRangeIntN(Int32 min, Int32 max);
Cell * NewBitRange(UInt32 bit_count);

void PrintRange(Cell * cell);
Cell * RangeEval(Cell * cell);

#ifdef _DEBUG

Cell * RangeMin(Cell * range);
Cell * RangeMax(Cell * range);
Cell * RangeStep(Cell * range);

#else

#define RangeMin(range) ((range)->l)
#define RangeMax(range) ((range)->r)
#define RangeStep(range) ((range)->m)

#endif
