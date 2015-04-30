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

/* cov.c
 *
 * Routine to compute the covariance of a matrix
 */

int covariance(double **X, int m, int n, double **COV);

int covariance(double **X, int m, int n, double **COV)
{
int i,j,k,l;
double *temp_vec;

temp_vec = COV[0];

/* Compute the Means */
for (i=0; i<n; i++) {
  temp_vec[i] = 0.0;
  for (j=0; j<m; j++)
    temp_vec[i] += X[j][i];
  temp_vec[i] /= ((double) m);
  }

/* Subtract out the means */
for (i=0; i<n; i++)
  for (j=0; j<m; j++)
    X[j][i] -= temp_vec[i];

/* Compute E[(X1 - mu1)*(X2 - mu2)]  */
for (k=0; k<n; k++)
  for (l=0; l<=k; l++) {
    COV[k][l] = 0.0;
    for (j=0; j<m; j++)
      COV[k][l] += X[j][k]*X[j][l];
    COV[k][l] /= ((double) (m-1));
    COV[l][k] = COV[k][l];
  }

return 0;
}
