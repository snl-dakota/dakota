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

#if !defined(AFX_IMMOGLS_H__EE158132_9605_11D3_814C_000000000000__INCLUDED_CPP)
#define AFX_IMMOGLS_H__EE158132_9605_11D3_814C_000000000000__INCLUDED_CPP

// TIMMOGLS.cpp: implementation of the TIMMOGLS class.
//
//////////////////////////////////////////////////////////////////////

#include "immogls.h"

template <class TProblemSolution>
void TIMMOGLS <TProblemSolution>::SetParameters (int PopulationSize, 
		int NumberOfIterations, int EliteSize,
		TScalarizingFunctionType ScalarizingFunctionType) {
	this->PopulationSize = PopulationSize;
	this->EliteSize = EliteSize;
	this->ScalarizingFunctionType = ScalarizingFunctionType;
	this->NumberOfIterations = NumberOfIterations;
}

template <class TProblemSolution>
void TIMMOGLS <TProblemSolution>::FindInitialPopulation()
{
	// Clear the weight vector
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++)
		WeightVector [iobj] = 0;

	TPoint TempReferencePoint;
	for (iobj = 0; iobj < NumberOfObjectives; iobj++)
		TempReferencePoint.ObjectiveValues [iobj] = 0;

	// Optimize each objective individually
	for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		if (Objectives [iobj].bActive) {
			WeightVector [iobj] = 1;

			TListSet <TProblemSolution> TempNondominatedSet;
			TProblemSolution* Solution = new TProblemSolution (_Linear, TempReferencePoint, WeightVector, TempNondominatedSet);

			bool bAdded = this->pNondominatedSet->Update (*Solution);
			bool bAddedSet = this->pNondominatedSet->Update (TempNondominatedSet);
			bAdded = bAdded || bAddedSet;
			if (bAdded) {
				// Callback function
				this->NewNondominatedSolutionFound ();
			}

			// Callback function
			NewSolutionGenerated (*Solution);

			WeightVector [iobj] = 0;
		}
	}
	
	// Generate new random local optima 
	do {
		// Draw at random new weight vector
		WeightVector = GetRandomWeightVector ();

		WeightVector.Rescale (this->pNondominatedSet->ApproximateIdealPoint,
			this->pNondominatedSet->ApproximateNadirPoint);

		// Find new local optimum
		TListSet <TProblemSolution> TempNondominatedSet;
		TProblemSolution* Solution = new TProblemSolution (ScalarizingFunctionType, this->pNondominatedSet->ApproximateIdealPoint, WeightVector, TempNondominatedSet);

		Population.push_back (Solution);

		// Callback function
		NewSolutionGenerated (*Solution);

		bool bAdded = this->pNondominatedSet->Update (*Solution);
		bool bAddedSet = this->pNondominatedSet->Update (TempNondominatedSet);
		bAdded = bAdded || bAddedSet;
		if (bAdded) {
			// Callback function
			this->NewNondominatedSolutionFound ();
		}

	} while (Population.size () < PopulationSize);

}

