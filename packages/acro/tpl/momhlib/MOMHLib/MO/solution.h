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

#ifndef __SOLUTION_H
#define __SOLUTION_H

#include "global.h"
#include "weights.h"

/** Possible relations between points in multiple objective space */
enum TCompare {_Dominating, _Dominated, _Nondominated, _Equal};  

/** Point in objective space */
class TPoint {
public:
	/** Vector of objective values */
	vector<double> ObjectiveValues;

	/** Shifts the point in dialgonal direction of the
	*	given weight vector
	*
	*	Used to shift the reference point used in Chebycheff 
	*	scalarizing function 
	*/
	void Augment (TPoint& ApproximateIdealPoint, TPoint ApproximateNadirPoint);

	/** Constructor */
	TPoint () {
		if (NumberOfObjectives == 0) {
			cout << "TPoint::TPoint ()\n";
			cout << "NumberOfObjectives == 0\n";
			exit (1);
		}
		ObjectiveValues.resize (NumberOfObjectives);
	}

	/** Copy constructor */
	TPoint (TPoint& Point) {
		ObjectiveValues.resize (NumberOfObjectives);
		int i; for (i = 0; i < NumberOfObjectives; i++)
			ObjectiveValues [i] = Point.ObjectiveValues [i];
	}

	/** Copy operator */
	TPoint& operator = (TPoint& Point);

	/** Compares two points in multiple objective space
	*
	*	Returns _Dominated if this is dominated
	*	Returns _Dominating if this is dominating
	*	It may also return _Nondominated or _Equal */
	TCompare Compare(TPoint& Point);

	/**	Returns value of the linear scalarizing function (weighted sum) 
	*	defined by the given weight vector and reference point */
	double LinearScalarizingFunction (TWeightVector& WeightVector,
		TPoint& ReferencePoint);

	/**	Returns value of the Chebycheff scalarizing function 
	*	defined by the given weight vector and reference point */
	double ChebycheffScalarizingFunction (TWeightVector& WeightVector,
		TPoint& ReferencePoint);

	/** Calculates Euclidean distance between two points in the
	*	normalized objective space.
	*
	*	The objectives are normalized with the ranges between the 
	*	given ideal and nadir points. */
	double Distance (TPoint& Point, TPoint& IdealPoint, TPoint& NadirPoint);

	/** Reads the point from the stream */
    virtual istream& Load(istream& Stream);

	/** Saves objective values to an open Stream
	*
	*	Values are separated by TAB character */
    virtual ostream& Save(ostream& Stream);             
};

/** Possible types of scalarizing functions */
enum TScalarizingFunctionType {_Linear, _Chebycheff};

/** Abstract class - generalization of solutions of various problems */
class TSolution : public TPoint
{
protected:
	/** Current type of scalarizing function used by the solution */
	TScalarizingFunctionType ScalarizingFunctionType;

	/** Vector of old objective values */
	vector<double> OldObjectiveValues;

	/** Sets current weight vector */
	void SetWeightVector (TWeightVector WeightVector) {
		this->WeightVector = WeightVector;
	}

public:
	/** Current weight vector used by the solution */
	TWeightVector WeightVector;

	/** Sets current type of scalarizing function */
	void SetScalarizingFunctionType (TScalarizingFunctionType ScalarizingFunctionType) {
		this->ScalarizingFunctionType = ScalarizingFunctionType;
	}

	/** Value of the scalarizing value stored to speed up the claculations */
	double ScalarizingFunctionValue;

	/** Saves ObjectiveValues in OldObjectiveValues */
	void RestoreObjectiveValues();

	/** Restores ObjectiveValues from OldObjectiveValues */
	void SaveObjectiveValues();

	/** Empty constructor */
    TSolution() : TPoint () {
		ScalarizingFunctionType = _Linear;//_Chebycheff;
		OldObjectiveValues.resize (NumberOfObjectives);
	}

	/** Copy constructor */
    TSolution(TSolution& Solution) : TPoint (Solution) {
		ScalarizingFunctionType = Solution.ScalarizingFunctionType;
		WeightVector = Solution.WeightVector;
		OldObjectiveValues.resize (NumberOfObjectives);
	}

    virtual ~TSolution (){
	}

	/** Returns value of the scalarizing function calculated for the solution
	*
	*	The scalarizing function is defined by 
	*	this->WeightVector and this->ScalarizingFunctionType */
	double ScalarizingFunction (TPoint& ReferencePoint);

	/** Returns value of the scalarizing function calculated for the solution
	*
	*	The scalarizing function is defined by 
	*	WeightVector and this->ScalarizingFunctionType */
	double ScalarizingFunction (TPoint& ReferencePoint, TWeightVector& WeightVector);

	/**
	 * Return compare mask for objectives of all solutions
	 *
	 * @author Radoslaw Ziembinski
	 **/
	bool CompareTo(TSolution &oSolution, vector<bool> &bResult);
};
#endif
