/* cobyla : contrained optimization by linear approximation */
/* Example driver */

/*
 * Copyright (c) 1992, Michael J. D. Powell (M.J.D.Powell@damtp.cam.ac.uk)
 * Copyright (c) 2004, Jean-Sebastien Roy (js@jeannot.org)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * This software is a C version of COBYLA, contrained optimization by linear
 * approximation package originally developed by Michael J. D. Powell in
 * Fortran.
 * 
 * The original source code can be found at :
 * http://plato.la.asu.edu/topics/problems/nlores.html
 */

static char const rcsid[] =
  "@(#) $Jeannot: example.c,v 1.10 2004/04/17 23:19:15 js Exp $";

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "cobyla.h"

cobyla_function calcfc;

typedef struct 
{
  int nprob;
} example_state;

/* ------------------------------------------------------------------------- */
/* Main program of test problems in Report DAMTP 1992/NA5. */
/* ------------------------------------------------------------------------- */
int main(int argc, char **argv)
{
  /* System generated locals */
  int i__1;
  double d__1;

  /* Local variables */
  double rhoend;
  double temp, xopt[10];
  double x[10], tempa, tempb, tempc, tempd, rhobeg;
  int i__, m = 0, n = 0, icase;
  int maxfun, iprint, rc;
  example_state state;

  for (state.nprob = 1; state.nprob <= 10; ++state.nprob) {
    if (state.nprob == 1) {

/* Minimization of a simple quadratic function of two variables. */

      fprintf(stderr, "\n       Output from test problem 1 (Simple quadratic)\n");
      n = 2;
      m = 0;
      xopt[0] = -1.;
      xopt[1] = 0.;
    } else if (state.nprob == 2) {

/* Easy two dimensional minimization in unit circle. */

      fprintf(stderr, "\n       Output from test problem 2 (2D unit circle calculation)\n");
      n = 2;
      m = 1;
      xopt[0] = sqrt(.5);
      xopt[1] = -xopt[0];
    } else if (state.nprob == 3) {

/* Easy three dimensional minimization in ellipsoid. */

      fprintf(stderr, "\n       Output from test problem 3 (3D ellipsoid calculation)\n");
      n = 3;
      m = 1;
      xopt[0] = 1. / sqrt(3.);
      xopt[1] = 1. / sqrt(6.);
      xopt[2] = -.33333333333333331;
    } else if (state.nprob == 4) {

/* Weak version of Rosenbrock's problem. */

      fprintf(stderr, "\n       Output from test problem 4 (Weak Rosenbrock)\n");
      n = 2;
      m = 0;
      xopt[0] = -1.;
      xopt[1] = 1.;
    } else if (state.nprob == 5) {

/* Intermediate version of Rosenbrock's problem. */

      fprintf(stderr, "\n       Output from test problem 5 (Intermediate Rosenbrock)\n");
      n = 2;
      m = 0;
      xopt[0] = -1.;
      xopt[1] = 1.;
    } else if (state.nprob == 6) {

/* This problem is taken from Fletcher's book Practical Methods of */
/* Optimization and has the equation number (9.1.15). */

      fprintf(stderr, "\n       Output from test problem 6 (Equation (9.1.15) in Fletcher)\n");
      n = 2;
      m = 2;
      xopt[0] = sqrt(.5);
      xopt[1] = xopt[0];
    } else if (state.nprob == 7) {

/* This problem is taken from Fletcher's book Practical Methods of */
/* Optimization and has the equation number (14.4.2). */

      fprintf(stderr, "\n       Output from test problem 7 (Equation (14.4.2) in Fletcher)\n");
      n = 3;
      m = 3;
      xopt[0] = 0.;
      xopt[1] = -3.;
      xopt[2] = -3.;
    } else if (state.nprob == 8) {

/* This problem is taken from page 66 of Hock and Schittkowski's book Test */
/* Examples for Nonlinear Programming Codes. It is their test problem Number */
/* 43, and has the name Rosen-Suzuki. */

      fprintf(stderr, "\n       Output from test problem 8 (Rosen-Suzuki)\n");
      n = 4;
      m = 3;
      xopt[0] = 0.;
      xopt[1] = 1.;
      xopt[2] = 2.;
      xopt[3] = -1.;
    } else if (state.nprob == 9) {

/* This problem is taken from page 111 of Hock and Schittkowski's */
/* book Test Examples for Nonlinear Programming Codes. It is their */
/* test problem Number 100. */

      fprintf(stderr, "\n       Output from test problem 9 (Hock and Schittkowski 100)\n");
      n = 7;
      m = 4;
      xopt[0] = 2.330499;
      xopt[1] = 1.951372;
      xopt[2] = -.4775414;
      xopt[3] = 4.365726;
      xopt[4] = -.624487;
      xopt[5] = 1.038131;
      xopt[6] = 1.594227;
    } else if (state.nprob == 10) {

/* This problem is taken from page 415 of Luenberger's book Applied */
/* Nonlinear Programming. It is to maximize the area of a hexagon of */
/* unit diameter. */

      fprintf(stderr, "\n       Output from test problem 10 (Hexagon area)\n");
      n = 9;
      m = 14;
      for (i__ = 1; i__ <= n; ++i__) {
        xopt[i__] = 0.0;
      }
    }
    for (icase = 1; icase <= 2; ++icase) {
      i__1 = n;
      for (i__ = 1; i__ <= i__1; ++i__) {
        x[i__ - 1] = 1.;
      }
      rhobeg = .5;
      rhoend = 1e-6;
      if (icase == 2) {
        rhoend = 1e-8;
      }
      iprint = 1;
      maxfun = 3500;
      rc = cobyla(n, m, x, rhobeg, rhoend, iprint, &maxfun, 0,calcfc, &state);
      if (state.nprob == 10) {
        tempa = x[0] + x[2] + x[4] + x[6];
        tempb = x[1] + x[3] + x[5] + x[7];
        tempc = .5 / sqrt(tempa * tempa + tempb * tempb);
        tempd = tempc * sqrt(3.);
        xopt[0] = tempd * tempa + tempc * tempb;
        xopt[1] = tempd * tempb - tempc * tempa;
        xopt[2] = tempd * tempa - tempc * tempb;
        xopt[3] = tempd * tempb + tempc * tempa;
        for (i__ = 1; i__ <= 4; ++i__) {
          xopt[i__ + 3] = xopt[i__ - 1];
        }
      }
      temp = 0.;
      i__1 = n;
      for (i__ = 1; i__ <= i__1; ++i__) {
        d__1 = x[i__ - 1] - xopt[i__ - 1];
        temp += d__1 * d__1;
      }
      fprintf(stderr, "\n     Least squares error in variables =    %.6E\n", sqrt(temp));
    }
    fprintf(stderr, "  ------------------------------------------------------------------\n");
  }
  return 0;
} /* main */

