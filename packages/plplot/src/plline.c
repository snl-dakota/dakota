/* $Id: plline.c 3186 2006-02-15 18:17:33Z slbrow $

	Routines dealing with line generation.

   Copyright (C) 2004  Maurice LeBrun

   This file is part of PLplot.

   PLplot is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published
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

#define INSIDE(ix,iy) (BETW(ix,xmin,xmax) && BETW(iy,ymin,ymax))

static PLINT xline[PL_MAXPOLY], yline[PL_MAXPOLY];

static PLINT lastx = PL_UNDEFINED, lasty = PL_UNDEFINED;

/* Function prototypes */

/* Draws a polyline within the clip limits. */

static void
pllclp(PLINT *x, PLINT *y, PLINT npts);

/* Get clipped endpoints */

static int
clipline(PLINT *p_x1, PLINT *p_y1, PLINT *p_x2, PLINT *p_y2,
	 PLINT xmin, PLINT xmax, PLINT ymin, PLINT ymax);

/* General line-drawing routine.  Takes line styles into account. */

static void
genlin(short *x, short *y, PLINT npts);

/* Draws a dashed line to the specified point from the previous one. */

static void
grdashline(short *x, short *y);

/* Determines if a point is inside a polygon or not */

static int
pointinpolygon( int n, short *x, short *y, PLINT xp, PLINT yp );

/*----------------------------------------------------------------------*\
 * void pljoin()
 *
 * Draws a line segment from (x1, y1) to (x2, y2).
\*----------------------------------------------------------------------*/

void
c_pljoin(PLFLT x1, PLFLT y1, PLFLT x2, PLFLT y2)
{
    plP_movwor(x1, y1);
    plP_drawor(x2, y2);
}

/*----------------------------------------------------------------------*\
 * void plline()
 *
 * Draws line segments connecting a series of points.
\*----------------------------------------------------------------------*/

void
c_plline(PLINT n, PLFLT *x, PLFLT *y)
{
    if (plsc->level < 3) {
	plabort("plline: Please set up window first");
	return;
    }
    plP_drawor_poly(x, y, n);
}

/*----------------------------------------------------------------------*\
 * void plline3(n, x, y, z)
 *
 * Draws a line in 3 space.  You must first set up the viewport, the
 * 2d viewing window (in world coordinates), and the 3d normalized
 * coordinate box.  See x18c.c for more info.
 *
 * This version adds clipping against the 3d bounding box specified in plw3d
\*----------------------------------------------------------------------*/
void
c_plline3(PLINT n, PLFLT *x, PLFLT *y, PLFLT *z)
{
    int i;
    PLFLT vmin[3], vmax[3], zscale;

    if (plsc->level < 3) {
	plabort("plline3: Please set up window first");
	return;
    }

    /* get the bounding box in 3d */
    plP_gdom(&vmin[0], &vmax[0], &vmin[1], &vmax[1]);
    plP_grange(&zscale, &vmin[2], &vmax[2]);

    /* interate over the vertices */
    for( i=0; i < n-1; i++ ) {
      PLFLT p0[3], p1[3];
      int axis;

      /* copy the end points of the segment to allow clipping */
      p0[0] = x[i]; p0[1] = y[i]; p0[2] = z[i];
      p1[0] = x[i+1]; p1[1] = y[i+1]; p1[2] = z[i+1];

      /* check against each axis of the bounding box */
      for(axis = 0; axis < 3; axis++) {
	if(p0[axis] < vmin[axis]) { /* first out */
	  if(p1[axis] < vmin[axis]) {
	    break; /* both endpoints out so quit */
	  } else {
	    int j;
	    /* interpolate to find intersection with box */
	    PLFLT t = (vmin[axis] - p0[axis]) / (p1[axis] - p0[axis]);
	    p0[axis] = vmin[axis];
	    for(j = 1; j<3; j++) {
	      int k = (axis+j)%3;
	      p0[k] = (1-t)*p0[k] + t*p1[k];
	    }
	  }
	} else if(p1[axis] < vmin[axis]) { /* second out */
	  int j;
	  /* interpolate to find intersection with box */
	  PLFLT t = (vmin[axis] - p0[axis]) / (p1[axis] - p0[axis]);
	  p1[axis] = vmin[axis];
	  for(j = 1; j<3; j++) {
	    int k = (axis+j)%3;
	    p1[k] = (1-t)*p0[k] + t*p1[k];
	  }
	}
	if(p0[axis] > vmax[axis]) { /* first out */
	  if(p1[axis] > vmax[axis]) {
	    break; /* both out so quit */
	  } else {
	    int j;
	    /* interpolate to find intersection with box */
	    PLFLT t = (vmax[axis] - p0[axis]) / (p1[axis] - p0[axis]);
	    p0[axis] = vmax[axis];
	    for(j = 1; j<3; j++) {
	      int k = (axis+j)%3;
	      p0[k] = (1-t)*p0[k] + t*p1[k];
	    }
	  }
	} else if(p1[axis] > vmax[axis]) { /* second out */
	  int j;
	  /* interpolate to find intersection with box */
	  PLFLT t = (vmax[axis] - p0[axis]) / (p1[axis] - p0[axis]);
	  p1[axis] = vmax[axis];
	  for(j = 1; j<3; j++) {
	    int k = (axis+j)%3;
	    p1[k] = (1-t)*p0[k] + t*p1[k];
	  }
	}
      }
      /* if we made it to here without "break"ing out of the loop, the
	 remaining segment is visible */
      if( axis == 3 ) { /*  not clipped away */
	PLFLT u0, v0, u1, v1;
	u0 = plP_wcpcx(plP_w3wcx( p0[0], p0[1], p0[2] ));
	v0 = plP_wcpcy(plP_w3wcy( p0[0], p0[1], p0[2] ));
	u1 = plP_wcpcx(plP_w3wcx( p1[0], p1[1], p1[2] ));
	v1 = plP_wcpcy(plP_w3wcy( p1[0], p1[1], p1[2] ));
	plP_movphy(u0,v0);
	plP_draphy(u1,v1);
      }
    }
    return;
}
/*----------------------------------------------------------------------*\
 * void plpoly3( n, x, y, z, draw, ifcc )
 *
 * Draws a polygon in 3 space.  This differs from plline3() in that
 * this attempts to determine if the polygon is viewable.  If the back
 * of polygon is facing the viewer, then it isn't drawn.  If this
 * isn't what you want, then use plline3 instead.
 *
 * n specifies the number of points.  They are assumed to be in a
 * plane, and the directionality of the plane is determined from the
 * first three points.  Additional points do not /have/ to lie on the
 * plane defined by the first three, but if they do not, then the
 * determiniation of visibility obviously can't be 100% accurate...
 * So if you're 3 space polygons are too far from planar, consider
 * breaking them into smaller polygons.  "3 points define a plane" :-).
 *
 * For ifcc == 1, the directionality of the polygon is determined by assuming
 * the points are laid out in counter-clockwise order.
 *
 * For ifcc == 0, the directionality of the polygon is determined by assuming
 * the points are laid out in clockwise order.
 *
 * BUGS:  If one of the first two segments is of zero length, or if
 * they are colinear, the calculation of visibility has a 50/50 chance
 * of being correct.  Avoid such situations :-).  See x18c for an
 * example of this problem.  (Search for "20.1").
\*----------------------------------------------------------------------*/

