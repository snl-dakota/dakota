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

// SABase.h: interface for the TSABase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SABASE_H__5C59D552_391A_4BC3_ACDA_77CD85A0725C__INCLUDED_)
#define AFX_SABASE_H__5C59D552_391A_4BC3_ACDA_77CD85A0725C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "global.h"
#include "weights.h"
#include "momhsolution.h"
#include "nondominatedset.h"
#include "momethod.h"

/** Element of generating population in simulate annealing algorithms
*
*	Element contains both solution and its normalized (not scaled) weight vector */
class TGeneratingPopulationElement {
public:
	/** Generating solution */
	TMOMHSolution* GeneratingSolution;

	/** Weight vector associated with GeneratingSolution before rescaling */
	TWeightVector WeightVector;

	/** Removes GeneratingSolution */
	~TGeneratingPopulationElement () {
		delete GeneratingSolution;
	};
};

/** Base abstract class for simulated annealing algorithms 
*
*	This class contains a number of common fields 
*	and implements single objective simulated annealing */
template <class TProblemSolution> class TSABase : public TMOMethod <TProblemSolution>
{
protected:
	/** Number of moves accepted since the start of the algorithm */
	int iAcceptedMoves;

	/** Number of moves accepted on the current temperature level */
	int iAcceptedMovesOnTemperatureLevel;

	/** Current temeprature */
	double Temperature;

	double StartingTemperature;

	double FinalTemperature;

	/** After each temeprature level (plateau) he current temperature is multiplied by this value */
	double TemperatureDecreaseCoefficient;

	/** Number of moves to be performed on a temperature level by a single solution */
	int MovesOnTemperatureLevel;

	/** Current move a temperature level */
	int iMove; 

	/** Virtual callback function to be (optionally) defined in 
	*	a specialization of TPSA 
	*
	*	TPSA class 
	*	calls this method when 
	*	temperature has been decreased */
	virtual void TemperatureDecreased () {};

	/** By default calls ::GetRandomWeightVector ()
	*
	*	May be overriten in order to change
	*	the probability distribution of weight vectors,
	*	e.g. in order to impose some constraints on the
	*	feasible weight verctors */
	virtual TWeightVector GetRandomWeightVector () {
		return ::GetRandomWeightVector ();
	};

	/** Restores feasibility of the weight vector.
	*	By default is empty.
	*
	*	You may overrite this function if you want to 
	*	impose some constraints on the weight vector.
	*	Called after changing each weight vector associated
	*	with a generating solution. 
	*	It is not called after a weight vector is drawn 
	*	at random! */
	virtual void RestoreFeasibility (TWeightVector& WeightVector) {};

public:
	/**	Runs single objective simulated annealing algorithm */
	virtual TProblemSolution SingleObjectiveOptimization (TPoint ReferencePoint, 
		TScalarizingFunctionType ScalarizingFunctionType, 
		TWeightVector& WeightVector,
		bool bUpdateNondominatedSet);

};

#include "sabase.cpp"

#endif // !defined(AFX_SABASE_H__5C59D552_391A_4BC3_ACDA_77CD85A0725C__INCLUDED_)
