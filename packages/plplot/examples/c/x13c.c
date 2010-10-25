/* $Id: x13c.c 3186 2006-02-15 18:17:33Z slbrow $

	Pie chart demo.
*/

#include "plcdemos.h"

static char *text[] =
{
    "Maurice",
    "Geoffrey",
    "Alan",
    "Rafael",
    "Vince"
};

/*--------------------------------------------------------------------------*\
 * main
 *
 * Does a simple pie chart.
\*--------------------------------------------------------------------------*/

int
main(int argc, char *argv[])
{
    int i, j, dthet, theta0, theta1, theta;
    PLFLT just, dx, dy;
    static PLFLT x[500], y[500], per[5];

    per[0] = 10.;
    per[1] = 32.;
    per[2] = 12.;
    per[3] = 30.;
    per[4] = 16.;

/* Parse and process command line arguments */

    (void) plparseopts(&argc, argv, PL_PARSE_FULL);

/* Initialize plplot */

    plinit();

    plenv(0., 10., 0., 10., 1, -2);
    plcol0(2);
    /* n.b. all theta quantities scaled by 2*pi/500 to be integers to avoid
     * floating point logic problems. */
    theta0 = 0;
    dthet = 1;
    for (i = 0; i <= 4; i++) {
	j = 0;
	x[j] = 5.;
	y[j++] = 5.;
        /* n.b. the theta quantities multiplied by 2*pi/500 afterward so
	 * in fact per is interpreted as a percentage. */
	theta1 = theta0 + 5 * per[i];
	if (i == 4)
	    theta1 = 500;
	for (theta = theta0; theta <= theta1; theta += dthet) {
	    x[j] = 5 + 3 * cos((2.*PI/500.)*theta);
	    y[j++] = 5 + 3 * sin((2.*PI/500.)*theta);
	}
	plcol0(i + 1);
	plpsty((i + 3) % 8 + 1);
	plfill(j, x, y);
	plcol0(1);
	plline(j, x, y);
	just = (2.*PI/500.)*(theta0 + theta1)/2.;
	dx = .25 * cos(just);
	dy = .25 * sin(just);
	if ((theta0 + theta1)  < 250 || (theta0 + theta1) > 750) 
	    just = 0.;
	else 
	    just = 1.;

	plptex((x[j / 2] + dx), (y[j / 2] + dy), 1.0, 0.0, just, text[i]);
	theta0 = theta - dthet;
    }
    plfont(2);
    plschr(0., 1.3);
    plptex(5.0, 9.0, 1.0, 0.0, 0.5, "Percentage of Sales");

/* Don't forget to call PLEND to finish off! */

    plend();
    exit(0);
}
