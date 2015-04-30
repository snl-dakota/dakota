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

// TMOKPSolution.cpp: implementation of the TMOKPSolution class.
//
//////////////////////////////////////////////////////////////////////

#include "mokpsolution.h"
#include "mokpproblem.h"
#include "nondominatedset.h"
#include "tlistset.h"

TMOKPSolution::TMOKPSolution (TMOKPSolution& MOKPSolution)  : TMOMHSolution (MOKPSolution)
{
	ItemsInKnapsack.resize (Problem.NumberOfItems);
	WeightsOfKanpsacks.resize (Problem.NumberOfKnapsacks);
	int iKnap; for (iKnap = 0; iKnap < Problem.NumberOfKnapsacks; iKnap++) {
		WeightsOfKanpsacks [iKnap] = MOKPSolution.WeightsOfKanpsacks [iKnap];
	}
	int iItem; for (iItem = 0; iItem < Problem.NumberOfItems; iItem++) {
		ItemsInKnapsack [iItem] = MOKPSolution.ItemsInKnapsack [iItem];
	}

#ifdef _DEBUG
	Check ();
#endif
}

TMOKPSolution::TMOKPSolution () {
	ItemsInKnapsack.resize (Problem.NumberOfItems);
	WeightsOfKanpsacks.resize (Problem.NumberOfKnapsacks);
	int iKnap; for (iKnap = 0; iKnap < Problem.NumberOfKnapsacks; iKnap++) {
		WeightsOfKanpsacks [iKnap] = 0;
		ObjectiveValues [iKnap] = 0;
	}

	int iItem; for (iItem = 0; iItem < Problem.NumberOfItems; iItem++) {
		if (rand () % 2 == 0)
			ItemsInKnapsack [iItem] = false;
		else
			ItemsInKnapsack [iItem] = true;
		if (ItemsInKnapsack [iItem]) {
			for (iKnap = 0; iKnap < Problem.NumberOfKnapsacks; iKnap++) {
				WeightsOfKanpsacks [iKnap] += Problem.Knapsacks [iKnap][iItem].Weight;
				ObjectiveValues [iKnap] += Problem.Knapsacks [iKnap][iItem].Profit;
			}
		}
	}

	ScalarizingFunctionType = _Linear;

	TPoint TempReferencePoint (*this);
	RestoreFeasibility (TempReferencePoint);

#ifdef _DEBUG
	Check ();
#endif
}

TMOKPSolution::TMOKPSolution (TScalarizingFunctionType ScalarizingFunctionType,
		TPoint& ReferencePoint, TWeightVector WeightVector, TNondominatedSet &NondominatedSet)  : TMOMHSolution (ScalarizingFunctionType, ReferencePoint, WeightVector, (TNondominatedSet &)NondominatedSet)
{
	ItemsInKnapsack.resize (Problem.NumberOfItems);
	WeightsOfKanpsacks.resize (Problem.NumberOfKnapsacks);
	int iKnap; for (iKnap = 0; iKnap < Problem.NumberOfKnapsacks; iKnap++) {
		WeightsOfKanpsacks [iKnap] = 0;
		ObjectiveValues [iKnap] = 0;
	}

	int iItem; for (iItem = 0; iItem < Problem.NumberOfItems; iItem++) 
		ItemsInKnapsack [iItem] = false;

	FillKnapsacks (ReferencePoint);

#ifdef _DEBUG
	Check ();
#endif
}
	