void
c_plpoly3(PLINT n, PLFLT *x, PLFLT *y, PLFLT *z, PLINT *draw, PLINT ifcc)
{
    int i;
    PLFLT vmin[3], vmax[3], zscale;
    PLFLT u1, v1, u2, v2, u3, v3;
    PLFLT c;

    if (plsc->level < 3) {
	plabort("plpoly3: Please set up window first");
	return;
    }

    if ( n < 3 ) {
	plabort("plpoly3: Must specify at least 3 points");
	return;
    }

/* Now figure out which side this is. */

    u1 = plP_wcpcx(plP_w3wcx( x[0], y[0], z[0] ));
    v1 = plP_wcpcy(plP_w3wcy( x[0], y[0], z[0] ));

    u2 = plP_wcpcx(plP_w3wcx( x[1], y[1], z[1] ));
    v2 = plP_wcpcy(plP_w3wcy( x[1], y[1], z[1] ));

    u3 = plP_wcpcx(plP_w3wcx( x[2], y[2], z[2] ));
    v3 = plP_wcpcy(plP_w3wcy( x[2], y[2], z[2] ));

    c = (u1-u2)*(v3-v2)-(v1-v2)*(u3-u2);

    if ( c *(1 - 2*ifcc) < 0. )
        return;

    /* get the bounding box in 3d */
    plP_gdom(&vmin[0], &vmax[0], &vmin[1], &vmax[1]);
    plP_grange(&zscale, &vmin[2], &vmax[2]);

    /* interate over the vertices */
    for( i=0; i < n-1; i++ ) {
      PLFLT p0[3], p1[3];
      int axis;

      /* copy the end points of the segment to allow clipping */
      p0[0] = x[i]; p0[1] = y[i]; p0[2] = z[i];
      p1[0] = x[i+1]; p1[1] = y[i+1]; p1[2] = z[i+1];

      /* check against each axis of the bounding box */
      for(axis = 0; axis < 3; axis++) {
	if(p0[axis] < vmin[axis]) { /* first out */
	  if(p1[axis] < vmin[axis]) {
	    break; /* both endpoints out so quit */
	  } else {
	    int j;
	    /* interpolate to find intersection with box */
	    PLFLT t = (vmin[axis] - p0[axis]) / (p1[axis] - p0[axis]);
	    p0[axis] = vmin[axis];
	    for(j = 1; j<3; j++) {
	      int k = (axis+j)%3;
	      p0[k] = (1-t)*p0[k] + t*p1[k];
	    }
	  }
	} else if(p1[axis] < vmin[axis]) { /* second out */
	  int j;
	  /* interpolate to find intersection with box */
	  PLFLT t = (vmin[axis] - p0[axis]) / (p1[axis] - p0[axis]);
	  p1[axis] = vmin[axis];
	  for(j = 1; j<3; j++) {
	    int k = (axis+j)%3;
	    p1[k] = (1-t)*p0[k] + t*p1[k];
	  }
	}
	if(p0[axis] > vmax[axis]) { /* first out */
	  if(p1[axis] > vmax[axis]) {
	    break; /* both out so quit */
	  } else {
	    int j;
	    /* interpolate to find intersection with box */
	    PLFLT t = (vmax[axis] - p0[axis]) / (p1[axis] - p0[axis]);
	    p0[axis] = vmax[axis];
	    for(j = 1; j<3; j++) {
	      int k = (axis+j)%3;
	      p0[k] = (1-t)*p0[k] + t*p1[k];
	    }
	  }
	} else if(p1[axis] > vmax[axis]) { /* second out */
	  int j;
	  /* interpolate to find intersection with box */
	  PLFLT t = (vmax[axis] - p0[axis]) / (p1[axis] - p0[axis]);
	  p1[axis] = vmax[axis];
	  for(j = 1; j<3; j++) {
	    int k = (axis+j)%3;
	    p1[k] = (1-t)*p0[k] + t*p1[k];
	  }
	}
      }
      /* if we made it to here without "break"ing out of the loop, the
	 remaining segment is visible */
      if( axis == 3 && draw[i] ) { /*  not clipped away */
	PLFLT u0, v0, u1, v1;
	u0 = plP_wcpcx(plP_w3wcx( p0[0], p0[1], p0[2] ));
	v0 = plP_wcpcy(plP_w3wcy( p0[0], p0[1], p0[2] ));
	u1 = plP_wcpcx(plP_w3wcx( p1[0], p1[1], p1[2] ));
	v1 = plP_wcpcy(plP_w3wcy( p1[0], p1[1], p1[2] ));
	plP_movphy(u0,v0);
	plP_draphy(u1,v1);
      }
    }
    return;
}

