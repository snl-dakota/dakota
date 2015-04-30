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

#ifndef __TQUADTREE_CPP_
#define __TQUADTREE_CPP_

#include "weights.h"
#include "tquadtree.h"
#include "nondominatedset.h"
#include "tlistset.h"
#include "trandomgenerator.h"

template <class TProblemSolution>
TQuadTree <TProblemSolution>::TQuadTree() : TNondominatedSet()
{
	iSetSize = 0;
	m_pQuadRoot = NULL;
    m_bQueueSort = true;
	resize(0);

	m_dUpdateTime = 0;
	m_dReinsertTime = 0;
}

template <class TProblemSolution>
bool TQuadTree <TProblemSolution>::RemoveChild(TQuadNode *pParentNode, TQuadNode *pChildNode)
{
vector <bool>			bKSuccessor;
TQuadNode				*pQuadNode = NULL;
TProblemSolution		*pParentSolution = NULL;
TProblemSolution		*pChildSolution = NULL;

	assert((pParentNode -> m_iListPosition >= 0) && (pChildNode -> m_iListPosition >= 0));

	pParentSolution = (TProblemSolution *)(*this)[pParentNode -> m_iListPosition];
	pChildSolution = (TProblemSolution *)(*this)[pChildNode -> m_iListPosition];

	assert((pParentSolution != NULL) && (pChildSolution != NULL));

	// find vector
	pParentSolution -> CompareTo(*pChildSolution, bKSuccessor);

	// remove child
	if (pParentNode -> m_oOffsprings.RemoveBNode(bKSuccessor, &pChildNode) == false) {
		return false;
	}

	if (pChildNode != NULL) {
 		delete pChildNode;
		pChildNode = NULL;
	}

	return true;
}

template <class TProblemSolution>
bool TQuadTree <TProblemSolution>::AddNode(TQuadNodePtr pParentNode, 
										   TSolution *pSolution, 
										   int iListPosition, 
										   vector <bool> &bKSuccessor)
{
TQuadNode		*pQuadNode = NULL;
TQuadNode		*pPreviousNode = NULL;
TSolution		*pNewSolution = NULL;

	pQuadNode = new TQuadNode;

	if ((iListPosition < 0) && (pSolution != NULL)) {
		// new solution

		pNewSolution = new TProblemSolution(*((TProblemSolution *)pSolution));

		if ((pQuadNode == NULL) || (pNewSolution == NULL)) {
			return false;
		}

		// update ideal and nadir points
		// call before iSetSize has been updated		
		UpdateIdealAndNadir(*(TProblemSolution *)pNewSolution);

		// add new solution
		push_back(pNewSolution);
		iSetSize += 1;
		pQuadNode -> m_iListPosition = size() - 1;

		pNewSolution = NULL;
	} else {
		// solution that exist on list

		pQuadNode -> m_iListPosition = iListPosition;
	}

	pQuadNode -> m_pParentNode = pParentNode;

	if (pParentNode == NULL) {
		if (m_pQuadRoot != NULL) {
			// bug
			assert(false);
		}

		// insert root
		m_pQuadRoot = pQuadNode;
	} else {	
		// insert node as a k-subnode of pParentNode
		if (pParentNode -> m_oOffsprings.AddBNode(bKSuccessor, pQuadNode, &pPreviousNode) == false) {
			return false;			
		}
		
		// some node has been stored on a this place
		// a such situation shouldn't accident
		if (pPreviousNode != NULL) {
			// bug
			assert(false);
		}
	}

	pQuadNode = NULL;

	return true;
}

template <class TProblemSolution>
bool TQuadTree <TProblemSolution>::ReinsertRemoved(vector <int> &iReinsertIndices)
{
unsigned int			i = 0;

	for(i = 0; i < iReinsertIndices.size(); i++) {
		if (iReinsertIndices[i] >= 0) {
			if (AddExistSolution(iReinsertIndices[i]) == false) {
				return false;
			}
		}
	}

	return true;
}

