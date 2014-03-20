#include "language.h"

typedef struct {
	Var * min[5];
	Int16 min_dist;
	UInt16 cnt;
} SimmilarNames;

void SimmilarNamesInit(SimmilarNames * names)
{
	names->min_dist = 3;
	names->cnt = 0;
}

void SimmilarNamesAdd(SimmilarNames * names, char * name,  Var * v)
{
	Int16 dist;
	if (v->name != NULL && v->idx == 0) {
		dist = StrEditDistance(name, v->name);
		if (dist < 2) {
			if (dist < names->min_dist) {
				names->min[0] = v;
				names->min_dist = dist;
				names->cnt = 1;
			} else if (dist == names->min_dist) {
				if (names->cnt < 5) names->min[names->cnt] = v;
				names->cnt++;
			}
		}
	}
}

void ReportSimilarNames(char * name)
{
	SimmilarNames names;
	Var * v;
	UInt16 i;
	Var * scope;
	UInt8 proc_cnt;
	Bool printed = false;
	UInt16 len;
	UInt8 color;


	v = NULL;

	// Find names with lowest edit distance.
	// There may be up to 5 such names.
	// We are first trying to find the names within scope.

	len = StrLen(name);

	color = PrintColor(COLOR_HINTS);

	// Do not try to suggest simmilar names, if the variable consists of only one character

	if (len > 1) {
		SimmilarNamesInit(&names);
		for(scope = SCOPE; scope != NULL; scope = scope->scope) {
			FOR_EACH_LOCAL(scope, v)
				SimmilarNamesAdd(&names, name, v);
			NEXT_LOCAL
		}

		if (names.cnt > 0) {
			Print("Did you mean ");
			if (names.cnt > 5) names.cnt = 5;
			for(i=0; i<names.cnt; i++) {
				if (i>0) {
					if (i == names.cnt-1) {
						Print(" or ");
					} else {
						Print(" ,");
					}
				}
				PrintQuotedCellName(names.min[i]);
			}
			Print("?\n");
			printed = true;
		}
	}

	// We are trying to locate same variable in different scope
	SimmilarNamesInit(&names);
	FOR_EACH_VAR(v)
		if (!VarIsArg(v)) {
			SimmilarNamesAdd(&names, name, v);
		}
	NEXT_VAR

	if (names.min_dist == 0) {
		if (names.cnt > 0) {
			if (names.cnt > 5) names.cnt = 5;
			if (printed) PrintEOL();
			printed = false;
			proc_cnt = 0;
			for(i=0; i<names.cnt; i++) {
				v = names.min[i];
				if (v->line_no != 0) {

					if (!printed) {
						Print("Variable with the same name was declared in different scope:"); PrintEOL(); PrintEOL();
						printed = true;
					}

					Print("   "); Print(v->file->name); Print("("); PrintInt(v->line_no); Print(")"); 
					// Find some scope with name

					scope = v->scope;
					while(scope != NULL && scope->name == NULL) scope = scope->scope;
					if (scope != NULL) {
						if (scope->type->mode == INSTR_FN) {
							Print("  in procedure '"); 
						} else if (scope->type->mode == INSTR_VOID) {
							Print("  in scope '"); 
						}
						Print(scope->name); Print("'");
						proc_cnt++;
					}
					PrintEOL();
				}
			}

			if (proc_cnt > 0) {
				PrintEOL();
				Print("Did you mean to declare the variable as global?");
			}
		}
	}
	PrintColor(color);
}
