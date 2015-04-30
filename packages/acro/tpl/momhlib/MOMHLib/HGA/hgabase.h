/*
Multiple Objective MetaHeuristics Library in C++ MOMHLib++
Copyright (C) 2001 Andrzej Jaszkiewicz

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

// HGABase.h: interface for the THGABase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HGABase_H__EE158132_9605_11D3_814C_000000000000__INCLUDED_)
#define AFX_HGABase_H__EE158132_9605_11D3_814C_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "global.h"
#include "weights.h"
#include "momhsolution.h"
#include "nondominatedset.h"
#include "momethod.h"

/** Virtual population. Just the value of a scalarizing 
*	function of the solutons */
typedef vector<double> TVirtualPopulation;

/** Base abstract class for hybrid genetic algorithms 
*
*	This class implements a method for finding set of initial solutions
*	and single objective genetic local search */
template <class TProblemSolution> class THGABase : public TMOMethod <TProblemSolution>
{
private:
	/** Used only by FindInitialPopulation () method 
	*	
	*	Udaptes virtual Population
	*	Adds the new solution to the population if it is
	*	better than the worst solution in the population
	*	on the current scalarizing function with the
	*	current weight vector 
	*	In order to speed up the updating the temporary population
	*	is always sorted from the best ([0]) to the worst ([size()-1]) 
	*	The solution in temporary population must have unique
	*	value of Scalarizing funztion */
	void UpdateVirtualPopulation (TVirtualPopulation& VirtualPopulation, 
		double NewSolutionValue);

	/** Used only by FindInitialPopulation () method 
	*	
	*	Virtual populatons corrsponding to all generated solutions
	*	Each virtual populations contains TempPopulationSize best 
	*	value of scalarizing functions on the solution's scalarizing 
	*	value */
	vector<TVirtualPopulation> VirtualPopulations;

	/** Used only by FindInitialPopulation () method 
	*	
	*	Last not emty virtual population */
	unsigned int LastVirtualPopulationUpdated;

	/** Used only by FindInitialPopulation () method 
	*	
	*	Return difference between the average quality over all direction of
	*	TempPopulationSize best solution from the population and 
	*	average quality of local optimum found by local optimization in this direction*/
	double EvaluateInitialPopulation();

protected:
	/** Finds initial poulation of solutions
	*
	*	The initial population assures that average quality over all direction of
	*	TempPopulationSize best solution from the population is equal to 
	*	average quality of local optimum found by local optimization in this direction*/
	void FindInitialPopulation ();

	/** Current type of scalarizing function used in local search 
	*
	*	Default value - _Chebycheff
	*/
	TScalarizingFunctionType ScalarizingFunctionType;

	/** Current weight vector */
	TWeightVector WeightVector;

	/** Main population */
	TSolutionsSet MainPopulation;

	/** Temporary population */
	TSolutionsSet TempPopulation;

	/** Size of the temporary population 
	*
	*	Set by SetParameters method. 
	*	Default value - 20
	*/
	unsigned int TempPopulationSize;

	/** Size of the initial population 
	*
	*	Set by SetParameters method. See description of
	*	SetParameters method to learn about interpetation
	*	of this field.
	*	Default value - 0
	*/
	unsigned int InitialPopulationSize;

	/** Virtual callback function to be (optionally) defined in 
	*	a specialization of THGABase 
	*
	*	THGABase class 
	*	calls this method when generation
	*	of the initial population is finished */
	virtual void InitialPopulationFound () {};

	/** Udaptes temporary MOGLS Population
	*
	*	Adds the new solution to the population if it is
	*	better than the worst solution in the population
	*	on the current scalarizing function with the
	*	current weight vector 
	*	In order to speed up the updating the temporary population
	*	is always sorted from the best ([0]) to the worst ([size()-1]) 
	*	The solution in temporary population must have unique
	*	value of Scalarizing funztion 
	*	Returns true in the new solution is added */
	bool UpdateTempPopulation (TPoint& ReferencePoint, TMOMHSolution& Solution);

	/** By default calls ::GetRandomWeightVector ()
	*
	*	May be overriten in order to changer
	*	the probability distribution of weight vectors,
	*	e.g. in order to impose some constraints on the
	*	feasible weight vectors */
	virtual TWeightVector GetRandomWeightVector () {
		return ::GetRandomWeightVector ();
	};

public:
	/**	Runs single objective genetic local search algorithm
	*	
	*	Uses population of size defined by TempPopulationSize fields */
	virtual TProblemSolution SingleObjectiveOptimization (TPoint ReferencePoint, 
		TScalarizingFunctionType ScalarizingFunctionType, 
		TWeightVector& WeightVector,
		bool bUpdateNondominatedSet);


};

#include "hgabase.cpp"

#endif // !defined(AFX_HGABase_H__EE158132_9605_11D3_814C_000000000000__INCLUDED_)
