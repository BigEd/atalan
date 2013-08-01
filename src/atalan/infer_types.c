/*

Infer types

(c) 2010 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

*/

#include "language.h"

#define RESTRICTION 0

UInt32 paths;		// number of paths
UInt32 undefined;
Bool   looped;		//self referencing and looped (without found bound)

typedef struct {
	Bool modified;
	Bool modified_blocks;
	Bool final_pass;
} InferData;

Type * FindTypeCall(Var * proc, Var * var)
/*
Purpose:
	Find type of variable.
*/
{
	Var * arg;
	FOR_EACH_OUT_ARG(proc, arg)
		if (arg == var) return arg->type;
	NEXT_OUT_ARG
	return NULL;
}

Type * TypeRestrictBlk(Type * type, Var * var, InstrBlock * blk, Bool neg)
{
	Type * rt, * vt;
	Instr * i;
	InstrOp op;
	Var * var2;

	rt = type;

	// 1. Undefined type may not be restricted
	if (type == NULL || type->variant == TYPE_UNDEFINED) goto done;

	if (blk == NULL) goto done;
	i = blk->last;
	if (i == NULL) goto done;
	op = i->op;
	if (!IS_INSTR_BRANCH(i->op)) goto done;

	// Select appropriate variable
	if (i->arg1 == var) {
		var2 = i->arg2;
		vt = i->type[ARG2];
	} else if (i->arg2 == var) {
		var2 = i->arg1;
		op = OpRelSwap(op);
		vt = i->type[ARG1];
	} else {
		goto done;
	}

	// If this is path not taken, swap the operation
	if (neg) {
		op = OpNot(op);
	}

	//TODO: We may use the inferred type from instruction
	if (vt == NULL) vt = var2->type;

	rt = TypeRestrictOp(type, vt, op);

done:
	return rt;
}


Type * FindTypeBlock(Loc * loc, Var * var, Type * index_type, InstrBlock * blk, Instr * instr)
/*
Purpose:
	Find type of variable var used in instruction at location loc in specified block beginning at instruction instr.
Result:
	NULL             Specified variable is not modified in this block (or any block that jumps to this block).
	                 This may happen for example for loops, where the variable is not modified in the loop.
                     When NULL is combined with other type, result is the non-null type.
	TYPE_UNDEFINED   Undefined type means, that the type is somehow modified, but we do not know exactly how.
	                 (We cannot infer the type).
					 When UNDEFINED type is combined with any other type, result is UNDEFINED type.
*/
{
	Instr * i;
	InstrBlock * caller;
	Type * type, * type2, * old_type;
	Var * var2;
	UInt16 caller_count;

	if (blk == NULL) return NULL;

#ifdef TRACE_INFER
	PrintRepeat("  ", g_fb_level); Print("#"); PrintInt(blk->seq_no); Print("\n");
#endif
	// If the block has been already processed, return the remembered result.
	// In case of loop, the result will be NULL, which means this branch does not alter the type of the variable in any way.

	if (blk->processed) goto done2;

	blk->type = NULL;

	if (instr == NULL) {
		i = blk->last;
		blk->processed = true;
	} else {
		i = instr->prev;
	}

	type = NULL;

	// Definition of the variable may be in this block
	for(; i != NULL; i = i->prev) {
		if (i->op == INSTR_LINE) continue;

		if (i->op == INSTR_CALL) {
			type = FindTypeCall(i->result, var);
			if (type != NULL) goto done;
		}

		if (i->result == NULL) continue;

		if (VarIdentical(i->result,var)) {

			type = i->type[RESULT];
			if (type != NULL && FlagOff(i->flags, InstrRestriction)) goto done;

			// We have found the same instruction again.
			// This means, it is part of some loop.
			if (i == loc->i) {
				looped = true;
				if (InstrIsSelfReferencing(i)) {

					if (!VarIdentical(i->arg2, i->result)) {
						var2 = i->arg2;
						if (i->type[ARG2] != NULL) {
							type = NewSequence(NULL, i->type[ARG2], i->op, NULL, INSTR_VOID);
							goto sub1;		//continue;
						}
					}
					type = TUNDEFINED;	// we are not able to deduct the type of the instruction now
					goto done;
				}
			}

//			if (type == NULL || FlagOn(i->flags, InstrRestriction)) type = TUNDEFINED;
			type = TUNDEFINED;
sub1:
			#ifdef TRACE_INFER
				PrintRepeat("  ", g_fb_level); Print("instr:"); PrintType(type); Print("\n");
			#endif
			goto done;
		// For array, we check, that A(x) fits B(y) so, that A = B and x contains y
		} else if (var->mode == INSTR_ELEMENT) {
			if (i->result->mode == INSTR_ELEMENT && var->adr == i->result->adr) {
				// Now we need to find the type of result index & type of var index
				if (IsSubset(index_type, i->result_index_type)) {
					type = i->type[RESULT];
					if (type != NULL && FlagOff(i->flags, InstrRestriction)) goto done;
				}
			}
		}
	}


	// We are in starting block and there is no definition.
	// This is use of undefined variable. (Except for procedure input arguments, in variables and global variables).
	
	if (blk->from == NULL && blk->callers == NULL) {
		// If we are at the beginning of the procedure and this is an input argument, we can use the type of variable as an argument.
		// Input register variables are considered defined here too.
		if (VarIsInArg(var) || InVar(var)) {
			type = var->type;
		} else {
			undefined++;
			type = TUNDEFINED;
		}
	}


	old_type = type;
	caller_count = 0;
	if (blk->from != NULL) {
		caller_count++;
#ifdef TRACE_INFER
		g_fb_level++;
#endif
		type2 = FindTypeBlock(loc, var, index_type, blk->from, NULL);
#ifdef TRACE_INFER
		g_fb_level--;
#endif

		// This is just speed optimization, if the type is undefined, we do not need to continue processing
		if (type2 != NULL) {
			if (type2->variant == TYPE_UNDEFINED) {
				type = type2;
				goto done;
			}
			type2 = TypeRestrictBlk(type2, var, blk->from, true);
			type = VarUnion(type, type2);
			type = TypeRestrictBlk(type, var, blk->from, true);
		}
	}

	for(caller = blk->callers; caller != NULL; caller = caller->next_caller) {
		caller_count++;
//		if (caller_count > 1) paths++;
#ifdef TRACE_INFER
		g_fb_level++;
#endif
		type2 = FindTypeBlock(loc, var, index_type, caller, NULL);
#ifdef TRACE_INFER		
		g_fb_level--;
#endif
		if (type2 != NULL) {
			if (type2->variant == TYPE_UNDEFINED) {
				type = type2;
				goto done;
			}
			type2 = TypeRestrictBlk(type2, var, caller, false);
			type = VarUnion(type, type2);
			type = TypeRestrictBlk(type, var, caller, false);
		}
	}

	if (caller_count > 0) paths += caller_count - 1;

done:
	blk->type = type;
done2:
	return blk->type;
}

