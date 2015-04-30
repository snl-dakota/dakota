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

// TMOKPProblem.h: interface for the TMOKPProblem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TMOKPPROBLEM_H__3867D634_2451_11D4_8230_000000000000__INCLUDED_)
#define AFX_TMOKPPROBLEM_H__3867D634_2451_11D4_8230_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "problem.h"

/** Item characterized by its weights and profit */
class TItem {
public:
	int Weight;

	int Profit;
};

/** Knapsack - set of items to be potentially placed in knapsack
*	characterized by its capacity */
class TKnapsack : public vector<TItem> {
public:
	double Capacity;
};

/** Instance of the multiple objective knapsack problem */
class TMOKPProblem : public TProblem
{
/** TMOKPSolution is granted full access to memebers of TMOKPProblem */
friend class TMOKPSolution;

protected:
	int NumberOfKnapsacks;

	int NumberOfItems;

	vector<TKnapsack> Knapsacks;
public:
	/** Loads the instance data from a file with the given name 
	*
	*	Returns true if the instance is read successfully */
    virtual bool Load(char* FileName);
};

/** Declaration of external Problem variable (defined in TMOKPProblem.cpp)
*	makes it accessible to all modules that include this header */
extern TMOKPProblem Problem;

#endif // !defined(AFX_TMOKPPROBLEM_H__3867D634_2451_11D4_8230_000000000000__INCLUDED_)
