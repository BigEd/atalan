typedef struct {
	Cell * init;		// initial value of the step
	InstrOp op;			// step_type INSTR_ADD, INSTR_SUB, INSTR_MUL, INSTR_DIV, ...
	Cell * step;		// type of argument (step value)
	InstrOp compare_op;
	Cell * limit;		// limit value of step (for ADD, MUL this is top value, for SUB, DIV this is bottom value)
} SequenceCell;
