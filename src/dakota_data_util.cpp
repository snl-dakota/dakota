/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description:  This file contains code related to data utilities that should
//-               be compiled, rather than inlined in data_util.hpp.
//-
//- Owner:        Mike Eldred
//- Version: $Id: dakota_data_util.cpp 7024 2010-10-16 01:24:42Z mseldre $

#include "dakota_data_util.hpp"
#include <boost/math/special_functions/round.hpp>
#include <boost/algorithm/string.hpp>
#include "SurrogatesPolynomialRegression.hpp"
#include "util_metrics.hpp"

using MatrixMap = Eigen::Map<Eigen::MatrixXd>;
using VectorMap = Eigen::Map<Eigen::VectorXd>;

namespace Dakota {

// ------------
// == operators
// ------------


bool nearby(const RealVector& rv1, const RealVector& rv2, Real rel_tol)
{
  // Check for equality in array lengths
  size_t len = rv1.length();
  if ( rv2.length() != len )
    return false;
	
  // Check each value (labels are ignored!)
  Real abs_tol = DBL_MIN; // ~ 2.2e-308
  for (size_t i=0; i<len; i++)
    // prevent division by 0
    if (std::abs(rv1[i]) < abs_tol) { //(rv1[i] == 0.)
      if (std::abs(rv2[i]) > abs_tol) //(rv2[i] != 0.)
	return false;
    }
    else if ( std::abs(1. - rv2[i]/rv1[i]) > rel_tol ) // DBL_EPSILON
      return false;

  return true;
}


bool operator==(const ShortArray& dsa1, const ShortArray& dsa2)
{
  // Check for equality in array lengths
  size_t len = dsa1.size();
  if ( dsa2.size() != len )
    return false;

  // Check each value
  for (size_t i=0; i<len; ++i)
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
  for (size_t i=0; i<len; ++i)
    if ( dsa2[i] != dsa1[i] )
      return false;

  return true;
}


// ---------------------------------
// miscellaneous numerical utilities
// ---------------------------------

Real rel_change_L2(const RealVector& curr_rv, const RealVector& prev_rv)
{
  size_t i, rv_len = prev_rv.length();
  Real norm = 0.;

  // check previous vector for zeros
  bool zero_prev = false, zero_curr = false;
  for (i=0; i<rv_len; ++i)
    if (Pecos::is_small(prev_rv[i]))
      { zero_prev = true; break; }
  // check current vector for zeros
  if (zero_prev)
    for (i=0; i<rv_len; ++i)
      if (Pecos::is_small(curr_rv[i]))
	{ zero_curr = true; break; }

  // Compute norm of relative change one of three ways
  if (!zero_prev) { // change relative to previous
    for (i=0; i<rv_len; ++i)
      norm += std::pow(curr_rv[i] / prev_rv[i] - 1., 2.);
    return std::sqrt(norm);
  }
  else if (!zero_curr) { // change relative to current
    for (i=0; i<rv_len; ++i)
      norm += std::pow(prev_rv[i] / curr_rv[i] - 1., 2.);
    return std::sqrt(norm);
  }
  else { // absolute change scaled by norm of previous
    Real scaling = 0.;
    for (i=0; i<rv_len; ++i) {
      norm    += std::pow(curr_rv[i] - prev_rv[i], 2.);
      scaling += std::pow(prev_rv[i], 2.);
    }
    return !Pecos::is_small(scaling) ?
      std::sqrt(norm / scaling) : std::sqrt(norm);
  }
}


Real rel_change_L2(const RealVector& curr_rv1, const RealVector& prev_rv1,
		   const IntVector&  curr_iv,  const IntVector&  prev_iv,
		   const RealVector& curr_rv2, const RealVector& prev_rv2)
{
  size_t i, rv1_len = prev_rv1.length(), iv_len = prev_iv.length(),
    rv2_len = prev_rv2.length();
  Real norm = 0.;

  // check previous vectors for zeros
  bool zero_prev = false, zero_curr = false;
  for (i=0; i<rv1_len; ++i)
    if (Pecos::is_small(prev_rv1[i]))
      { zero_prev = true; break; }
  if (!zero_prev)
    for (i=0; i<iv_len; ++i)
      if (std::abs(prev_iv[i]))
	{ zero_prev = true; break; }
  if (!zero_prev)
    for (i=0; i<rv2_len; ++i)
      if (Pecos::is_small(prev_rv2[i]))
	{ zero_prev = true; break; }
  // check current vectors for zeros
  if (zero_prev) {
    for (i=0; i<rv1_len; ++i)
      if (Pecos::is_small(curr_rv1[i]))
	{ zero_curr = true; break; }
    if (!zero_prev)
      for (i=0; i<iv_len; ++i)
	if (std::abs(curr_iv[i]))
	  { zero_curr = true; break; }
    if (!zero_prev)
      for (i=0; i<rv2_len; ++i)
	if (Pecos::is_small(curr_rv2[i]))
	  { zero_curr = true; break; }
  }

  // Compute norm of relative change one of three ways
  if (!zero_prev) { // change relative to previous
    for (i=0; i<rv1_len; ++i)
      norm += std::pow(curr_rv1[i] / prev_rv1[i] - 1., 2.);
    for (i=0; i<iv_len; ++i)
      norm += std::pow(curr_iv[i]  / prev_iv[i]  - 1., 2.);
    for (i=0; i<rv2_len; ++i)
      norm += std::pow(curr_rv2[i] / prev_rv2[i] - 1., 2.);
    return std::sqrt(norm);
  }
  else if (!zero_curr) { // change relative to current
    for (i=0; i<rv1_len; ++i)
      norm += std::pow(prev_rv1[i] / curr_rv1[i] - 1., 2.);
    for (i=0; i<iv_len; ++i)
      norm += std::pow(prev_iv[i]  / curr_iv[i]  - 1., 2.);
    for (i=0; i<rv2_len; ++i)
      norm += std::pow(prev_rv2[i] / curr_rv2[i] - 1., 2.);
    return std::sqrt(norm);
  }
  else { // absolute change scaled by norm of previous
    Real scaling = 0.;
    for (i=0; i<rv1_len; ++i) {
      norm    += std::pow(curr_rv1[i] - prev_rv1[i], 2.);
      scaling += prev_rv1[i] * prev_rv1[i];
    }
    for (i=0; i<iv_len; ++i) {
      norm    += std::pow(curr_iv[i] - prev_iv[i], 2.);
      scaling += prev_iv[i] * prev_iv[i];
    }
    for (i=0; i<rv2_len; ++i) {
      norm    += std::pow(curr_rv2[i] - prev_rv2[i], 2.);
      scaling += prev_rv2[i] * prev_rv2[i];
    }
    return !Pecos::is_small(scaling) ?
      std::sqrt(norm / scaling) : std::sqrt(norm);
  }
}

//----------------------------------------------------------------

void compute_col_means(RealMatrix& matrix, RealVector& avg_vals)
{
  int num_cols = matrix.numCols();
  int num_rows = matrix.numRows();

  avg_vals.resize(num_cols);
  
  RealVector ones_vec(num_rows);
  ones_vec.putScalar(1.0);
 
  for(int i=0; i<num_cols; ++i){
    const RealVector& col_vec = Teuchos::getCol(Teuchos::View, matrix, i);
    avg_vals(i) = col_vec.dot(ones_vec)/((Real) num_rows);
  }
}

//----------------------------------------------------------------

void compute_col_stdevs(RealMatrix& matrix, RealVector& avg_vals, 
                        RealVector& std_devs)
{
  int num_cols = matrix.numCols();
  int num_rows = matrix.numRows();

  std_devs.resize(num_cols);
  RealVector res_vec(num_rows);

  for(int i=0; i<num_cols; ++i){
    const RealVector& col_vec = Teuchos::getCol(Teuchos::View, matrix, i);
    for(int j = 0; j<num_rows; ++j){
      res_vec(j) = col_vec(j) - avg_vals(i);
    }
    std_devs(i) = std::sqrt(res_vec.dot(res_vec)/((Real) num_rows-1));
  }
}

//----------------------------------------------------------------

void sort_vector( const RealVector & vec, RealVector & sort_vec, IntVector & indices )
{
  if( indices.length() != vec.length() )
    indices.resize(vec.length()); // avoid initialization to 0.0

  // initialize indices from 0 to vec.length()-1
  std::iota(indices.values(), indices.values()+vec.length(), 0);

  // Sort indices using values from incoming vec
  std::sort( indices.values(), indices.values()+vec.length(), [&](Real i,Real j){return vec[i]<vec[j];} );

  if( sort_vec.length() != vec.length() )
    sort_vec.resize(vec.length()); // avoid initialization to 0.0

  // Copy into target vector in sorted order
  for( int i=0; i<vec.length(); ++i )
    sort_vec[i] = vec[indices[i]];
}

//----------------------------------------------------------------

void sort_matrix_columns( const RealMatrix & mat, RealMatrix & sort_mat, IntMatrix & indices )
{
  if( (mat.numRows() != sort_mat.numRows()) ||
      (mat.numCols() != sort_mat.numCols()) )
    sort_mat.shapeUninitialized(mat.numRows(), mat.numCols()); // avoid initialization to 0.0

  if( (mat.numRows() != indices.numRows()) ||
      (mat.numCols() != indices.numCols()) )
    indices.shapeUninitialized(mat.numRows(), mat.numCols()); // avoid initialization to 0.0

  for( int i=0; i<mat.numCols(); ++i )
  {
    RealMatrix & nonconst_mat = const_cast<RealMatrix&>(mat);
    const RealVector & unsrt_vec = Teuchos::getCol(Teuchos::View, nonconst_mat, i);
    RealVector sorted_vec   = Teuchos::getCol(Teuchos::View, sort_mat, i);
    IntVector  sort_indices = Teuchos::getCol(Teuchos::View, indices, i);
    sort_vector( unsrt_vec, sorted_vec, sort_indices);
  }
}

//----------------------------------------------------------------

void center_matrix_rows( RealMatrix & mat )
{
  int num_row = mat.numRows(), num_col = mat.numCols();
  for (int i=0; i<num_row; i++) {
    // normalize each row (input/output factor) by its mean across observations
    Real row_mean = 0.0;
    for (int j=0; j<num_col; j++)
      row_mean += mat(i,j);
    row_mean /= (Real)num_col;
    for (int j=0; j<num_col; j++)
      mat(i,j) -= row_mean;
  }
}

//----------------------------------------------------------------

void center_matrix_cols( RealMatrix & mat )
{
  int num_row = mat.numRows(), num_col = mat.numCols();
  for (int j=0; j<num_col; j++) {
    // normalize each column (input/output factor) by its mean across observations
    Real col_mean = 0.0;
    for (int i=0; i<num_row; i++)
      col_mean += mat(i,j);
    col_mean /= (Real)num_row;
    for (int i=0; i<num_row; i++)
      mat(i,j) -= col_mean;
  }
}

//----------------------------------------------------------------

bool is_matrix_symmetric( const RealMatrix & matrix )
{
  int num_cols = matrix.numCols();
  int num_rows = matrix.numRows();

  if( num_cols != num_rows )
    return false;

  bool is_symmetric = true;

  for( int i=0; i<num_cols; ++i )
    for( int j=i+1; j<num_cols; ++j ) {
      if( matrix(i,j) != matrix(j,i) ) {
        is_symmetric = false;
        break;
      }
    }

  return is_symmetric;
}

//----------------------------------------------------------------

Real compute_regression_coeffs( const RealMatrix & samples, const RealMatrix & resps,
                                      RealVector & reg_coeffs )
{
  if (samples.numRows() == 0)
    return 0.0;

  // Copy Teuchos data to Eigen data to work with surrogates
  MatrixXd copy_samples;
  MatrixXd copy_responses;

  copy_data(samples, copy_samples);
  copy_data(resps,   copy_responses);

  Teuchos::ParameterList param_list("Polynomial Test Parameters");
  // Use the defaults for now, cf
  //     PolynomialRegression::default_options()
  // ... which provides regular RCs (from linear model)

  dakota::surrogates::PolynomialRegression pr(copy_samples, copy_responses, param_list);
  const MatrixXd & coeffs = pr.get_polynomial_coeffs();

  // Slice off the constant coeff
  int slc = samples.numCols();

  // A workaround to deal with the inability of RealVector to use const double* data
  VectorXd nonconst_coeffs = coeffs.col(0).tail(slc);
  reg_coeffs = RealVector(Teuchos::Copy, nonconst_coeffs.data(), slc);
  
  // Compute and return coefficient of determination, R^2
  VectorXd sur_vals = pr.value(copy_samples);
  return dakota::util::compute_metric(sur_vals, copy_responses.col(0), "rsquared");
}

//----------------------------------------------------------------

Real compute_std_regression_coeffs( const RealMatrix & samples, const RealMatrix & resps,
                                          RealVector & std_reg_coeffs)
{
  if (samples.numRows() == 0)
    return 0.0;

  // Compute the non-standardized RCs and R^2
  RealVector reg_coeffs;
  Real r2 = compute_regression_coeffs(samples, resps, reg_coeffs);

  // Then scale each RC by (stdev_i/stdev_r)
  // where:
  //    stdev_i is std dev of variable i
  //    stdev_r is std dev of response

  // We could avoid copying into MatrixXd and instead use RealMatrix
  // objects if we implement a stddev utility for the RealMatrix columns.
  MatrixXd copy_samples;
  MatrixXd copy_responses;
  copy_data(samples, copy_samples);
  copy_data(resps,   copy_responses);

  // Compute sample stddevs
  int ncols = copy_samples.cols();
  VectorXd means(ncols);
  VectorXd stddevs(ncols);
  double stddev;
  for (int j = 0; j < ncols; j++) {
    means(j) = copy_samples.col(j).mean();
    VectorXd tmp = copy_samples.col(j).array() - means(j);
    stddev = tmp.dot(tmp)/(copy_samples.rows()-1);
    stddevs(j) = std::sqrt(stddev);
  }

  // Compute response stddev
  VectorXd tmp = copy_responses.col(0).array() - copy_responses.col(0).mean();
  double resp_stddev = std::sqrt(tmp.dot(tmp)/(copy_responses.rows()-1));

  // Handle the case of resp_stddev = 0 (based on Dakota's definition of small)
  if (resp_stddev < Pecos::SMALL_NUMBER) {
    std_reg_coeffs = reg_coeffs;
    std_reg_coeffs.putScalar(0.0);
    return 0.0;
  }

  // Scale RCs by ratio of std devs, assumes linear regression model
  VectorMap polynomial_coeffs(reg_coeffs.values(), ncols);
  VectorXd std_polynomial_coeffs = (polynomial_coeffs.array()*stddevs.array())/resp_stddev;

  std_reg_coeffs = RealVector(Teuchos::Copy, std_polynomial_coeffs.data(), ncols);
  // We return the R^2 based on the non-standardized linear regression model
  return r2;
}

//----------------------------------------------------------------

void remove_column(RealMatrix& matrix, int index)
{
  int num_cols = matrix.numCols();
  RealMatrix matrix_new(matrix.numRows(), num_cols-1);
  for (int i = 0; i<num_cols; ++i){
      const RealVector& col_vec = Teuchos::getCol(Teuchos::View, matrix, i);
    if (i < index){
      Teuchos::setCol(col_vec, i, matrix_new);
    }
    if (i > index){
      Teuchos::setCol(col_vec, i-1, matrix_new);
    }
  }
  matrix.reshape(matrix.numRows(), num_cols-1);
  matrix = matrix_new;
}

//----------------------------------------------------------------

void copy_data( const MatrixXd & src_mat, RealMatrix & dst_mat )
{
  const int nrows = src_mat.rows();
  const int ncols = src_mat.cols();
  dst_mat.reshape(nrows, ncols);
  for( int i=0; i<nrows; ++i )
    for( int j=0; j<ncols; ++j )
      dst_mat(i,j) = src_mat(i,j);
}

//----------------------------------------------------------------

void copy_data( const RealMatrix & src_mat, MatrixXd & dst_mat )
{
  const int nrows = src_mat.numRows();
  const int ncols = src_mat.numCols();
  dst_mat.resize(nrows, ncols);
  for( int i=0; i<nrows; ++i )
    for( int j=0; j<ncols; ++j )
      dst_mat(i,j) = src_mat(i,j);
}

//----------------------------------------------------------------

void view_data( const RealMatrix & src_mat, Eigen::Map<MatrixXd> & mat_view )
{
  const int nrows = src_mat.numRows();
  const int ncols = src_mat.numCols();
  new (&mat_view) Eigen::Map<MatrixXd>(src_mat.values(), nrows, ncols);
}

//----------------------------------------------------------------

std::vector<std::string> strsplit(const std::string& input)
{
  std::vector<std::string> fields;
  std::string trimmed_input(boost::trim_copy(input));
  boost::split(fields, trimmed_input, boost::is_any_of(" \t"),
	       boost::token_compress_on);
  return fields;
}


std::string::size_type longest_strlen(const std::vector<std::string>& vecstr)
{
  auto size_less = [](const std::string& a, const std::string& b)
    { return a.size() < b.size(); };

  return std::max_element(vecstr.begin(), vecstr.end(), size_less)->size();
}


void iround(const RealVector& input_vec, IntVector& rounded_vec)
{
  int len = input_vec.length();
  if (rounded_vec.length() != len)
    rounded_vec.resize(len);
  for (int i=0; i<len; ++i)
    rounded_vec[i] = boost::math::iround(input_vec[i]);
}

} // namespace Dakota