template <class TProblemSolution>
void TIMMOGLS <TProblemSolution>::Run() {
	// Callback function
	this->Start ();
	
	Population.DeleteAll ();
	this->pNondominatedSet->DeleteAll ();
	
	FindInitialPopulation();
	// Callback function
	InitialPopulationFound ();
	
	// Main loop
	int igen; for (igen = 0; (igen <= NumberOfIterations) && !this->bStop; igen++) {
		int iOldPopulationSize = Population.size ();
		// Find new generation
		int i; for (i = 0; i < PopulationSize; i++) {
			// Draw at random new weight vector
			WeightVector = GetRandomWeightVector ();
			
			WeightVector.Rescale (this->pNondominatedSet->ApproximateIdealPoint,
				this->pNondominatedSet->ApproximateNadirPoint);
			
			// Calculate reference point
			TPoint TempReferencePoint = this->pNondominatedSet->ApproximateIdealPoint;
			TempReferencePoint.Augment (this->pNondominatedSet->ApproximateIdealPoint,
				this->pNondominatedSet->ApproximateNadirPoint);
			
			int j; for (j = 0; j < Population.size (); j++) {
				Population [j]->SetScalarizingFunctionType (ScalarizingFunctionType);
				Population [j]->ScalarizingFunctionValue = 
					Population [j]->ScalarizingFunction (TempReferencePoint, WeightVector); 
			}
			
			// Find the best value of the current function
			double best = Population [0]->ScalarizingFunctionValue;
			for (j = 1; j < Population.size (); j++) {
				if (best > Population [j]->ScalarizingFunctionValue)
					best = Population [j]->ScalarizingFunctionValue;
			}
			
			// Find the worst value of the current function
			double worst = Population [0]->ScalarizingFunctionValue;
			for (j = 1; j < Population.size (); j++) {
				if (worst < Population [j]->ScalarizingFunctionValue)
					worst = Population [j]->ScalarizingFunctionValue;
			}

			// Find two solutions for recombination by roulette wheel selection
			double SumFitness = 0;
			for (j = 0; j < Population.size (); j++) {
				SumFitness += -(Population [j]->ScalarizingFunctionValue - worst);
			}
			
			double RandFitness = (rand () % 10000) / 10000.0 * SumFitness;
			
			double PartialSum = 0;
			j = 0;
			PartialSum += -(Population [0]->ScalarizingFunctionValue - worst);
			while ((PartialSum < RandFitness) && (j < Population.size () - 1)) {
				j++;
				PartialSum += -(Population [j]->ScalarizingFunctionValue - worst);
			}
			int isol1 = j;
			
			RandFitness = (rand () % 10000) / 10000.0 * SumFitness;
			
			PartialSum = 0;
			j = 0;
			PartialSum += -(Population [0]->ScalarizingFunctionValue - worst);
			while ((PartialSum < RandFitness) && (j < Population.size () - 1)) {
				j++;
				PartialSum += -(Population [j]->ScalarizingFunctionValue - worst);
			}
			
			int isol2 = j;

			// Generate new solution by recombination
			TListSet <TProblemSolution> TempNondominatedSet;			
			TProblemSolution* Solution = 
				new TProblemSolution (*(TProblemSolution*)Population [isol1], *(TProblemSolution*)Population [isol2],
				ScalarizingFunctionType, TempReferencePoint, WeightVector, TempNondominatedSet);		

			Population.push_back (Solution);
			
			// Update the nondominated set
			bool bAdded = this->pNondominatedSet->Update (*Solution);
			bool bAddedSet = this->pNondominatedSet->Update (TempNondominatedSet);
			bAdded = bAdded || bAddedSet;
			if (bAdded) {
				// Callback function
				this->NewNondominatedSolutionFound ();
			}
			// Callback function
			NewSolutionGenerated (*Solution);
		}
		
		// Add elite solutions to the population
		for (i = 0; i < EliteSize; i++) {
			TSolution  *pEliteSolution = NULL;
			this->pNondominatedSet -> GetRandomSolution(pEliteSolution);
			if (pEliteSolution != NULL) {
				TProblemSolution* Solution = new TProblemSolution(*((TProblemSolution *)pEliteSolution));
				Population.push_back(Solution);
			}
		}
		
		// Remove old population
		for (i = 0; i < iOldPopulationSize; i++) {
			delete (TProblemSolution*)Population [0];
			Population.erase (Population.begin ());
		}
	}
	Population.DeleteAll ();
	
	// Callback function
	this->End ();
}

template <class TProblemSolution>
void TDisplayIMMOGLS <TProblemSolution>::End () {
	cout << "Optimization finished\n";
	cout << this->pNondominatedSet -> iSetSize << '\t';
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		if (Objectives [iobj].bActive)
			cout << this->pNondominatedSet->ApproximateIdealPoint.ObjectiveValues [iobj] << '\t';
		if (Objectives [iobj].bActive)
			cout << this->pNondominatedSet->ApproximateNadirPoint.ObjectiveValues [iobj] << '\t';
	}
	cout << '\n';
}

template <class TProblemSolution>
void TDisplayIMMOGLS <TProblemSolution>::NewNondominatedSolutionFound () {
	cout << this->pNondominatedSet->iSetSize << '\t';
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		if (Objectives [iobj].bActive)
			cout << this->pNondominatedSet->ApproximateIdealPoint.ObjectiveValues [iobj] << '\t';
		if (Objectives [iobj].bActive)
			cout << this->pNondominatedSet->ApproximateNadirPoint.ObjectiveValues [iobj] << '\t';
	}
	cout << '\n';
	this->pNondominatedSet->Save ("NdTemp.txt");
	double Min;
	for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		Min = (*this->pNondominatedSet)[0]->ObjectiveValues [iobj];
		int i; for (i = 1; i < this->pNondominatedSet->size (); i++) {
            if ((*this->pNondominatedSet)[i] == NULL) {
                continue;
            }
			if (Min > (*this->pNondominatedSet) [i]->ObjectiveValues [iobj])
				Min = (*this->pNondominatedSet) [i]->ObjectiveValues [iobj];
		}
		if (Min != this->pNondominatedSet->ApproximateNadirPoint.ObjectiveValues [iobj]) {
			cout << "Error\n";
			cout << Min << '\t' << this->pNondominatedSet->ApproximateNadirPoint.ObjectiveValues [iobj] << '\n';
			exit (0);
		}
	}
//	cout << '+';
}

template <class TProblemSolution>
void TDisplayIMMOGLS <TProblemSolution>::InitialPopulationFound () {
 	cout << this->Population.size () << " initial solutions generated\n";
}

#endif // !defined(AFX_TIMMOGLS_H__EE158132_9605_11D3_814C_000000000000__INCLUDED_CPP)
