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

#if !defined(AFX_HGABase_H__EE158132_9605_11D3_814C_000000000000__INCLUDED_CPP)
#define AFX_HGABase_H__EE158132_9605_11D3_814C_000000000000__INCLUDED_CPP

// PMA.cpp: implementation of the THGABase class.
//
//////////////////////////////////////////////////////////////////////

#include "hgabase.h"

template <class TProblemSolution>
void THGABase <TProblemSolution>::UpdateVirtualPopulation (TVirtualPopulation& VirtualPopulation, 
							  double NewSolutionValue) {
	// Accept the solution value if it better than a solution in the population
	// and is different (on the scalarizing funztion) to all the solutions 

	bool bAdd = false;

	// If the TempPopulation is empty
	if (VirtualPopulation.size () == 0) {
		VirtualPopulation.push_back (NewSolutionValue);
		return;
	}

	// If the size of the temporary ppulation is smaller than VirtualPopulationSize
	// or if the new solution is better than the worst (last) solution in the population
	if (VirtualPopulation.size () < TempPopulationSize)
		bAdd = true;
	else if (NewSolutionValue < 
		VirtualPopulation [VirtualPopulation.size () - 1])
		bAdd = true;

	if (bAdd) {
		// If new solution is worse than currently worst add it at the end
		if (NewSolutionValue > 
			VirtualPopulation [VirtualPopulation.size () - 1])
			VirtualPopulation.push_back (NewSolutionValue);

		// If new solution is better than currently best add it at the begining
		else if (NewSolutionValue < 
			VirtualPopulation [0])
			VirtualPopulation.insert (VirtualPopulation.begin (), NewSolutionValue);

		else {
			
			// Find proper (assuring proper sorting of the population) position to place
			// the new solution
			
			// Two extreme positions
			int Pos1 = 0;
			int Pos2 = VirtualPopulation.size () - 1;
			
			// Repeat until the solution is placed
			bool bPlaced = false;
			while (!bPlaced) {
				// If the insertion position is found
				if (Pos2 - Pos1 <= 1) {
					VirtualPopulation.insert (VirtualPopulation.begin () + Pos2, NewSolutionValue);
					bPlaced = true;
				}
				else {
					// Divide the search region
					int Pos3 = (Pos2 + Pos1) / 2;
					if (NewSolutionValue >= 
						VirtualPopulation [Pos3])
						Pos1 = Pos3;
					else
						Pos2 = Pos3;
				}
			} // while (!bPlaced)
		}
		if (VirtualPopulation.size () > TempPopulationSize)
			VirtualPopulation.pop_back ();
	} // if (bAdd)
}

template <class TProblemSolution>
void THGABase <TProblemSolution>::FindInitialPopulation()
{
TVirtualPopulation VirtualPopulation;
bool bEnd;

	LastVirtualPopulationUpdated = 0;

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
				this->NewNondominatedSolutionFound ();
			}

			this->NewSolutionGenerated (*Solution);

			WeightVector [iobj] = 0;
		}
	}
	
	// Generate new initial solutions while the the quality of the MainPopulation 
	// is not sufficient
	double PopulationEvaluation = 1e30;
	do {
		// Draw at random new weight vector
		WeightVector = GetRandomWeightVector ();

		WeightVector.Rescale (this->pNondominatedSet->ApproximateIdealPoint,
			this->pNondominatedSet->ApproximateNadirPoint);

		// Find new initial solution
		TListSet <TProblemSolution> TempNondominatedSet;
		TProblemSolution* Solution = new TProblemSolution (ScalarizingFunctionType, this->pNondominatedSet->ApproximateIdealPoint, WeightVector, TempNondominatedSet);

		VirtualPopulations.push_back (VirtualPopulation);

		MainPopulation.push_back (Solution);

		NewSolutionGenerated (*Solution);

		bool bAdded = this->pNondominatedSet->Update (*Solution);
		bool bAddedSet = this->pNondominatedSet->Update (TempNondominatedSet);
		bAdded = bAdded || bAddedSet;
		if (bAdded) {
			this->NewNondominatedSolutionFound ();
		}

		bEnd = false;
		if (InitialPopulationSize <= 0) {
			if ((MainPopulation.size () > TempPopulationSize) && 
				(MainPopulation.size () % TempPopulationSize == 0)){
				PopulationEvaluation = EvaluateInitialPopulation ();
				bEnd = PopulationEvaluation <= 0;
			}
		}
		else
			bEnd = (MainPopulation.size () >= InitialPopulationSize) &&
				(MainPopulation.size () >= TempPopulationSize);
	} while (!bEnd);

	VirtualPopulations.clear ();

	InitialPopulationSize = MainPopulation.size ();
}