template <class TProblemSolution>
bool TQuadTree <TProblemSolution>::VerifyRemoved(TSolution &oNewSolution, 
												 vector <int> &iReinsertIndices)
{
TProblemSolution		*pSolutionToDelete = NULL;
unsigned int			i = 0;
int						iListPos = 0;
TCompare				iResult = _Equal;
vector <int>			iNewTable;

	iNewTable.resize(0);
	for(i = 0; i < iReinsertIndices.size(); i++) {
		iListPos = iReinsertIndices[i];

		pSolutionToDelete = (TProblemSolution *)(*this)[iListPos];

		iResult = pSolutionToDelete -> Compare(oNewSolution);

		if ((iResult == _Dominated) || (iResult == _Equal)) {
			// delete element from list
			(*this)[iListPos] = NULL;
			iSetSize -= 1;

			delete pSolutionToDelete;
			pSolutionToDelete = NULL;
			
			iReinsertIndices[i] = -1;
		} else {
			iNewTable.push_back(iReinsertIndices[i]);
		}
	}

	// copy list of nondominated vectors
	iReinsertIndices.clear();
	for(i = 0; i < iNewTable.size(); i++) {
		iReinsertIndices.push_back(iNewTable[i]);
	}

	return true;
}

template <class TProblemSolution>
bool TQuadTree <TProblemSolution>::DeleteNode(TQuadNodePtr &pCurrentNode,
											  vector <int> &iReinsertIndices)
{
TQuadNode				*pParentNode = NULL;
TProblemSolution		*pSolutionToDelete = NULL;
TCompare				iResult = _Equal;
int						i = 0;
int						iListPos = 0;

	// collect solutions to reinsert
	if (pCurrentNode -> CollectOffsprings(iReinsertIndices) == false) {
		return false;	
	}

	iListPos = pCurrentNode -> m_iListPosition;
	pParentNode = pCurrentNode -> m_pParentNode;
	
	// remove TQuadNode
	if (pParentNode == NULL) {
		// delete root node
		delete m_pQuadRoot;
		m_pQuadRoot = NULL;
	} else {
		// delete child node
		if (RemoveChild(pParentNode, pCurrentNode) == false) {
			return false;
		}
	}

	// remove TProblemSolution after !!! node is removed
	pSolutionToDelete = (TProblemSolution *)(*this)[iListPos];
	(*this)[iListPos] = NULL;
	iSetSize -= 1;

	// update nadir
	// before counter iSetSize is updated 
	UpdateNadir(*pSolutionToDelete);

	delete pSolutionToDelete;
	pSolutionToDelete = NULL;

	pCurrentNode = NULL;

	return true;
}

template <class TProblemSolution>
bool TQuadTree <TProblemSolution>::FindRecursiveDominatingInTree(TSolution &oNewSolution,
																 TQuadNode &oQuadNode,
																 bool &bIsDominated)
{
vector <TQuadNode *>	oOffsprings;
vector <bool>			bNSuccessor;
TProblemSolution		*pCurrentSolution = NULL;
TCompare				iResult = _Equal;
unsigned int			i = 0;
	
	// check dominance relation
	// find children
	pCurrentSolution = (TProblemSolution *)(*this)[oQuadNode.m_iListPosition];

	iResult = pCurrentSolution -> Compare(oNewSolution);
	
	if ((iResult == _Dominating) || (iResult == _Equal)) {
		bIsDominated = true;
		return true;
	}

	if (pCurrentSolution -> CompareTo(oNewSolution, bNSuccessor) == false) {
		return false;	
	}

	oQuadNode.m_oOffsprings.FindLesser(bNSuccessor, oOffsprings);

	for(i = 0; i < oOffsprings.size(); i++) {
		if (FindRecursiveDominatingInTree(oNewSolution, *oOffsprings[i], bIsDominated) == false) {
			return false;
		}

		if (bIsDominated == true) {
			return true;
		}
	}

	return true;
}

