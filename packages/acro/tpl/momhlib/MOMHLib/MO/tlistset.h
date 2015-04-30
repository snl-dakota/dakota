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

#ifndef __TLISTSET_H_
#define __TLISTSET_H_

#include "global.h"
#include "solution.h"
#include "nondominatedset.h"

/** Implementation of nondominated set based on list
 *
 * This is simple but not the most efficient solution of nondominated set.
 **/
template <class TProblemSolution> class TListSet: public TNondominatedSet
{
protected:
	
    /** Update nadir point values
     * 
     * Method updates nadir point approximation. 
     * Nadir point is approximated on current nondominated set.  
     *
     * @param iRemovedSolution index of solution to be removed
     **/
	void UpdateNadir (int iRemovedSolution);

public:

    /** Reduce set to given number of inviduals using clustering 
     *
     * This is internal representation of function to reduce nondominated set.
     *
     * @param iPopulationSize target population size
     * @param iReducedIndividuals vector of indices to solutions that will be clustered
     * @param iSpaceType if PARAMETER_SPACE distance will be in parameter space, if OBJECTIVE_SPACE distance will be measured in objective space.
     * @param bScalarize scalarization of distance on every objective; approximation of 
     *        ideal and nadir points are used for scalarization
     * @return if true ok, false - error
     **/
	virtual bool ReduceSetByClustering(int iPopulationSize, bool bScalarize);

    /** Update set using given solution
     * 
     * This function reduce nondominated set to given number of solutions.
     *
     * @param Solution possibly nondominated solution
     * @return if true solution is nondominated and set has been updated with this solution, false solution is dominated by solution in set
     **/
	virtual bool Update(TSolution& Solution);
    
    /** Merge two nondominated sets and give final nondominated set.
     * 
     * This function add two sets and eliminate dominated solutions from both sets.
     *
     * @param NondominatedSet possibly nondominated solution
     * @return if true there weren't dominated solution, if false some solutions have been eliminated 
     **/
	virtual bool Update(TNondominatedSet& NondominatedSet);

    /** This function choose random solution from set of solutions
     * 
     * Probability of choose for every solution should be equal. 
     *
     * @param pSolution reference to pointer where solution will be placed
     **/
	virtual void GetRandomSolution(TSolution* &pSolution);

    /** Delete all solutions from set.
     * 
     * Every solution in set is released and vector is reallocated to size 0. 
     **/
	virtual void DeleteAll();

	/** Destruct object 
     *
     **/
	TListSet();
	
	~TListSet () {
		DeleteAll();
	};
};

#include "tlistset.cpp"

#endif // _TLISTSET_H_