template <class TProblemSolution>
double THGABase <TProblemSolution>::EvaluateInitialPopulation()
{
	double PopulationEvaluation = 0;

	// For all solutions
	unsigned int isol; for (isol = 0; isol < MainPopulation.size (); isol++) {
		// Current weight vector is taken from isol
		WeightVector = MainPopulation [isol]->WeightVector;

		MainPopulation [isol]->ScalarizingFunctionValue = 
			MainPopulation [isol]->ScalarizingFunction (this->pNondominatedSet->ApproximateIdealPoint, WeightVector);

		// First solution to start updating virtual population
		int iStartingSolution;
		if (isol <= LastVirtualPopulationUpdated)
			iStartingSolution = LastVirtualPopulationUpdated + 1;
		else
			iStartingSolution = 0;

		// For all solutions different than isol
		unsigned int isol2; for (isol2 = iStartingSolution; isol2 < MainPopulation.size (); isol2++) {
			double SolutionValue = MainPopulation [isol2]->ScalarizingFunction (this->pNondominatedSet->ApproximateIdealPoint, WeightVector);
			if (isol2 != isol) {
				UpdateVirtualPopulation (VirtualPopulations [isol], SolutionValue);
			}
		}

		// Calculate average scalarizing function value in TempPopulation
		double Sum = 0;
		unsigned int i; for (i = 0; i < TempPopulationSize; i++) {
			Sum += VirtualPopulations [isol][i];
		}
		double AverageScalarizingFunctionValue = Sum / TempPopulationSize;

		double t = MainPopulation [isol]->ScalarizingFunctionValue;

		PopulationEvaluation += AverageScalarizingFunctionValue - MainPopulation [isol]->ScalarizingFunctionValue;
	}

	LastVirtualPopulationUpdated = VirtualPopulations.size () - 1;

	return PopulationEvaluation;
}

template <class TProblemSolution>
bool THGABase <TProblemSolution>::UpdateTempPopulation (TPoint& ReferencePoint, TMOMHSolution& Solution) {
	// Accept the solution if it better than a solution in the population
	// and is different (on the scalarizing funztion) to all the solutions 

	bool bAdd = false;
	bool bAdded = false;

	Solution.SetScalarizingFunctionType (ScalarizingFunctionType);
	Solution.ScalarizingFunctionValue = 
		Solution.ScalarizingFunction (ReferencePoint, WeightVector); 

	// If the TempPopulation is empty
	if (TempPopulation.size () == 0) {
		TempPopulation.push_back (&Solution);
		return true;
	}

	// If the size of the temporary population is smaller than TempPopulationSize
	// or if the new solution is better than the worst (last) solution in the population
	if (TempPopulation.size () < TempPopulationSize)
		bAdd = true;
	else if (Solution.ScalarizingFunctionValue < 
		TempPopulation [TempPopulation.size () - 1]->ScalarizingFunctionValue)
		bAdd = true;

	if (bAdd) {
		bool bEqual = false;
			
		// If new solution is worse than currently worst add it at the end
		if (Solution.ScalarizingFunctionValue > 
			TempPopulation [TempPopulation.size () - 1]->ScalarizingFunctionValue) {
			TempPopulation.push_back (&Solution);
			bAdded = true;
		}

		// If new solution is better than currently best add it at the begining
		else if (Solution.ScalarizingFunctionValue < 
			TempPopulation [0]->ScalarizingFunctionValue) {
			TempPopulation.insert (TempPopulation.begin (), &Solution);
			bAdded = true;
		}

		else {
			// Find proper (assuring proper sorting of the population) position to place
			// the new solution

			// Dwie skraje pozycje
			int Pos1 = 0;
			int Pos2 = TempPopulation.size () - 1;
			
			// Czy nowe rozwi¹zanie jest równe ostatniemu lub pierwszemu
			if ((Solution.ScalarizingFunctionValue == 
				TempPopulation [TempPopulation.size () - 1]->ScalarizingFunctionValue) ||
				(Solution.ScalarizingFunctionValue ==
				TempPopulation [0]->ScalarizingFunctionValue))
				bEqual = true;
			
			// Powtarzaj dopóki nie umieszczono rozwi¹zania
			bool bPlaced = false;
			while (!bPlaced && !bEqual) {
				// Jezeli znaleziono pozycje wstawienia
				if (Pos2 - Pos1 == 1) {
					TempPopulation.insert (TempPopulation.begin () + Pos2, &Solution);
					bPlaced = true;
					bAdded = true;
				}
				else {
					// Podziel obszar poszukiwan na po³owê
					int Pos3 = (Pos2 + Pos1) / 2;
					if (Solution.ScalarizingFunctionValue == 
						TempPopulation [Pos3]->ScalarizingFunctionValue) {
						bEqual = true;
					}
					else {
						if (Solution.ScalarizingFunctionValue > 
							TempPopulation [Pos3]->ScalarizingFunctionValue)
							Pos1 = Pos3;
						else
							Pos2 = Pos3;
					}
				}
			} // while (!bPlaced && !bEqual)
		}
		if (!bEqual) {
			if (TempPopulation.size () > TempPopulationSize)
				TempPopulation.pop_back ();
		}
	} // if (bAdd)

	return bAdded;
}