Type * FindType(Loc * loc, Var * var, Bool report_errors)
/*
Purpose:
	Find type of variable at specified location.
Result:
	NULL if type was not found or is undefined
*/
{
	Type * type = NULL;
	Type * arr_type;
	Type * index_type;
	Type * left, * right;

	if (var == NULL) return NULL;

	// Type of integer constants

	switch(var->mode) {
	case INSTR_INT:
		type = var;
		break;

	case INSTR_RANGE:
		left = FindType(loc, var->adr, report_errors);
		right = FindType(loc, var->var, report_errors);
		type = NewRange(left, right);
		break;
		
	case INSTR_TUPLE:
		left = FindType(loc, var->adr, report_errors);
		right = FindType(loc, var->var, report_errors);
		type = TypeTuple(left, right);
		break;

	case INSTR_ADD:
	case INSTR_SUB:
	case INSTR_MUL:
	case INSTR_DIV:
	case INSTR_MOD:
		left = FindType(loc, var->l, report_errors);
		right = FindType(loc, var->r, report_errors);
		if (left != NULL && right != NULL) {
			type = CellOp(var->mode, left, right);
		}
		break;

	case INSTR_BYTE:
		type = TypeByte();
		break;

	case INSTR_TYPE:
		type = var;
		break;

	default:
		if (var->type->variant == TYPE_PROC) {
			type = var->type;
		} else {

#ifdef TRACE_INFER
		g_fb_level = 0;
#endif
			paths = 1;
			looped = false;
			undefined = 0;
			MarkBlockAsUnprocessed(loc->proc->instr);
			index_type = NULL;
			if (VarIsArrayElement(var)) {
				index_type = FindType(loc, var->var, false);
			}
			type = FindTypeBlock(loc, var, index_type, loc->blk, loc->i);

			// Type has not been specified in previous code
			if (type->variant == TYPE_UNDEFINED) {

				if (var->mode == INSTR_ELEMENT) {
					arr_type = var->adr->type;
					if (VarIsArrayElement(var)) {
						type = arr_type->element;
					//Using address to access array element
					} else if (arr_type->variant == TYPE_ADR) {
						if (arr_type->element->variant == TYPE_ARRAY) {
							// adr of array of type
							type = arr_type->element->element;
						} else {
							Print("");
						}
					}
				}
			}

			// Let-adr instruction generates address of specified variable.
			// The address will be valied even in case the variable has not been initialized yet, therefore
			// we will not report 'uninitialized variable'.
			if (loc->i->op == INSTR_LET_ADR) {
				type = TypeAdrOf(type);
			} else {
				// Type was not found. This means, the variable has not been assigned yet (or at least at some path).
				// In such case, we are not able to infer the type (or infer it completely).
				// User will be asked to specify the type for the variable.
				// It also means, we may be using undefined variable!

				if (type->variant == TYPE_UNDEFINED) {

					if (report_errors) {
						// Argument to let adr 
						if (undefined > 0) {
							// Input register does not have to be explicitly initialized.
							// Global variables are supposed to be initialized before entering the procedure.
							if (!InVar(var) && (var->mode == INSTR_VAR) && VarIsLocal(var, loc->proc)) {
								ErrArg(var);
								if (paths > 1) {
									LogicWarningLoc("Possible use of uninitialized variable [A].\nThere exists a path where it is not initialized before it is used here.", loc);
								} else {
									LogicWarningLoc("Use of uninitialized variable [A].", loc);
								}
							}
						} else if (looped) {
							LogicWarningLoc("looped variable", loc);
						}
					}
				}
			}
		}
	}

	if (type != NULL && type->variant == TYPE_UNDEFINED) type = NULL;
	return type;
}

