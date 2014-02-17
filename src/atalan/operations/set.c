/*

Cell set operations

(c) 2013 Rudolf Kudla 
Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php


*/

#include "../language.h"

Var * VarUnionRange(Var * left, Var * right)
/*
Purpose:
	Create union of two variables, if the result will be one range (or const) variable.
	In some cases, left or right may be returned.
	If this is not possible (INSTR_VARIANT union would be necessary), return NULL.
*/
{
	BigInt * l_min, * l_max;
	BigInt * r_min, * r_max;
	BigInt * min, * max;
	BigInt diff1, diff2;
	Var * result;

	if (left == right) return left;
	if (left == NULL) return right;
	if (right == NULL) return left;

	result = NULL;

	l_min = NULL; l_max = NULL;
	r_min = NULL; r_max = NULL;

	VarRange(left, &l_min, &l_max);
	VarRange(right, &r_min, &r_max);

	if (l_min != NULL && l_max != NULL && r_min != NULL && r_max != NULL) {

		IntSub(&diff1, r_min, l_max);
		IntSub(&diff2, l_min, r_max);

		// If the right min is higher than left max, there is a gap and we cannot create single range
		if (IntHigher(&diff1, Int1()) || IntHigher(&diff2, Int1())) {
			// this is two intervals with no intersection
		} else {
			min = IntMin(l_min, r_min);
			max = IntMax(l_max, r_max);
			result = NewRange(IntCell(min), IntCell(max));
		}
	}
	return result;
}

Var * VarUnion2(Var * left, Var * right, Var ** p_merged)
/*
Purpose:
	Find union of specified variable and some item of the variable.
	If such union does not exist (there is no intersection between an existing variable item and the new item or
	exactly same item exists), null is returned.
*/
{
	Var * un = NULL;
	Var * un2;

	if (left == NULL) {
//		*p_merged = right;
		return right;
	}
	if (left == right) {
		return left;
	}

	if (left->mode == INSTR_VARIANT) {
		un = VarUnion2(left->adr, right, p_merged);
		if (un == left->adr) un = left; 	//TODO: Test the other half here
//		if (un == right) return right;
		if (un != NULL && un != left) {
			un = NewVariant(un, left->var);
		} else {
			un2 = VarUnion2(left->var, right, p_merged);
			if (un2 == left->var) return left;
			if (un2 == right) return right;
			if (un2 != NULL) {
				un = NewVariant(left->adr, un2);
			}
		}
	} else if (left->mode == INSTR_INT || left->mode == INSTR_RANGE) {
		un = VarUnionRange(left, right);
		if (un != NULL && un != left) {
			*p_merged = un;
		}
	}
	return un;
}

Var * VarFindUnionRange(Var * var, Var * item)
/*
Purpose:
	Find union of specified variable and some item of the variable.
	If such union does not exist (there is no intersection between an existing variable item and the new item or
	exactly same item exists), null is returned.
*/
{
	Var * un = NULL;

	if (var == NULL) return NULL;
	if (item == NULL) return NULL;
	if (var == item) return NULL;

	if (var->mode == INSTR_VARIANT) {
		un = VarFindUnionRange(var->adr, item);
		if (un == NULL) un = VarFindUnionRange(var->var, item);
	} else if (var->mode == INSTR_INT || var->mode == INSTR_RANGE) {
		un = VarUnionRange(var, item);
	}
	return un;
}

Var * VarRemoveContainedItems(Var * var, Var * item)
/*
Purpose:
	Remove all parts of list (variant) that are fully enclosed in the specified item.
	Does not remove the item itself.
*/
{
	Var * nv1, *nv2;
	if (item == NULL || var == item) return var;
	if (var->mode == INSTR_VARIANT) {
		nv1 = VarRemoveContainedItems(var->adr, item);
		nv2 = VarRemoveContainedItems(var->var, item);
		if (nv1 == NULL) return nv2;
		if (nv2 == NULL) return nv1;
		if (nv1 != var->adr || nv2 != var->var) {
			return NewVariant(nv1, nv2);
		}
	} else if (var->mode == INSTR_INT || var->mode == INSTR_RANGE) {
		nv1 = VarUnionRange(var, item);
		if (nv1 == item) return NULL;
	}
	return var;
}

