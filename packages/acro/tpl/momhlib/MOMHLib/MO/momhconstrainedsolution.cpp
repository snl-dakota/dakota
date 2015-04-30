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

// momhconstrainedsolution.cpp: implementation of the TMOMHConstrainedSolution class.
//
//////////////////////////////////////////////////////////////////////

#include "momhconstrainedsolution.h"

bool TMOMHConstrainedSolution::ConstraintMet (int iConstraint) {
	switch (Constraints [iConstraint].ConstraintType) {
	case _EqualTo :
			   return fabs (ConstraintValues [iConstraint] - Constraints [iConstraint].Threshold) <
				   Constraints [iConstraint].EqualityPrecision;
	case _LowerThan :
				return ConstraintValues [iConstraint] <= Constraints [iConstraint].Threshold;
	case _GreaterThan :
				return ConstraintValues [iConstraint] >= Constraints [iConstraint].Threshold;
	default :
		cout << "bool TMOMHConstrainedSolution::ConstraintMet (int iConstraint)\n";
		cout << "Unknown ConstraintType\n";
		exit (0);
	}
	return true;
}

double TMOMHConstrainedSolution::ConstraintViolation (int iConstraint) {
	switch (Constraints [iConstraint].ConstraintType) {
	case _EqualTo :
				return fabs (ConstraintValues [iConstraint] - Constraints [iConstraint].Threshold);
	case _LowerThan :
				if (ConstraintValues [iConstraint] <= Constraints [iConstraint].Threshold)
					return 0;
				else
					return ConstraintValues [iConstraint] - Constraints [iConstraint].Threshold;
	case _GreaterThan :
				if (ConstraintValues [iConstraint] >= Constraints [iConstraint].Threshold)
					return 0;
				else
					return Constraints [iConstraint].Threshold - ConstraintValues [iConstraint];
	default :
		cout << "bool TMOMHConstrainedSolution::ConstraintViolation (int iConstraint)\n";
		cout << "Unknown ConstraintType\n";
		exit (0);
	}
	return 0;
}

bool TMOMHConstrainedSolution::Feasible () {
	bool bFeasible = true;
	unsigned int iConstraint;
	for (iConstraint = 0; (iConstraint < ::NumberOfConstraints) && bFeasible; iConstraint++)
		bFeasible = ConstraintMet (iConstraint);
	return bFeasible;
}

double TMOMHConstrainedSolution::TotalConstraintsViolation () {
	double TotalViolation = 0;
	unsigned int iConstraint;
	for (iConstraint = 0; iConstraint < ::NumberOfConstraints; iConstraint++)
		TotalViolation += ConstraintViolation (iConstraint);
	return TotalViolation;
}

TCompare TDPAMConstrainedSolution::ConstrainedCompare (TDPAMConstrainedSolution& Solution) {
	if (Feasible () && !Solution.Feasible ())
		return _Dominating;
	else if (!Feasible () && Solution.Feasible ())
		return _Dominated;
	else if (!Feasible () && !Solution.Feasible ()) {
		if (TotalConstraintsViolation () < Solution.TotalConstraintsViolation ())
			return _Dominating;
		else if (TotalConstraintsViolation () > Solution.TotalConstraintsViolation ())
			return _Dominated;
		else // TotalConstraintsViolation () == Solution.TotalConstraintsViolation ()
			return Compare (Solution);
	}
	else // Feasible () && Solution.Feasible ())
		return Compare (Solution);
}