Bool DistributeRestrictionBlk(Loc * loc, Var * var, Type * restriction, InstrBlock * blk, Instr * instr)
/*
Purpose:
	Spread information about restriction of specified variable.
	The type specified defines restriction.
Result:
	Returns true, if something was modified.
*/
{
	Instr * i;
//	InstrBlock * caller;
	Type * type;
	Bool modified = false;

	if (blk == NULL) return false;

	// If the block has been already processed, return the remembered result.
	// In case of loop, the result will be NULL, which means this branch does not alter the type of the variable in any way.

	if (blk->processed) return false;

	if (instr == NULL) {
		i = blk->last;
		blk->processed = true;
	} else {
		i = instr->prev;
	}

	// Definition of the variable may be in this block
	for(; i != NULL; i = i->prev) {
		if (i->op == INSTR_LINE) continue;
		if (i->result == NULL) continue;

		if (VarIdentical(i->result, var)) {

			// We have found the same instruction again.
			// This means, it is part of some loop.
			if (i == loc->i) {
				if (InstrIsSelfReferencing(i)) {
					looped = true;

					type = TUNDEFINED;	// we are not able to deduct the type of the instruction now
					goto done;
				}
			}

			type = i->type[RESULT];
			if (type == NULL || type->variant == TYPE_UNDEFINED) {
				i->type[RESTRICTION] = restriction;
				SetFlagOn(i->flags, InstrRestriction);
				modified = true;
			} else {
				// type may be already restriction
				// then we came from different branch here and the restrictions should combine
			}
			goto done;
		}
	}

	// We are in starting block and we haven't found the variable.
	// It must be either input argument or global variable.
	// In other case, this would be use of undefined variable, that is however handled elsewhere.
	
	if (blk->from == NULL && blk->callers == NULL) {
		type = CellIntersection(var->type, restriction);
		if (type != var->type) {
			var->type = type;
			modified = true;
		}
	}


done:
	blk->processed = true;
	return modified;
}

Bool PropagateConstraint(Loc * loc, Var * var, Type * restriction, InstrBlock * blk, Instr * instr)
{
	paths = 1;
	looped = false;
	undefined = 0;
	MarkBlockAsUnprocessed(loc->proc->instr);

	return DistributeRestrictionBlk(loc, var, restriction, blk, instr);
}

void VarConstraints(Loc * loc, Var * var, InferData * d)
{
	Var * idx;
	Type * ti;

	// Index of array access must match the type specified in array
	if (VarIsArrayElement(var)) {
		idx = var->var;
		if (idx->mode == INSTR_VAR || idx->mode == INSTR_INT) {
			ti = FindType(loc, idx, d->final_pass);
			// Type of the index is undefined, this is restriction
			if (ti == NULL || ti->variant == TYPE_UNDEFINED) {
				if (PropagateConstraint(loc, idx, var->adr->type->index, loc->blk, loc->i)) {
					d->modified = true;
				}
			}
		}
	}
}

Bool TypeBitMask(Type * type, UInt32 * p_bit_size)
{
	UInt32 max_bits;
	BigInt * max;

	max = TypeMax(type);
	if (max != NULL) {
		max_bits = IntBitSize(max);
		*p_bit_size = max_bits;
		return true;
	}
	return false;
}

Type * BitType(InstrOp op, Type * left, Type * right)
{
	Type * rt = NULL;
	UInt32 left_bits, right_bits, bits;

	if (TypeBitMask(left, &left_bits) && TypeBitMask(right, &right_bits)) {
		bits = left_bits; 
		switch(op) {
		case INSTR_XOR:
			if (right_bits > left_bits) bits = right_bits;
			break;
		case INSTR_OR:
			if (right_bits > left_bits) bits = right_bits;
			break;
		case INSTR_AND:
			if (right_bits < left_bits) bits = right_bits;
			break;
		}

		rt = NewBitRange(bits);
	}

	return rt;
}