template <class TProblemSolution>
bool TQuadTree <TProblemSolution>::FindDominatingInTree(TSolution &oNewSolution,
														bool &bIsDominated)
{
	bIsDominated = false;
	
	if (m_pQuadRoot != NULL) {
		if (FindRecursiveDominatingInTree(oNewSolution, *m_pQuadRoot, bIsDominated) == false) {
			return false;
		}

		if (bIsDominated == true) {
			return true;
		}
	}

	return true;
}


template <class TProblemSolution>
bool TQuadTree <TProblemSolution>::FindRecursiveAndRemoveDominatedInTree(TSolution &oNewSolution,
																		 TQuadNodePtr &pCurrentNode,
																		 vector <int> &iReinsertIndices)
{
vector <TQuadNode *>	oOffsprings;
vector <bool>			bNSuccessor;
TProblemSolution		*pCurrentSolution = NULL;
TCompare				iResult = _Equal;
unsigned int			i = 0;

	pCurrentSolution = (TProblemSolution *)(*this)[pCurrentNode -> m_iListPosition];

	iResult = pCurrentSolution -> Compare(oNewSolution);

	// if root is dominated remove it
	if (iResult == _Dominated) {
		// delete dominated node
		if (DeleteNode(pCurrentNode, 
					   iReinsertIndices) == false) {

			return false;	
		}

		return true;
	}

	if (pCurrentSolution -> CompareTo(oNewSolution, bNSuccessor) == false) {
		return false;	
	}

	pCurrentNode -> m_oOffsprings.FindGreater(bNSuccessor, oOffsprings);

	for(i = 0; i < oOffsprings.size(); i++) {
		if (FindRecursiveAndRemoveDominatedInTree(oNewSolution,
												  oOffsprings[i],
												  iReinsertIndices) == false) {

			return false;
		}
	}

	return true;
}


template <class TProblemSolution>
bool TQuadTree <TProblemSolution>::FindAndRemoveDominatedInTree(TSolution &oNewSolution,
																vector <int> &iReinsertIndices)
{
	iReinsertIndices.resize(0);

	if (m_pQuadRoot != NULL) {
		if (FindRecursiveAndRemoveDominatedInTree(oNewSolution, m_pQuadRoot, iReinsertIndices) == false) {
			return false;
		}
	}

	return true;
}


template <class TProblemSolution>
bool TQuadTree <TProblemSolution>::AddRecursiveNewSolution(TQuadNodePtr &pCurrentNode,
														   TSolution *pSolution)
{
TQuadNode			*pParentNode = NULL;
TQuadNode			*pQuadNode = NULL;	
vector <bool>		bKSuccessor;
TProblemSolution	*pCurrentSolution = NULL;
int					iCurrentListPosition = 0;

	
	pParentNode = pCurrentNode -> m_pParentNode;
	iCurrentListPosition =  pCurrentNode -> m_iListPosition;
	
	if (iCurrentListPosition >= 0) {
		pCurrentSolution = (TProblemSolution *)(*this)[iCurrentListPosition];
	} else {
		assert(false);
	}


	if (pCurrentSolution -> CompareTo(*pSolution, bKSuccessor) == false) {
		return false;	
	}

	// insert if not exist
	pQuadNode = NULL;
	pCurrentNode -> m_oOffsprings.FindNode(bKSuccessor, &pQuadNode);

	// in this direction is a subnode
	if (pQuadNode != NULL) {
		// go to subnode
		if (AddRecursiveNewSolution(pQuadNode, 
									pSolution) == false) {

			return false;
		}

		return true;
	} else {
		if (AddNode(pCurrentNode, pSolution, -1, bKSuccessor) == false) {
			return false;
		}
			
		return true;
	}
	
	return true;
}


template <class TProblemSolution>
bool TQuadTree <TProblemSolution>::AddNewSolution(TSolution *pSolution)
{	
	// insert root if not exist
	if (m_pQuadRoot == NULL) {
		vector <bool>  bKSuccessor;
		
		if (AddNode(m_pQuadRoot, pSolution, -1, bKSuccessor) == false) {
			return false;
		}
			
		return true;
	} else {
		if (AddRecursiveNewSolution(m_pQuadRoot,
									pSolution) == false) {

			return false;
		}
	}

	return true;
}