TMOKPSolution::TMOKPSolution (TMOKPSolution& Parent1, TMOKPSolution& Parent2,
		TScalarizingFunctionType ScalarizingFunctionType,
		TPoint& ReferencePoint, TWeightVector WeightVector,
		TNondominatedSet &NondominatedSet) : 
			TMOMHSolution (Parent1, Parent2, ScalarizingFunctionType, ReferencePoint, WeightVector, (TNondominatedSet &)NondominatedSet)
{
	ItemsInKnapsack.resize (Problem.NumberOfItems);
	WeightsOfKanpsacks.resize (Problem.NumberOfKnapsacks);
	int iKnap; for (iKnap = 0; iKnap < Problem.NumberOfKnapsacks; iKnap++) {
		WeightsOfKanpsacks [iKnap] = 0;
		ObjectiveValues [iKnap] = 0;
	}

	int iCrossOverPoint = rand () % (Problem.NumberOfItems - 1);

	int iItem; for (iItem = 0; iItem < Problem.NumberOfItems; iItem++) {
		if (iItem <= iCrossOverPoint)
			ItemsInKnapsack [iItem] = (Parent1).ItemsInKnapsack [iItem];
		else
			ItemsInKnapsack [iItem] = (Parent2).ItemsInKnapsack [iItem];
		if (rand () % 100 == 0)
			ItemsInKnapsack [iItem] = !ItemsInKnapsack [iItem];
		if (ItemsInKnapsack [iItem]) {
			for (iKnap = 0; iKnap < Problem.NumberOfKnapsacks; iKnap++) {
				WeightsOfKanpsacks [iKnap] += Problem.Knapsacks [iKnap][iItem].Weight;
				ObjectiveValues [iKnap] += Problem.Knapsacks [iKnap][iItem].Profit;
			}
		}
	}

	RestoreFeasibility (ReferencePoint);

	FillKnapsacks (ReferencePoint);

#ifdef _DEBUG
	Check ();
#endif
}

TMOKPSolution::TMOKPSolution (TMOKPSolution& Parent1, TMOKPSolution& Parent2) : 
			TMOMHSolution (Parent1, Parent2)
{
	ItemsInKnapsack.resize (Problem.NumberOfItems);
	WeightsOfKanpsacks.resize (Problem.NumberOfKnapsacks);
	int iKnap; for (iKnap = 0; iKnap < Problem.NumberOfKnapsacks; iKnap++) {
		WeightsOfKanpsacks [iKnap] = 0;
		ObjectiveValues [iKnap] = 0;
	}

	int iCrossOverPoint = rand () % (Problem.NumberOfItems - 1);

	int iItem; for (iItem = 0; iItem < Problem.NumberOfItems; iItem++) {
		if (iItem <= iCrossOverPoint)
			ItemsInKnapsack [iItem] = (Parent1).ItemsInKnapsack [iItem];
		else
			ItemsInKnapsack [iItem] = (Parent2).ItemsInKnapsack [iItem];
		if (rand () % 100 == 0)
			ItemsInKnapsack [iItem] = !ItemsInKnapsack [iItem];
		if (ItemsInKnapsack [iItem]) {
			for (iKnap = 0; iKnap < Problem.NumberOfKnapsacks; iKnap++) {
				WeightsOfKanpsacks [iKnap] += Problem.Knapsacks [iKnap][iItem].Weight;
				ObjectiveValues [iKnap] += Problem.Knapsacks [iKnap][iItem].Profit;
			}
		}
	}

	WeightVector = GetRandomWeightVector ();

	RestoreFeasibility (*this);

	FillKnapsacks (*this);

#ifdef _DEBUG
	Check ();
#endif
}

void TMOKPSolution::FindLocalMove () {
	// Save current solution
	SaveObjectiveValues ();
	SavedItemsInKnapsack.resize (Problem.NumberOfItems);
	SavedWeightsOfKanpsacks.resize (Problem.NumberOfKnapsacks);
	SavedItemsInKnapsack = ItemsInKnapsack;
	SavedWeightsOfKanpsacks = WeightsOfKanpsacks;

	// Mutation as in Knowles and Corne
	int iItem; for (iItem = 0; iItem < Problem.NumberOfItems; iItem++) {
		if (rand () % (Problem.NumberOfItems / 4) == 0) {
			if (ItemsInKnapsack [iItem]) {
				ItemsInKnapsack [iItem] = false;
				int iKnap; for (iKnap = 0; iKnap < Problem.NumberOfKnapsacks; iKnap++) {
					WeightsOfKanpsacks [iKnap] -= Problem.Knapsacks [iKnap][iItem].Weight;
					ObjectiveValues [iKnap] -= Problem.Knapsacks [iKnap][iItem].Profit;
				}
			}
			else {
				ItemsInKnapsack [iItem] = true;
				int iKnap; for (iKnap = 0; iKnap < Problem.NumberOfKnapsacks; iKnap++) {
					WeightsOfKanpsacks [iKnap] += Problem.Knapsacks [iKnap][iItem].Weight;
					ObjectiveValues [iKnap] += Problem.Knapsacks [iKnap][iItem].Profit;
				}
			}
		}
	}

	RestoreFeasibility (*this);

#ifdef _DEBUG
	Check ();
#endif
}

