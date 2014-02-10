#define FOR_EACH_ITEM(en, item, list) item = FirstItem(&en, list); while(item != NULL) {
#define NEXT_ITEM(en, item) item = NextItem(&en); }

Var * FirstItem(Var ** p_en, Var * list);
Var * NextItem(Var ** p_en);

Bool ContainsItem(Var * tuple, Var * item);
