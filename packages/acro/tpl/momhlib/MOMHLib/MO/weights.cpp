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

#include "weights.h"
#include "solution.h"

double TWeightVector::Distance (const TWeightVector& WeightVector) {
	double s = 0;
	int i; for (i = 0; i < NumberOfObjectives; i++) {
		double s1 = (*this) [i] - WeightVector [i];
		s += s1 * s1;
	}
	return sqrt (s);
}

TWeightVector& TWeightVector::operator = (const TWeightVector& SourceWeights) {
	clear ();
	int i; for (i = 0; i < NumberOfObjectives; i++) {
		double d = SourceWeights [i];
		push_back (SourceWeights [i]);
	}

	return *this;
}

void TWeightsSet::GenerateUniformCover (int Steps) {
TWeightVector WeightVector;

	// Find the number of active objective
	int NumberOfActiveObjectives = 0;
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++)
		if (Objectives [iobj].bActive)
			NumberOfActiveObjectives++;

	if (NumberOfActiveObjectives == 0)
		return;

	// Prepare IntVector vector
//	vector<int> IntVector;
//	IntVector.resize (NumberOfActiveObjectives);
	int* IntVector = new int [NumberOfActiveObjectives];
	int i; for (i = 0; i < NumberOfActiveObjectives; i++)
		IntVector [i] = -1;

	// Generate all vectors with elements 0,1..Steps
	int level = 0;
	while (level >= 0) {
		if (IntVector [level] <= Steps - 1) {
			int i; for (i = level + 1; i < NumberOfActiveObjectives; i++)
				IntVector [i] = -1;
			IntVector [level]++;
			if (level < NumberOfActiveObjectives - 1)
				level++;
			else
			{
				int Sum = 0;
				int i; for (i = 0; i < NumberOfActiveObjectives; i++)
					Sum += IntVector [i];
				// If sum of elements in the IntVector evctor is equal to Steps
				if (Sum == Steps) {
					int iActiveObj = 0;
					int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++)
						if (Objectives [iobj].bActive) {
							WeightVector[iobj] = (double)IntVector [iActiveObj] / (double)Steps;
							iActiveObj++;
						}
						else
							WeightVector[iobj] = -1;
					push_back (WeightVector);
					UniformWeightVectors.push_back (WeightVector);
				}
			}
		}
		else
			level--;
	}

	delete [] IntVector;

}

void TWeightVector::Normalize () {
	double Sum = 0;
	int i; for (i = 0; i < NumberOfObjectives; i++)
		Sum += (*this)[i];
	for (i = 0; i < NumberOfObjectives; i++)
		(*this)[i] /= Sum;
}

void TWeightVector::Rescale(TPoint& IdealPoint, TPoint& NadirPoint)
{
	int i; for (i = 0; i < NumberOfObjectives; i++) {
		if (fabs (IdealPoint.ObjectiveValues [i] - NadirPoint.ObjectiveValues [i]) > 0)
			(*this)[i] /= fabs (IdealPoint.ObjectiveValues [i] - NadirPoint.ObjectiveValues [i]);
	}
}

void TWeightsSet::NormalizeForEvaluation()
{
	unsigned int i; for (i = 0; i < size (); i++) {
		TWeightVector* pWeightVector = &UniformWeightVectors [i];
		double MaxWeight = 0;
		int j; for (j = 0; j < NumberOfObjectives; j++) {
			if (MaxWeight < (*pWeightVector) [j])
				MaxWeight = (*pWeightVector) [j];
		}
		for (j = 0; j < NumberOfObjectives; j++) {
			(*pWeightVector) [j] /= MaxWeight;
		}
	}

}

void TWeightsSet::Rescale (TPoint& IdealPoint, TPoint& NadirPoint) {
	unsigned int i; for (i = 0; i < size (); i++) {
		(*this) [i] = UniformWeightVectors [i];
		(*this) [i].Rescale (IdealPoint, NadirPoint);
	}
}

TWeightVector GetRandomWeightVector () {
	TWeightVector WeightVector;

	double WeightConstraint = 0;//1.0 / NumberOfObjectives * (3.0 / 4);

	bool bFeasible;
	do {
		double WeightsSum = 0;
		int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
			double R = (double)rand () / (double)RAND_MAX;
			if (iobj < NumberOfObjectives - 1) {
				WeightVector [iobj] = (1 - WeightsSum) * (1 - pow (R, 1 / ((double)NumberOfObjectives - iobj - 1)));
			}
			else
				WeightVector [iobj] = (1 - WeightsSum);
			WeightsSum += WeightVector [iobj];
		}
		bFeasible = true;
		for (iobj = 0; bFeasible && (iobj < NumberOfObjectives); iobj++) 
			bFeasible = WeightVector [iobj] >= WeightConstraint;
	} while (!bFeasible);

	return WeightVector;
}
