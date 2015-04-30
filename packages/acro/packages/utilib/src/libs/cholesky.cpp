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

//
// cholesky.cpp
//
//
// This code assumes that A is symmetric positive definite.  It generates
//	a lower triangular G s.t. A = G*G'.
//
// This code assumes that the memory in G has been allocated contiguously.
//

#include <utilib_config.h>
#include <utilib/_generic.h>
#include <utilib/_math.h>
#include <utilib/linpack.h>

using namespace std;

namespace utilib {

#ifdef UTILIB_YES_LINPACK

/* Note:  This code only utilizes the lower triangular parts of A and G.  
	The user can call this routine with the same matrix for G as for A
	as long as the memory for A has been allocated contiguously.
*/
 
int cholesky(DoubleMatrix& A, DoubleMatrix& G, int n, double* rcond)
{
int info;
int i,j;

if (A != G) 
   for (i=0; i<n; i++)
     for (j=0; j<=i; j++)
       G[i][j] = A[i][j];

if (rcond != NULL) {
   DoubleVector Z(n);

   dpoco_(&(G[0][0]), &n, &n, rcond, Z.data(), &info);
   }
else
   dpofa_(&(G[0][0]), &n, &n, &info);

if (info != 0) return ERR;
return OK;
}


#else


/* Note:  This code only utilizes the upper triangular part of G.  The user
	can call this routine with the same matrix for G as for A.
 
 * Developed using pseudo-code in "Matrix Computations", Golub and van Loan
 */


int cholesky(DoubleMatrix& A, DoubleMatrix& G, size_type n)
{
size_type k, i;

for (k=0; k<n; k++) {
  {double tmp=0.0;
   size_type p;
   for (p=0; p<(k-1); p++)
     tmp += A[k][p] * A[k][p];
   G[k][k] = sqrt( A[k][k] - tmp );
  }

  for (i=k; i<n; i++) {
    double tmp=0.0;
    size_type p;
    for (p=0; p<(k-1); p++)
      tmp += A[i][p] * A[k][p];
    G[i][k] = (A[i][k] - tmp) / A[k][k];
    }
  }
return 0;
}

} // namespace utilib

#endif


/**  Stuff for testing the code.


static double staticA[5][5] = {	{ 4,  6,   8,  10,  12},
				{ 6, 25,  32,  39,  46},
				{ 8, 32,  77,  92, 107},
				{10, 39,  92, 174, 200},
				{12, 46, 107, 200, 330}};

main()
{
int i,j, status;
double** A;
double rcond;

A = dmatrix(0,4,0,4);

for (i=0; i<5; i++) {
  for (j=0; j<5; j++) {
    A[i][j] = staticA[i][j];
    fprintf(stdout, "%lf ", A[i][j]);
  }
  fprintf(stdout,"\n");
  }

status = cholesky(A,A,5,&rcond);

fprintf(stdout,"\n");
for (i=0; i<5; i++) {
  for (j=0; j<5; j++) {
    fprintf(stdout, "%lf ", A[i][j]);
    }
  fprintf(stdout,"\n");
  }

fprintf(stdout,"\nstatus %d\trcond %lf\n", status, rcond);
}

**/
