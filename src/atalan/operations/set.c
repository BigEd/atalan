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

Var * VarUnion(Var * left, Var * right)
{
	Var * result, * rl, * rr;

	if (left == NULL) return right;
	if (right == NULL) return left;
	if (left == right) return left;

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
		result = VarUnion(right, left);
	}

	if (result == NULL) {
		result = NewVariant(left, right);
	}

	return result;
}

//////////////////////////// CellRestrict ////////////////////////////////

Type * CellIntersectionRange(Type * cell, Var * rmin, Var * rmax)
{
	Type * r = cell;
	Var * min, * max;

	if (CellRange(cell, &min, &max)) {
		// <    > cell range
		// (    ) restriction range
		//  ----  resulting cell range

		// 1.  (   <    >     )      => void cell
		if ( IsLowerEq(rmin, min) && IsHigherEq(rmax, max)) {
			r = NULL;
			// 1. <---(    )--->
		} else if (CellIsLower(min, rmin) && CellIsLower(rmax, max)) {
			r = TypeUnion(NewRangeOpenRight(min, rmin), NewRangeOpenLeft(rmax, max));
			// 2. <------>  (     )
		} else if (CellIsLower(max, rmin)) {
			r = cell;
			// 4.  (      )  <----->
		} else if (CellIsLower(rmax,  min)) {
			r = cell;
			// 3.  <---(     >   )
		} else if (CellIsLower(min, rmin)) {
			r = NewRangeOpenRight(min, rmin);
			// 5.  (     <  )----->
		} else if (IsHigherEq(rmax, min)) {
			r = NewRangeOpenLeft(rmax, max);
		}
	} else {
		switch(cell->mode) {
		case INSTR_TUPLE:
			r = TypeUnion(CellIntersectionRange(cell->left, rmin, rmax), CellIntersectionRange(cell->right, rmin, rmax));
			break;
		case INSTR_VAR:
			r = CellIntersectionRange(cell->type, rmin, rmax);
			break;
		case INSTR_TYPE:
			r = CellIntersectionRange(cell->possible_values, rmin, rmax);
			break;
		default:
			TODO("Unsupported cell.");
		}
	}

	return r;
}


Type * TypeRestrictOp(Type * type, Type * restriction, InstrOp op)
{
	Type * rt, * left, * right;
	BigInt min, max, init, step, ib;
	
	// If no restriction is defined, the type is returned unrestricted
	if (restriction == NULL || restriction->variant == TYPE_UNDEFINED) return type;

	rt = type;

	// For variant type, we apply both variants as restrictions and use union of them.
	if (restriction->variant == TYPE_VARIANT) {
		left = TypeRestrictOp(rt, restriction->left, op);
		right = TypeRestrictOp(rt, restriction->right, op);
		rt = TypeUnion(left, right);
		goto done;
	}

	// Compute minimal and maximal value defined by restriction.
	// If the value is not known, extreme value is returned.
	IntSet(&min, TypeMin(restriction));
	IntSet(&max, TypeMax(restriction));

	if (type != NULL) {
		if (type->mode == INSTR_SEQUENCE) {
			switch(op) {
			case INSTR_NE:
				if (type->seq.compare_op != op) {
					type->seq.compare_op = op;
					type->seq.limit = NewRangeInt(&max, &max);
					type = ResolveSequence(type);			// type =
				}
				break;
			case INSTR_LT:
				max--;
				//continue to IFLE
			case INSTR_LE:
				if (TypeIsInt(type->seq.step) && TypeIsInt(type->seq.init)) {
					init = type->seq.init->range.min;
					step = type->seq.step->range.max;			// maximal step
					if (type->seq.op == INSTR_ADD) {
						IntAdd(&ib, &max, &step);
						rt = NewRangeInt(&init, &ib);	// we may overstep maximal value by step
						IntFree(&ib);
					}
				} else {
					if (type->seq.compare_op != op) {
						type->seq.compare_op = op;
						type->seq.limit = NewRangeInt(&max, &max);
					}
				}
				break;
			case INSTR_GT:
				break;

			case INSTR_GE:
				break;

			case INSTR_MATCH_TYPE:
			case INSTR_NMATCH_TYPE:
				break;
			default:
				ASSERT(false);	// unknown operator
			}

		} else if (type->variant == TYPE_INT) {

			rt = NULL;
			switch(op) {

			case INSTR_NE:
				rt = CellIntersection(type, restriction);
				break;

			// For Eq, resulting type is the range of source variable
			case INSTR_EQ:
				rt = CellIntersection(restriction, type);
				break;

			case INSTR_LE: IntAdd(&min, IntMax(&min, &max), Int1()); IntSetMax(&max); break;	// remove anything bigger than 
			case INSTR_LT: IntSet(&min, IntMax(&min, &max)); IntSetMax(&max); break;
			case INSTR_GE: IntSetMin(&min); IntSub(&max, IntMin(&min, &max), Int1()); break;
			case INSTR_GT: IntSetMin(&min); IntSet(&max, IntMin(&min, &max)); break;

			default: ;
			}

			if (rt == NULL) {
				rt = CellIntersectionRange(type, IntCell(&min), IntCell(&max));
			}
		}
		IntFree(&min); IntFree(&max);
	}
done:
	return rt;
}