/*----------------------------------------------------------------------*\
 * void plstyl()
 *
 * Set up a new line style of "nms" elements, with mark and space
 * lengths given by arrays "mark" and "space".
\*----------------------------------------------------------------------*/

void
c_plstyl(PLINT nms, PLINT *mark, PLINT *space)
{
    short int i;

    if (plsc->level < 1) {
	plabort("plstyl: Please call plinit first");
	return;
    }
    if ((nms < 0) || (nms > 10)) {
	plabort("plstyl: Broken lines cannot have <0 or >10 elements");
	return;
    }
    for (i = 0; i < nms; i++) {
	if ((mark[i] < 0) || (space[i] < 0)) {
	    plabort("plstyl: Mark and space lengths must be > 0");
	    return;
	}
    }

    plsc->nms = nms;
    for (i = 0; i < nms; i++) {
	plsc->mark[i] = mark[i];
	plsc->space[i] = space[i];
    }

    plsc->curel = 0;
    plsc->pendn = 1;
    plsc->timecnt = 0;
    plsc->alarm = nms > 0 ? mark[0] : 0;
}

/*----------------------------------------------------------------------*\
 * void plP_movphy()
 *
 * Move to physical coordinates (x,y).
\*----------------------------------------------------------------------*/

void
plP_movphy(PLINT x, PLINT y)
{
    plsc->currx = x;
    plsc->curry = y;
}

/*----------------------------------------------------------------------*\
 * void plP_draphy()
 *
 * Draw to physical coordinates (x,y).
\*----------------------------------------------------------------------*/

void
plP_draphy(PLINT x, PLINT y)
{
    xline[0] = plsc->currx;
    xline[1] = x;
    yline[0] = plsc->curry;
    yline[1] = y;

    pllclp(xline, yline, 2);
}

/*----------------------------------------------------------------------*\
 * void plP_movwor()
 *
 * Move to world coordinates (x,y).
\*----------------------------------------------------------------------*/

void
plP_movwor(PLFLT x, PLFLT y)
{
    plsc->currx = plP_wcpcx(x);
    plsc->curry = plP_wcpcy(y);
}

/*----------------------------------------------------------------------*\
 * void plP_drawor()
 *
 * Draw to world coordinates (x,y).
\*----------------------------------------------------------------------*/

void
plP_drawor(PLFLT x, PLFLT y)
{
    xline[0] = plsc->currx;
    xline[1] = plP_wcpcx(x);
    yline[0] = plsc->curry;
    yline[1] = plP_wcpcy(y);

    pllclp(xline, yline, 2);
}

/*----------------------------------------------------------------------*\
 * void plP_draphy_poly()
 *
 * Draw polyline in physical coordinates.
 * Need to draw buffers in increments of (PL_MAXPOLY-1) since the
 * last point must be repeated (for solid lines).
\*----------------------------------------------------------------------*/

void
plP_draphy_poly(PLINT *x, PLINT *y, PLINT n)
{
    PLINT i, j, ib, ilim;

    for (ib = 0; ib < n; ib += PL_MAXPOLY - 1) {
	ilim = MIN(PL_MAXPOLY, n - ib);

	for (i = 0; i < ilim; i++) {
	    j = ib + i;
	    xline[i] = x[j];
	    yline[i] = y[j];
	}
	pllclp(xline, yline, ilim);
    }
}

/*----------------------------------------------------------------------*\
 * void plP_drawor_poly()
 *
 * Draw polyline in world coordinates.
 * Need to draw buffers in increments of (PL_MAXPOLY-1) since the
 * last point must be repeated (for solid lines).
\*----------------------------------------------------------------------*/

void
plP_drawor_poly(PLFLT *x, PLFLT *y, PLINT n)
{
    PLINT i, j, ib, ilim;

    for (ib = 0; ib < n; ib += PL_MAXPOLY - 1) {
	ilim = MIN(PL_MAXPOLY, n - ib);

	for (i = 0; i < ilim; i++) {
	    j = ib + i;
	    xline[i] = plP_wcpcx(x[j]);
	    yline[i] = plP_wcpcy(y[j]);
	}
	pllclp(xline, yline, ilim);
    }
}

/*----------------------------------------------------------------------*\
 * void pllclp()
 *
 * Draws a polyline within the clip limits.
 * Merely a front-end to plP_pllclp().
\*----------------------------------------------------------------------*/

static void
pllclp(PLINT *x, PLINT *y, PLINT npts)
{
    plP_pllclp(x, y, npts, plsc->clpxmi, plsc->clpxma,
	       plsc->clpymi, plsc->clpyma, genlin);
}

/*----------------------------------------------------------------------*\
 * void plP_pllclp()
 *
 * Draws a polyline within the clip limits.
 *
 * (AM)
 * Wanted to change the type of xclp, yclp to avoid overflows!
 * But that changes the type for the drawing routines too!
\*----------------------------------------------------------------------*/

void
plP_pllclp(PLINT *x, PLINT *y, PLINT npts,
	   PLINT xmin, PLINT xmax, PLINT ymin, PLINT ymax,
	   void (*draw) (short *, short *, PLINT))
{
    PLINT x1, x2, y1, y2;
    PLINT i, iclp = 0;
    short xclp[PL_MAXPOLY], yclp[PL_MAXPOLY];
    int drawable;

    for (i = 0; i < npts - 1; i++) {
	x1 = x[i];
	x2 = x[i + 1];
	y1 = y[i];
	y2 = y[i + 1];

	drawable = (INSIDE(x1, y1) && INSIDE(x2, y2));
	if ( ! drawable)
	    drawable = ! clipline(&x1, &y1, &x2, &y2, xmin, xmax, ymin, ymax);

	if (drawable) {

/* First point of polyline. */

	    if (iclp == 0) {
		xclp[iclp] = x1;
		yclp[iclp] = y1;
		iclp++;
		xclp[iclp] = x2;
		yclp[iclp] = y2;
	    }

/* Not first point.  Check if first point of this segment matches up to
   previous point, and if so, add it to the current polyline buffer. */

	    else if (x1 == xclp[iclp] && y1 == yclp[iclp]) {
		iclp++;
		xclp[iclp] = x2;
		yclp[iclp] = y2;
	    }

/* Otherwise it's time to start a new polyline */

	    else {
		if (iclp + 1 >= 2)
		    (*draw)(xclp, yclp, iclp + 1);
		iclp = 0;
		xclp[iclp] = x1;
		yclp[iclp] = y1;
		iclp++;
		xclp[iclp] = x2;
		yclp[iclp] = y2;
	    }
	}
    }

/* Handle remaining polyline */

    if (iclp + 1 >= 2)
	(*draw)(xclp, yclp, iclp + 1);

    plsc->currx = x[npts-1];
    plsc->curry = y[npts-1];
}

