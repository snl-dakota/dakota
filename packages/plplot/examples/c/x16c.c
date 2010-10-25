/* $Id: x16c.c 3186 2006-02-15 18:17:33Z slbrow $

	plshade demo, using color fill.

	Maurice LeBrun
	IFS, University of Texas at Austin
	20 Mar 1994
*/

#include "plcdemos.h"

/* Fundamental settings.  See notes[] for more info. */

static int ns = 20;		/* Default number of shade levels */
static int nx = 35;		/* Default number of data points in x */
static int ny = 46;		/* Default number of data points in y */
static int exclude = 0;         /* By default do not plot a page illustrating
				 * exclusion.  API is probably going to change
				 * anyway, and cannot be reproduced by any
				 * front end other than the C one. */

/* polar plot data */
#define PERIMETERPTS 100

/* Transformation function */

PLFLT tr[6];

static void
mypltr(PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, void *pltr_data)
{
    *tx = tr[0] * x + tr[1] * y + tr[2];
    *ty = tr[3] * x + tr[4] * y + tr[5];
}

/* Function prototypes */

static void
f2mnmx(PLFLT **f, PLINT nx, PLINT ny, PLFLT *fmin, PLFLT *fmax);

/* Options data structure definition. */

static PLOptionTable options[] = {
{
    "exclude",			/* Turns on page showing exclusion */
    NULL,
    NULL,
    &exclude,
    PL_OPT_BOOL,
    "-exclude",
    "Plot the \"exclusion\" page." },
{
    "ns",			/* Number of shade levels */
    NULL,
    NULL,
    &ns,
    PL_OPT_INT,
    "-ns levels",
    "Sets number of shade levels" },
{
    "nx",			/* Number of data points in x */
    NULL,
    NULL,
    &nx,
    PL_OPT_INT,
    "-nx xpts",
    "Sets number of data points in x" },
{
    "ny",			/* Number of data points in y */
    NULL,
    NULL,
    &ny,
    PL_OPT_INT,
    "-ny ypts",
    "Sets number of data points in y" },
{
    NULL,			/* option */
    NULL,			/* handler */
    NULL,			/* client data */
    NULL,			/* address of variable to set */
    0,				/* mode flag */
    NULL,			/* short syntax */
    NULL }			/* long syntax */
};

static char *notes[] = {
"To get smoother color variation, increase ns, nx, and ny.  To get faster",
"response (especially on a serial link), decrease them.  A decent but quick",
"test results from ns around 5 and nx, ny around 25.",
NULL};

PLINT
zdefined (PLFLT x, PLFLT y)
{
  PLFLT z = sqrt(x * x + y * y);

  return z < 0.4 || z > 0.6;
}



/*--------------------------------------------------------------------------*\
 * main
 *
 * Does several shade plots using different coordinate mappings.
\*--------------------------------------------------------------------------*/

