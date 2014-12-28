Cell * NewArrayType(Type * index, Type * item);

#ifdef _DEBUG

Type * ItemType(Type * arr_type);
Type * IndexType(Type * arr_type);
Var * ArrayStep(Type * arr_type);
void SetArrayStep(Type * arr_type, Var * step);

#else

#define ItemType(arr_type) (arr_type)->type
#define IndexType(arr_type) (arr_type)->l
#define ArrayStep(arr_type) (arr_type)->r
#define SetArrayStep(arr_type, step) (arr_type)->r = step;

#endif

Var * ArrayTypeField(Var * arr_type, char * fld_name);
