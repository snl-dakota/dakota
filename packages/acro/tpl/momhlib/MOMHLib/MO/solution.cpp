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

#include "solution.h"

ostream& TPoint::Save(ostream& Stream)  
{
int i;
  for (i = 0; i < NumberOfObjectives; i++) {
	 Stream << ObjectiveValues [i];
	 Stream << '\x09';
  }
  Stream << '\x09';
  return Stream;
}

TPoint& TPoint::operator = (TPoint& Point) {
	ObjectiveValues.resize (NumberOfObjectives);
	int i; for (i = 0; i < NumberOfObjectives; i++)
		ObjectiveValues [i] = Point.ObjectiveValues [i];
	return *this;
}

istream& TPoint::Load(istream& Stream) {
	int i; for (i = 0; i < NumberOfObjectives; i++) {
		Stream >> ObjectiveValues [i];
	}
	return Stream;
}

double TPoint::LinearScalarizingFunction (TWeightVector& WeightVector,
										  TPoint& ReferencePoint) {
	double Sum = 0;
	int i; for (i = 0; i < NumberOfObjectives; i++)
		if (Objectives [i].bActive) {
			if (Objectives [i].ObjectiveType == _Max)
				Sum += WeightVector [i] * (ReferencePoint.ObjectiveValues [i] - ObjectiveValues [i]);
			else
				Sum -= WeightVector [i] * (ReferencePoint.ObjectiveValues [i] - ObjectiveValues [i]);
		}
	return Sum;
}

double TPoint::ChebycheffScalarizingFunction (TWeightVector& WeightVector,
											  TPoint& ReferencePoint) {
	double Max = -1e30;
	int i; for (i = 0; i < NumberOfObjectives; i++) {
		if (Objectives [i].bActive) {
			double s;
			double w = WeightVector [i];
			if (Objectives [i].ObjectiveType == _Max)
				s = WeightVector [i] * (ReferencePoint.ObjectiveValues [i] - ObjectiveValues [i]);
			else
				s = -WeightVector [i] * (ReferencePoint.ObjectiveValues [i] - ObjectiveValues [i]);
			if (s > Max)
				Max = s;
		}
	}

	return Max/*+ 0.001 * LinearScalarizingFunction (WeightVector, ReferencePoint)*/;
}

double TPoint::Distance (TPoint& Point, TPoint& IdealPoint, TPoint& NadirPoint) {
	double s = 0;
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		double Range = IdealPoint.ObjectiveValues [iobj] - NadirPoint.ObjectiveValues [iobj];
		if (Range == 0)
			Range = 1;
		double s1 = (ObjectiveValues [iobj] - Point.ObjectiveValues [iobj]);
		s += s1 * s1;
	}
	return sqrt (s);
}

TCompare TPoint::Compare(TPoint& Point) {
	bool bBetter = false;
	bool bWorse = false;

	int i = 0;
	do {
		if (Objectives [i].bActive) {
			if (Objectives [i].ObjectiveType == _Max) {
				if (ObjectiveValues [i] > Point.ObjectiveValues [i])
					bBetter = true;
				if (Point.ObjectiveValues [i] > ObjectiveValues [i])
					bWorse = true;
			}
			else {
				if (ObjectiveValues [i] < Point.ObjectiveValues [i])
					bBetter = true;
				if (Point.ObjectiveValues [i] < ObjectiveValues [i])
					bWorse = true;
			}
		}
		i++;
	}
	while (!(bWorse && bBetter) && (i < NumberOfObjectives));
	if (bWorse) {
		if (bBetter)
			return _Nondominated;
		else
			return _Dominated;
	}
	else {
		if (bBetter)
			return _Dominating;
		else
			return _Equal;
	}
}

double TSolution::ScalarizingFunction (TPoint& ReferencePoint, TWeightVector& WeightVector) {
	switch (ScalarizingFunctionType) {
	case _Linear :
			   return LinearScalarizingFunction (WeightVector, ReferencePoint);
	case _Chebycheff :
				return ChebycheffScalarizingFunction (WeightVector, ReferencePoint);
	default :
		cout << "double TSolution::ScalarizingFunction (TPoint& ReferencePoint)\n";
		cout << "Unknown ScalarizingFunctionType\n";
		exit (0);
	}
	return 0;
}

double TSolution::ScalarizingFunction (TPoint& ReferencePoint) {
	switch (ScalarizingFunctionType) {
	case _Linear :
			   return LinearScalarizingFunction (WeightVector, ReferencePoint);
	case _Chebycheff :
				return ChebycheffScalarizingFunction (WeightVector, ReferencePoint);
	default :
		cout << "double TSolution::ScalarizingFunction (TPoint& ReferencePoint)\n";
		cout << "Unknown ScalarizingFunctionType\n";
		exit (0);
	}
	return 0;
}

void TSolution::SaveObjectiveValues()
{
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		if (Objectives [iobj].bActive)
			OldObjectiveValues [iobj] = ObjectiveValues [iobj];
	}
}

void TSolution::RestoreObjectiveValues()
{
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		if (Objectives [iobj].bActive)
			ObjectiveValues [iobj] = OldObjectiveValues [iobj];
	}
}

void TPoint::Augment (TPoint& ApproximateIdealPoint, TPoint ApproximateNadirPoint) {
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		ObjectiveValues [iobj] += (ApproximateIdealPoint.ObjectiveValues [iobj] -
			ApproximateNadirPoint.ObjectiveValues [iobj]) * 0.1;
	}
}

bool TSolution::CompareTo(TSolution &oSolution, vector<bool> &bResult)
{
int		i = 0;

	if ((ObjectiveValues.size() != oSolution.ObjectiveValues.size()) ||
	    (ObjectiveValues.size() != NumberOfObjectives) ||
		(oSolution.ObjectiveValues.size() != NumberOfObjectives)) {

		return false;
	}

	bResult.resize(NumberOfObjectives);

	for(i = 0; i < NumberOfObjectives; i++)
	{
		if (Objectives[i].ObjectiveType == _Max) {
			if (ObjectiveValues[i] > oSolution.ObjectiveValues[i]) {
				bResult[i] = true;
			} else {
				bResult[i] = false;
			}

			continue;
		}
		
		if (Objectives[i].ObjectiveType == _Min) {
			if (ObjectiveValues[i] > oSolution.ObjectiveValues[i]) {
				bResult[i] = false;
			} else {
				bResult[i] = true;
			}
			
			continue;
		}
	}

	return true;
}
