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

#ifndef __NONDOMINATEDSET_H_
#define __NONDOMINATEDSET_H_

#include "global.h"
#include "solution.h"
#include "solutionsset.h"

const int		PARAMETER_SPACE = 1;
const int		OBJECTIVE_SPACE = 2;

/** This class represent set of nondominated solutions 
 * 
 * This class has methods usable to filter out nondominated solutions to set.
 * It also contains functions that reduce number of solutions in set using
 * clustering. Nondominated set is a TSolutionSet that contains only nondominated
 * solution. It also can have null values in vector storing solutions.  
 **/
class TNondominatedSet: public TSolutionsSet
{
private:
    /** Compute average distance between two clusters 
     * 
     * Average distance is divided sume of distances between every pair of solutions
     * taken from two sets.
     *
     * @param iCluster1 indices contains information about solutions from first cluster
     * @param iCluster2 indices contains information about solutions from second cluster
     * @param iSpaceType if PARAMETER_SPACE distance will be in parameter space, if OBJECTIVE_SPACE distance will be measured in objective space.
     * @param bScalarize scalarization of distance on every objective; approximation of 
     *        ideal and nadir points are used for scalarization 
     * @param dIdeal ideal point
     * @param dNadir nadir point 
     * @return distance between clusters
     **/
   	double ClusterDistance(IntVector iCluster1, IntVector iCluster2, int iSpaceType, bool bScalarize, vector<double> &dIdeal, vector<double> &dNadir);
	

   /** Get centroid of some cluster
     * 
     * Average distance is divided sume of distances between every pair of solutions
     * taken from two sets.
     *
     * @param iCluster  indices contains information about solutions from cluster
     * @param iSpaceType if PARAMETER_SPACE distance will be in parameter space, if OBJECTIVE_SPACE distance will be measured in objective space.
     * @param bScalarize scalarization of distance on every objective; approximation of 
     *        ideal and nadir points are used for scalarization 
     * @param dIdeal ideal point
     * @param dNadir nadir point 
     * @return index of centroid point (or near point)
     **/
    int GetCentroid(IntVector iCluster, int iSpaceType, bool bScalarize, vector<double> &dIdeal, vector<double> &dNadir);

protected:

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
	bool ReduceIndividuals(unsigned int iPopulationSize, vector<int> &iReducedIndividuals, int iSpaceType, bool bScalarize);

public:

    /** Reduce nondominated set to given size using clustering.
     * 
     * This function reduce nondominated set to given number of solutions.
     * Should be overloaded by offspring class.
     *
     * @param iPopulationSize target size of population
     * @param bScalarize if true the distances by objectives will be scalarized
     * @return if true ok, false - error
     **/
    virtual bool ReduceSetByClustering(int iPopulationSize,  bool bScalarize) 
    {
        return false;
    }

    /** Update set using given solution
     * 
     * This function reduce nondominated set to given number of solutions.
     * Should be overloaded by offspring class.
     *
     * @param Solution possibly nondominated solution
     * @return true if solution is nondominated and set has been updated with this solution, false solution is dominated by solution in set
     **/
	virtual bool Update(TSolution& Solution)
    {
        return false;
    }

    /** Merge two nondominated sets and give final nondominated set.
     * 
     * This function add two sets and eliminate dominated solutions from both sets.
     * Should be overloaded by offspring class.
     *
     * @param NondominatedSet possibly nondominated solution
     * @return if true there weren't dominated solution, if false some solutions have been eliminated 
     **/
	virtual bool Update(TNondominatedSet& NondominatedSet)
    {
        return false;
    }

    /** This function choose random solution from set of solutions
     * 
     * Probability of choose for every solution should be equal. 
     * Should be overloaded by offspring class.
     *
     * @param pSolution reference to pointer where solution will be placed
     **/
	virtual void GetRandomSolution(TSolution* &pSolution)
    {
        pSolution = NULL;
    }

    /** Delete all solutions from set.
     * 
     * Every solution in set is released and vector is reallocated to size 0. 
     **/
    virtual void DeleteAll()
    {
    }

	TNondominatedSet() : TSolutionsSet()
	{ 
	}
};

#endif
