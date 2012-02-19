/*

Int Limits

(c) 2011 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php


This module contains functions, that are used to define integer variable limits.
Instructions are generated to specified procedure and block of code.

Instructions are always generated to some specified instruction block, which is located in some procedure.
Current position, where will be the next instruction generated is stored in GENLOC varaible.

*/

#include "language.h"

#define BIGINT_MIN (-2147483647 - 1)		// To prevent error in some compilers parser
#define BIGINT_MAX 2147483647L

static BigInt BIGINT_0 = 0;
static BigInt BIGINT_1 = 1;

BigInt * Int0()
{
	return &BIGINT_0;
}

BigInt * Int1()
{
	return &BIGINT_1;
}

void IntInit(BigInt * dest, Int32 n)
{
	*dest = (BigInt)n;
}

void IntSet(BigInt * dest, BigInt * src)
{
	*dest = *src;
}

void IntSetMin(BigInt * dest)
{
	*dest = BIGINT_MIN;
}

void IntSetMax(BigInt * dest)
{
	*dest = BIGINT_MAX;
}

void IntAdd(BigInt * dest, BigInt * l, BigInt * r)
{
	*dest = *l + *r;
}

void IntSub(BigInt * dest, BigInt * l, BigInt * r)
{
	*dest = *l - *r;
}

void IntMul(BigInt * dest, BigInt * l, BigInt * r)
{
	*dest = *l * *r;
}

void IntDiv(BigInt * dest, BigInt * l, BigInt * r)
{
	*dest = *l / *r;
}

Bool IntEq(BigInt * l, BigInt * r)
{
	return *l == *r;
}

Bool IntLower(BigInt * l, BigInt * r)
{
	return *l < *r;
}

Bool IntHigher(BigInt * l, BigInt * r)
{
	return *l > *r;
}

Bool IntLowerEq(BigInt * l, BigInt * r)
{
	return IntLower(l,r) || IntEq(l,r);
}

Bool IntHigherEq(BigInt * l, BigInt * r)
{
	return IntHigher(l,r) || IntEq(l,r);
}

// Implementation independent funtions implemented using lower lever functions

BigInt * IntMin(BigInt * l, BigInt * r)
{
	return IntLower(l, r)?l:r;
}

BigInt * IntMax(BigInt * l, BigInt * r)
{
	return IntLower(l, r)?r:l;
}
