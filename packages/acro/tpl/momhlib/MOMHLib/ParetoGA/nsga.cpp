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

#ifndef __NSGA_CPP_
#define __NSGA_CPP_

#include "tmotools.h"
#include "tindexedpoint.h"

#include "nsga.h"

template <class TProblemSolution>
TNSGA <TProblemSolution>::TNSGA()
{
	this->m_iPopulationSize = 100;
	this->m_iNumberOfGenerations = 1000;
	this->m_dMutationProbability = 0.2;
	this->m_dNeighbourhoodDistance = 0.5;
	this->m_bScalarize = 0;
}

template <class TProblemSolution>
void TNSGA <TProblemSolution>::SetParameters(int iPopulationSize, 
											 int iNumberOfGenerations,
											 double dMutationProbability,
											 double dNeighbourhoodDistance,
											 bool bScalarize) 
{
	this->m_iPopulationSize = iPopulationSize;
	this->m_iNumberOfGenerations = iNumberOfGenerations;
	this->m_dMutationProbability = dMutationProbability;
	this->m_dNeighbourhoodDistance = dNeighbourhoodDistance;
	this->m_bScalarize = bScalarize;
}

template <class TProblemSolution>
void TNSGA <TProblemSolution>::Run () 
{
vector<double>							dParetoRank;
vector<double>							dSharedValue;
vector<double>							dFitness;
vector<int>							iNeighbourhoodCount;
TQuadTree <TIndexedSolutionPoint>		oTmpQuadTree;

int							i = 0;
int							j = 0;
double						d = 0.0;
double						dCurrentRank = 0.0;
double						dFitnessSum = 0.0;
double						dFitnessMax = 0.0;
double						dFitnessMin = 0.0;
double						dDistance = 0.0;
int							iCurrentGeneration = 0;
int							iSolution1 = 0;
int							iSolution2 = 0;
TProblemSolution			*pParentA = NULL;
TProblemSolution			*pParentB = NULL;
TProblemSolution			*pOffspring = NULL;
TIndexedSolutionPoint		*pIndexedPoint = NULL;
bool						bExit = false;



	if (this->pNondominatedSet == NULL) 
	{
		return;
	}

	this->pNondominatedSet -> DeleteAll();

	this->Start();

	dParetoRank.resize(this->m_iPopulationSize);
	dSharedValue.resize(this->m_iPopulationSize);
	dFitness.resize(this->m_iPopulationSize);
	iNeighbourhoodCount.resize(this->m_iPopulationSize);

	// reserve memory for population
	m_oParentPopulation.resize(this->m_iPopulationSize);
	m_oChildrenPopulation.resize(this->m_iPopulationSize);
	m_oDummyPopulation.resize(this->m_iPopulationSize);

	// clean variables
	for(i = 0 ; i < this->m_iPopulationSize; i++)
	{
		m_oParentPopulation[i] = NULL;
		m_oChildrenPopulation[i] = NULL;
		m_oDummyPopulation[i] = NULL;
	}

	// Generate random population
	for(i = 0; i < this->m_iPopulationSize; i++) 
	{
		// generate random solution
		pParentA = new TProblemSolution();

		// store it in population
		m_oParentPopulation[i] = pParentA;

		// update nondominated set
		if (this->pNondominatedSet -> Update(*pParentA) == true) 
		{
			this->NewNondominatedSolutionFound();
		}

		NewSolutionGenerated(*(TProblemSolution *)pParentA);
	}

	// main algorithm loop
	iCurrentGeneration = 0;
    this->m_iGeneration = iCurrentGeneration;
	do
	{
		// Assign ranks to the solutions
		// Assign position numbers to solutions
		for(i = 0; i < this->m_iPopulationSize; i++) 
		{
			// create dummy population with simple constructor
			if (m_oParentPopulation[i] != NULL)
			{
				pIndexedPoint = new TIndexedSolutionPoint(*m_oParentPopulation[i]);
				pIndexedPoint -> m_iListPosition = i;
				m_oDummyPopulation[i] = pIndexedPoint;
			}
			else
			{
				m_oDummyPopulation[i] = NULL;
			}		


			dParetoRank[i] = 0.0;
			dSharedValue[i] = 0.0;
			iNeighbourhoodCount[i] = 0;

			dFitness[i] = 0.0;
		}

		// clear variables
		dFitnessMax = 1.0;
		dFitnessMin = 1.0;
		dFitnessSum = 0.0;

		// use quad tree to filter out solutions
		dCurrentRank = 1.0;
		do 
		{
			// set exit flag to true
			bExit = true;

			oTmpQuadTree.DeleteAll();
			// extract front
			for(i = 0; i < this->m_iPopulationSize; i++) 
			{
				// no empty slots (only in nondominated set)
				if (m_oDummyPopulation[i] == NULL) 
				{
					continue;
				}

				// choose only element not belong to any front
				// update temporary nondominated set
				if (dParetoRank[i] == 0) 
				{
					bExit = false;
					oTmpQuadTree.Update(*(m_oDummyPopulation[i]));
				}
			}
	
			if (m_bScalarize == true)
			{
				m_oParentPopulation.UpdateIdealNadir();
			}

			// compute shared fitness 
			for(i = 0; i < oTmpQuadTree.size(); i++) 
			{
				if (oTmpQuadTree[i] == NULL) 
				{
					continue;
				}
				
				pIndexedPoint = (TIndexedSolutionPoint *)oTmpQuadTree[i];
				iSolution1 = pIndexedPoint -> m_iListPosition;
				
				// solution won't be compared to oneself
				// so assign 1.0
				// shared value must be greater than 1
				// to ensure that fitness values will decrease
				dSharedValue[iSolution1] = 1.0;
				// assign rank (it is also flag for front determining)
				dParetoRank[iSolution1] = dCurrentRank;

				// compute shared fitness value
				for(j = 0; j < oTmpQuadTree.size(); j++) 
				{
					if ((oTmpQuadTree[j] == NULL) ||
						(j == i))
					{
						continue;
					}

					// get phenotypic distance between two solutions
					pIndexedPoint = (TIndexedSolutionPoint *)oTmpQuadTree[i];
					pParentA = (TProblemSolution *)m_oParentPopulation[pIndexedPoint->m_iListPosition];
					pIndexedPoint = (TIndexedSolutionPoint *)oTmpQuadTree[j];
					pParentB = (TProblemSolution *)m_oParentPopulation[pIndexedPoint->m_iListPosition];

					// we needn't scalarization because distance is measured in parameter 
					dDistance = (*pParentA).ObjectiveValuesDistance(*pParentB, 
																	m_bScalarize, 
																	m_oParentPopulation.ApproximateIdealPoint.ObjectiveValues, 
																	m_oParentPopulation.ApproximateNadirPoint.ObjectiveValues);

					if (dDistance < m_dNeighbourhoodDistance)
					{
						// see Deb "Multiobjective Opt. Using NSGA"
						// dDistance < m_dNeighbourhoodDistance
						d = dDistance / m_dNeighbourhoodDistance;
						dSharedValue[iSolution1] += 1.0 - d * d;

						iNeighbourhoodCount[iSolution1] += 1;
					} 
					else 
					{
						// only to show you second variant
						dSharedValue[iSolution1] += 0.0;
					}
				}

				// share solution
				dFitness[iSolution1] = dFitnessMax / dSharedValue[iSolution1];

				if (dFitness[iSolution1] < dFitnessMin) 
				{
					dFitnessMin = dFitness[iSolution1];
				}

				dFitnessSum += dFitness[iSolution1];
			}

			dCurrentRank +=  1.0;
			dFitnessMax = dFitnessMin;
			// next front must have less fitness 
			dFitnessMax -= 0.01 * dFitnessMax;
		}
		while(bExit == false);

		// sort solution according to fitness	
		TMOTools::QuickSortSolutionUsingFitnessMax(m_oParentPopulation, dFitness);

		for(i = 0; i < this->m_iPopulationSize; i++)
		{	
			// recombination
			iSolution1 = TMOTools::GetRandomSolutionUsingFitnessMax(m_oParentPopulation, dFitness, dFitnessSum);
			iSolution2 = TMOTools::GetRandomSolutionUsingFitnessMax(m_oParentPopulation, dFitness, dFitnessSum);

			pOffspring = new TProblemSolution(*(TProblemSolution *)m_oParentPopulation[iSolution1], *(TProblemSolution *)m_oParentPopulation[iSolution2]);

			// mutation
			if (TMOTools::DoMutate(this->m_dMutationProbability) == true) 
			{
				pOffspring->Mutate();
			}

			// update nondominated set
			if (this->pNondominatedSet -> Update(*pOffspring) == true) 
			{
				this->NewNondominatedSolutionFound();
			}

			// store offspring on new solution pull
			m_oChildrenPopulation[i] = pOffspring;
			pOffspring = NULL;

			NewSolutionGenerated(*(TProblemSolution *)m_oChildrenPopulation[i]);
		}

		// erase old population
		// move new population on place of previous
		for(i = 0; i < this->m_iPopulationSize; i++)
		{
			// clear dummy population
			if (m_oDummyPopulation[i] != NULL) 
			{
				delete m_oDummyPopulation[i];
				m_oDummyPopulation[i] = NULL;
			}


			if (m_oParentPopulation[i] != NULL) 
			{
				delete (TProblemSolution *)m_oParentPopulation[i];
				m_oParentPopulation[i] = m_oChildrenPopulation[i];
				m_oChildrenPopulation[i] = NULL;
			}
			else
			{
				m_oParentPopulation[i] = NULL;
				m_oChildrenPopulation[i] = NULL;
			}
		}
		
		iCurrentGeneration += 1;
        this->m_iGeneration = iCurrentGeneration;
    } 
    while((this->bStop == false) && 
          ((this->bOnlineOfflineEvaluating == true) || (this->m_iGeneration < this->m_iNumberOfGenerations)));


	// end callback
	this->End();
	
	// Cleanup object
	// Remove current population
	for(i = 0; i < this->m_iPopulationSize; i++) 
	{
		delete (TProblemSolution *)m_oParentPopulation[i];
		m_oParentPopulation[i] = NULL;
	}
}

