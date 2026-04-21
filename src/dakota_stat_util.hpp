/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
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
    for (i=0; i<nc; ++i)
      avg_vec[i] = average(mat[i], nr);
    break;
  case 1: // average over index 1 (cols), retaining index 0 (rows)
    avg_vec.sizeUninitialized(nr);
    switch (nc) {
    case 0:
      for (i=0; i<nr; ++i) avg_vec[i] = std::numeric_limits<Real>::quiet_NaN();
      break;
    case 1:
      for (i=0; i<nr; ++i) avg_vec[i] = mat(i,0);
      break;
    default:
      for (i=0; i<nr; ++i) {
	Real& avg_i = avg_vec[i];
	avg_i = 0.;
	for (j=0; j<nc; ++j) avg_i += mat(i,j);
	avg_i /= nc;
      }
      break;
    }
    break;
  default:
    Cerr << "Error: bad averaging index (" << avg_index
	 << ") in average(RealMatrix)." << std::endl;
    abort_handler(-1); break;
  }
}


/// compute row-averages for each column or column-averages for each row
inline void average(const Sizet2DArray& array, RealVector& avg_array,
		    size_t avg_index = 1)
{
  // typically the second dimension is number of QoI, which does not vary
  size_t i, j, num_array = array.size(), array_len;
  switch (avg_index) {
  case 0: // average over index 0 (num_array), retaining index 1 (array_len)
    switch (num_array) {
    case 0:
      avg_array.size(0);
      //avg_array.putScalar(std::numeric_limits<Real>::quiet_NaN());
      break;
    case 1: {
      const SizetArray& array_0 = array[0];
      array_len = array_0.size();
      avg_array.sizeUninitialized(array_len);
      for (j=0; j<array_len; ++j)
	avg_array[j] = array_0[j];
      break;
    }
    default: {
      size_t max_array_len = 0;
      for (i=0; i<num_array; ++i) {
	array_len = array[i].size();
	if (array_len > max_array_len) max_array_len = array_len;
      }
      avg_array.size(max_array_len); // init to 0
      SizetArray count;  count.assign(max_array_len, 0);
      for (i=0; i<num_array; ++i) {
	array_len = array[i].size();
	for (j=0; j<array_len; ++j)
	  { avg_array[j] += array[i][j];  ++count[j]; }
      }
      for (j=0; j<max_array_len; ++j)
	avg_array[j] /= count[j];
      break;
    }
    }
    break;
  case 1: // average over index 1 (array_len), retaining index 0 (num_array)
    avg_array.sizeUninitialized(num_array);
    for (i=0; i<num_array; ++i)
      avg_array[i] = average(array[i]); // handles quiet_nan et al.
    break;
  default:
    Cerr << "Error: bad averaging index (" << avg_index
	 << ") in average(Sizet2DArray)." << std::endl;
    abort_handler(-1); break;
  }
}


inline Real power_sum(const RealVector& vec, Real p)
{
  Real sum = 0.;  int i, len = vec.length();
  for (i=0; i<len; ++i)
    sum += std::pow(std::abs(vec[i]), p);
  return sum;
}


inline Real power_sum(const RealMatrix& rm, Real p)
{
  Real sum = 0.;  int r, c, nr = rm.numRows(), nc = rm.numCols();
  for (r=0; r<nr; ++r)
    for (c=0; c<nc; ++c)
      sum += std::pow(std::abs(rm(r,c)), p);
  return sum;
}


inline Real power_sum(const RealSymMatrix& rsm, Real p)
{
  Real sum = 0.;  int r, c, nrc = rsm.numRows();
  for (r=0; r<nrc; ++r) {
    sum += std::pow(std::abs(rsm(r,r)), p);
    for (c=0; c<r; ++c)
      sum += 2.*std::pow(std::abs(rsm(r,c)), p);
  }
  return sum;
}


// Note that p >= 1 is required to satisfy the formal definition of a norm
// (0 < p < 1 is computable but not a norm: violates triangle inequality)
inline Real p_norm(const RealVector& vec, Real p)
{ return std::pow(power_sum(vec, p), 1./p); }


inline Real p_norm(const RealMatrix& rm, Real p)
{ return std::pow(power_sum(rm, p), 1./p); }


inline Real p_norm(const RealSymMatrix& rsm, Real p)
{ return std::pow(power_sum(rsm, p), 1./p); }


inline Real p_norm_diff(const RealVector& v1, const RealVector& v2, Real p)
{
  int i, len = v1.length();
  if (v2.length() != len) {
    Cerr << "Error: incompatible vector lengths in p_norm_diff()." << std::endl;
    abort_handler(-1);
  }
  Real sum = 0.;
  for (i=0; i<len; ++i)
    sum += std::pow(std::abs(v1[i] - v2[i]), p);
  return std::pow(sum, 1./p);
}


inline Real p_norm_diff(const RealMatrix& rm1, const RealMatrix& rm2, Real p)
{
  int r, c, nr = rm1.numRows(), nc = rm1.numCols();
  if (rm2.numRows() != nr || rm2.numCols() != nc) {
    Cerr << "Error: incompatible matrix dimensions in p_norm_diff()."
	 << std::endl;
    abort_handler(-1);
  }
  Real sum = 0.;
  for (r=0; r<nr; ++r)
    for (c=0; c<nc; ++c)
      sum += std::pow(std::abs(rm1(r,c) - rm2(r,c)), p);
  return std::pow(sum, 1./p);
}


inline Real p_norm_diff(const RealSymMatrix& rsm1,
			const RealSymMatrix& rsm2, Real p)
{
  int r, c, nrc = rsm1.numRows();
  if (rsm2.numRows() != nrc) {
    Cerr << "Error: incompatible symmetric matrix dimension in p_norm_diff()."
	 << std::endl;
    abort_handler(-1);
  }
  Real sum = 0.;
  for (r=0; r<nrc; ++r) {
    sum += std::pow(std::abs(rsm1(r,r) - rsm2(r,r)), p);
    for (c=0; c<r; ++c)
      sum += 2.*std::pow(std::abs(rsm1(r,c) - rsm2(r,c)), p);
  }
  return std::pow(sum, 1./p);
}


inline Real p_norm_diff(const RealSymMatrix& rsm, const RealMatrix& rm, Real p)
{
  int r, c, nrc = rsm.numRows();
  if (rm.numRows() != nrc || rm.numCols() != nrc) {
    Cerr << "Error: incompatible matrix dimensions in p_norm_diff()."
	 << std::endl;
    abort_handler(-1);
  }
  // rsm access to upper triangle is managed by operator()
  Real sum = 0.;
  for (r=0; r<nrc; ++r)
    for (c=0; c<nrc; ++c)
      sum += std::pow(std::abs(rsm(r,c) - rm(r,c)), p);
  return std::pow(sum, 1./p);
}


/// compute maximum of a vector of values
inline Real maximum(const RealVector& vec)
{
  Real max = -DBL_MAX;
  size_t i, len = vec.length();
  for (i=0; i<len; ++i)
    if (vec[i] > max)
      max = vec[i];
  return max;
}

} // namespace Dakota

#endif // DAKOTA_STAT_UTIL_H
