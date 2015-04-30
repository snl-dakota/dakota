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

#ifndef _TMATRIX_H_
#define _TMATRIX_H_

#include <global.h>

/** This class is two dimensional template of matrix
 *
 * This class is two dimensional matrix with functions to data store and 
 * retrieve.
 **/
template <class VClass> class TMatrix 
{
protected:
    /** Internal data 
     **/
	VClass  *m_pData;

public:

    /** Number of rows in matrix. **/
	long	m_lMaxRow;

    /** Number of columns in matrix. **/
	long	m_lMaxColumn;

	/** Construct matrix object
	 **/
	TMatrix(void);

	/** Construct matrix object and allocate matrix
     * 
     * @param lColumns number of columns in matrix
     * @param lRows number of rows in matrix 
	 **/
	TMatrix(long lColumns,long lRows);

	/** Construct and initalize matrix object
	 **/
	void Initialize(void);

	/** Construct matrix object
	 **/
	virtual ~TMatrix(void);

	/** Create two dimensional martix for given dimensions
     *
     * Previous matrix is released.
     *
     * @param lColumns number of columns in matrix
     * @param lRows number of rows in matrix 
	 **/
	bool Create(long lColumns,long lRows);

	/** Release matrix
	 **/
	void Release(void);

	/** Get value from matrix
     *
     * @param lColumn selected column
     * @param lRow selected row
     * @return element of matrix from given coords
     **/
	inline VClass Get(long lColumn,long lRow);

	/** Put value to matrix
     *
     * @param lColumn selected column
     * @param lRow selected row
     * @param value element put to matrix for given coords
	 **/
	inline void Put(long lColumn, long lRow, VClass value);

	/** Set matrix fields to value
     *
     * 
	 **/
	inline void Set(VClass value);

	/** Virtual function dedicated for loading matrix from stream. 
     * 
     * Not implemented yet !
	 **/
	virtual istream& Load(istream& stream);

	/** Virtual function dedicated for saving matrix to stream. 
     * 
     * Save matrix to table where elements are separated using comma.
     *
     * @param stream output stream
     * @return output stream
	 **/
    virtual ostream& Save(ostream& stream);
};

#include "tmatrix.cpp"

#endif
