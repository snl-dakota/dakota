#!/usr/bin/env python

import math
from Numeric import *
from pl import *
import sys
import os

module_dir = "@MODULE_DIR@"

if module_dir[0] == '@':
	module_dir = os.getcwd ()

sys.path.insert (0, module_dir)


#define XPTS    35		/* Data points in x */
#define YPTS    46		/* Datat points in y */

xpts = 35
ypts = 46

#define XSPA    2./(XPTS-1)
#define YSPA    2./(YPTS-1)

##static PLFLT clevel[11] =
##{-1., -.8, -.6, -.4, -.2, 0, .2, .4, .6, .8, 1.};

clevel = array( [-1., -.8, -.6, -.4, -.2, 0, .2, .4, .6, .8, 1.] )

#/* Transformation function */

##PLFLT tr[6] =
##/*{XSPA, 0.0, -1.0, 0.0, YSPA, -1.0};*/
##{1.0, 0.0, 0.0, 0.0, 1.0, 0.0};
##
##void
##mypltr(PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, void *pltr_data)
##{
##    *tx = tr[0] * x + tr[1] * y + tr[2];
##    *ty = tr[3] * x + tr[4] * y + tr[5];
##}

##/*--------------------------------------------------------------------------*\
## * main
## *
## * Does several contour plots using different coordinate mappings.
##\*--------------------------------------------------------------------------*/
##
##int
##main(int argc, char *argv[])
##{

def main():
##    int i, j;
##    PLFLT xx, yy, argx, argy, distort;
##    static PLINT mark = 1500, space = 1500;

    mark = 1500
    space = 1500

##    PLFLT **z, **w;
##    PLFLT xg1[XPTS], yg1[YPTS];
##    PLcGrid  cgrid1;
##    PLcGrid2 cgrid2;
##
##/* Parse and process command line arguments */
##
##    (void) plParseOpts(&argc, argv, PL_PARSE_FULL);
##
##/* Initialize plplot */
##
##    plinit();

    plinit()

##/* Set up function arrays */

##    plAlloc2dGrid(&z, XPTS, YPTS);
##    plAlloc2dGrid(&w, XPTS, YPTS);
##
##    for (i = 0; i < XPTS; i++) {
##	xx = (double) (i - (XPTS / 2)) / (double) (XPTS / 2);
##	for (j = 0; j < YPTS; j++) {
##	    yy = (double) (j - (YPTS / 2)) / (double) (YPTS / 2) - 1.0;
##	    z[i][j] = xx * xx - yy * yy;
##	    w[i][j] = 2 * xx * yy;
##	}
##    }

    z = zeros( xpts, ypts )
    w = zeros( xpts, ypts )

    for i in range(xpts):
	xx = (i - .5*xpts) / (.5*xpts)
	for j in range(ypts):
	    yy = (j - .5*ypts) / (.5*ypts) - 1.
	    z[i,j] = xx * xx - yy * yy
	    w[i,j] = 2. * xx * yy

##/* Set up grids */
##
##    cgrid1.xg = xg1;
##    cgrid1.yg = yg1;
##    cgrid1.nx = XPTS;
##    cgrid1.ny = YPTS;
##
##    plAlloc2dGrid(&cgrid2.xg, XPTS, YPTS);
##    plAlloc2dGrid(&cgrid2.yg, XPTS, YPTS);
##    cgrid2.nx = XPTS;
##    cgrid2.ny = YPTS;
##
##    for (i = 0; i < XPTS; i++) {
##	for (j = 0; j < YPTS; j++) {
##	    mypltr((PLFLT) i, (PLFLT) j, &xx, &yy, NULL);
##
##	    argx = xx * PI/2;
##	    argy = yy * PI/2;
##	    distort = 0.4;
##
##	    cgrid1.xg[i] = xx + distort * cos(argx);
##	    cgrid1.yg[j] = yy - distort * cos(argy);
##
##	    cgrid2.xg[i][j] = xx + distort * cos(argx) * cos(argy);
##	    cgrid2.yg[i][j] = yy - distort * cos(argx) * cos(argy);
##	}
##    }
##
##/* Plot using identity transform */
##
##/*    plenv(-1.0, 1.0, -1.0, 1.0, 0, 0);
## */
##    plenv(.0, 1.*XPTS, .0, 1.*YPTS, 0, 0);
##    plcol(2);
##/*    plcont(z, XPTS, YPTS, 1, XPTS, 1, YPTS, clevel, 11, mypltr, NULL);
## */
##    plcont(z, XPTS, YPTS, 1, XPTS, 1, YPTS, clevel, 11, pltr0, NULL);
##    plstyl(1, &mark, &space);
##    plcol(3);
##/*    plcont(w, XPTS, YPTS, 1, XPTS, 1, YPTS, clevel, 11, mypltr, NULL);
## */
##    plcont(w, XPTS, YPTS, 1, XPTS, 1, YPTS, clevel, 11, pltr0, NULL);
##    plstyl(0, &mark, &space);
##    plcol(1);
##    pllab("X Coordinate", "Y Coordinate", "Streamlines of flow");

    plenv( 0., 1.*xpts, 0., 1.*ypts, 0, 0 )
    plcol(2)
    plcont2( z, 1, xpts, 1, ypts, clevel )

    plstyl( 1, mark, space )
    plcol(3)
    plcont2( w, 1, 1.*xpts, 1, 1.*ypts, clevel )
    plstyl( 0, mark, space )

    # other stuff

    plcol(1)
    pllab("X Coordinate", "Y Coordinate", "Streamlines of flow");

    pleop()

main()
