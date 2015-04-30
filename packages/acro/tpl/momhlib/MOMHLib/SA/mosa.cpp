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

#if !defined(AFX_MOSA_H__C79F4BD1_DD06_11D4_832E_000000000000__INCLUDED_CPP)
#define AFX_MOSA_H__C79F4BD1_DD06_11D4_832E_000000000000__INCLUDED_CPP

// MOSA.cpp: implementation of the TMOSA class.
//
//////////////////////////////////////////////////////////////////////

#include "mosa.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

template <class TProblemSolution>
TMOSA <TProblemSolution>::TMOSA()
{
	this->StartingTemperature = 1;
	this->FinalTemperature = 0.01;
	this->TemperatureDecreaseCoefficient = 0.9;
	this->MovesOnTemperatureLevel = 100;
	this->GeneratingPopulationSize = 10;

}

template <class TProblemSolution>
void TMOSA <TProblemSolution>::SetParameters (double StartingTemperature,
		double FinalTemperature,
		double TemperatureDecreaseCoefficient,
		int MovesOnTemperatureLevel,
		int GeneratingPopulationSize) {

	this->StartingTemperature = StartingTemperature;
	this->FinalTemperature = FinalTemperature;
	this->TemperatureDecreaseCoefficient = TemperatureDecreaseCoefficient;
	this->MovesOnTemperatureLevel = MovesOnTemperatureLevel;
	this->GeneratingPopulationSize = GeneratingPopulationSize;

}

template <class TProblemSolution>
void TMOSA <TProblemSolution>::FindInitialPopulation () {
	int i; for (i = 0; i < GeneratingPopulationSize; i++) {
		TGeneratingPopulationElement* GeneratingPopulationElement = 
			new TGeneratingPopulationElement;
		GeneratingPopulationElement->GeneratingSolution = new TProblemSolution;
		GeneratingPopulationElement->GeneratingSolution->SetScalarizingFunctionType (_Linear);
	
		bool bAdded = this->pNondominatedSet->Update ((TProblemSolution&)*(GeneratingPopulationElement->GeneratingSolution));
		if (bAdded) {
			this->NewNondominatedSolutionFound ();
		}

		GeneratingPopulation.push_back (GeneratingPopulationElement);
	}
	TWeightsSet RandomWeightsSet;

	// Generate 500 random weight vectors
	for (i = 0; i < 500; i++) {
		RandomWeightsSet.push_back (GetRandomWeightVector ());
	}

	// Put the first random weight vector
	GeneratingPopulation [0]->WeightVector =
		GetRandomWeightVector ();

	// Put GeneratingPopulationSize - 1 weight vectors
	int j; for (j = 1; j < GeneratingPopulationSize; j++) {
		int iFurthestWeightVector;
		double dFurthestDistance;
		int i; for (i = 0; i < RandomWeightsSet.size (); i++) {
			double dClosestDistance = RandomWeightsSet [i].Distance (GeneratingPopulation [0]->WeightVector);
			int l; for (l = 1; l < j; l++) {
				double dDistance = RandomWeightsSet [i].Distance (GeneratingPopulation [l]->WeightVector);
				if (dClosestDistance > dDistance) {
					dClosestDistance = dDistance;
				}
			}
			if (i == 0) {
				iFurthestWeightVector = 0;
				dFurthestDistance = dClosestDistance;
			}
			else if (dFurthestDistance < dClosestDistance) {
				dFurthestDistance = dClosestDistance;
				iFurthestWeightVector = i;
			}
		}
		GeneratingPopulation [j]->WeightVector =
			RandomWeightsSet [iFurthestWeightVector];
	}

}

template <class TProblemSolution>
void TMOSA <TProblemSolution>::Run () {
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
				TProblemSolution* Solution = (TProblemSolution*)GeneratingPopulation [isol]->GeneratingSolution;

				Solution->WeightVector = 
					GeneratingPopulation [isol]->WeightVector;
				Solution->WeightVector.Rescale (
					this->pNondominatedSet->ApproximateIdealPoint, this->pNondominatedSet->ApproximateNadirPoint);

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
void TDisplayMOSA <TProblemSolution>::End () {
	cout << "Optimization finished\n";
	cout << this->pNondominatedSet->iSizeSet << '\t';
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		if (Objectives [iobj].bActive)
			cout << this->pNondominatedSet->ApproximateIdealPoint.ObjectiveValues [iobj] << '\t';
		if (Objectives [iobj].bActive)
			cout << this->pNondominatedSet->ApproximateNadirPoint.ObjectiveValues [iobj] << '\t';
	}
	cout << '\n';
}

template <class TProblemSolution>
void TDisplayMOSA <TProblemSolution>::NewNondominatedSolutionFound () {
}

template <class TProblemSolution>
void TDisplayMOSA <TProblemSolution>::TemperatureDecreased() {
	cout << "T = " << this->Temperature << '\t';
	cout << this->iAcceptedMovesOnTemperatureLevel / (double)this->MovesOnTemperatureLevel;
	cout << '\n';
	cout << this->NondominatedSet->iSetSize << '\t';
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		if (Objectives [iobj].bActive)
			cout << this->pNondominatedSet->ApproximateIdealPoint.ObjectiveValues [iobj] << '\t';
		if (Objectives [iobj].bActive)
			cout << this->pNondominatedSet->ApproximateNadirPoint.ObjectiveValues [iobj] << '\t';
	}
	cout << '\n';
	cout << '.';
}

#endif // !defined(AFX_MOSA_H__C79F4BD1_DD06_11D4_832E_000000000000__INCLUDED_CPP)
