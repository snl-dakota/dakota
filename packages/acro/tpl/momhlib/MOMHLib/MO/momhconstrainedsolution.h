/*
Multiple Objective MetaHeuristics Library in C++ MOMHLib++
Copyright (C) 2001 Andrzej Jaszkiewicz

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation (www.gnu.org); 
either version 2.1 of the License, or (at your option) any later 
version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// momhconstrainedsolution.h: interface for the TMOMHConstrainedSolution class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOMHCONSTRAINEDSOLUTION_H__5C941268_767E_4272_A2DC_09B39E3E5E56__INCLUDED_)
#define AFX_MOMHCONSTRAINEDSOLUTION_H__5C941268_767E_4272_A2DC_09B39E3E5E56__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "momhsolution.h"

/** Base class for solutions of all constrained problems with explicit constraints handling */
class TMOMHConstrainedSolution : public TMOMHSolution  
{
public:
	/** Vector of constraint values */
	vector <double> ConstraintValues;

	/** Copy constructor */
	TMOMHConstrainedSolution (TMOMHConstrainedSolution& Solution) : TMOMHSolution (Solution) {
		ConstraintValues = Solution.ConstraintValues;
	}

	/** Returns true if iConstraint is met in the solution */
	bool ConstraintMet (int iConstraint);

	/** Returns value by wich iConstraint is violated in the solution, 0 if no violation
	*
	*	Note that for constraints of _EqualTo type the value may be greater than 0 but within 
	*	TConstraint::EqualityPrecision and the contsraint is still met */
	double ConstraintViolation (int iConstraint);

	/** Returns true if all constraints are met */
	bool Feasible ();

	/** Returns sum of constraints violations */
	double TotalConstraintsViolation ();

	/** Compares two solution with constraints
	*
	*	Returns _Dominated if this is constrained-dominated
	*	Returns _Dominating if this is constrained-dominating
	*	It may also return constrained-_Nondominated or constrained-_Equal */
	TCompare ConstrainedCompare (TMOMHConstrainedSolution& Solution);

	TMOMHConstrainedSolution () : TMOMHSolution () {
	}

	TMOMHConstrainedSolution (TMOMHConstrainedSolution& Parent1, TMOMHConstrainedSolution& Parent2) : TMOMHSolution (Parent1, Parent2) {
	}
};

/** Base class for solutions of constrained problems with explicit constraints handling as proposed by
*	Deb, Pratap, Agarwal, Meyarivan */
class TDPAMConstrainedSolution : public TMOMHConstrainedSolution {
public:
	/** Compares two solution with constraints
	*
	*	Returns _Dominated if this is constrained-dominated
	*	Returns _Dominating if this is constrained-dominating
	*	It may also return constrained-_Nondominated or constrained-_Equal */
	TCompare ConstrainedCompare (TDPAMConstrainedSolution& Solution);

};

#endif // !defined(AFX_MOMHCONSTRAINEDSOLUTION_H__5C941268_767E_4272_A2DC_09B39E3E5E56__INCLUDED_)
