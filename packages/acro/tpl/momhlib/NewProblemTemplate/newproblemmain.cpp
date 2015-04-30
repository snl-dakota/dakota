#include "MOGLS.h"
#include "PSA.h"
#include "NewSolution.h"


// @@@@ Step 10.2
class TMOGLSForNewProblem : public TMOGLS <TNewSolution> {
	// To do:	Override the virtual methods defined in TMOGLS to
	//			the desired behavior
};

// @@@@ Step 10.3
template <class TProblemSolution> class TNewVersionOfMOGLS : public TMOGLS <TProblemSolution> {
	// To do:	Override the virtual methods defined in TMOGLS to
	//			the desired behavior
};

main () {
// @@@@ Step 10.1
	TMOGLS <TNewSolution> MOGLSForNewProblem;
	TPSA <TNewSolution> PSAForNewProblem;

// @@@@ Step 10.2
	TMOGLSForNewProblem MOGLSForNewProblem2;

// @@@@ Step 10.3
	TNewVersionOfMOGLS <TNewSolution> MOGLSForNewProblem3;

// @@@@ Step 11
	// The parameters are exemplary
	MOGLSForNewProblem.SetParameters (10, 0, 50, _Chebycheff); 
	MOGLSForNewProblem.Run ();
	MOGLSForNewProblem.pNondominatedSet->Save ("MyResults.txt");

	return 0;
}