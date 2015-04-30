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

#if !defined(AFX_TPMA_H__EE158132_9605_11D3_814C_000000000000__INCLUDED_CPP)
#define AFX_TPMA_H__EE158132_9605_11D3_814C_000000000000__INCLUDED_CPP

// PMA.cpp: implementation of the TPMA class.
//
//////////////////////////////////////////////////////////////////////

#include "pma.h"

template <class TProblemSolution>
void TPMA <TProblemSolution>::SetParameters (int TempPopulationSize, int InitialPopulationSize,
		int NumberOfIterations,
		TScalarizingFunctionType ScalarizingFunctionType) {
	this->TempPopulationSize = TempPopulationSize;
	this->InitialPopulationSize = InitialPopulationSize;
	this->ScalarizingFunctionType = ScalarizingFunctionType;
	this->NumberOfIterations = NumberOfIterations;
}

template <class TProblemSolution>
void TPMA <TProblemSolution>::Run()
{
	// Callback function
	this->Start ();

	this->MainPopulation.DeleteAll ();
	this->pNondominatedSet->DeleteAll ();

	this->FindInitialPopulation();

	// Callback function
	this->InitialPopulationFound ();

   	MainPopulationSize = this->MainPopulation.size () * this->TempPopulationSize;

	// Main loop
	unsigned int Iteration = 0;
	do {
		// Draw at random new weight vector
		this->WeightVector = GetRandomWeightVector ();

		// Rescale the weight vector
		this->WeightVector.Rescale (this->pNondominatedSet->ApproximateIdealPoint,
			this->pNondominatedSet->ApproximateNadirPoint);

		// Calculate reference point
		TPoint TempReferencePoint = this->pNondominatedSet->ApproximateIdealPoint;
		TempReferencePoint.Augment (this->pNondominatedSet->ApproximateIdealPoint,
			this->pNondominatedSet->ApproximateNadirPoint);

		// Find two different solutions for recombination
		int TournamentSize = (int)floor (3.0 * this->MainPopulation.size () / (this->TempPopulationSize + 1) + 0.5);

		// Parent 1 - the best solution in tournament
		// Parent 2 - the second best solution in tournament
		TProblemSolution* Parent1 = (TProblemSolution*)this->MainPopulation [rand () % this->MainPopulation.size ()];
		TProblemSolution* Parent2;
		do {
			Parent2 = (TProblemSolution*)this->MainPopulation [rand () % this->MainPopulation.size ()];
		} while (Parent1 == Parent2);
		
		Parent1->SetScalarizingFunctionType (this->ScalarizingFunctionType);
		Parent1->ScalarizingFunctionValue = 
			Parent1->ScalarizingFunction (TempReferencePoint, this->WeightVector); 

		Parent2->SetScalarizingFunctionType (this->ScalarizingFunctionType);
		Parent2->ScalarizingFunctionValue = 
			Parent2->ScalarizingFunction (TempReferencePoint, this->WeightVector); 

		// Perform tournament
		int i; for (i = 2; i < TournamentSize; i++) {
			TProblemSolution* Challenger = (TProblemSolution*)this->MainPopulation [rand () % this->MainPopulation.size ()];
			Challenger->SetScalarizingFunctionType (this->ScalarizingFunctionType);
			Challenger->ScalarizingFunctionValue = 
				Challenger->ScalarizingFunction (TempReferencePoint, this->WeightVector); 
			if (Challenger->ScalarizingFunctionValue < Parent1->ScalarizingFunctionValue) {
				Parent2 = Parent1;
				Parent1 = Challenger;
			}
			else if ((Challenger->ScalarizingFunctionValue < Parent2->ScalarizingFunctionValue) && (Parent1 != Challenger)) {
				Parent2 = Challenger;
			}
		}

		// Generate new solution by recombination
		TListSet <TProblemSolution> TempNondominatedSet;
		TProblemSolution* Solution = 
			new TProblemSolution (*Parent1, *Parent2,
			this->ScalarizingFunctionType, TempReferencePoint, this->WeightVector, TempNondominatedSet);

		// Update the nondominated set
		bool bAdded = this->pNondominatedSet->Update (*Solution);
		bool bAddedSet = this->pNondominatedSet->Update (TempNondominatedSet);
		bAdded = bAdded || bAddedSet;
		Solution->ScalarizingFunctionValue = 
			Solution->ScalarizingFunction (TempReferencePoint, this->WeightVector); 
		bool bBetter = Solution->ScalarizingFunctionValue < Parent2->ScalarizingFunctionValue;
		if (bAdded) {
			// Callback function
			this->NewNondominatedSolutionFound ();
		}

		// If the new solution is nondominated or better than
		// the worst solution in the current population
		// Add it to the main population
		if (bAdded || bBetter) {
			this->MainPopulation.push_back (Solution);
			if (this->MainPopulation.size () > MainPopulationSize) {
				delete ((TProblemSolution*)this->MainPopulation [0]);
				this->MainPopulation.erase (this->MainPopulation.begin ());
			}
		}
		else
			delete Solution;

		// Callback function
		NewSolutionGenerated (*Solution);

		Iteration++;

	} while (!this->bStop && 
		((this->bOnlineOfflineEvaluating) || 
		(Iteration < NumberOfIterations * this->InitialPopulationSize)));

	this->MainPopulation.DeleteAll ();

	// Callback function
	this->End ();
}

template <class TProblemSolution>
void TDisplayPMA <TProblemSolution>::End () {
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
void TDisplayPMA <TProblemSolution>::NewNondominatedSolutionFound () {
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
void TDisplayPMA <TProblemSolution>::InitialPopulationFound () {
 	cout << this->MainPopulation.size () << " initial solutions generated\n";
}

#endif // !defined(AFX_TPMA_H__EE158132_9605_11D3_814C_000000000000__INCLUDED_CPP)
