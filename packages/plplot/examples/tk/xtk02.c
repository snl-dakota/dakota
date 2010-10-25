/* $Id: xtk02.c 3186 2006-02-15 18:17:33Z slbrow $

   Copyright (C) 2004  Joao Cardoso

   This file is part of PLplot.

   PLplot is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Library Public License as published
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

#include "pltk.h"
#include <math.h>

static int
AppInit(Tcl_Interp *interp);

/*--------------------------------------------------------------------------*\
 * A pithy little proc to show off how to install and use a tclMatrix
 * extension subcommand. This example is silly--only intended to show
 * how to do it.  What to do with it is your problem.  Could implement
 * subcommands for filling a matrix with special functions, performing
 * fft's, etc.
\*--------------------------------------------------------------------------*/

int stuff( tclMatrix *pm, Tcl_Interp *interp,
	   int argc, char *argv[] )
{
    int i;
    PLFLT x, y;

/* Should check that matrix is right type, size, etc. */

    for( i = 0; i < pm->n[0]; i++ ) {
	x = (PLFLT) i / pm->n[0];
	y = sin( 6.28 * 4. * i / pm->n[0] ) * x * (1. - x) * 2 +
	    2. * x * (1. - x);
	pm->fdata[i] = y;
    }

    interp->result = "Things are cool in gumbyville.";
    return TCL_OK;
}


/*--------------------------------------------------------------------------*\
 * main --
 *
 * Just a stub routine to call pltkMain.  The latter is nice to have
 * when building extended wishes, since then you don't have to rely on
 * sucking the Tk main out of libtk (which doesn't work correctly on all
 * systems/compilers/linkers/etc).  Hopefully in the future Tk will
 * supply a sufficiently capable tkMain() type function that can be used
 * instead.
\*--------------------------------------------------------------------------*/

int
main(int argc, char **argv)
{
    exit(pltkMain(argc, argv, NULL, AppInit));
}

/*
 *--------------------------------------------------------------------------
 *
 * AppInit --
 *
 *	This procedure performs application-specific initialization.
 *	Most applications, especially those that incorporate additional
 *	packages, will have their own version of this procedure.
 *
 * Results:
 *	Returns a standard Tcl completion code, and leaves an error
 *	message in interp->result if an error occurs.
 *
 * Side effects:
 *	Depends on the startup script.
 *
 * Taken from tkAppInit.c --
 *
 * Copyright (c) 1993 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *--------------------------------------------------------------------------
 */

int   myplotCmd        (ClientData, Tcl_Interp *, int, char **);

static int
AppInit(Tcl_Interp *interp)
{
    Tk_Window main;

    main = Tk_MainWindow(interp);

/*
 * Call the init procedures for included packages.  Each call should
 * look like this:
 *
 * if (Mod_Init(interp) == TCL_ERROR) {
 *     return TCL_ERROR;
 * }
 *
 * where "Mod" is the name of the module.
 */

    if (Pltk_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }

/*
 * Call Tcl_CreateCommand for application-specific commands, if
 * they weren't already created by the init procedures called above.
 */

    Tcl_CreateCommand(interp, "myplot", (Tcl_CmdProc*) myplotCmd,
                      (ClientData) main, (Tcl_CmdDeleteProc*) NULL);

    Tcl_MatrixInstallXtnsn( "stuff", stuff );

    return TCL_OK;
}

void myplot1();
void myplot2();
void myplot3();
void myplot4();

/* Plots several simple functions */
/* Note the compiler should automatically convert all non-pointer arguments
   to satisfy the prototype, but some have problems with constants. */

static PLFLT x[101], y[101];
static PLFLT xscale, yscale, xoff, yoff, xs[6], ys[6];
static PLINT space0 = 0, mark0 = 0, space1 = 1500, mark1 = 1500;

void plot1(void);
void plot2(void);
void plot3(void);

/* This has been superceeded by Tcl code in tk02 */

void myplot1()
{
/* Set up the data */
/* Original case */

    xscale = 6.;
    yscale = 1.;
    xoff = 0.;
    yoff = 0.;

/* Do a plot */

    plot1();
}

