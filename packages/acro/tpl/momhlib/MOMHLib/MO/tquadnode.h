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

#ifndef __TQUADNODE_H_
#define __TQUADNODE_H_

#include "tbtree.h"

/** Node of quad tree
 *
 * This class contains node of quad tree used to bulid tree. Every node has
 * reference (position) to nondominated point in objective space 
 * (image of nondominated solution in objective space) 
 * stored in vector of TQuadTree.
 **/
class TQuadNode 
{
public:

    /** Construct nodes **/
	TQuadNode();

    /** Release nodes and sub nodes **/
    virtual ~TQuadNode();

    /** Get all offsprings of given node 
     * 
     * This function browses subtree and returns vector of indices contains references to
     * child points. Intices point to TQuadTree vector of nondominated set  
     *
     * @param iIndices list of offspring indices
     * @return true - success, false - error 
     **/
	bool CollectOffsprings(vector<int> &iIndices);

   /** Save quad node to stream.
     *  
     * This function allows to serialize node structure (with offsprings!).
     *
     * @param oStream output stream
     * @param iSpace number of spaces used in indentations generation
     * @return output stream
     **/
	ostream& Save(ostream& oStream, int iSpace);

	/** Reference to nondominated solution stored in TQuadTree vector **/
    int				m_iListPosition;

    /** Binary tree object contains children nodes **/
	TBTree			m_oOffsprings;

    /** Pointer to parent TQuadNode - very usefull **/
	TQuadNode		*m_pParentNode;
};

/** Type to simplify function parameters **/
typedef TQuadNode *			TQuadNodePtr;

#endif 