Type * TypeEval(InstrOp op, Type * left, Type * right);

Type * IntTypeEval(InstrOp op, Type * left, Type * right)
{
	Type * rt = NULL;

	// When assigning the value, resulting type is simply the same as original value
	switch(op) {

	case INSTR_LO:
	case INSTR_HI:
		rt = TypeByte();		//NewRangeInt(0, 255);
		break;
	
	default:
		if (right != NULL && right->variant == TYPE_INT) {

			switch(op) {
			case INSTR_XOR:
			case INSTR_AND:
			case INSTR_OR:
				rt = BitType(op, left, right);
				break;
			default:
				rt = CellOp(op, left, right);
			}
		}
	}
	return rt;
}

Type * SeqTypeEval(InstrOp op, Type * left, Type * right)
{
	Type * rt = NULL;
//	RangeTransform r_fn;

	if (right->variant == TYPE_INT) {
		switch(op) {
		case INSTR_SUB:
		case INSTR_ADD:
			if (left->seq.init != NULL) {
				rt = NewSequence(
					TypeEval(op, left->seq.init, right),
					left->seq.step,
					left->seq.op,
					left->seq.limit,
					left->seq.compare_op
					);
			}
			break;
		}
	}
	//TODO: Only if there is same operation and step
	return rt;
}


Type * AdrTypeEval(InstrOp op, Type * left, Type * right)
{
	Type * rt = NULL;

	switch(op) {

	case INSTR_LO:
	case INSTR_HI:
		rt = TypeByte();
		break;
		default:
		break;
	}
	return rt;
}

Type * TypeEval(InstrOp op, Type * left, Type * right)
{
	Type * rt = NULL;

	if (left != NULL) {

		if (op == INSTR_LO || op == INSTR_HI) {
			rt = TypeByte();
		} else if (op == INSTR_LET_ADR) {
			rt = TypeAdrOf(left);
		} else if (op == INSTR_LET) {
			rt = left;
		} else {

			rt = CellOp(op, left, right);
/*
			switch(left->variant) {
			case TYPE_INT:
				rt = IntTypeEval(op, left, right);
				break;
			case TYPE_SEQUENCE:
				rt = SeqTypeEval(op, left, right);
				break;
			case TYPE_ADR:
				rt = AdrTypeEval(op, left, right);
				break;
			default:
				ASSERT("Unknown type eval.");
			}
*/
		}
	}

	return rt;
}

Bool InstrConstraints(Loc * loc, void * data)
/*
Deduce type of instruction arguments based on type of result, other argument and the type of operation.
*/
{
	Var * result;
	InstrOp op;
	Instr * i;
	Type * tr, * tl;
	InferData * d = (InferData *)data;

	i = loc->i;
	result = i->result;
				
	if (i->type[RESULT] != NULL) {
		if (i->arg1 != NULL && i->type[ARG1] == NULL) {
			if (i->type[ARG2] != NULL) {
				// Restriction given by instruction
				tl = i->type[RESULT];
				tr = i->type[ARG2];
				op = INSTR_NULL;
				switch(i->op) {
				case INSTR_ADD: op = INSTR_SUB;	break;
				case INSTR_SUB: op = INSTR_ADD; break;
				case INSTR_MUL: op = INSTR_DIV; break;
				case INSTR_DIV: op = INSTR_MUL; break;
				default:
					break;
				}
				if (op != INSTR_NULL) {
					tr = TypeEval(op, tl, tr);
					if (PropagateConstraint(loc, i->arg1, tr, loc->blk, i)) {
						d->modified = true;
					}
				}
			}
		}

		if (i->arg2 != NULL && i->type[ARG2] == NULL) {
		}
	}

	VarConstraints(loc, i->result, d);
	VarConstraints(loc, i->arg1, d);
	VarConstraints(loc, i->arg2, d);
	return false;
}

void CheckIndex(Loc * loc, Var * var)
// Check array indexes
// Array may have one or two indexes
// Index may be simple, or it can be range
{
	Type * type;
	Var * idx;

	if (VarIsArrayElement(var)) {
		idx = var->var;
		type = FindType(loc, var->var, true);
		if (type != NULL) {
			if (!IsSubset(type, var->adr->type->index)) {
				if (type->variant == TYPE_INT) {
					ErrArg(CellMax(type));
					ErrArg(CellMin(type));
					ErrArg(var->adr);
					LogicWarningLoc("Index of array [A] out of bounds.\nThe index range is [B]..[C].", loc);
				} else {
					ErrArg(var->adr);
					LogicErrorLoc("Cannot infer type of index of array [A].", loc);
				}
			}
		} else {
			// failed to compute index, what does it means?
		}
	}

}

#define MIN1 TypeMin(i->type[ARG1])
#define MAX1 TypeMax(i->type[ARG1])
#define MIN2 TypeMin(i->type[ARG2])
#define MAX2 TypeMax(i->type[ARG2])

