/* $Id: x21c.c 3186 2006-02-15 18:17:33Z slbrow $
	Grid data demo

   Copyright (C) 2004  Joao Cardoso

   This file is part of PLplot.

   PLplot is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Library Public License as published
   by the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   PLplot is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with PLplot; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

*/

#include "plcdemos.h"
#include <time.h>

#if !defined(HAVE_ISNAN)
#  define isnan(x) ((x) != (x))
#endif

/* Options data structure definition. */

static PLINT pts = 500;
static PLINT xp = 25;
static PLINT yp = 20;
static PLINT nl = 15;
static int knn_order = 20;
static PLFLT threshold = 1.001;
static PLFLT wmin = -1e3;
static int randn = 0;
static int rosen = 0;

static PLOptionTable options[] = {
  {
    "npts",
    NULL,
    NULL,
    &pts,
    PL_OPT_INT,
    "-npts points",
    "Specify number of random points to generate [500]" },
  {
    "randn",
    NULL,
    NULL,
    &randn,
    PL_OPT_BOOL,
    "-randn",
    "Normal instead of uniform sampling -- the effective \n\
\t\t\t  number of points will be smaller than the specified." },
  {
    "rosen",
    NULL,
    NULL,
    &rosen,
    PL_OPT_BOOL,
    "-rosen",
    "Generate points from the Rosenbrock function."},
  {
    "nx",
    NULL,
    NULL,
    &xp,
    PL_OPT_INT,
    "-nx points",
    "Specify grid x dimension [25]" },
  {
    "ny",
    NULL,
    NULL,
    &yp,
    PL_OPT_INT,
    "-ny points",
    "Specify grid y dimension [20]" },
  {
    "nlevel",
    NULL,
    NULL,
    &nl,
    PL_OPT_INT,
    "-nlevel ",
    "Specify number of contour levels [15]" },
  {
    "knn_order",
    NULL,
    NULL,
    &knn_order,
    PL_OPT_INT,
    "-knn_order order",
    "Specify the number of neighbors [20]" },
  {
    "threshold",
    NULL,
    NULL,
    &threshold,
    PL_OPT_FLOAT,
    "-threshold float",
    "Specify what a thin triangle is [1. < [1.001] < 2.]" },

  {
    NULL,			/* option */
    NULL,			/* handler */
    NULL,			/* client data */
    NULL,			/* address of variable to set */
    0,				/* mode flag */
    NULL,			/* short syntax */
    NULL }			/* long syntax */
};

void create_data(PLFLT **xi, PLFLT **yi, PLFLT **zi, int pts);
void free_data(PLFLT *x, PLFLT *y, PLFLT *z);
void create_grid(PLFLT **xi, int px, PLFLT **yi, int py);
void free_grid(PLFLT *x, PLFLT *y);

static void
cmap1_init()
{
  PLFLT i[2], h[2], l[2], s[2];

  i[0] = 0.0;		/* left boundary */
  i[1] = 1.0;		/* right boundary */

  h[0] = 240; /* blue -> green -> yellow -> */
  h[1] = 0;   /* -> red */

  l[0] = 0.6;
  l[1] = 0.6;

  s[0] = 0.8;
  s[1] = 0.8;

  plscmap1n(256);
  c_plscmap1l(0, 2, i, h, l, s, NULL);
}

PLFLT xm, xM, ym, yM;

