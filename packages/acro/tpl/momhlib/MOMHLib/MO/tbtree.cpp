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

#include "tbtree.h"
#include "tquadnode.h"

TBTree::TBTree()
{
}

TBTree::~TBTree()
{
	ReleaseTree();
}

bool TBTree::AddBNode(vector<bool> &oValue, TQuadNode *pQuadNode, TQuadNode **pPreviousNode)
{
int						i = 0;
TBNode					*pNode = &m_oRoot;

	if ((oValue.size() != NumberOfObjectives) || 
		(pQuadNode == NULL) || 
		(pPreviousNode == NULL)) {

		// not compatible quadrant or
		// mising pointers
		return false;
	}

	*pPreviousNode = NULL;

	for(i = 0; i < NumberOfObjectives; i++) {
		if (oValue[i] == true) {
			if (pNode -> m_pOneOffspring == NULL) {
				pNode -> m_pOneOffspring = new TBNode;
				pNode -> m_pOneOffspring -> m_bValue = true;
			}
			pNode = pNode -> m_pOneOffspring;
		} else {
			if (pNode -> m_pZeroOffspring == NULL) {
				pNode -> m_pZeroOffspring = new TBNode;
				pNode -> m_pZeroOffspring -> m_bValue = false;
			}
			pNode = pNode -> m_pZeroOffspring;
		}
	}

	// pNode contain leaf ot b-tree 
	// now is necessary to assignd QuadNode;
	if (pNode -> m_pQuadNode != NULL) {
		*pPreviousNode = pNode -> m_pQuadNode;
	}
	pNode -> m_pQuadNode = pQuadNode;

	return true;
}

bool TBTree::RemoveBNode(vector<bool> &oValue, TQuadNode **pPreviousNode)
{
int						i = 0;
TBNode					*pNode = &m_oRoot;
vector	<TBNode *>		oNodeVector;

	if ((oValue.size() != NumberOfObjectives) || 
		(pPreviousNode == NULL)) {

		// not compatible quadrant or
		// mising pointers
		return false;
	}
	
	*pPreviousNode = NULL;
	oNodeVector.resize(NumberOfObjectives);

	for(i = 0; i < NumberOfObjectives; i++) {
		oNodeVector[i] = NULL;

		if (oValue[i] == true) {
			if (pNode -> m_pOneOffspring == NULL) {
				// no such pointer was stored
				return false;
			}
			pNode = pNode -> m_pOneOffspring;
		} else {
			if (pNode -> m_pZeroOffspring == NULL) {
				// no such pointer was stored
				return false;
			}
			pNode = pNode -> m_pZeroOffspring;
		}

		// store nodes for remove operations
		oNodeVector[i] = pNode; 
	}

	// this procedure doesn't remove final pointer
	if (pNode -> m_pQuadNode != NULL) {
		*pPreviousNode = pNode -> m_pQuadNode;
		pNode -> m_pQuadNode = NULL;
	}

	// go back and remove child
	for(i = NumberOfObjectives - 1; i >= 0; i--) {
		pNode = oNodeVector[i];
		if ((pNode -> m_pOneOffspring == NULL) &&
			(pNode -> m_pZeroOffspring == NULL) &&
			(pNode -> m_pQuadNode == NULL)) {

			delete pNode;
			pNode = NULL;
			oNodeVector[i] = NULL;

			if (i > 0) {
				if (oValue[i] == true) {
					oNodeVector[i - 1] -> m_pOneOffspring = NULL;
				} else {
					oNodeVector[i - 1] -> m_pZeroOffspring = NULL;
				}
			} else {
				if (oValue[0] == true) {
					m_oRoot.m_pOneOffspring = NULL;
				} else {
					m_oRoot.m_pZeroOffspring = NULL;
				}
			}
		} else {
			return true;
		}
	}

	return true;
}

bool TBTree::FindNode(vector<bool> &oValue, TQuadNode **pPreviousNode)
{
int						i = 0;
TBNode					*pNode = &m_oRoot;

	if ((oValue.size() != NumberOfObjectives) || 
		(pPreviousNode == NULL)) {

		// not compatible quadrant or
		// mising pointers
		return false;
	}
	
	*pPreviousNode = NULL;

	for(i = 0; i < NumberOfObjectives; i++) {
		if (oValue[i] == true) {
			if (pNode -> m_pOneOffspring == NULL) {
				// no such pointer was stored
				return false;
			}
			pNode = pNode -> m_pOneOffspring;
		} else {
			if (pNode -> m_pZeroOffspring == NULL) {
				// no such pointer was stored
				return false;
			}
			pNode = pNode -> m_pZeroOffspring;
		}
	}

	// return associated poiner
	if (pNode -> m_pQuadNode != NULL) {
		*pPreviousNode = pNode -> m_pQuadNode;
	}

	return true;
}

