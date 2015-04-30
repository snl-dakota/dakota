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

// tmoscpsolution.cpp: implementation of the TMOSCPSolution class.
//
//////////////////////////////////////////////////////////////////////

#include "tmoscpsolution.h"
#include "nondominatedset.h"

void TMOSCPSolution::ConstructRandomSolution () {
	// Initialize objective values
	int iobj;
	for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		ObjectiveValues [iobj] = 0;
	}

	// Mark all items as uncovered yet
	vector <bool> CoveredItems;
	CoveredItems.resize (Problem.NumberOfItems, false);

	int iItem;
	
	// For each item starting from random item
	int StartingItem = rand () % Problem.NumberOfItems;
	int i1;
	for (i1 = 0; i1 < Problem.NumberOfItems; i1++) {
		iItem = (StartingItem + i1) % Problem.NumberOfItems;

		// If the item has not been covered yet
		if (!CoveredItems [iItem]) {
			
			// Draw a subset covering iItem
			int iSubset = Problem.CoveringSubsets [iItem]->GetRandomItem ();

			CoveredItems [iItem] = true;
			
			// If iSubset has not been selected yet
			if (!SelectedSubsets->IsMember (iSubset)) {

				// Cover all memebers of iSubset
				int iItem2;
				for (iItem2 = Problem.Subsets [iSubset]->FirstItem ();
				iItem2 >= 0;
				iItem2 = Problem.Subsets [iSubset]->NextItem ()) {
					CoveringSubsets [iItem2]->Insert (iSubset);
					if (!CoveredItems [iItem2]) {
						CoveredItems [iItem2] = true;
					}
				}
								
				SelectedSubsets->Insert (iSubset);

				// Update objective values
				for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
					ObjectiveValues [iobj] += Problem.Subsets [iSubset]->Costs [iobj];
				}
			}
				
		}
	}
}

void TMOSCPSolution::Recombine (TMOMHSolution& Parent1, TMOMHSolution& Parent2) {
	// Initialize objective values
	int iobj;
	for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		ObjectiveValues [iobj] = 0;
	}

	// For each subset in Parent1
	int iSubset;
	for (iSubset = ((TMOSCPSolution&)Parent1).SelectedSubsets->FirstItem ();
	iSubset >= 0;
	iSubset = ((TMOSCPSolution&)Parent1).SelectedSubsets->NextItem ()) {
		// If the subset appears in both parents
		if (((TMOSCPSolution&)Parent2).SelectedSubsets->IsMember (iSubset))
			SelectedSubsets->Insert (iSubset);
		else
			// Select the subset with 50% probability
			if (rand () % 2 == 0)
				SelectedSubsets->Insert (iSubset);
	}


	// For each subset in Parent2
	for (iSubset = ((TMOSCPSolution&)Parent2).SelectedSubsets->FirstItem ();
	iSubset >= 0;
	iSubset = ((TMOSCPSolution&)Parent2).SelectedSubsets->NextItem ()) {
		// If the subset does not appear in both parents
		if (!((TMOSCPSolution&)Parent1).SelectedSubsets->IsMember (iSubset))
			if (rand () % 2 == 0)
				SelectedSubsets->Insert (iSubset);
	}


	// Mark all items as uncovered yet
	vector <bool> ItemCovered;
	ItemCovered.resize (Problem.NumberOfItems, false);

	// Mark items covered by selected subsets
	for (iSubset = SelectedSubsets->FirstItem ();
	iSubset >= 0;
	iSubset = SelectedSubsets->NextItem ()) {
		// Cover all memebers of iSubset
		int iItem;
		for (iItem = Problem.Subsets [iSubset]->FirstItem ();
		iItem >= 0;
		iItem = Problem.Subsets [iSubset]->NextItem ()) {
			CoveringSubsets [iItem]->Insert (iSubset);
			ItemCovered [iItem] = true;
		}
								
		for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
			ObjectiveValues [iobj] += Problem.Subsets [iSubset]->Costs [iobj];
		}
	}

	// Add cover for uncovered items
	int iItem;
	// For each item
	for (iItem = 0; iItem < Problem.NumberOfItems; iItem++) {
		// Mutate cover of the item wth 5% probability
		if (rand () % 20 == 0) {
			iSubset = Problem.CoveringSubsets [iItem]->GetRandomItem ();

			SelectedSubsets->Insert (iSubset);

			int iItem2;
			for (iItem2 = Problem.Subsets [iSubset]->FirstItem ();
			iItem2 >= 0;
			iItem2 = Problem.Subsets [iSubset]->NextItem ()) {
				CoveringSubsets [iItem2]->Insert (iSubset);
				ItemCovered [iItem2] = true;
			}
			
			for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
				ObjectiveValues [iobj] += Problem.Subsets [iSubset]->Costs [iobj];
			}
		}
		// If the item is not covered yet
		else if (!ItemCovered [iItem]) {
			// Cover the item with a subset that covers it in a randomly selected parent
			if (rand () % 2 == 0)
				iSubset = ((TMOSCPSolution&)Parent1).CoveringSubsets [iItem]->GetRandomItem ();
			else
				iSubset = ((TMOSCPSolution&)Parent2).CoveringSubsets [iItem]->GetRandomItem ();

			SelectedSubsets->Insert (iSubset);

			// Cover all items covered by iSubset
			int iItem2;
			for (iItem2 = Problem.Subsets [iSubset]->FirstItem ();
			iItem2 >= 0;
			iItem2 = Problem.Subsets [iSubset]->NextItem ()) {
				CoveringSubsets [iItem2]->Insert (iSubset);
				ItemCovered [iItem2] = true;
			}
			
			// Update objective values
			for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
				ObjectiveValues [iobj] += Problem.Subsets [iSubset]->Costs [iobj];
			}
		}
	}
}