template <class TProblemSolution>
bool TQuadTree <TProblemSolution>::AddRecursiveExistSolution(TQuadNodePtr &pCurrentNode,
															 int iListPosition)
{
TQuadNode			*pParentNode = NULL;
TQuadNode			*pQuadNode = NULL;	
vector <bool>		bKSuccessor;
TProblemSolution	*pCurrentSolution = NULL;
TSolution			*pSolution;
int					iCurrentListPosition = 0;

	
	pParentNode = pCurrentNode -> m_pParentNode;
	iCurrentListPosition =  pCurrentNode -> m_iListPosition;
	
	if (iCurrentListPosition >= 0) {
		pCurrentSolution = (TProblemSolution *)(*this)[iCurrentListPosition];
	} else {
		assert(false);
	}

	assert(((*this)[iListPosition] != NULL));
	pSolution = (*this)[iListPosition];

	if (pCurrentSolution -> CompareTo(*pSolution, bKSuccessor) == false) {
		return false;	
	}

	// insert if not exist
	pQuadNode = NULL;
	pCurrentNode -> m_oOffsprings.FindNode(bKSuccessor, &pQuadNode);

	// in this direction is a subnode
	if (pQuadNode != NULL) {
		// go to subnode
		if (AddRecursiveExistSolution(pQuadNode, 
									  iListPosition) == false) {

			return false;
		}

		return true;
	} else {
		if (AddNode(pCurrentNode, NULL, iListPosition, bKSuccessor) == false) {
			return false;
		}
			
		return true;
	}
	
	return true;
}

template <class TProblemSolution>
bool TQuadTree <TProblemSolution>::AddExistSolution(int iListPosition)
{	
	// insert root if not exist
	if (m_pQuadRoot == NULL) {
		vector <bool>  bKSuccessor;
		
		if (AddNode(m_pQuadRoot, NULL, iListPosition, bKSuccessor) == false) {
			return false;
		}
			
		return true;
	} else {
		if (AddRecursiveExistSolution(m_pQuadRoot,
									  iListPosition) == false) {

			return false;
		}
	}

	return true;
}

template <class TProblemSolution>
bool TQuadTree <TProblemSolution>::Update (TSolution& Solution)
{
bool					bIsDominated = false;
vector <int>			iReinsertIndices;
int						i = 0;
static int				iCountDown = 0;
//clock_t					dUT1 = 0.0;
//clock_t					dUT2 = 0.0;
//clock_t					dRT1 = 0.0;
//clock_t					dRT2 = 0.0;
	

	//dUT1 = clock();

	// rebuild tree be 100000
	//if ((iCountDown % 100000) == 100000 - 1) {
	//	CompactTree();	
	//}

	if (FindDominatingInTree(Solution, bIsDominated) == false) {
		//dUT2 = clock(); m_dUpdateTime += (double)dUT2 - dUT1;
		iListSize = size();
		return false;
	}

	if (bIsDominated == true) {
		//dUT2 = clock(); m_dUpdateTime += (double)dUT2 - dUT1;
		iListSize = size();
		return false;
	} 

	//dRT1 = clock();
	if (FindAndRemoveDominatedInTree(Solution, iReinsertIndices) == false) {
		//dUT2 = clock(); m_dUpdateTime += (double)dUT2 - dUT1;
		//dRT2 = clock(); m_dReinsertTime += (double)dRT2 - dRT1;
		iListSize = size();
		return false;
	}

	if (VerifyRemoved(Solution, iReinsertIndices) == false) {
		//dUT2 = clock(); m_dUpdateTime += (double)dUT2 - dUT1;
		//dRT2 = clock(); m_dReinsertTime += (double)dRT2 - dRT1;
		iListSize = size();
		return false;
	}
    
    if (m_bQueueSort == true) {
	    if (QueueSort(iReinsertIndices) == false) {
		    //dUT2 = clock(); m_dUpdateTime += (double)dUT2 - dUT1;
		    //dRT2 = clock(); m_dReinsertTime += (double)dRT2 - dRT1;
		    iListSize = size();
		    return false;
	    }
    }

	if (ReinsertRemoved(iReinsertIndices) == false) {
		//dUT2 = clock(); m_dUpdateTime += (double)dUT2 - dUT1;
		//dRT2 = clock(); m_dReinsertTime += (double)dRT2 - dRT1;
		iListSize = size();
		return false;
	}

	if (AddNewSolution(&Solution) == false) {
		//dUT2 = clock(); m_dUpdateTime += (double)dUT2 - dUT1;
		//dRT2 = clock(); m_dReinsertTime += (double)dRT2 - dRT1;
		iListSize = size();
		return false;
	}

	iListSize = size();

	//dRT2 = clock(); m_dReinsertTime += (double)dRT2 - dRT1;
	//dUT2 = clock(); m_dUpdateTime += (double)dUT2 - dUT1;
	
	return true;
}

