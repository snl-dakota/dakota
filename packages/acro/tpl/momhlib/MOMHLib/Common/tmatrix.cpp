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

#ifndef __TMATRIX_CPP_
#define __TMATRIX_CPP_

#include "tmatrix.h"

template <class VClass>
TMatrix<VClass>::TMatrix(void)
{
	Initialize();
}

template <class VClass>
void TMatrix<VClass>::Initialize(void)
{
	m_pData = NULL;
	m_lMaxRow = 0;
	m_lMaxColumn = 0;
}

template <class VClass>
TMatrix<VClass>::TMatrix(long lColumns,long lRows)
{
	m_pData = NULL;
	m_lMaxRow = 0;
	m_lMaxColumn = 0;

	Create(lColumns, lRows);
}

template <class VClass>
TMatrix<VClass>::~TMatrix(void)
{
	Release();
}

template <class VClass>
bool TMatrix<VClass>::Create(long lColumns,long lRows)
{	
	if ((lColumns <= 0) || (lRows <= 0)) { 
		return(false);		
	}

	Release();

	m_pData = new VClass[lColumns*lRows];
	if (m_pData == NULL) {
		return(false);
	}
	memset(m_pData, 0, sizeof(VClass)*lColumns*lRows);
	
	m_lMaxColumn = lColumns;
	m_lMaxRow = lRows;

	return(true);
}

template <class VClass>
void TMatrix<VClass>::Release(void)
{
	if (m_pData != NULL) {
		delete m_pData;
		m_pData = NULL;
	}

	m_lMaxRow = 0;
	m_lMaxColumn = 0;
}

template <class VClass>
inline VClass TMatrix<VClass>::Get(long lColumn, long lRow)
{
	return(m_pData[lRow*m_lMaxColumn+lColumn]);
}

template <class VClass>
inline void TMatrix<VClass>::Put(long lColumn, long lRow, VClass value)
{
	m_pData[(lRow*m_lMaxColumn+lColumn)] = (VClass)value;
}

template <class VClass>
inline void TMatrix<VClass>::Set(VClass value)
{
int			i;
int			len = m_lMaxColumn * m_lMaxRow;

	for(i = 0; i < len; i++) {
		m_pData[i] = (VClass)value;
	}	
}


template <class VClass>
istream& TMatrix<VClass>::Load(istream& stream)
{
	return(stream);
}

template <class VClass>
ostream& TMatrix<VClass>::Save(ostream& stream)	
{	
	return(stream);
}

#endif
