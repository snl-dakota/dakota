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

// TMOKPSolution.h: interface for the TMOKPSolution class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TMOKPSOLUTION_H__3867D633_2451_11D4_8230_000000000000__INCLUDED_)
#define AFX_TMOKPSOLUTION_H__3867D633_2451_11D4_8230_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "momhsolution.h"

/** Solution of the multiple objective knapsack problem */
class TMOKPSolution : public TMOMHSolution
{
private:
	/** Used by FindLocalMove and RejectLocalMove */
	vector<bool> SavedItemsInKnapsack;

	/** Used by FindLocalMove and RejectLocalMove */
	vector<int> SavedWeightsOfKanpsacks;

protected:
	/** Items placed in the knapsacks.
	*
	*	Value true means that the items is placed in all knapsacks */
	vector<bool> ItemsInKnapsack;

	/** Sum of item weights for each knapsack.
	*
	*	The fild is redundant but speeds up calculations */
	vector<int> WeightsOfKanpsacks;

	/** Returns true if the solution is feasible */
	bool Feasible ();

	/** Returns true if it is feasible to add the given item to all knapsacks */ 
	bool Fits (int iItem);

	/** Restores feasibility of the solution.
	*
	*	Removes items from knapsacks guided by the scalarizing
	*	function value */
	void RestoreFeasibility (TPoint& ReferencePoint);

	/** Fills the knapsacks completely (such that no item can be added).
	*
	*	Adds items to knapsacks guided by the scalarizing
	*	function value */
	void FillKnapsacks (TPoint& ReferencePoint);

	/** Cheks integrity of the solution.
	*
	*	Checks feasibility and values of WeightsOfKanpsacks and 
	*	ObjectiveValues fields */
	void Check ();

public:
	/** Constructs new random solution */
	TMOKPSolution ();

	/** Constructs new random solution and calls RestoreFeasibility and FllKnapsacks */
	TMOKPSolution (TScalarizingFunctionType ScalarizingFunctionType,
		TPoint& ReferencePoint, TWeightVector WeightVector,
		TNondominatedSet &NondominatedSet);

	/** Constructs new solution by recombination of Parent1 and Parent 2 
	*	and calls RestoreFeasibility and FllKnapsacks */
	TMOKPSolution (TMOKPSolution& Parent1, TMOKPSolution& Parent2,
		TScalarizingFunctionType ScalarizingFunctionType,
		TPoint& ReferencePoint, TWeightVector WeightVector,
		TNondominatedSet &NondominatedSet);

	/** Constructs new solution by recombination of Parent1 and Parent 2 */
	TMOKPSolution (TMOKPSolution& Parent1, TMOKPSolution& Parent2);

	/** Mutates the solution */
	virtual void Mutate ();

	/** Find next local move to be performed in local search type
	*	algorithms */
	virtual void FindLocalMove ();

	/**	Rejects the local move found by FindLocalMove () method */
	virtual void RejectLocalMove ();

	TMOKPSolution (TMOKPSolution& MOKPSolution);
};

#endif // !defined(AFX_TMOKPSOLUTION_H__3867D633_2451_11D4_8230_000000000000__INCLUDED_)
