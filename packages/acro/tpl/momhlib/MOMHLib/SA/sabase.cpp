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

// SABase.cpp: implementation of the TSABase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SABASE_CPP__5C59D552_391A_4BC3_ACDA_77CD85A0725C__INCLUDED_)
#define AFX_SABASE_CPP__5C59D552_391A_4BC3_ACDA_77CD85A0725C__INCLUDED_

#include "sabase.h"

template <class TProblemSolution>
TProblemSolution TSABase <TProblemSolution>::SingleObjectiveOptimization (TPoint ReferencePoint, 
	TScalarizingFunctionType ScalarizingFunctionType, 
	TWeightVector& WeightVector,
	bool bUpdateNondominatedSet) {

	// Construct the initial solution
	TProblemSolution* Solution = new TProblemSolution;
	Solution->WeightVector = WeightVector;
	Solution->SetScalarizingFunctionType (ScalarizingFunctionType);

	TProblemSolution BestSolution (*Solution);

	double CurrentScalarizingFunctionValue = 
		Solution->ScalarizingFunction (ReferencePoint);

	// Main loop
	Temperature = StartingTemperature;
	while (Temperature >= FinalTemperature) {
		iAcceptedMovesOnTemperatureLevel = 0;
		// Loop on the curent temeprature level
		for (iMove = 0; iMove < MovesOnTemperatureLevel; iMove++) {

			// Find and evaluate local move
			Solution->FindLocalMove ();
			double NewScalarizingFunctionValue = 
				Solution->ScalarizingFunction (ReferencePoint);
			
			// Clculate probability of acceptance
			double AcceptanceProbability;
			if (NewScalarizingFunctionValue <= 0)
				AcceptanceProbability = 1;
			else
				AcceptanceProbability = exp (CurrentScalarizingFunctionValue - NewScalarizingFunctionValue / Temperature);
			
			// Accept the local move with calculated probability
			if ((rand () % 10000) / 10000.0 < AcceptanceProbability) {
				iAcceptedMovesOnTemperatureLevel++;
				iAcceptedMoves++;
				Solution->AcceptLocalMove ();
				if (bUpdateNondominatedSet) {
					//Update the nondominated set
					bool bAdded = this->pNondominatedSet->Update (*Solution);
					if (bAdded) {
						// Callback function
						this->NewNondominatedSolutionFound ();
					}
				}
				if (BestSolution.ScalarizingFunction (ReferencePoint) >
					Solution->ScalarizingFunction (ReferencePoint))
					BestSolution = *Solution;
				// Callback function
				NewSolutionGenerated (*Solution);
			}
			else
				Solution->RejectLocalMove ();
		}

		Temperature *= TemperatureDecreaseCoefficient;

		// Callback function
		TemperatureDecreased ();
	}

	delete Solution;

	return BestSolution;
}

#endif // !defined(AFX_SABASE_CPP__5C59D552_391A_4BC3_ACDA_77CD85A0725C__INCLUDED_)
