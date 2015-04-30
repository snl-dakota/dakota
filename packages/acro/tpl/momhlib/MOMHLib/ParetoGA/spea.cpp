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

#ifndef __SPEA_CPP_
#define __SPEA_CPP_

#include "tmotools.h"

#include "spea.h"

template <class TProblemSolution>
TSPEA <TProblemSolution>::TSPEA()
{
	this->m_iPopulationSize = 100;
	m_iNondominatedPopulationSize = 10000;
	m_iClusteringLevel = 20000;
	this->m_iNumberOfGenerations = 1000;
	this->m_dMutationProbability = 0.2;
	m_bScalarize = false;

	this->m_iGeneration = 0;
}

template <class TProblemSolution>
void TSPEA <TProblemSolution>::SetParameters(int iPopulationSize, 
											 int iNumberOfGenerations,
											 double dMutationProbability,
											 int iNondominatedPopulationSize,
											 int iClusteringLevel,
											 bool bScalarize)
{
	this->m_iPopulationSize = iPopulationSize;
	this->m_iNumberOfGenerations = iNumberOfGenerations;
	this->m_dMutationProbability = dMutationProbability;
	this->m_iNondominatedPopulationSize = iNondominatedPopulationSize;
	this->m_iClusteringLevel = iClusteringLevel;
	this->m_bScalarize = bScalarize;

	this->m_iGeneration = 0;
}