void TMOSCPSolution::FindLocalMove () {
	SubsetsToInsert->Empty ();

	SaveObjectiveValues ();

	// Draw at arndom a subset to remove
	iSubsetToRemove = SelectedSubsets->GetRandomItem ();

	// Evaluate removal of the subset
	int iobj;
	for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		ObjectiveValues [iobj] -= Problem.Subsets [iSubsetToRemove]->Costs [iobj];
	}
	
	// Find the set of uncovered items
	TIntSet UncoveredItems (Problem.NumberOfItems);
	int iItem;
	for (iItem = Problem.Subsets [iSubsetToRemove]->FirstItem ();
	iItem >= 0;
	iItem = Problem.Subsets [iSubsetToRemove]->NextItem ()) {
		if (CoveringSubsets [iItem]->Size () == 1) {
			UncoveredItems.Insert (iItem);
		}
	}
	
	// In greedy manner find subsets repairing the solution
	while (UncoveredItems.Size () > 0) {
		double UnfeasibleScalarizingFunctionValue = ScalarizingFunction (*this);
		int iBestSubset;
		double BestEvaluation;
		vector <bool> SubsetTested;
		SubsetTested.resize (Problem.Subsets.size (), false);
		
		iItem = UncoveredItems.FirstItem ();
		
		// For each uncovered item
		bool bFirst = true;
		for (iItem = UncoveredItems.FirstItem ();
		iItem >= 0;
		iItem = UncoveredItems.NextItem ()) {
			// For each subset that covers iItem
			int iSubset2;
			for (iSubset2 = Problem.CoveringSubsets[iItem]->FirstItem ();
			iSubset2 >= 0;
			iSubset2 = Problem.CoveringSubsets [iItem]->NextItem ()) {
				// If iSubset has not been selected for removal and has not been tested yet
				if ((iSubset2 != iSubsetToRemove ) && !SubsetTested [iSubset2]) {
					// Consider insertion of iSubset2
					TSolution NewSolution (*this);
					
					SubsetTested [iSubset2] = true;
					for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
						NewSolution.ObjectiveValues [iobj] += Problem.Subsets [iSubset2]->Costs [iobj];
					}
					
					double NewScalarizingFunctionValue = NewSolution.ScalarizingFunction (*this);
					
					// Find the number of uncovered items covered by iSubset2
					int NumberOfUncoveredItems = 0;
					int iItem2;
					for (iItem2 = UncoveredItems.FirstItem ();
					iItem2 >= 0;
					iItem2 = UncoveredItems.NextItem ()) {
						if (Problem.Subsets [iSubset2]->IsMember (iItem2))
							NumberOfUncoveredItems++;
					}
					
					// Evaluate insertion of iSubset2
					double SubsetEvaluation = (NewScalarizingFunctionValue - UnfeasibleScalarizingFunctionValue) / 
						NumberOfUncoveredItems;
					
					// If first iteration
					if (bFirst) {
						bFirst = false;
						BestEvaluation = SubsetEvaluation;
						iBestSubset = iSubset2;
					}
					else {
						// If the evaluation of insertion of iSubset2 is better than the best evaluation
						if (BestEvaluation > SubsetEvaluation) {
							BestEvaluation = SubsetEvaluation;
							iBestSubset = iSubset2;
						}
					}
				}
			}
		}
		// Select (insert) the best subset
		SubsetsToInsert->Insert (iBestSubset);
		
		// Cover al items covered by the best subset
		for (iItem = Problem.Subsets [iBestSubset]->FirstItem ();
		iItem >= 0;
		iItem = Problem.Subsets [iBestSubset]->NextItem ()) {
			UncoveredItems.Remove (iItem);
		}
		
		// Update objective values
		for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
			ObjectiveValues [iobj] += Problem.Subsets [iBestSubset]->Costs [iobj];
		}
	}
}

