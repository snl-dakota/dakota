//---------------------------------------------------------------------------//
// $Id: x14.cc 3186 2006-02-15 18:17:33Z slbrow $
//---------------------------------------------------------------------------//
//
//---------------------------------------------------------------------------//
// Copyright (C) 2004  Andrew Ross <andrewr@coriolis.greenend.org.uk>
// Copyright (C) 2004  Alan W. Irwin
//
// This file is part of PLplot.
//
// PLplot is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; version 2 of the License.
//
// PLplot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with PLplot; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
//---------------------------------------------------------------------------//
//
//---------------------------------------------------------------------------//
// Implementation of PLplot example 14 in C++.
//---------------------------------------------------------------------------//

#include "plstream.h"

#include <iostream>
#include <cmath>

#ifdef USE_NAMESPACE
using namespace std;
#endif

class x14 {

public:
  x14(int, char **);
  void plot1(plstream *);
  void plot2(plstream *);
  void plot3(plstream *);
  void plot4(plstream *);
  void plot5(plstream *);
  static void mypltr(PLFLT, PLFLT, PLFLT *, PLFLT *, void *);
private:
  // Class data
  plstream *pls1, *pls2;

  PLFLT x[101], y[101];
  PLFLT xscale, yscale, xoff, yoff, xs[6], ys[6];
  static PLINT space0, mark0, space1, mark1;

  static const int xpts, ypts;
  static const PLFLT xspa, yspa;
  static const PLFLT tr[6];
};

PLINT x14::space0 = 0, x14::mark0 = 0,
  x14::space1 = 1500, x14::mark1 = 1500;

const int x14::xpts = 35;
const int x14::ypts = 46;
const PLFLT x14::xspa = 2./(x14::xpts-1);
const PLFLT x14::yspa = 2./(x14::ypts-1);

const PLFLT x14::tr[6] =
{x14::xspa, 0.0, -1.0, 0.0, x14::yspa, -1.0};

void x14::mypltr(PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, void *pltr_data)
{
    *tx = tr[0] * x + tr[1] * y + tr[2];
    *ty = tr[3] * x + tr[4] * y + tr[5];
}

static PLFLT clevel[11] =
{-1., -.8, -.6, -.4, -.2, 0, .2, .4, .6, .8, 1.};



/*--------------------------------------------------------------------------*\
 * x14()
 *
 * Plots several simple functions from other example programs.
 *
 * This version sends the output of the first 4 plots (one page) to two
 * independent streams.
\*--------------------------------------------------------------------------*/
x14::x14( int argc, char ** argv ) {
  int i, digmax;
  int xleng0 = 400, yleng0 = 300, xoff0 = 200, yoff0 = 200;
  int xleng1 = 400, yleng1 = 300, xoff1 = 500, yoff1 = 500;

  // Select either TK or DP driver and use a small window
  // Using DP results in a crash at the end due to some odd cleanup problems
  // The geometry strings MUST be in writable memory

  char driver[80];
  char geometry_master[] = "500x410+100+200";
  char geometry_slave[]  = "500x410+650+200";

  // plplot initialization

  pls1 = new plstream();

  // Parse and process command line arguments.
  pls1->parseopts( &argc, argv, PL_PARSE_FULL );

  pls1->gdev(driver);

  cout << "Demo of multiple output streams via the " <<
    driver << " driver." << endl;
  cout << "Running with the second stream as slave.\n" << endl;

  // Set up first stream

  pls1->SetOpt("geometry", geometry_master);

  pls1->sdev(driver);
  pls1->ssub(2, 2);

  // Initialize PLplot.
  pls1->init();

  pls2 = new plstream();
  // Turn off pause to make this a slave (must follow master)

  pls2->SetOpt("geometry", geometry_slave);
  pls2->spause(0);
  pls2->sdev(driver);
  pls2->init();

  // Set up the data & plot
  // Original case

  xscale = 6.;
  yscale = 1.;
  xoff = 0.;
  yoff = 0.;
  plot1(pls1);

  // Set up the data & plot

  xscale = 1.;
  yscale = 1.e+6;
  plot1(pls1);

  // Set up the data & plot

  xscale = 1.;
  yscale = 1.e-6;
  digmax = 2;
  pls1->syax(digmax, 0);
  plot1(pls1);

  // Set up the data & plot

  xscale = 1.;
  yscale = 0.0014;
  yoff = 0.0185;
  digmax = 5;
  pls1->syax(digmax, 0);
  plot1(pls1);

  // To slave
  // The eop() ensures the eop indicator gets lit.

  plot4(pls2);
  pls2->eop();

  // Back to master

  plot2(pls1);
  plot3(pls1);

  // To slave

  plot5(pls2);
  pls2->eop();

  // Back to master to wait for user to advance

  pls1->eop();

  delete pls1;
  delete pls2;

}

 /* =============================================================== */

