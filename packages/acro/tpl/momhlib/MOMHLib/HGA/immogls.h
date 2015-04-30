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

// TIMMOGLS.h: interface for the TIMMOGLS class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMMOGLS_H__9A9E3751_C24D_11D4_82E6_000000000000__INCLUDED_)
#define AFX_IMMOGLS_H__9A9E3751_C24D_11D4_82E6_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "global.h"
#include "weights.h"
#include "momhsolution.h"
#include "nondominatedset.h"
#include "momethod.h"
#include "hgabase.h"

/** Multiple Objective Genetic Local Search proposed by Ishibuchi 
	and Murata 
*
*	By default it generates approximately Pareto-optimal
*	solutions from all regions of the Pareto set, but
*	may be focused on a region of this set by overrirding
*	GetRandomWeightVector method.
*/
template <class TProblemSolution> class TIMMOGLS : public THGABase <TProblemSolution> 
{
protected:
	/** Finds initial population of solutions*/
	void FindInitialPopulation ();

	/** Number of iterations - set by SetParameters 
	*
	*	Default value - 10
	*/
	int NumberOfIterations;

	/** Current type of scalarizing function usd by MOGLS in local search 
	*
	*	Default value - _Chebycheff
	*/
	TScalarizingFunctionType ScalarizingFunctionType;

	/** Current weight vector */
	TWeightVector WeightVector;

	/** MOGLS Population 
	*
	*	Default value - 100*/
	TSolutionsSet Population;

	/** Size of the MOGLS Population */
	int PopulationSize ;

	/** Size of the elite - number of potentially nondominated 
	*	solutions added to each generation */
	int EliteSize;

	/** Virtual callback function to be (optionally) defined in 
	*	a specialization of TMOGLS 
	*
	*	TMOGLS class 
	*	calls this method when generation
	*	of the initial population is finished */
	virtual void InitialPopulationFound () {};

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
	/**	Sets parameters of the method
	*
	*	PopulationSize - size of the population,
	*	generally the higher the value, the slower improvement
	*	but the better final results.
	*
	*	NumberOfIterations (generations) - controls running time of the
	*	algorithm. Number of local searches =
	*	NumberOfIterations * PopulationSize
	*
	*	ScalarizingFunctionType - type of the scalarizing function
	*	used by the algorithm.
	*/
	void SetParameters (int PopulationSize,
		int NumberOfIterations, int EliteSize,
		TScalarizingFunctionType ScalarizingFunctionType);

	/** Runs the algorithm */
	virtual void Run ();

	/** Default constructor
	*
	*	Sets default values of NumberOfIterations, PopulationSize
	*	and ScalarizingFunctionType fields
	*/
	TIMMOGLS() {
		NumberOfIterations = 10;
		PopulationSize = 100;
		EliteSize = 10;
		ScalarizingFunctionType = _Chebycheff;
	};

};

/** Specialization of TMOGLS class displaying information
*	about progress of the optimization */
template <class TProblemSolution> class TDisplayIMMOGLS : public TIMMOGLS <TProblemSolution> {
protected:
	/** Displays:
	*	Information about the ned of optimization
	*	Number of potentially nondominated (Pareto-optimal) solutions
	*	Ideal point
	*	Nadir point
	*/
	virtual void End ();

	/** Displays:
	*	Number of potentially nondominated (Pareto-optimal) solutions
	*	Ideal point
	*	Nadir point
	*/
	virtual void NewNondominatedSolutionFound ();

	/** Displays 
	*	Information that the initial population was generated
	*	Initial population size
	*/
	virtual void InitialPopulationFound ();
};

#include "immogls.cpp"

#endif // !defined(AFX_IMMOGLS_H__9A9E3751_C24D_11D4_82E6_000000000000__INCLUDED_)