Var * Union(Var * left, Var * right)
{
	Var * result, * rl, * rr, * t;

	if (left == NULL) return right;
	if (right == NULL) return left;
	if (left == right) return left;

	if (left->mode == INSTR_EMPTY) return right;
	if (right->mode == INSTR_EMPTY) return left;

	// If one of the types is sequence, make sure it is the left one
	if (right->mode == INSTR_SEQUENCE) {
		t = left;
		left = right;
		right = t;
	}

	// Sequence union
	if (left->mode == INSTR_SEQUENCE) {

		// Sequence & sequence
		if (right->mode == INSTR_SEQUENCE) {

			// same operator
			if (right->seq.op == left->seq.op) {
				return NewSequence(
					Union(left->seq.init, right->seq.init),
					Union(left->seq.step, right->seq.step),
					right->seq.op,
					NULL,
					INSTR_NULL
					);
			}

			// Step is on the left side

		} else if (CellIsIntConst(right)) {
			if (left->seq.init == NULL) {
				return NewSequence(right, left->seq.step, left->seq.op, left->seq.limit, left->seq.compare_op);
			} else {
				rl = ResolveSequence(left);
				if (rl != NULL) {
					return Union(rl, right);
				}
			}
		}
	}

	result = NULL;

	if (right->mode == INSTR_INT || right->mode == INSTR_RANGE) {
		rr = right;
		do {
			left = VarRemoveContainedItems(left, rr);
			rl = rr;
			rr = VarFindUnionRange(left, rr);
		} while(rr != NULL);

		right = rl;
		if (left == NULL) return right;

	} else if (right->mode == INSTR_VARIANT) {
		result = Union(right, left);
	}

	if (result == NULL) {
		result = NewVariant(left, right);
	}

	return result;
}

Type * RemoveRange(Type * cell, Var * rmin, Var * rmax)
{
	Type * r = cell;
	Var * min, * max;

	if (cell->mode == INSTR_SEQUENCE) {
		TODO("Restrict range");
	}

	if (CellRange(cell, &min, &max)) {

		if (rmin == NULL) rmin = min;		//TODO: Maybe we may use MAX & MIN
		if (rmax == NULL) rmax = max;

		// <    > cell range
		// (    ) restriction range
		//  ----  resulting cell range

		// 1.  (   <    >     )      => void cell
		if ( IsLowerEq(rmin, min) && IsHigherEq(rmax, max)) {
			r = NULL;
			// 1. <---(    )--->
		} else if (IsLower(min, rmin) && IsLower(rmax, max)) {
			r = Union(NewRangeOpenRight(min, rmin), NewRangeOpenLeft(rmax, max));
			// 2. <------>  (     )
		} else if (IsLower(max, rmin)) {
			r = cell;
			// 4.  (      )  <----->
		} else if (IsLower(rmax,  min)) {
			r = cell;
			// 3.  <---(     >   )
		} else if (IsLower(min, rmin)) {
			r = NewRangeOpenRight(min, rmin);
			// 5.  (     <  )----->
		} else if (IsHigherEq(rmax, min)) {
			r = NewRangeOpenLeft(rmax, max);
		}
	} else {
		switch(cell->mode) {
		case INSTR_TUPLE:
			r = Union(RemoveRange(cell->left, rmin, rmax), RemoveRange(cell->right, rmin, rmax));
			break;
		case INSTR_VAR:
			r = RemoveRange(cell->type, rmin, rmax);
			break;
		case INSTR_TYPE:
			r = RemoveRange(cell->possible_values, rmin, rmax);
			break;
		default:
			TODO("Unsupported cell.");
		}
	}

	return r;
}


