/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_STAT_UTIL_H
#define DAKOTA_STAT_UTIL_H

#include "dakota_data_types.hpp"
#include "dakota_data_util.hpp"

// Statistics-related utilities

namespace Dakota {

/// clock microseconds-based random seed in [1, 1000000]
int generate_system_seed();

#ifdef HAVE_DAKOTA_SURROGATES
/// Compute (non-standardized) linear regression coefficients and return R^2
void compute_regression_coeffs( const RealMatrix & samples,
                                const RealMatrix & resps,
                                RealMatrix & coeffs,
                                RealVector & cods );

/// Compute standardized linear regression coefficients
void compute_std_regression_coeffs( const RealMatrix & samples,
                                    const RealMatrix & resps,
                                    RealMatrix & coeffs,
                                    RealVector & cods );
#endif


/// compute sum of a set of observations
template <typename OrdinalType, typename ScalarType> 
ScalarType sum(const ScalarType* ptr, OrdinalType ptr_len)
{
  ScalarType sum = 0.;
  for (OrdinalType i=0; i<ptr_len; ++i)
    sum += ptr[i];
  return sum;
}


/// compute sum of a set of observations
template <typename OrdinalType, typename ScalarType> 
ScalarType sum(const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& vec)
{ return sum(vec.values(), vec.length()); }


/// compute sum of a set of observations
template <typename ScalarType> 
ScalarType sum(const std::vector<ScalarType>& vec)
{
  ScalarType sum = 0.;  size_t i, len = vec.size();
  for (i=0; i<len; ++i)
    sum += vec[i];
  return sum;
}


/// compute average of a set of observations
template <typename OrdinalType, typename ScalarType> 
Real average(const ScalarType* ptr, OrdinalType ptr_len)
{
  switch (ptr_len) {
  case 0:  return std::numeric_limits<Real>::quiet_NaN();   break;
  case 1:  return (Real)ptr[0];                             break;
  default: return (Real)sum(ptr, ptr_len) / (Real)ptr_len;  break;
  }
}


/// compute average of a vector of observations
inline Real average(const RealVector& vec)
{ return average(vec.values(), vec.length()); }


/// compute average of a matrix of observations
inline Real average(const RealMatrix& mat)
{ return average(mat.values(), mat.numRows() * mat.numCols()); }


/// compute sum of a vector of observations
inline Real average(const SizetVector& vec)
{ return average(vec.values(), vec.length()); }


// compute sum of a matrix of observations
//inline Real average(const SizetMatrix& mat)
//{ return average(mat.values(), mat.numRows() * mat.numCols()); }


/// compute average of a vector of counts
inline Real average(const SizetArray& sa)
{
  size_t len = sa.size();
  switch (len) {
  case 0:  return std::numeric_limits<Real>::quiet_NaN();  break;
  case 1:  return (Real)sa[0];         break;
  default: return sum(sa) / (Real)len; break;
  }
}


/// compute row-averages for each column or column-averages for each row
inline void average(const RealMatrix& mat, size_t avg_index,
		    RealVector& avg_vec)
{
  size_t i, j, nr = mat.numRows(), nc = mat.numCols();
  switch (avg_index) {
  case 0: // average over index 0, retaining index 1
    avg_vec.sizeUninitialized(nc);
    for (i=0; i<nc; ++i)   // average over rows for each col vec
      avg_vec[i] = average(mat[i], nr);
    break;
  case 1:
    avg_vec.sizeUninitialized(nr);
    for (i=0; i<nr; ++i) { // average over cols for each row vec
      Real& avg_i = avg_vec[i];
      switch (nc) {
      case 0: avg_i = std::numeric_limits<Real>::quiet_NaN(); break;
      case 1: avg_i = mat(i,0); break;
      default:
	avg_i = 0.;
	for (j=0; j<nc; ++j) avg_i += mat(i,j);
	avg_i /= nc;
	break;
      }
    }
    break;
  default:
    Cerr << "Error: bad averaging index (" << avg_index
	 << ") in NonDEnsembleSampling::average(RealMatrix)." << std::endl;
    abort_handler(-1); break;
  }
}


/// eliminate inner dimension of 2D array by averaging over each inner vector
inline void average(const Sizet2DArray& N_2D, RealVector& N_1D)
{
  size_t i, len = N_2D.size();
  N_1D.sizeUninitialized(len);
  for (i=0; i<len; ++i)
    N_1D[i] = average(N_2D[i]);
}

} // namespace Dakota

#endif // DAKOTA_STAT_UTIL_H