template <class TProblemSolution>
void TSPEA <TProblemSolution>::Run () 
{
vector<IntVector>						iDominating;
vector<double>							dNondominatedStrength;	
TSolutionsSet							oMatingPull;
vector<double>							dFitness;
vector<bool>							bInNondominatedSet;


int										i = 0;
int										j = 0;
int										k = 0;
double									d = 0.0;
int										iCurrentGeneration = 0;
int										iSolution = 0;
int										iSolution1 = 0;
int										iSolution2 = 0;
TProblemSolution						*pSolutionA = NULL;
TProblemSolution						*pSolutionB = NULL;
TProblemSolution						*pOffspring = NULL;
bool									bExit = false;

	if (this->pNondominatedSet == NULL) 
	{
		return;
	}

	this->pNondominatedSet -> DeleteAll();

	this->Start();

	// reserve memory for population
	m_oPopulation.resize(this->m_iPopulationSize);
	iDominating.resize(this->m_iPopulationSize);
	
	// Generate random Population and update nondominated set
	for(i = 0; i < this->m_iPopulationSize; i++) 
	{
		// generate random solution
		pSolutionA = new TProblemSolution();

		// store it in population
		m_oPopulation[i] = pSolutionA;

		// update nondominated set
		if (this->pNondominatedSet -> Update(*pSolutionA)) 
		{
			this->NewNondominatedSolutionFound();
		}

		NewSolutionGenerated(*pSolutionA);
	}


	// main algorithm loop
	iCurrentGeneration = 0;
    this->m_iGeneration = iCurrentGeneration;

	do
	{
		// clear mating pull
		oMatingPull.resize(0);
		bInNondominatedSet.resize(0);
		dFitness.resize(0);

		// prepare population strenght variables
		for(i = 0; i < this->m_iPopulationSize; i++)
		{
			iDominating[i].resize(0);
		}

		dNondominatedStrength.resize(this->pNondominatedSet -> iSetSize);
		
		for(i = 0, k = 0; (i < this->pNondominatedSet -> size()) && (k < this->pNondominatedSet -> iSetSize); i++) 
		{
			// if empty slot ommit
			pSolutionA = (TProblemSolution *)(*this->pNondominatedSet)[i];
			if (pSolutionA == NULL) 
			{
				continue;
			}

			// compute strength
			d = 0.0;
			for(j = 0; j < this->m_iPopulationSize; j++)
			{
				pSolutionB = (TProblemSolution *)m_oPopulation[j];
				if ((*pSolutionA).Compare(*pSolutionB) == _Dominating)
				{
					d += 1.0;
					iDominating[j].push_back(k);
				}
			}
			dNondominatedStrength[k] = d / (this->m_iPopulationSize + 1.0);
			
			// put this solution to mating pull
			oMatingPull.push_back(pSolutionA);
			bInNondominatedSet.push_back(true);
			dFitness.push_back(dNondominatedStrength[k]);

			// next solution
			k += 1;
		}

		// compute population fitness as sum of 
		// beware on duplicated solutions in nondominated set and population
		for(i = 0; i < this->m_iPopulationSize; i++)
		{
			// this solution is nonominated
			if (iDominating[i].size() == 0)
			{
				// we have copy of this solution in nondominated set
				delete (TProblemSolution *)m_oPopulation[i];
				m_oPopulation[i] = NULL;
			}
			else
			{
				// sum up strenght of parent nondominated solutions 
				// dominated solution must be worse than nondominated
				d = 1.0;
				for(j = 0; j < iDominating[i].size(); j++)
				{
					k = iDominating[i][j];
					d += dNondominatedStrength[k];
				}

				// add solution to mating pull
				oMatingPull.push_back(m_oPopulation[i]);
				bInNondominatedSet.push_back(false);
				dFitness.push_back(d);

				m_oPopulation[i] = NULL;
			}
		}

		// create Q population based on nisched operator
		for(i = 0; i < this->m_iPopulationSize; i++)
		{
			iSolution1 = TMOTools::GetSolutionUsingBinaryTournamentMin(oMatingPull, dFitness);
			iSolution2 = TMOTools::GetSolutionUsingBinaryTournamentMin(oMatingPull, dFitness);
	
			// recombination		
			pOffspring = new TProblemSolution(*(TProblemSolution *)oMatingPull[iSolution1], *(TProblemSolution *)oMatingPull[iSolution2]);

			// mutation
			if (TMOTools::DoMutate(this->m_dMutationProbability) == true) 
			{
				pOffspring->Mutate();
			}

			// store offspring on new solution pull
			// update nondominated set later (we have copy in mating pull)
			m_oPopulation[i] = pOffspring;
			pOffspring = NULL;

			NewSolutionGenerated(*(TProblemSolution *)(m_oPopulation[i]));	
		}
		
		// remove old population from mating pull that is not in nondominated set
		for(i = 0; i < oMatingPull.size(); i++)
		{		
			// delete rest of old population
			if (bInNondominatedSet[i] == false) 
			{
				if (oMatingPull[i] != NULL) 
				{
					delete (TProblemSolution *)oMatingPull[i];
				}
			}
			oMatingPull[i] = NULL;
		}

		// update nondominated solutions
		for(i = 0; i < this->m_iPopulationSize; i++)
		{
			if (this->pNondominatedSet -> Update(*m_oPopulation[i]) == true) 
			{
				this->NewNondominatedSolutionFound();
			}
		}

		if (m_iClusteringLevel < this->pNondominatedSet -> iSetSize)
		{
			this->pNondominatedSet -> ReduceSetByClustering(m_iNondominatedPopulationSize, m_bScalarize);
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
		if (m_oPopulation[i] != NULL) 
		{
			delete (TProblemSolution *)m_oPopulation[i];
			m_oPopulation[i] = NULL;
		}
	}
}

template <class TProblemSolution>
void TDisplaySPEA <TProblemSolution>::End() 
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
void TDisplaySPEA <TProblemSolution>::NewNondominatedSolutionFound() 
{
	cout << "NDSetSize :" << this->pNondominatedSet->iSetSize << '\n';
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) 
	{
		if (Objectives [iobj].bActive) 
		{
			cout << "Objective :" << iobj << "\t" << "Ideal Nadir: ";
			cout << this->pNondominatedSet->ApproximateIdealPoint.ObjectiveValues [iobj] << '\t';
			cout << this->pNondominatedSet->ApproximateNadirPoint.ObjectiveValues [iobj] << '\n';
		}
	}
	cout << "\n";
}

template <class TProblemSolution>
void TDisplaySPEA <TProblemSolution>::NewGenerationFound() 
{
 	cout << "Generation: " << this->m_iGeneration + 1 << "\n";
}

#endif
