/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_global_defs.hpp"
#include "dakota_data_util.hpp"
#include "dakota_linear_algebra.hpp"
#include "Teuchos_LAPACK.hpp"

namespace Dakota {

void singular_value_decomp(RealMatrix& matrix, RealVector& singular_vals,
			   RealMatrix& v_trans, bool compute_vectors)
{
  Teuchos::LAPACK<int, Real> la;

  // ----
  // compute the SVD of the incoming matrix
  // ----

  char JOBU  = 'N';
  char JOBVT = 'N';
  if (compute_vectors) {
    JOBU  = 'O'; // overwrite A with U
    JOBVT = 'A'; // compute all singular vectors VT
  }
  int M(matrix.numRows()), N(matrix.numCols()), LDA = matrix.stride(),
    num_singular_values = std::min(M, N);
  singular_vals.resize(num_singular_values);
  Real* U = NULL;
  int LDU = 1, LDVT = 1;
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
    Cerr << "\nError: singular_value_decomp() failed. " << "The "
	 << std::abs(info) << "-th argument had an illegal value." << std::endl;
    abort_handler(-1);
  }
  if (info > 0) {
    Cerr << "\nError: singular_value_decomp() failed. " << info
	 << " superdiagonals of an intermediate bidiagonal form B did not "
	 << "converge to 0." << std::endl;
    abort_handler(-1);
  }
}


void singular_values(RealMatrix& matrix, RealVector& singular_vals)
{
  // empty matrix with NULL .values()
  RealMatrix v_trans;
  singular_value_decomp(matrix, singular_vals, v_trans, false);
}


void pseudo_inverse(RealMatrix& A, RealMatrix& A_inv, Real& rcond)
{
  // TO DO: accept A as const and allocate separately for U
  // (overload singular_value_decomp() above)

  RealVector Sigma;  RealMatrix V_T;
  singular_value_decomp(A, Sigma, V_T, true); // U overwrites A
  //Cout << "Singular values:\n" << Sigma << std::endl;

  // Form inverse A^{-1} = V S^{-1} U^T or pseudo-inverse A* = V S* U^T:
  Real s_tol = 1.e-12; // halfway between DBL_EPSILON, sqrt(DBL_EPSILON)
  size_t r, c, n = Sigma.length(); // min(M, N)
  RealMatrix Sinv_U_T(n, n); // init to 0
  Real s_ref = Sigma[0]; // largest singular value
  if (s_ref <= 0.) {
    Cerr << "Error: no positive singular values in pseudo_inverse()."
	 << std::endl;
    abort_handler(-1);
  }
  rcond = Sigma[n-1] / s_ref; // inverse condition number < 1
  for (r=0; r<n; ++r) {
    Real s_val = Sigma[r], s_ratio = s_val / s_ref; // smallest ratio is rcond
    Real*  A_r = A[r]; // col vector of U^T
    if (s_ratio > s_tol) // else truncate
      for (c=0; c<n; ++c)
	Sinv_U_T(r,c) = A_r[c] / s_val; // A(c,r) / s_val
  }
  // (pseudo-)inverse = V scaled_UT
  A_inv.shape(n,n);
  A_inv.multiply(Teuchos::TRANS, Teuchos::NO_TRANS, 1., V_T, Sinv_U_T, 0.);
}


void pseudo_inverse(const RealSymMatrix& A, RealMatrix& A_inv, Real& rcond)
{
  RealMatrix A_rm; copy_data(A, A_rm); // RealSymMatrix to RealMatrix
  pseudo_inverse(A_rm, A_inv, rcond);
}


void pseudo_inverse(const RealSymMatrix& A, RealSymMatrix& A_inv, Real& rcond)
{
  RealMatrix A_rm, A_rm_inv;
  copy_data(A, A_rm);         // RealSymMatrix to RealMatrix
  pseudo_inverse(A_rm, A_rm_inv, rcond);
  copy_data(A_rm_inv, A_inv); // RealMatrix to RealSymMatrix
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


void symmetric_eigenvalue_decomposition( const RealSymMatrix &matrix, 
					 RealVector &eigenvalues, 
					 RealMatrix &eigenvectors )
{
  Teuchos::LAPACK<int, Real> la;

  int N( matrix.numRows() );
  eigenvectors.shapeUninitialized( N, N );
  //eigenvectors.assign( matrix );
  for ( int j=0; j<N; j++)
    for ( int i=0; i<=j; i++)
      eigenvectors(i,j) = matrix( i,j );

  char jobz = 'V'; // compute eigenvectors
  char uplo = 'U'; // assume only upper triangular part of matrix is stored

  eigenvalues.sizeUninitialized( N );

  int info;        // Teuchos::LAPACK output flag
  RealVector work; // Teuchos::LAPACK work array;
  int lwork = -1;  // Size of Teuchos::LAPACK work array
  
  // Compute optimal size of work array
  work.sizeUninitialized( 1 ); // temporary work array
  la.SYEV( jobz, uplo, N, eigenvectors.values(), eigenvectors.stride(), 
	   eigenvalues.values(), work.values(), lwork, &info );

  lwork = (int)work[0];
  work.sizeUninitialized( lwork );
  
  la.SYEV( jobz, uplo, N, eigenvectors.values(), eigenvectors.stride(), 
	   eigenvalues.values(), work.values(), lwork, &info );

  if ( info > 0 )
    {
      std::stringstream msg;
      msg << "The algorithm failed to converge." << info
	  << " off-diagonal elements of an intermediate tridiagonal "
	  << "form did not converge to zero.";
      throw( std::runtime_error( msg.str() ) );
    }
  else if ( info < 0 )
    {
      std::stringstream msg;
      msg << " The " << std::abs( info ) << " argument had an illegal value.";
      throw( std::runtime_error( msg.str() ) );
    }
};

}  // namespace Dakota
