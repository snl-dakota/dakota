//---------------------------------------------------------------------------//
// $Id: x19.cc 3186 2006-02-15 18:17:33Z slbrow $
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
// Implementation of PLplot example 19 in C++.
//---------------------------------------------------------------------------//

#include "plstream.h"

#include <iostream>
#include <cmath>

#ifdef USE_NAMESPACE
using namespace std;
#endif

class x19 {

public:
  x19(int, char **);

private:
  // Class data
  plstream *pls;


};

//--------------------------------------------------------------------------
// mapform19
//
// Defines specific coordinate transformation for example 19.
// Not to be confused with mapform in src/plmap.c.
// x[], y[] are the coordinates to be plotted.
//--------------------------------------------------------------------------

void
mapform19(PLINT n, PLFLT *x, PLFLT *y)
{
    int i;
    double xp, yp, radius;
    for (i = 0; i < n; i++) {
        radius = 90.0 - y[i];
        xp = radius * cos(x[i] * M_PI / 180.0);
        yp = radius * sin(x[i] * M_PI / 180.0);
        x[i] = xp;
        y[i] = yp;
    }
}


x19::x19( int argc, char ** argv ) {

  PLFLT minx, maxx, miny, maxy;
  int c;

  // plplot initialization

  pls = new plstream();

  // Parse and process command line arguments.
  pls->parseopts( &argc, argv, PL_PARSE_FULL );

  // Longitude (x) and latitude (y)

  miny = -70;
  maxy = 80;

  // Initialize PLplot.
  pls->init();
  // Cartesian plots
  // Most of world

  minx = 190;
  maxx = 190+360;

  pls->col0(1);
  pls->env(minx, maxx, miny, maxy, 1, -1);
  pls->map(NULL, "usaglobe", minx, maxx, miny, maxy);

  // The Americas

  minx = 190;
  maxx = 340;

  pls->col0(1);
  pls->env(minx, maxx, miny, maxy, 1, -1);
  pls->map(NULL, "usaglobe", minx, maxx, miny, maxy);

  // Polar, Northern hemisphere
  minx = 0;
  maxx = 360;

  pls->env(-75., 75., -75., 75., 1, -1);
  pls->map(mapform19,"globe", minx, maxx, miny, maxy);

  pls->lsty(2);
  pls->meridians(mapform19,10.0, 10.0, 0.0, 360.0, -10.0, 80.0);

  delete pls;

}

int main( int argc, char ** argv ) {
  x19 *x = new x19( argc, argv );

  delete x;
}


//---------------------------------------------------------------------------//
//                              End of x19.cc
//---------------------------------------------------------------------------//
