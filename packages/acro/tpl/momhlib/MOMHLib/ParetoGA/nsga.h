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

#ifndef __NSGA_H_
#define __NSGA_H_

#include "global.h"
#include "weights.h"
#include "momhsolution.h"
#include "nondominatedset.h"
#include "tindexedpoint.h"
#include "momethod.h"
#include "paretobase.h"

/** Old and not too efficient NSGA algorithm.
 *
 * This implementation uses Quad Tree to extract solutions from nondominate set.
 **/
template <class TProblemSolution> class TNSGA : public TParetoBase <TProblemSolution>
{
protected:
    /** Maximal distance taken into consideration in computation of sharing value **/
	double						m_dNeighbourhoodDistance;

    /** Scalarization enabled or disabled **/
	bool						m_bScalarize;

    /** Child population **/
	TSolutionsSet				m_oChildrenPopulation;

    /** Parent population **/
	TSolutionsSet				m_oParentPopulation;

    /** Temporaty population of TIndexedSolutionPoint used in computations **/
	TSolutionsSet				m_oDummyPopulation;

public:	

    /** Current iteration - generation **/
	int							m_iGeneration;

	TNSGA();

    /** Set NSGA algorithm parameters. 
     *
     * @param iPopulationSize population size
     * @param iNumberOfGenerations number of main iterations of algorithm  
     * @param dMutationProbability probability of mutation after recombination
     * @param dNeighbourhoodDistance maximal distance between two solutions taken into consideration in sharing values computation
     * @param bScalarize computation of distance will be done on scalarized objective values
     **/
	void SetParameters(int iPopulationSize, 
					   int iNumberOfGenerations,
					   double dMutationProbability,
					   double dNeighbourhoodDistance,
					   bool bScalarize);
    
    /** Call to start computations
     **/
	void Run();
};

/** Visualize computation process
 **/
template <class TProblemSolution> class TDisplayNSGA : public TNSGA <TProblemSolution> 
{
protected:
	virtual void End ();
	virtual void NewNondominatedSolutionFound ();
	virtual void NewGenerationFound ();
};

#include "nsga.cpp"

#endif