/* ------------------------------------------------------------------------- */
int calcfc(int n, int m, double *x, double *f, double *con, void *state_)
{
  example_state *state = (example_state *)state_;
  /* System generated locals */
  double d__1, d__2, d__3, d__4, d__5, d__6, d__7;

  /* Parameter adjustments */
  --con;
  --x;

  /* Function Body */
  if (state->nprob == 1) {

/* Test problem 1 (Simple quadratic) */

    d__1 = x[1] + 1.;
    d__2 = x[2];
    *f = d__1 * d__1 * 10. + d__2 * d__2;
  } else if (state->nprob == 2) {

/*  Test problem 2 (2D unit circle calculation) */

    *f = x[1] * x[2];
    d__1 = x[1];
    d__2 = x[2];
    con[1] = 1. - d__1 * d__1 - d__2 * d__2;
  } else if (state->nprob == 3) {

/* Test problem 3 (3D ellipsoid calculation) */

    *f = x[1] * x[2] * x[3];
    d__1 = x[1];
    d__2 = x[2];
    d__3 = x[3];
    con[1] = 1. - d__1 * d__1 - d__2 * d__2 * 2. - d__3 * d__3 * 3.;
  } else if (state->nprob == 4) {

/* Test problem 4 (Weak Rosenbrock) */

    d__2 = x[1];
    d__1 = d__2 * d__2 - x[2];
    d__3 = x[1] + 1.;
    *f = d__1 * d__1 + d__3 * d__3;
  } else if (state->nprob == 5) {

/* Test problem 5 (Intermediate Rosenbrock) */

    d__2 = x[1];
    d__1 = d__2 * d__2 - x[2];
    d__3 = x[1] + 1.;
    *f = d__1 * d__1 * 10. + d__3 * d__3;
  } else if (state->nprob == 6) {

/* Test problem 6 (Equation (9.1.15) in Fletcher's book) */

    *f = -x[1] - x[2];
    d__1 = x[1];
    con[1] = x[2] - d__1 * d__1;
    d__1 = x[1];
    d__2 = x[2];
    con[2] = 1. - d__1 * d__1 - d__2 * d__2;
  } else if (state->nprob == 7) {

/* Test problem 7 (Equation (14.4.2) in Fletcher's book) */

    *f = x[3];
    con[1] = x[1] * 5. - x[2] + x[3];
    d__1 = x[1];
    d__2 = x[2];
    con[2] = x[3] - d__1 * d__1 - d__2 * d__2 - x[2] * 4.;
    con[3] = x[3] - x[1] * 5. - x[2];
  } else if (state->nprob == 8) {

/* Test problem 8 (Rosen-Suzuki) */

    d__1 = x[1];
    d__2 = x[2];
    d__3 = x[3];
    d__4 = x[4];
    *f = d__1 * d__1 + d__2 * d__2 + d__3 * d__3 * 2.f + d__4 * d__4 - x[
        1] * 5. - x[2] * 5. - x[3] * 21. + x[4] * 7.;
    d__1 = x[1];
    d__2 = x[2];
    d__3 = x[3];
    d__4 = x[4];
    con[1] = 8. - d__1 * d__1 - d__2 * d__2 - d__3 * d__3 - d__4 * d__4 - 
        x[1] + x[2] - x[3] + x[4];
    d__1 = x[1];
    d__2 = x[2];
    d__3 = x[3];
    d__4 = x[4];
    con[2] = 10. - d__1 * d__1 - d__2 * d__2 * 2. - d__3 * d__3 - d__4 * 
        d__4 * 2. + x[1] + x[4];
    d__1 = x[1];
    d__2 = x[2];
    d__3 = x[3];
    con[3] = 5. - d__1 * d__1 * 2.f - d__2 * d__2 - d__3 * d__3 - x[1] * 
        2. + x[2] + x[4];
  } else if (state->nprob == 9) {

/* Test problem 9 (Hock and Schittkowski 100) */

    d__1 = x[1] - 10.;
    d__2 = x[2] - 12.;
/* Computing 4th power */
    d__3 = x[3], d__3 *= d__3;
    d__4 = x[4] - 11.;
/* Computing 6th power */
    d__5 = x[5], d__5 *= d__5;
    d__6 = x[6];
/* Computing 4th power */
    d__7 = x[7], d__7 *= d__7;
    *f = d__1 * d__1 + d__2 * d__2 * 5. + d__3 * d__3 + d__4 * d__4 * 3. 
        + d__5 * (d__5 * d__5) * 10. + d__6 * d__6 * 7. + d__7 * d__7 
        - x[6] * 4. * x[7] - x[6] * 10. - x[7] * 8.;
    d__1 = x[1];
/* Computing 4th power */
    d__2 = x[2], d__2 *= d__2;
    d__3 = x[4];
    con[1] = 127. - d__1 * d__1 * 2. - d__2 * d__2 * 3. - x[3] - d__3 * 
        d__3 * 4. - x[5] * 5.;
    d__1 = x[3];
    con[2] = 282. - x[1] * 7. - x[2] * 3. - d__1 * d__1 * 10. - x[4] + x[
        5];
    d__1 = x[2];
    d__2 = x[6];
    con[3] = 196. - x[1] * 23. - d__1 * d__1 - d__2 * d__2 * 6. + x[7] * 
        8.;
    d__1 = x[1];
    d__2 = x[2];
    d__3 = x[3];
    con[4] = d__1 * d__1 * -4. - d__2 * d__2 + x[1] * 3. * x[2] - d__3 * 
        d__3 * 2. - x[6] * 5. + x[7] * 11.;
  } else if (state->nprob == 10) {

/* Test problem 10 (Hexagon area) */

    *f = (x[1] * x[4] - x[2] * x[3] + x[3] * x[9] - x[5] * x[9] + x[5] * 
        x[8] - x[6] * x[7]) * -.5;
    d__1 = x[3];
    d__2 = x[4];
    con[1] = 1. - d__1 * d__1 - d__2 * d__2;
    d__1 = x[9];
    con[2] = 1. - d__1 * d__1;
    d__1 = x[5];
    d__2 = x[6];
    con[3] = 1. - d__1 * d__1 - d__2 * d__2;
    d__1 = x[1];
    d__2 = x[2] - x[9];
    con[4] = 1. - d__1 * d__1 - d__2 * d__2;
    d__1 = x[1] - x[5];
    d__2 = x[2] - x[6];
    con[5] = 1. - d__1 * d__1 - d__2 * d__2;
    d__1 = x[1] - x[7];
    d__2 = x[2] - x[8];
    con[6] = 1. - d__1 * d__1 - d__2 * d__2;
    d__1 = x[3] - x[5];
    d__2 = x[4] - x[6];
    con[7] = 1. - d__1 * d__1 - d__2 * d__2;
    d__1 = x[3] - x[7];
    d__2 = x[4] - x[8];
    con[8] = 1. - d__1 * d__1 - d__2 * d__2;
    d__1 = x[7];
    d__2 = x[8] - x[9];
    con[9] = 1. - d__1 * d__1 - d__2 * d__2;
    con[10] = x[1] * x[4] - x[2] * x[3];
    con[11] = x[3] * x[9];
    con[12] = -x[5] * x[9];
    con[13] = x[5] * x[8] - x[6] * x[7];
    con[14] = x[9];
  }
  return 0;
} /* calcfc */
