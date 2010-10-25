/* $Id: sccont.c 3186 2006-02-15 18:17:33Z slbrow $

	Contour plotter front-ends for Fortran.

   Copyright (C) 2004  Alan W. Irwin

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

#include "plstubs.h"

/*----------------------------------------------------------------------*\
* pltr0f()
*
* Identity transformation for plots from Fortran.
* Only difference from C-language identity function (pltr0) is that the
* Fortran paradigm for array index is used, i.e. starting at 1.
\*----------------------------------------------------------------------*/

void
pltr0f(PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, void *pltr_data)
{
    *tx = x + 1.0;
    *ty = y + 1.0;
}

/*----------------------------------------------------------------------*\
* pltr2f()
*
* Does linear interpolation from doubly dimensioned coord arrays
* (row dominant, i.e. Fortran ordering).
*
* This routine includes lots of checks for out of bounds.  This would
* occur occasionally due to a bug in the contour plotter that is now fixed.
* If an out of bounds coordinate is obtained, the boundary value is provided
* along with a warning.  These checks should stay since no harm is done if
* if everything works correctly.
\*----------------------------------------------------------------------*/

void
pltr2f(PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, void *pltr_data)
{
    PLINT ul, ur, vl, vr;
    PLFLT du, dv;
    PLFLT xll, xlr, xrl, xrr;
    PLFLT yll, ylr, yrl, yrr;
    PLFLT xmin, xmax, ymin, ymax;

    PLcGrid *cgrid = (PLcGrid *) pltr_data;
    PLFLT *xg = cgrid->xg;
    PLFLT *yg = cgrid->yg;
    PLINT nx = cgrid->nx;
    PLINT ny = cgrid->ny;

    ul = (PLINT) x;
    ur = ul + 1;
    du = x - ul;

    vl = (PLINT) y;
    vr = vl + 1;
    dv = y - vl;

    xmin = 0;
    xmax = nx - 1;
    ymin = 0;
    ymax = ny - 1;

    if (x < xmin || x > xmax || y < ymin || y > ymax) {
	plwarn("pltr2f: Invalid coordinates");

	if (x < xmin) {

	    if (y < ymin) {
		*tx = *xg;
		*ty = *yg;
	    }
	    else if (y > ymax) {
		*tx = *(xg + (ny - 1) * nx);
		*ty = *(yg + (ny - 1) * nx);
	    }
	    else {
		ul = 0;
		xll = *(xg + ul + vl * nx);
		yll = *(yg + ul + vl * nx);
		xlr = *(xg + ul + vr * nx);
		ylr = *(yg + ul + vr * nx);

		*tx = xll * (1 - dv) + xlr * (dv);
		*ty = yll * (1 - dv) + ylr * (dv);
	    }
	}
	else if (x > xmax) {

	    if (y < ymin) {
		*tx = *(xg + (nx - 1));
		*ty = *(yg + (nx - 1));
	    }
	    else if (y > ymax) {
		*tx = *(xg + (nx - 1) + (ny - 1) * nx);
		*ty = *(yg + (nx - 1) + (ny - 1) * nx);
	    }
	    else {
		ul = nx - 1;
		xll = *(xg + ul + vl * nx);
		yll = *(yg + ul + vl * nx);
		xlr = *(xg + ul + vr * nx);
		ylr = *(yg + ul + vr * nx);

		*tx = xll * (1 - dv) + xlr * (dv);
		*ty = yll * (1 - dv) + ylr * (dv);
	    }
	}
	else {
	    if (y < ymin) {
		vl = 0;
		xll = *(xg + ul + vl * nx);
		xrl = *(xg + ur + vl * nx);
		yll = *(yg + ul + vl * nx);
		yrl = *(yg + ur + vl * nx);

		*tx = xll * (1 - du) + xrl * (du);
		*ty = yll * (1 - du) + yrl * (du);
	    }
	    else if (y > ymax) {
		vr = ny - 1;
		xlr = *(xg + ul + vr * nx);
		xrr = *(xg + ur + vr * nx);
		ylr = *(yg + ul + vr * nx);
		yrr = *(yg + ur + vr * nx);

		*tx = xlr * (1 - du) + xrr * (du);
		*ty = ylr * (1 - du) + yrr * (du);
	    }
	}
    }

/* Normal case.
   Look up coordinates in row-dominant array.
   Have to handle right boundary specially -- if at the edge, we'd
   better not reference the out of bounds point. */

    else {

	xll = *(xg + ul + vl * nx);
	yll = *(yg + ul + vl * nx);

/* ur is out of bounds */

	if (ur == nx && vr < ny) {

	    xlr = *(xg + ul + vr * nx);
	    ylr = *(yg + ul + vr * nx);

	    *tx = xll * (1 - dv) + xlr * (dv);
	    *ty = yll * (1 - dv) + ylr * (dv);
	}

/* vr is out of bounds */

	else if (ur < nx && vr == ny) {

	    xrl = *(xg + ur + vl * nx);
	    yrl = *(yg + ur + vl * nx);

	    *tx = xll * (1 - du) + xrl * (du);
	    *ty = yll * (1 - du) + yrl * (du);
	}

/* both ur and vr are out of bounds */

	else if (ur == nx && vr == ny) {

	    *tx = xll;
	    *ty = yll;
	}

/* everything in bounds */

	else {

	    xrl = *(xg + ur + vl * nx);
	    xlr = *(xg + ul + vr * nx);
	    xrr = *(xg + ur + vr * nx);

	    yrl = *(yg + ur + vl * nx);
	    ylr = *(yg + ul + vr * nx);
	    yrr = *(yg + ur + vr * nx);
/* INDENT OFF */
	    *tx = xll * (1 - du) * (1 - dv) + xlr * (1 - du) * (dv) +
		  xrl *   (du)   * (1 - dv) + xrr *   (du)   * (dv);

	    *ty = yll * (1 - du) * (1 - dv) + ylr * (1 - du) * (dv) +
		  yrl *   (du)   * (1 - dv) + yrr *   (du)   * (dv);
/* INDENT ON */
	}
    }
}

