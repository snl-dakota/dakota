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

#ifndef __NSGAIIC_H_
#define __NSGAIIC_H_

#include "global.h"
#include "weights.h"
#include "momhsolution.h"
#include "nondominatedset.h"
#include "momethod.h"
#include "paretobase.h"

/** Implements Nondominated Sorting Genetic Algorithm II - controled approach proposed by K. Deb
 **/
template <class TProblemSolution> class TNSGAIIC : public TParetoBase <TProblemSolution>
{
protected:
    /** Size of population multipled by two */
	int					m_iDoubledPopulationSize;

    /** Geometrical factor used in generation geometrical sequence used in element selection to mating population **/
	double				m_dPopulationGeometricalFactor;

    /** Q population - child population **/
	TSolutionsSet		m_oQPopulation;

    /** P population - parent population **/
	TSolutionsSet		m_oPPopulation;

    /** R population - merged population **/
	TSolutionsSet		m_oRPopulation;

public:

	TNSGAIIC();

	/** Set NSGAIIC algorithm parameters. 
     *
     * @param iPopulationSize population size
     * @param iNumberOfGenerations number of main iterations of algorithm  
     * @param dMutationProbability probability of mutation after recombination
     * @param dPopulationGeometricalFactor constant used in generation of geometrical sequence used in mating population selection 
     * @param bScalarize computation of distance will be done on scalarized objective values
     **/
	void SetParameters(int iPopulationSize, 
					   int iNumberOfGenerations,
					   double dMutationProbability,
					   double dPopulationGeometricalFactor,
					   bool bScalarize);

    /** Call to start computations
     **/
	void Run();
};

/** Visualize computation process
 **/
template <class TProblemSolution> class TDisplayNSGAIIC : public TNSGAIIC <TProblemSolution> 
{
protected:
	virtual void End ();
	virtual void NewNondominatedSolutionFound ();
	virtual void NewGenerationFound ();
};

#include "nsgaiic.cpp"

#endif 
