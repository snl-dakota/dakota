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
 * \file math_matrix.h
 *
 * Defines and constants for numerical operations on matrices.
 */

#ifndef utilib_math_matrix_h
#define utilib_math_matrix_h

#include <utilib_config.h>
#include <utilib/math_array.h>
#include <utilib/Basic2DArray.h>
#include <utilib/DoubleMatrix.h>

namespace utilib {


/// Compute the means of the rows of a 2D array.
template <class T>
NumArray<double> mean(const Basic2DArray<T>& matrix, const int stats_flag)
{
NumArray<double> temp(matrix.ncols());

temp = 0.0;

for (size_type j=0; j<matrix.nrows(); j++)
  for (size_type i=0; i<matrix.ncols(); i++)
    temp[i] += matrix[j][i];

if (stats_flag == TRUE)
     temp /= ((double) (matrix.nrows() - 1));
else
     temp /= ((double) matrix.nrows());

return( temp );
}


/// Compute the variances of the rows of a matrix.
template <class T>
DoubleVector var(const Basic2DArray<T>& mat, const int sampleflag=TRUE)
{
DoubleVector array_mean;
return( var(mat, array_mean, sampleflag) );
}


/// Compute the variances of the rows of a 2D array, given the means.
template <class T>
DoubleVector var(const Basic2DArray<T>& mat, BasicArray<double>& array_mean, 
							const int sampleflag=TRUE)
{
DoubleVector ans(mat.ncols());
array_mean &= mean(mat);

ans = 0.0;
for (size_type i=0; i<mat.nrows(); i++)
  for (size_type j=0; j<ans.size(); j++)
    ans[j] += ((mat[i][j] - array_mean[j]) * (mat[i][j] - array_mean[j]));

if ((sampleflag == FALSE) || (mat.nrows() == 1))
   ans /= (double) (mat.nrows());
else
   ans /= (double)(mat.nrows()-1);
return ans;
}


/// Return the minimal element of a 2D array.
template <class T>
int min(const Basic2DArray<T>& mat)
{
T ans,tmp;

ans = mat[0][0];
for (size_type i=0; i<mat.nrows(); i++)
  for (size_type j=0; j<mat.ncols(); j++) {
    tmp = mat[i][j];
    if (tmp < ans) ans=tmp;
    }

return ans;
}


/// Return the maximal element of a 2D array.
template <class T>
T max(const Basic2DArray<T>& mat)
{
T ans,tmp;

ans = mat[0][0];
for (size_type i=0; i<mat.nrows(); i++)
  for (size_type j=0; j<mat.ncols(); j++) {
    tmp = mat[i][j];
    if (tmp > ans) ans=tmp;
    }

return ans;
}


/// Return the sum of the elements of a 2D array.
template <class T>
T sum(const Basic2DArray<T>& mat)
{
T ans;

ans = 0;
for (size_type i=0; i<mat.nrows(); i++)
  for (size_type j=0; j<mat.ncols(); j++)
    ans += mat[i][j];

return ans;
}


/// Scale the rows of a 2D array by \a a.
template <class T>
void rowscale(Basic2DArray<T>& a)
{
double tmp;
size_type j,i;

for (i=0; i<a.nrows(); i++) {
  tmp = 0.0;
  for (j=0; j<a.ncols(); j++)
    tmp += a[i][j];
  if (tmp != 0.0)
     for (j=0; j<a.ncols(); j++)
       a[i][j] /= tmp;
  }
}


/**
 * Compute the Cholesky decomposition of \a A.
 * This code assumes that \a A is symmetric positive definite.  It generates
 * a lower triangular \a G s.t. A = G*G'. This code assumes that the memory in 
 * \a G has been allocated contiguously.
 */
int cholesky (DoubleMatrix& A, DoubleMatrix& G, size_type n);


} // namespace utilib

#endif
