/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

/**
 * \file SparseMatrix.h
 *
 * Defines the utilib::SparseMatrix, utilib::RMSparseMatrix and
 * CMSparseMatrix classes
 */

#ifndef utilib_SparseMatrix_h
#define utilib_SparseMatrix_h

#include <utilib_config.h>
#include <utilib/BitArray.h>
#include <utilib/CharString.h>
#include <utilib/NumArray.h>
#include <utilib/Ereal.h>

namespace utilib {

//--- CONSTANTS ---//

//--- TYPES ---//

template <class T>
class CMSparseMatrix; // Forward decl

template <class T>
class RMSparseMatrix; // Forward decl


/**
 * Compute the inner product between a NumArray and RMSparseMatrix.
 *
 * \note This is probably not numerically stable.
 */
template <class T>
inline void product(NumArray<T>& res, const RMSparseMatrix<T>& mat, 
						const NumArray<T>& vec)
{
res.resize(mat.get_nrows());

int nrows=mat.get_nrows();
for (int i=0; i<nrows; i++) {
  res[i] = 0.0;
  const T* val = &(mat.matval[mat.matbeg[i]]);
  const int*    col = &(mat.matind[mat.matbeg[i]]);
  for (int k=0; k<mat.matcnt[i]; k++, val++, col++) {
    res[i] += vec[*col] * (*val);
    }
  }
}


//---------------------------------------------------------------------------

/**
 * A kernel for sparse matrix classes.
 * Adapted from the sparse matrix classes developed by Kalyan S. Perumalla 
 * and Jeff T. Linderoth.
 *
 * Note: this code uses integer indeces.  This limits the total size of the
 * sparse matrix to a smaller size than would be possible if 'long integers'
 * were used for indexing, but this is compatable with CPLEX's use of 
 * sparse arrays.
 */
template <class T>
class SparseMatrix : public PackObject
{
public:

  ///
  void write(std::ostream& ostr) const = 0;

  ///
  void read(std::istream& istr) = 0;

  ///
  void write(PackBuffer& ostr) const;

  ///
  void read(UnPackBuffer& istr);

  /// Returns the number of columns in the matrix.
  int get_ncols( void ) const { return ncols; }

  /// Returns the number of rows in the matrix.
  int get_nrows( void ) const { return nrows; }

  /// Returns the number of non-zeros in the matrix.
  int get_nnzero( void ) const {return nnzeros;}

  /// Returns the \c matbeg array.
  NumArray<int>& get_matbeg( void ) { return matbeg; }

  /// Returns the \c matcnt array.
  NumArray<int>& get_matcnt( void ) { return matcnt; }

  /// Returns the \c matind array.
  NumArray<int>& get_matind( void ) { return matind; }

  /// Returns the \c matval array.
  NumArray<T>& get_matval( void ) { return matval; }

  /// Returns the \c matbeg array.
  const NumArray<int>& get_matbeg( void ) const { return matbeg; }

  /// Returns the \c matcnt array.
  const NumArray<int>& get_matcnt( void ) const { return matcnt; }

  /// Returns the \c matind array.
  const NumArray<int>& get_matind( void ) const { return matind; }

  /// Returns the \c matval array.
  const NumArray<T>& get_matval( void ) const { return matval; }

  /// Returns the value of the element at the \a row-th row and \a col-th column
  virtual T& operator()(const int row, const int col) = 0;

  /// Returns the value of the element at the \a row-th row and \a col-th column
  virtual const T& operator()(const int row, const int col) const = 0;

  /// Print out the matrix in a pretty format
  virtual int pretty_print(std::ostream& ostr) const;

  /// Delete the \a i-th row.
  virtual void delete_row(const int i) = 0;

  /// Delete the \a i-th column.
  virtual void delete_col(const int i) = 0;

  /// Delete the \a (row,col)-th element.
  virtual void delete_element(const int row, const int col) = 0;

protected:

  /// A protected constructor, which initializes elements in this class.
  SparseMatrix( void ) :
	ncols(0), nrows(0), nnzeros(0) 
   { (void)registrations_complete; }

  /// Dummy destructor
  virtual ~SparseMatrix() {}

  /// Initialize a sparse matrix
  virtual void initialize(int nrow, int ncol, int nnzero) = 0;

  /// Initialize the memory of a sparse matrix
  void setup(int ncol, int nrow, int nnzero, int flag);

  /// Make a call to setup with appropriate flag
  virtual void addSpace(int nrow, int ncol, int nnzero) = 0;

  /// The number of columns in the matrix
  int          ncols;

  /// The number of rows in the matrix
  int          nrows;

  /// The number of nonzeros in the matrix
  int	       nnzeros;

  /// Index where the \a i-th row/column begins in matval.
  NumArray<int>    matbeg;

  /// The number of values in the \a i-th row/column
  NumArray<int>    matcnt;

  /// matind[i] is the column/row number for the \a i-th value in row/column
  NumArray<int>    matind;

  /// The vector of values in the matrix
  NumArray<T> matval;

private:

   static const volatile bool registrations_complete;

   template<typename FROM, typename TO>
   static int convert_cast(const Any& from, Any& to)
   {
      to.set<TO>().convert(from.template expose<FROM>());
      return OK;
   }