template <class TProblemSolution>
bool TQuadTree <TProblemSolution>::Update(TNondominatedSet& NondominatedSet) 
{	
bool bAdded = false;

	unsigned int i;
	for (i = 0; i < NondominatedSet.size(); i++) {
		if (NondominatedSet[i] != NULL) {
			bool bTemp = (Update(*(NondominatedSet[i])));
			bAdded = bAdded || bTemp;
		}
	}

    return bAdded; 
}

template <class TProblemSolution>
ostream& TQuadTree <TProblemSolution>::Save(ostream& oStream, int iSpace)
{
char	*pSpace = NULL;

	pSpace = new char [iSpace + 1];
	memset(pSpace, 0, (iSpace + 1) * sizeof(char));
	for(int i = 0; i < iSpace; i++) {
		pSpace[i] = ' ';
	}

	if (m_pQuadRoot != NULL) {
		oStream << pSpace << "<BTREE>\n";
		m_pQuadRoot -> Save(oStream, iSpace + 1);
		oStream << pSpace << "</BTREE>\n";
	}

	if (pSpace != NULL) {
		delete pSpace;
		pSpace = NULL;
	}

	return oStream;
}

/**
 * Quick sort (sort indices by given objective)
 *
 **/
template <class TProblemSolution>
void  TQuadTree <TProblemSolution>::QueueSortObjective(int iObjective, int iStartPos,int iEndPos, vector <int> &iIndices, vector <int> &iPositions)
{
int			iCurrentStartPos = 0;
int			iCurrentEndPos = 0; 
int			i = 0;
double		dSplitValue = 0.0;

	if (Objectives[iObjective].bActive == false) {
		return;
	}

	iCurrentStartPos = iStartPos; 
	iCurrentEndPos = iEndPos;
	
	dSplitValue = ((*this)[iIndices[(iStartPos + iEndPos) / 2]]) -> ObjectiveValues[iObjective];

	do {
		if (Objectives[iObjective].ObjectiveType == _Max) {
			while((*this)[iIndices[iCurrentStartPos]] -> ObjectiveValues[iObjective] < dSplitValue) {
				iCurrentStartPos += 1;
			}

			while((*this)[iIndices[iCurrentEndPos]] -> ObjectiveValues[iObjective] > dSplitValue) {
				iCurrentEndPos -= 1;
			}
		} else {
			while((*this)[iIndices[iCurrentStartPos]] -> ObjectiveValues[iObjective] > dSplitValue) {
				iCurrentStartPos += 1;
			}

			while((*this)[iIndices[iCurrentEndPos]] -> ObjectiveValues[iObjective] < dSplitValue) {
				iCurrentEndPos -= 1;
			}		
		}

		if (iCurrentStartPos <= iCurrentEndPos)
		{
			i = iIndices[iCurrentStartPos]; 
			iIndices[iCurrentStartPos] = iIndices[iCurrentEndPos]; 
			iIndices[iCurrentEndPos] = i;

			i = iPositions[iCurrentStartPos]; 
			iPositions[iCurrentStartPos] = iPositions[iCurrentEndPos]; 
			iPositions[iCurrentEndPos] = i;

			iCurrentStartPos += 1;
			iCurrentEndPos -= 1;
		}
	} while(iCurrentStartPos <= iCurrentEndPos);

	if (iStartPos < iCurrentEndPos)  {
		QueueSortObjective(iObjective, iStartPos, iCurrentEndPos, iIndices, iPositions);
	}

	if (iEndPos > iCurrentStartPos)  {
		QueueSortObjective(iObjective, iCurrentStartPos, iEndPos, iIndices, iPositions);
	}
}

