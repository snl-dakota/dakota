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

#if !defined(AFX_MOMETHOD_H__46C81620_9636_11D4_829A_000000000000__INCLUDED_CPP)
#define AFX_MOMETHOD_H__46C81620_9636_11D4_829A_000000000000__INCLUDED_CPP

#include "momethod.h"
#include "nondominatedset.h"
#include "tlistset.h"
#include "tquadtree.h"

template <class TProblemSolution>
bool TMOMethod <TProblemSolution>::InitializeNondominatedSet(int iType)
{
    if (pNondominatedSet != NULL)
    {
        delete pNondominatedSet;
        pNondominatedSet = NULL;
    }

	switch(iType) 
    {
		case MO_NDS_LIST:
			pNondominatedSet = new TListSet <TProblemSolution>;
			iTypeOfNondominatedSet = MO_NDS_LIST;
			break;

		case MO_NDS_QUAD_TREE:
			pNondominatedSet = new TQuadTree <TProblemSolution>;
			iTypeOfNondominatedSet = MO_NDS_QUAD_TREE;
			break;

		default:;
			return false;
	}

	return true;
}


template <class TProblemSolution>
void TMOMethod <TProblemSolution>::RunMOSALike (TScalarizingFunctionType ScalarizingFunctionType,
												int WeightSpaceSamplingParameter) {
	// Generate the sample of weight vectors
	TWeightsSet WeightsSet;
	WeightsSet.GenerateUniformCover (WeightSpaceSamplingParameter);

	// At first particular objectives are optimized
	TWeightVector WeightVector;
	TPoint TempReferencePoint;

	unsigned int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		WeightVector [iobj] = 1;
		SingleObjectiveOptimization (TempReferencePoint, _Linear, WeightVector, true);
		WeightVector [iobj] = 0;
	}

	WeightsSet.Rescale (pNondominatedSet->ApproximateIdealPoint, pNondominatedSet->ApproximateNadirPoint);

	// Optimize the rest of the weight vectors except of those corresponding
	// to optimization of particular objectives
	unsigned int i; for (i = 0; i < WeightsSet.size (); i++) {
		int iGreaterThanZero = 0;
		int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
			if (WeightsSet [i][iobj] > 0)
				iGreaterThanZero++;
		}
		if (iGreaterThanZero > 1) {
			SingleObjectiveOptimization (pNondominatedSet->ApproximateIdealPoint, ScalarizingFunctionType, WeightsSet [i], true);
			cout << '.';
		}
	}

}

