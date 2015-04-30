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

#ifndef __PARETOBASE_H_
#define __PARETOBASE_H_

#include "global.h"
#include "weights.h"
#include "momhsolution.h"
#include "momethod.h"
#include "nondominatedset.h"

/** This template is an abstract for multiobjective genetic algorithms based on pareto ranking.
 * 
 * This class contains common variables and methods used in all pareto algorithms.
 * Used in implementation of NSGA, NSGAII, NSGAIIC, SPEA.
 **/
template <class TProblemSolution> class TParetoBase : public TMOMethod <TProblemSolution>
{
protected:

    /** Population size **/
    int							m_iPopulationSize;
    
    /** Number of generations to compute **/
	int							m_iNumberOfGenerations;

    /** Probability of mutation in algoritm **/
	double						m_dMutationProbability;

    /** Current iteration - generation **/
	int					        m_iGeneration;

    /** Scalarization enabled or disabled **/
	bool				        m_bScalarize;

	/**	Runs single objective optimization of scalarizing function
	 *
	 *	The scalarizing function is defined by ReferencePoint
	 *	ScalarizingFunctionType and WeightVector.
	 *	This method is optional. It may be useful while testing
	 *	your implementation and while evaluation the multiple
	 *	objective method. 
	 *	Note that this method will return object of TProblemSolution
	 *	only if it has a = operator defined. By default it will 
	 *	return object of TPoint class. In this case to compare is used 
     *  single objective version of NSGA
     *  
     **/
	virtual TProblemSolution SingleObjectiveOptimization(TPoint ReferencePoint, 
		TScalarizingFunctionType ScalarizingFunctionType, 
		TWeightVector& WeightVector,
		bool bUpdateNondominatedSet);
};

#include "paretobase.cpp"

#endif
