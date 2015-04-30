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

#include "tbnode.h"
#include "tquadnode.h"
#include <stdio.h>

TBNode::TBNode()
{
	m_bValue = false;
	m_pOneOffspring = NULL;
	m_pZeroOffspring = NULL;
	m_pQuadNode = NULL;
}

TBNode::~TBNode()
{
	m_bValue = false;

	if (m_pOneOffspring != NULL) {
		delete m_pOneOffspring;
		m_pOneOffspring = NULL;
	}

	if (m_pZeroOffspring != NULL) {
		delete m_pZeroOffspring;
		m_pZeroOffspring = NULL;
	}

	if (m_pQuadNode != NULL) {
		delete m_pQuadNode;
		m_pQuadNode = NULL;
	}
}

ostream& TBNode::Save(ostream& oStream, int iSpace = 0)  
{
char	szBuffer[256] = {0};
char	*pSpace = NULL;

	pSpace = new char [iSpace + 1];
	memset(pSpace, 0, (iSpace + 1) * sizeof(char));
	for(int i = 0; i < iSpace; i++) {
		pSpace[i] = ' ';
	}

	oStream << pSpace << "<BNODE ";
	oStream << "Value = \"" << ((m_bValue == true)?("TRUE"):("FALSE")) << "\" ";
	sprintf(szBuffer, "%lx\0", this);
	oStream << "Address = \"" << szBuffer << "\" ";
	sprintf(szBuffer, "%lx\0", m_pOneOffspring);
	oStream << "OneOffspring = \"" << szBuffer << "\" ";
	sprintf(szBuffer, "%lx\0", m_pZeroOffspring);
	oStream << "ZeroOffspring = \"" << szBuffer << "\" ";
	sprintf(szBuffer, "%lx\0", m_pQuadNode);
	oStream << "QuadNode = \"" << szBuffer << "\" ";
	oStream << ">\n";
	
	oStream.flush();

	if (m_pOneOffspring != NULL ) {
		oStream << pSpace << "<ONE>\n";
		m_pOneOffspring -> Save(oStream, iSpace + 1);
		oStream << pSpace << "</ONE>\n";
	}

	oStream.flush();

	if (m_pZeroOffspring != NULL ) {
		oStream << pSpace << "<ZERO>\n";
		m_pZeroOffspring -> Save(oStream, iSpace + 1);
		oStream << pSpace << "</ZERO>\n";
	}

	oStream.flush();

	if (m_pQuadNode != NULL ) {
		oStream << pSpace << "<CHILD>\n";
		m_pQuadNode -> Save(oStream, iSpace + 1);
		oStream << pSpace << "</CHILD>\n";
	}

	oStream << pSpace << "</BNODE>\n";

	if (pSpace != NULL) {
		delete pSpace;
		pSpace = NULL;
	}

	oStream.flush();

	return oStream;
}
