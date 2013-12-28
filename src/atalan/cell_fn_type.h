Cell * NewFnType(Type * arg, Type * result);

#ifdef _DEBUG

Type * ArgType(Type * arr_type);
Type * ResultType(Type * arr_type);

#else

#define ResultType(arr_type) (arr_type)->type
#define ArgType(arr_type) (arr_type)->l

#endif