void myplot2()
{
    PLINT digmax;

/* Set up the data */

    xscale = 1.;
    yscale = 0.0014;
    yoff = 0.0185;

/* Do a plot */

    digmax = 5;
    plsyax(digmax, 0);
    plot1();
}

void myplot3()
{
    plot2();
}

void myplot4()
{
    plot3();
}

 /* =============================================================== */

void
plot1(void)
{
    int i;
    PLFLT xmin, xmax, ymin, ymax;

    for (i = 0; i < 60; i++) {
	x[i] = xoff + xscale * (i + 1) / 60.0;
	y[i] = yoff + yscale * pow(x[i], 2.);
    }

    xmin = x[0];
    xmax = x[59];
    ymin = y[0];
    ymax = y[59];

    for (i = 0; i < 6; i++) {
	xs[i] = x[i * 10 + 3];
	ys[i] = y[i * 10 + 3];
    }

/* Set up the viewport and window using PLENV. The range in X is */
/* 0.0 to 6.0, and the range in Y is 0.0 to 30.0. The axes are */
/* scaled separately (just = 0), and we just draw a labelled */
/* box (axis = 0). */

    plcol(1);
    plenv(xmin, xmax, ymin, ymax, 0, 0);
    plcol(6);
    pllab("(x)", "(y)", "#frPLplot Example 1 - y=x#u2");

/* Plot the data points */

    plcol(9);
    plpoin(6, xs, ys, 9);

/* Draw the line through the data */

    plcol(4);
    plline(60, x, y);
}

 /* =============================================================== */

void
plot2(void)
{
    int i;

/* Set up the viewport and window using PLENV. The range in X is -2.0 to
   10.0, and the range in Y is -0.4 to 2.0. The axes are scaled separately
   (just = 0), and we draw a box with axes (axis = 1). */

    plcol(1);
    plenv(-2.0, 10.0, -0.4, 1.2, 0, 1);
    plcol(2);
    pllab("(x)", "sin(x)/x", "#frPLplot Example 1 - Sinc Function");

/* Fill up the arrays */

    for (i = 0; i < 100; i++) {
	x[i] = (i - 19.0) / 6.0;
	y[i] = 1.0;
	if (x[i] != 0.0)
	    y[i] = sin(x[i]) / x[i];
    }

/* Draw the line */

    plcol(3);
    plline(100, x, y);

}

 /* =============================================================== */

void
plot3(void)
{
    int i;

/* For the final graph we wish to override the default tick intervals, and
   so do not use PLENV */

    pladv(0);

/* Use standard viewport, and define X range from 0 to 360 degrees, Y range
       from -1.2 to 1.2. */

    plvsta();
    plwind(0.0, 360.0, -1.2, 1.2);

/* Draw a box with ticks spaced 60 degrees apart in X, and 0.2 in Y. */

    plcol(1);
    plbox("bcnst", 60.0, 2, "bcnstv", 0.2, 2);

/* Superimpose a dashed line grid, with 1.5 mm marks and spaces. plstyl
   expects a pointer!! */

    plstyl(1, &mark1, &space1);
    plcol(2);
    plbox("g", 30.0, 0, "g", 0.2, 0);
    plstyl(0, &mark0, &space0);

    plcol(3);
    pllab("Angle (degrees)", "sine", "#frPLplot Example 1 - Sine function");

    for (i = 0; i < 101; i++) {
	x[i] = 3.6 * i;
	y[i] = sin(x[i] * 3.141592654 / 180.0);
    }

    plcol(4);
    plline(101, x, y);
}

int   myplotCmd( ClientData cd, Tcl_Interp *interp, int argc, char **argv )
{
    if (!strcmp(argv[1],"1"))
      myplot1();

    if (!strcmp(argv[1],"2"))
      myplot2();

    if (!strcmp(argv[1],"3"))
      myplot3();

    if (!strcmp(argv[1],"4"))
      myplot4();

    plflush();
    return TCL_OK;
}
