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

// MOSA.h: interface for the TMOSA class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOSA_H__C79F4BD1_DD06_11D4_832E_000000000000__INCLUDED_)
#define AFX_MOSA_H__C79F4BD1_DD06_11D4_832E_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "global.h"
#include "weights.h"
#include "momhsolution.h"
#include "nondominatedset.h"
#include "momethod.h"
#include "sabase.h"

/**	Multiple Objective Simulated Annealing
*
*	For more informations about the method see:
*	Ulungu E.L., Teghem J., Fortemps Ph., Tuyttens D. (1999), MOSA method: a tool for solving multiobjective combinatorial optimization problems, Journal of Multi-Criteria Decision Analysis, 8, 221-236.
*/
template <class TProblemSolution> class TMOSA : public TSABase <TProblemSolution>
{
protected:
	/* Number of generating solutions, i.e. number of search directions */
	int GeneratingPopulationSize;

	/** Generating population */
	vector<TGeneratingPopulationElement*> GeneratingPopulation;

	/** Finds initial solutions and assigns weight vectors to them */
	void FindInitialPopulation ();

public:
	/** Sets default values of parameters */
	TMOSA();

	/** Sets parameters of the method
	*
	*	Note that the total number of moves performed by all generating solutions
	*	on a given temperature level is equal to MovesOnTemperatureLevel * GeneratingPopulationSize */
	void SetParameters (double StartingTemperature,
		double FinalTemperature,
		double TemperatureDecreaseCoefficient,
		int MovesOnTemperatureLevel,
		int GeneratingPopulationSize);

	/** Runs the algorithm */
	virtual void Run ();
};

/** Specialization of TMOSA class displaying information
*	about progress of the optimization */
template <class TProblemSolution> class TDisplayMOSA : public TMOSA <TProblemSolution> {
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

	/** Displays:
	*	New temperature
	*	Percentage of accepted moves
	*/
	virtual void TemperatureDecreased ();


};

#include "mosa.cpp"

#endif // !defined(AFX_MOSA_H__C79F4BD1_DD06_11D4_832E_000000000000__INCLUDED_)