void TMOSCPSolution::AcceptLocalMove () {
	// Remove the subset found by FindLocalMove ()
	SelectedSubsets->Remove (iSubsetToRemove);
	
	int iItem;
	for (iItem = Problem.Subsets [iSubsetToRemove]->FirstItem ();
	iItem >= 0;
	iItem = Problem.Subsets [iSubsetToRemove]->NextItem ()) {
		CoveringSubsets [iItem]->Remove (iSubsetToRemove);
	}

	// Add SubsetsToInsert
	int iSubset2;
	for (iSubset2 = SubsetsToInsert->FirstItem ();
	iSubset2 >= 0;
	iSubset2 = SubsetsToInsert->NextItem ()) {
		
		// Cover all memebers of iSubset2
		int iItem2;
		for (iItem2 = Problem.Subsets [iSubset2]->FirstItem ();
		iItem2 >= 0;
		iItem2 = Problem.Subsets [iSubset2]->NextItem ()) {
			CoveringSubsets [iItem2]->Insert (iSubset2);
		}
								
		SelectedSubsets->Insert (iSubset2);
	}

}

void TMOSCPSolution::RejectLocalMove () {
	RestoreObjectiveValues ();
}

void TMOSCPSolution::LocalSearch (TPoint& ReferencePoint) {
	bool bImprovementFound;
	do {
		bImprovementFound = false;
		double CurrentScalarizingFunctionValue = ScalarizingFunction (ReferencePoint);

		int iBestSubsetForRemoval;

		double BestMoveEvaluation;

		TIntSet BestSubsetsToInsert (Problem.Subsets.size ());

		// For each selected subset
		bool bFirstMove = true;
		int iSubset;
		for (iSubset = SelectedSubsets->FirstItem ();
		iSubset >= 0;
		iSubset = SelectedSubsets->NextItem ()) {
			// Consider removal of iSubset
			TSolution Solution (*this);

			int iobj;
			for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
				Solution.ObjectiveValues [iobj] -= Problem.Subsets [iSubset]->Costs [iobj];
			}

			// Find items that would be uncovered after removal of iSubset
			TIntSet UncoveredItems (Problem.NumberOfItems);
			int iItem;
			for (iItem = Problem.Subsets [iSubset]->FirstItem ();
			iItem >= 0;
			iItem = Problem.Subsets [iSubset]->NextItem ()) {
				if (CoveringSubsets [iItem]->Size () == 1) {
					UncoveredItems.Insert (iItem);
				}
			}

			TIntSet SubsetsToInsert (Problem.Subsets.size ());

			double NewEvaluation = Solution.ScalarizingFunction (ReferencePoint);

			// In greedy manner find subsets repairing the solution
			bool bImprovementPossible = true;
			// While not all items are covered and it is possible to improve scalarizing function value
			while (bImprovementPossible && (UncoveredItems.Size () > 0)) {
				double UnfeasibleScalarizingFunctionValue = Solution.ScalarizingFunction (ReferencePoint);
				int iBestSubset;
				double BestEvaluation;
				vector <bool> SubsetTested;
				SubsetTested.resize (Problem.Subsets.size ());

				iItem = UncoveredItems.FirstItem ();

				// For each potentially uncovered item (if iSubset would be removed)
				bool bFirst = true;
				for (iItem = UncoveredItems.FirstItem ();
				iItem >= 0;
				iItem = UncoveredItems.NextItem ()) {
					// For each subset covering iItem
					int iSubset2;
					for (iSubset2 = Problem.CoveringSubsets[iItem]->FirstItem ();
					iSubset2 >= 0;
					iSubset2 = Problem.CoveringSubsets [iItem]->NextItem ()) {
						if (!SubsetTested [iSubset2]) {
							// Consider insertion of iSubset2
							TSolution NewSolution (Solution);

							SubsetTested [iSubset2] = true;
							for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
								NewSolution.ObjectiveValues [iobj] += Problem.Subsets [iSubset2]->Costs [iobj];
							}

							double NewScalarizingFunctionValue = NewSolution.ScalarizingFunction (ReferencePoint);
							
							// Find the number of potentially uncovered items covered by iSubset2
							int NumberOfUncoveredItems = 0;
							int iItem2;
							for (iItem2 = UncoveredItems.FirstItem ();
							iItem2 >= 0;
							iItem2 = UncoveredItems.NextItem ()) {
								if (Problem.Subsets [iSubset2]->IsMember (iItem2))
									NumberOfUncoveredItems++;
							}

							// Evaluate insertion of iSubset2
							double SubsetEvaluation = (NewScalarizingFunctionValue - UnfeasibleScalarizingFunctionValue) / 
								NumberOfUncoveredItems;

							// If first iteration of greedy heuristic
							if (bFirst) {
								bFirst = false;
								BestEvaluation = SubsetEvaluation;
								iBestSubset = iSubset2;
							}
							else {
								// If the evaluation of insertion of iSubset2 is better than the best evaluation
								if (BestEvaluation > SubsetEvaluation) {
									BestEvaluation = SubsetEvaluation;
									iBestSubset = iSubset2;
								}
							}
						}
					}
				}
				SubsetsToInsert.Insert (iBestSubset);

				// Cover all potentially uncovered items covered by iBestSubset
				for (iItem = Problem.Subsets [iBestSubset]->FirstItem ();
				iItem >= 0;
				iItem = Problem.Subsets [iBestSubset]->NextItem ()) {
					UncoveredItems.Remove (iItem);
				}

				// Update objective value
				for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
					Solution.ObjectiveValues [iobj] += Problem.Subsets [iBestSubset]->Costs [iobj];
				}

				NewEvaluation = Solution.ScalarizingFunction (ReferencePoint);

				// If the new scalarizing function value is already worse than the current value
				// then no improvement is possible
				if (NewEvaluation > CurrentScalarizingFunctionValue)
					bImprovementPossible = false;
			}

			// If the first iteration of local search
			if (bFirstMove) {
				bFirstMove = false;
				BestMoveEvaluation = NewEvaluation;
				iBestSubsetForRemoval = iSubset;
				BestSubsetsToInsert = SubsetsToInsert;
			}
			// If the move is better than the best move
			else if (BestMoveEvaluation > NewEvaluation) {
				BestMoveEvaluation = NewEvaluation;
				iBestSubsetForRemoval = iSubset;
				BestSubsetsToInsert = SubsetsToInsert;
			}
		}

		// If improvement was found
		if (BestMoveEvaluation < CurrentScalarizingFunctionValue) {
			bImprovementFound = true;

			// Remove iBestSubsetForRemoval
			SelectedSubsets->Remove (iBestSubsetForRemoval);
			
			int iItem;
			for (iItem = Problem.Subsets [iBestSubsetForRemoval]->FirstItem ();
			iItem >= 0;
			iItem = Problem.Subsets [iBestSubsetForRemoval]->NextItem ()) {
				CoveringSubsets [iItem]->Remove (iBestSubsetForRemoval);
				
			}

			// Update objective values
			int iobj;
			for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
				ObjectiveValues [iobj] -= Problem.Subsets [iBestSubsetForRemoval]->Costs [iobj];
			}

			// Add BestSubsetsToInsert
			int iSubset2;
			for (iSubset2 = BestSubsetsToInsert.FirstItem ();
			iSubset2 >= 0;
			iSubset2 = BestSubsetsToInsert.NextItem ()) {

				// Cover all memebers of iSubset2
				int iItem2;
				for (iItem2 = Problem.Subsets [iSubset2]->FirstItem ();
				iItem2 >= 0;
				iItem2 = Problem.Subsets [iSubset2]->NextItem ()) {
					CoveringSubsets [iItem2]->Insert (iSubset2);
				}
								
				SelectedSubsets->Insert (iSubset2);
				
				// Update objective values
				for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
					ObjectiveValues [iobj] += Problem.Subsets [iSubset2]->Costs [iobj];
				}
			}

		}
	// While an improving move was found
	} while (bImprovementFound);
}