template <class TProblemSolution>
double TMOMethod <TProblemSolution>::OnlineOfflineEvaluation (int NumberOfWeightVectors, 
															  TScalarizingFunctionType ScalarizingFunctionType,
															  double MaximumEffectivenessIndex) {
	this->MaximumEffectivenessIndex = MaximumEffectivenessIndex;
	EvaluationScalarizingFunctionType = ScalarizingFunctionType;

	TWeightVector WeightVector;
	time_t StartingTime;
	TSolutionsSet Solutions;

	// Generate the weights set
	int i; for (i = 0; i < NumberOfWeightVectors; i++) {
		TWeightVector WeightVector = GetRandomWeightVector ();
		EvaluationWeightsSet.push_back (WeightVector);
	}
	// Find reference point

	// Clear the weight vector
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++)
		WeightVector [iobj] = 0;

	// Optimize each objective individually 
	TPoint TempReferencePoint;
	for (iobj = 0; iobj < NumberOfObjectives; iobj++)
		TempReferencePoint.ObjectiveValues [iobj] = 0;

	TSolutionsSet SolutionsSet;

	for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		if (Objectives [iobj].bActive) {
			WeightVector [iobj] = 1;

			TListSet <TProblemSolution> TempNondominatedSet;
			TProblemSolution* Solution = new TProblemSolution (_Linear, TempReferencePoint, WeightVector, TempNondominatedSet);

			int iobj1; for (iobj1 = 0; iobj1 < NumberOfObjectives; iobj1++) {
				cout << Solution->ObjectiveValues [iobj1] << '\t';
			}

			SolutionsSet.push_back (Solution);

			WeightVector [iobj] = 0;
		}
	}

	SolutionsSet.UpdateIdealNadir ();
	
	for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		if (Objectives [iobj].bActive) {
			ReferencePoint.ObjectiveValues [iobj] = SolutionsSet.ApproximateIdealPoint.ObjectiveValues [iobj];
		}
	}

	for (i = 0; i < NumberOfWeightVectors; i++) {
		EvaluationWeightsSet [i].Rescale (SolutionsSet.ApproximateIdealPoint, 
		SolutionsSet.ApproximateNadirPoint);
	}

	SolutionsSet.DeleteAll ();

	AverageOnlineScalarizingFunctionValue = 0;
	AverageOnlineTime = 0;

	int iIters = 0;

	vector<TPoint*> Points;

	for (i = 0; i < NumberOfWeightVectors; i++) {
        cout << i << "\n";
		StartingTime = time (NULL);
		TPoint Point = SingleObjectiveOptimization (ReferencePoint, ScalarizingFunctionType, EvaluationWeightsSet [i], false);
		AverageOnlineTime += time (NULL) - StartingTime;

		Points.push_back (new TPoint (Point));

		double Res;

		switch (ScalarizingFunctionType) {
			case _Linear :
			   Res = Point.LinearScalarizingFunction (EvaluationWeightsSet [i], ReferencePoint);
			   break;
			case _Chebycheff :
				Res = Point.ChebycheffScalarizingFunction (EvaluationWeightsSet [i], ReferencePoint);
				break;
		}

		AverageOnlineScalarizingFunctionValue += Res;
			
		iIters++;

	}

	AverageOnlineScalarizingFunctionValue /= NumberOfWeightVectors;

	for (i = 0; i < Points.size (); i++) 
		delete Points [i];

	AverageOnlineTime /= (double)NumberOfWeightVectors;

	TimeUnit = AverageOnlineTime / 10;
	if (TimeUnit < 1)
		TimeUnit = 1;

	bOnlineOfflineEvaluating = true;

	OfflineTime = 0;
	PreviousTime = time (NULL);

	Run ();

	time_t ElapsedTime = time (NULL) - PreviousTime;
	OfflineTime += ElapsedTime;

	bOnlineOfflineEvaluating = false;

	double EffectivenessIndex = OfflineTime / AverageOnlineTime;

	if (EffectivenessIndex > MaximumEffectivenessIndex)
		return -1;
	else
		return EffectivenessIndex;
}

template <class TProblemSolution>
void TMOMethod <TProblemSolution>::NewSolutionGenerated (TProblemSolution& NewSolution) {
	if (bOnlineOfflineEvaluating) {
		time_t ElapsedTime = time (NULL) - PreviousTime;
		if (ElapsedTime > TimeUnit) {
			OfflineTime += ElapsedTime;
			if (OfflineTime / AverageOnlineTime > MaximumEffectivenessIndex)
				bStop = true;
			double AverageOfflineScalarizingFunctionValue = pNondominatedSet->AverageScalarizingFunction (ReferencePoint,
				EvaluationScalarizingFunctionType, EvaluationWeightsSet);
			if (AverageOfflineScalarizingFunctionValue < 
				AverageOnlineScalarizingFunctionValue) 
				bStop = true;
			cout << "AverageOnline = " << AverageOnlineScalarizingFunctionValue << '\t';
			cout << "AverageOffline = " << AverageOfflineScalarizingFunctionValue << '\t';
			cout << "AverageOnlineTime = " << AverageOnlineTime << '\t';
			cout << "OfflineTime = " << OfflineTime << '\n';
			PreviousTime = time (NULL);

		}
	}
}

#endif // !defined(AFX_MOMETHOD_H__46C81620_9636_11D4_829A_000000000000__INCLUDED_CPP)
