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

// MOMSLS.cpp: implementation of the TMOMSLS class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOMSLS_H__75C0B360_9D53_11D4_82A7_000000000000__INCLUDED_CPP)
#define AFX_MOMSLS_H__75C0B360_9D53_11D4_82A7_000000000000__INCLUDED_CPP

#include "momsls.h"

template <class TProblemSolution>
void TMOMSLS <TProblemSolution>::SetParameters (int NumberOfIterations,
		TScalarizingFunctionType ScalarizingFunctionType) {
	this->ScalarizingFunctionType = ScalarizingFunctionType;
	this->NumberOfIterations = NumberOfIterations;
}

template <class TProblemSolution>
TProblemSolution TMOMSLS <TProblemSolution>::SingleObjectiveOptimization (TPoint ReferencePoint, 
															   TScalarizingFunctionType ScalarizingFunctionType, 
															   TWeightVector& WeightVector,
															   bool bUpdateNondominatedSet) {
	TProblemSolution BestSolution;

	// Generate NumberOfIterations initial solutions and store the best solution
	int i; for (i = 0; i < NumberOfIterations; i++) {
		TListSet <TProblemSolution> TempNondominatedSet;
		TProblemSolution* Solution = new TProblemSolution (ScalarizingFunctionType, ReferencePoint, WeightVector, TempNondominatedSet);

		// Callback function
		NewSolutionGenerated (*Solution);
		
		if (bUpdateNondominatedSet) {
			// Update the nondominated set
			bool bAdded = this->pNondominatedSet->Update (*Solution);
			bool bAddedSet = this->pNondominatedSet->Update (TempNondominatedSet);
			bAdded = bAdded || bAddedSet;
			if (bAdded) {
				// Callback function
				this->NewNondominatedSolutionFound ();
			}
		}

		if (i == 0)
			BestSolution = *Solution;
		else
			if (BestSolution.ScalarizingFunction (ReferencePoint) >
				Solution->ScalarizingFunction (ReferencePoint))
				BestSolution = *Solution;

		delete Solution;
	}

	return BestSolution;
}

template <class TProblemSolution>
void TMOMSLS <TProblemSolution>::Run() {
	// Callback method
	this->Start ();

	this->pNondominatedSet->DeleteAll ();

	TWeightVector WeightVector;

	// Clear the weight vector
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++)
		WeightVector [iobj] = 0;

	TPoint TempReferencePoint;
	for (iobj = 0; iobj < NumberOfObjectives; iobj++)
		TempReferencePoint.ObjectiveValues [iobj] = 0;

	int Iteration = 0;

	// Optimize each objective individually
	for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		if (Objectives [iobj].bActive) {
			Iteration++;

			WeightVector [iobj] = 1;

			TListSet <TProblemSolution> TempNondominatedSet;
			TProblemSolution* Solution = new TProblemSolution (_Linear, TempReferencePoint, WeightVector, TempNondominatedSet);

			// Update the nondominated set
			bool bAdded = this->pNondominatedSet->Update (*Solution);
			bool bAddedSet = this->pNondominatedSet->Update (TempNondominatedSet);
			bAdded = bAdded || bAddedSet;
			if (bAdded) {
				// Callback method
				this->NewNondominatedSolutionFound ();
			}

			// Callback method
			NewSolutionGenerated (*Solution);

			WeightVector [iobj] = 0;

			delete Solution;
		}
	}
	
	do {
		Iteration++;
		
		// Draw at random new weight vector
		WeightVector = GetRandomWeightVector ();
		
		WeightVector.Rescale (this->pNondominatedSet->ApproximateIdealPoint,
			this->pNondominatedSet->ApproximateNadirPoint);
		
		// Generate new initial solution
		TListSet <TProblemSolution> TempNondominatedSet;
		TProblemSolution* Solution = new TProblemSolution (ScalarizingFunctionType, this->pNondominatedSet->ApproximateIdealPoint, WeightVector, TempNondominatedSet);
		
		// Update the nondominated set
		bool bAdded = this->pNondominatedSet->Update (*Solution);
		bool bAddedSet = this->pNondominatedSet->Update (TempNondominatedSet);
		bAdded = bAdded || bAddedSet;
		if (bAdded) {
			// Callback method
			this->NewNondominatedSolutionFound ();
		}

		// Callback method
		NewSolutionGenerated (*Solution);

		delete Solution;
		
	} while (!this->bStop && 
		((this->bOnlineOfflineEvaluating) || 
		(Iteration < NumberOfIterations)));

	// Callback method
	this->End ();
}

template <class TProblemSolution>
void TDisplayMOMSLS <TProblemSolution>::End () {
	cout << "Optimization finished\n";
	cout << this->pNondominatedSet->iSetSize << '\t';
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		if (Objectives [iobj].bActive)
			cout << this->pNondominatedSet->ApproximateIdealPoint.ObjectiveValues [iobj] << '\t';
		if (Objectives [iobj].bActive)
			cout << this->pNondominatedSet->ApproximateNadirPoint.ObjectiveValues [iobj] << '\t';
	}
	cout << '\n';
}

template <class TProblemSolution>
void TDisplayMOMSLS <TProblemSolution>::NewNondominatedSolutionFound () {
	cout << this->pNondominatedSet->iSetSize << '\t';
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		if (Objectives [iobj].bActive)
			cout << this->pNondominatedSet->ApproximateIdealPoint.ObjectiveValues [iobj] << '\t';
		if (Objectives [iobj].bActive)
			cout << this->pNondominatedSet->ApproximateNadirPoint.ObjectiveValues [iobj] << '\t';
	}
	cout << '\n';
}

#endif // !defined(AFX_MOMSLS_H__75C0B360_9D53_11D4_82A7_000000000000__INCLUDED_CPP)