void TMOSCPSolution::RemoveRedundantSubsets (TPoint& ReferencePoint) {
	
	// Find reduntantly covered (covered by more than one subset) items
	// amd redundand subsets
	TIntSet RedundantlyCoveredItems (Problem.NumberOfItems);
	RedundantlyCoveredItems.MakeFull ();

	TIntSet RedundantSubsets (Problem.Subsets.size ());
	RedundantSubsets = *SelectedSubsets;

	// For each redundantly covered item
	int iItem;
	for (iItem = RedundantlyCoveredItems.FirstItem (); 
		iItem >= 0; 
		iItem = RedundantlyCoveredItems.NextItem ()) {
		if (CoveringSubsets [iItem]->Size () == 1) {
			RedundantlyCoveredItems.Remove (iItem);
			int iSubset = CoveringSubsets [iItem]->FirstItem ();
			RedundantSubsets.Remove (iSubset);
		}
	}

	// While there are redundant subsets
	while (RedundantSubsets.Size () > 0) {
		// Find the best subset for removal
		int iBestSubset;
		double BestEvaluation;

		double CurrentScalarizingFunctionValue = ScalarizingFunction (ReferencePoint);

		int iSubset;
		bool bFirst = true;
		// For each redundant subset
		for (iSubset = RedundantSubsets.FirstItem ();
			iSubset >= 0;
			iSubset = RedundantSubsets.NextItem ()) {
			// Consider removal of iSubset
			TSolution Solution (*this);

			int iobj;
			for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
				Solution.ObjectiveValues [iobj] -= Problem.Subsets [iSubset]->Costs [iobj];
			}
			double NewScalarizingFunctionValue = Solution.ScalarizingFunction (ReferencePoint);

			// Subset evaluation = gain / subset size
			double SubsetEvaluation = (CurrentScalarizingFunctionValue -
				NewScalarizingFunctionValue) / Problem.Subsets [iSubset]->Size ();

			// If first iteration
			if (bFirst) {
				iBestSubset = iSubset;
				BestEvaluation = SubsetEvaluation;
			}
			// Else if the new evaluation is better than the best evaluation
			else if (BestEvaluation < SubsetEvaluation) {
				iBestSubset = iSubset;
				BestEvaluation = SubsetEvaluation;
			}

			bFirst = false;
		}

		// Remove the best subset
		SelectedSubsets->Remove (iBestSubset);

		for (iItem = Problem.Subsets [iBestSubset]->FirstItem ();
			iItem >= 0;
			iItem = Problem.Subsets [iBestSubset]->NextItem ()) {
			CoveringSubsets [iItem]->Remove (iBestSubset);

			if (CoveringSubsets [iItem]->Size () == 1) {
				RedundantlyCoveredItems.Remove (iItem);
				int iSubset = CoveringSubsets [iItem]->FirstItem ();
				RedundantSubsets.Remove (iSubset);
			}
		}

		// Update objective values
		int iobj;
		for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
			ObjectiveValues [iobj] -= Problem.Subsets [iBestSubset]->Costs [iobj];
		}

		// Update reduntantly covered (covered by more than one subset) items
		// amd redundand subsets
		RedundantSubsets.Remove (iBestSubset);

	}

}