/*----------------------------------------------------------------------*\
* Contour plotter front-ends.
* These specify the row-dominant function evaluator in the plfcont
* argument list.  NO TRANSPOSE IS NECESSARY.  The routines are as follows:
*
* - plcon0	no transformation
* - plcon1	linear interpolation from singly dimensioned coord arrays
* - plcon2	linear interpolation from doubly dimensioned coord arrays
*
* The latter two work by calling plfcont() with the appropriate grid
* structure for input to pltr2f().
\*----------------------------------------------------------------------*/

/* no transformation */

void
PLCON07(PLFLT *z, PLINT *nx, PLINT *ny, PLINT *kx, PLINT *lx,
	PLINT *ky, PLINT *ly, PLFLT *clevel, PLINT *nlevel)
{
    PLfGrid fgrid;

    fgrid.nx = *nx;
    fgrid.ny = *ny;
    fgrid.f = z;

    plfcont(plf2evalr, (void *) &fgrid,
	    *nx, *ny, *kx, *lx, *ky, *ly, clevel, *nlevel,
	    pltr0f, NULL);
}

/* 1-d transformation */

void
PLCON17(PLFLT *z, PLINT *nx, PLINT *ny, PLINT *kx, PLINT *lx,
	PLINT *ky, PLINT *ly, PLFLT *clevel, PLINT *nlevel,
	PLFLT *xg, PLFLT *yg)
{
    PLfGrid fgrid;
    PLcGrid cgrid;

    fgrid.nx = *nx;
    fgrid.ny = *ny;
    fgrid.f = z;

    cgrid.nx = *nx;
    cgrid.ny = *ny;
    cgrid.xg = xg;
    cgrid.yg = yg;

    plfcont(plf2evalr, (void *) &fgrid,
	    *nx, *ny, *kx, *lx, *ky, *ly, clevel, *nlevel,
	    pltr1, (void *) &cgrid);
}

/* 2-d transformation */

void
PLCON27(PLFLT *z, PLINT *nx, PLINT *ny, PLINT *kx, PLINT *lx,
	PLINT *ky, PLINT *ly, PLFLT *clevel, PLINT *nlevel,
	PLFLT *xg, PLFLT *yg)
{
    PLfGrid fgrid;
    PLcGrid cgrid;

    fgrid.nx = *nx;
    fgrid.ny = *ny;
    fgrid.f = z;

    cgrid.nx = *nx;
    cgrid.ny = *ny;
    cgrid.xg = xg;
    cgrid.yg = yg;

    plfcont(plf2evalr, (void *) &fgrid,
	    *nx, *ny, *kx, *lx, *ky, *ly, clevel, *nlevel,
	    pltr2f, (void *) &cgrid);
}

