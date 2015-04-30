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

// tmoscpsolution.h: interface for the TMOSCPSolution class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TMOSCPSOLUTION_H__2831D546_FB21_4F57_B992_79E9CBE1C0EC__INCLUDED_)
#define AFX_TMOSCPSOLUTION_H__2831D546_FB21_4F57_B992_79E9CBE1C0EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MOMHSolution.h"
#include "moscpproblem.h"

/** Solution of multiple objective set covering problem */
class TMOSCPSolution : public TMOMHSolution
{
private:
	/* Used by FindLocalMove () and AcceptLocalMove () */
	int iSubsetToRemove;

	/* Used by FindLocalMove () and AcceptLocalMove () */
	TIntSet* SubsetsToInsert;
		
protected:
	/** For each item the set of indices of all covering subsets */
	vector <TIntSet*> CoveringSubsets;

	/** Set of indices off all selected subsets */
	TIntSet *SelectedSubsets;

	/** constructs random, possibly redundant solution */
	void ConstructRandomSolution ();

	/** Removes redundant subsets in greedy manner.
	*
	*	The greedy procedure is guided by scalarizing function defined by 
	*	this->ScalarizingFunctionType, ReferencePoint and this->WeightVector
	*/
	void RemoveRedundantSubsets (TPoint& ReferencePoint);

	/** Removes redundant subsets in random order.
	*/
	void RemoveRedundantSubsets ();

	/** Runs local search 
	*
	*	The local search is guided by scalarizing function
	*	defined by ScalarizingFunctionType, ReferencePoint
	*	and WeightVector.
	*/
	void LocalSearch (TPoint& ReferencePoint);

	/** Builds new solution by recombination of the parents */
	void Recombine (TMOMHSolution& Parent1, TMOMHSolution& Parent2);

public:
	/** Removes randomly selected subset and repairs the solution in greedy manner.
	*
	*	The subset removed cannot be inserted again.
	*	The greedy procedure is guided by scalarizing function defined by 
	*	this->ScalarizingFunctionType, ReferencePoint = *this and this->WeightVector.
	*	The move is not actually performed but defined by fields iSubsetToRemove
	*	and SubsetsToInsert.
	*	ObjectiveValues are updated.
	*/
	virtual void FindLocalMove ();

	/** Accepts the move found by FindLocalMove ()
	*
	*	Removes iSubsetToRemove and inserts SubsetsToInsert */
	virtual void AcceptLocalMove ();

	/** Rejects local move found by FindLocalMove ().
	*
	*	Just restores old ObjectiveValues */
	virtual void RejectLocalMove ();

	/** Saves the solution to an open Stream */
    virtual ostream& Save(ostream& Stream);             

	/** Copy operator */
	TMOSCPSolution& operator = (TMOSCPSolution& SourceSolution);

	/** Copy constructor */
	TMOSCPSolution (TMOSCPSolution& SourceSolution);

	/** Construct an initial soluion */
	TMOSCPSolution ();

	/** Constructs new random initial solution and calls 
	*	the greedy heuristic to remove redundant subsets. 
	*	The greedy heuristic is guided by scalarizing function
	*	defined by ScalarizingFunctionType, ReferencePoint
	*	and WeightVector.
	*	NondominatedSet is not updated */
	TMOSCPSolution (TScalarizingFunctionType ScalarizingFunctionType,
		TPoint& ReferencePoint, TWeightVector WeightVector, TNondominatedSet &NondominatedSet);

	/** Constructs new solution by recombination of the parents 
	*	and calls the greedy heuristic to remove redundant subsets. 
	*	The greedy heuristic is guided by scalarizing function
	*	defined by ScalarizingFunctionType, ReferencePoint
	*	and WeightVector.
	*	NondominatedSet is not updated */
	TMOSCPSolution (TMOMHSolution& Parent1, TMOMHSolution& Parent2,
		TScalarizingFunctionType ScalarizingFunctionType,
		TPoint& ReferencePoint, TWeightVector WeightVector, TNondominatedSet &NondominatedSet);

	/** Constructs new solution by recombination of the parents 
	*	and removes redundant subsets in random order */
	TMOSCPSolution (TMOMHSolution& Parent1, TMOMHSolution& Parent2);

	/** Frees allocated memory */
	virtual ~TMOSCPSolution () {
		delete SelectedSubsets;
		int iItem;
		for (iItem = 0; iItem < Problem.NumberOfItems; iItem++) 
			delete CoveringSubsets [iItem];
	}

};

#endif // !defined(AFX_TMOSCPSOLUTION_H__2831D546_FB21_4F57_B992_79E9CBE1C0EC__INCLUDED_)