Bool TypeIsComplete(Type * type)
{
	if (type == NULL) return false;
	if (type->mode == INSTR_SEQUENCE) {
		return type->seq.init != NULL && type->seq.step != NULL && type->seq.limit != NULL && type->seq.op != INSTR_VOID;
	}
	return true;
}

void ReplaceConst(Var ** p_var, Type * type)
/*
Purpose:
	If the possible type of variable is range where min = max, replace the variable with constant.
*/
{
	Var * var;
	var = *p_var;
	if (var != NULL && var->mode != INSTR_INT && !InVar(var)) {
		if (type != NULL && type->variant == TYPE_INT && IntEq(&type->range.min, &type->range.max)) {
			*p_var = IntCell(&type->range.min);
		}
	}
}


Bool InstrInferType(Loc * loc, void * data)
{
	Var * result;
	Instr * i;
	Type * tr, * ti;
	InferData * d = (InferData *)data;
	Bool taken, not_taken, not;

	i = loc->i;

	if (loc->blk->seq_no == 1 && loc->n == 7) {
		Print("");
	}

	// Conditional jumps have special support
	if (i->op == INSTR_IF) {
		if (i->type[ARG1] == NULL) {
			i->type[ARG1] = FindType(loc, i->arg1, d->final_pass);
			if (i->type[ARG1] != NULL) d->modified = true;
		}
		ReplaceConst(&i->arg1, i->type[ARG1]);

		if (i->type[ARG1] != NULL && !InVar(i->arg1)) {
			taken = false;
			not_taken = false;
			not = false;
			switch (i->op) {
			case INSTR_NE:
				not = true;
			case INSTR_EQ:
				// We know for sure the condition is true, if both values are in fact same integer constants
				taken = CellIsIntConst(i->type[ARG1]) && CellIsIntConst(i->type[ARG2]) && IntEq(MIN1, MIN2);
				not_taken = IntLower(MAX1, MIN2) || IntHigher(MIN1, MAX2);
				break;

			case INSTR_GE:
				not = true;
			case INSTR_LT:
				taken = IntLower(MAX1, MIN2);
				not_taken = IntHigherEq(MIN1, MAX2);
				break;

			case INSTR_LE:
				not = true;
			case INSTR_GT:
				taken = IntHigher(MIN1, MAX2);
				not_taken = IntLowerEq(MAX1, MIN2);
				break;
			default:
				break;

			case INSTR_NMATCH_TYPE:
				not = true;
			case INSTR_MATCH_TYPE:
				if (TypeIsComplete(i->type[ARG1]) && TypeIsComplete(i->type[ARG2])) {
					if (TypeIsEqual(i->type[ARG1], i->type[ARG2])) {
						taken = true;
					} else {
						not_taken = true;
					}
				}
				break;
			}

			if (not) {
				if (taken) {
					not_taken = true;
					taken = false;
				} else if (not_taken) {
					taken = true;
					not_taken = false;
				}
			}

			if (taken) {
				// Change to goto
//				i->op = INSTR_GOTO;
//				i->arg1 = i->arg2 = NULL;
				i->arg1 = ONE;
				d->modified_blocks = true;
				return true;
			} else if (not_taken) {
				i->result->write--;
				InstrDelete(loc->blk, i);
				d->modified_blocks = true;
				return true;
			}
		}

	} else if ((i->result != NULL && (i->type[RESULT] == NULL || FlagOn(i->flags, InstrRestriction)))) {

		// If the result of the instruction is array index, try to infer type of the index.
		// The type may be later used when looking for the type of some instruction argument.

		if (i->result_index_type == NULL && VarIsArrayElement(i->result)) {
			i->result_index_type = FindType(loc, i->result->var, d->final_pass);
			if (i->result_index_type != NULL) d->modified = true;
		}

		if (i->arg1 != NULL && i->type[ARG1] == NULL) {
			i->type[ARG1] = FindType(loc, i->arg1, d->final_pass);
			if (i->type[ARG1] != NULL) d->modified = true;
		}

		if (i->arg2 != NULL && i->type[ARG2] == NULL) {
			i->type[ARG2] = FindType(loc, i->arg2, d->final_pass);
			if (i->type[ARG2] != NULL) d->modified = true;
		}

		// In some cases, we may find out, that the type at this place does allow only one value.
		// That means, we may replace the variable with the constant.
		ReplaceConst(&i->arg1, i->type[ARG1]);
		ReplaceConst(&i->arg2, i->type[ARG2]);

		// For comparisons, we may check whether the condition is not always true or always false
//		if (IS_INSTR_BRANCH(i->op)) {

		result = i->result;

		tr = TypeEval(i->op, i->type[ARG1], i->type[ARG2]);

		// Type was evaluated, test, whether there is not an error while assigning it
		if (tr != NULL /*&& !InstrIsSelfReferencing(i)*/) {
			if (FlagOn(result->submode, SUBMODE_USER_DEFINED) || (result->mode == INSTR_ELEMENT && FlagOn(result->adr->submode, SUBMODE_USER_DEFINED))) {

				// We allow assigning values to arrays, so we must allow this operation in type checker

				ti = result->type;

				// We initialize array with list of elements
				// TODO: Parser should probably create element of array borders, to distinguish it from
				//       assigning arrays.

				if (i->op == INSTR_LET && ti->variant == TYPE_ARRAY && tr->variant != TYPE_ARRAY) {
					ti = ti->element;
				}

				if (!IsSubset(tr, ti)) {
					if (d->final_pass) {
						if (tr->variant == TYPE_INT && ti->variant == TYPE_INT) {
							ErrArg(result);
							ErrArg(IntCell(&ti->range.max));
							ErrArg(IntCell(&ti->range.min));
							ErrArg(IntCell(&tr->range.max));
							ErrArg(IntCell(&tr->range.min));
							if (CellIsIntConst(tr)) {
								LogicWarningLoc("The value [A] does not fit into variable", loc);
							} else {
								LogicWarningLoc("Result of expression does not fit the target variable.\nThe range of result is [A]..[B]. The range of variable is [C]..[D].", loc);
							}
						} else {
							LogicWarningLoc("Value does not fit into variable", loc);								
						}
					}
				}
			}

		// If the resulting type is defined and we failed to compute the type, we may try to
		// deduce the type 'backwards'.
		} else {
		}

		if (tr != NULL) {
			i->type[RESULT] = tr;
			SetFlagOff(i->flags, InstrRestriction);
			d->modified = true;
		}
	}

	if (d->final_pass) {
//		if (loc->blk->seq_no == 8 && loc->n == 15) {
//			Print("");
//		}
		CheckIndex(loc, i->result);
		CheckIndex(loc, i->arg1);
		CheckIndex(loc, i->arg2);
	}

	return false;
}

