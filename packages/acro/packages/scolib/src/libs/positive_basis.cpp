/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

//
// Coliny_positive_basis.cpp
//
// This code is adapted from R.M. Lewis' code.  The modifications
// involved (a) using the BasicArray class in SGOPT, (b) reordering the
// array index (to be in a C format), and (c) avoiding some fortran-isms.
//

/*
 * Copyright (C) 1998 Robert Michael Lewis.  All Rights Reserved.
 *
 * The development of this software was supported by the National
 * Aeronautics and Space Administration under NASA Contracts No.
 * NAS1-19480 and NAS1-97046 while the author was in residence at 
 * the Institute for Computer Applications in Science and Engineering
 * (ICASE), NASA Langley Research Center, Hampton, VA 23681-2199.
 */

#include <acro_config.h>
#include <utilib/NumArray.h>

using namespace utilib;
using namespace std;

namespace scolib {

namespace {

void gps_regular_simplex (int n, BasicArray<NumArray<double> >& S, double edge_length)
{
  /**********************************************************************
   *
   * This procedure constructs a regular simplex, given an initial
   * point and the edge length for the initial simplex.
   *
   * Arguments:
   *   n -> The dimension of the domain.
   *
   *   S <-> The regular simplex, represented as an  (n+1) x n  matrix.
   *
   *     On entry, the first row of S is a vertex about which
   *     to construct the simplex.
   *
   *     On return, the rows of S are the vertices of a regular
   *     simplex, with the first rows unchanged.
   *
   *   edge_length -> The desired edge length.
   */

  int i, j;
  double p, q, r;

  /*****************************************************************
   * The formula for computing the vertices of a regular simplex
   * is taken from
   *
   *   Sequential Application of Simplex Designs in Optimisation
   *   and Evolutionary Operation, by W. Spendley, G. R. Hext,
   *   and F. R. Himsworth, Technometrics, November 1962, vol. 4,
   *   no. 4, 441--461.
   *
   * Calculate the auxiliary values 'p' and 'q' as follows:
   *
   *                  SQRT(n + 1) - 1 + n
   *           p  =  --------------------- * edge_length
   *                    n * SQRT(2)
   *
   *                  SQRT(n + 1) - 1
   *           q  =  ----------------- * edge_length
   *                    n * SQRT(2)
   *
   * The first row of S contains the point from which the rest of 
   * the simplex is computed.
   *
   * The coordinates of the vertices of the simplex are computed as 
   * follows:
   *
   *    S(*,1)   = (S(1,1),     S(2,1),     S(3,1),     ..., S(n,1))^{T}
   *    S(*,2)   = (S(1,1) + p, S(2,1) + q, S(3,1) + q, ..., S(n,1) + q)^{T}
   *             .
   *             .
   *             .
   *    S(*,n+1) = (S(1,1) + q, S(2,1) + q, S(3,1) + q, ..., S(n,1) + p)^{T}
   *
   * (where the indexing above follows the Fortran convention).
   */

  r = n + 1.0;
  q = ((sqrt(r) - 1.0) / (n * sqrt(2.0))) * edge_length;
  p = q + ((1.0 / sqrt(2.0)) * edge_length);

  for (j = 1; j < n+1; j++) {     /* Loop over row of S (vertices). */
    for (i = 0; i < j-1; i++)
      S[j][i] = S[0][i] + q;
    S[j][j-1] = S[0][j-1] + p;
    for (i = j; i < n; i++)
      S[j][i] = S[0][i] + q;
  }

  return;
}

} // empty namespace


void gps_minimal_positive_basis (int n, BasicArray<NumArray<double> >& P)
{

  /*******************************************************************
   *
   * A minimal standard positive basis consists of n+1 vectors.
   * This procedure computes a minimal basis by first constructing 
   * a regular simplex, and then taking as the positive basis the set
   * of directions from the centroid of this simplex to the vertices.
   *
   * The basis so constructed has the attractive feature that all the
   * angles between search directions are the same.  The positive basis
   * vectors returned by this procedure have length one.
   *
   * As with any positive basis, one can multiply the minimal 
   * positive basis returned by this procedure by any n x n 
   * invertible matrix to obtain another positive basis.  Such a
   * basis will also be minimal in size.
   *
   * Arguments:
   *   n -> The dimension of the domain.
   * 
   *   P <- A minimal positive basis.
   *
   *   These basis vectors are scaled to have unit length.
   */

  int i, j;
  double sum;
  double edge_length = 1.0;
  double length;

  P.resize(n+1);
  for (int k=0; k<n+1; k++)
    P[k].resize(n);

  /*
   * Construct a regular simplex with one vertex at the origin.
   */

  for (i = 0; i < n; i++) {
    P[0][i] = 0.0;
  }

  gps_regular_simplex(n, P, edge_length);

  /*
   * Compute the centroid of the simplex.  We overwrite the first column 
   * of S, which we know to be a column of zeros.
   */

  for (i = 0; i < n; i++) {
    for (j = 1, sum = 0.0; j < n+1; j++) {
      sum += P[j][i];
    }

    P[0][i] = sum/(n+1);
  }

  /*
   * Now compute the directions from the centroid to the vertices.
   * In terms of the current entries in S, these are
   *    S(:,j) - S(:,0), j = 1, ..., n
   *          -S(:,0),   j = 0.
   */

  for (j = 1; j < n+1; j++) {
    for (i = 0; i < n; i++) {
      P[j][i] -= P[0][i];
    }
  }

  for (i = 0; i < n; i++) {
    P[0][i] = -P[0][i];
  }

  /*
   * At this point each vector has length = sqrt( n / (2*(n+1)) ).
   * Now normalize the lengths of the vectors that determine the 
   * pattern.
   */

  length = sqrt( ((double) n)/((double) (2*(n+1))) );
  for (j = 0; j < n+1; j++) {
    for (i = 0; i < n; i++) {
      P[j][i] /= length;
    }
  }
  
  return;
}


} // namespace scolib