/*----------------------------------------------------------------------*\
* Vector plotter front-ends.
* These specify the row-dominant function evaluator in the plfvect
* argument list.  NO TRANSPOSE IS NECESSARY.  The routines are as follows:
*
* - plvec0	no transformation
* - plvec1	linear interpolation from singly dimensioned coord arrays
* - plvec2	linear interpolation from doubly dimensioned coord arrays
*
* The latter two work by calling plfvect() with the appropriate grid
* structure for input to pltr2f().
\*----------------------------------------------------------------------*/

/* no transformation */

void
PLVEC07(PLFLT *u, PLFLT *v, PLINT *nx, PLINT *ny, PLFLT *scale)
{
    PLfGrid fgrid1, fgrid2;

    fgrid1.nx = *nx;
    fgrid1.ny = *ny;
    fgrid1.f = u;

    fgrid2.nx = *nx;
    fgrid2.ny = *ny;
    fgrid2.f = v;

    plfvect(plf2evalr, (void *) &fgrid1, (void *) &fgrid2,
	    *nx, *ny, *scale, pltr0f, NULL);
}

/* 1-d transformation */

void
PLVEC17(PLFLT *u, PLFLT *v, PLINT *nx, PLINT *ny, PLFLT *scale,
	PLFLT *xg, PLFLT *yg)
{
    PLfGrid fgrid1;
    PLfGrid fgrid2;
    PLcGrid cgrid;

    fgrid1.nx = *nx;
    fgrid1.ny = *ny;
    fgrid1.f = u;

    fgrid2.nx = *nx;
    fgrid2.ny = *ny;
    fgrid2.f = v;

    cgrid.nx = *nx;
    cgrid.ny = *ny;
    cgrid.xg = xg;
    cgrid.yg = yg;

    plfvect(plf2evalr, (void *) &fgrid1, (void *) &fgrid2,
	    *nx, *ny, *scale, pltr1, (void *) &cgrid);
}

/* 2-d transformation */

void
PLVEC27(PLFLT *u, PLFLT *v, PLINT *nx, PLINT *ny, PLFLT *scale,
	PLFLT *xg, PLFLT *yg)
{
    PLfGrid fgrid1;
    PLfGrid fgrid2;
    PLcGrid cgrid;

    fgrid1.nx = *nx;
    fgrid1.ny = *ny;
    fgrid1.f = u;

    fgrid2.nx = *nx;
    fgrid2.ny = *ny;
    fgrid2.f = v;

    cgrid.nx = *nx;
    cgrid.ny = *ny;
    cgrid.xg = xg;
    cgrid.yg = yg;

    plfvect(plf2evalr, (void *) &fgrid1, (void *) &fgrid2,
	    *nx, *ny, *scale, pltr2f, (void *) &cgrid);
}

/*----------------------------------------------------------------------*\
* Here are the old contour plotters.
\*----------------------------------------------------------------------*/

static void
pltr(PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, void *pltr_data)
{
    PLFLT *tr = (PLFLT *) pltr_data;

    *tx = tr[0] * x + tr[1] * y + tr[2];
    *ty = tr[3] * x + tr[4] * y + tr[5];
}

void
PLCONT7(PLFLT *z, PLINT *nx, PLINT *ny, PLINT *kx, PLINT *lx,
	PLINT *ky, PLINT *ly, PLFLT *clevel, PLINT *nlevel, PLFLT *ftr)
{
    PLfGrid fgrid;

    fgrid.nx = *nx;
    fgrid.ny = *ny;
    fgrid.f = z;

    plfcont(plf2evalr, (void *) &fgrid,
	    *nx, *ny, *kx, *lx, *ky, *ly, clevel, *nlevel,
	    pltr, (void *) ftr);
}

