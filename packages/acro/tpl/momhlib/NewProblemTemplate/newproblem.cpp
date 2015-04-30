// TNewProblem.cpp: implementation of the TNewProblem class.
//
//////////////////////////////////////////////////////////////////////

#include "NewProblem.h"

// @@@@ Step 4 (Optional)
TNewProblem Problem;

// @@@@ Step 3 (Optional)
bool TNewProblem::Load(char* FileName) {
	// To do:	Put the code that read the data of your problem from a file named FileName
	//			The method should return true if the problem data were successfully read

	// @@@@ Step 5 (version 2)
	// To do:	Add code setting NumberOfObjectives and Objectives global variables
	// Example:
	// NumberOfObjectives = NumberOfObjectivesReadFromFile;
	// Objectives.resize(NumberOfObjectives);
	// Objectives [0].ObjectiveType = TypeOfTheObjectiveReadFromFile;
	// Objectives [0].bActive = ActivenessTheObjectiveReadFromFile;
	// ...

	return true;
}


// @@@@ Step 5 (version 1)
TNewProblem::TNewProblem () {
	// To do:	Add code setting NumberOfObjectives and Objectives global variables
	// Example:
	// NumberOfObjectives = 3
	// Objectives.resize(NumberOfObjectives);
	// Objectives [0].ObjectiveType = _Min;
	// Objectives [0].bActive = true;
	// Objectives [1].ObjectiveType = _Max;
	// Objectives [1].bActive = true;
	// Objectives [2].ObjectiveType = _Min;
	// Objectives [2].bActive = true;

}
