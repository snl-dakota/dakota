/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description:  This file contains code related to data utilities that should
//-               be compiled, rather than inlined in data_util.h.
//-
//- Owner:        Mike Eldred
//- Version: $Id: data_util.C 7024 2010-10-16 01:24:42Z mseldre $

#include "data_util.h"
#ifdef DAKOTA_NEWMAT
#include "newmat.h"
#endif // DAKOTA_NEWMAT
#ifdef DAKOTA_DDACE
#include "DDaceSamplePoint.h"
#endif // DAKOTA_DDACE


namespace Dakota {

// ------------
// == operators
// ------------


bool operator==(const ShortArray& dsa1, const ShortArray& dsa2)
{
  // Check for equality in array lengths
  size_t len = dsa1.size();
  if ( dsa2.size() != len )
    return false;

  // Check each value
  size_t i;
  for (i=0; i<len; ++i)
    if ( dsa2[i] != dsa1[i] )
      return false;

  return true;
}


bool operator==(const StringArray& dsa1, const StringArray& dsa2)
{
  // Check for equality in array lengths
  size_t len = dsa1.size();
  if ( dsa2.size() != len )
    return false;

  // Check each string
  size_t i;
  for (i=0; i<len; ++i)
    if ( dsa2[i] != dsa1[i] )
      return false;

  return true;
}


bool operator==(const SizetArray& sa, SizetMultiArrayConstView smav)
{
  // Check for equality in array lengths
  size_t len = sa.size();
  if ( smav.size() != len )
    return false;

  // Check each size_t
  size_t i;
  for (i=0; i<len; ++i)
    if ( smav[i] != sa[i] )
      return false;

  return true;
}


// ----------------------------
// non-templated copy functions
// ----------------------------
// Note: retaining these functions in data_types.C instead of inlining them
// in data_*.h avoids collisions between NEWMAT and other headers.

#ifdef DAKOTA_NEWMAT

// copy RealSymMatrix to NEWMAT::SymmetricMatrix
void copy_data(const RealSymMatrix& rsdm, NEWMAT::SymmetricMatrix& sm)
{
  // SymmetricMatrix = symmetric and square, but Dakota::Matrix can be general
  // (e.g., functionGradients = numFns x numVars).  Therefore, have to verify
  // sanity of the copy.  Could copy square submatrix of rsdm into sm, but 
  // aborting with an error seems better since this should only currently be
  // used for copying Hessian matrices.
  size_t nr = rsdm.numRows(), nc = rsdm.numCols();
  if (nr != nc) {
    Cerr << "Error: copy_data(const Dakota::RealSymMatrix& rsdm, "
	 << "SymmetricMatrix& sm) called with nonsquare rsdm." << std::endl;
    abort_handler(-1);
  }
  if (sm.Nrows() != nr) // sm = symmetric & square -> only 1 dimension needed
    sm.ReSize(nr);
  for (size_t i=0; i<nr; i++)
    for (size_t j=0; j<nr; j++)
      sm(i+1,j+1) = rsdm(i,j);
}

// copy RealMatrix to NEWMAT::Matrix
void copy_data(const RealMatrix& rdm, NEWMAT::Matrix& m)
{
  // Newmat Matrix and Dakota::RealMatrix are general rectangular matrices.
  RealMatrix::ordinalType nr = rdm.numRows(), nc = rdm.numCols();
  if (m.Nrows() != nr || m.Ncols() != nc)
    m.ReSize(nr, nc);
  for (RealMatrix::ordinalType i=0; i<nr; ++i)
    for (RealMatrix::ordinalType j=0; j<nc; ++j)
      m(i+1,j+1) = rdm(i,j);
}

// copy NEWMAT::ColumnVector to RealVector
void copy_data(const NEWMAT::ColumnVector& cv, RealVector& rdv)
{
  int size_cv = cv.Nrows();
  if (rdv.length() != size_cv)
    rdv.sizeUninitialized(size_cv);
  for (size_t i=0; i<size_cv; i++)
    rdv[i] = cv(i+1);
}

// copy Real* (pointer to ColVec of Teuchos_SDMatrix) to NEWMAT::ColumnVector
void copy_data(const Real* rv, const int num_items, NEWMAT::ColumnVector& cv)
{
  if (cv.Nrows() != num_items)
    cv.ReSize(num_items);
  for (size_t i=0; i<num_items; ++i)
    cv(i+1) = rv[i];
}

// copy RealVector to NEWMAT::ColumnVector
void copy_data(const RealVector& rv, NEWMAT::ColumnVector& cv)
{
  int size_rv = rv.length();
  if (cv.Nrows() != size_rv)
    cv.ReSize(size_rv);
  for (size_t i=0; i<size_rv; ++i)
    cv(i+1) = rv[i];
}
#endif // DAKOTA_NEWMAT

#ifdef DAKOTA_DDACE
// copy DDACE point to RealVector
void copy_data(const DDaceSamplePoint& dsp, RealVector& rv)
{
  int j, vec_len = dsp.length();
  if (rv.length() != vec_len)
    rv.sizeUninitialized(vec_len);
  for (j=0; j<vec_len; ++j)
    rv[j] = dsp[j];
}

// copy DDACE point array to RealVectorArray
void copy_data(const std::vector<DDaceSamplePoint>& dspa,
	       RealVectorArray& rva)
{
  int i, num_vec = dspa.size();
  if (rva.size() != num_vec)
    rva.resize(num_vec);
  for (i=0; i<num_vec; ++i)
    copy_data(dspa[i], rva[i]);
}

// copy DDACE point array to Real*
void copy_data(const std::vector<DDaceSamplePoint>& dspa, Real* ptr,
	       const int ptr_len)
{
  int i, j, num_vec = dspa.size(), total_len = 0, cntr = 0;
  for (i=0; i<num_vec; ++i)
    total_len += dspa[i].length();
  if (total_len != ptr_len) {
    Cerr << "Error: pointer allocation (" << ptr_len << ") does not equal "
	 << "total Array<DDaceSamplePoint> length (" << total_len << ") in "
	 << "copy_data(Array<DDaceSamplePoint>, Real*)." << std::endl;
    abort_handler(-1);
  }
  for (i=0; i<num_vec; ++i) {
    int vec_len = dspa[i].length();
    for (j=0; j<vec_len; ++j)
      ptr[cntr++] = dspa[i][j];
  }
}
#endif // DAKOTA_DDACE

} // namespace Dakota
