/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ReducedBasis.hpp"

#include <Teuchos_SerialDenseHelpers.hpp>

namespace Dakota {

// ------------------------------------------

ReducedBasis::ReducedBasis() :
  is_valid_svd(false)
{
}

// ------------------------------------------

void
ReducedBasis::set_matrix(const RealMatrix & mat)
{
  matrix = mat;
  is_valid_svd = false;
}

// ------------------------------------------

void
ReducedBasis::center_matrix()
{
  compute_column_means(matrix, column_means);

  // working vector
  RealVector column_vec(matrix.numRows());

  for( int i=0; i<matrix.numCols(); ++i ) {
    column_vec.putScalar(column_means(i));
    RealVector matrix_column = Teuchos::getCol(Teuchos::View, matrix, i);
    matrix_column -= column_vec;
  }

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
  compute_svd(workingMatrix, workingMatrix.numRows(), workingMatrix.numCols(), S_values, VT_matrix);
  U_matrix = workingMatrix;

  is_valid_svd = true;
}

}  // namespace Dakota
