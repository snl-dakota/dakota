//---------------------------------------------------------------------------//
// $Id: x16.cc 3186 2006-02-15 18:17:33Z slbrow $
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
// Implementation of PLplot example 16 in C++.
//---------------------------------------------------------------------------//

#include "plstream.h"

#include <iostream>
#include <cmath>

#ifdef USE_NAMESPACE
using namespace std;
#endif

class x16 {

public:
  x16(int, char **);

private:
  // Class data
  plstream *pls;

  // number of shade levels, x data points, y data points,
  // and perimeter points.
  static int ns;
  static int nx;
  static int ny;
  static int exclude;
  const static int PERIMETERPTS;

  // calculated constants and array that depends on them
  PLFLT xspa;
  PLFLT ypsa;
public:
  static PLFLT tr[];

  static PLOptionTable options[];
  static char *notes[];

};

int x16::ns = 20;
int x16::nx = 35;
int x16::ny = 46;
int x16::exclude = 0;
const int x16::PERIMETERPTS = 100;

PLFLT x16::tr[] = {2./(nx-1), 0.0, -1.0, 0.0, 2./(ny-1), -1.0};


/* Options data structure definition. */

PLOptionTable x16::options[] = {
{
    "exclude",                  /* Turns on page showing exclusion */
    NULL,
    NULL,
    &exclude,
    PL_OPT_BOOL,
    "-exclude",
    "Plot the \"exclusion\" page." },
{
    "ns",                       /* Number of shade levels */
    NULL,
    NULL,
    &ns,
    PL_OPT_INT,
    "-ns levels",
    "Sets number of shade levels" },
{
    "nx",                       /* Number of data points in x */
    NULL,
    NULL,
    &nx,
    PL_OPT_INT,
    "-nx xpts",
    "Sets number of data points in x" },
{
    "ny",                       /* Number of data points in y */
    NULL,
    NULL,
    &ny,
    PL_OPT_INT,
    "-ny ypts",
    "Sets number of data points in y" },
{
    NULL,                       /* option */
    NULL,                       /* handler */
    NULL,                       /* client data */
    NULL,                       /* address of variable to set */
    0,                          /* mode flag */
    NULL,                       /* short syntax */
    NULL }                      /* long syntax */
};

char *x16::notes[] = {
"To get smoother color variation, increase ns, nx, and ny.  To get faster",
"response (especially on a serial link), decrease them.  A decent but quick",
"test results from ns around 5 and nx, ny around 25.",
NULL};

/* Transformation function */

static void mypltr(PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, void *pltr_data) {
  *tx = x16::tr[0] * x + x16::tr[1] * y + x16::tr[2];
  *ty = x16::tr[3] * x + x16::tr[4] * y + x16::tr[5];
}

PLINT zdefined (PLFLT x, PLFLT y) {
  PLFLT z = sqrt(x * x + y * y);

  return z < 0.4 || z > 0.6;
}

