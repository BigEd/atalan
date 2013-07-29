void TypeLimits(Type * type, Var ** p_min, Var ** p_max);

BigInt * TypeMax(Type * type);
BigInt * TypeMin(Type * type);

UInt32 TypeSize(Type * type);
UInt32 VarByteSize(Var * var);
