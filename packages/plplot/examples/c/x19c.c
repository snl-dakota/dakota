/* $Id: x19c.c 3186 2006-02-15 18:17:33Z slbrow $

	Illustrates backdrop plotting of world, US maps.
	Contributed by Wesley Ebisuzaki.
*/

#include "plcdemos.h"

/*--------------------------------------------------------------------------*\
 * mapform19
 *
 * Defines specific coordinate transformation for example 19.
 * Not to be confused with mapform in src/plmap.c.
 * x[], y[] are the coordinates to be plotted.
\*--------------------------------------------------------------------------*/

void 
mapform19(PLINT n, PLFLT *x, PLFLT *y) 
{
    int i;
    double xp, yp, radius;
    for (i = 0; i < n; i++) {
	radius = 90.0 - y[i];
	xp = radius * cos(x[i] * PI / 180.0);
	yp = radius * sin(x[i] * PI / 180.0);
	x[i] = xp;
	y[i] = yp;
    }	
}

/*--------------------------------------------------------------------------*\
 * main
 *
 * Shows two views of the world map.
\*--------------------------------------------------------------------------*/

int 
main(int argc, char **argv) 
{
    PLFLT minx, maxx, miny, maxy;
    int c;

/* Parse and process command line arguments */

    (void) plparseopts(&argc, argv, PL_PARSE_FULL);

/* Longitude (x) and latitude (y) */

    miny = -70;
    maxy = 80;

    plinit();

/* Cartesian plots */
/* Most of world */

    minx = 190;
    maxx = 190+360;

    plcol0(1);
    plenv(minx, maxx, miny, maxy, 1, -1);
    plmap(NULL, "usaglobe", minx, maxx, miny, maxy);

/* The Americas */

    minx = 190;
    maxx = 340;

    plcol0(1);
    plenv(minx, maxx, miny, maxy, 1, -1);
    plmap(NULL, "usaglobe", minx, maxx, miny, maxy);

/* Polar, Northern hemisphere */

    minx = 0;
    maxx = 360;

    plenv(-75., 75., -75., 75., 1, -1);
    plmap(mapform19,"globe", minx, maxx, miny, maxy);

    pllsty(2);
    plmeridians(mapform19,10.0, 10.0, 0.0, 360.0, -10.0, 80.0);
    plend();
    exit(0);
}
