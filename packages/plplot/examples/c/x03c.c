/* $Id: x03c.c 3186 2006-02-15 18:17:33Z slbrow $

	Polar plot demo.
*/

#include "plcdemos.h"

/*--------------------------------------------------------------------------*\
 * main
 *
 * Generates polar plot, with 1-1 scaling.
\*--------------------------------------------------------------------------*/

int
main(int argc, char *argv[])
{
    int i, j;
    PLFLT dtr, theta, dx, dy, r;
    char text[4];
    static PLFLT x0[361], y0[361];
    static PLFLT x[361], y[361];

    dtr = PI / 180.0;
    for (i = 0; i <= 360; i++) {
	x0[i] = cos(dtr * i);
	y0[i] = sin(dtr * i);
    }

/* Parse and process command line arguments */

    (void) plparseopts(&argc, argv, PL_PARSE_FULL);

/* Initialize plplot */

    plinit();

/* Set up viewport and window, but do not draw box */

    plenv(-1.3, 1.3, -1.3, 1.3, 1, -2);
    for (i = 1; i <= 10; i++) {
	for (j = 0; j <= 360; j++) {
	    x[j] = 0.1 * i * x0[j];
	    y[j] = 0.1 * i * y0[j];
	}

    /* Draw circles for polar grid */

	plline(361, x, y);
    }

    plcol0(2);
    for (i = 0; i <= 11; i++) {
	theta = 30.0 * i;
	dx = cos(dtr * theta);
	dy = sin(dtr * theta);

    /* Draw radial spokes for polar grid */

	pljoin(0.0, 0.0, dx, dy);
	sprintf(text, "%d", ROUND(theta));

    /* Write labels for angle */

/* Slightly off zero to avoid floating point logic flips at 90 and 270 deg. */
	if (dx >= -0.00001)
	    plptex(dx, dy, dx, dy, -0.15, text);
	else
	    plptex(dx, dy, -dx, -dy, 1.15, text);
    }

/* Draw the graph */

    for (i = 0; i <= 360; i++) {
	r = sin(dtr * (5 * i));
	x[i] = x0[i] * r;
	y[i] = y0[i] * r;
    }
    plcol0(3);
    plline(361, x, y);

    plcol0(4);
    plmtex("t", 2.0, 0.5, 0.5, "#frPLplot Example 3 - r(#gh)=sin 5#gh");

/* Close the plot at end */

    plend();
    exit(0);
}