void
PLVECT7(PLFLT *u, PLFLT *v, PLINT *nx, PLINT *ny, PLFLT *scale,
	PLFLT *ftr)
{
    PLfGrid fgrid1;
    PLfGrid fgrid2;

    fgrid1.nx = *nx;
    fgrid1.ny = *ny;
    fgrid1.f = u;

    fgrid2.nx = *nx;
    fgrid2.ny = *ny;
    fgrid2.f = v;

    plfvect(plf2evalr, (void *) &fgrid1, (void *) &fgrid2,
	    *nx, *ny, *scale,
	    pltr, (void *) ftr);
}

/*----------------------------------------------------------------------*\
 * plfshade front-ends.
 * These specify the row-dominant function evaluator in the plfshade
 * argument list.  NO TRANSPOSE IS NECESSARY.  The routines are as follows:
 *
 * - plshade0	map indices to xmin, xmax, ymin, ymax.
 * The next two work by calling plfshade() with the appropriate grid
 * structure for input to pltr2f().
 * - plshade1	linear interpolation from singly dimensioned coord arrays
 * - plshade2	linear interpolation from doubly dimensioned coord arrays
 * - plshade    tr array transformation
 *
\*----------------------------------------------------------------------*/

void
PLSHADE07(PLFLT *z, PLINT *nx, PLINT *ny, char *defined,
	  PLFLT *xmin, PLFLT *xmax, PLFLT *ymin, PLFLT *ymax,
	  PLFLT *shade_min, PLFLT *shade_max,
	  PLINT *sh_cmap, PLFLT *sh_color, PLINT *sh_width,
	  PLINT *min_color, PLINT *min_width,
	  PLINT *max_color, PLINT *max_width, PLINT *lx)
{
   PLINT rect = 1;
   PLFLT ** a;
   int i,j;

/* Create a vectored a array from transpose of the fortran z array. */
   plAlloc2dGrid(&a, *nx, *ny);
   for (i = 0; i < *nx; i++) {
      for (j = 0; j < *ny; j++) {
	 a[i][j] = z[i +j * *lx];
      }
   }

   c_plshade( a, *nx, *ny, NULL,
	      *xmin, *xmax, *ymin, *ymax,
	      *shade_min, *shade_max,
	      *sh_cmap, *sh_color, *sh_width,
	      *min_color, *min_width, *max_color, *max_width,
	      c_plfill, rect, NULL, NULL);

/* Clean up memory allocated for a */
   plFree2dGrid(a, *nx, *ny);
}


/* 1-d transformation */

void
PLSHADE17(PLFLT *z, PLINT *nx, PLINT *ny, char *defined,
	  PLFLT *xmin, PLFLT *xmax, PLFLT *ymin, PLFLT *ymax,
	  PLFLT *shade_min, PLFLT *shade_max,
	  PLINT *sh_cmap, PLFLT *sh_color, PLINT *sh_width,
	  PLINT *min_color, PLINT *min_width,
	  PLINT *max_color, PLINT *max_width,
	  PLFLT *xg1, PLFLT *yg1, PLINT *lx)
{
   PLINT rect = 1;
   PLFLT ** a;
   int i,j;
   PLcGrid cgrid;

/* Create a vectored a array from transpose of the fortran z array. */
   plAlloc2dGrid(&a, *nx, *ny);
   for (i = 0; i < *nx; i++) {
      for (j = 0; j < *ny; j++) {
	 a[i][j] = z[i +j * *lx];
      }
   }

   cgrid.nx = *nx;
   cgrid.ny = *ny;
   cgrid.xg = xg1;
   cgrid.yg = yg1;
   c_plshade( a, *nx, *ny, NULL,
	      *xmin, *xmax, *ymin, *ymax,
	      *shade_min, *shade_max,
	      *sh_cmap, *sh_color, *sh_width,
	      *min_color, *min_width, *max_color, *max_width,
	      c_plfill, rect, pltr1, (PLPointer) &cgrid);

/* Clean up memory allocated for a */
   plFree2dGrid(a, *nx, *ny);
}

/* 2-d transformation */

