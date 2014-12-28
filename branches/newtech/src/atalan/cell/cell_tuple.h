#define FOR_EACH_ITEM(en, item, list) item = FirstItem(&en, list); while(item != NULL) {
#define NEXT_ITEM(en, item) item = NextItem(&en); }

Var * FirstItem(Var ** p_en, Var * list);
Var * NextItem(Var ** p_en);

Var * NewTuple(Var * left, Var * right);
Bool ContainsItem(Var * tuple, Var * item);

typedef struct {
	Cell * list;
	Cell * last_tuple;
} ListBuilder;

void ListInit(ListBuilder * cl);
void ListAppend(ListBuilder * cl, Cell * item, InstrOp op);
