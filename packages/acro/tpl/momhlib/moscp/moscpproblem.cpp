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

// MSSCPProblem.cpp: implementation of the TMSSCPProblem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Moscpproblem.h"

// The only variable of TMOSCPProblem type
TMOSCPProblem Problem;

void TIntSet::Insert (int Item) {
#ifdef _DEBUG
	if ((Item < 0) || (Item >= MaxSize)) {
		cout << "void TSet::Insert (int Item)\n";
		cout << "(Item < 0) || (Item >= MaxSize)\n";
		cout << "Item = " << Item << '\n';
		exit (0);
	}
#endif
	Items.insert (Item);
}

void TIntSet::Remove (int Item) {
#ifdef _DEBUG
	if ((Item < 0) || (Item >= MaxSize)) {
		cout << "void TSet::Remove (int Item)\n";
		cout << "(Item < 0) || (Item >= MaxSize)\n";
		cout << "Item = " << Item << '\n';
		exit (0);
	}
#endif
	if (!bIteratorInvalid) {
		if (*Iterator == Item) {
			if (Iterator == Items.begin ()) {
				bIteratorInvalid = true;
			}
			else {
				Iterator--;

			}
		}
	}

	Items.erase (Item);
}

bool TIntSet::IsMember (int Item) {
#ifdef _DEBUG
	if ((Item < 0) || (Item >= MaxSize)) {
		cout << "void TSet::IsMember (int Item)\n";
		cout << "(Item < 0) || (Item >= MaxSize)\n";
		cout << "Item = " << Item << '\n';
		exit (0);
	}
#endif
	return (Items.find (Item) != Items.end ());
}

void TIntSet::Add (TIntSet& Set) {
	int Item;
	for (Item = Set.FirstItem (); Item >= 0; Item = Set.NextItem ())
		Insert (Item);
}

void TIntSet::Subtract (TIntSet& Set) {
	int Item;
	for (Item = Set.FirstItem (); Item >= 0; Item = Set.NextItem ())
		Remove (Item);
}

void TIntSet::Union (TIntSet& Set) {
	int Item;
	for (Item = FirstItem (); Item >= 0; Item = NextItem ())
		if (!Set.IsMember (Item))
			Remove (Item);
}

int TIntSet::FirstItem () {
	Iterator = Items.begin ();
	bIteratorInvalid = false;
	if (Iterator != Items.end ()) {
		return *Iterator;
	}
	else {
		return -1;
	}
}

int TIntSet::NextItem () {
	if (bIteratorInvalid) {
		bIteratorInvalid = false;
		Iterator = Items.begin ();
	}
	else
		Iterator++;
	if (Iterator != Items.end ())
		return *Iterator;
	else {
		bIteratorInvalid = true;
		return -1;
	}
}

int TIntSet::GetRandomItem () {
	if (Size () == 0)
		return -1;

	int iPos = rand () % Size ();

	int iItem = FirstItem ();
	int i;
	for (i = 0; i < iPos; i++) 
		iItem = NextItem ();

	return iItem;
}

void TIntSet::Empty () {
	bIteratorInvalid = true;
	Items.clear ();
}
	
void TIntSet::MakeFull () {
	Items.clear ();
	int iItem;
	for (iItem = 0; iItem < MaxSize; iItem++)
		Insert (iItem);
}


TIntSet& TIntSet::operator = (TIntSet& Set) {
	MaxSize = Set.MaxSize;
	Iterator = Items.end ();
	Items = Set.Items;

	return *this;
}

void TMOSCPProblem::Save(char* FileName) {
	// Open stream for writting
	fstream Stream (FileName, ios::out);

	Stream << NumberOfObjectives << '\n';
	Stream << Problem.NumberOfItems << '\n';
	Stream << Problem.Subsets.size () << '\n';

	int iSubset;
	
	for (iSubset = 0; iSubset < Subsets.size (); iSubset++) {
		int iobj;
		for (iobj = 0; iobj < NumberOfObjectives; iobj++)
			Stream << Subsets [iSubset]->Costs [iobj] << '\t';
		int iItem;
		for (iItem = 0; iItem < Problem.NumberOfItems; iItem++) {
			if (Subsets [iSubset]->IsMember (iItem))
				Stream << "1\t";
			else
				Stream << "0\t";
		}
		Stream << '\n';
	}

	Stream.close ();
}

bool TMOSCPProblem::Load(char* FileName) {
	// If the problem was already read
	if (bProblemRead) {
		// Free allocated memeory
		int i;
		for (i = 0; i < NumberOfItems; i++)
			delete CoveringSubsets [i];
		
		for (i = 0; i < Subsets.size (); i++)
			delete Subsets [i];
	}

	// Open stream for reading
	fstream Stream (FileName, ios::in);

	Stream >> NumberOfObjectives;
	if (Stream.rdstate () != ios::goodbit) {
		Stream.close ();
		return false;
	}

	Stream >> NumberOfItems;
	if (Stream.rdstate () != ios::goodbit) {
		Stream.close ();
		return false;
	}

	unsigned int NumberOfSubsets;
	Stream >> NumberOfSubsets;
	if (Stream.rdstate () != ios::goodbit) {
		Stream.close ();
		return false;
	}

	// Allocate CoveringSubsets
	int i; 
	CoveringSubsets.resize (NumberOfItems);
	for (i = 0; i < NumberOfItems; i++)
		CoveringSubsets [i] = new TIntSet (NumberOfSubsets);

	// Allocate Subsets
	Subsets.resize (NumberOfSubsets);
	for (i = 0; i < NumberOfSubsets; i++)
		Subsets [i] = new TMOSCPSubset (NumberOfItems);

	int iobj;
	for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		for (i = 0; i < NumberOfSubsets; i++) {
			Stream >> Subsets [i]->Costs [iobj];
			if (Stream.rdstate () != ios::goodbit) {
				Stream.close ();
				return false;
			}
		}
	}

	for (i = 0; i < NumberOfItems; i++) {
		unsigned int NumberOfCoveringSubsets;
		Stream >> NumberOfCoveringSubsets;
		if (Stream.rdstate () != ios::goodbit) {
			Stream.close ();
			return false;
		}
		int j;
		for (j = 0; j < NumberOfCoveringSubsets; j++) {
			unsigned int iSubset;
			Stream >> iSubset;
			if (Stream.rdstate () != ios::goodbit) {
				Stream.close ();
				return false;
			}
			// iSubset - 1 because in the files subsets are indexed starting from 1
			CoveringSubsets [i]->Insert (iSubset - 1);
			Subsets [iSubset - 1]->Insert (i);
		}
	}

	Objectives.resize (NumberOfObjectives);
	for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		Objectives [iobj].ObjectiveType = _Min;
		Objectives [iobj].bActive = true;
	}

	bProblemRead = true;

	return true;
}

TMOSCPProblem::~TMOSCPProblem()
{
	// Free allocated memeory
	int i;
	for (i = 0; i < NumberOfItems; i++)
		delete CoveringSubsets [i];

	for (i = 0; i < Subsets.size (); i++)
		delete Subsets [i];
}