void TMOKPSolution::Mutate () {
	// Mutation as in Knowles and Corne
	int iItem; for (iItem = 0; iItem < Problem.NumberOfItems; iItem++) {
		if (rand () % (Problem.NumberOfItems / 4) == 0) {
			if (ItemsInKnapsack [iItem]) {
				ItemsInKnapsack [iItem] = false;
				int iKnap; for (iKnap = 0; iKnap < Problem.NumberOfKnapsacks; iKnap++) {
					WeightsOfKanpsacks [iKnap] -= Problem.Knapsacks [iKnap][iItem].Weight;
					ObjectiveValues [iKnap] -= Problem.Knapsacks [iKnap][iItem].Profit;
				}
			}
			else {
				ItemsInKnapsack [iItem] = true;
				int iKnap; for (iKnap = 0; iKnap < Problem.NumberOfKnapsacks; iKnap++) {
					WeightsOfKanpsacks [iKnap] += Problem.Knapsacks [iKnap][iItem].Weight;
					ObjectiveValues [iKnap] += Problem.Knapsacks [iKnap][iItem].Profit;
				}
			}
		}
	}

	RestoreFeasibility (*this);

#ifdef _DEBUG
	Check ();
#endif
}

void TMOKPSolution::RejectLocalMove () {
	// Restore previous solution (saved in FindLocalMove)
	RestoreObjectiveValues ();
	ItemsInKnapsack = SavedItemsInKnapsack;
	WeightsOfKanpsacks = SavedWeightsOfKanpsacks;
}

bool TMOKPSolution::Feasible()
{
	bool bFeasible = true;

	// Check if none knapsack is overloaded
	int iKnap; for (iKnap = 0; bFeasible && (iKnap < Problem.NumberOfKnapsacks); iKnap++)  {
		bFeasible = WeightsOfKanpsacks [iKnap] <= Problem.Knapsacks [iKnap].Capacity;
	}

	return bFeasible;
}

bool TMOKPSolution::Fits (int iItem) {
	bool bFits = true;

	// Check if the given item fits in each knapsack
	int iKnap; for (iKnap = 0; bFits && (iKnap < Problem.NumberOfKnapsacks); iKnap++) 
		bFits = WeightsOfKanpsacks [iKnap] + Problem.Knapsacks [iKnap][iItem].Weight < Problem.Knapsacks [iKnap].Capacity;

	return bFits;
}

void TMOKPSolution::RestoreFeasibility (TPoint& ReferencePoint) {
	while (!Feasible ()) {
		TSolution Solution (*this);
		
		double CurrentValue = Solution.ScalarizingFunction (ReferencePoint);
		
		double BestRemovalEvaluation = 1e30;
		int iBestItem;
		int iItem; for (iItem = 0; iItem < Problem.NumberOfItems; iItem++) {
			if (ItemsInKnapsack [iItem]) {
				double WeightChange = 0;
				int iKnap; for (iKnap = 0; iKnap < Problem.NumberOfKnapsacks; iKnap++) {
					Solution.ObjectiveValues [iKnap] = ObjectiveValues [iKnap] - Problem.Knapsacks [iKnap][iItem].Profit;
					if (WeightsOfKanpsacks [iKnap] - Problem.Knapsacks [iKnap].Capacity > 0)
						WeightChange += (double)Problem.Knapsacks [iKnap][iItem].Weight * (WeightsOfKanpsacks [iKnap] - Problem.Knapsacks [iKnap].Capacity);
				}
				
				double NewValue = Solution.ScalarizingFunction (ReferencePoint);;
				
				double ItemRemovalEvaluation = (NewValue - CurrentValue) / WeightChange;
				if (ItemRemovalEvaluation < BestRemovalEvaluation) {
					BestRemovalEvaluation = ItemRemovalEvaluation;
					iBestItem = iItem;
				}
			}
		}

		ItemsInKnapsack [iBestItem] = false;
		int iKnap; for (iKnap = 0; iKnap < Problem.NumberOfKnapsacks; iKnap++) {
			WeightsOfKanpsacks [iKnap] -= Problem.Knapsacks [iKnap][iBestItem].Weight;
			ObjectiveValues [iKnap] -= Problem.Knapsacks [iKnap][iBestItem].Profit;
		}
		
	}
}

