/* $Id: sc3d.c 3186 2006-02-15 18:17:33Z slbrow $

	Stub routines for 3d plots.

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

void
PLOT3DC(PLFLT *x, PLFLT *y, PLFLT *z,
	PLINT *nx, PLINT *ny, PLINT *opt,
	PLFLT *clevel, PLINT *nlevel, PLINT *lx)
{
   PLFLT ** a;
   int i,j;

/* Create a vectored a array from transpose of the fortran z array. */
   plAlloc2dGrid(&a, *nx, *ny);
   for (i = 0; i < *nx; i++) {
      for (j = 0; j < *ny; j++) {
	 a[i][j] = z[i +j * *lx];
      }
   }

   c_plot3dc(x, y, a, *nx, *ny, *opt, clevel, *nlevel);

/* Clean up memory allocated for a */
   plFree2dGrid(a, *nx, *ny);

}

void
PLSURF3D(PLFLT *x, PLFLT *y, PLFLT *z,
       PLINT *nx, PLINT *ny, PLINT *opt,
       PLFLT *clevel, PLINT *nlevel, PLINT *lx)
{
    int i, j;
    PLFLT **temp;

    /* Create the vectored C matrix from the Fortran matrix */
    /* To make things easy we save a temporary copy of the transpose of the
       Fortran matrix, so that the first dimension of z corresponds to the x
       direction. */

    if ( ! (temp = (PLFLT **) malloc((size_t) * nx * sizeof(PLFLT *)))) {
	plabort("PLSURF3D: Out of memory");
	return;
    }

    for (i = 0; i < *nx; i++) {
	if ( ! (temp[i] = (PLFLT *) malloc((size_t) * ny * sizeof(PLFLT)))) {
	    int ii;

	    for (ii = 0; ii < i-1; ii++)
		free((void *) temp[i]);
	    free((void *) temp);
	    plabort("PLSURF3D: Out of memory");
	    return;
	}
    }

    for (i = 0; i < *nx; i++)
	for (j = 0; j < *ny; j++)
	    temp[i][j] = *(z + j * *lx + i);

    c_plsurf3d(x, y, temp, *nx, *ny, *opt, clevel, *nlevel);

    for (i = 0; i < *nx; i++)
	free((void *) temp[i]);

    free((void *) temp);
}

void
PLMESH(PLFLT *x, PLFLT *y, PLFLT *z,
       PLINT *nx, PLINT *ny, PLINT *opt, PLINT *lx)
{
   PLINT optlocal, nlevel = 0;
   PLFLT clevel = 0.;

   optlocal = *opt | MESH;
   PLOT3DC(x, y, z, nx, ny, &optlocal, &clevel, &nlevel, lx);
}

void
PLMESHC(PLFLT *x, PLFLT *y, PLFLT *z,
       PLINT *nx, PLINT *ny, PLINT *opt,
       PLFLT *clevel, PLINT *nlevel, PLINT *lx)
{
   PLINT optlocal;
   optlocal = *opt | MESH;
   PLOT3DC(x, y, z, nx, ny, &optlocal, clevel, nlevel, lx);
}


void
PLOT3D(PLFLT *x, PLFLT *y, PLFLT *z,
       PLINT *nx, PLINT *ny, PLINT *opt, PLINT *side, PLINT *lx)
{
   PLINT optlocal, nlevel = 0;
   PLFLT clevel = 0.;

   optlocal = *opt | (*side == 1 ? DRAW_SIDES : 0);
   PLOT3DC(x, y, z, nx, ny, &optlocal, &clevel, &nlevel, lx);
}

