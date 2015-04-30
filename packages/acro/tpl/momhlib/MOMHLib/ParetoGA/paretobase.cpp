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

#ifndef __PARETOBASE_CPP_
#define __PARETOBASE_CPP_

#include "paretobase.h"
#include "tmotools.h"

template <class TProblemSolution>
TProblemSolution TParetoBase <TProblemSolution> :: SingleObjectiveOptimization(TPoint ReferencePoint,
		                                                                       TScalarizingFunctionType ScalarizingFunctionType,
		                                                                       TWeightVector& WeightVector,
		                                                                       bool bUpdateNondominatedSet)
{
const int                               iNoImprovementLevel = 10;
int							            iCurrentGeneration = 0;
TSolutionsSet				            oChildrenPopulation;
TSolutionsSet				            oParentPopulation; 
DoubleVector                            dFitness;
TProblemSolution                        oTheBestSolution;
double                                  dTheBestFitness = 0.0;
int							            i = 0;
int							            j = 0;
int							            iSolution1 = 0;
int							            iSolution2 = 0;
TProblemSolution			            *pSolution = NULL;

	// allocate fitness
    dFitness.resize(m_iPopulationSize);
	
    // populacje
    oParentPopulation.resize(m_iPopulationSize);
    oChildrenPopulation.resize(m_iPopulationSize);
	
	// reserve memory for population
	for(i = 0 ; i < m_iPopulationSize; i++)
	{
		oParentPopulation[i] = NULL;
		oChildrenPopulation[i] = NULL;
        dFitness[i] = -1;
	}

	// Generate random initial population
	for(i = 0; i < m_iPopulationSize; i++) 
	{
		pSolution = new TProblemSolution();
        oParentPopulation[i] = pSolution;

        // compute fitness based on scalarization fuction 
		pSolution -> SetScalarizingFunctionType (ScalarizingFunctionType);
        dFitness[i] = pSolution -> ScalarizingFunction(ReferencePoint, WeightVector);

        // update nondominated set
        if (bUpdateNondominatedSet == true) 
        {
            if (this->pNondominatedSet -> Update(*pSolution) == true) 
		    {
                this->NewNondominatedSolutionFound();
		    }
        }

        NewSolutionGenerated(*pSolution);

        if ((i == 0) || ((i > 0) && (dTheBestFitness < dFitness[i])))
        {
            dTheBestFitness = dFitness[i];
            oTheBestSolution = *pSolution;
        }
	}

    // main loop of algorithm
    while((m_iNumberOfGenerations > iCurrentGeneration) /*&& (iNoImprovement < iNoImprovementLevel)*/)
    {
        // generate next generation
        for(i = 0; i < m_iPopulationSize; i++)
        {
            iSolution1 = TMOTools::GetSolutionUsingBinaryTournamentMin(oParentPopulation, dFitness);
            iSolution2 = TMOTools::GetSolutionUsingBinaryTournamentMin(oParentPopulation, dFitness);

            pSolution = new TProblemSolution(*((TProblemSolution *)oParentPopulation[iSolution1]), 
                                             *((TProblemSolution *)oParentPopulation[iSolution2]));
			pSolution -> SetScalarizingFunctionType (ScalarizingFunctionType);

            if (TMOTools::DoMutate(m_dMutationProbability) == true)
            {
                pSolution -> Mutate();
            }
  
            oChildrenPopulation[i] = pSolution;
        }

        // release parent population
        for(i = 0; i < m_iPopulationSize; i++) 
	    {
            if (oParentPopulation[i] != NULL)
            {
                delete (TProblemSolution *)oParentPopulation[i];
                oParentPopulation[i] = NULL;
            }
        }

        // copy children to parent population and prepare fitness values
        for(i = 0; i < m_iPopulationSize; i++) 
	    {
		    pSolution = (TProblemSolution *)oChildrenPopulation[i];
            oChildrenPopulation[i] = NULL;
            oParentPopulation[i] = pSolution;

            // update nondominated set
            if (bUpdateNondominatedSet == true) 
            {
		        if (this->pNondominatedSet -> Update(*pSolution) == true) 
		        {
			        this->NewNondominatedSolutionFound();
		        }
            }

            NewSolutionGenerated(*pSolution);

            // compute fitness based on scalarization fuction 
            dFitness[i] = pSolution -> ScalarizingFunction(ReferencePoint, WeightVector);
                   
            // if new solution is better change the best one to current
            if (dTheBestFitness > dFitness[i])
            {
                dTheBestFitness = dFitness[i];
                oTheBestSolution = *pSolution;
            }
        }

        iCurrentGeneration += 1;
    }

    // release children and parent population
    for(i = 0; i < m_iPopulationSize; i++) 
	{
        if (oChildrenPopulation[i] != NULL)
        {
            delete (TProblemSolution *)oChildrenPopulation[i];
            oChildrenPopulation[i] = NULL;
        }

        if (oParentPopulation[i] != NULL)
        {
            delete (TProblemSolution *)oParentPopulation[i];
            oParentPopulation[i] = NULL;
        }
    }

    return oTheBestSolution;
}

#endif
