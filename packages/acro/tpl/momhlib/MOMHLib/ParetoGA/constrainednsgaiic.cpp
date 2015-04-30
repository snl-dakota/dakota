/*
Multiple Objective MetaHeuristics Library in C++ MOMHLib++
Copyright (C) 2001 Andrzej Jaszkiewicz, Radoslaw Ziembinski (radekz@free.alpha.net.pl)

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


#ifndef __CONSTRAINED_NSGAIIC_CPP_
#define __CONSTRAINED_NSGAIIC_CPP_

#include "tmotools.h"

#include "constrainednsgaiic.h"

template <class TProblemSolution>
void TConstrainedNSGAIIC <TProblemSolution>::Run() 
{
vector<IntVector>						iF;
vector<double>							dParentFitness;	
vector<double>							dDistance;
vector<double>							dParentDistance;
vector<IntVector>						iSortTable;

int						i = 0;
int						j = 0;
int						k = 0;
int						iPopulation = 0;
int						iNumberToMove = 0;
int						iFrontSize = 0;
int						iFront = 0;
double					d = 0.0;
double					dDifference;
int						iCurrentGeneration = 0;
int						iSolution = 0;
int						iSolution1 = 0;
int						iSolution2 = 0;
TProblemSolution		*pParentA = NULL;
TProblemSolution		*pParentB = NULL;
TProblemSolution		*pOffspring = NULL;
bool					bExit = false;

	if (pNondominatedSet == NULL) 
	{
		return;
	}

	pNondominatedSet -> DeleteAll();

	Start();

	dParentFitness.resize(m_iPopulationSize);
	dParentDistance.resize(m_iPopulationSize);
	dDistance.resize(m_iDoubledPopulationSize);
	
	// reserve memory for population
	m_oPPopulation.resize(m_iPopulationSize);
	m_oQPopulation.resize(m_iPopulationSize);
	m_oRPopulation.resize(m_iDoubledPopulationSize);

	// Prepare initial P and Q populations
	
	// Generate random P population
	for(i = 0; i < m_iPopulationSize; i++) 
	{
		// generate random solution
		pParentA = new TProblemSolution();

		// store it in population
		m_oPPopulation[i] = pParentA;

		// update nondominated set
		if (pNondominatedSet -> Update(*pParentA)) 
		{
			NewNondominatedSolutionFound();
		}

		NewSolutionGenerated(*(TProblemSolution *)pParentA);
	}

	// Fast nondominated sorting algorithm
	TMOTools::ConstrainedFastNondominatedSort(m_oPPopulation, iF);

	// clear fitness
	for(i = 0; i < m_iPopulationSize; i++)
	{
		dParentFitness[i] = 0.0;
	}

	// assign fitness from front list
	for(i = 0; i < iF.size(); i++) 
	{
		for(j = 0; j < iF[i].size(); j++)
		{
			iSolution = iF[i][j];
			dParentFitness[iSolution] = i + 1.0; 
		}
	}

	// debug
	for(i = 0; i < m_iPopulationSize; i++)
	{
		assert(dParentFitness[i] > 0.0);
	}

	// create Q population
	for(i = 0; i < m_iPopulationSize; i++)
	{
		iSolution1 = TMOTools::GetSolutionUsingBinaryTournamentMin(m_oPPopulation, dParentFitness);
		iSolution2 = TMOTools::GetSolutionUsingBinaryTournamentMin(m_oPPopulation, dParentFitness);
	
		// recombination		
		pOffspring = new TProblemSolution(*(TProblemSolution *)m_oPPopulation[iSolution1], *(TProblemSolution *)m_oPPopulation[iSolution2]);

		// mutation
		if (TMOTools::DoMutate(m_dMutationProbability) == true) 
		{
			pOffspring->Mutate();
		}

		// update nondominated set
		if (pNondominatedSet -> Update(*pOffspring) == true) 
		{
			NewNondominatedSolutionFound();
		}

		// store offspring on new solution pull
		m_oQPopulation[i] = pOffspring;
		pOffspring = NULL;

		NewSolutionGenerated(*(TProblemSolution *)m_oQPopulation[i]);	
	}

	// main algorithm loop
	iCurrentGeneration = 0;
    m_iGeneration = iCurrentGeneration;

	do
	{
		// join population P and Q to R
		for(i = 0; i < m_iPopulationSize; i++)
		{
			m_oRPopulation[i] = m_oPPopulation[i];
			m_oRPopulation[i + m_iPopulationSize] = m_oQPopulation[i];

			m_oPPopulation[i] = NULL;
			m_oQPopulation[i] = NULL;

			// clean variables
			dDistance[i] = 0.0;
			dDistance[i + m_iPopulationSize] = 0.0;
		}
		
		// fast nondominated sort on R population
		TMOTools::FastNondominatedSort(m_oRPopulation, iF);

		// compute aproximation of ideal nadir on R population
		// needed for scalarization
		if (m_bScalarize == true) 
		{
			m_oRPopulation.UpdateIdealNadir();
		}

		for(iFront = 0; iFront < iF.size(); iFront++)
		{
			// last front is empty ommit empty fronts
			if (iF[iFront].size() == 0)
			{
				continue;
			}

			// sort solutions by objective
			// to compute crowding distance
			iSortTable.resize(0);

			iFrontSize = iF[iFront].size();
			TMOTools::QuickSortByObjectivesMax(m_oRPopulation, iF[iFront], iSortTable);

			for(i = 0; i < NumberOfObjectives; i++) 
			{	
				// ommit objectives that not active
				if (Objectives[i].bActive == false) 
				{
					continue;
				}

				iSolution = iSortTable[i][0];
				dDistance[iSolution] = -1.0; // infinity

				iSolution = iSortTable[i][iFrontSize - 1];
				dDistance[iSolution] = -1.0; // infinity		

				for(j = 1; j < iFrontSize - 2; j++) 
				{
					iSolution = iSortTable[i][j];
				
					if (dDistance[iSolution] >= 0.0) 
					{
						iSolution1 = iSortTable[i][j - 1];
						iSolution2 = iSortTable[i][j + 1];
					
						d = m_oRPopulation[iSolution1]->ObjectiveValues[i] - m_oRPopulation[iSolution2]->ObjectiveValues[i];
					
						// optional scalarization 
						if (m_bScalarize == true)
						{
							dDifference = m_oRPopulation.ApproximateIdealPoint.ObjectiveValues[i] - m_oRPopulation.ApproximateNadirPoint.ObjectiveValues[i];
							if (dDifference != 0.0)
							{
								d = d / dDifference;
							}	 
							else
							{
								d = 0;
							}
						}
					
						dDistance[iSolution] += abs(d);
					}
				}
			}

			// sort solutions in the front from greatest distance
			// to lower
			TMOTools::QuickSortByValueMax(m_oRPopulation, dDistance, iF[iFront]);
		}

		// create new parent P population from till of R population
		// this is controlled aproach 
		// solutions are selected not from first front but from all
		iPopulation = 0;
		iNumberToMove = 0;

		for(iFront = 0; (iFront < iF.size()) && (iPopulation < m_iPopulationSize); iFront++)
		{
			if (iF[iFront].size() <= 0)
			{
				continue;
			}

			// geometrical sequence
			d = 1.0 - m_dPopulationGeometricalFactor;
			d = d / (1.0 - pow(m_dPopulationGeometricalFactor, iF.size() - 2.0));
			d = floor(m_oRPopulation.size()*d*pow(m_dPopulationGeometricalFactor, iFront));

			iNumberToMove += (int)d;

			if (iNumberToMove >= iF[iFront].size())
			{
				// if number of slots is greater then move whole population from front
				for(j = 0; (j < iF[iFront].size()) && (iPopulation < m_iPopulationSize); j++, iPopulation ++)
				{
					iSolution = iF[iFront][j];

					assert(m_oRPopulation[iSolution] != NULL);

					m_oPPopulation[iPopulation] = m_oRPopulation[iSolution];
					dParentFitness[iPopulation] = iFront + 1.0;
					dParentDistance[iPopulation] = dDistance[iSolution];

					m_oRPopulation[iSolution] = NULL;
				}

				// reduce number of empty slots
				iNumberToMove -= iF[iFront].size();
			}
			else
			{
				// use tournament selection to choose the best solution
				for(j = 0; (j < iNumberToMove) && (iPopulation < m_iPopulationSize); j++, iPopulation ++)
				{
					iSolution1 = TMOTools::GetRandomIndex(m_oRPopulation, iF[iFront]);	
					iSolution2 = TMOTools::GetRandomIndex(m_oRPopulation, iF[iFront]);

					assert(iSolution1 != -1);
					assert(iSolution2 != -1);

					if (dDistance[iSolution1] > dDistance[iSolution2])
					{
						assert(m_oRPopulation[iSolution1] != NULL);
						
						m_oPPopulation[iPopulation] = m_oRPopulation[iSolution1];
						dParentFitness[iPopulation] = iFront + 1.0;
						dParentDistance[iPopulation] = dDistance[iSolution1];

						m_oRPopulation[iSolution1] = NULL;
					} 
					else
					{
						assert(m_oRPopulation[iSolution2] != NULL);

						m_oPPopulation[iPopulation] = m_oRPopulation[iSolution2];
						dParentFitness[iPopulation] = iFront + 1.0;
						dParentDistance[iPopulation] = dDistance[iSolution2];

						m_oRPopulation[iSolution2] = NULL;
					}
				}
				
				iNumberToMove = 0;
			}
		}

		// numerical errors can cause that geometrical sequence
		// doesn't cover whole  parent population
		if (iPopulation < m_iPopulationSize)
		{
			for(iFront = 0, k =0; (iFront < iF.size()) && (iPopulation < m_iPopulationSize); iFront++)
			{
				for(j = 0; (j < iF[iFront].size()) && (iPopulation < m_iPopulationSize); j++, iPopulation++)
				{
					iSolution = iF[iFront][j];
					if (m_oRPopulation[iSolution] != NULL)
					{
						// move the first meet individual
						m_oPPopulation[iPopulation] = m_oRPopulation[iSolution];
						dParentFitness[iPopulation] = iFront + 1.0;
						dParentDistance[iPopulation] = dDistance[iSolution];

						m_oRPopulation[iSolution] = NULL;
					}
				}
			}
		}	

		// release rest of R population
		for(i = 0; i < m_iDoubledPopulationSize; i++)
		{
			if (m_oRPopulation[i] != NULL)
			{
				delete (TProblemSolution *)m_oRPopulation[i];
				m_oRPopulation[i] = NULL;
			}
		}
				
		// create Q population based on nisched operator
		for(i = 0; i < m_iPopulationSize; i++)
		{
			iSolution1 = TMOTools::GetSolutionUsingBinaryTournamentNischedMin(m_oPPopulation, dParentFitness, dParentDistance);
			iSolution2 = TMOTools::GetSolutionUsingBinaryTournamentNischedMin(m_oPPopulation, dParentFitness, dParentDistance);
	
			pParentA = (TProblemSolution *)m_oPPopulation[iSolution1];
			pParentB = (TProblemSolution *)m_oPPopulation[iSolution2];

			// recombination		
			pOffspring = new TProblemSolution(*pParentA, *pParentB);

			// mutation
			if (TMOTools::DoMutate(m_dMutationProbability) == true) 
			{
				pOffspring->Mutate();
			}

			// update nondominated set
			if (pNondominatedSet -> Update(*pOffspring) == true) 
			{
				NewNondominatedSolutionFound();
			}

			// store offspring on new solution pull
			m_oQPopulation[i] = pOffspring;
			pOffspring = NULL;

			NewSolutionGenerated(*(TProblemSolution *)m_oQPopulation[i]);	
		}
		
		iCurrentGeneration += 1;
        m_iGeneration = iCurrentGeneration;
	}
    while((bStop == false) && 
          ((bOnlineOfflineEvaluating == true) || (m_iGeneration < m_iNumberOfGenerations)));
	
	// end callback
	End();

	// Cleanup object
	// Remove current population
	for(i = 0; i < m_iPopulationSize; i++) 
	{
		if (m_oPPopulation[i] != NULL) 
		{
			delete (TProblemSolution *)m_oPPopulation[i];
			m_oPPopulation[i] = NULL;
		}

		if (m_oQPopulation[i] != NULL) 
		{
			delete (TProblemSolution *)m_oQPopulation[i];
			m_oQPopulation[i] = NULL;
		}
	}
}

template <class TProblemSolution>
void TDisplayConstrainedNSGAIIC <TProblemSolution>::End() 
{
	cout << "Optimization finished\n";
	cout << pNondominatedSet->iSetSize << '\t';
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) 
	{
		if (Objectives [iobj].bActive)
		{
			cout << pNondominatedSet->ApproximateIdealPoint.ObjectiveValues [iobj] << '\t';
		}

		if (Objectives [iobj].bActive) 
		{
			cout << pNondominatedSet->ApproximateNadirPoint.ObjectiveValues [iobj] << '\t';
		}
	}
	cout << '\n';
}

template <class TProblemSolution>
void TDisplayConstrainedNSGAIIC <TProblemSolution>::NewNondominatedSolutionFound() 
{
	cout << pNondominatedSet->iSetSize << '\t';
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) 
	{
		if (Objectives [iobj].bActive) 
		{
			cout << pNondominatedSet->ApproximateIdealPoint.ObjectiveValues [iobj] << '\t';
		}

		if (Objectives [iobj].bActive) 
		{
			cout << pNondominatedSet->ApproximateNadirPoint.ObjectiveValues [iobj] << '\t';
		}
	}
	cout << '\n';
}

template <class TProblemSolution>
void TDisplayConstrainedNSGAIIC <TProblemSolution>::NewGenerationFound() 
{
 	cout << "Generation: " << m_iGeneration + 1 << "\n";
}

#endif