void
PLSHADE27(PLFLT *z, PLINT *nx, PLINT *ny, char *defined,
	  PLFLT *xmin, PLFLT *xmax, PLFLT *ymin, PLFLT *ymax,
	  PLFLT *shade_min, PLFLT *shade_max,
	  PLINT *sh_cmap, PLFLT *sh_color, PLINT *sh_width,
	  PLINT *min_color, PLINT *min_width,
	  PLINT *max_color, PLINT *max_width,
	  PLFLT *xg2, PLFLT *yg2, PLINT *lx)
{
   PLINT rect = 0;
   PLFLT **a;
   int i,j;
   PLcGrid2 cgrid2;

/* Create a vectored a array from transpose of the fortran z array. */
   plAlloc2dGrid(&a, *nx, *ny);
   plAlloc2dGrid(&cgrid2.xg, *nx, *ny);
   plAlloc2dGrid(&cgrid2.yg, *nx, *ny);
   cgrid2.nx = *nx;
   cgrid2.ny = *ny;
   for (i = 0; i < *nx; i++) {
      for (j = 0; j < *ny; j++) {
	 a[i][j] = z[i +j * *lx];
	 cgrid2.xg[i][j] = xg2[i +j * *lx];
	 cgrid2.yg[i][j] = yg2[i +j * *lx];
      }
   }

   c_plshade( a, *nx, *ny, NULL,
	      *xmin, *xmax, *ymin, *ymax,
	      *shade_min, *shade_max,
	      *sh_cmap, *sh_color, *sh_width,
	      *min_color, *min_width, *max_color, *max_width,
	      c_plfill, rect, pltr2, (void *) &cgrid2);

/* Clean up memory allocated for a */
   plFree2dGrid(a, *nx, *ny);
   plFree2dGrid(cgrid2.xg, *nx, *ny);
   plFree2dGrid(cgrid2.yg, *nx, *ny);

}

void
PLSHADE7(PLFLT *z, PLINT *nx, PLINT *ny, char *defined,
	 PLFLT *xmin, PLFLT *xmax, PLFLT *ymin, PLFLT *ymax,
	 PLFLT *shade_min, PLFLT *shade_max,
	 PLINT *sh_cmap, PLFLT *sh_color, PLINT *sh_width,
	 PLINT *min_color, PLINT *min_width,
	 PLINT *max_color, PLINT *max_width, PLFLT *ftr, PLINT *lx)
{
   PLINT rect = 1;
   PLFLT ** a;
   int i,j;

/* Create a vectored a array from transpose of the fortran z array. */
   plAlloc2dGrid(&a, *nx, *ny);
   for (i = 0; i < *nx; i++) {
      for (j = 0; j < *ny; j++) {
	 a[i][j] = z[i +j * *lx];
      }
   }

   c_plshade( a, *nx, *ny, NULL,
	      *xmin, *xmax, *ymin, *ymax,
	      *shade_min, *shade_max,
	      *sh_cmap, *sh_color, *sh_width,
	      *min_color, *min_width, *max_color, *max_width,
	      c_plfill, rect, pltr, (void *) ftr);

/* Clean up memory allocated for a */
   plFree2dGrid(a, *nx, *ny);
}

/*----------------------------------------------------------------------*\
 * plshades front-ends.
 *
 * - plshades0	map indices to xmin, xmax, ymin, ymax
 * - plshades1	linear interpolation from singly dimensioned coord arrays
 * - plshades2	linear interpolation from doubly dimensioned coord arrays
 * - plshades   pass tr information with plplot common block (and
 *              then pass tr as last argument of PLSHADES7)
\*----------------------------------------------------------------------*/

void
PLSHADES07(PLFLT *z, PLINT *nx, PLINT *ny, char *defined,
	   PLFLT *xmin, PLFLT *xmax, PLFLT *ymin, PLFLT *ymax,
	   PLFLT *clevel, PLINT *nlevel, PLINT *fill_width,
	   PLINT *cont_color, PLINT *cont_width, PLINT *lx)
{
   PLINT rect = 1;
   PLFLT ** a;
   int i,j;

/* Create a vectored a array from transpose of the fortran z array. */
   plAlloc2dGrid(&a, *nx, *ny);
   for (i = 0; i < *nx; i++) {
      for (j = 0; j < *ny; j++) {
	 a[i][j] = z[i +j * *lx];
      }
   }

   c_plshades( a, *nx, *ny, NULL,
	       *xmin, *xmax, *ymin, *ymax,
	       clevel, *nlevel, *fill_width,
	       *cont_color, *cont_width,
	       c_plfill, rect, NULL, NULL);

/* Clean up memory allocated for a */
   plFree2dGrid(a, *nx, *ny);
}

