/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ReducedBasis.hpp"
#include "dakota_linear_algebra.hpp"
#include "dakota_global_defs.hpp"

#include <Teuchos_SerialDenseHelpers.hpp>

namespace Dakota {

// ------------------------------------------

ReducedBasis::ReducedBasis() :
  col_means_computed(false), is_centered(false), is_valid_svd(false)
{
}

// ------------------------------------------

void
ReducedBasis::set_matrix(const RealMatrix & mat)
{
  matrix = mat;
  col_means_computed = false;
  is_centered = false;
  is_valid_svd = false;
}

// ------------------------------------------

void
ReducedBasis::center_matrix()
{
  if ( is_centered )
    return;

  compute_column_means(matrix, column_means);
  col_means_computed = true;

  // working vector
  RealVector column_vec(matrix.numRows());

  for( int i=0; i<matrix.numCols(); ++i ) {
    column_vec.putScalar(column_means(i));
    RealVector matrix_column = Teuchos::getCol(Teuchos::View, matrix, i);
    matrix_column -= column_vec;
  }

  is_centered = true;
  is_valid_svd = false;
}

// ------------------------------------------

void
ReducedBasis::update_svd(bool do_center)
{
  if( is_valid_svd )
    return;

  if( matrix.empty() )
    throw std::runtime_error("Matrix is empty.  Make sure to call set_matrix(...) first.");

  if( do_center )
    center_matrix();

  workingMatrix = matrix; // because the matrix gets overwritten by U_matrix values
  svd(workingMatrix, S_values, VT_matrix);
  U_matrix = workingMatrix;

  RealVector ones(S_values.length());
  ones = 1.0;
  singular_values_sum = ones.dot(S_values);

  eigen_values_sum = 0.0;
  for( int i=0; i<S_values.length(); ++i )
    eigen_values_sum += S_values(i)*S_values(i);

  is_valid_svd = true;
}

// ------------------------------------------

RealVector
ReducedBasis::get_singular_values(const TruncationCondition & truncation_cond) const
{
  int num_values = truncation_cond.get_num_components(*this);

  RealVector vec(num_values);
  for( int i=0; i<num_values; ++i )
    vec(i) = S_values(i);

  return vec;
}

// ------------------------------------------
// -------- Truncation Conditions  ----------
// ------------------------------------------


void
ReducedBasis::TruncationCondition::sanity_check(const ReducedBasis & basis) const
{
  if( !basis.is_valid() ) {
    Cerr << "\nError: Truncation condition cannot be applied before computing a valid ReducedBasis SVD."
      << std::endl;
    abort_handler(-1);
  }
}

// ------------------------------------------

ReducedBasis::Untruncated::Untruncated() :
  TruncationCondition()
{ }

int ReducedBasis::Untruncated::get_num_components(const ReducedBasis & basis) const
{
  sanity_check(basis);
  return basis.get_singular_values().length();
}

// ------------------------------------------

ReducedBasis::VarianceExplained::VarianceExplained(Real var_exp) :
  TruncationCondition(),
  variance_explained(var_exp)
{ 
  if( (0.0 > var_exp) || (1.0 < var_exp) ) {
    Cerr << "\nError: VarianceExplained Truncation condition must be in the range (0.0, 1,0)."
      << std::endl;
    abort_handler(-1);
  }
}

int ReducedBasis::VarianceExplained::get_num_components(const ReducedBasis & basis) const
{
  sanity_check(basis);

  Real total_sum = basis.get_eigen_values_sum();
  const RealVector & singular_vals = basis.get_singular_values();
  int num_comp = 0;
  Real partial_sum = 0.0;

  while( partial_sum/total_sum < variance_explained )
    partial_sum += singular_vals(num_comp)*singular_vals(num_comp++);

  return num_comp;
}

// ------------------------------------------

ReducedBasis::HeuristicVarianceExplained::HeuristicVarianceExplained(Real var_exp) :
  TruncationCondition(),
  variance_explained(var_exp)
{ 
  if( (0.0 > var_exp) || (1.0 < var_exp) ) {
    Cerr << "\nError: HeuristicVarianceExplained Truncation condition must be in the range (0.0, 1,0)."
      << std::endl;
    abort_handler(-1);
  }
}

int ReducedBasis::HeuristicVarianceExplained::get_num_components(const ReducedBasis & basis) const
{
  sanity_check(basis);

  const RealVector & singular_vals = basis.get_singular_values();
  Real largest_eig_val = singular_vals(0)*singular_vals(0);
  int num_comp = 0;
  Real ratio = 1.0;

  while( ratio > (1.0-variance_explained) )
    ratio = singular_vals(num_comp)*singular_vals(num_comp++)/largest_eig_val;

  return num_comp;
}

// ------------------------------------------

ReducedBasis::NumComponents::NumComponents(int num_comp) :
  TruncationCondition(),
  num_components(num_comp)
{ }

int ReducedBasis::NumComponents::get_num_components(const ReducedBasis & basis) const
{
  sanity_check(basis);
  return num_components;
}

}  // namespace Dakota