bool TBTree::GetRecursiveGreater(TBNode *pNode, unsigned int iLevel, bool bEqual, vector<bool> &oValue, vector<TQuadNode *> &oNodes)
{
	if (((pNode -> m_pOneOffspring != NULL) ||
		(pNode -> m_pZeroOffspring != NULL)) &&
		(pNode -> m_pQuadNode != NULL)) {

		return false;
	}

	if (iLevel < oValue.size()) {
		if (oValue[iLevel] == false) {
			// go to subnode if exist
			if (pNode -> m_pOneOffspring != NULL) {
				if (GetRecursiveGreater(pNode -> m_pOneOffspring, iLevel + 1, false, oValue, oNodes) == false) {
					return false;
				}
			}

			// go to subnode if exist
			if (pNode -> m_pZeroOffspring != NULL) {
				if (GetRecursiveGreater(pNode -> m_pZeroOffspring, iLevel + 1, bEqual, oValue, oNodes) == false) {
					return false;
				}
			}
		} else {
			// if must be zero go to zero only 
			if (pNode -> m_pOneOffspring != NULL) {
				if (GetRecursiveGreater(pNode -> m_pOneOffspring, iLevel + 1, bEqual, oValue, oNodes) == false) {
					return false;
				}
			}
		}
	}

	if ((pNode -> m_pZeroOffspring == NULL) && 
		(pNode -> m_pOneOffspring == NULL) &&
		(pNode -> m_pQuadNode != NULL) /*&&
		(bEqual == false)*/) {
		
		oNodes.push_back(pNode -> m_pQuadNode);
	}

	return true;
}

bool TBTree::FindGreater(vector<bool> &oValue, vector<TQuadNode *> &oNodes)
{
	oNodes.resize(0);

	if (GetRecursiveGreater(&m_oRoot, 0, true, oValue, oNodes) == false) {
		return false;
	}

	return true;
}

bool TBTree::GetRecursiveLesser(TBNode *pNode, unsigned int iLevel, bool bEqual, vector<bool> &oValue, vector<TQuadNode *> &oNodes)
{
	if (((pNode -> m_pOneOffspring != NULL) ||
		(pNode -> m_pZeroOffspring != NULL)) &&
		(pNode -> m_pQuadNode != NULL)) {

		return false;
	}

	if (iLevel < oValue.size()) {
		if (oValue[iLevel] == true) {
			// go to subnode if exist
			if (pNode -> m_pOneOffspring != NULL) {
				if (GetRecursiveLesser(pNode -> m_pOneOffspring, iLevel + 1, bEqual, oValue, oNodes) == false) {
					return false;
				}
			}

			// go to subnode if exist
			if (pNode -> m_pZeroOffspring != NULL) {		
				if (GetRecursiveLesser(pNode -> m_pZeroOffspring, iLevel + 1, false, oValue, oNodes) == false) {
					return false;
				}
			}
		} else {
			// if must be zero go to zero only 
			if (pNode -> m_pZeroOffspring != NULL) {
				if (GetRecursiveLesser(pNode -> m_pZeroOffspring, iLevel + 1, bEqual, oValue, oNodes) == false) {
					return false;
				}
			}
		}
	}

	if ((pNode -> m_pZeroOffspring == NULL) && 
		(pNode -> m_pOneOffspring == NULL) &&
		(pNode -> m_pQuadNode != NULL) /*&&
		(bEqual == false)*/) {
		
		oNodes.push_back(pNode -> m_pQuadNode);
	}

	return true;
}

bool TBTree::FindLesser(vector<bool> &oValue, vector<TQuadNode *> &oNodes)
{
	oNodes.resize(0);

	if (GetRecursiveLesser(&m_oRoot, 0, true, oValue, oNodes) == false) {
		return false;
	}

	return true;
}

bool TBTree::GetRecursiveAll(TBNode *pNode, vector<TQuadNode *> &oNodes)
{
	if (((pNode -> m_pOneOffspring != NULL) ||
		(pNode -> m_pZeroOffspring != NULL)) &&
		(pNode -> m_pQuadNode != NULL)) {

		return false;
	}

	// go to subnode if exist
	if (pNode -> m_pOneOffspring != NULL) {
		if (GetRecursiveAll(pNode -> m_pOneOffspring, oNodes) == false) {
			return false;
		}
	}

	// go to subnode if exist
	if (pNode -> m_pZeroOffspring != NULL) {
		if (GetRecursiveAll(pNode -> m_pZeroOffspring, oNodes) == false) {
			return false;
		}
	}
	
	if ((pNode -> m_pZeroOffspring == NULL) && 
		(pNode -> m_pOneOffspring == NULL) &&
		(pNode -> m_pQuadNode != NULL)) {
		
		oNodes.push_back(pNode -> m_pQuadNode);
	}

	return true;
}


bool TBTree::FindAll(TBNode &oRootNode, vector<TQuadNode *> &oNodes)
{
	oNodes.resize(0);
	if (GetRecursiveAll(&oRootNode, oNodes) == false) {
		return false;
	} 

	return true;
}

void TBTree::ReleaseTree()
{
	if (m_oRoot.m_pOneOffspring != NULL) {
		delete m_oRoot.m_pOneOffspring;
		m_oRoot.m_pOneOffspring = NULL;
	}

	if (m_oRoot.m_pZeroOffspring != NULL) {
		delete m_oRoot.m_pZeroOffspring;
		m_oRoot.m_pZeroOffspring = NULL;
	}

	if (m_oRoot.m_pQuadNode != NULL) {
		delete m_oRoot.m_pQuadNode;
		m_oRoot.m_pQuadNode = NULL;
	}
}

ostream& TBTree::Save(ostream& oStream, int iSpace = 0)  
{
char	*pSpace = NULL;

	pSpace = new char [iSpace + 1];
	memset(pSpace, 0, (iSpace + 1) * sizeof(char));
	for(int i = 0; i < iSpace; i++) {
		pSpace[i] = ' ';
	}

	oStream << pSpace << "<BTREE>\n";
	m_oRoot.Save(oStream, iSpace + 1);
	oStream << pSpace << "</BTREE>\n";

	if (pSpace != NULL) {
		delete pSpace;
		pSpace = NULL;
	}

	return oStream;
}
