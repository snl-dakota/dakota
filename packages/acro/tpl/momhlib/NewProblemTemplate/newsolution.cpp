// TNewSolution.cpp: implementation of the TNewSolution class.
//
//////////////////////////////////////////////////////////////////////

#include "NewSolution.h"

// @@@@ Step 7 (Optional)
TNewSolution::TNewSolution (TNewSolution& SourceSolution) : TMOMHSolution (SourceSolution) {
	// To do:	Put the code that constructs a copy SourceSolution
}

// @@@@ Step 8 (Optional)
ostream& TNewSolution::Save(ostream& Stream) {
	// To do:	Put the code that saves the data of your solution

	return Stream;
}

// @@@@ Step 9.1.1
void TNewSolution::LocalSearch (TPoint& ReferencePoint) {
	// To do:	Put the code that improves locally the ScalarizingFunction (ReferencePoint)

	// Example:
	// Greedy local search
	// double CurrentScalarizingFunctionValue = ScalarizingFunction (ReferencePoint);
	//	do {
		// for all neighborhood moves in random order {
			// SaveObjectiveValues ();
			// Evaluate the local move and set appropriately ObjectiveValues[]
			// double NewScalarizingFunctionValue = ScalarizingFunction (ReferencePoint);
			// if (NewScalarizingFunctionValue < CurrentScalarizingFunctionValue)
				// Perform the move and quit the loop on  neighborhood moves
			// else
				// RestoreObjectiveValues ();
		// }
	// } while (improving move has been found)
}

// @@@@ Step 9.1.2 
TNewSolution::TNewSolution (TScalarizingFunctionType ScalarizingFunctionType, TPoint& ReferencePoint, TWeightVector WeightVector, TNondominatedSet &NondominatedSet) :
		TMOMHSolution (ScalarizingFunctionType, ReferencePoint, WeightVector, NondominatedSet)
{
	// To do:	Put the code that constructs new initial solutions being good on the scalarizing function defined by ScalarizingFunctionType, ReferencePoint and WeightVector.

	// Construction of the new solution
	
	// Recommended
	//LocalSearch (ReferencePoint);

}

// @@@@ Step 9.1.3 
	TNewSolution::TNewSolution (TNewSolution& Parent1, TNewSolution& Parent2, TScalarizingFunctionType ScalarizingFunctionType, TPoint& ReferencePoint, TWeightVector WeightVector, TNondominatedSet &NondominatedSet) : 
		TMOMHSolution (Parent1, Parent2, ScalarizingFunctionType, ReferencePoint, WeightVector, NondominatedSet)
{
	// To do:	Put the code that constructs a new solutions by recombination of the parents being good on the scalarizing function defined by ScalarizingFunctionType, ReferencePoint and WeightVector.	
			
	// Recombination code
	
	// Recommended
	// LocalSearch (ReferencePoint);
}

// @@@@ Step 9.2.1 and 9.3.1
TNewSolution::TNewSolution () 
{
	// To do:	Put the code that constructs new initial solutions 

}

// @@@@ Step 9.2.2
void TNewSolution::FindLocalMove () 
{
	SaveObjectiveValues ();	
	// To do:	Put the code that finds at random a local move

}

// @@@@ Step 9.2.3 (Optional)
void TNewSolution::AcceptLocalMove () 
{
	// To do:	Put the code that accepts the move found by FindLocalMove () method
}

// @@@@ Step 9.2.4
void TNewSolution::RejectLocalMove () 
{
	// To do:	Put the code that rejects the move found by FindLocalMove () method

	RestoreObjectiveValues ();	
}

// @@@@ Step 9.3.2 
	TNewSolution::TNewSolution (TNewSolution& Parent1, TNewSolution& Parent2) : 
		TMOMHSolution (Parent1, Parent2)
{
	// To do:	Put the code that constructs a new solutions by recombination of the parents 	
			
	// Recombination code
	
}

// @@@@ Step 9.3.3 (Optional)
void TNewSolution::Mutate () 
{
	// To do:	Put the code implements the mutation operator

}

