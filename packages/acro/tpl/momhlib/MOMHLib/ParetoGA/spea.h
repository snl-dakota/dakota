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

#ifndef __SPEA_H_
#define __SPEA_H_

#include "paretobase.h"

/** Implements Strength Pareto Evolutionary Algorithm proposed by E. Zizler
 *
 * This is template of one of the best and simplest multiobjective genetic algorithm. 
 **/
template <class TProblemSolution> class TSPEA : public TParetoBase <TProblemSolution>
{
protected:

    /** Number of individuals that should remain after clustering **/
	int					m_iNondominatedPopulationSize;

    /** Size of nondominated set above the set will be clustered. **/
	int					m_iClusteringLevel;

    /** Scalarize objective values in distance computation **/
	bool				m_bScalarize;

    /** Population of solutions used by SPEA **/
	TSolutionsSet		m_oPopulation;

public:

    /** Current generation **/
	int					m_iGeneration;

	TSPEA();

	/** Set SPEA algorithm parameters. 
     *
     * @param iPopulationSize population size
     * @param iNumberOfGenerations number of main iterations of algorithm  
     * @param dMutationProbability probability of mutation after recombination
     * @param iNondominatedPopulationSize size of nondominated set after clustering
     * @param iClusteringLevel size of population above clustering will be performed.
     * @param bScalarize computation of distance will be done on scalarized objective values
     **/
    void SetParameters(int iPopulationSize, 
					   int iNumberOfGenerations,
					   double dMutationProbability,
					   int iNondominatedPopulationSize,
					   int iClusteringLevel,
					   bool bScalarize);

    /** Call to start computations
     **/
	void Run();
};

/** Visualize computation process
 **/
template <class TProblemSolution> class TDisplaySPEA : public TSPEA <TProblemSolution> 
{
protected:
	virtual void End ();
	virtual void NewNondominatedSolutionFound ();
	virtual void NewGenerationFound ();
};

#include "spea.cpp"

#endif 
