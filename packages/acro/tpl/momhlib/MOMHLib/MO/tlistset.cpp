/*
Multiple Objective MetaHeuristics Library in C++ MOMHLib++
Copyright (C) 2001 Andrzej Jaszkiewicz, Radoslaw Ziembinski (radekz@free.alpha.net.pl)

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

#ifndef __TLISTSET_CPP_
#define __TLISTSET_CPP_

#include "tlistset.h"

template <class TProblemSolution>
TListSet <TProblemSolution>::TListSet() : TNondominatedSet () 
{

};

template <class TProblemSolution>
bool TListSet <TProblemSolution>::Update(TSolution& Solution)
{
	bool bEqual, bDominated, bDominating;
	
	bool bAdded = false;

	if (size () == 0) {
		bAdded = true;
		TProblemSolution* pPattern = (TProblemSolution *)&Solution;
		TProblemSolution* pNewSolution = new TProblemSolution(*pPattern);
		this->push_back(pNewSolution);
		UpdateIdealNadir ();
	}
	else {
		bEqual = bDominated = bDominating = false;
		
		unsigned int i; for (i = 0; (i < size ()) && !bEqual && !bDominated ; i++) {
			
			TCompare ComparisonResult = Solution.Compare (*(*this)[i]);
			
			switch (ComparisonResult) {
			case _Dominating:
				UpdateNadir (i);
				delete (*this)[i];
				erase (begin() + i);
				i--;
				bDominating = true;
				break;
			case _Dominated:
				bDominated = true;
				break;
			case _Nondominated:
				break;
			case _Equal:
				bEqual = true;
				break;
			}
		}
		
		if (bDominated && bDominating) {
			// Exception
			cout << Solution.ObjectiveValues [0] << "  " << Solution.ObjectiveValues [1] << "  " ;
			cout << "Exception\n";
			cout << "void TListSet::Update (TPoint& Point)\n";
			cout << "bDominated && bDominating\n";
			exit (0);
		}
		
		if (!bDominated && !bEqual) {
			TProblemSolution* pPattern = (TProblemSolution*)&Solution;
			TProblemSolution* pNewSolution = new TProblemSolution(*pPattern);
			UpdateIdealNadir (*pNewSolution);
			this->push_back (pNewSolution);
			bAdded = true;
		}
	}

	iSetSize = size();
	iListSize = size();

	return bAdded;
}

template <class TProblemSolution>
bool TListSet <TProblemSolution>::Update(TNondominatedSet& NondominatedSet) 
{
bool bAdded = false;

	unsigned int i; for (i = 0; i < NondominatedSet.size(); i++) {
		if (NondominatedSet[i] != NULL) {
			bool bTemp = (Update(*(NondominatedSet[i])));
			bAdded = bAdded || bTemp;
		}
	}

    return bAdded; 
}


template <class TProblemSolution>
void TListSet <TProblemSolution>::UpdateNadir(int iRemovedSolution) {
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		if (Objectives [iobj].bActive) {
			if ((*this)[iRemovedSolution]->ObjectiveValues [iobj] == ApproximateNadirPoint.ObjectiveValues [iobj]) {
				bool bFirst = true;
				unsigned int i; for (i = 0; i < size (); i++) {
					if (i != iRemovedSolution) {
						if (bFirst) {
							ApproximateNadirPoint.ObjectiveValues [iobj] = (*this)[i]->ObjectiveValues [iobj];
							bFirst = false;
						}
						else {
							if (Objectives [iobj].ObjectiveType == _Max) {
								if (ApproximateNadirPoint.ObjectiveValues [iobj] > (*this)[i]->ObjectiveValues [iobj])
									ApproximateNadirPoint.ObjectiveValues [iobj] = (*this)[i]->ObjectiveValues [iobj];
							}
							if (Objectives [iobj].ObjectiveType == _Min) {
								if (ApproximateNadirPoint.ObjectiveValues [iobj] < (*this)[i]->ObjectiveValues [iobj])
									ApproximateNadirPoint.ObjectiveValues [iobj] = (*this)[i]->ObjectiveValues [iobj];
							}
						}
					}
				}
			}
		}
	}
}

template <class TProblemSolution>
void TListSet <TProblemSolution>::DeleteAll()
{
	TSolutionsSet::DeleteAll();

	iSetSize = 0;
	iListSize = 0;
};

 
template <class TProblemSolution>
void TListSet <TProblemSolution>::GetRandomSolution(TSolution* &pSolution)
{
int						iIndex = 0;

	// old code
	if (iSetSize <= 0) {
		pSolution = NULL;
	} else {
		iIndex = rand() % iSetSize;
		pSolution = (TSolution*)(*this)[iIndex];
	}
};

template <class TProblemSolution>
bool TListSet <TProblemSolution>::ReduceSetByClustering(int iPopulationSize, bool bScalarize)
{
vector<int>		            iIndices;
TListSet <TProblemSolution>		oTmpSet;
unsigned int		            i = 0;

	if (ReduceIndividuals(iPopulationSize, iIndices, OBJECTIVE_SPACE, bScalarize) == false)
	{
		return true;
	}
	oTmpSet.resize(0);

	for(i = 0; i < iIndices.size(); i++) 
	{
		oTmpSet.push_back((*this)[iIndices[i]]);
		(*this)[iIndices[i]] = NULL;
	}

	DeleteAll();

	Update(oTmpSet);

	UpdateIdealNadir();

	return true;
}


#endif