/*

Type inferencer tries to determine type for result of every instruction (that is for every instruction that has result).

1. Type was deduced
2. Type was not deduced, but the destination variable has been defined by user (use the user defined type)
3. Type was not deduced, destination variable is immediately assigned to variable with defined type

*/

Bool InstrInitInfer(Loc * loc, void * data)
{
	Instr * i = loc->i;
	i->type[0] = i->type[1] = i->type[2] = NULL;
	i->result_index_type = NULL;
	i->flags = 0;
	return false;
}

Bool LoopSteps(Instr * i, BigInt * min_steps, BigInt * max_steps)
/*
Purpose:
	Compute number of steps performed by loop depending on loop instruction.
*/
{
	Type * type;
	BigInt min_span, max_span;

	// Compute number of repeats for this loop (returned as integer type)
	if (i->type[ARG1] != NULL && i->type[ARG2] != NULL) {
		type = i->type[ARG1];
		if (type->mode == INSTR_SEQUENCE) {
			if (TypeIsInt(type->seq.step) && TypeIsInt(type->seq.init) && TypeIsInt(type->seq.limit)) {

				max_span = (type->seq.limit->range.max - type->seq.init->range.min) + 1;
				min_span = (type->seq.limit->range.min - type->seq.init->range.max) + 1;

				if (type->seq.op == INSTR_ADD) {
					*min_steps = min_span / type->seq.step->range.max;
					*max_steps = max_span / type->seq.step->range.min;
					return true;
				}
			}
		}
	}
	return false;
}

Bool UseLoop(Var * proc, InstrBlock * header, InstrBlock * end, BigInt min_steps, BigInt max_steps)
{
	Bool modified = false;
	Loc loc;
	InstrBlock * stop = end->next;
	Instr * i;
	Type * type;
	BigInt stop_min, stop_max;

	loc.proc = proc;
	loc.blk  = header;
	for(loc.blk = header; loc.blk != stop; loc.blk = loc.blk->next) {
		for(loc.i = loc.blk->first; loc.i != NULL; loc.i = loc.i->next) {
			i = loc.i;
			type = i->type[RESULT];
			if (type != NULL && type->mode == INSTR_SEQUENCE) {
				if (TypeIsInt(type->seq.step) && TypeIsInt(type->seq.init) && type->seq.limit == NULL) {
					if (type->seq.op == INSTR_ADD) {
						stop_min = type->seq.init->range.min + ((min_steps - 1) * type->seq.step->range.min);
						stop_max = type->seq.init->range.max + ((max_steps - 1) * type->seq.step->range.max);
						i->type[RESULT] = NewRangeInt(&stop_min, &stop_max);
						modified = true;
					} else if (type->seq.op == INSTR_SUB) {
//						stop_max = type->seq.init->range.max - ((min_steps - 1) * type->seq.step->range.min);
						stop_min = type->seq.init->range.min - ((max_steps - 1) * type->seq.step->range.max);
						i->type[RESULT] = NewRangeInt(&stop_min, &type->seq.init->range.max);
						modified = true;
					}
				}
			}
		}
	}
	return modified;
}