/*----------------------------------------------------------------------*\
 * int circulation()
 *
 * Returns the circulation direction for a given polyline: positive is
 * counterclockwise, negative is clockwise (right hand rule).
 *
 * Used to get the circulation of the fill polygon around the bounding box,
 * when the fill polygon is larger than the bounding box.  Counts left
 * (positive) vs right (negative) hand turns using a cross product, instead of
 * performing all the expensive trig calculations needed to get this 100%
 * correct.  For the fill cases encountered in plplot, this treatment should
 * give the correct answer most of the time, by far.  When used with plshades,
 * the typical return value is 3 or -3, since 3 turns are necessary in order
 * to complete the fill region.  Only for really oddly shaped fill regions
 * will it give the wrong answer.
 *
 * AM:
 * Changed the computation: use the outer product to compute the surface
 * area, the sign determines if the polygon is followed clockwise or
 * counterclockwise. This is more reliable. Floating-point numbers
 * are used to avoid overflow.
\*----------------------------------------------------------------------*/

static int
circulation(PLINT *x, PLINT *y, PLINT npts)
{
    PLFLT xproduct;
    int direction = 0;
    PLFLT x1, y1, x2, y2, x3, y3;
    int i;

    xproduct = 0.0 ;
    for (i = 0; i < npts - 1; i++) {
	x1 = x[i]; x2 = x[i+1];
	y1 = y[i]; y2 = y[i+1];
	if (i < npts-2) {
	    x3 = x[i+2]; y3 = y[i+2];
	} else {
	    x3 = x[0]; y3 = y[0];
	}
	xproduct = xproduct + (x2-x1)*(y3-y2) - (y2-y1)*(x3-x2);
    }

    if (xproduct > 0.0) direction = 1;
    if (xproduct < 0.0) direction = -1;
    return direction;
}

/*----------------------------------------------------------------------*\
 * void plP_plfclp()
 *
 * Fills a polygon within the clip limits.
\*----------------------------------------------------------------------*/