Type * CellIntersection(Type * cell, Type * restriction)
{
	Type * r = cell;
	Var * rmin, * rmax;

	if (restriction == NULL || restriction->variant == TYPE_UNDEFINED) return r;

	if (CellRange(restriction, &rmin, &rmax)) {
		r = CellIntersectionRange(cell, rmin, rmax);
	} else {
		switch(restriction->mode) {
		case INSTR_TUPLE:
			r = TypeUnion(CellIntersection(cell, restriction->left), CellIntersection(cell, restriction->right));
			break;
		case INSTR_TYPE:
			r = CellIntersection(cell, restriction->possible_values);
			break;
		case INSTR_VAR:
			r = CellIntersection(cell, restriction->type);
			break;
		default:
			TODO("Unsupported restriction");
			break;
		}
	}

	return r;
}

/////////////////////////////// Union ////////////////////////////////

Type * TypeUnion(Type * left, Type * right)
/*
Purpose:
	Combine types so, that resulting type may contain values from left or from the right type.
	This may be used when a variable is initialized in multiple code branches.
*/
{
	Type * type = NULL, * t;
	BigInt * min, * max;

	if (left == right) return left;

	// Make sure, that if there is NON-NULL type, it is the left one
	if (left == NULL) {
		left = right;
		right = NULL;
	}

	// In case there is only one type or no type, return the one type or NULL
	if (right == NULL) return left;

	// When making union with empty value, return the nonempty one)

	if (left->mode == INSTR_EMPTY) return right;
	if (right->mode == INSTR_EMPTY) return left;


	// If any of the types is undefined, result is undefined
	if (left->variant == TYPE_UNDEFINED) return left;
	if (right->variant == TYPE_UNDEFINED) return right;

	// If one of the types is sequence, make sure it is the left one
	if (right->mode == INSTR_SEQUENCE) {
		t = left;
		left = right;
		right = t;
	}

	if (left->mode == INSTR_SEQUENCE) {

		// Sequence & sequence
		if (right->mode == INSTR_SEQUENCE) {

			// same operator
			if (right->seq.op == left->seq.op) {
				return NewSequence(
					TypeUnion(left->seq.init, right->seq.init),
					TypeUnion(left->seq.step, right->seq.step),
					right->seq.op,
					NULL,
					INSTR_NULL
					);
			}

		// Step is on the left side

		} else if (right->variant == TYPE_INT) {
			if (left->seq.init == NULL) {
				type = TypeCopy(left);
				type->seq.init = right;
				return type;
			}

			type = ResolveSequence(left);
			if (type != NULL) {
				return TypeUnion(type, right);
			}
/*
			if (left->seq.op == INSTR_ADD && left->seq.step->range.min >=0) {
				type = TypeAlloc(TYPE_INT);
				type->range.max = INT_MAX;
				type->range.min = right->range.min;
				return type;
			}
*/
		}
	}

	if (right == NULL) return right;

	switch (left->variant) {
	case TYPE_INT:
		if (right->variant == TYPE_INT) {
			if (IntHigherEq(&left->range.max, &right->range.min) || IntHigherEq(&right->range.max, &left->range.min)) {
				min = IntMin(&left->range.min, &right->range.min);
				max = IntMax(&left->range.max, &right->range.max); //&right->range.max; if ( left->range.max > max) max =  left->range.max;
				if (IntEq(&left->range.min, min) && IntEq(&left->range.max, max)) {
					type = left;
				} else if (IntEq(&left->range.min, min) && IntEq(&left->range.max, max)) {
					type = right;
				} else {
					type = NewRangeInt(min, max);
				}
			}
		}
		break;
	case TYPE_VARIANT:
		if (IsSubset(right, left->left) || IsSubset(right, left->right)) return left;
		break;
	default:
		break;
	}

	// Default case is VARIANT type
	if (type == NULL) {
		type = TypeAlloc(TYPE_VARIANT);
		type->left  = left;
		type->right = right;
	}
	return type;
}

/******************************************************************************************

IsSubset

*******************************************************************************************/