void x14::plot1(plstream *pls)
{
    int i;
    PLFLT xmin, xmax, ymin, ymax;

    for (i = 0; i < 60; i++) {
	x[i] = xoff + xscale * (i + 1) / 60.0;
	y[i] = yoff + yscale * pow((double)x[i], 2.);
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

    pls->col0(1);
    pls->env(xmin, xmax, ymin, ymax, 0, 0);
    pls->col0(6);
    pls->lab("(x)", "(y)", "#frPLplot Example 1 - y=x#u2");

/* Plot the data points */

    pls->col0(9);
    pls->poin(6, xs, ys, 9);

/* Draw the line through the data */

    pls->col0(4);
    pls->line(60, x, y);
    pls->flush();
}


 /* =============================================================== */

void x14::plot2(plstream *pls)
{
    int i;

/* Set up the viewport and window using PLENV. The range in X is -2.0 to
       10.0, and the range in Y is -0.4 to 2.0. The axes are scaled separately
       (just = 0), and we draw a box with axes (axis = 1). */

    pls->col0(1);
    pls->env(-2.0, 10.0, -0.4, 1.2, 0, 1);
    pls->col0(2);
    pls->lab("(x)", "sin(x)/x", "#frPLplot Example 1 - Sinc Function");

/* Fill up the arrays */

    for (i = 0; i < 100; i++) {
	x[i] = (i - 19.0) / 6.0;
	y[i] = 1.0;
	if (x[i] != 0.0)
	    y[i] = sin(x[i]) / x[i];
    }

/* Draw the line */

    pls->col0(3);
    pls->line(100, x, y);
    pls->flush();
}

 /* =============================================================== */

void x14::plot3(plstream *pls)
{
    int i;

/* For the final graph we wish to override the default tick intervals, and
       so do not use PLENV */

    pls->adv(0);

/* Use standard viewport, and define X range from 0 to 360 degrees, Y range
       from -1.2 to 1.2. */

    pls->vsta();
    pls->wind(0.0, 360.0, -1.2, 1.2);

    /* Draw a box with ticks spaced 60 degrees apart in X, and 0.2 in Y. */

    pls->col0(1);
    pls->box("bcnst", 60.0, 2, "bcnstv", 0.2, 2);

    /* Superimpose a dashed line grid, with 1.5 mm marks and spaces. plstyl
       expects a pointer!! */

    pls->styl(1, &mark1, &space1);
    pls->col0(2);
    pls->box("g", 30.0, 0, "g", 0.2, 0);
    pls->styl(0, &mark0, &space0);

    pls->col0(3);
    pls->lab("Angle (degrees)", "sine", "#frPLplot Example 1 - Sine function");

    for (i = 0; i < 101; i++) {
	x[i] = 3.6 * i;
	y[i] = sin(x[i] * M_PI / 180.0);
    }

    pls->col0(4);
    pls->line(101, x, y);
    pls->flush();
}

 /* =============================================================== */

void x14::plot4(plstream *pls)
{
    int i, j;
    PLFLT dtr, theta, dx, dy, r;
    char text[3];
    PLFLT x0[361], y0[361];
    PLFLT x[361], y[361];

    dtr = M_PI / 180.0;
    for (i = 0; i <= 360; i++) {
	x0[i] = cos(dtr * i);
	y0[i] = sin(dtr * i);
    }

/* Set up viewport and window, but do not draw box */

    pls->env(-1.3, 1.3, -1.3, 1.3, 1, -2);
    for (i = 1; i <= 10; i++) {
	for (j = 0; j <= 360; j++) {
	    x[j] = 0.1 * i * x0[j];
	    y[j] = 0.1 * i * y0[j];
	}

/* Draw circles for polar grid */

	pls->line(361, x, y);
    }

    pls->col0(2);
    for (i = 0; i <= 11; i++) {
	theta = 30.0 * i;
	dx = cos(dtr * theta);
	dy = sin(dtr * theta);

/* Draw radial spokes for polar grid */

	pls->join(0.0, 0.0, dx, dy);
	sprintf(text, "%d", (int)rint(theta));

/* Write labels for angle */

//Slightly off zero to avoid floating point logic flips at 90 and 270 deg.
	if (dx >= -0.00001)
	    pls->ptex(dx, dy, dx, dy, -0.15, text);
	else
	    pls->ptex(dx, dy, -dx, -dy, 1.15, text);
    }

/* Draw the graph */

    for (i = 0; i <= 360; i++) {
	r = sin(dtr * (5 * i));
	x[i] = x0[i] * r;
	y[i] = y0[i] * r;
    }
    pls->col0(3);
    pls->line(361, x, y);

    pls->col0(4);
    pls->mtex("t", 2.0, 0.5, 0.5,
	   "#frPLplot Example 3 - r(#gh)=sin 5#gh");
    pls->flush();
}

 /* =============================================================== */

/* Demonstration of contour plotting */

void x14::plot5(plstream *pls)
{
    int i, j;
    PLFLT xx, yy;
    PLFLT **z, **w;
    static PLINT mark = 1500, space = 1500;

/* Set up function arrays */

    pls->Alloc2dGrid(&z, xpts, ypts);
    pls->Alloc2dGrid(&w, xpts, ypts);

    for (i = 0; i < xpts; i++) {
	xx = (double) (i - (xpts / 2)) / (double) (xpts / 2);
	for (j = 0; j < ypts; j++) {
	    yy = (double) (j - (ypts / 2)) / (double) (ypts / 2) - 1.0;
	    z[i][j] = xx * xx - yy * yy;
	    w[i][j] = 2 * xx * yy;
	}
    }

    pls->env(-1.0, 1.0, -1.0, 1.0, 0, 0);
    pls->col0(2);
    pls->cont(z, xpts, ypts, 1, xpts, 1, ypts, clevel, 11, x14::mypltr, NULL);
    pls->styl(1, &mark, &space);
    pls->col0(3);
    pls->cont(w, xpts, ypts, 1, xpts, 1, ypts, clevel, 11, x14::mypltr, NULL);
    pls->col0(1);
    pls->lab("X Coordinate", "Y Coordinate", "Streamlines of flow");
    pls->flush();
}


int main( int argc, char ** argv ) {
  x14 *x = new x14( argc, argv );

  delete x;
}


//---------------------------------------------------------------------------//
//                              End of x14.cc
//---------------------------------------------------------------------------//