void TMOKPSolution::FillKnapsacks (TPoint& ReferencePoint) {

	// Find items fitting in all knapsacks
	vector<int> FittingItems;
	int iItem; for (iItem = 0; iItem < Problem.NumberOfItems; iItem++) {
		if (!ItemsInKnapsack [iItem] && Fits (iItem)) {
			FittingItems.push_back (iItem);
		}
	}

	// While there are fitting items
	while (FittingItems.size () > 0) {
		TSolution Solution (*this);

		double CurrentValue = Solution.ScalarizingFunction (ReferencePoint);
		
		// Find the best item for insertion
		double BestInsertionEvaluation = -1e30;
		int iBestItem;
		int iItemIndex; for (iItemIndex = 0; iItemIndex < FittingItems.size (); iItemIndex++) {
			int iItem = FittingItems [iItemIndex];
			
			double WeightChange = 0;
			int iKnap; for (iKnap = 0; iKnap < Problem.NumberOfKnapsacks; iKnap++) {
				Solution.ObjectiveValues [iKnap] = ObjectiveValues [iKnap] + Problem.Knapsacks [iKnap][iItem].Profit;
				WeightChange += (double)Problem.Knapsacks [iKnap][iItem].Weight / (Problem.Knapsacks [iKnap].Capacity - WeightsOfKanpsacks [iKnap] + 1);
			}
			

			double NewValue = Solution.ScalarizingFunction (ReferencePoint);

			double ItemInsertionEvaluation = (CurrentValue - NewValue) / WeightChange;
			if (ItemInsertionEvaluation > BestInsertionEvaluation) {
				BestInsertionEvaluation = ItemInsertionEvaluation;
				iBestItem = iItemIndex;
			}
			
		}

		// Insert the item
		ItemsInKnapsack [FittingItems [iBestItem]] = true;
		{
		int iKnap; for (iKnap = 0; iKnap < Problem.NumberOfKnapsacks; iKnap++) {
			WeightsOfKanpsacks [iKnap] += Problem.Knapsacks [iKnap][FittingItems [iBestItem]].Weight;
			ObjectiveValues [iKnap] += Problem.Knapsacks [iKnap][FittingItems [iBestItem]].Profit;
		}
		}

		// Update the set of items fitting in all knapsacks
		FittingItems.erase (FittingItems.begin () + iBestItem);
		for (iItemIndex = 0; iItemIndex < FittingItems.size (); ) {
			int iItem = FittingItems [iItemIndex];
			if (!Fits (iItem))
				FittingItems.erase (FittingItems.begin () + iItemIndex);
			else
				iItemIndex++;

		}
	}
}

void TMOKPSolution::Check () {
	vector<int> WeightsOfKanpsacksCkeck;
	vector<int> ObjectiveValuesCkeck;

	WeightsOfKanpsacksCkeck.resize (NumberOfObjectives, 0);
	ObjectiveValuesCkeck.resize (NumberOfObjectives, 0);

	int iItem; for (iItem = 0; iItem < Problem.NumberOfItems; iItem++) {
		if (ItemsInKnapsack [iItem]) {
			int iKnap; for (iKnap = 0; iKnap < Problem.NumberOfKnapsacks; iKnap++) {
				ObjectiveValuesCkeck [iKnap] += Problem.Knapsacks [iKnap][iItem].Profit;
				WeightsOfKanpsacksCkeck [iKnap] += Problem.Knapsacks [iKnap][iItem].Weight;
			}
		}
	}

	int iKnap; for (iKnap = 0; iKnap < Problem.NumberOfKnapsacks; iKnap++) {
		if (ObjectiveValuesCkeck [iKnap] != ObjectiveValues [iKnap]) {
			cout << "void TMOKPSolution::Check ()\n";
			cout << "ObjectiveValuesCkeck [iKnap] != ObjectiveValues [iKnap]\n";
			exit (0);
		}
		if (WeightsOfKanpsacksCkeck [iKnap] != WeightsOfKanpsacks [iKnap]) {
			cout << "void TMOKPSolution::Check ()\n";
			cout << "WeightsOfKanpsacksCkeck [iKnap] != WeightsOfKanpsacks [iKnap]\n";
			exit (0);
		}
		if (WeightsOfKanpsacks [iKnap] > Problem.Knapsacks [iKnap].Capacity) {
			cout << "void TMOKPSolution::Check ()\n";
			cout << "WeightsOfKanpsacks [iKnap] > Problem.Knapsacks [iKnap].Capacity\n";
			exit (0);
		}
	}
}