template <class TProblemSolution>
void  TQuadTree <TProblemSolution>::QueueSortReinsert(int iStart, int iEnd, vector <IVector> &oIndicesMatrix, vector <IVector> &oPositionMatrix, vector <int> &iReinsertIndices, vector <bool> &bInserted)
{
int		iCurrent = 0;
int		i = 0;
int		iPosition = 0;

	if (iStart <= iEnd) {
		iCurrent = (iStart + iEnd) / 2;
		
		// reinsert sorted indices
		for(i = 0; i < NumberOfObjectives; i++) {
			iPosition = oPositionMatrix[i][iCurrent];

			if (bInserted[iPosition] == false) {
				bInserted[iPosition] = true;

				iReinsertIndices.push_back(oIndicesMatrix[i][iCurrent]);
			}
		}

		if ((iStart < iCurrent) && (iCurrent < iEnd)) {
			QueueSortReinsert(iStart, iCurrent, oIndicesMatrix, oPositionMatrix, iReinsertIndices, bInserted);
		}

		if ((iEnd > iCurrent) && (iCurrent > iStart)) {
			QueueSortReinsert(iCurrent, iEnd, oIndicesMatrix, oPositionMatrix, iReinsertIndices, bInserted);
		}
	}
}

template <class TProblemSolution>
bool TQuadTree <TProblemSolution>::QueueSort(vector <int> &iReinsertIndices)
{
vector <IVector>	oIndicesMatrix;
vector <IVector>	oPositionMatrix;
vector <bool>		bInserted;
int					i = 0;
int					j = 0;
int					iListSize = 0;
int					iPosition = 0;

	iListSize  = iReinsertIndices.size();
	
	if (iListSize <= 0) {
		// nothing to sort
		return true;
	}

	oIndicesMatrix.resize(NumberOfObjectives);
	oPositionMatrix.resize(NumberOfObjectives);

	for(i = 0; i < NumberOfObjectives; i++) {
		oIndicesMatrix[i].resize(iListSize);
		oPositionMatrix[i].resize(iListSize);
		
		for(j = 0; j < iListSize; j++) {
			oIndicesMatrix[i][j] = iReinsertIndices[j];
			oPositionMatrix[i][j] = j;
		}

		// sort indices in matrix using objective values
		QueueSortObjective(i, 0, iListSize - 1, oIndicesMatrix[i], oPositionMatrix[i]);
	}

	bInserted.resize(iListSize);
	for(i = 0; i < iListSize; i++) {
		bInserted[i] = false;
	}
		
	// remove old indices
	iReinsertIndices.clear();	

	// reinsert from sort table
	QueueSortReinsert(0, iListSize - 1, oIndicesMatrix, oPositionMatrix, iReinsertIndices, bInserted);

	// reinsert rest of indices
	//for(i = 0; i < NumberOfObjectives; i++) {
	//	for(j = 0; j < iListSize; j++) {
	//		iPosition = oPositionMatrix[i][j];

	//		if (bInserted[iPosition] == false) {
	//			assert(false);
				
	//			bInserted[iPosition] = true;
	//
	//			iReinsertIndices.push_back(oIndicesMatrix[i][j]);
	//		}
	//	}
	//}

	assert(iListSize == iReinsertIndices.size());

	return true;
}