static Bool TypeInferLoops(Var * proc)
/*
Purpose:
	Try to compute number of steps of every loop in the procedure.
	If sucessfull, this information may be used to infer some more types.
*/ {
	InstrBlock * header;
	Instr * i;
	BigInt min_steps, max_steps;

	Bool modified = false;
	Loc loc;

	loc.proc = proc;

	MarkLoops(proc);
	for(loc.blk = proc->instr; loc.blk != NULL; loc.blk = loc.blk->next) {
		header = NULL;
		if (loc.blk->jump_type == JUMP_LOOP) {
			header = loc.blk->cond_to;
			if (header == NULL) header = loc.blk->to;
		}

		// This is end of loop.
		// We may try to infer some information if we know maximal and/or minimal number of repeats
		if (header != NULL) {
//			Print("Loop "); PrintInt(header->seq_no); Print(".."); PrintInt(loc.blk->seq_no); PrintEOL();
			i = LastInstr(loc.blk);

			// Check sequence like
			//     ifeq ...  goto LAB
			//     ----
			//     goto loop
			//     ----
			//LAB@
			if (i == NULL && loc.blk->callers == NULL && loc.blk->from != NULL) {
				i = LastInstr(loc.blk->from);
				if (i != NULL && IS_INSTR_BRANCH(i->op)) {
					if (LoopSteps(i, &min_steps, &max_steps)) {
					}
				}
			} else if (i != NULL && IS_INSTR_BRANCH(i->op)) {
//				InstrPrint(i);
				if (LoopSteps(i, &min_steps, &max_steps)) {
					// we have the number of steps of this loop
					modified = UseLoop(proc, header, loc.blk, min_steps, max_steps);
				}
			}
		}
	}

	return modified;
}

static Bool InstrFreeIncomplete(Loc * loc, void * data)
{
	UInt8 n;
	Instr * i = loc->i;

	Type * type;
	for(n=0; n<=2; n++) {
		type = i->type[n];
		if (type != NULL && !TypeIsComplete(type)) i->type[n] = NULL;
	}
	return false;
}

Var * FindNamedDestVar(Loc * loc, Var * var)
/*
Purpose:
	Find non temporary variable, into which the specified variable is assigned.
	The variable is search for reporting purposes (therefore it must have name).
*/
{

	Instr * i;
	for(i = loc->i; i != NULL; i = i->next) {
		if (i->op == INSTR_LET && i->arg1 == var) {
			var = i->result;
			if (!VarIsTmp(var)) return var;
		}
	}
	return NULL;
}

void TypeDeduce(Var * proc)
{
	InferData data;
	UInt16 steps;

	data.final_pass = false;
retry:
	do {
		// 1. For every instruction in the code try to infer the type of it's result
		// 2. Repeat this until no new result type was inferred
		steps = 0;
		do {
			data.modified = false;
			data.modified_blocks = false;
			ProcInstrEnum(proc, &InstrInferType, &data);
			if (data.modified_blocks) {
				GenerateBasicBlocks(proc);
				DeadCodeElimination(proc);
			}
			steps++;
		} while (data.modified);

		steps = 0;
		do {
			data.modified = false;
			ProcInstrEnum(proc, &InstrConstraints, &data);
			steps++;
		} while(data.modified);

	} while (steps > 1);

	if (TypeInferLoops(proc)) {
		// Some of the types may be incomplete sequences, we need to clear these types so they will be evaluated again
		ProcInstrEnum(proc, &InstrFreeIncomplete, NULL);
		goto retry;
	}

}

Bool InstrUseDeclaredTypes(Loc * loc, void * data)
{
	Instr * i;
	InferData * d = (InferData *)data;

	i = loc->i;

	if (i->result != NULL && i->type[0] == NULL) {
		if (i->arg1 != NULL && i->type[1] == NULL) {
			if (FlagOn(i->arg1->submode, SUBMODE_USER_DEFINED)) {
				i->type[1] = i->arg1->type;
				d->modified = true;
			}
		}

		if (i->arg2 != NULL && i->type[2] == NULL) {
			if (FlagOn(i->arg2->submode, SUBMODE_USER_DEFINED)) {
				i->type[2] = i->arg2->type;
				d->modified = true;
			}
		}

		if ((i->arg1 != NULL && i->type[1] == NULL) || (i->arg2 != NULL && i->type[2] == NULL)) {
			if (FlagOn(i->result->submode, SUBMODE_USER_DEFINED)) {
				i->type[0] = i->result->type;
				d->modified = true;
			}
		}
	}
	return false;
}

extern char * TMP_NAME;