void TMOSCPSolution::RemoveRedundantSubsets () {
	
	// Find reduntantly coverd (covered by more than one subset) items
	// amd redundand subsets
	TIntSet RedundantlyCoveredItems (Problem.NumberOfItems);
	RedundantlyCoveredItems.MakeFull ();

	TIntSet RedundantSubsets (Problem.Subsets.size ());
	RedundantSubsets = *SelectedSubsets;

	int iItem;
	for (iItem = RedundantlyCoveredItems.FirstItem (); 
		iItem >= 0; 
		iItem = RedundantlyCoveredItems.NextItem ()) {
		if (CoveringSubsets [iItem]->Size () == 1) {
			RedundantlyCoveredItems.Remove (iItem);
			int iSubset = CoveringSubsets [iItem]->FirstItem ();
			RedundantSubsets.Remove (iSubset);
		}
	}

	// While there are redundant subsets
	while (RedundantSubsets.Size () > 0) {
		// Draw a subset for removal
		int iBestSubset = RedundantSubsets.GetRandomItem ();

		// Remove the best subset
		SelectedSubsets->Remove (iBestSubset);

		for (iItem = Problem.Subsets [iBestSubset]->FirstItem ();
			iItem >= 0;
			iItem = Problem.Subsets [iBestSubset]->NextItem ()) {
			CoveringSubsets [iItem]->Remove (iBestSubset);

		}

		// Update objective values
		int iobj;
		for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
			ObjectiveValues [iobj] -= Problem.Subsets [iBestSubset]->Costs [iobj];
		}

		// Update reduntantly covered (covered by more than one subset) items
		// amd redundand subsets
		RedundantSubsets.Remove (iBestSubset);
		for (iItem = RedundantlyCoveredItems.FirstItem (); 
		iItem >= 0; 
		iItem = RedundantlyCoveredItems.NextItem ()) {
			if (CoveringSubsets [iItem]->Size () == 1) {
				RedundantlyCoveredItems.Remove (iItem);
				int iSubset;
				for (iSubset = CoveringSubsets [iItem]->FirstItem ();
				iSubset >= 0;
				iSubset = CoveringSubsets [iItem]->NextItem ())
					RedundantSubsets.Remove (iSubset);
			}
		}
	}
}

