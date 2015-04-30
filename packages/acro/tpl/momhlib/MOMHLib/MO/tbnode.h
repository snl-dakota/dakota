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

#ifndef __TBNODE_H_
#define __TBNODE_H_

#include "global.h"
#include "solution.h"

class TQuadNode;


/** This class is node pattern for binary tree.
 * 
 * This node is used by TBTree as building block of binary tree.
 **/
class TBNode  
{
public:
	TBNode();
	virtual ~TBNode();

    /** Save node of tree to stream.
     *  
     * This function allows to serialize binary tree structure.
     *
     * @param Stream output stream
     * @param iSpace number of spaces used in indentations generation
     * @return output stream
     **/
	ostream& Save(ostream& Stream, int iSpace);

    /** Value of binary node (parent connection) true-1 or false-0 **/ 
	bool		m_bValue;

    /** Pointer to zero offspring **/
	TBNode		*m_pOneOffspring;

    /** Pointer to one offspring **/
	TBNode		*m_pZeroOffspring;

    /** Pointer to nested quead node. Used only in case when node is a leaf. **/
	TQuadNode	*m_pQuadNode;
};

#endif
