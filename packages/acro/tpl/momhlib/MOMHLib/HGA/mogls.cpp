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

#if !defined(AFX_TMOGLS_H__EE158132_9605_11D3_814C_000000000000__INCLUDED_CPP)
#define AFX_TMOGLS_H__EE158132_9605_11D3_814C_000000000000__INCLUDED_CPP

// MOGLS.cpp: implementation of the TMOGLS class.
//
//////////////////////////////////////////////////////////////////////

#include "mogls.h"

template <class TProblemSolution>
void TMOGLS <TProblemSolution>::SetParameters (int TempPopulationSize, int InitialPopulationSize,
		int NumberOfIterations,
		TScalarizingFunctionType ScalarizingFunctionType) {
	this->TempPopulationSize = TempPopulationSize;
	this->InitialPopulationSize = InitialPopulationSize;
	this->ScalarizingFunctionType = ScalarizingFunctionType;
	this->NumberOfIterations = NumberOfIterations;
}

template <class TProblemSolution>
void TMOGLS <TProblemSolution>::Run()
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
	int Iteration = 0;
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

		// Find temporary population
		this->TempPopulation.clear ();
		int isol; for (isol = 0; isol < this->MainPopulation.size (); isol++) {
			this->UpdateTempPopulation (TempReferencePoint, *(TMOMHSolution*)(this->MainPopulation [isol]));
		}

		// Find two different solutions from the temporary population for recombination
		int isol1 = rand () % this->TempPopulation.size ();
		int isol2;
		do {
			isol2 = rand () % this->TempPopulation.size ();
		} while (isol1 == isol2);

		// Generate new solution by recombination
		TListSet <TProblemSolution> TempNondominatedSet;
		TProblemSolution* Solution = 
			new TProblemSolution (*(TProblemSolution*)this->TempPopulation [isol1], *(TProblemSolution*)this->TempPopulation [isol2],
			this->ScalarizingFunctionType, TempReferencePoint, this->WeightVector, TempNondominatedSet);

		// Update the nondominated set
		bool bAdded = this->pNondominatedSet->Update (*Solution);
		bool bAddedSet = this->pNondominatedSet->Update (TempNondominatedSet);
		bAdded = bAdded || bAddedSet;
		bool bBetter = UpdateTempPopulation (TempReferencePoint, *Solution);
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
void TDisplayMOGLS <TProblemSolution>::End () {
	cout << "Optimization finished\n";
	cout << this->pNondominatedSet->size () << '\t';
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		if (Objectives [iobj].bActive)
			cout << this->pNondominatedSet->ApproximateIdealPoint.ObjectiveValues [iobj] << '\t';
		if (Objectives [iobj].bActive)
			cout << this->pNondominatedSet->ApproximateNadirPoint.ObjectiveValues [iobj] << '\t';
	}
	cout << '\n';
}

template <class TProblemSolution>
void TDisplayMOGLS <TProblemSolution>::NewNondominatedSolutionFound () {
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
void TDisplayMOGLS <TProblemSolution>::InitialPopulationFound () {
 	cout << this->MainPopulation.size () << " initial solutions generated\n";
}

#endif // !defined(AFX_TMOGLS_H__EE158132_9605_11D3_814C_000000000000__INCLUDED_CPP)