void ReportUnusedVars(Var * proc)
/*
Purpose:
	Report variables declared in this procedure that are not used.
*/
{
	Var * var;
	Type * type;

	FOR_EACH_LOCAL(proc, var)
		if (var->name != NULL && var->name != TMP_NAME && FlagOff(var->submode, SUBMODE_SYSTEM) && var->mode == INSTR_VAR && FlagOff(var->submode, SUBMODE_USED_AS_TYPE)) {
			type = var->type;
			if (type != NULL && type->variant == TYPE_LABEL) continue;
			if (var->read == 0) {
				ErrArg(var);
				if (var->write == 0) {
					LogicWarning("Unreferenced variable [-A].", SetBookmarkVar(var));
				} else {
					if (!OutVar(var)) {
						LogicWarning("Unread variable [-A].", SetBookmarkVar(var));
					}
				}
			}
		}
	NEXT_LOCAL
}

void ReportAlwaysFalseAsserts(Var * proc)
/*
	Test, if there is some assert that is known at compile time to be always false.
	Such assert reports error.
*/
{
	Bool assert_begin;
	Instr * i;
	Loc loc;

	for(loc.blk = proc->instr; loc.blk != NULL; loc.blk = loc.blk->next) {
		assert_begin = false;
		for(i = loc.blk->first; i != NULL; i = i->next) {
			if (i->op == INSTR_ASSERT_BEGIN) {
				assert_begin = true;
			} else if (i->op == INSTR_ASSERT) {
				if (assert_begin) {
					loc.i = i;
					LogicErrorLoc("Assert is always false.", &loc);
				}
			}
		}
	}
}

void TypeInfer(Var * proc)
/*
Purpose:
	Try to infer types for all variables used in given procedure.
*/
{

	Instr * i;
	Type * tr, * tl;
	Var * var, * dest_var, * dest_proc;
	Loc loc;
	InferData data;
	UInt32 n;

	ProcInstrEnum(proc, &InstrInitInfer, NULL);

	if (Verbose(proc)) {
		PrintHeader(2, proc->name);
		PrintProc(proc);
	}

	TypeDeduce(proc);

	data.modified = false;
	ProcInstrEnum(proc, &InstrUseDeclaredTypes, &data);

	if (data.modified) {
		TypeDeduce(proc);
	}
	
	// We may deduce some information based on loops

	// Perform final pass, which will print error messages.
	data.final_pass = true;
	ProcInstrEnum(proc, &InstrInferType, &data);
	ProcInstrEnum(proc, &InstrConstraints, &data);

	if (Verbose(proc)) {
		PrintProcFlags(proc, PrintInferredTypes);
	}

	// Extend the type of variables to handle
	// - Check variables, whose types can not be inferred here
	// - Check array indexes (this may already lead to argument inference algorithm)

	loc.proc = proc;
	for(loc.blk = proc->instr; loc.blk != NULL; loc.blk = loc.blk->next) {
		for(n = 1, loc.i = loc.blk->first; loc.i != NULL; loc.i = loc.i->next, n++) {
			i = loc.i;

			if (i->op == INSTR_LINE) continue;

			var = i->result;
			if (var != NULL && !VarIsLabel(var) && !VarIsArrayElement(var) && !VarIsArray(var)) {
				tr = i->type[RESULT];
				if (VarIsLocal(var, proc) && FlagOff(var->submode, SUBMODE_USER_DEFINED)) {
					if (tr != NULL && !(tr->mode == INSTR_TYPE && tr->variant == TYPE_UNDEFINED) && FlagOff(i->flags, InstrRestriction)) {
						var->type = VarUnion(var->type, i->type[RESULT]);
					} else {

						// For temporary variable, there is no reason to define 
						if (VarIsTmp(var)) {
							dest_var = FindNamedDestVar(&loc, var);
							if (dest_var != NULL) {
								ErrArg(dest_var);
								ErrArg(var);
								if (VarIsArg(dest_var)) {
									dest_proc = dest_var->scope;
									ErrArg(dest_proc);
									LogicErrorLoc("Cannot infer type of expression passed to argument [-C] of procedure [A].", &loc);
								}
							} else {
								LogicErrorLoc("Cannot infer type of result of operator [*].", &loc);
							}
						} else {
							ErrArg(var);
							LogicErrorLoc("Cannot infer type of variable [A].\nPlease define the type or use assert to give the compiler some more information.", &loc);
						}
					}
				} else if (tr == NULL) {
					tl = i->type[ARG1];
					if (tl != NULL) {
						if (tl->mode == INSTR_SEQUENCE) {
							//TODO: This test is probably never good
//							ErrArg(var);
//							LogicErrorLoc("Cyclic modification of variable [A].\nVariable will eventually go out of it's defined bounds.\n", &loc);
						}
					}
				}
			}
		}
	}

	if (Verbose(proc)) {
		PrintVars(proc);
	}

	ReportUnusedVars(proc);
	ReportAlwaysFalseAsserts(proc);

}
