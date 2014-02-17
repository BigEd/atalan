/*

Cell boolean operations

(c) 2013 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php


*/

#include "../language.h"


Var * Not(Var * v)
{
	InstrOp op;
	Var * r;
	if (v == NULL) return NULL;

	op = v->mode;

	if (IS_RELATIONAL_OP(op)) {
		r = NewOp(OpNot(op), v->l, v->r);
	} else if (op == INSTR_NOT) {
		r = v->l;
	} else if (op == INSTR_AND) {
		r = NewOp(INSTR_OR, Not(v->l), Not(v->r));
	} else if (op == INSTR_OR) {
		r = NewOp(INSTR_AND, Not(v->l), Not(v->r));
//	} else if (op == INSTR_XOR) {
//		r = NewOp(INSTR_EQ, v->l, v->r);
	} else {
		r = NewOp(INSTR_NOT, v, NULL);
	}
	return r;
}