void
plP_plfclp(PLINT *x, PLINT *y, PLINT npts,
	   PLINT xmin, PLINT xmax, PLINT ymin, PLINT ymax,
	   void (*draw) (short *, short *, PLINT))
{
    PLINT i, x1, x2, y1, y2;
    int iclp = 0, iout = 2;
    short xclp[2*PL_MAXPOLY+2], yclp[2*PL_MAXPOLY+2];
    int drawable;
    int crossed_xmin1 = 0, crossed_xmax1 = 0;
    int crossed_ymin1 = 0, crossed_ymax1 = 0;
    int crossed_xmin2 = 0, crossed_xmax2 = 0;
    int crossed_ymin2 = 0, crossed_ymax2 = 0;

/* Must have at least 3 points and draw() specified */
    if (npts < 3 || !draw) return;

    for (i = 0; i < npts - 1; i++) {
	x1 = x[i]; x2 = x[i+1];
	y1 = y[i]; y2 = y[i+1];

	drawable = (INSIDE(x1, y1) && INSIDE(x2, y2));
	if ( ! drawable)
	    drawable = ! clipline(&x1, &y1, &x2, &y2, xmin, xmax, ymin, ymax);

	if (!drawable) {
	/* Store the edges outside the viewport */
	    xclp[iout] = x2;
	    yclp[iout] = y2;
	    iout ++;
	} else {
	/* Boundary crossing condition -- coming in. */
	    crossed_xmin2 = (x1 == xmin); crossed_xmax2 = (x1 == xmax);
	    crossed_ymin2 = (y1 == ymin); crossed_ymax2 = (y1 == ymax);
	    iout = iclp+2;

	/* If the first segment, just add it. */

	    if (iclp == 0) {
		xclp[iclp] = x1; yclp[iclp] = y1; iclp++;
		xclp[iclp] = x2; yclp[iclp] = y2; iclp++;
	    }

	/* Not first point.  If first point of this segment matches up to the
	   previous point, just add it.  */

	    else if (x1 == xclp[iclp-1] && y1 == yclp[iclp-1]) {
		xclp[iclp] = x2; yclp[iclp] = y2; iclp++;
	    }

	/* Otherwise, we need to add both points, to connect the points in the
	 * polygon along the clip boundary.  If we encircled a corner, we have
	 * to add that first.
	 */

	    else {
	    /* Treat the case where we encircled two corners:
	       Construct a polygon out of the subset of vertices
	       Note that the direction is important too when adding
	       the extra points */
		xclp[iclp+1] = x2; yclp[iclp+1] = y2;
		xclp[iclp+2] = x1; yclp[iclp+2] = y1;
		iout = iout - iclp;
	    /* Upper two */
		if ( ((crossed_xmin1 && crossed_xmax2) ||
			     (crossed_xmin2 && crossed_xmax1)) &&
			pointinpolygon(iout,&xclp[iclp+1],&yclp[iclp+1],xmin,ymax) )
		{
		    if ( crossed_xmin1 )
		    {
		        xclp[iclp] = xmin; yclp[iclp] = ymax; iclp++;
		        xclp[iclp] = xmax; yclp[iclp] = ymax; iclp++;
		    } else {
		        xclp[iclp] = xmax; yclp[iclp] = ymax; iclp++;
		        xclp[iclp] = xmin; yclp[iclp] = ymax; iclp++;
		    }
		}
	    /* Lower two */
		else if ( ((crossed_xmin1 && crossed_xmax2) ||
			          (crossed_xmin2 && crossed_xmax1)) &&
			pointinpolygon(iout,&xclp[iclp+1],&yclp[iclp+1],xmin,ymin) )
		{
		    if ( crossed_xmin1 )
		    {
		        xclp[iclp] = xmin; yclp[iclp] = ymin; iclp++;
		        xclp[iclp] = xmax; yclp[iclp] = ymin; iclp++;
		    } else {
		        xclp[iclp] = xmax; yclp[iclp] = ymin; iclp++;
		        xclp[iclp] = xmin; yclp[iclp] = ymin; iclp++;
		    }
		}
	    /* Left two */
		else if ( ((crossed_ymin1 && crossed_ymax2) ||
			          (crossed_ymin2 && crossed_ymax1)) &&
			pointinpolygon(iout,&xclp[iclp+1],&yclp[iclp+1],xmin,ymin) )
		{
		    if ( crossed_ymin1 )
		    {
		        xclp[iclp] = xmin; yclp[iclp] = ymin; iclp++;
		        xclp[iclp] = xmin; yclp[iclp] = ymax; iclp++;
		    } else {
		        xclp[iclp] = xmin; yclp[iclp] = ymax; iclp++;
		        xclp[iclp] = xmin; yclp[iclp] = ymin; iclp++;
		    }
		}
	    /* Right two */
		else if ( ((crossed_ymin1 && crossed_ymax2) ||
			          (crossed_ymin2 && crossed_ymax1)) &&
			pointinpolygon(iout,&xclp[iclp+1],&yclp[iclp+1],xmax,ymin) )
		{
		    if ( crossed_ymin1 )
		    {
		        xclp[iclp] = xmax; yclp[iclp] = ymin; iclp++;
		        xclp[iclp] = xmax; yclp[iclp] = ymax; iclp++;
		    } else {
		        xclp[iclp] = xmax; yclp[iclp] = ymax; iclp++;
		        xclp[iclp] = xmax; yclp[iclp] = ymin; iclp++;
		    }
		}
	    /* Now the case where we encircled one corner */
	    /* Lower left */
		else if ( (crossed_xmin1 && crossed_ymin2) ||
			  (crossed_ymin1 && crossed_xmin2) )
		{
		    xclp[iclp] = xmin; yclp[iclp] = ymin; iclp++;
		}
	    /* Lower right */
		else if ( (crossed_xmax1 && crossed_ymin2) ||
			  (crossed_ymin1 && crossed_xmax2) )
		{
		    xclp[iclp] = xmax; yclp[iclp] = ymin; iclp++;
		}
	    /* Upper left */
		else if ( (crossed_xmin1 && crossed_ymax2) ||
			  (crossed_ymax1 && crossed_xmin2) )
		{
		    xclp[iclp] = xmin; yclp[iclp] = ymax; iclp++;
		}
	    /* Upper right */
		else if ( (crossed_xmax1 && crossed_ymax2) ||
			  (crossed_ymax1 && crossed_xmax2) )
		{
		    xclp[iclp] = xmax; yclp[iclp] = ymax; iclp++;
		}

	    /* Now add current segment. */
		xclp[iclp] = x1; yclp[iclp] = y1; iclp++;
		xclp[iclp] = x2; yclp[iclp] = y2; iclp++;
	    }

	/* Boundary crossing condition -- going out. */
	    crossed_xmin1 = (x2 == xmin); crossed_xmax1 = (x2 == xmax);
	    crossed_ymin1 = (y2 == ymin); crossed_ymax1 = (y2 == ymax);
	}
    }

/* Limit case - all vertices are outside of bounding box.  So just fill entire
   box, *if* the bounding box is completely encircled.
*/

    if (iclp == 0) {
	PLINT xmin1, xmax1, ymin1, ymax1;
	xmin1 = xmax1 = x[0];
	ymin1 = ymax1 = y[0];
	for (i = 1; i < npts; i++) {
	    if (x[i] < xmin1) xmin1 = x[i];
	    if (x[i] > xmax1) xmax1 = x[i];
	    if (y[i] < ymin1) ymin1 = y[i];
	    if (y[i] > ymax1) ymax1 = y[i];
	}
	if (xmin1 <= xmin && xmax1 >= xmax && ymin1 <= ymin && ymax1 >= ymax ) {
	    xclp[iclp] = xmin; yclp[iclp] = ymin; iclp++;
	    xclp[iclp] = xmin; yclp[iclp] = ymax; iclp++;
	    xclp[iclp] = xmax; yclp[iclp] = ymax; iclp++;
	    xclp[iclp] = xmax; yclp[iclp] = ymin; iclp++;
	    (*draw)(xclp, yclp, iclp);
	    return;
	}
    }

/* Now handle cases where fill polygon intersects two sides of the box */

    if (iclp >= 2) {
	int debug=0;
	int dir = circulation(x, y, npts);
	if (debug) {
	    if ( (xclp[0] == xmin && xclp[iclp-1] == xmax) ||
		 (xclp[0] == xmax && xclp[iclp-1] == xmin) ||
		 (yclp[0] == ymin && yclp[iclp-1] == ymax) ||
		 (yclp[0] == ymax && yclp[iclp-1] == ymin) ||
		 (xclp[0] == xmin && yclp[iclp-1] == ymin) ||
		 (yclp[0] == ymin && xclp[iclp-1] == xmin) ||
		 (xclp[0] == xmax && yclp[iclp-1] == ymin) ||
		 (yclp[0] == ymin && xclp[iclp-1] == xmax) ||
		 (xclp[0] == xmax && yclp[iclp-1] == ymax) ||
		 (yclp[0] == ymax && xclp[iclp-1] == xmax) ||
		 (xclp[0] == xmin && yclp[iclp-1] == ymax) ||
		 (yclp[0] == ymax && xclp[iclp-1] == xmin) ) {
		printf("dir=%d, clipped points:\n", dir);
		for (i=0; i < iclp; i++)
		    printf(" x[%d]=%d y[%d]=%d", i, xclp[i], i, yclp[i]);
		printf("\n");
		printf("pre-clipped points:\n");
		for (i=0; i < npts; i++)
		    printf(" x[%d]=%d y[%d]=%d", i, x[i], i, y[i]);
		printf("\n");
	    }
	}

    /* The cases where the fill region is divided 2/2 */
    /* Divided horizontally */
	if (xclp[0] == xmin && xclp[iclp-1] == xmax)
	{
	    if (dir > 0) {
		xclp[iclp] = xmax; yclp[iclp] = ymax; iclp++;
		xclp[iclp] = xmin; yclp[iclp] = ymax; iclp++;
	    }
	    else {
		xclp[iclp] = xmax; yclp[iclp] = ymin; iclp++;
		xclp[iclp] = xmin; yclp[iclp] = ymin; iclp++;
	    }
	}
	else if (xclp[0] == xmax && xclp[iclp-1] == xmin)
	{
	    if (dir > 0) {
		xclp[iclp] = xmin; yclp[iclp] = ymin; iclp++;
		xclp[iclp] = xmax; yclp[iclp] = ymin; iclp++;
	    }
	    else {
		xclp[iclp] = xmin; yclp[iclp] = ymax; iclp++;
		xclp[iclp] = xmax; yclp[iclp] = ymax; iclp++;
	    }
	}

    /* Divided vertically */
	else if (yclp[0] == ymin && yclp[iclp-1] == ymax)
	{
	    if (dir > 0) {
		xclp[iclp] = xmin; yclp[iclp] = ymax; iclp++;
		xclp[iclp] = xmin; yclp[iclp] = ymin; iclp++;
	    }
	    else {
		xclp[iclp] = xmax; yclp[iclp] = ymax; iclp++;
		xclp[iclp] = xmax; yclp[iclp] = ymin; iclp++;
	    }
	}
	else if (yclp[0] == ymax && yclp[iclp-1] == ymin)
	{
	    if (dir > 0) {
		xclp[iclp] = xmax; yclp[iclp] = ymin; iclp++;
		xclp[iclp] = xmax; yclp[iclp] = ymax; iclp++;
	    }
	    else {
		xclp[iclp] = xmin; yclp[iclp] = ymin; iclp++;
		xclp[iclp] = xmin; yclp[iclp] = ymax; iclp++;
	    }
	}

    /* The cases where the fill region is divided 3/1 --
          LL           LR           UR           UL
       +-----+      +-----+      +-----+      +-----+
       |     |      |     |      |    \|      |/    |
       |     |      |     |      |     |      |     |
       |\    |      |    /|      |     |      |     |
       +-----+      +-----+      +-----+      +-----+

       Note when we go the long way around, if the direction is reversed the
       three vertices must be visited in the opposite order.
    */
    /* LL, short way around */
	else if ((xclp[0] == xmin && yclp[iclp-1] == ymin && dir < 0) ||
		 (yclp[0] == ymin && xclp[iclp-1] == xmin && dir > 0) )
	{
	    xclp[iclp] = xmin; yclp[iclp] = ymin; iclp++;
	}
    /* LL, long way around, counterclockwise */
	else if ((xclp[0] == xmin && yclp[iclp-1] == ymin && dir > 0))
	{
	    xclp[iclp] = xmax; yclp[iclp] = ymin; iclp++;
	    xclp[iclp] = xmax; yclp[iclp] = ymax; iclp++;
	    xclp[iclp] = xmin; yclp[iclp] = ymax; iclp++;
	}
    /* LL, long way around, clockwise */
	else if ((yclp[0] == ymin && xclp[iclp-1] == xmin && dir < 0))
	{
	    xclp[iclp] = xmin; yclp[iclp] = ymax; iclp++;
	    xclp[iclp] = xmax; yclp[iclp] = ymax; iclp++;
	    xclp[iclp] = xmax; yclp[iclp] = ymin; iclp++;
	}
    /* LR, short way around */
	else if ((xclp[0] == xmax && yclp[iclp-1] == ymin && dir > 0) ||
		 (yclp[0] == ymin && xclp[iclp-1] == xmax && dir < 0) )
	{
	    xclp[iclp] = xmax; yclp[iclp] = ymin; iclp++;
	}
    /* LR, long way around, counterclockwise */
	else if (yclp[0] == ymin && xclp[iclp-1] == xmax && dir > 0)
	{
	    xclp[iclp] = xmax; yclp[iclp] = ymax; iclp++;
	    xclp[iclp] = xmin; yclp[iclp] = ymax; iclp++;
	    xclp[iclp] = xmin; yclp[iclp] = ymin; iclp++;
	}
    /* LR, long way around, clockwise */
	else if (xclp[0] == xmax && yclp[iclp-1] == ymin && dir < 0)
	{
	    xclp[iclp] = xmin; yclp[iclp] = ymin; iclp++;
	    xclp[iclp] = xmin; yclp[iclp] = ymax; iclp++;
	    xclp[iclp] = xmax; yclp[iclp] = ymax; iclp++;
	}
    /* UR, short way around */
	else if ((xclp[0] == xmax && yclp[iclp-1] == ymax && dir < 0) ||
		 (yclp[0] == ymax && xclp[iclp-1] == xmax && dir > 0) )
	{
	    xclp[iclp] = xmax; yclp[iclp] = ymax; iclp++;
	}
    /* UR, long way around, counterclockwise */
	else if (xclp[0] == xmax && yclp[iclp-1] == ymax && dir > 0)
	{
	    xclp[iclp] = xmin; yclp[iclp] = ymax; iclp++;
	    xclp[iclp] = xmin; yclp[iclp] = ymin; iclp++;
	    xclp[iclp] = xmax; yclp[iclp] = ymin; iclp++;
	}
    /* UR, long way around, clockwise */
	else if (yclp[0] == ymax && xclp[iclp-1] == xmax && dir < 0)
	{
	    xclp[iclp] = xmax; yclp[iclp] = ymin; iclp++;
	    xclp[iclp] = xmin; yclp[iclp] = ymin; iclp++;
	    xclp[iclp] = xmin; yclp[iclp] = ymax; iclp++;
	}
    /* UL, short way around */
	else if ((xclp[0] == xmin && yclp[iclp-1] == ymax && dir > 0) ||
		 (yclp[0] == ymax && xclp[iclp-1] == xmin && dir < 0) )
	{
	    xclp[iclp] = xmin; yclp[iclp] = ymax; iclp++;
	}
    /* UL, long way around, counterclockwise */
	else if (yclp[0] == ymax && xclp[iclp-1] == xmin && dir > 0)
	{
	    xclp[iclp] = xmin; yclp[iclp] = ymin; iclp++;
	    xclp[iclp] = xmax; yclp[iclp] = ymin; iclp++;
	    xclp[iclp] = xmax; yclp[iclp] = ymax; iclp++;
	}
    /* UL, long way around, clockwise */
	else if (xclp[0] == xmin && yclp[iclp-1] == ymax && dir < 0)
	{
	    xclp[iclp] = xmax; yclp[iclp] = ymax; iclp++;
	    xclp[iclp] = xmin; yclp[iclp] = ymax; iclp++;
	    xclp[iclp] = xmin; yclp[iclp] = ymin; iclp++;
	}
    }

/* Draw the sucker */
    if (iclp >= 3)
	(*draw)(xclp, yclp, iclp);
}