template <class TProblemSolution>
void TDisplayNSGA <TProblemSolution>::End() 
{
	cout << "Optimization finished\n";
	cout << this->pNondominatedSet->iSetSize << '\t';
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) 
	{
		if (Objectives [iobj].bActive) 
		{
			cout << this->pNondominatedSet->ApproximateIdealPoint.ObjectiveValues [iobj] << '\t';
		}

		if (Objectives [iobj].bActive) 
		{
			cout << this->pNondominatedSet->ApproximateNadirPoint.ObjectiveValues [iobj] << '\t';
		}
	}
	cout << '\n';
}

template <class TProblemSolution>
void TDisplayNSGA <TProblemSolution>::NewNondominatedSolutionFound() 
{
	cout << this->pNondominatedSet->iSetSize << '\t';
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) 
	{
		if (Objectives [iobj].bActive) 
		{
			cout << this->pNondominatedSet->ApproximateIdealPoint.ObjectiveValues [iobj] << '\t';
		}

		if (Objectives [iobj].bActive) 
		{
			cout << this->pNondominatedSet->ApproximateNadirPoint.ObjectiveValues [iobj] << '\t';
		}
	}
	cout << '\n';
}

template <class TProblemSolution>
void TDisplayNSGA <TProblemSolution>::NewGenerationFound() 
{
 	cout << "Generation: " << this->m_iGeneration + 1 << "\n";
}

#endif
