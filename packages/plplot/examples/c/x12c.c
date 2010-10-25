/* $Id: x12c.c 3186 2006-02-15 18:17:33Z slbrow $

	Bar chart demo.
*/

#include "plcdemos.h"

void
plfbox(PLFLT x0, PLFLT y0);

/*--------------------------------------------------------------------------*\
 * main
 *
 * Does a simple bar chart, using color fill.  If color fill is
 * unavailable, pattern fill is used instead (automatic).
\*--------------------------------------------------------------------------*/

int
main(int argc, char *argv[])
{
    int i;
    char string[20];
    PLFLT y0[10];

/* Parse and process command line arguments */

    (void) plparseopts(&argc, argv, PL_PARSE_FULL);

/* Initialize plplot */

    plinit();

    pladv(0);
    plvsta();
    plwind(1980.0, 1990.0, 0.0, 35.0);
    plbox("bc", 1.0, 0, "bcnv", 10.0, 0);
    plcol0(2);
    pllab("Year", "Widget Sales (millions)", "#frPLplot Example 12");

    y0[0] = 5;
    y0[1] = 15;
    y0[2] = 12;
    y0[3] = 24;
    y0[4] = 28;
    y0[5] = 30;
    y0[6] = 20;
    y0[7] = 8;
    y0[8] = 12;
    y0[9] = 3;

    for (i = 0; i < 10; i++) {
	plcol0(i + 1);
    /*
	plcol1((PLFLT) ((i + 1)/10.0));
	*/
	plpsty(0);
	plfbox((1980. + i), y0[i]);
	sprintf(string, "%.0f", y0[i]);
	plptex((1980. + i + .5), (y0[i] + 1.), 1.0, 0.0, .5, string);
	sprintf(string, "%d", 1980 + i);
	plmtex("b", 1.0, ((i + 1) * .1 - .05), 0.5, string);
    }

/* Don't forget to call plend() to finish off! */

    plend();
    exit(0);
}

void
plfbox(PLFLT x0, PLFLT y0)
{
    PLFLT x[4], y[4];

    x[0] = x0;
    y[0] = 0.;
    x[1] = x0;
    y[1] = y0;
    x[2] = x0 + 1.;
    y[2] = y0;
    x[3] = x0 + 1.;
    y[3] = 0.;
    plfill(4, x, y);
    plcol0(1);
    pllsty(1);
    plline(4, x, y);
}
