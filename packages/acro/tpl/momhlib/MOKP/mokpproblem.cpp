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

// TMOKPProblem.cpp: implementation of the TMOKPProblem class.
//
//////////////////////////////////////////////////////////////////////

#include "mokpproblem.h"

#include <string.h>


//* Global variable declared external in TMOKPProblem.h */
TMOKPProblem Problem;

bool TMOKPProblem::Load(char* FileName) {

	// Open stream for reading
	fstream Stream (FileName, ios::in);

	// Just skip the first line
	char c;
	do {
		Stream.get (c);
	}
	while (c != '\n');

	// Check if the next token is "="
	char s [100];
	Stream >> s;
	if (strcmp (s, "=") != 0) {
		Stream.close ();
		return false;
	}

	// Clear the set of knapsacks
	Knapsacks.clear ();

	// Check if the next token is "knapsack"
	Stream >> s;
	if (strcmp (s, "knapsack") != 0) {
		Stream.close ();
		return false;
	}

	// Read data of all knapsacks
	int iKnapsack = 0;
	while (strcmp (s, "knapsack") == 0) {

		// Skip the rest of the line
		char c;
		do {
			Stream.get (c);
		}
		while (c != '\n');
		
		// Add ne empty knapsack
		TKnapsack Knapsack;
		Knapsacks.push_back (Knapsack);

		// Check if the next token is "capacity"
		Stream >> s;
		if (strcmp (s, "capacity:") != 0) {
			Stream.close ();
			return false;
		}
		
		// Read capacity of the knapsack
		double Capacity;
		Stream >> Capacity;
		if (Stream.rdstate () != ios::goodbit) {
			Stream.close ();
			return false;
		}
		if (Capacity <= 0) {
			Stream.close ();
			return false;
		}
		Knapsacks [iKnapsack].Capacity = Capacity;

		// Read all items
		int iItem = 0;
		Stream >> s;
		while (strcmp (s, "item") == 0) {
			// Read the rest of the line
			char c;
			do {
				Stream.get (c);
			}
			while (c != '\n');

			// Read data of the item
			Stream >> s;
			if (strcmp (s, "weight:") != 0) {
				Stream.close ();
				return false;
			}
			int Weight;
			Stream >> Weight;
			if (Stream.rdstate () != ios::goodbit) {
				Stream.close ();
				return false;
			}
			if (Weight <= 0) {
				Stream.close ();
				return false;
			}

			Stream >> s;
			if (strcmp (s, "profit:") != 0) {
				Stream.close ();
				return false;
			}
			int Profit;
			Stream >> Profit;
			if (Stream.rdstate () != ios::goodbit) {
				Stream.close ();
				return false;
			}
			if (Profit <= 0) {
				Stream.close ();
				return false;
			}

			// Add new item to the knapsack
			TItem Item;
			Item.Weight = Weight;
			Item.Profit = Profit;
			Knapsacks [iKnapsack].push_back (Item);

			iItem++;
		
			Stream >> s;
		}

		iKnapsack++;

		Stream >> s;
	}

	// Check if the number of knapsacks is > 0
	NumberOfKnapsacks = Knapsacks.size ();
	if (NumberOfKnapsacks == 0) {
		Stream.close ();
		return false;
	}

	// Check if the number of items in the first knapsack is > 0
	NumberOfItems = Knapsacks [0].size ();
	if (NumberOfItems == 0) {
		Stream.close ();
		return false;
	}

	// Check if the number of items in all knapsacks is equal 
	// to the number in the first knapsack 
	int i; for (i = 1; i < NumberOfKnapsacks; i++) {
		if (Knapsacks [i].size () != NumberOfItems) {
			Stream.close ();
			return false;
		}
	}

	Stream.close ();

	// Set global variables NumberOfObjectives and Objectives
	NumberOfObjectives = NumberOfKnapsacks;

	Objectives.resize (NumberOfObjectives);
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		Objectives [iobj].ObjectiveType = _Max;
		Objectives [iobj].bActive = true;
	}

	return true;
}