template <class TProblemSolution>
bool TQuadTree <TProblemSolution>::QueueRandomly(vector <int> &iReinsertIndices)
{
time_t						oTime;
TRandomGenerator			oGenerator;
int							i = 0;
int							iNewPos = 0;
int							iValue = 0;

	// randomize hash table
	time(&oTime);
	oGenerator.SetSeed((long)oTime);
	for(i = 0; i < iReinsertIndices.size(); i++) {
		int iValue = 0;
		int	iNewPos = 0;
		
		iNewPos = oGenerator.GetLongRND(iReinsertIndices.size() - 1);
		iValue = iReinsertIndices[i];
		iReinsertIndices[i] = iReinsertIndices[iNewPos];
		iReinsertIndices[iNewPos] = iValue;
	}

	return true;
}


template <class TProblemSolution>
bool TQuadTree <TProblemSolution>::CompactTree()
{
vector <int>				iReinsertIndices;
vector	<TSolution *>		pNewTable;
unsigned int				i = 0;

	pNewTable.clear();
	iReinsertIndices.clear();
	pNewTable.reserve(size());
	iReinsertIndices.reserve(size());

	// remove whole tree
	if (m_pQuadRoot != NULL) {
		delete m_pQuadRoot;
		m_pQuadRoot = NULL;
	}

	// compact pointers
	for(i = 0; i < size(); i++) {
		if ((*this)[i] != NULL) {
			pNewTable.push_back((*this)[i]);			
		}
	}

	// move solutions
	clear();
	for(i = 0; i < pNewTable.size(); i++) {
		push_back(pNewTable[i]);
	}

	//
	// insert randomly choosed solutions
	// 
	// prepare hash table
	iReinsertIndices.resize(size());
	for(i = 0; i < iReinsertIndices.size(); i++) {
		iReinsertIndices[i] = i;
	}

	if (QueueSort(iReinsertIndices) == false) {
		return false;
	}
	
	if (ReinsertRemoved(iReinsertIndices) == false) {
		return false;
	}

	return true;
}

template <class TProblemSolution>
void TQuadTree <TProblemSolution>::DeleteAll()
{
	// remove whole tree
	if (m_pQuadRoot != NULL) {
		delete m_pQuadRoot;
		m_pQuadRoot = NULL;
	}
	
	TSolutionsSet::DeleteAll();
	resize(0);

	iSetSize = 0;
	iListSize = 0;

	m_dUpdateTime = 0;
	m_dReinsertTime = 0;
}

template <class TProblemSolution>
void TQuadTree <TProblemSolution>::GetRandomSolution(TSolution* &pSolution)
{
int						iIndex = 0;
int						iTimeout = 10;

	// old code
	if (iSetSize <= 0) {
		pSolution = NULL;
	} else {
		if (iSetSize * 2 < iListSize) {
			// tree is not efficient
			// it is neccessary to compact it
			CompactTree();	
		}

		do {
			iTimeout -= 1;
			iIndex = (int)(rand()/(RAND_MAX + 1.0)*(double)iListSize);
			pSolution = (TSolution*)(*this)[iIndex];
		} while((iTimeout >= 0) && (pSolution == NULL));
	}
};

