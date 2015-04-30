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

// MOMSLS.h: interface for the TMOMSLS class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOMSLS_H__75C0B360_9D53_11D4_82A7_000000000000__INCLUDED_)
#define AFX_MOMSLS_H__75C0B360_9D53_11D4_82A7_000000000000__INCLUDED_

#include "momethod.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/** Multiple Objective Multiple Start Local Search 
*
*	By default it generates approximately Pareto-optimal
*	solutions from all regions of the Pareto set, but
*	may be focused on a region of this set by overrirding
*	GetRandomWeightVector method.
*/
template <class TProblemSolution> class TMOMSLS : public TMOMethod <TProblemSolution>
{
protected:
	/** Number of iterations - local searches */
	int NumberOfIterations;

	/** Current type of scalarizing function usd by MOGLS in local search 
	*
	*	Default value - _Chebycheff
	*/
	TScalarizingFunctionType ScalarizingFunctionType;

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
	/**	Runs single objective multiple start local search algorithm
	*	
	*	Performs NumberOfIterations local searches */
	virtual TProblemSolution SingleObjectiveOptimization (TPoint ReferencePoint, 
		TScalarizingFunctionType ScalarizingFunctionType, 
		TWeightVector& WeightVector,
		bool bUpdateNondominatedSet);

	/**	Sets parameters of the method
	*
	*	NumberOfIterations - controls running time of the
	*	algorithm. Defines the number of local searches
	*
	*	ScalarizingFunctionType - type of the scalarizing function
	*	used by the algorithm.
	*/
	void SetParameters (int NumberOfIterations,		
		TScalarizingFunctionType ScalarizingFunctionType);

	/** Runs the algorithm */
	virtual void Run ();

	/** Default constructor
	*
	*	Sets default values of NumberOfIterations, TempPopulationSize,
	*	InitialPopulatioSize and ScalarizingFunctionType fields
	*/
	TMOMSLS () {
		NumberOfIterations = 100;
		ScalarizingFunctionType = _Chebycheff;
	};

};

/** Specialization of TMOMSLS class displaying information
*	about progress of the optimization */
template <class TProblemSolution> class TDisplayMOMSLS : public TMOMSLS <TProblemSolution> {
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

};

#include "momsls.cpp"

#endif // !defined(AFX_MOMSLS_H__75C0B360_9D53_11D4_82A7_000000000000__INCLUDED_)