TMOSCPSolution::TMOSCPSolution () {
	// Allocate memory
	SubsetsToInsert = new TIntSet (Problem.Subsets.size ());

	SelectedSubsets = new TIntSet (Problem.Subsets.size ());
	CoveringSubsets.resize (Problem.NumberOfItems);
	int iItem;
	for (iItem = 0; iItem < Problem.NumberOfItems; iItem++) 
		CoveringSubsets [iItem] = new TIntSet (Problem.Subsets.size ());

	ConstructRandomSolution ();

	RemoveRedundantSubsets ();
}

TMOSCPSolution::TMOSCPSolution (TScalarizingFunctionType ScalarizingFunctionType,
								TPoint& ReferencePoint, TWeightVector WeightVector, TNondominatedSet &NondominatedSet) : TMOMHSolution 
								(ScalarizingFunctionType, ReferencePoint, WeightVector, NondominatedSet) {
	// Allocate memory
	SelectedSubsets = new TIntSet (Problem.Subsets.size ());
	CoveringSubsets.resize (Problem.NumberOfItems);
	int iItem;
	for (iItem = 0; iItem < Problem.NumberOfItems; iItem++) 
		CoveringSubsets [iItem] = new TIntSet (Problem.Subsets.size ());
	
	ConstructRandomSolution ();
	RemoveRedundantSubsets (ReferencePoint);
	LocalSearch (ReferencePoint);
}

