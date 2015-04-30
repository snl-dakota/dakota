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

// TPMA.h: interface for the TPMA class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TPMA_H__EE158132_9605_11D3_814C_000000000000__INCLUDED_)
#define AFX_TPMA_H__EE158132_9605_11D3_814C_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "global.h"
#include "weights.h"
#include "momhsolution.h"
#include "nondominatedset.h"
#include "momethod.h"
#include "hgabase.h"

/** Pareto Memetic Algorithm 
*
*	By default it generates approximately Pareto-optimal
*	solutions from all regions of the Pareto set, but
*	may be focused on a region of this set by overrirding
*	THGABase::GetRandomWeightVector method.
*/
template <class TProblemSolution> class TPMA : public THGABase <TProblemSolution>
{
protected:
	/** Number of iterations - set by SetParameters 
	*
	*	Default value - 10
	*/
	int NumberOfIterations;

	/** Size of the main PMA Population */
	unsigned int MainPopulationSize ;

public:
	/**	Sets parameters of the method
	*
	*	TempPopulationSize - size of the temporary population,
	*	generally the higher the value, the slower improvement
	*	but the better final results.
	*
	*	InitialPopulationSize - size of the initial population;
	*	If InitialPopulationSize <= 0 then the number of initial 
	*	solutions will be determined automatically by TPMA::Run.
	*	If InitialPopulationSize < TempPopulationSize then 
	*	a warning is displayed and
	*	TempPopulationSize initial solutions are generated (we 
	*	do not recommend this approach).
	*	If InitialPopulationSize > TempPopulationSize this parameter
	*	specifies the exact number of initial solutions.
	*
	*	NumberOfIterations - controls running time of the
	*	algorithm. Number of local searches =
	*	NumberOfIterations * InitialPopulationSize
	*
	*	ScalarizingFunctionType - type of the scalarizing function
	*	used by the algorithm.
	*/
	void SetParameters (int TempPopulationSize, int InitialPopulationSize,
		int NumberOfIterations,
		TScalarizingFunctionType ScalarizingFunctionType);

	/** Runs the algorithm */
	virtual void Run ();

	/** Default constructor
	*
	*	Sets default values of NumberOfIterations, TempPopulationSize,
	*	InitialPopulatioSize and ScalarizingFunctionType fields
	*/
	TPMA () {
		NumberOfIterations = 10;
		this->TempPopulationSize = 20;
		this->InitialPopulationSize = 0;
		this->ScalarizingFunctionType = _Chebycheff;
	};

};

/** Specialization of TPMA class displaying information
*	about progress of the optimization */
template <class TProblemSolution> class TDisplayPMA : public TPMA <TProblemSolution> {
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

#include "pma.cpp"

#endif // !defined(AFX_TPMA_H__EE158132_9605_11D3_814C_000000000000__INCLUDED_)