int
main(int argc, char *argv[])
{
    int i, j;
    PLFLT x, y, argx, argy, distort, r, t;
    PLFLT px[PERIMETERPTS], py[PERIMETERPTS];

    PLFLT **z, **w, zmin, zmax;
    PLFLT *clevel, *shedge, *xg1, *yg1;
    PLcGrid  cgrid1;
    PLcGrid2 cgrid2;

    PLINT fill_width = 2, cont_color = 0, cont_width = 0;

/* Parse and process command line arguments */

    plMergeOpts(options, "x16c options", notes);
    plparseopts(&argc, argv, PL_PARSE_FULL);

/* Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display */

    plscmap0n(3);

/* Initialize plplot */

    plinit();

/* Set up transformation function */

    tr[0] = 2./(nx-1);
    tr[1] = 0.0;
    tr[2] = -1.0;
    tr[3] = 0.0;
    tr[4] = 2./(ny-1);
    tr[5] = -1.0;

/* Allocate data structures */

    clevel = (PLFLT *) calloc(ns, sizeof(PLFLT));
    shedge = (PLFLT *) calloc(ns+1, sizeof(PLFLT));
    xg1 = (PLFLT *) calloc(nx, sizeof(PLFLT));
    yg1 = (PLFLT *) calloc(ny, sizeof(PLFLT));

    plAlloc2dGrid(&z, nx, ny);
    plAlloc2dGrid(&w, nx, ny);

/* Set up data array */

    for (i = 0; i < nx; i++) {
	x = (double) (i - (nx / 2)) / (double) (nx / 2);
	for (j = 0; j < ny; j++) {
	    y = (double) (j - (ny / 2)) / (double) (ny / 2) - 1.0;

	    z[i][j] = - sin(7.*x) * cos(7.*y) + x*x - y*y;
	    w[i][j] = - cos(7.*x) * sin(7.*y) + 2 * x * y;
	}
    }
    f2mnmx(z, nx, ny, &zmin, &zmax);
    for (i = 0; i < ns; i++)
	clevel[i] = zmin + (zmax - zmin) * (i + 0.5) / (PLFLT) ns;

    for (i = 0; i < ns+1; i++)
	shedge[i] = zmin + (zmax - zmin) * (PLFLT) i / (PLFLT) ns;

/* Set up coordinate grids */

    cgrid1.xg = xg1;
    cgrid1.yg = yg1;
    cgrid1.nx = nx;
    cgrid1.ny = ny;

    plAlloc2dGrid(&cgrid2.xg, nx, ny);
    plAlloc2dGrid(&cgrid2.yg, nx, ny);
    cgrid2.nx = nx;
    cgrid2.ny = ny;

    for (i = 0; i < nx; i++) {
	for (j = 0; j < ny; j++) {
	    mypltr((PLFLT) i, (PLFLT) j, &x, &y, NULL);

	    argx = x * PI/2;
	    argy = y * PI/2;
	    distort = 0.4;

	    cgrid1.xg[i] = x + distort * cos(argx);
	    cgrid1.yg[j] = y - distort * cos(argy);

	    cgrid2.xg[i][j] = x + distort * cos(argx) * cos(argy);
	    cgrid2.yg[i][j] = y - distort * cos(argx) * cos(argy);
	}
    }

/* Plot using identity transform */

    pladv(0);
    plvpor(0.1, 0.9, 0.1, 0.9);
    plwind(-1.0, 1.0, -1.0, 1.0);

    plpsty(0);

    plshades(z, nx, ny, NULL, -1., 1., -1., 1., 
	     shedge, ns+1, fill_width,
	     cont_color, cont_width,
	     plfill, 1, NULL, NULL);

    plcol0(1);
    plbox("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
    plcol0(2);
/*
    plcont(w, nx, ny, 1, nx, 1, ny, clevel, ns, mypltr, NULL);
    */
    pllab("distance", "altitude", "Bogon density");

/* Plot using 1d coordinate transform */
    
    pladv(0);
    plvpor(0.1, 0.9, 0.1, 0.9);
    plwind(-1.0, 1.0, -1.0, 1.0);

    plpsty(0);

    plshades(z, nx, ny, NULL, -1., 1., -1., 1., 
	     shedge, ns+1, fill_width,
	     cont_color, cont_width,
	     plfill, 1, pltr1, (void *) &cgrid1);

    plcol0(1);
    plbox("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
    plcol0(2);
/*
    plcont(w, nx, ny, 1, nx, 1, ny, clevel, ns, pltr1, (void *) &cgrid1);
    */
    pllab("distance", "altitude", "Bogon density");

/* Plot using 2d coordinate transform */

    pladv(0);
    plvpor(0.1, 0.9, 0.1, 0.9);
    plwind(-1.0, 1.0, -1.0, 1.0);

    plpsty(0);

    plshades(z, nx, ny, NULL, -1., 1., -1., 1., 
	     shedge, ns+1, fill_width,
	     cont_color, cont_width,
	     plfill, 0, pltr2, (void *) &cgrid2);

    plcol0(1);
    plbox("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
    plcol0(2);
    plcont(w, nx, ny, 1, nx, 1, ny, clevel, ns, pltr2, (void *) &cgrid2);

    pllab("distance", "altitude", "Bogon density, with streamlines");

/* Plot using 2d coordinate transform */

    pladv(0);
    plvpor(0.1, 0.9, 0.1, 0.9);
    plwind(-1.0, 1.0, -1.0, 1.0);

    plpsty(0);

    plshades(z, nx, ny, NULL, -1., 1., -1., 1., 
	     shedge, ns+1, fill_width,
	     2, 3,
	     plfill, 0, pltr2, (void *) &cgrid2);

    plcol0(1);
    plbox("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
    plcol0(2);
/*    plcont(w, nx, ny, 1, nx, 1, ny, clevel, ns, pltr2, (void *) &cgrid2);*/

    pllab("distance", "altitude", "Bogon density");

/* Note this exclusion API will probably change. */

/* Plot using 2d coordinate transform and exclusion*/

    if(exclude) {
    pladv(0);
    plvpor(0.1, 0.9, 0.1, 0.9);
    plwind(-1.0, 1.0, -1.0, 1.0);

    plpsty(0);

    plshades(z, nx, ny, zdefined, -1., 1., -1., 1., 
	     shedge, ns+1, fill_width,
	     cont_color, cont_width,
	     plfill, 0, pltr2, (void *) &cgrid2);

    plcol0(1);
    plbox("bcnst", 0.0, 0, "bcnstv", 0.0, 0);

    pllab("distance", "altitude", "Bogon density with exclusion");

    }
/* Example with polar coordinates. */

    pladv(0);
    plvpor( .1, .9, .1, .9 );
    plwind( -1., 1., -1., 1. );

    plpsty(0);

    /* Build new coordinate matrices. */
    
    for (i = 0; i < nx; i++) {
        r = ((PLFLT) i)/ (nx-1);
	for (j = 0; j < ny; j++) {
	   t = (2.*PI/(ny-1.))*j;
	   cgrid2.xg[i][j] = r*cos(t);
	   cgrid2.yg[i][j] = r*sin(t);
	   z[i][j] = exp(-r*r)*cos(5.*PI*r)*cos(5.*t);
	}
    }

    /* Need a new shedge to go along with the new data set. */

    f2mnmx(z, nx, ny, &zmin, &zmax);

    for (i = 0; i < ns+1; i++)
	shedge[i] = zmin + (zmax - zmin) * (PLFLT) i / (PLFLT) ns;

    /*  Now we can shade the interior region. */
    plshades(z, nx, ny, NULL, -1., 1., -1., 1., 
	     shedge, ns+1, fill_width,
	     cont_color, cont_width,
	     plfill, 0, pltr2, (void *) &cgrid2);

/* Now we can draw the perimeter.  (If do before, shade stuff may overlap.) */
      for (i = 0; i < PERIMETERPTS; i++) {
	       t = (2.*PI/(PERIMETERPTS-1))*(double)i;
	       px[i] = cos(t);
	       py[i] = sin(t);
      }
      plcol0(1);
      plline(PERIMETERPTS, px, py);
                  
      /* And label the plot.*/

      plcol0(2);
      pllab( "", "",  "Tokamak Bogon Instability" );


/* Clean up */

    free((void *) clevel);
    free((void *) shedge);
    free((void *) xg1);
    free((void *) yg1);
    plFree2dGrid(z, nx, ny);
    plFree2dGrid(w, nx, ny);
    plFree2dGrid(cgrid2.xg, nx, ny);
    plFree2dGrid(cgrid2.yg, nx, ny);

    plend();

    exit(0);
}

/*--------------------------------------------------------------------------*\
 * f2mnmx
 *
 * Returns min & max of input 2d array.
\*--------------------------------------------------------------------------*/

static void
f2mnmx(PLFLT **f, PLINT nx, PLINT ny, PLFLT *fmin, PLFLT *fmax)
{
    int i, j;

    *fmax = f[0][0];
    *fmin = *fmax;

    for (i = 0; i < nx; i++) {
	for (j = 0; j < ny; j++) {
            *fmax = MAX(*fmax, f[i][j]);
            *fmin = MIN(*fmin, f[i][j]);
	}
    }
}