/*----------------------------------------------------------------------*\
 * int clipline()
 *
 * Get clipped endpoints
\*----------------------------------------------------------------------*/

static int
clipline(PLINT *p_x1, PLINT *p_y1, PLINT *p_x2, PLINT *p_y2,
	 PLINT xmin, PLINT xmax, PLINT ymin, PLINT ymax)
{
    PLINT t, dx, dy, flipx, flipy;
    double dydx = 0, dxdy = 0;

/* If both points are outside clip region with no hope of intersection,
   return with an error */

    if ((*p_x1 <= xmin && *p_x2 <= xmin) ||
	(*p_x1 >= xmax && *p_x2 >= xmax) ||
	(*p_y1 <= ymin && *p_y2 <= ymin) ||
	(*p_y1 >= ymax && *p_y2 >= ymax))
	return 1;

    flipx = 0;
    flipy = 0;

    if (*p_x2 < *p_x1) {
	*p_x1 = 2 * xmin - *p_x1;
	*p_x2 = 2 * xmin - *p_x2;
	xmax = 2 * xmin - xmax;
	t = xmax;
	xmax = xmin;
	xmin = t;
	flipx = 1;
    }

    if (*p_y2 < *p_y1) {
	*p_y1 = 2 * ymin - *p_y1;
	*p_y2 = 2 * ymin - *p_y2;
	ymax = 2 * ymin - ymax;
	t = ymax;
	ymax = ymin;
	ymin = t;
	flipy = 1;
    }

    dx = *p_x2 - *p_x1;
    dy = *p_y2 - *p_y1;

    if (dx != 0 && dy != 0) {
	dydx = (double) dy / (double) dx;
	dxdy = 1./ dydx;
    }

    if (*p_x1 < xmin) {
	if (dx != 0 && dy != 0)
	    *p_y1 = *p_y1 + ROUND((xmin - *p_x1) * dydx);
	*p_x1 = xmin;
    }

    if (*p_y1 < ymin) {
	if (dx != 0 && dy != 0)
	    *p_x1 = *p_x1 + ROUND((ymin - *p_y1) * dxdy);
	*p_y1 = ymin;
    }

    if (*p_x1 >= xmax || *p_y1 >= ymax)
	return 1;

    if (*p_y2 > ymax) {
	if (dx != 0 && dy != 0)
	    *p_x2 = *p_x2 - ROUND((*p_y2 - ymax) * dxdy);
	*p_y2 = ymax;
    }

    if (*p_x2 > xmax) {
	if (dx != 0 && dy != 0)
	    *p_y2 = *p_y2 - ROUND((*p_x2 - xmax) * dydx);
	*p_x2 = xmax;
    }

    if (flipx) {
	*p_x1 = 2 * xmax - *p_x1;
	*p_x2 = 2 * xmax - *p_x2;
    }

    if (flipy) {
	*p_y1 = 2 * ymax - *p_y1;
	*p_y2 = 2 * ymax - *p_y2;
    }

    return 0;
}