TMOSCPSolution::TMOSCPSolution (TMOMHSolution& Parent1, TMOMHSolution& Parent2) {
	// Allocate memory
	SelectedSubsets = new TIntSet (Problem.Subsets.size ());
	CoveringSubsets.resize (Problem.NumberOfItems);
	int iItem;
	for (iItem = 0; iItem < Problem.NumberOfItems; iItem++) 
		CoveringSubsets [iItem] = new TIntSet (Problem.Subsets.size ());
	
	Recombine (Parent1, Parent2);
	RemoveRedundantSubsets ();
}

TMOSCPSolution::TMOSCPSolution (TMOMHSolution& Parent1, TMOMHSolution& Parent2,
				TScalarizingFunctionType ScalarizingFunctionType,
				TPoint& ReferencePoint, TWeightVector WeightVector, TNondominatedSet &NondominatedSet) : TMOMHSolution 
				(Parent1, Parent2, ScalarizingFunctionType, ReferencePoint, WeightVector, NondominatedSet) {
	// Allocate memory
	SelectedSubsets = new TIntSet (Problem.Subsets.size ());
	CoveringSubsets.resize (Problem.NumberOfItems);
	int iItem;
	for (iItem = 0; iItem < Problem.NumberOfItems; iItem++) 
		CoveringSubsets [iItem] = new TIntSet (Problem.Subsets.size ());
	
	Recombine (Parent1, Parent2);
	LocalSearch (ReferencePoint);
}

ostream& TMOSCPSolution::Save(ostream& Stream) {
	TPoint::Save (Stream);

	// Uncomment the code below in order to save full descriptions of the solutions
/*	int iSubset;
	for (iSubset = SelectedSubsets->FirstItem ();
		iSubset >= 0;
		iSubset = SelectedSubsets->NextItem ()) {
		Stream << iSubset + 1 << '\t';
	}*/
	return Stream;
}

TMOSCPSolution::TMOSCPSolution (TMOSCPSolution& SourceSolution) : TMOMHSolution (SourceSolution) {
	SelectedSubsets = new TIntSet (Problem.Subsets.size ());

	CoveringSubsets.resize (Problem.NumberOfItems);
	int iItem;
	for (iItem = 0; iItem < Problem.NumberOfItems; iItem++) 
		CoveringSubsets [iItem] = new TIntSet (Problem.Subsets.size ());

	*SelectedSubsets = *SourceSolution.SelectedSubsets;

	for (iItem = 0; iItem < Problem.NumberOfItems; iItem++) 
		*CoveringSubsets [iItem] = *SourceSolution.CoveringSubsets [iItem];
}

TMOSCPSolution& TMOSCPSolution::operator = (TMOSCPSolution& SourceSolution) {
	(TMOMHSolution&)(*this) = (TMOMHSolution&)SourceSolution;

	SelectedSubsets = new TIntSet (Problem.Subsets.size ());
	
	CoveringSubsets.resize (Problem.NumberOfItems);
	int iItem;
	for (iItem = 0; iItem < Problem.NumberOfItems; iItem++) 
		CoveringSubsets [iItem] = new TIntSet (Problem.Subsets.size ());

	*SelectedSubsets = *SourceSolution.SelectedSubsets;

	for (iItem = 0; iItem < Problem.NumberOfItems; iItem++) 
		*CoveringSubsets [iItem] = *SourceSolution.CoveringSubsets [iItem];

	return *this;
}
