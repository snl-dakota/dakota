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

#ifndef __TMOTOOLS_H_
#define __TMOTOOLS_H_

#include "global.h"
#include "solutionsset.h"


/** This class contains common used methods that can be applied in many algoriths
 *
 * This libray of static functions that simplify coding of new methods
 **/
class TMOTools  
{
public:
    
    /** Sort selected solutions in solution set by each of objective.
     *
     * This function returns matrix that contan position of elements sorted be every objective. 
     * iSortTable returns set of tables of size equal to number of objectives. Each of table contains
     * sorted solutions position by given objective. This function uses quick sort.
     *
     * @param oSolutions solution set that will be sorted
     * @param iSelectedIndices selected indices that wil be sorted
     * @param iSortTable returns matrix of indices 
     * @return true if ok, false if error
     **/
	static bool QuickSortByObjectivesMax(TSolutionsSet &oSolutions, IntVector &iSelectedIndices, vector<IntVector> &iSortTable);

    /** Sort selected solutions using value
     *
     * This function returns returns sorted indices to element. It uses values from dValue
     * and sort elements from maximal value to minimal. 
     *
     * @param oSolutionSet solution set that will be sorted
     * @param dValue vector of values associated with oSolutionSet
     * @param iIndices returns sorted indices 
     **/    
    static void QuickSortByValueMax(TSolutionsSet &oSolutionSet, vector<double> &dValue, vector<int> &iIndices);

    /** Sort oSolutionSet using fitness
     *
     * This function returns returns sorted solutions set. It uses values from dFitness
     * and sort elements from maximal value to minimal. 
     *
     * @param oSolutionSet solution set that will be sorted
     * @param dFitness vector with fitness values associated with oSolutionSet
     **/    
    static void QuickSortSolutionUsingFitnessMax(TSolutionsSet &oSolutionSet, vector<double> &dFitness);

    /** Extract Pareto optimal fronts from solution set 
     *
     * This function used in NSGAII extract Pareto optimal fronts from set of solutions
     *
     * @param oSolutions solution set that will be analysed
     * @param iF vector of int vectors with indices for each of front; size of iF is number of fronts plus 1 
     * @return true if ok, false if error
     **/    
	static bool FastNondominatedSort(TSolutionsSet &oSolutions, vector<IntVector> &iF);
    
    /** Extract Pareto optimal fronts from solution set 
     *
     * This function used in NSGAII extract Pareto optimal fronts from set of solutions
	 * Uses TMOMHConstrainedSolution::ConstrainedCompare instead of TMOMHSolution::Compare
     *
     * @param oSolutions solution set that will be analysed
     * @param iF vector of int vectors with indices for each of front; size of iF is number of fronts plus 1 
     * @return true if ok, false if error
     **/    
	static bool ConstrainedFastNondominatedSort(TSolutionsSet &oSolutions, vector<IntVector> &iF);
    
    /** Compute that you should or not perform mutation
     *
     * @param dProbability probability of mutation
     * @return true do mutation, false don't do
     **/    
	static bool DoMutate(double dProbability);
    
    /** Get solution using stochastic proportionate selection from oSolutions.
     *
     * dFitness is taken into consideration. If dFitness is greater for some solution the probability of
     * selection is also greated.
     * 
     * @param oSolutions set of solution from which we choose one.
     * @param dFitness fitness vector associated with oSolutions
     * @param dSumOfFitness to speedup process (sum must be computed earlier)
     * @return index of selected solution
     **/    
    static int  GetRandomSolutionUsingFitnessMax(vector<TSolution *> &oSolutions, vector<double> &dFitness, double dSumOfFitness);
	
    /** Get random index from solutions set and selected set of indices.
     *
     * This function get one indice from iIndices randomly but only whan pointed entry in oSolutionSet
     * is not equal null. This function works also with nondominated set.
     * 
     * @param oSolutionSet set of solution from which we choose one.
     * @param iIndices list of indices from which index is choosen
     * @return index of selected solution
     **/   
	static int  GetRandomIndex(TSolutionsSet &oSolutionSet, vector<int> &iIndices);
	
    /** Implements binary tournament selection. 
     *
     * This function choose solution with less fitness value from 2 randomly choosen solutions
     * 
     * @param oSolutions set of solution from which we choose one.
     * @param iIndices list of indices from which index is choosen
     * @return index of selected solution
     **/   
    static int  GetSolutionUsingBinaryTournamentMin(vector<TSolution *> &oSolutions, vector<double> &dFitness);

    /** Implements binary tournament selection. 
     *
     * This function choose solution with less fitness value from 2 randomly choosen solutions
     * 
     * @param oSolutions set of solution from which we choose one.
     * @param iIndices list of indices from which index is choosen
     * @return index of selected solution
     **/   
    static int  GetSolutionUsingBinaryTournamentMax(vector<TSolution *> &oSolutions, vector<double> &dFitness);

    /** Implements binary tournament selection using distance values. 
     *
     * This function choose solution with less fitness value from 2 randomly choosen solutions.
     * If fitness is equal distance is taken into consideration. Prefered solution is with greater distance.  
     * 
     * @param oSolutions set of solution from which we choose one.
     * @param dFitness associated with solutions fitness values
     * @param dDistance associated with solutions distance values
     * @return index of selected solution
     **/   
    static int  GetSolutionUsingBinaryTournamentNischedMin(vector<TSolution *> &oSolutions, vector<double> &dFitness, vector<double> &dDistance);

private:

    /** Sort oSolutionSet using fitness - recursive implementation.
     *
     * This function returns returns sorted solutions set. It uses values from dFitness
     * and sort elements from maximal value to minimal. 
     *
     * @param oSolutionSet solution set that will be sorted
     * @param dFitness vector with fitness values associated with oSolutionSet
     **/    
	static void  RecursiveQuickSortSolutionsMax(int iStartPos, int iEndPos, TSolutionsSet &oSolutionSet, vector<double> &dFitness);
    
    /** Sort selected solutions in solution set by each of objective.
     *
     * This function returns matrix that contan position of elements sorted be every objective. 
     * iSortTable returns set of tables of size equal to number of objectives. Each of table contains
     * sorted solutions position by given objective. This function uses quick sort. Sorting method take into consideration
     * that objective minimize or maximize values. This function implements quick sort.
     *
     * @param iObjective selected objective
     * @param iStartPos start position (range to sort)
     * @param iEndPos end position (range to sort)
     * @param oSolutions solutions set that will be sorted
     * @param iIndices return sorted map of indices 
     **/
    static void  RecursiveQuickSortObjectiveMax(int iObjective, int iStartPos,int iEndPos, TSolutionsSet &oSolutions, IntVector &iIndices);

    /** Sort selected solutions using value - recursive implementation.
     *
     * This function returns returns sorted indices to element. It uses values from dValue
     * and sort elements from maximal value to minimal. It is quick sort.
     *
     * @param iStartPos start position (range to sort)
     * @param iEndPos end position (range to sort)
     * @param oSolutionSet solution set that will be sorted
     * @param dValue vector of values associated with oSolutionSet
     * @param iIndices returns sorted indices 
     **/        
    static void  RecursiveQuickSortByValueMax(int iStartPos, int iEndPos, TSolutionsSet &oSolutionSet, vector<double> &dValue, vector<int> &iIndices);
};

#endif 