x16::x16( int argc, char ** argv ) {

  int i, j;
  PLFLT x, y, argx, argy, distort, r, t, zmin, zmax;
  PLFLT *px = new PLFLT[PERIMETERPTS];
  PLFLT *py = new PLFLT[PERIMETERPTS];
  PLFLT *xg1 = new PLFLT[nx];
  PLFLT *yg1 = new PLFLT[ny];
  PLFLT **z;
  PLFLT **w;
  PLcGrid cgrid1;
  PLcGrid2 cgrid2;
  PLFLT *clevel = new PLFLT[ns];
  PLFLT *shedge = new PLFLT[ns+1];
  const int fill_width = 2, cont_color = 0, cont_width = 0;


  // plplot initialization

  pls = new plstream();

  // Parse and process command line arguments.
  pls->MergeOpts(options, "x16c options", notes);
  pls->parseopts( &argc, argv, PL_PARSE_FULL );

  // Reduce colors in cmap 0 so that cmap 1 is useful on a
  //16-color display
  pls->scmap0n(3);


  // Initialize PLplot.
  pls->init();

  pls->Alloc2dGrid(&z,nx,ny);
  pls->Alloc2dGrid(&w,nx,ny);

  // calculated constants and array that depends on them
  PLFLT xspa =  2./(nx-1);
  PLFLT yspa =  2./(ny-1);
  PLFLT tr[] = {xspa, 0.0, -1.0, 0.0, yspa, -1.0};



  // Set up data array

  for (i = 0; i < nx; i++) {
    x = (PLFLT) (i - (nx / 2)) / (PLFLT) (nx / 2);
    for (j = 0; j < ny; j++) {
      y = (PLFLT) (j - (ny / 2)) / (PLFLT) (ny / 2) - 1.0;

      z[i][j] = - sin(7.*x) * cos(7.*y) + x*x - y*y;
      w[i][j] = - cos(7.*x) * sin(7.*y) + 2 * x * y;
    }
  }

  pls->MinMax2dGrid(z, nx, ny, &zmax, &zmin );

  for (i = 0; i < ns; i++)
    clevel[i] = zmin + (zmax - zmin) * (i + 0.5) / (PLFLT) ns;

  for (i = 0; i < ns+1; i++)
    shedge[i] = zmin + (zmax - zmin) * (PLFLT) i / (PLFLT) ns;

/* Set up coordinate grids */

  cgrid1.xg = xg1;
  cgrid1.yg = yg1;
  cgrid1.nx = nx;
  cgrid1.ny = ny;

  pls->Alloc2dGrid(&cgrid2.xg, nx, ny);
  pls->Alloc2dGrid(&cgrid2.yg, nx, ny);
  cgrid2.nx = nx;
  cgrid2.ny = ny;

  for (i = 0; i < nx; i++) {
    for (j = 0; j < ny; j++) {
      mypltr((PLFLT) i, (PLFLT) j, &x, &y, NULL);

      argx = x * M_PI/2;
      argy = y * M_PI/2;
      distort = 0.4;

      cgrid1.xg[i] = x + distort * cos(argx);
      cgrid1.yg[j] = y - distort * cos(argy);

      cgrid2.xg[i][j] = x + distort * cos(argx) * cos(argy);
      cgrid2.yg[i][j] = y - distort * cos(argx) * cos(argy);
    }
  }

  // Plot using identity transform

  pls->adv(0);
  pls->vpor(0.1, 0.9, 0.1, 0.9);
  pls->wind(-1.0, 1.0, -1.0, 1.0);

  pls->psty(0);

  pls->shades(z, nx, ny, NULL, -1., 1., -1., 1.,
	       shedge, ns+1, fill_width,
	       cont_color, cont_width,
	       plstream::fill, 1, NULL, NULL);

  pls->col0(1);
  pls->box("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
  pls->col0(2);
  pls->lab("distance", "altitude", "Bogon density");

  // Plot using 1d coordinate transform

  pls->adv(0);
  pls->vpor(0.1, 0.9, 0.1, 0.9);
  pls->wind(-1.0, 1.0, -1.0, 1.0);

  pls->psty(0);

  pls->shades(z, nx, ny, NULL, -1., 1., -1., 1.,
	       shedge, ns+1, fill_width,
	       cont_color, cont_width,
	       plstream::fill, 1, plstream::tr1, (void *) &cgrid1);

  pls->col0(1);
  pls->box("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
  pls->col0(2);

  pls->lab("distance", "altitude", "Bogon density");

  // Plot using 2d coordinate transform

  pls->adv(0);
  pls->vpor(0.1, 0.9, 0.1, 0.9);
  pls->wind(-1.0, 1.0, -1.0, 1.0);

  pls->psty(0);

  pls->shades(z, nx, ny, NULL, -1., 1., -1., 1.,
	       shedge, ns+1, fill_width,
	       cont_color, cont_width,
	       plstream::fill, 0, plstream::tr2, (void *) &cgrid2);

  pls->col0(1);
  pls->box("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
  pls->col0(2);
  pls->cont(w, nx, ny, 1, nx, 1, ny, clevel, ns, plstream::tr2, (void *) &cgrid2);

  pls->lab("distance", "altitude", "Bogon density, with streamlines");

  // Plot using 2d coordinate transform

  pls->adv(0);
  pls->vpor(0.1, 0.9, 0.1, 0.9);
  pls->wind(-1.0, 1.0, -1.0, 1.0);

  pls->psty(0);

  pls->shades(z, nx, ny, NULL, -1., 1., -1., 1.,
	       shedge, ns+1, fill_width,
	       2, 3,
	       plstream::fill, 0, plstream::tr2, (void *) &cgrid2);

  pls->col0(1);
  pls->box("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
  pls->col0(2);

  pls->lab("distance", "altitude", "Bogon density");

  // Note this exclusion API will probably change
  // Plot using 2d coordinate transform and exclusion

  if(exclude) {
    pls->adv(0);
    pls->vpor(0.1, 0.9, 0.1, 0.9);
    pls->wind(-1.0, 1.0, -1.0, 1.0);

    pls->psty(0);

    pls->shades(z, nx, ny, zdefined, -1., 1., -1., 1.,
             shedge, ns+1, fill_width,
             cont_color, cont_width,
             plstream::fill, 0, plstream::tr2, (void *) &cgrid2);

    pls->col0(1);
    pls->box("bcnst", 0.0, 0, "bcnstv", 0.0, 0);

    pls->lab("distance", "altitude", "Bogon density with exclusion");

    }

  //Example with polar coordinates.

  pls->adv(0);
  pls->vpor( .1, .9, .1, .9 );
  pls->wind( -1., 1., -1., 1. );

  pls->psty(0);

  // Build new coordinate matrices.

  for (i = 0; i < nx; i++) {
    r = ((PLFLT) i)/ (nx-1);
    for (j = 0; j < ny; j++) {
      t = (2.*M_PI/(ny-1.))*j;
      cgrid2.xg[i][j] = r*cos(t);
      cgrid2.yg[i][j] = r*sin(t);
      z[i][j] = exp(-r*r)*cos(5.*M_PI*r)*cos(5.*t);
    }
  }

  //Need a new shedge to go along with the new data set.

  pls->MinMax2dGrid(z, nx, ny, &zmax, &zmin );

  for (i = 0; i < ns+1; i++)
    shedge[i] = zmin + (zmax - zmin) * (PLFLT) i / (PLFLT) ns;

  pls->shades(z, nx, ny, NULL, -1., 1., -1., 1.,
	       shedge, ns+1, fill_width,
	       cont_color, cont_width,
	       plstream::fill, 0, plstream::tr2, (void *) &cgrid2);

  // Now we can draw the perimeter.  (If do before, shade stuff may overlap.)
  for (i = 0; i < PERIMETERPTS; i++) {
    t = (2.*M_PI/(PERIMETERPTS-1))*(PLFLT)i;
    px[i] = cos(t);
    py[i] = sin(t);
  }
  pls->col0(1);
  pls->line(PERIMETERPTS, px, py);

  // And label the plot.

  pls->col0(2);
  pls->lab( "", "",  "Tokamak Bogon Instability" );

  // Clean up

  // pls->end();

  pls->Free2dGrid(z,nx,ny);
  pls->Free2dGrid(w,nx,ny);
  pls->Free2dGrid(cgrid2.xg, nx, ny);
  pls->Free2dGrid(cgrid2.yg, nx, ny);

  delete[] px;
  delete[] py;
  delete[] xg1;
  delete[] yg1;
  delete[] clevel;
  delete[] shedge; 
  delete pls;

}

int main( int argc, char ** argv ) {
  x16 *x = new x16( argc, argv );

  delete x;
}


//---------------------------------------------------------------------------//
//                              End of x16.cc
//---------------------------------------------------------------------------//