void
PLSHADES17(PLFLT *z, PLINT *nx, PLINT *ny, char *defined,
	   PLFLT *xmin, PLFLT *xmax, PLFLT *ymin, PLFLT *ymax,
	   PLFLT *clevel, PLINT *nlevel, PLINT *fill_width,
	   PLINT *cont_color, PLINT *cont_width,
	   PLFLT *xg1, PLFLT *yg1, PLINT *lx)
{
   PLINT rect = 1;
   PLFLT ** a;
   int i,j;
   PLcGrid cgrid;

/* Create a vectored a array from transpose of the fortran z array. */
   plAlloc2dGrid(&a, *nx, *ny);
   for (i = 0; i < *nx; i++) {
      for (j = 0; j < *ny; j++) {
	 a[i][j] = z[i +j * *lx];
      }
   }

   cgrid.nx = *nx;
   cgrid.ny = *ny;
   cgrid.xg = xg1;
   cgrid.yg = yg1;

   c_plshades( a, *nx, *ny, NULL,
	       *xmin, *xmax, *ymin, *ymax,
	       clevel, *nlevel, *fill_width,
	       *cont_color, *cont_width,
	       c_plfill, rect, pltr1, (PLPointer) &cgrid);

/* Clean up memory allocated for a */
   plFree2dGrid(a, *nx, *ny);
}

void
PLSHADES27(PLFLT *z, PLINT *nx, PLINT *ny, char *defined,
	   PLFLT *xmin, PLFLT *xmax, PLFLT *ymin, PLFLT *ymax,
	   PLFLT *clevel, PLINT *nlevel, PLINT *fill_width,
	   PLINT *cont_color, PLINT *cont_width,
	   PLFLT *xg2, PLFLT *yg2, PLINT *lx)
{
   PLINT rect = 0;
   PLFLT **a;
   int i,j;
   PLcGrid2 cgrid2;

/* Create a vectored a array from transpose of the fortran z array. */
   plAlloc2dGrid(&a, *nx, *ny);
   plAlloc2dGrid(&cgrid2.xg, *nx, *ny);
   plAlloc2dGrid(&cgrid2.yg, *nx, *ny);
   cgrid2.nx = *nx;
   cgrid2.ny = *ny;
   for (i = 0; i < *nx; i++) {
      for (j = 0; j < *ny; j++) {
	 a[i][j] = z[i +j * *lx];
	 cgrid2.xg[i][j] = xg2[i +j * *lx];
	 cgrid2.yg[i][j] = yg2[i +j * *lx];
      }
   }

   c_plshades( a, *nx, *ny, NULL,
	     *xmin, *xmax, *ymin, *ymax,
	     clevel, *nlevel, *fill_width,
	     *cont_color, *cont_width,
	     c_plfill, rect, pltr2, (void *) &cgrid2);

/* Clean up allocated memory */
   plFree2dGrid(a, *nx, *ny);
   plFree2dGrid(cgrid2.xg, *nx, *ny);
   plFree2dGrid(cgrid2.yg, *nx, *ny);
}

void
PLSHADES7(PLFLT *z, PLINT *nx, PLINT *ny, char *defined,
	   PLFLT *xmin, PLFLT *xmax, PLFLT *ymin, PLFLT *ymax,
	   PLFLT *clevel, PLINT *nlevel, PLINT *fill_width,
	   PLINT *cont_color, PLINT *cont_width, PLFLT *ftr, PLINT *lx)
{
   PLINT rect = 1;
   PLFLT ** a;
   int i,j;

/* Create a vectored a array from transpose of the fortran z array. */
   plAlloc2dGrid(&a, *nx, *ny);
   for (i = 0; i < *nx; i++) {
      for (j = 0; j < *ny; j++) {
	 a[i][j] = z[i +j * *lx];
      }
   }

   c_plshades( a, *nx, *ny, NULL,
	     *xmin, *xmax, *ymin, *ymax,
	     clevel, *nlevel, *fill_width,
	     *cont_color, *cont_width,
	     c_plfill, rect, pltr, (void *) ftr);

/* Clean up memory allocated for a */
   plFree2dGrid(a, *nx, *ny);
}
