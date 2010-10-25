/* $Id: x04c.c 3186 2006-02-15 18:17:33Z slbrow $

	Log plot demo.
*/

#include "plcdemos.h"

void plot1(int type);

/*--------------------------------------------------------------------------*\
 * main
 *
 * Illustration of logarithmic axes, and redefinition of window.
\*--------------------------------------------------------------------------*/

int
main(int argc, char *argv[])
{
/* Parse and process command line arguments */

    (void) plparseopts(&argc, argv, PL_PARSE_FULL);

/* Initialize plplot */

    plinit();
    plfont(2);

/* Make log plots using two different styles. */

    plot1(0);
    plot1(1);

    plend();
    exit(0);
}

/*--------------------------------------------------------------------------*\
 * plot1
 *
 * Log-linear plot.
\*--------------------------------------------------------------------------*/

void
plot1(int type)
{
    int i;
    static PLFLT freql[101], ampl[101], phase[101];
    PLFLT f0, freq;

    pladv(0);

/* Set up data for log plot */

    f0 = 1.0;
    for (i = 0; i <= 100; i++) {
	freql[i] = -2.0 + i / 20.0;
	freq = pow(10.0, freql[i]);
	ampl[i] = 20.0 * log10(1.0 / sqrt(1.0 + pow((freq / f0), 2.)));
	phase[i] = -(180.0 / PI) * atan(freq / f0);
    }

    plvpor(0.15, 0.85, 0.1, 0.9);
    plwind(-2.0, 3.0, -80.0, 0.0);

/* Try different axis and labelling styles. */

    plcol0(1);
    switch (type) {
    case 0:
	plbox("bclnst", 0.0, 0, "bnstv", 0.0, 0);
	break;
    case 1:
	plbox("bcfghlnst", 0.0, 0, "bcghnstv", 0.0, 0);
	break;
    }

/* Plot ampl vs freq */

    plcol0(2);
    plline(101, freql, ampl);
    plcol0(1);
    plptex(1.6, -30.0, 1.0, -20.0, 0.5, "-20 dB/decade");

/* Put labels on */

    plcol0(1);
    plmtex("b", 3.2, 0.5, 0.5, "Frequency");
    plmtex("t", 2.0, 0.5, 0.5, "Single Pole Low-Pass Filter");
    plcol0(2);
    plmtex("l", 5.0, 0.5, 0.5, "Amplitude (dB)");

/* For the gridless case, put phase vs freq on same plot */

    if (type == 0) {
	plcol0(1);
	plwind(-2.0, 3.0, -100.0, 0.0);
	plbox("", 0.0, 0, "cmstv", 30.0, 3);
	plcol0(3);
	plline(101, freql, phase);
	plcol0(3);
	plmtex("r", 5.0, 0.5, 0.5, "Phase shift (degrees)");
    }
}
