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

#if !defined(AFX_PSA_H__C79F4BD1_DD06_11D4_832E_000000000000__INCLUDED_CPP)
#define AFX_PSA_H__C79F4BD1_DD06_11D4_832E_000000000000__INCLUDED_CPP

// PSA.cpp: implementation of the TPSA class.
//
//////////////////////////////////////////////////////////////////////

#include "psa.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

template <class TProblemSolution>
TPSA <TProblemSolution>::TPSA()
{
	this->StartingTemperature = 1;
	this->FinalTemperature = 0.01;
	this->TemperatureDecreaseCoefficient = 0.9;
	this->WeightsChangeCoefficient = 0.001;
	this->MovesOnTemperatureLevel = 100;
	this->GeneratingPopulationSize = 10;

}

template <class TProblemSolution>
void TPSA <TProblemSolution>::SetParameters (double StartingTemperature,
		double FinalTemperature,
		double TemperatureDecreaseCoefficient,
		double WeightsChangeCoefficient,
		int MovesOnTemperatureLevel,
		int GeneratingPopulationSize) {

	this->StartingTemperature = StartingTemperature;
	this->FinalTemperature = FinalTemperature;
	this->TemperatureDecreaseCoefficient = TemperatureDecreaseCoefficient;
	this->WeightsChangeCoefficient = WeightsChangeCoefficient;
	this->MovesOnTemperatureLevel = MovesOnTemperatureLevel;
	this->GeneratingPopulationSize = GeneratingPopulationSize;

}

template <class TProblemSolution>
void TPSA <TProblemSolution>::FindInitialPopulation () {
	int i; for (i = 0; i < GeneratingPopulationSize; i++) {
		TGeneratingPopulationElement* GeneratingPopulationElement = 
			new TGeneratingPopulationElement;
		GeneratingPopulationElement->GeneratingSolution = new TProblemSolution;
		GeneratingPopulationElement->GeneratingSolution->WeightVector = GetRandomWeightVector ();
		GeneratingPopulationElement->GeneratingSolution->SetScalarizingFunctionType (_Linear);
	
		bool bAdded = this->pNondominatedSet->Update ((TProblemSolution&)*(GeneratingPopulationElement->GeneratingSolution));
		if (bAdded) {
			this->NewNondominatedSolutionFound ();
		}
		GeneratingPopulationElement->WeightVector =
			GeneratingPopulationElement->GeneratingSolution->WeightVector;

		GeneratingPopulation.push_back (GeneratingPopulationElement);
	}
}

template <class TProblemSolution>
void TPSA <TProblemSolution>::UpdateWeightVector (int isol) {
	if (GeneratingPopulationSize > 1) {
		// Find the closest solution
		bool bFirstIter = true;
		double MinDistance;
		int iClosestSolution;
		int isol2; for (isol2 = 0; isol2 < GeneratingPopulationSize; isol2++) {
			if (isol != isol2) {
				if (GeneratingPopulation [isol]->GeneratingSolution->Compare((TProblemSolution&)*GeneratingPopulation [isol2]->GeneratingSolution) == _Nondominated) {
					if (bFirstIter) {
						bFirstIter = false;
						MinDistance = 
							GeneratingPopulation [isol]->GeneratingSolution->Distance((TProblemSolution&)*GeneratingPopulation [isol2]->GeneratingSolution,
							this->pNondominatedSet->ApproximateIdealPoint, this->pNondominatedSet->ApproximateNadirPoint);
						iClosestSolution = isol2;
					}
					else {
						double Distance = 
							GeneratingPopulation [isol]->GeneratingSolution->Distance((TProblemSolution&)*GeneratingPopulation [isol2]->GeneratingSolution,
							this->pNondominatedSet->ApproximateIdealPoint, this->pNondominatedSet->ApproximateNadirPoint);
						if (Distance < MinDistance) { 
							MinDistance = Distance;
							iClosestSolution = isol2;
						}
					}
				}
			}
		}
		
		// Update weights
		int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
			if (Objectives [iobj].bActive) {
				bool bWorse;
				if (!bFirstIter) {
					if (Objectives [iobj].ObjectiveType == _Max) {
						//!!!!
						if (GeneratingPopulation [isol]->GeneratingSolution->ObjectiveValues [iobj] >=
							GeneratingPopulation [iClosestSolution]->GeneratingSolution->ObjectiveValues [iobj])
							bWorse = false;
						else
							bWorse = true;
					}
					if (Objectives [iobj].ObjectiveType == _Min) {
						if (GeneratingPopulation [isol]->GeneratingSolution->ObjectiveValues [iobj] <=
							GeneratingPopulation [iClosestSolution]->GeneratingSolution->ObjectiveValues [iobj])
							bWorse = false;
						else
							bWorse = true;
					}
				}
				else
					bWorse = rand () % 2 == 0;
				if (bWorse) {
					if (GeneratingPopulation [isol]->WeightVector [iobj] > 0.001)
						GeneratingPopulation [isol]->WeightVector [iobj] -= WeightsChangeCoefficient;
				}
				else {
					if (GeneratingPopulation [isol]->WeightVector [iobj] < 0.999)
						GeneratingPopulation [isol]->WeightVector [iobj] += WeightsChangeCoefficient;
				}
			}
		}
		this->RestoreFeasibility (GeneratingPopulation [isol]->WeightVector);
	}
	else { // if (GeneratingPopulationSize > 1)
		bool bIsWeightGreaterThanZero = false;
		int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
			if (Objectives [iobj].bActive) {
				// Draw at random value from [-0.1, 0.1]
				double WeightChange = (rand () % 10000) / 10000.0 * 0.2 - 0.1;
				GeneratingPopulation [isol]->WeightVector [iobj] += WeightChange;
				if (GeneratingPopulation [isol]->WeightVector [iobj] > 1)
					GeneratingPopulation [isol]->WeightVector [iobj] = 1;
				if (GeneratingPopulation [isol]->WeightVector [iobj] < 0)
					GeneratingPopulation [isol]->WeightVector [iobj] = 0;
				if (GeneratingPopulation [isol]->WeightVector [iobj] > 0)
					bIsWeightGreaterThanZero = true;
			}
		}
		if (!bIsWeightGreaterThanZero)
			GeneratingPopulation [isol]->WeightVector = GetRandomWeightVector ();
	}
	
	GeneratingPopulation [isol]->WeightVector.Normalize ();
	GeneratingPopulation [isol]->GeneratingSolution->WeightVector = 
		GeneratingPopulation [isol]->WeightVector;
	GeneratingPopulation [isol]->GeneratingSolution->WeightVector.Rescale (
		this->pNondominatedSet->ApproximateIdealPoint, this->pNondominatedSet->ApproximateNadirPoint);
}

