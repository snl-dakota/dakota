/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

/**
 * \file linpack.h
 *
 * Declarations for some of linpack's fortran routines.
 * \note Note actively used.
 */

#include <utilib_config.h>
#include <utilib/_generic.h>

#if !defined(DOXYGEN)

#ifdef __cplusplus
extern "C" {
#endif

	
/* Chapter 3 */

void dpofa_ _((double* A, int* lda, int* n, int* info));
void dpoco_ _((double* A, int* lda, int* n, double* rcond, double* Z, int* info));

/* A[lda, n]	-	Contains the symmetrix matrix whose factorization
				is to be computed.
   
   rcond	-	an estimate of the reciprocal condition.
   Z[n]		-	work space
   info		-	error conditions:
				0	A positive definite and successfully
						factored
				K>0	Leading submatrix of order K is not
						positive definite.
*/


/* Chapter # */
void dqrdc_ _((double* x, int* ldx, int* n, int* p, double* qraux,
		int* jpvt, double* work, int* job));

/* x[n,p]	-	matrix whose QR decomposition is computed
   jpvt[p]	-	array to control pivotting
   work[p]	-	work array (not referenced if job=0) 
   job		-	integer which initiates column pivotting.  If
				job=0, pivoting is not done.

   qraux[p]	-	contains info for recovering the orthogonal part
				of the decomposition
*/

void dqrsl_ _((double* x, int* ldx, int* n, int* k, double* qraux,
		double* y, double* qy, double* qty, double* b,
		double* rsd, double* xb, int* job, int* info));

/* x[n,p]	-	the output array from dqrdc
   qraux[p]	-	output from dqrdc
   y[n]		-	??
   job		-	determines what is to be computed
				a != 0		qy computed
				b,c,d,or e !=0	qty computed
				c != 0		b computed
				d != 0		rsd computed
				e != 0		xb computed
   
   qy[n]	-	contains Qy
   qty[n]	-	contains Q'y
   b[p]		-	contains soln to ls problem |y - X_kb|
   rsd[n]	-	contains residual vector y - X_kb
   xb[n]	-	contains least squares approximation to Xb
   information	-	zero unless requested to compute B, and R is
				exactly singular.  In that case, INFO is the 
				index of the first zero diagonal elt of R.
*/

/* Routine from LinPack p. 9.11 
   The params for this command are explained above.  The dqrst code
   automatically transforms the X matrix to column major order,
   so the user can treat this command like any other C routine. */
void dqrst _((double** x, int ldx, int n, int p, double* y, double tol,
                double* b, double* rsd, int* k, int* jpvt, double* qraux,
                double* work, int trans_flag));

/* Adaptation of dqrst which handles multiple regression problems */
void lsfit1 _((double** x, int ldx, int n, int p, double** y, int nregs,
                double tol, double** b, int* k, int trans_flag));
int lsfit2 _((double** x, int ldx, int n, int p, double** y, int nregs,
                double tol, double** b));
int lsfit2_nr _((double** x, int ldx, int n, int p, double** y, int nregs,
                double tol, double** b, double* w, double** v));

#ifdef __cplusplus
}

#endif
#endif
