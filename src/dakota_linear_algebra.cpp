/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_global_defs.hpp"
#include "dakota_linear_algebra.hpp"
#include "Teuchos_LAPACK.hpp"

namespace Dakota {

void svd(RealMatrix& matrix, RealVector& singular_vals, RealMatrix& v_trans)
{
  Teuchos::LAPACK<int, Real> la;

  // ----
  // compute the SVD of the incoming matrix
  // ----

  char JOBU = 'O';  // overwrite A with U
  char JOBVT = 'A'; // compute all singular vectors VT
  int M(matrix.numRows());
  int N(matrix.numCols());
  int LDA = matrix.stride();
  int num_singular_values = std::min(M, N);
  singular_vals.resize(num_singular_values);
  Real* U = NULL;
  int LDU = 1;
  v_trans.reshape(N, N);
  int LDVT = N;
  int info = 0;
  // Not used by real SVD?
  double* RWORK = NULL;

  int work_size = -1;         // special code for workspace query
  double* work = new Real[1]; // temporary work array
  la.GESVD(JOBU, JOBVT, M, N, matrix[0], LDA, &singular_vals[0],
	   U, LDU, v_trans[0], LDVT, work, work_size, RWORK, &info);
  work_size = (int)work[0];   // optimal work array size returned by query
  delete [] work;

  work = new Real[work_size];
  la.GESVD(JOBU, JOBVT, M, N, matrix[0], LDA, &singular_vals[0],
	   U, LDU, v_trans[0], LDVT, work, work_size, RWORK, &info);
  delete [] work;

  if (info < 0) {
    Cerr << "\nError: svd() failed. " << "The " << std::abs( info ) 
	 << "-th argument had an illegal value.\n";
    abort_handler(-1);
  }
  if (info > 0) {
    Cerr << "\nError: svd() failed. " << info << "superdiagonals of an "
	 << "intermediate bidiagonal form B did not converge to 0.\n";
    abort_handler(-1);
  }
}

}  // namespace Dakota