/*----------------------------------------------------------------------*\
 * void genlin()
 *
 * General line-drawing routine.  Takes line styles into account.
 * If only 2 points are in the polyline, it is more efficient to use
 * plP_line() rather than plP_polyline().
\*----------------------------------------------------------------------*/

static void
genlin(short *x, short *y, PLINT npts)
{
/* Check for solid line */

    if (plsc->nms == 0) {
	if (npts== 2)
	    plP_line(x, y);
	else
	    plP_polyline(x, y, npts);
    }

/* Right now dashed lines don't use polyline capability -- this
   should be improved */

    else {

	PLINT i;

        /* Call escape sequence to draw dashed lines, only for drivers
	   that have this capability */
        if (plsc->dev_dash) {
	    plsc->dev_npts = npts;
	    plsc->dev_x = x;
	    plsc->dev_y = y;
	    plP_esc(PLESC_DASH, NULL);
            return;
        }

	for (i = 0; i < npts - 1; i++) {
	    grdashline(x+i, y+i);
	}
    }
}

/*----------------------------------------------------------------------*\
 * void grdashline()
 *
 * Draws a dashed line to the specified point from the previous one.
\*----------------------------------------------------------------------*/

static void
grdashline(short *x, short *y)
{
    PLINT nx, ny, nxp, nyp, incr, temp;
    PLINT modulo, dx, dy, i, xtmp, ytmp;
    PLINT tstep, pix_distance, j;
    int loop_x;
    short xl[2], yl[2];
    double nxstep, nystep;

/* Check if pattern needs to be restarted */

    if (x[0] != lastx || y[0] != lasty) {
	plsc->curel = 0;
	plsc->pendn = 1;
	plsc->timecnt = 0;
	plsc->alarm = plsc->mark[0];
    }

    lastx = xtmp = x[0];
    lasty = ytmp = y[0];

    if (x[0] == x[1] && y[0] == y[1])
	return;

    nx = x[1] - x[0];
    dx = (nx > 0) ? 1 : -1;
    nxp = ABS(nx);

    ny = y[1] - y[0];
    dy = (ny > 0) ? 1 : -1;
    nyp = ABS(ny);

    if (nyp > nxp) {
	modulo = nyp;
	incr = nxp;
	loop_x = 0;
    }
    else {
	modulo = nxp;
	incr = nyp;
	loop_x = 1;
    }

    temp = modulo / 2;

/* Compute the timer step */

    nxstep = nxp * plsc->umx;
    nystep = nyp * plsc->umy;
    tstep = sqrt( nxstep * nxstep + nystep * nystep ) / modulo;
    if (tstep < 1) tstep = 1;

    /* tstep is distance per pixel moved */

    i = 0;
    while (i < modulo) {
        pix_distance = (plsc->alarm - plsc->timecnt + tstep - 1) / tstep;
	i += pix_distance;
	if (i > modulo)
	    pix_distance -= (i - modulo);
	plsc->timecnt += pix_distance * tstep;

	temp += pix_distance * incr;
	j = temp / modulo;
	temp = temp % modulo;

	if (loop_x) {
	    xtmp += pix_distance * dx;
	    ytmp += j * dy;
	}
	else {
	    xtmp += j * dx;
	    ytmp += pix_distance * dy;
	}
	if (plsc->pendn != 0) {
	    xl[0] = lastx;
	    yl[0] = lasty;
	    xl[1] = xtmp;
	    yl[1] = ytmp;
	    plP_line(xl, yl);
	}

/* Update line style variables when alarm goes off */

	while (plsc->timecnt >= plsc->alarm) {
	    if (plsc->pendn != 0) {
		plsc->pendn = 0;
		plsc->timecnt -= plsc->alarm;
		plsc->alarm = plsc->space[plsc->curel];
	    }
	    else {
		plsc->pendn = 1;
		plsc->timecnt -= plsc->alarm;
		plsc->curel++;
		if (plsc->curel >= plsc->nms)
		    plsc->curel = 0;
		plsc->alarm = plsc->mark[plsc->curel];
	    }
	}
	lastx = xtmp;
	lasty = ytmp;
    }
}

