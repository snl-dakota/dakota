/* $Id: pltick.c 3186 2006-02-15 18:17:33Z slbrow $

	Routines for drawing error bars and tick marks.

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

#include "plplotP.h"

/*----------------------------------------------------------------------*\
 * void plxtik()
 *
 * Draws a tick parallel to x.
\*----------------------------------------------------------------------*/

void
plxtik(PLINT x, PLINT y, PLINT below, PLINT above)
{
    plP_draphy(x, y);

    if (below != 0)
	plP_draphy(x, y - below);

    if (above != 0)
	plP_draphy(x, y + above);

    plP_draphy(x, y);
}

/*----------------------------------------------------------------------*\
 * void plytik()
 *
 * Draws a tick parallel to y.
\*----------------------------------------------------------------------*/

void
plytik(PLINT x, PLINT y, PLINT left, PLINT right)
{
    plP_draphy(x, y);

    if (left != 0)
	plP_draphy(x - left, y);

    if (right != 0)
	plP_draphy(x + right, y);

    plP_draphy(x, y);
}

/*----------------------------------------------------------------------*\
 * void plstik()
 *
 * Draws a slanting tick at position (mx,my) (measured in mm) of
 * vector length (dx,dy).
\*----------------------------------------------------------------------*/

void
plstik(PLFLT mx, PLFLT my, PLFLT dx, PLFLT dy)
{
    plP_draphy(plP_mmpcx(mx), plP_mmpcy(my));
    plP_draphy(plP_mmpcx((PLFLT) (mx + dx)), plP_mmpcy((PLFLT) (my + dy)));
    plP_draphy(plP_mmpcx(mx), plP_mmpcy(my));
}

/*----------------------------------------------------------------------*\
 * void plerx1()
 *
 * Plot single horizontal error bar.
\*----------------------------------------------------------------------*/

static void
plerx1(PLFLT xmin, PLFLT xmax, PLFLT y)
{
    PLINT yminor;

    yminor = MAX(1.0, plsc->minht * plsc->ypmm);
    plP_movwor(xmin, y);
    plxtik(plP_wcpcx(xmin), plP_wcpcy(y), yminor, yminor);
    plP_drawor(xmax, y);
    plxtik(plP_wcpcx(xmax), plP_wcpcy(y), yminor, yminor);
}

/*----------------------------------------------------------------------*\
 * void plery1()
 *
 * Plot single vertical error bar.
\*----------------------------------------------------------------------*/

static void
plery1(PLFLT x, PLFLT ymin, PLFLT ymax)
{
    PLINT xminor;

    xminor = MAX(1.0, plsc->minht * plsc->xpmm);
    plP_movwor(x, ymin);
    plytik(plP_wcpcx(x), plP_wcpcy(ymin), xminor, xminor);
    plP_drawor(x, ymax);
    plytik(plP_wcpcx(x), plP_wcpcy(ymax), xminor, xminor);
}

/*----------------------------------------------------------------------*\
 * void plerrx()
 *
 * Plot horizontal error bars (xmin(i),y(i)) to (xmax(i),y(i)).
\*----------------------------------------------------------------------*/

void
c_plerrx(PLINT n, PLFLT *xmin, PLFLT *xmax, PLFLT *y)
{
    PLINT i;

    if (plsc->level < 3) {
	plabort("plerrx: Please set up window first");
	return;
    }

    for (i = 0; i < n; i++)
	plerx1(xmin[i], xmax[i], y[i]);
}

/*----------------------------------------------------------------------*\
 * void plerry()
 *
 * Plot vertical error bars (x,ymin(i)) to (x(i),ymax(i)).
\*----------------------------------------------------------------------*/

void
c_plerry(PLINT n, PLFLT *x, PLFLT *ymin, PLFLT *ymax)
{
    PLINT i;

    if (plsc->level < 3) {
	plabort("plerry: Please set up window first");
	return;
    }

    for (i = 0; i < n; i++)
	plery1(x[i], ymin[i], ymax[i]);
}
