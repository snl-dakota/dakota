/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ReducedBasis.hpp"

namespace Dakota {

// ------------------------------------------

ReducedBasis::ReducedBasis() :
  is_valid(false)
{
}

// ------------------------------------------

void
ReducedBasis::set_matrix(const RealMatrix & mat)
{
  matrix = mat;

  is_valid = false;
}

// ------------------------------------------

void
ReducedBasis::update_svd()
{
  if( is_valid )
    return;

  if( matrix.empty() )
    throw std::runtime_error("Matrix is empty.  Make sure to call set_matrix(...) first.");

  workingMatrix = matrix; // because the matrix gets overwritten
  compute_svd(workingMatrix, workingMatrix.numRows(), workingMatrix.numCols(), S_values, VT_matrix);
  U_matrix = workingMatrix;

  is_valid = true;
}

}  // namespace Dakota
