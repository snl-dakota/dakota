// TNewProblem.h: interface for the TNewProblem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TNEWPROBLEM_H__D5674F73_1B7C_11D5_8398_000000000000__INCLUDED_)
#define AFX_TNEWPROBLEM_H__D5674F73_1B7C_11D5_8398_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "PROBLEM.H"

// @@@@ Step 2 (Optional)
// To do:	Change the name of this class from TNewProblem to name you want to use
//			Do it in all places!
class TNewProblem : public TProblem {

	friend class TNewSolution; // The solution class is friend class of the problem class

protected:
	// To do:	Define fields storing all the data of your problem

// @@@@ Step 3 (Optional)
public:
	virtual bool Load(char* FileName);

// @@@@ Step 5 (version 1)
	TNewProblem ();
};

// @@@@ Step 4 (Optional)
extern TNewProblem Problem;

#endif // !defined(AFX_TNEWPROBLEM_H__D5674F73_1B7C_11D5_8398_000000000000__INCLUDED_)