/*----------------------------------------------------------------------*\
 * int pointinpolygon()
 *
 * Returns 1 if the point is inside the polygon, 0 otherwise
\*----------------------------------------------------------------------*/

static int
pointinpolygon( int n, short *x, short *y, PLINT xp, PLINT yp )
{
    int i;
    int count_crossings;
    PLFLT x1, y1, x2, y2, xpp, ypp, xout, yout, xmax;
    PLFLT xvp, yvp, xvv, yvv, xv1, yv1, xv2, yv2;
    PLFLT inprod1, inprod2;

    xpp = (PLFLT) xp;
    ypp = (PLFLT) yp;

    count_crossings = 0;


    /* Determine a point outside the polygon  */

    xmax = x[0] ;
    xout = x[0] ;
    yout = y[0] ;
    for ( i = 0; i < n ; i ++ ) {
        if ( xout > x[i] ) {
            xout = x[i] ;
        }
        if ( xmax < x[i] ) {
            xmax = x[i] ;
        }
    }
    xout = xout - (xmax-xout) ;

    /* Determine for each side whether the line segment between
       our two points crosses the vertex */

    xpp = (PLFLT) xp;
    ypp = (PLFLT) yp;

    xvp = xpp - xout;
    yvp = ypp - yout;

    for ( i = 0; i <= n; i ++ ) {
        x1 = (PLFLT) x[i] ;
        y1 = (PLFLT) y[i] ;
        if ( i < n ) {
            x2 = (PLFLT) x[i+1] ;
            y2 = (PLFLT) y[i+1] ;
        } else {
            x2 = (PLFLT) x[0] ;
            y2 = (PLFLT) y[0] ;
        }

        /* Skip zero-length segments */
        if ( x1 == x2 && y1 == y2 ) {
            continue;
        }

        /* Line through the two fixed points:
           Are x1 and x2 on either side? */
        xv1 = x1 - xout;
        yv1 = y1 - yout;
        xv2 = x2 - xout;
        yv2 = y2 - yout;
        inprod1 = xv1*xvp + yv1*yvp;
        inprod2 = xv2*xvp + yv2*yvp;
        if ( inprod1 * inprod2 > 0.0 ) {
            /* No crossing possible! */
            continue;
        }

        /* Line through the two vertices:
           Are xout and xpp on either side? */
        xvv = x2   - x1;
        yvv = y2   - y1;
        xv1 = xpp  - x1;
        yv1 = ypp  - y1;
        xv2 = xout - x2;
        yv2 = yout - y2;
        inprod1 = xv1*xvv + yv1*yvv;
        inprod2 = xv2*xvv + yv2*yvv;
        if ( inprod1 * inprod2 > 0.0 ) {
            /* No crossing possible! */
            continue;
        }

        /* We do have a crossing */
        count_crossings ++;
    }

    /* Return the result: an even number of crossings means the
       point is outside the polygon */

    printf( "Number of crossings: %d\n", count_crossings );
    return (count_crossings%2);
}