template <class TProblemSolution>
void TPSA <TProblemSolution>::Run () {

	// Callback function
	this->Start ();

	this->pNondominatedSet->DeleteAll ();

	FindInitialPopulation ();

	this->iAcceptedMoves = 0;

	// Main loop
	this->Temperature = this->StartingTemperature;

	while (this->Temperature >= this->FinalTemperature) {
		this->iAcceptedMovesOnTemperatureLevel = 0;
		for (this->iMove = 0; this->iMove < this->MovesOnTemperatureLevel; this->iMove++) {
			int isol; for (isol = 0; isol < GeneratingPopulationSize; isol++) {
				UpdateWeightVector (isol);
				TProblemSolution* Solution = (TProblemSolution*)GeneratingPopulation [isol]->GeneratingSolution;

				TPoint ReferencePoint = *Solution;

				// Find and evaluate local move
				Solution->FindLocalMove ();
				double NewScalarizingFunctionValue = 
					Solution->ScalarizingFunction (ReferencePoint);

				// Calculate probability of acceptance
				double AcceptanceProbability;
				if (NewScalarizingFunctionValue <= 0)
					AcceptanceProbability = 1;
				else
					AcceptanceProbability = exp (-NewScalarizingFunctionValue / this->Temperature);

				// Accept the local move with calculated probability
				if ((rand () % 10000) / 10000.0 < AcceptanceProbability) {
					this->iAcceptedMovesOnTemperatureLevel++;
					this->iAcceptedMoves++;
					Solution->AcceptLocalMove ();
					//Update the nondominated set
					bool bAdded = this->pNondominatedSet->Update (*Solution);
					if (bAdded) {
						// Callback function
						this->NewNondominatedSolutionFound ();
					}
					// Callback function
					NewSolutionGenerated (*Solution);
				}
				else
					Solution->RejectLocalMove ();
			}
		}

		this->Temperature *= this->TemperatureDecreaseCoefficient;

		// Callback function
		this->TemperatureDecreased ();
	}

	int isol; for (isol = 0; isol < GeneratingPopulationSize; isol++) 
		delete GeneratingPopulation [isol];

	GeneratingPopulation.clear ();

	// Callback function
	this->End ();
}

template <class TProblemSolution>
void TDisplayPSA <TProblemSolution>::End () {
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
void TDisplayPSA <TProblemSolution>::NewNondominatedSolutionFound () {
}

template <class TProblemSolution>
void TDisplayPSA <TProblemSolution>::TemperatureDecreased() {
	cout << "T = " << this->Temperature << '\t';
	cout << this->iAcceptedMovesOnTemperatureLevel / (double)this->MovesOnTemperatureLevel;
	cout << '\n';
	cout << this->pNondominatedSet->iSetSize << '\t';
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		if (Objectives [iobj].bActive)
			cout << this->pNondominatedSet->ApproximateIdealPoint.ObjectiveValues [iobj] << '\t';
		if (Objectives [iobj].bActive)
			cout << this->pNondominatedSet->ApproximateNadirPoint.ObjectiveValues [iobj] << '\t';
	}
	cout << '\n';
	cout << '.';
}

#endif // !defined(AFX_PSA_H__C79F4BD1_DD06_11D4_832E_000000000000__INCLUDED_CPP)
