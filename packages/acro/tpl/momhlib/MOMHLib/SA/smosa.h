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

// SMOSA.h: interface for the TSMOSA class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SMOSA_H__C79F4BD1_DD06_11D4_832E_000000000000__INCLUDED_)
#define AFX_SMOSA_H__C79F4BD1_DD06_11D4_832E_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "global.h"
#include "weights.h"
#include "momhsolution.h"
#include "nondominatedset.h"
#include "momethod.h"
#include "sabase.h"

/**	Serafini's Multiople Objective Simulated Annealing
*
*	For more informations about the method see:
*	Serafini P. (1994), Simulated annealing for multiple objective optimization prob-lems, in: G.H. Tzeng, H.F. Wang, V.P. Wen, P.L. Yu (eds.), Multiple Criteria Deci-sion Making. Expand and Enrich the Domains of Thinking and Application, Springer, Berlin, 283-292.
*/
template <class TProblemSolution> class TSMOSA : public TSABase <TProblemSolution>
{
protected:
	/** Generating solution with its not scaled weight vector */
	TGeneratingPopulationElement* GeneratingSolution;

	/** Finds initial solution and assigns weight vectors to it */
	void FindInitialSolution ();

	/** Updates weight vector of generating solution */
	void UpdateWeightVector ();

public:
	/** Sets default values of parameters */
	TSMOSA();

	/** Sets parameters of the method */
	void SetParameters (double StartingTemperature,
		double FinalTemperature,
		double TemperatureDecreaseCoefficient,
		int MovesOnTemperatureLevel);

	/** Runs the algorithm */
	virtual void Run ();

};

/** Specialization of TSMOSA class displaying information
*	about progress of the optimization */
template <class TProblemSolution> class TDisplaySMOSA : public TSMOSA <TProblemSolution> {
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

#include "smosa.cpp"

#endif // !defined(AFX_SMOSA_H__C79F4BD1_DD06_11D4_832E_000000000000__INCLUDED_)
