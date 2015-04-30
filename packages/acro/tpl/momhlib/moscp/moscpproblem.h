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

// MSSCPProblem.h: interface for the TMSSCPProblem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOSCPPROBLEM_H__27901A31_39FE_416D_B49F_FCBCE524F713__INCLUDED_)
#define AFX_MPSCPPROBLEM_H__27901A31_39FE_416D_B49F_FCBCE524F713__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <set>

#include "problem.h"
#include "solution.h"
#include "weights.h"


typedef set <int> SetInt;

/** Set of integers between 0 and MaxSize - 1 */
class TIntSet {
private:
	SetInt::iterator Iterator;

	bool bIteratorInvalid;
protected:
	SetInt Items;

public:
	/** Maximum size of the set */
	int MaxSize;

	/** Returns current size (number of items) of the set */
	int Size () {
		return Items.size ();
	}

	/** Creates empty set prepared to store MaxSize items */
	TIntSet (unsigned int MaxSize) {
		this->MaxSize = MaxSize;
		bIteratorInvalid = true;
		Iterator = NULL;
	}

	/** Insert new item to the set 
	*
	*	Constraints:
	*	0 <= Item < MaxSize;
	*/
	virtual void Insert (int Item);

	/** Removes item from the set
	*
	*	If the item was not memeber of the set does nothing 
	*	Constraints:
	*	0 <= Item < MaxSize;
	*/
	virtual void Remove (int Item);

	/** Returns true if the given item is a member of the set 
	*
	*	Constraints:
	*	0 <= Item < MaxSize;
	*/
	virtual bool IsMember (int Item);

	/** Adds the given set to this */
	virtual void Add (TIntSet& Set);

	/** Subtracts the given set from this */
	virtual void Subtract (TIntSet& Set);

	/** Makes union of the given set and this */
	virtual void Union (TIntSet& Set);

	/** Return first item in the set - the item with the lowest value.
	*
	*	If the set is empty it returns -1
	*/
	int FirstItem ();

	/** Returns next item in the set - the next with respect to the
	*	item returned by FirstItem ()
	*
	*	The items returned by FirstItem () and NextItem are sorted
	*	in increasing order.
	*	If the last returned item was the last item in the set it returns -1.
	*/
	int NextItem ();

	/** Returns randomly selected item */
	int GetRandomItem ();

	/** Empties the set */
	void Empty ();

	/** Fills the set with all items from 0 to MaxSize - 1 */
	void MakeFull ();

	/** Copy operator */
	TIntSet& operator = (TIntSet& Set);

};

/** Subset of integers uses in multiple objective set covering problem */
class TMOSCPSubset : public TIntSet {
public:
	/** Costs of the subset */
	vector <int> Costs;

	// Constructs subset with space for MaxSize items */
	TMOSCPSubset (unsigned int MaxSize) : TIntSet (MaxSize) {
		Costs.resize (NumberOfObjectives);
	}
};

/** Multiple objective set covering problem */
class TMOSCPProblem : public TProblem
{

	friend class TMOSCPSolution; // The solution class is friend class of the problem class

protected:
	/** True if a problem have been already read */
	bool bProblemRead;

	/** Number of items in the base set */
	unsigned int NumberOfItems;

	/** Subsets of the problem */
	vector <TMOSCPSubset*> Subsets;

	/** For each item the set of subsets that cover it */
	vector <TIntSet*> CoveringSubsets;

public:
	/** Reads a problem instance from the given file.
	*
	*
	*	Format is based on http://www.univ-valenciennes.fr/ROAD/MCDM/
	*	It has been modified, however, by adding the number of objectives at the begining:
	*	Number of objectives (J)
	*	Number of items (rows) in the base set (k) 
	*	Number of subsets (columns) (I) 
	*	For each objective j (j=1,...,J) : the cost of each column c(i,j); i=1,...,n 
	*	for each row i (i=1,...,I) : the number of columns which cover row l followed by a list of the columns which cover row l 
	**/
  	virtual bool Load(char* FileName);

  	void Save(char* FileName);


	TMOSCPProblem() {
		bProblemRead = false;		
	}

	//* Frees allocated memory */
	virtual ~TMOSCPProblem();

};

// Makes Problem visible to all modules that include this header
extern TMOSCPProblem Problem;

#endif // !defined(AFX_MOSCPPROBLEM_H__27901A31_39FE_416D_B49F_FCBCE524F713__INCLUDED_)
