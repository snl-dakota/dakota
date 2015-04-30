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

#if !defined(AFX_SMOSA_H__C79F4BD1_DD06_11D4_832E_000000000000__INCLUDED_CPP)
#define AFX_SMOSA_H__C79F4BD1_DD06_11D4_832E_000000000000__INCLUDED_CPP

// SMOSA.cpp: implementation of the TSMOSA class.
//
//////////////////////////////////////////////////////////////////////

#include "smosa.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

template <class TProblemSolution>
TSMOSA <TProblemSolution>::TSMOSA()
{
	this->StartingTemperature = 1;
	this->FinalTemperature = 0.01;
	this->TemperatureDecreaseCoefficient = 0.9;
	this->MovesOnTemperatureLevel = 100;

}

template <class TProblemSolution>
void TSMOSA <TProblemSolution>::SetParameters (double StartingTemperature,
		double FinalTemperature,
		double TemperatureDecreaseCoefficient,
		int MovesOnTemperatureLevel) {

	this->StartingTemperature = StartingTemperature;
	this->FinalTemperature = FinalTemperature;
	this->TemperatureDecreaseCoefficient = TemperatureDecreaseCoefficient;
	this->MovesOnTemperatureLevel = MovesOnTemperatureLevel;

}

template <class TProblemSolution>
void TSMOSA <TProblemSolution>::FindInitialSolution () {
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
	
	GeneratingSolution = GeneratingPopulationElement;
}

template <class TProblemSolution>
void TSMOSA <TProblemSolution>::UpdateWeightVector () {
	
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		if (Objectives [iobj].bActive) {
			// Draw at random value from [-0.1, 0.1]
			double WeigtChange = (rand () % 10000) / 10000.0 * 0.2 - 0.1;
			GeneratingSolution->WeightVector [iobj] += WeigtChange;
			if (GeneratingSolution->WeightVector [iobj] > 1)
				GeneratingSolution->WeightVector [iobj] = 1;
			if (GeneratingSolution->WeightVector [iobj] < 0)
				GeneratingSolution->WeightVector [iobj] = 0;
		}
	}

//	GeneratingSolution->WeightVector = GetRandomWeightVector ();
	
	GeneratingSolution->WeightVector.Normalize ();
	GeneratingSolution->GeneratingSolution->WeightVector = 
		GeneratingSolution->WeightVector;
	GeneratingSolution->GeneratingSolution->WeightVector.Rescale (
		this->pNondominatedSet->ApproximateIdealPoint, this->pNondominatedSet->ApproximateNadirPoint);
}

template <class TProblemSolution>
void TSMOSA <TProblemSolution>::Run () {
	// Callback function
	this->Start ();

	this->pNondominatedSet->DeleteAll ();

	FindInitialSolution ();

	this->iAcceptedMoves = 0;

	// Main loop
	this->Temperature = this->StartingTemperature;
	while (this->Temperature >= this->FinalTemperature) {
		this->iAcceptedMovesOnTemperatureLevel = 0;
		for (this->iMove = 0; this->iMove < this->MovesOnTemperatureLevel; this->iMove++) {
			UpdateWeightVector ();

			TProblemSolution* Solution = (TProblemSolution*)GeneratingSolution->GeneratingSolution;
			
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
				this->NewSolutionGenerated (*Solution);
			}
			else
				Solution->RejectLocalMove ();
		}

		this->Temperature *= this->TemperatureDecreaseCoefficient;

		// Callback function
		this->TemperatureDecreased ();

	}

	delete GeneratingSolution;

	// Callback function
	this->End ();
}

template <class TProblemSolution>
void TDisplaySMOSA <TProblemSolution>::End () {
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
void TDisplaySMOSA <TProblemSolution>::NewNondominatedSolutionFound () {
}

template <class TProblemSolution>
void TDisplaySMOSA <TProblemSolution>::TemperatureDecreased() {
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

#endif // !defined(AFX_SMOSA_H__C79F4BD1_DD06_11D4_832E_000000000000__INCLUDED_CPP)
