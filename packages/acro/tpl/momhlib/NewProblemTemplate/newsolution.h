// TNewSolution.h: interface for the TNewSolution class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TNEWSOLUTION_H__D5674F74_1B7C_11D5_8398_000000000000__INCLUDED_)
#define AFX_TNEWSOLUTION_H__D5674F74_1B7C_11D5_8398_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "MOMHSolution.h"

// @@@@ Step 6 
// To do:	Change the name of this class from TNewProblem to name you want to use
//			Do it in all places!
class TNewSolution : public TMOMHSolution  
{
protected:
	// To do:	Define fields storing all the data of your solution

// @@@@ Step 7 (Optional)
public:
	TNewSolution (TNewSolution& SourceSolution);

// @@@@ Step 8 (Optional)
public:
	virtual ostream& Save(ostream& Stream);

// @@@@ Step 9.1.1
public:
	void LocalSearch (TPoint& ReferencePoint);
	
// @@@@ Step 9.1.2 
public:
	TNewSolution (TScalarizingFunctionType ScalarizingFunctionType, TPoint& ReferencePoint, TWeightVector WeightVector, TNondominatedSet &NondominatedSet);

// @@@@ Step 9.1.3
public:
	TNewSolution::TNewSolution (TNewSolution& Parent1, TNewSolution& Parent2, TScalarizingFunctionType ScalarizingFunctionType, TPoint& ReferencePoint, TWeightVector WeightVector, TNondominatedSet &NondominatedSet);

// @@@@ Step 9.2.1 and 9.3.1
	TNewSolution ();

// @@@@ Step 9.2.2
	void FindLocalMove ();

// @@@@ Step 9.2.3 (Optional)
	void AcceptLocalMove ();

// @@@@ Step 9.2.4 
	void RejectLocalMove ();

// @@@@ Step 9.3.2
	TNewSolution::TNewSolution (TNewSolution& Parent1, TNewSolution& Parent2);

// @@@@ Step 9.3.3 (Optional)
	void Mutate ();
};

#endif // !defined(AFX_TNEWSOLUTION_H__D5674F74_1B7C_11D5_8398_000000000000__INCLUDED_)