template <class TProblemSolution>
void TQuadTree <TProblemSolution>::UpdateNadir(TSolution& Solution)
{
int				iObjective = 0;
unsigned int	i = 0;
bool			bFirst = true;

	if (iSetSize == 0) {
		// if no solutions on list erase values
		for(iObjective = 0; iObjective < NumberOfObjectives; iObjective++) {
			ApproximateNadirPoint.ObjectiveValues[iObjective] = 0.0;
			ApproximateIdealPoint.ObjectiveValues[iObjective] = 0.0;
		}
	} else {
		// if some solutions exist update nadir values
		for(iObjective = 0; iObjective < NumberOfObjectives; iObjective++) {
			if (Objectives[iObjective].bActive == true) {
				if (Solution.ObjectiveValues[iObjective] == ApproximateNadirPoint.ObjectiveValues[iObjective]) {

					bFirst = true;

					// update objectives from list
					for(i = 0; i < size(); i++) {
						// ommit empty entries
						if ((*this)[i] == NULL) {
							continue;
						}

						if (bFirst == true) {
							bFirst = false;
	
							// set nadir point
							ApproximateNadirPoint.ObjectiveValues[iObjective] = (*this)[i]->ObjectiveValues[iObjective];
						} else {
							// update nadir point
							if (Objectives[iObjective].ObjectiveType == _Max) {
								if (ApproximateNadirPoint.ObjectiveValues[iObjective] > (*this)[i]->ObjectiveValues[iObjective]) {
									ApproximateNadirPoint.ObjectiveValues[iObjective] = (*this)[i]->ObjectiveValues[iObjective];
								}
							}

							if (Objectives[iObjective].ObjectiveType == _Min) {
								if (ApproximateNadirPoint.ObjectiveValues[iObjective] < (*this)[i]->ObjectiveValues[iObjective]) {
									ApproximateNadirPoint.ObjectiveValues[iObjective] = (*this)[i]->ObjectiveValues[iObjective];
								}
							}
						}
					}
				}
			}
		}
	}
}

/**
 * Use this function before counter iSetSize has been updated
 **/

template <class TProblemSolution>
void TQuadTree <TProblemSolution>::UpdateIdealAndNadir(TSolution& Solution)
{
int				iObjective = 0;
	
	if (iSetSize == 0) {
		for(iObjective = 0; iObjective < NumberOfObjectives; iObjective++) {
			if (Objectives[iObjective].bActive == true) {
				ApproximateIdealPoint.ObjectiveValues[iObjective] = Solution.ObjectiveValues[iObjective];
				ApproximateNadirPoint.ObjectiveValues[iObjective] = Solution.ObjectiveValues[iObjective];
			} else {
				ApproximateIdealPoint.ObjectiveValues[iObjective] = 0;
				ApproximateNadirPoint.ObjectiveValues[iObjective] = 0;
			}
		}
	} else {
		for(iObjective = 0; iObjective < NumberOfObjectives; iObjective++) {
			if (Objectives[iObjective].bActive == true) {

				if (Objectives[iObjective].ObjectiveType == _Max) {				
					// nadir
					if (ApproximateIdealPoint.ObjectiveValues[iObjective] < Solution.ObjectiveValues[iObjective]) {
						ApproximateIdealPoint.ObjectiveValues[iObjective] = Solution.ObjectiveValues[iObjective];
					}

					// ideal
					if (ApproximateNadirPoint.ObjectiveValues[iObjective] > Solution.ObjectiveValues[iObjective]) {
						ApproximateNadirPoint.ObjectiveValues[iObjective] = Solution.ObjectiveValues[iObjective];
					}
				}

				if (Objectives [iObjective].ObjectiveType == _Min) {
					// nadir
					if (ApproximateIdealPoint.ObjectiveValues[iObjective] > Solution.ObjectiveValues[iObjective]) {
						ApproximateIdealPoint.ObjectiveValues[iObjective] = Solution.ObjectiveValues[iObjective];
					}

					// ideal
					if (ApproximateNadirPoint.ObjectiveValues[iObjective] < Solution.ObjectiveValues[iObjective]) {
						ApproximateNadirPoint.ObjectiveValues[iObjective] = Solution.ObjectiveValues[iObjective];
					}
				}
			}
		}
	}
}

template <class TProblemSolution>
bool TQuadTree <TProblemSolution>::ReduceSetByClustering(int iPopulationSize, bool bScalarize)
{
vector <int>		                iIndices;
TListSet <TProblemSolution>		    oTmpSet;
unsigned int		                i = 0;

	if (ReduceIndividuals(iPopulationSize, iIndices, OBJECTIVE_SPACE, bScalarize) == false)
	{
		return false;	
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