template <class TProblemSolution>
TProblemSolution THGABase <TProblemSolution>::SingleObjectiveOptimization (TPoint ReferencePoint, 
															   TScalarizingFunctionType ScalarizingFunctionType, 
															   TWeightVector& WeightVector,
															   bool bUpdateNondominatedSet) {
	int PopulationSize = TempPopulationSize;

	TProblemSolution BestSolution;

	TempPopulationSize = PopulationSize;
	this->ScalarizingFunctionType = ScalarizingFunctionType;
	this->WeightVector = WeightVector;

	TProblemSolution* Parent1;
	// Generate initial population
	int i; for (i = 0; i < PopulationSize; i++) {
		TListSet <TProblemSolution> TempNondominatedSet;

		TProblemSolution* Solution = new TProblemSolution (ScalarizingFunctionType, ReferencePoint, WeightVector, TempNondominatedSet);
		
		MainPopulation.push_back (Solution);
		UpdateTempPopulation (ReferencePoint, *Solution);
		if (TempPopulation.size () > 1)
			Parent1 = (TProblemSolution*)TempPopulation [1];

		// Callback function
		NewSolutionGenerated (*Solution);
		
		if (bUpdateNondominatedSet) {
			// Update the nondominated set
			bool bAdded = this->pNondominatedSet->Update (*Solution);
			bool bAddedSet = this->pNondominatedSet->Update(TempNondominatedSet);
			bAdded = bAdded || bAddedSet;
			if (bAdded) {
				// Callback function
				this->NewNondominatedSolutionFound ();
			}
		}

	}

	int Iterations = 0;
	int IterationWithNoImprovement = 0;
	do {
		Iterations ++;
		// Find two different solutions for recombination
		int isol1 = rand () % TempPopulation.size ();
		int isol2;
		do {
			isol2 = rand () % TempPopulation.size ();
		} while ((isol1 == isol2) && (TempPopulation.size () != 1));

		TProblemSolution* Parent1 = (TProblemSolution*)TempPopulation [isol1];
		TProblemSolution* Parent2 = (TProblemSolution*)TempPopulation [isol2];

		// Recombine the two parents
		TListSet <TProblemSolution> TempNondominatedSet;
		TProblemSolution* Solution = 
			new TProblemSolution (*Parent1, *Parent2, ScalarizingFunctionType, ReferencePoint, WeightVector, TempNondominatedSet);

		MainPopulation.push_back (Solution);

		double NewValue = Solution->ScalarizingFunction (ReferencePoint, WeightVector);

		bool bBetter = UpdateTempPopulation (ReferencePoint, *Solution);

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

		if (!bBetter) {
			IterationWithNoImprovement++;
		}
		else {
			IterationWithNoImprovement = 0;
		}

	} while (IterationWithNoImprovement < PopulationSize);

	double BestValue = TempPopulation [0]->ScalarizingFunctionValue;

	BestSolution = *((TProblemSolution*)TempPopulation [0]);

	TempPopulation.clear ();
	MainPopulation.DeleteAll ();
		
	return BestSolution;
}

#endif // !defined(AFX_HGABase_H__EE158132_9605_11D3_814C_000000000000__INCLUDED_CPP)
