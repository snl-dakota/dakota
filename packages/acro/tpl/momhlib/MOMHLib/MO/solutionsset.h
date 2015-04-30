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

#if !defined(AFX_TSOLUTIONSSET_H__25978BD1_92C8_11D3_8149_000000000000__INCLUDED_)
#define AFX_TSOLUTIONSSET_H__25978BD1_92C8_11D3_8149_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "global.h"
#include "solution.h"

/**	Set of solutions 
*
*	It is just a set of pointers */
class TSolutionsSet : public vector<TSolution*>
{
protected:
	/** Update nadir and ideal point values
     * 
     * Method updates nadir and ideal point approximation.
     * New point is taken into consideration.
     *
     * @param Solution new solution added to nondominated set
     **/
	void UpdateIdealNadir (TSolution& Solution);
public:
	/** Tells how many nondominated sulutions is on list */
	int				iSetSize;

	/** Tells us how big is list */
	int				iListSize;

	/** Aproximate ideal point 
	*
	*	The field is set by UpdateIdealNadir () method such that
	*	each component is equal to the best value of the objective 
	*	in the set of solutions */
	TPoint ApproximateIdealPoint;

	/** Aproximate nadir point 
	*
	*	The field is set by UpdateIdealNadir () method such that
	*	each component is equal to the worst value of the objective 
	*	in the set of solutions */
	TPoint ApproximateNadirPoint;

	/**	Calculates coverage of two sets of solutions */
	void CalculateCoverage (TSolutionsSet& Set2, double &CoverageByThis, double &CoverageBySet2);

	/**	Compares the two sets with the outperformance relations */
	void OutperformanceCompare (TSolutionsSet& Set2, bool &bWeak1By2, bool &bStrong1By2, bool &bComplete1By2, bool &bWeak2By1, bool &bStrong2By1, bool &bComplete2By1);

	/** Calculates average value of scalarizing function measure */
	double AverageScalarizingFunction (TPoint& ReferencePoint,
		TScalarizingFunctionType ScalarizingFunctionType, TWeightsSet WeightsSet);

	/** Saves the set of solutions */
	virtual void Save(char* FileName);

	/** Loads the set of solutions */
	void Load(char* FileName);

	/** Updates ApproximateIdealPoint and ApproximateNadirPoint */
	void UpdateIdealNadir ();

	/* Deletes all solutions */
	virtual void DeleteAll ();

};

#endif // !defined(AFX_TSOLUTIONSSET_H__25978BD1_92C8_11D3_8149_000000000000__INCLUDED_)