   //static int cast_to_rm_from_dense(const Any& from, Any& to);
   static int cast_to_cm_from_BasicArrayArray(const Any& from, Any& to);
   //static int cast_from_rm_to_dense(const Any& from, Any& to);
   static int cast_from_cm_to_BasicArrayArray(const Any& from, Any& to);

   static bool register_aux_functions();
};


//============================================================================

template<typename T>
const volatile bool SparseMatrix<T>::registrations_complete = 
   SparseMatrix<T>::register_aux_functions();

//---------------------------------------------------------------------------

/**
 * A column-major sparse matrix class.
 * Adapted from the CMatrix class of Kalyan S. Perumalla and Jeff T. Linderoth.
 */
template <class T>
class 
CMSparseMatrix : public SparseMatrix<T>
{
public:

  #if !defined(DOXYGEN)
  typedef SparseMatrix<T> base_t;
  #if !defined(UTILIB_SOLARIS_CC)
  using base_t::nrows;
  using base_t::ncols;
  using base_t::nnzeros;
  using base_t::matind;
  using base_t::matval;
  using base_t::matbeg;
  using base_t::matcnt;
  #endif

  #if defined(UTILIB_SOLARIS_CC)
  friend RMSparseMatrix<T>;
  #else
  template <class U> friend class RMSparseMatrix;
  #endif
  #endif

  /// Empty constructor.
  CMSparseMatrix( void ) {}

  /// Construct the matrix and resize the internal data.
  CMSparseMatrix( int nrow, int ncol, int nnzero )
		{initialize(nrow,ncol,nnzero);}

  /// Copy constructor
  CMSparseMatrix( CMSparseMatrix<Ereal<T> >& mat);
 
  /// Dummy virtual destructor
  ~CMSparseMatrix() {}

  /// Setup a SparseMatrix that will be filled in later.
  void initialize(int nrow, int ncol, int nnzero)
		{
		this->setup(nrow,ncol,nnzero,1);
		ncols=ncol;
		nrows=nrow;
		nnzeros=nnzero;
		}

  /// Add some space to a SparseMatrix.  Don't update the size values
  /// so the matrix can grow into it.

  void addSpace(int nrow, int ncol, int nnzero)
		{
		this->setup(nrow,ncol,nnzero,1);
		};

  
  /// Returns the value of the element at the \a row-th row and \a col-th column
  T& operator()(const int row, const int col);

  /// Returns the value of the element at the \a row-th row and \a col-th column
  const T& operator()(const int row, const int col) const;

  /// compare the two matrices (<0 for "<", 0 for "=", >0 for ">")
  int compare(const CMSparseMatrix &rhs) const;

  /// Returns true if the two matricies are equal
  bool operator==(const CMSparseMatrix &rhs) const
     { return compare(rhs) == 0; }

  /// Returns true if this matrix is "less than" rhs (for STL support)
  bool operator<(const CMSparseMatrix &rhs) const
     { return compare(rhs) < 0; }

  /// Add a column to the matrix.
  int adjoinColumn(int count, int* rowPosition, T* value);

  ///
  void write(std::ostream& ostr) const;

  ///
  void read(std::istream& istr);

  ///
  void write(PackBuffer& ostr) const
		{ SparseMatrix<T>::write(ostr); }

  ///
  void read(UnPackBuffer& istr)
		{ SparseMatrix<T>::read(istr); }

  /// Convert a row-major matrix to a column-major matrix.
  void convert(const RMSparseMatrix<T>& rowmajor);

  /// Delete \a count consecutive columns beginning with the \a i-th.
  void delete_cols(const size_t i, const size_t count);

  /// Delete the \a i-th row.
  void delete_row(const int i);

  /// Delete the \a i-th column.
  void delete_col(const int i);

  /// Delete the \a (row,col)-th element.
  void delete_element(const int row, const int col);

};



//---------------------------------------------------------------------------

/**
 * A row-major sparse matrix class.
 * Adapted from the RMatrix class of Kalyan S. Perumalla and Jeff T. Linderoth.
 */
template <class T>
class 
RMSparseMatrix : public SparseMatrix<T>
{
public:

  #if !defined(DOXYGEN)
  typedef SparseMatrix<T> base_t;
  #if !defined(UTILIB_SOLARIS_CC)
  using base_t::nrows;
  using base_t::ncols;
  using base_t::nnzeros;
  using base_t::matind;
  using base_t::matval;
  using base_t::matbeg;
  using base_t::matcnt;
  #endif

  #if defined(UTILIB_SOLARIS_CC)
  friend CMSparseMatrix<T>;
  #else
  template <class U> friend class CMSparseMatrix;
  #endif

  #if !defined(UTILIB_NO_MEMBER_TEMPLATE_FRIENDS)
  friend void utilib::product<T>(NumArray<T>& res, 
				const RMSparseMatrix<T>& mat, 
				const NumArray<T>& vec);
  #else
  friend void product<T>(NumArray<T>& res, 
				const RMSparseMatrix<T>& mat, 
				const NumArray<T>& vec);
  #endif
  #endif

  /// Empty constructor.
  RMSparseMatrix( void ) {}

  /// Construct the matrix and resize the internal data.
  RMSparseMatrix( int nrow, int ncol, int nnzero )
		{initialize(nrow,ncol,nnzero);}

  /// Dummy virtual destructor
  virtual ~RMSparseMatrix() {}

  /// Setup a SparseMatrix that will be filled in later.
  void initialize(int nrow, int ncol, int nnzero)
		{
		this->setup(nrow,ncol,nnzero,0);
		ncols=ncol;
		nrows=nrow;
		nnzeros=nnzero;
		}

  /// Add some space to a SparseMatrix.  Don't update the size values
  /// so the matrix can grow into it.

  void addSpace(int nrow, int ncol, int nnzero)
		{
		this->setup(nrow,ncol,nnzero,0);
		};

  /// Resize the \a rowndx-th row to length \a rowlen.
  void resize(const int rowndx, const int rowlen);

  /// Returns the value of the element at the \a row-th row and \a col-th column
  T& operator()(const int row, const int col);

  /// Returns the value of the element at the \a row-th row and \a col-th column
  const T& operator()(const int row, const int col) const;

  /// compare the two matrices (<0 for "<", 0 for "=", >0 for ">")
  int compare(const RMSparseMatrix &rhs) const;

  /// Returns true if the two matricies are equal
  bool operator==(const RMSparseMatrix &rhs) const
     { return compare(rhs) == 0; }

  /// Returns true if this matrix is "less than" rhs (for STL support)
  bool operator<(const RMSparseMatrix &rhs) const
     { return compare(rhs) < 0; }

  /// Add a row to the matrix.
  int adjoinRow(int count, int* colPosition, T* value);

  /// Add a (dense) row to the matrix.
  int adjoinRow(int count, T* value);

  /// Add a RMSparse matrix to the bottom of this matrix
  void adjoinRows(const RMSparseMatrix &rhs);

  ///
  void write(std::ostream& ostr) const;

  ///
  void read(std::istream& istr);

  ///
  void write(PackBuffer& ostr) const
		{ SparseMatrix<T>::write(ostr); }

  ///
  void read(UnPackBuffer& istr)
		{ SparseMatrix<T>::read(istr); }

  /// Convert a column-major matrix to a row-major matrix.
  void convert(const CMSparseMatrix<T>& colmajor);

  /// Delete \a count consecutive rows beginning with the \a i-th.
  void delete_rows(const size_t i, const size_t count);

  /// Delete the \a i-th row.
  void delete_row(const int i);

  /// Delete the \a i-th column.
  void delete_col(const int i);

  /// Delete the \a (row,col)-th element.
  void delete_element(const int row, const int col);
};




//
// This just sets up the memory, but does not modify the ncols, nrows or
// nnzeros values
//
template <class T>
void SparseMatrix<T>::setup(int nrow, int ncol, int nnzero, int flag)
{
matind.resize(nnzero);
matval.resize(nnzero);
if (flag) {
   matbeg.resize(ncol);
   matcnt.resize(ncol);
   }
else {
   matbeg.resize(nrow);
   matcnt.resize(nrow);
   }
}


template <class T>
CMSparseMatrix<T>::CMSparseMatrix( CMSparseMatrix<Ereal<T> >& mat)
{
initialize(mat.get_nrows(), mat.get_ncols(), mat.get_nnzero());
matind << mat.get_matind();
matval << mat.get_matval();
matbeg << mat.get_matbeg();
matcnt << mat.get_matcnt();
}


template <class T>
T& CMSparseMatrix<T>::operator()(const int row, const int col)
{
if ((row < 0) || (row >= nrows) || (col < 0) || (col >= ncols))
   EXCEPTION_MNGR(runtime_error, "CMSparseMatrix<T>::operator : iterator out of range. " << row << "x" << col << " not in " << nrows << "x" << ncols);

int pt = matbeg[col];
for (int i=0; i<matcnt[col]; i++)
  if (matind[pt+i] == row)
     return matval[pt+i];

return matval[0];
}


template <class T>
const T& CMSparseMatrix<T>::operator()(const int row, const int col) const
{
if ((row < 0) || (row >= nrows) || (col < 0) || (col >= ncols))
   EXCEPTION_MNGR(runtime_error, "CMSparseMatrix<T>::operator : iterator out of range. " << row << "x" << col << " not in " << nrows << "x" << ncols);

int pt = matbeg[col];
for (int i=0; i<matcnt[col]; i++)
  if (matind[pt+i] == row)
     return matval[pt+i];

return matval[0];
}


template <class T>
T& RMSparseMatrix<T>::operator()(const int row, const int col)
{
if ((row < 0) || (row >= nrows) || (col < 0) || (col >= ncols))
   EXCEPTION_MNGR(runtime_error, "RMSparseMatrix<T>::operator : iterator out of range. " << row << "x" << col << " not in " << nrows << "x" << ncols);

int pt = matbeg[row];
for (int i=0; i<matcnt[row]; i++)
  if (matind[pt+i] == col)
     return matval[pt+i];

return matval[0];
}


template <class T>
const T& RMSparseMatrix<T>::operator()(const int row, const int col) const
{
if ((row < 0) || (row >= nrows) || (col < 0) || (col >= ncols))
   EXCEPTION_MNGR(runtime_error, "RMSparseMatrix<T>::operator : iterator out of range. " << row << "x" << col << " not in " << nrows << "x" << ncols);

int pt = matbeg[row];
for (int i=0; i<matcnt[row]; i++)
  if (matind[pt+i] == col)
     return matval[pt+i];

return matval[0];
}


template <class T>
int CMSparseMatrix<T>::compare(const CMSparseMatrix &rhs) const
{
   if ( ncols != rhs.ncols )
      return ncols < rhs.ncols ? -1 : 1;
   if ( nrows != rhs.nrows )
      return nrows < rhs.nrows ? -1 : 1;
   if ( nnzeros != rhs.nnzeros )
      return nnzeros < rhs.nnzeros ? -1 : 1;

   // now we have to test element-by-element
   int major = -1;
   int numMajor = 0;
   int rhs_major = -1;
   int rhs_numMajor = 0;
   for (size_t i=0; i < nnzeros; ++i)
   {
      while ( numMajor <= 0 )
      {
         ++major;
         numMajor = matcnt[major];
      }
      while ( rhs_numMajor <= 0 )
      {
         ++rhs_major;
         rhs_numMajor = rhs.matcnt[rhs_major];
      }
      // note the logic reversal... the lower major number indicates an
      // earlier non-zero, which makes the matrix actually *greater*
      if ( major != rhs_major )
         return major < rhs_major ? 1 : -1;
      if ( matind[i] != rhs.matind[i] )
         return matind[i] < rhs.matind[i] ? 1 : -1;
      // comparing values at the same index, logic returns to normal
      if ( matval[i] != rhs.matval[i] )
         return matval[i] < rhs.matval[i] ? -1 : 1;
      --numMajor;
      --rhs_numMajor;
   }
   return 0;
}

template <class T>
int RMSparseMatrix<T>::compare(const RMSparseMatrix &rhs) const
{
   if ( nrows != rhs.nrows )
      return nrows < rhs.nrows ? -1 : 1;
   if ( ncols != rhs.ncols )
      return ncols < rhs.ncols ? -1 : 1;
   if ( nnzeros != rhs.nnzeros )
      return nnzeros < rhs.nnzeros ? -1 : 1;

   // now we have to test element-by-element
   int major = -1;
   int numMajor = 0;
   int rhs_major = -1;
   int rhs_numMajor = 0;
   for (size_t i=0; i < nnzeros; ++i)
   {
      while ( numMajor <= 0 )
      {
         ++major;
         numMajor = matcnt[major];
      }
      while ( rhs_numMajor <= 0 )
      {
         ++rhs_major;
         rhs_numMajor = rhs.matcnt[rhs_major];
      }
      // note the logic reversal... the lower major number indicates an
      // earlier non-zero, which makes the matrix actually *greater*
      if ( major != rhs_major )
         return major < rhs_major ? 1 : -1;
      if ( matind[i] != rhs.matind[i] )
         return matind[i] < rhs.matind[i] ? 1 : -1;
      // comparing values at the same index, logic returns to normal
      if ( matval[i] != rhs.matval[i] )
         return matval[i] < rhs.matval[i] ? -1 : 1;
      --numMajor;
      --rhs_numMajor;
   }
   return 0;
}

template <class T>
int SparseMatrix<T>::pretty_print(std::ostream& ostr) const
{
for (int i=0; i<nrows; i++) {
  for (int j=0; j<ncols; j++)
    ostr << operator()(i,j) << " ";
  ostr << std::endl;
  }
return OK;
}


template <class T>
void CMSparseMatrix<T>::write(std::ostream& ostr) const 
{
ostr << nrows << " " << ncols << " " << nnzeros << ":" << std::endl;
int ndx=0;
for (int i=0; i<ncols; i++) {
  //ostr << "[" << matbeg[i] << ", " << matcnt[i] << "]" << std::endl;
  for (int j=0; j<matcnt[i]; j++) {
    ostr << " (" << matind[ndx] << "," << i << ") " << matval[ndx] << std::endl;
    ndx++;
    }
  }
}


template <class T>
void CMSparseMatrix<T>::read(std::istream& istr)
{
   static_cast<void>(istr);
   EXCEPTION_MNGR(runtime_error, "CMSparseMatrix<T>::read - not implemented!");
}


template <class T>
void SparseMatrix<T>::write(PackBuffer& ostr) const 
{
ostr << nrows << ncols << nnzeros;
ostr << matbeg << matcnt << matind << matval;
}


template <class T>
void SparseMatrix<T>::read(UnPackBuffer& istr)
{
int ntmp, ctmp, ztmp;
istr >> ntmp >> ctmp >> ztmp;
initialize(ntmp,ctmp,ztmp);
istr >> matbeg >> matcnt >> matind >> matval;
}


template <class T>
void RMSparseMatrix<T>::write(std::ostream& ostr) const 
{
ostr << nrows << " " << ncols << " " << nnzeros << ":" << std::endl;
int ndx=0;
for (int i=0; i<nrows; i++) {
  //ostr << "[" << matbeg[i] << ", " << matcnt[i] << "]" << std::endl;
  for (int j=0; j<matcnt[i]; j++) {
    ostr << " (" << i << "," << matind[ndx] << ") " << matval[ndx] << std::endl;
    ndx++;
    }
  }
}


template <class T>
void RMSparseMatrix<T>::read(std::istream& istr)
{
   static_cast<void>(istr);
   EXCEPTION_MNGR(runtime_error, "RMSparseMatrix<T>::read - not implemented!");
}



template <class T>
void RMSparseMatrix<T>::delete_rows(const size_t i, const size_t count)
{
   if ( i + count > static_cast<size_t>(nrows) )
      EXCEPTION_MNGR(std::runtime_error, "RMSparseMatrix<T>::delete_rows(): "
                     "row range (" << i << " to " << (i+count) << 
                     ") extends past end of matrix (" << nrows << ")");

   //
   // Delete all of the elements in the rows
   //
   size_t offset = 0;
   for(size_t j = 0; j<count; ++j)
      offset += matcnt[i+j];

   // shift the remaining data
   if ( offset )
   {
      nnzeros -= offset;
      for (size_t j=matbeg[i]; j<static_cast<size_t>(nnzeros); j++) {
         matind[j] = matind[j+offset];
         matval[j] = matval[j+offset];
      }
   }
 
   //
   // Delete the row itself
   //
   nrows -= count;
   for (size_t j=i; j<static_cast<size_t>(nrows); j++) {
      matbeg[j] = matbeg[j+count] - offset;
      matcnt[j] = matcnt[j+count];
   }
}


template <class T>
void RMSparseMatrix<T>::delete_row(const int i)
{
   delete_rows(i, 1);
#if 0
//
// Delete all of the elements in the row
//
while (matcnt[i] > 0)
  delete_element(i,matind[matbeg[i]]);
//
// Delete the row itself
//
for (int j=i; j<(nrows-1); j++) {
  matbeg[j] = matbeg[j+1];
  matcnt[j] = matcnt[j+1];
  }
nrows--;
#endif
}


template <class T>
void RMSparseMatrix<T>::delete_col(const int i)
{
//
// Delete all of the elements in the column
//
for (int j=0; j<nrows; j++) {
  //
  // Look for column i in row j
  //
  int k=0;
  while (k < matcnt[j]) {
    if (matind[matbeg[j]+k] == i)
       delete_element(j,i);
    else {
       if (matind[matbeg[j]+k] > i)
          matind[matbeg[j]+k]--;
       k++;
       }
    }
  }
//
// Delete the column itself (the column indeces are decremented above!)
//
ncols--;
}


template <class T>
void RMSparseMatrix<T>::delete_element(const int rowndx, const int colndx)
{
int k=0;
for (; k<matcnt[rowndx]; k++)
  if (matind[matbeg[rowndx]+k] == colndx) {
     break;
     }
if (k == matcnt[rowndx]) return;

for (int i=matbeg[rowndx]+k; i<(nnzeros-1); i++) {
  matind[i] = matind[i+1];
  matval[i] = matval[i+i];
  }
matcnt[rowndx]--;
for (int j=rowndx+1; j<nrows; j++)
  matbeg[j]--;
nnzeros--;
}


template <class T>
void RMSparseMatrix<T>::resize(const int rowndx, const int rowlen)
{
if (rowlen == matcnt[rowndx])
   return;
else if (rowlen > matcnt[rowndx]) {
   // TODO - resize the array up!
   }
else {
   int diff = rowlen-matcnt[rowndx];

   if (diff < 0) {
      for (int i=matbeg[rowndx]+rowlen; i<(nnzeros+diff); i++) {
        matind[i] = matind[i-diff];
        matval[i] = matval[i-diff];
        }
      }
   matcnt[rowndx] += diff;
   for (int j=rowndx+1; j<nrows; j++)
     matbeg[j] += diff;
   nnzeros += diff;
   }
}


template <class T>
void CMSparseMatrix<T>::convert(const RMSparseMatrix<T>& rowmajor)
{
matval.resize(rowmajor.nnzeros);
matind.resize(rowmajor.nnzeros);

ncols = rowmajor.ncols;
nrows = rowmajor.nrows;
nnzeros = rowmajor.nnzeros;
matcnt.resize(ncols);
matcnt << 0;
matbeg.resize(ncols);

NumArray<int> rowndx(nrows);
rowndx << 0;
int prev_cndx=0;

for (int i=0; i<nnzeros; i++) {
  //
  // Set dummy values for cndx and rndx
  //
  int cndx=ncols;
  int rndx=-1;
  //
  // Find the point in the matrix with the next highest column index
  //
  for (int j=0; j<nrows; j++) {
    if ((rowmajor.matcnt[j] > 0) && (rowmajor.matcnt[j] > rowndx[j])) {
       if (rowmajor.matind[rowmajor.matbeg[j]+rowndx[j]] < cndx) {
          cndx = rowmajor.matind[rowmajor.matbeg[j]+rowndx[j]];
          rndx = j;
          }
       }
    if (cndx == prev_cndx)	// Can stop early because we're in the same col
       break;
    }
  //
  // Add this new point to the CM matrix
  //
  if (matcnt[cndx] == 0)
     matbeg[cndx] = i;
  matcnt[cndx]++;
  matind[i] = rndx;
  matval[i] = rowmajor.matval[rowmajor.matbeg[rndx]+rowndx[rndx]];

  rowndx[rndx]++;
  prev_cndx=cndx;
  }
}


template <class T>
void RMSparseMatrix<T>::convert(const CMSparseMatrix<T>& colmajor)
{
matval.resize(colmajor.nnzeros);
matind.resize(colmajor.nnzeros);

ncols = colmajor.ncols;
nrows = colmajor.nrows;
nnzeros = colmajor.nnzeros;
matcnt.resize(nrows);
matcnt << 0;
matbeg.resize(nrows);

NumArray<int> colndx(ncols);
colndx << 0;
int prev_rndx=0;

for (int i=0; i<nnzeros; i++) {
  //
  // Find the point in the matrix with the next highest row index
  //
  int cndx=-1;
  int rndx=nrows;
  for (int j=0; j<ncols; j++) {
    if ((colmajor.matcnt[j] > 0) && (colmajor.matcnt[j] > colndx[j])) {
       if (colmajor.matind[colmajor.matbeg[j]+colndx[j]] < rndx) {
          cndx = j;
          rndx = colmajor.matind[colmajor.matbeg[j]+colndx[j]];
          }
       }
    if (prev_rndx == rndx)	// Can stop because we are on the same row
       break;
    }
  
  //
  // Add this new point to the RM matrix
  //
  if (matcnt[rndx] == 0)
     matbeg[rndx] = i;
  matcnt[rndx]++;
  matind[i] = cndx;
  matval[i] = colmajor.matval[colmajor.matbeg[cndx]+colndx[cndx]];

  colndx[cndx]++;
  prev_rndx=rndx;
  }
}



template <class T>
int CMSparseMatrix<T>::adjoinColumn(int count, int* rowPosition, T* value)
{
if (ncols == (int) matcnt.size()) {
   matcnt.resize(ncols+32);
   matbeg.resize(ncols+32);
   }

if (nnzeros + count > (int) matval.size()) {
   matval.resize(nnzeros+std::max(count,64));
   matind.resize(nnzeros+std::max(count,64));
   }

matbeg[ncols] = nnzeros;
matcnt[ncols++] = count;
for (int i=0; i<count; i++) {
  matind[nnzeros]   = rowPosition[i];
  // If we're building from scratch, number of rows is the highest row
  // number seen so far (+1 since 0 based)
  if (rowPosition[i] >= nrows)
    nrows = rowPosition[i] + 1;
  matval[nnzeros++] = value[i];
  }

return nnzeros;
}


template <class T>
void CMSparseMatrix<T>::delete_cols(const size_t i, const size_t count)
{
   if ( i + count > static_cast<size_t>(ncols) )
      EXCEPTION_MNGR(std::runtime_error, "CMSparseMatrix<T>::delete_cols(): "
                     "column range extends past end of matrix");

   //
   // Delete all of the elements in the cols
   //
   size_t offset = 0;
   for(size_t j = 0; j<count; ++j)
      offset += matcnt[i+j];

   // shift the remaining data
   if ( offset )
   {
      nnzeros -= offset;
      for (size_t j=matbeg[i]; j<static_cast<size_t>(nnzeros); j++) {
         matind[j] = matind[j+offset];
         matval[j] = matval[j+offset];
      }
   }
 
   //
   // Delete the col itself
   //
   ncols -= count;
   for (size_t j=i; j<static_cast<size_t>(ncols); j++) {
      matbeg[j] = matbeg[j+count] - offset;
      matcnt[j] = matcnt[j+count];
   }
}


template <class T>
void CMSparseMatrix<T>::delete_col(const int i)
{
   delete_cols(i, 1);
#if 0
//
// Delete all of the elements in the col 
//
while (matcnt[i])
  delete_element(matind[matbeg[i]],i);
//
// Delete the col itself
//
for (int j=i; j<(ncols-1); j++) {
  matbeg[j] = matbeg[j+1];
  matcnt[j] = matcnt[j+1];
  }
ncols--;
#endif
}


template <class T>
void CMSparseMatrix<T>::delete_row(const int i)
{
//
// Delete all of the elements in the column
//
for (int j=0; j<ncols; j++) {
  //
  // Look for row i in row j
  //
  int k=0;
  while (k < matcnt[j]) {
    if (matind[matbeg[j]+k] == i)
       delete_element(i,j);
    else {
       if (matind[matbeg[j]+k] > i)
          matind[matbeg[j]+k]--;
       k++;
       }
    }
  }
//
// Delete the row itself (the row indeces are decremented above!)
//
nrows--;
}


template <class T>
void CMSparseMatrix<T>::delete_element(const int rowndx, const int colndx)
{
int k=0;
for (; k<matcnt[colndx]; k++)
  if (matind[matbeg[colndx]+k] == rowndx) {
     break;
     }
if (k == matcnt[colndx]) return;

for (int i=matbeg[colndx]+k; i<(nnzeros-1); i++) {
  matind[i] = matind[i+1];
  matval[i] = matval[i+1];
  }
matcnt[colndx]--;
for (int j=colndx+1; j<ncols; j++)
  matbeg[j]--;
nnzeros--;
}



template <class T>
int RMSparseMatrix<T>::adjoinRow(int count, int* colPosition, T* value)
{
if (nrows == (int) matcnt.size()) {
   matcnt.resize(nrows+32);
   matbeg.resize(nrows+32);
   }

if (nnzeros + count > (int) matval.size()) {
   matval.resize(nnzeros+std::max(count,64));
   matind.resize(nnzeros+std::max(count,64));
   }

matbeg[nrows] = nnzeros;
matcnt[nrows++] = count;
for (int i=0; i<count; i++) {
  matind[nnzeros]   = colPosition[i];
  // If we're building from scratch, number of rows is the highest row
  // number seen so far (+1 since 0 based)
  if (colPosition[i] >= ncols)
    ncols = colPosition[i] + 1;
  matval[nnzeros++] = value[i];
  }

return nnzeros;
}


template <class T>
int RMSparseMatrix<T>::adjoinRow(int raw_count, T* value)
{
if (nrows == (int) matcnt.size()) {
   matcnt.resize(nrows+32);
   matbeg.resize(nrows+32);
   }

if (nnzeros + raw_count > (int) matval.size()) {
   matval.resize(nnzeros+std::max(raw_count,64));
   matind.resize(nnzeros+std::max(raw_count,64));
   }

matbeg[nrows] = nnzeros;

int count = raw_count;
for( int i=0; i<raw_count; ++i )
   if ( fabs(value[i]) > std::numeric_limits<double>::epsilon() )
   {
      matind[nnzeros] = i;
      matval[nnzeros++] = value[i];
   }
   else
      --count;

matcnt[nrows++] = count;

if (raw_count > ncols)
   ncols = raw_count;

return nnzeros;
}


template <class T>
void RMSparseMatrix<T>::adjoinRows(const RMSparseMatrix &rhs)
{
   size_t newNRows = nrows + rhs.nrows;
   if ( newNRows > matcnt.size() )
   {
      matcnt.resize(newNRows);
      matbeg.resize(newNRows);
   }

   size_t newNNZeros = nnzeros + rhs.nnzeros;
   if ( newNNZeros > matval.size() )
   {
      matval.resize(newNNZeros);
      matind.resize(newNNZeros);
   }

   size_t i = rhs.nrows;
   while ( i > 0 )
   {
      --i;
      matcnt[nrows + i] = rhs.matcnt[i];
      matbeg[nrows + i] = nnzeros + rhs.matbeg[i];
   }

   i = rhs.nnzeros;
   while ( i > 0 )
   {
      --i;
      matval[nnzeros + i] = rhs.matval[i];
      matind[nnzeros + i] = rhs.matind[i];
   }

   if ( rhs.ncols > ncols )
      ncols = rhs.ncols;
   nrows = newNRows;
   nnzeros = newNNZeros;
}


namespace {

template<typename DENSE, typename T>
int cast_to_rm_from_dense(const Any& from_, Any& to_)
{
   const DENSE& from = from_.template expose<DENSE>();
   utilib::RMSparseMatrix<T>& to 
      = to_.set<utilib::RMSparseMatrix<T> > ();

   if (from.size() == 0)
      return OK;

   T zero = T();
   size_t nr = from.size();
   size_t nc = nr ? from[0].size() : 0;
   size_t nnz = 0;
   for(size_t i = 0; i < nr; ++i)
   {
      if ( nc < from[i].size() )
         nc = from[i].size();
      for( size_t j=0; j < from[i].size(); ++j )
         if ( from[i][j] != zero )
            ++nnz;
   }

   to.initialize(nr, nc, nnz);

   NumArray<int>& matbeg = to.get_matbeg();
   NumArray<int>& matcnt = to.get_matcnt();
   NumArray<int>& matind = to.get_matind();
   NumArray<T>&   matval = to.get_matval();

   size_t ctr = 0;
   for (size_t i = 0; i < nr; i++)
   {
      matbeg[i] = -1;
      matcnt[i] = 0;
      for (size_t j = 0; j < from[i].size(); j++)
         if (from[i][j] != zero)
         {
            if (matbeg[i] == -1)
               matbeg[i] = ctr;
            matcnt[i] += 1;
            matind[ctr] = j;
            matval[ctr++] = from[i][j];
         }
   }
   return OK;
}


template<typename DENSE, typename T>
int cast_from_rm_to_dense(const Any& from_, Any& to_)
{
   DENSE & ans = to_.set<DENSE>();
   const RMSparseMatrix<T>& from 
     = from_.template expose<RMSparseMatrix<T> > ();

   T zero = T();
   size_t nc = from.get_ncols();
   size_t nr = from.get_nrows();

   ans.resize(nr);
   for (size_t i = 0; i < nr; i++)
   {
      ans[i].resize(nc);
      for(size_t j = 0; j < nc; ans[i][j++] = zero);
   }

   if (from.get_nnzero() > 0)
   {
      for (size_t i = 0; i < nr; i++)
      {
         size_t k = from.get_matcnt()[i];
         if ( ! k )
            continue;

         const T*   val = &(from.get_matval()[from.get_matbeg()[i]]);
         const int* col = &(from.get_matind()[from.get_matbeg()[i]]);
         for( size_t j = 0; j < k; ++j )
            ans[i][col[j]] = val[j];
      }
   }

   return OK;
}

} // namespace utilib::(local)


template<typename T>
int SparseMatrix<T>::cast_to_cm_from_BasicArrayArray(const Any& _from, Any& to)
{
   CMSparseMatrix<T>& ans = to.set<CMSparseMatrix<T> > ();
   const BasicArray<BasicArray<T> > & from
     = _from.template expose<BasicArray<BasicArray<T> > > ();

   if (from.size() > 0)
   {
      NumArray<int> matbeg(from[0].size());
      NumArray<int> matcnt(from[0].size());
      NumArray<int> matind(from.size()*from[0].size());
      NumArray<T> matval(from.size()*from[0].size());

      int ctr = 0;
      for (size_t j = 0; j < from[0].size(); j++)
      {
         matbeg[j] = -1;
         matcnt[j] = 0;
         for (size_t i = 0; i < from.size(); i++)
            if (from[i][j] != 0.0)
            {
               if (matbeg[j] == -1)
                  matbeg[j] = ctr;
               matcnt[j] += 1;
               matind[ctr] = i;
               matval[ctr++] = from[i][j];
            }
      }
      matind.resize(ctr);
      matval.resize(ctr);

      ans.initialize(from.size(), from[0].size(), ctr);
      ans.get_matbeg() << matbeg;
      ans.get_matcnt() << matcnt;
      ans.get_matind() << matind;
      ans.get_matval() << matval;

      //std::cerr << "matbeg " << matbeg << std::endl;
      //std::cerr << "matcnt " << matcnt << std::endl;
      //std::cerr << "matind " << matind << std::endl;
      //std::cerr << "matval " << matval << std::endl;
   }

   return OK;
}

template<typename T>
int SparseMatrix<T>::cast_from_cm_to_BasicArrayArray(const Any& _from, Any& to)
{
   BasicArray<BasicArray<T> > & ans 
     = to.set<BasicArray<BasicArray<T> > > ();
   const CMSparseMatrix<T>& from
     = _from.template expose<CMSparseMatrix<T> > ();

   if (from.get_ncols() > 0)
   {
      ans.resize(from.get_nrows());
      for (size_t i = 0; i < ans.size(); i++)
      {
         ans[i].resize(from.get_ncols());
         ans[i] << 0.0;
      }

      for (size_t i = 0; i < ans[0].size(); i++)
      {
         int k = from.matcnt[i];
         if ( ! k )
            continue;

         const T*   val = &(from.matval[from.matbeg[i]]);
         const int* row = &(from.matind[from.matbeg[i]]);
         while ( k > 0 )
         {
            ans[*row][i] = *val;
            k--;
            val++;
            row++;
         }
      }
   }

   return OK;
}


template<typename T>
bool utilib::SparseMatrix<T>::register_aux_functions()
{
   //Serializer().register_serializer<pvector<_Tp> >
   //   ( std::string("utilib::pvector;") + mangledName(typeid(_Tp)),
   //     utilib::STL_Serializers::sequence_serializer<pvector<_Tp> > );
   
   TypeManager()->register_lexical_cast
      (typeid(utilib::CMSparseMatrix<T>), 
       typeid(utilib::RMSparseMatrix<T>), 
       &convert_cast< utilib::CMSparseMatrix<T>,
                      utilib::RMSparseMatrix<T> >);
   TypeManager()->register_lexical_cast
      (typeid(utilib::RMSparseMatrix<T>), 
       typeid(utilib::CMSparseMatrix<T>), 
       &convert_cast< utilib::RMSparseMatrix<T>,
                      utilib::CMSparseMatrix<T> >);

   TypeManager()->register_lexical_cast
      (typeid(utilib::BasicArray<utilib::BasicArray<T> >), 
       typeid(utilib::RMSparseMatrix<T>), 
       &cast_to_rm_from_dense<utilib::BasicArray<utilib::BasicArray<T> >, T>);
   TypeManager()->register_lexical_cast
      (typeid(utilib::RMSparseMatrix<T>),
       typeid(utilib::BasicArray<utilib::BasicArray<T> >), 
       &cast_from_rm_to_dense<utilib::BasicArray<utilib::BasicArray<T> >, T>);

   TypeManager()->register_lexical_cast
      (typeid(std::vector<std::vector<T> >), 
       typeid(utilib::RMSparseMatrix<T>), 
       &cast_to_rm_from_dense<std::vector<std::vector<T> >, T>);
   TypeManager()->register_lexical_cast
      (typeid(utilib::RMSparseMatrix<T>),
       typeid(std::vector<std::vector<T> >), 
       &cast_from_rm_to_dense<std::vector<std::vector<T> >, T>);

   TypeManager()->register_lexical_cast
      (typeid(utilib::BasicArray<utilib::BasicArray<T> >), 
       typeid(utilib::CMSparseMatrix<T>), 
       &cast_to_cm_from_BasicArrayArray);
   TypeManager()->register_lexical_cast
      (typeid(utilib::CMSparseMatrix<T>),
       typeid(utilib::BasicArray<utilib::BasicArray<T> >), 
       &cast_from_cm_to_BasicArrayArray);
   
   return true;
}

} // namespace utilib


#ifdef CXXTEST_RUNNING
#include <cxxtest/ValueTraits.h>
namespace CxxTest {

/// Printing utility for use in CxxTest unit tests
template<class T>
class ValueTraits< utilib::RMSparseMatrix<T> >
{
public:
   ValueTraits(const utilib::RMSparseMatrix<T>& t)
   {
      std::ostringstream ss;
      ss << t;
      str = ss.str().c_str();
   }

   const char *asString() const
   { return str.c_str(); }

private:
   std::string str;
};


/// Printing utility for use in CxxTest unit tests
template<class T>
class ValueTraits< utilib::CMSparseMatrix<T> >
{
public:
   ValueTraits(const utilib::CMSparseMatrix<T>& t)
   {
      std::ostringstream ss;
      ss << t;
      str = ss.str().c_str();
   }

   const char *asString() const
   { return str.c_str(); }

private:
   std::string str;
};

} // namespace CxxTest
#endif // CXXTEST_RUNNING


#endif // utilib_SparseMatrix_h
