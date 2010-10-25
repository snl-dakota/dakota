/* $Id: x10c.c 3186 2006-02-15 18:17:33Z slbrow $

	Window positioning demo.
*/

#include "plcdemos.h"

/*--------------------------------------------------------------------------*\
 * main
 *
 * Demonstrates absolute positioning of graphs on a page.
\*--------------------------------------------------------------------------*/

int
main(int argc, char *argv[])
{

/* Parse and process command line arguments */

    (void) plparseopts(&argc, argv, PL_PARSE_FULL);

/* Initialize plplot */

    plinit();

    pladv(0);
    plvpor(0.0, 1.0, 0.0, 1.0);
    plwind(0.0, 1.0, 0.0, 1.0);
    plbox("bc", 0.0, 0, "bc", 0.0, 0);

    plsvpa(50.0, 150.0, 50.0, 100.0);
    plwind(0.0, 1.0, 0.0, 1.0);
    plbox("bc", 0.0, 0, "bc", 0.0, 0);
    plptex(0.5, 0.5, 1.0, 0.0, 0.5, "BOX at (50,150,50,100)");
    plend();
    exit(0);
}
