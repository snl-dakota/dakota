/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

/*! \file
    \brief Dakota linear algebra utilities

    Convenience functions to perform Teuchos::LAPACK operations on
    Dakota RealMatrix/RealVector
*/

#ifndef DAKOTA_LINEAR_ALGEBRA_H
#define DAKOTA_LINEAR_ALGEBRA_H

#include "dakota_data_types.hpp"

/* Functions to consider integrating here:

DakotaLeastSq:  
  la.GEQRF(M,N,Jmatrix,LDA,tau,work,N,&info);
  la.TRTRI(uplo, unitdiag, N, Jmatrix, LDA, &info); 
 
NonDLocalReliability:
  la.SYEV('N', A.UPLO(), num_kappa, A.values(), A.stride(), kappa_u.values(),
          work, lwork, &info);

ActiveSubspaceModel
  teuchos_blas.GEMV(Teuchos::TRANS, m, n, alpha, reducedBasis.values(), m, 
  		    mu_x.values(), incx, beta, mu_xi.values(), incy);

ExpDataUtils:
  la.TRTRI( covCholFactor_.UPLO(), 'N', 
  	    numDOF_, cholFactorInv_.values(), 
	    cholFactorInv_.stride(), &info );

  la.SYEV( jobz, uplo, N, eigenvectors.values(), eigenvectors.stride(), 
	   eigenvalues.values(), work.values(), lwork, &info );

*/


namespace Dakota {

/**
 * \brief Compute the SVD of an arbitrary matrix A = USV^T

   Uses Teuchos::LAPACK.GESVD() to compute the singular value
   decomposition, overwriting A with the left singular vectors U (or
   destroying A if compute_vectors = false); optionally returns right
   singular vectors in v_trans.
 */
void singular_value_decomp(RealMatrix& matrix, RealVector& singular_vals,
			   RealMatrix& v_trans, bool compute_vectors = true);

/// compute the singular values without storing any singular vectors
/// (A will be destroyed)
void singular_values(RealMatrix& matrix, RealVector& singular_values);

/// compute the pseudo-inverse by SVD (A is overwritten by U)
void pseudo_inverse(RealMatrix& A, RealMatrix& A_inv, Real& rcond);
/// compute the pseudo-inverse for a symmetric matrix by SVD
void pseudo_inverse(const RealSymMatrix& A, RealMatrix& A_inv, Real& rcond);
/// compute the pseudo-inverse for a symmetric matrix by SVD
void pseudo_inverse(const RealSymMatrix& A, RealSymMatrix& A_inv, Real& rcond);

/// Perform a Cholesky factorization and solve; propagate LAPACK return codes
int cholesky_solve(RealSymMatrix& A, RealMatrix& X, RealMatrix& B,
		   bool copy_A = false, bool copy_B = false,
		   bool hard_error = true);

/**
 * \brief Compute an in-place QR factorization A = QR

   Uses Teuchos::LAPACK.GEQRF() to compute the QR decomposition,
   overwriting A with the transformations and R.
 */
int qr(RealMatrix& A);

/**
 * \brief Perform a multiple right-hand sides Rinv * rhs solve using
 * the R from a qr factorization.

   Uses Teuchos::LAPACK.TRTRS() to perform a triangular backsolve
 */
int qr_rsolve(const RealMatrix& q_r, bool transpose, RealMatrix& rhs);

/// Use SVD to compute det(A'*A), destroying A with the SVD
double det_AtransA(RealMatrix& A);

/**
 * \brief Computes the eigenvalues and, optionally, eigenvectors of a
 *  real symmetric matrix A. 
 * 
 * Eigenvalues are returned in ascending order.
 */
void symmetric_eigenvalue_decomposition( const RealSymMatrix &matrix, 
					 RealVector &eigenvalues, 
					 RealMatrix &eigenvectors );

}  // namespace Dakota

#endif  // DAKOTA_LINEAR_ALGEBRA_H
