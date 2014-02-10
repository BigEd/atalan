Cell * NewFnType(Type * arg, Type * result);
void FnTypeFinalize(Type * fn_type);

#ifdef _DEBUG

Type * ArgType(Type * fn_type);
Type * ResultType(Type * fn_type);

#else

#define ResultType(fn_type) (fn_type)->type
#define ArgType(fn_type) (fn_type)->l

#endif

