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

void svd(RealMatrix& matrix, RealVector& singular_vals, RealMatrix& v_trans,
	 bool compute_vectors)
{
  Teuchos::LAPACK<int, Real> la;

  // ----
  // compute the SVD of the incoming matrix
  // ----

  char JOBU = 'N';
  char JOBVT = 'N';
  if (compute_vectors) {
    JOBU = 'O';  // overwrite A with U
    JOBVT = 'A'; // compute all singular vectors VT
  }
  int M(matrix.numRows());
  int N(matrix.numCols());
  int LDA = matrix.stride();
  int num_singular_values = std::min(M, N);
  singular_vals.resize(num_singular_values);
  Real* U = NULL;
  int LDU = 1;
  int LDVT = 1;
  if (compute_vectors) {
    v_trans.reshape(N, N);
    LDVT = N;
  }
  int info = 0;
  // Not used by real SVD?
  double* RWORK = NULL;

  int work_size = -1;         // special code for workspace query
  double* work = new Real[1]; // temporary work array
  la.GESVD(JOBU, JOBVT, M, N, matrix[0], LDA, &singular_vals[0],
	   U, LDU, v_trans.values(), LDVT, work, work_size, RWORK, &info);
  work_size = (int)work[0];   // optimal work array size returned by query
  delete [] work;

  work = new Real[work_size];
  la.GESVD(JOBU, JOBVT, M, N, matrix[0], LDA, &singular_vals[0],
	   U, LDU, v_trans.values(), LDVT, work, work_size, RWORK, &info);
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


void singular_values(RealMatrix& matrix, RealVector& singular_vals)
{
  // empty matrix with NULL .values()
  RealMatrix v_trans;
  svd(matrix, singular_vals, v_trans, false);
}


int qr(RealMatrix& A)
{
  Teuchos::LAPACK<int, Real> la;
 
  int M = A.numRows();
  int N = A.numCols();
  int LDA = A.stride();
  int tau_size = std::min(M, N);
  RealMatrix TAU(tau_size, tau_size);
  int info = 0;

  int work_size = -1;            // special code for workspace query
  double* work = new double[1];  // temporary work array
  la.GEQRF(M, N, A.values(), LDA, TAU.values(), work, work_size, &info);
  work_size = (int)work[0];   // optimal work array size returned by query
  delete [] work;

  work = new double[work_size];
  la.GEQRF(M, N, A.values(), LDA, TAU.values(), work, work_size, &info);
  delete [] work;

  if (info < 0) {
    Cerr << "Error (qr): the " << -info << "-th argument had an illegal "
	 << "value.";
    abort_handler(-1);
  }

  return info;
}


/** Returns info > 0 if the matrix is singular */
int qr_rsolve(const RealMatrix& q_r, bool transpose, RealMatrix& rhs)
{
  Teuchos::LAPACK<int, Real> la;

  char UPLO = 'U';
  char TRANS = transpose ? 'T' : 'N';
  char DIAG = 'N';
  int N = q_r.numCols();
  int NRHS = rhs.numCols();
  int LDA = q_r.stride();
  int LDB = rhs.stride();
  //int LDB = rhs.numRows();
  int info = 0;

  la.TRTRS(UPLO, TRANS, DIAG, N, NRHS, q_r.values(), LDA, rhs.values(), LDB, 
	   &info);

  if (info < 0) {
    Cerr << "Error (qr_rsolve): the " << -info << "-th argument had an illegal "
	 << "value.";
    abort_handler(-1);
  }
  return info;
}


double det_AtransA(RealMatrix& A)
{
  RealVector sing_vals;
  singular_values(A, sing_vals);
  double det = 1.0;
  for (int i=0; i<sing_vals.length(); ++i)
    det *= sing_vals[i] * sing_vals[i];

  return det;
}

}  // namespace Dakota
