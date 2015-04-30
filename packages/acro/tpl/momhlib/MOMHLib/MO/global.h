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

#ifndef __GLOBAL_H
#define __GLOBAL_H

#include <cstdlib>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
#include <ctime>
#include <string>
#include <cstring>

using namespace std;

#include <assert.h>

#include "trandomgenerator.h"

extern int NumberOfObjectives;

/** Possible types of objectives minized or maximized */
enum TObjectiveType {_Min, _Max};

/** Description of an objective */
class TObjective {
public:
	/** Type of the objective minized or maximized */
	TObjectiveType ObjectiveType;

	/** True if the objective is active.
	*
	*	If the objective is not active it is ignored when the
	*	points are compared and when scalarizing function values
	*	are calculated */
	bool bActive;
};

extern vector <TObjective> Objectives;

extern unsigned int NumberOfConstraints;

/** Possible types of constraints*/
enum TConstraintType {_EqualTo, _LowerThan, _GreaterThan};

/** Description of a constraint */
class TConstraint {
public:
	TConstraint () {
		EqualityPrecision = 0;
	}

	/** Type of the objective minized or maximized */
	TConstraintType ConstraintType;

	/** Level to which the constraint should be equal, lower than or 
	*	greater than depending on its type */
	double Threshold;

	/** Precision of equalities
	*
	*	Applies only to constraints of type _EqualTo
	*	Such constraint is feasible if its value is within the range Threshold +/- EqualityPrecision */
	double EqualityPrecision;
};

extern vector <TConstraint> Constraints;

/** Type - vector of integers **/
typedef	vector<int>					IntVector;

/** Type - vector of longs **/
typedef	vector<long>					LongVector;

/** Type - vector of double **/
typedef	vector<double>					DoubleVector;

/** Type - vector of bool **/
typedef	vector<bool>					BoolVector;

/** Type - vector of bytes **/
typedef	vector<unsigned char>			ByteVector;

/** Type - vector of characters **/
typedef	vector<char>					CharVector;

#endif //__GLOBAL_H