Type * Restrict(Type * type, Type * restriction, InstrOp op)
{
	Type * rt, * left, * right;
//	BigInt init, step;
	Var * min, * max, * slimit;
	InstrOp sop;

	// If no restriction is defined, the type is returned unrestricted
	if (restriction == NULL || restriction->mode == INSTR_EMPTY) return type;

	rt = type;

	// For variant type, we apply both variants as restrictions and use union of them.
	if (restriction->mode == INSTR_VARIANT) {
		left = Restrict(type, restriction->l, op);
		right = Restrict(type, restriction->r, op);
		rt = Union(left, right);
		goto done;
	}

	// Compute minimal and maximal value defined by restriction.
	// If the value is not known, extreme value is returned.

	if (type->mode == INSTR_SEQUENCE) {
		sop = type->seq.compare_op;
		slimit = type->seq.limit;
		if (op != INSTR_MATCH) {
			if (sop == INSTR_VOID) {
				rt = NewSequence(type->seq.init, type->seq.step, type->seq.op, restriction, op);
				return rt;
			// Restriction is the same as current sequence restriction
			} else if (sop == op && IsEqual(restriction, type->seq.limit)) {
				return type;
			}
		}
	}

	switch(op) {
	case INSTR_LE:
		if (type->mode == INSTR_SEQUENCE) {
			if (type->seq.compare_op == INSTR_LT) {
				if (IsLower(restriction, type->seq.limit)) {
					rt = NewSequence(type->seq.init, type->seq.step, type->seq.op, restriction, op);
				}
			} else {
				ASSERT("Unexpected");
			}
		} else {
			max = CellMax(restriction);
			if (max != NULL) {
				rt = RemoveRange(type, Add(max, ONE), NULL);
			}
		}
		break;

	case INSTR_LT:
		if (type->mode == INSTR_SEQUENCE) {
			if (type->seq.compare_op == INSTR_LT) {
				if (IsLower(restriction, type->seq.limit)) {
					rt = NewSequence(type->seq.init, type->seq.step, type->seq.op, restriction, op);
				}
			} else {
				ASSERT("Unexpected");
			}
		} else {
			max = CellMax(restriction);
			if (max != NULL) {
				rt = RemoveRange(type, max, NULL);
			}
		}
		break;
	case INSTR_GE:
		min = CellMin(restriction);
		if (min != NULL) {
			rt = RemoveRange(type, NULL, Sub(min, ONE));
		}
		break;
	case INSTR_GT:
		min = CellMin(restriction);
		if (min != NULL) {
			rt = RemoveRange(type, NULL, min);
		}
		break;

	case INSTR_NE:
		if (type->mode == INSTR_SEQUENCE) {
			if (type->seq.compare_op == INSTR_NE) {
				if (IsEqual(restriction, type->seq.limit)) {
					rt = type;
				} else if (type->seq.op == INSTR_ADD && IsEqual(type->seq.step, ONE) && IsLower(restriction, type->seq.limit)) {
					rt = NewSequence(type->seq.init, type->seq.step, type->seq.op, restriction, op);
				}
			} else {
				ASSERT("Unexpected");
			}
		} else {
			rt = Remove(type, restriction);
		}
		break;

	case INSTR_EQ:
		rt = Remove(restriction, type);
		break;
	}

done:
	return rt;
}


//TODO: Should be named SetDifference

Type * Remove(Type * cell, Type * restriction)
{
	Type * r = cell;
	Var * rmin, * rmax;

	if (restriction == NULL || restriction->mode == INSTR_EMPTY) return r;

	if (CellRange(restriction, &rmin, &rmax)) {
		r = RemoveRange(cell, rmin, rmax);
	} else {
		switch(restriction->mode) {
		case INSTR_TUPLE:
			r = Union(Remove(cell, restriction->left), Remove(cell, restriction->right));
			break;
		case INSTR_TYPE:
			r = Remove(cell, restriction->possible_values);
			break;
		case INSTR_VAR:
			r = Remove(cell, restriction->type);
			break;
		default:
			TODO("Unsupported restriction");
			break;
		}
	}

	return r;
}