int
main(int argc, char *argv[])
{
  PLFLT *x, *y, *z, *clev;
  PLFLT *xg, *yg, **zg, **szg;
  PLFLT zmin, zmax, lzm, lzM;
  long ct;
  int i, j, k;
  PLINT alg;
  char ylab[40], xlab[40];
  char *title[] = {"Cubic Spline Approximation",
		   "Delaunay Linear Interpolation",
		   "Natural Neighbors Interpolation",
		   "KNN Inv. Distance Weighted",
		   "3NN Linear Interpolation",
		   "4NN Around Inv. Dist. Weighted"};

  PLFLT opt[] = {0., 0., 0., 0., 0., 0.};

  xm = ym = -0.2;
  xM = yM = 0.8;

  plMergeOpts(options, "x21c options", NULL);
  plparseopts(&argc, argv, PL_PARSE_FULL);

  opt[2] = wmin;
  opt[3] = (PLFLT) knn_order;
  opt[4] = threshold;

  /* Initialize plplot */

  plinit();

  create_data(&x, &y, &z, pts); /* the sampled data */
  zmin = z[0];
  zmax = z[0];
  for (i=1; i<pts; i++) {
    if (z[i] > zmax)
      zmax = z[i];
    if (z[i] < zmin)
      zmin = z[i];
  }

  create_grid(&xg, xp, &yg, yp); /* grid the data at */
  plAlloc2dGrid(&zg, xp, yp); /* the output grided data */
  clev = (PLFLT *) malloc(nl * sizeof(PLFLT));

  sprintf(xlab, "Npts=%d gridx=%d gridy=%d", pts, xp, yp);
  plcol0(1);
  plenv(xm, xM, ym, yM, 2, 0);
  plcol0(15);
  pllab(xlab, "", "The original data");
  plcol0(2);
  plpoin(pts, x, y, 5);
  pladv(0);

  plssub(3,2);

  for (k=0; k<2; k++) {
    pladv(0);
    for (alg=1; alg<7; alg++) {

      ct = clock();
      plgriddata(x, y, z, pts, xg, xp, yg, yp, zg, alg, opt[alg-1]);
      sprintf(xlab, "time=%d ms", (clock() - ct)/1000);
      sprintf(ylab, "opt=%.3f", opt[alg-1]);

      /* - CSA can generate NaNs (only interpolates?!).
       * - DTLI and NNI can generate NaNs for points outside the convex hull
       *      of the data points.
       * - NNLI can generate NaNs if a sufficiently thick triangle is not found
       *
       * PLplot should be NaN/Inf aware, but changing it now is quite a job...
       * so, instead of not plotting the NaN regions, a weighted average over
       * the neighbors is done.
       */

      if (alg == GRID_CSA || alg == GRID_DTLI || alg == GRID_NNLI || alg == GRID_NNI) {
	int ii, jj;
	PLFLT dist, d;

	for (i=0; i<xp; i++) {
	  for (j=0; j<yp; j++) {
	    if (isnan(zg[i][j])) { /* average (IDW) over the 8 neighbors */

	      zg[i][j] = 0.; dist = 0.;

	      for (ii=i-1; ii<=i+1 && ii<xp; ii++) {
		for (jj=j-1; jj<=j+1 && jj<yp; jj++) {
		  if (ii >= 0 && jj >= 0 && !isnan(zg[ii][jj])) {
		    d = (abs(ii-i) + abs(jj-j)) == 1 ? 1. : 1.4142;
		    zg[i][j] += zg[ii][jj]/(d*d);
		    dist += d;
		  }
		}
	      }
	      if (dist != 0.)
		zg[i][j] /= dist;
	      else
		zg[i][j] = zmin;

	    }
	  }
	}
      }

      plMinMax2dGrid(zg, xp, yp, &lzM, &lzm);

      plcol0(1);
      pladv(alg);

      if (k == 0) {

	lzm = MIN(lzm, zmin);
	lzM = MAX(lzM, zmax);
	for (i=0; i<nl; i++)
	  clev[i] = lzm + (lzM-lzm)/(nl-1)*i;

	plenv0(xm, xM, ym, yM, 2, 0);
	plcol0(15);
	pllab(xlab, ylab, title[alg-1]);
	plshades(zg, xp, yp, NULL, xm, xM, ym, yM,
		 clev, nl, 1, 0, 1, plfill, 1, NULL, NULL);
	plcol0(2);
      } else {

	for (i=0; i<nl; i++)
	  clev[i] = lzm + (lzM-lzm)/(nl-1)*i;

	cmap1_init();
	plvpor(0.0, 1.0, 0.0, 0.9);
	plwind(-1.0, 1.0, -1.0, 1.5);
	/*
	 * For the comparition to be fair, all plots should have the
	 * same z values, but to get the max/min of the data generated
	 * by all algorithms would imply two passes. Keep it simple.
	 *
	 * plw3d(1., 1., 1., xm, xM, ym, yM, zmin, zmax, 30, -60);
	 */

	plw3d(1., 1., 1., xm, xM, ym, yM, lzm, lzM, 30, -60);
	plbox3("bnstu", ylab, 0.0, 0,
	       "bnstu", xlab, 0.0, 0,
	       "bcdmnstuv", "", 0.0, 4);
	plcol0(15);
	pllab("", "", title[alg-1]);
	plot3dc(xg, yg, zg, xp, yp, DRAW_LINEXY | MAG_COLOR | BASE_CONT, clev, nl);
      }
    }
  }

  plend();

  free_data(x, y, z);
  free_grid(xg, yg);
  free((void *)clev);
  plFree2dGrid(zg, xp, yp);
}


void
create_grid(PLFLT **xi, int px, PLFLT **yi, int py)
{

  PLFLT *x, *y;
  int i;

  x = *xi = (PLFLT *) malloc(px * sizeof(PLFLT));
  y = *yi = (PLFLT *) malloc(py * sizeof(PLFLT));

  for (i=0; i<px; i++)
    *x++ = xm + (xM-xm)*i/(px-1.);

  for (i=0; i<py; i++)
    *y++ = ym + (yM-ym)*i/(py-1.);
}

void
free_grid(PLFLT *xi, PLFLT *yi)
{
  free((void *)xi);
  free((void *)yi);
}

void
create_data(PLFLT **xi, PLFLT **yi, PLFLT **zi, int pts)
{
  int i;
  PLFLT *x, *y, *z, r;
  PLFLT xt, yt;

  *xi = x = (PLFLT *) malloc(pts * sizeof(PLFLT));
  *yi = y = (PLFLT *) malloc(pts * sizeof(PLFLT));
  *zi = z = (PLFLT *) malloc(pts * sizeof(PLFLT));

  for(i=0; i<pts; i++) {
    xt = drand48();
    yt = drand48();
    if (!randn) {
      *x = xt + xm;
      *y = yt + ym;
    } else { /* std=1, meaning that many points are outside the plot range */
      *x = sqrt(-2.*log(xt)) * cos(2.*PI*yt) + xm;
      *y = sqrt(-2.*log(xt)) * sin(2.*PI*yt) + ym;
    }
    if (!rosen) {
      r = sqrt((*x) * (*x) + (*y) * (*y));
      *z = exp(-r * r) * cos(2.0 * PI * r);
    } else {
      *z = log(pow(1. - *x, 2.) + 100. * pow(*y - pow(*x, 2.), 2.));
    }
    x++; y++; z++;
  }
}

void
free_data(PLFLT *x, PLFLT *y, PLFLT *z)
{
  free((void *)x);
  free((void *)y);
  free((void *)z);
}
