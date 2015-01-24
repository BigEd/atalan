/*
Sequences

Sequence cell is used in type inferring.
It describes how is a variable modified in loop, when type can not be directly defined.
It is partial type. It cannot be used to define variables.

(c) 2013 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

Var * NewSequence(Var * init, Var * step, InstrOp step_op, Var * limit, InstrOp compare_op);
Bool  SequenceRange(Type * type, Var ** p_min, Var ** p_max);
Var * ResolveSequence(Var * cell);
void PrintSequence(Cell * cell);
Cell * CollapseSequence(Cell * cell);


typedef struct {
	Cell * init;		// initial value of the step
	InstrOp op;			// step_type INSTR_ADD, INSTR_SUB, INSTR_MUL, INSTR_DIV, ...
	Cell * step;		// type of argument (step value)
	InstrOp compare_op;
	Cell * limit;		// limit value of step (for ADD, MUL this is top value, for SUB, DIV this is bottom value)
} SequenceCell;
