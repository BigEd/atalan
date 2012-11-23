typedef Int32 BigInt ;

void IntSet(BigInt * dest, BigInt * src);
void IntInit(BigInt * dest, Int32 n);
void IntSetMin(BigInt * dest);
void IntSetMax(BigInt * dest);

Int32 IntN(BigInt * n);

BigInt * IntMin(BigInt * l, BigInt * r);
BigInt * IntMax(BigInt * l, BigInt * r);

BigInt * Int0();
BigInt * Int1();

void IntAdd(BigInt * dest, BigInt * l, BigInt * r);
void IntSub(BigInt * dest, BigInt * l, BigInt * r);
void IntMul(BigInt * dest, BigInt * l, BigInt * r);
void IntDiv(BigInt * dest, BigInt * l, BigInt * r);

Bool IntEq(BigInt * l, BigInt * r);
Bool IntLower(BigInt * l, BigInt * r);
Bool IntHigher(BigInt * l, BigInt * r);
Bool IntLowerEq(BigInt * l, BigInt * r);
Bool IntHigherEq(BigInt * l, BigInt * r);

Bool IntIsN(BigInt * l, Int32 N);

void IntMulN(BigInt * dest, Int32 N);
void IntDivN(BigInt * dest, Int32 N);
void IntAddN(BigInt * dest, Int32 N);