/* $Id: x02c.c 3186 2006-02-15 18:17:33Z slbrow $

	Multiple window and color map 0 demo.
*/

#include "plcdemos.h"

/*--------------------------------------------------------------------------*\
 * main
 *
 * Demonstrates multiple windows and color map 0 palette, both default and
 * user-modified.
\*--------------------------------------------------------------------------*/

void demo1();
void demo2();
void draw_windows(int nw, int cmap0_offset);

int
main(int argc, char *argv[])
{
/* Parse and process command line arguments */
    (void) plparseopts(&argc, argv, PL_PARSE_FULL);

/* Initialize plplot */
    plinit();

/* Run demos */
    demo1();
    demo2();

    plend();
    exit(0);
}

/*--------------------------------------------------------------------------*\
 * demo1
 *
 * Demonstrates multiple windows and default color map 0 palette.
\*--------------------------------------------------------------------------*/

void demo1()
{
    plbop();

/* Divide screen into 16 regions */
    plssub(4, 4);

    draw_windows( 16, 0 );

    pleop();
}

/*--------------------------------------------------------------------------*\
 * demo2
 *
 * Demonstrates multiple windows, user-modified color map 0 palette, and
 * HLS -> RGB translation.
\*--------------------------------------------------------------------------*/

void demo2()
{
/* Set up cmap0 */
/* Use 100 custom colors in addition to base 16 */
    PLINT r[116], g[116], b[116];

/* Min & max lightness values */
    PLFLT lmin = 0.15, lmax = 0.85;

    int i;

    plbop();

/* Divide screen into 100 regions */

    plssub(10, 10);

    for (i = 0; i <= 99; i++) {
        PLFLT h, l, s;
        PLFLT r1, g1, b1;

    /* Bounds on HLS, from plhlsrgb() commentary --
     *	hue		[0., 360.]	degrees
     *	lightness	[0., 1.]	magnitude
     *	saturation	[0., 1.]	magnitude
     */

    /* Vary hue uniformly from left to right */
        h = (360. / 10. ) * ( i % 10 );
    /* Vary lightness uniformly from top to bottom, between min & max */
        l = lmin + (lmax - lmin) * (i / 10) / 9.;
    /* Use max saturation */
        s = 1.0;

        plhlsrgb(h, l, s, &r1, &g1, &b1);

        r[i+16] = r1 * 255;
        g[i+16] = g1 * 255;
        b[i+16] = b1 * 255;
    }

/* Load default cmap0 colors into our custom set */
    for (i = 0; i <= 15; i++)
        plgcol0(i, &r[i], &g[i], &b[i]);

/* Now set cmap0 all at once (faster, since fewer driver calls) */
    plscmap0(r, g, b, 116);

    draw_windows( 100, 16 );

    pleop();
}

/*--------------------------------------------------------------------------*\
 * draw_windows
 *
 * Draws a set of numbered boxes with colors according to cmap0 entry.
\*--------------------------------------------------------------------------*/

void draw_windows(int nw, int cmap0_offset)
{
    int i, j;
    PLFLT vmin, vmax;
    char text[3];

    plschr(0.0, 3.5);
    plfont(4);

    for (i = 0; i < nw; i++) {
	plcol0(i+cmap0_offset);
	sprintf(text, "%d", i);
	pladv(0);
	vmin = 0.1;
	vmax = 0.9;
	for (j = 0; j <= 2; j++) {
	    plwid(j + 1);
	    plvpor(vmin, vmax, vmin, vmax);
	    plwind(0.0, 1.0, 0.0, 1.0);
	    plbox("bc", 0.0, 0, "bc", 0.0, 0);
	    vmin = vmin + 0.1;
	    vmax = vmax - 0.1;
	}
	plwid(1);
	plptex(0.5, 0.5, 1.0, 0.0, 0.5, text);
    }
}
