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

#ifndef __TBTREE_H_
#define __TBTREE_H_

#include "global.h"
#include "solution.h"
#include "tbnode.h"

class TQuadNode;

/** This class implements binary tree.
 * 
 * This binary tree has been implemented to improve access to child node in quad
 * tree. Thanks to tree access to next point is perform in logaritmic time.
 **/
class TBTree  
{
public:

    /** Remove whole tree and release every child QuadNode. 
     *
     * This function recursively release sub tree of quad node. 
     **/
	void ReleaseTree();

    /** Remove QuadNode from tree but not destroy this object.
     *
     * This function release path in tree to element and release QuadNode placed in leaf.
     *
     * @param oValue path to leaf
     * @param pPreviousNode returned node
     * @return true - success, false - error (incorrect path to node)
     **/
	bool RemoveBNode(vector<bool> &oValue, TQuadNode **pPreviousNode);

    /** Add QuadNode to tree.
     *
     * This function release path in tree to element and release QuadNode placed in leaf.
     *
     * @param oValue path to leaf
     * @param pQuadNode node that should be add
     * @param pPreviousNode pointer to previous QuadNode in tree
     * @return true - success, false - error
     **/
	bool AddBNode(vector<bool> &oValue, TQuadNode *pQuadNode, TQuadNode **pPreviousNode);

    /** Find QuadNode in Tree.
     *
     * This function finds QuadNode element in tree.
     *
     * @param oValue path to leaf
     * @param pPreviousNode stored pointer to node if has been found or null
     * @return true - success, false - error
     **/
	bool FindNode(vector<bool> &oValue, TQuadNode **pPreviousNode);

    /** Find set of nodes that contains more 'one' on path vector thet oValue.
     *
     * This function gets every child node that in path has more 'one' in vector and
     * also 'one' on the same places.
     *
     * @param oValue path to leaf
     * @param oNodes vector of pointers to retrieved solutions
     * @return true - success, false - error
     **/
	bool FindGreater(vector<bool> &oValue, vector<TQuadNode *> &oNodes);

    /** Find set of nodes that contains more 'zero' on path vector thet oValue.
     *
     * This function gets every child node that in path has more 'zero' in vector and
     * also 'zero' on the same places.
     *
     * @param oValue path to leaf
     * @param oNodes vector of pointers to retrieved solutions
     * @return true - success, false - error
     **/
	bool FindLesser(vector<bool> &oValue, vector<TQuadNode *> &oNodes);

    /** Retrieve every QuadNode stored in tree.
     *
     * This function gets every child node stored in binary tree
     *
     * @param oRootNode root node of tree
     * @param oNodes vector of pointers to retrieved solutions
     * @return true - success, false - error
     **/
    bool FindAll(TBNode &oRootNode, vector<TQuadNode *> &oNodes);


    /** Save tree to stream.
     *  
     * This function allows to serialize binary tree structure.
     *
     * @param oStream output stream
     * @param iSpace number of spaces used in indentations generation
     * @return output stream
     **/
	ostream& Save(ostream& oStream, int iSpace);

	TBTree();

	virtual ~TBTree();

    /** Root node to tree **/
	TBNode		m_oRoot;

private:

    /** Recursive function to retrieve all solutions.
     *
     * @param pNode parent node
     * @param oNodes vector of pointers to retrieved solutions
     * @return true - success, false - error
     **/
	bool GetRecursiveAll(TBNode *pNode, vector<TQuadNode *> &oNodes);

    /** Recursive function to retrieve greater solutions.
     *
     * @param pNode parent node
     * @param iLevel index of level in tree
     * @param bEqual true - equal element will also in set
     * @param oValue path used as a point of reference
     * @param oNodes vector of pointers to retrieved solutions
     * @return true - success, false - error
     **/
	bool GetRecursiveGreater(TBNode *pNode, unsigned int iLevel, bool bEqual, vector<bool> &oValue, vector<TQuadNode *> &oNodes);

    /** Recursive function to retrieve lesser solutions.
     *
     * @param pNode parent node
     * @param iLevel index of level in tree
     * @param bEqual true - equal element will also in set
     * @param oValue path used as a point of reference
     * @param oNodes vector of pointers to retrieved solutions
     * @return true - success, false - error
     **/
    bool GetRecursiveLesser(TBNode *pNode, unsigned int iLevel, bool bEqual, vector<bool> &oValue, vector<TQuadNode *> &oNodes);
};

#endif
