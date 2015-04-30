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

#include "tquadnode.h"
#include "tbtree.h"
#include <assert.h>
#include <stdio.h>


TQuadNode::TQuadNode()
{
	m_pParentNode = NULL;
	m_iListPosition = 0;
}

TQuadNode::~TQuadNode()
{
	m_oOffsprings.ReleaseTree();
	m_iListPosition = 0;
	m_pParentNode = NULL;
}

bool TQuadNode::CollectOffsprings(vector <int> &iIndices)
{
unsigned int				i = 0;
vector <TQuadNode *>		oTmpQuadNodes;
	
	if (m_oOffsprings.FindAll(m_oOffsprings.m_oRoot, oTmpQuadNodes) == false) {
		return false;
	}	

	for(i = 0; i < oTmpQuadNodes.size(); i++) {
		iIndices.push_back(oTmpQuadNodes[i] -> m_iListPosition);
	}

	for(i = 0; i < oTmpQuadNodes.size(); i++) {
		if (oTmpQuadNodes[i] -> CollectOffsprings(iIndices) == false) {
			return false;	
		}
	}

	return true;
}

ostream& TQuadNode::Save(ostream& oStream, int iSpace = 0)  
{
char	szBuffer[256] = {0};
char	*pSpace = NULL;

	pSpace = new char [iSpace + 1];
	memset(pSpace, 0, (iSpace + 1) * sizeof(char));
	for(int i = 0; i < iSpace; i++) {
		pSpace[i] = ' ';
	}

	oStream << pSpace << "<QUADNODE ";
	sprintf(szBuffer, "%lx\0", this);
	oStream << "Address = \"" << szBuffer << "\" ";
	sprintf(szBuffer, "%lx\0", m_iListPosition);
	oStream << "ListPosition = \"" << szBuffer << "\" ";
	oStream << ">\n";
	
	m_oOffsprings.Save(oStream, iSpace + 1);

	oStream.flush();

	oStream << pSpace << "</QUADNODE>\n";

	oStream.flush();

	if (pSpace != NULL) {
		delete pSpace;
		pSpace = NULL;
	}

	return oStream;
}
