//---------------------------------------------------------------------------//
// $Id: x03.cc 3186 2006-02-15 18:17:33Z slbrow $
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
// Implementation of PLplot example 3 in C++.
//---------------------------------------------------------------------------//

#include "plstream.h"

#include <iostream>
#include <cmath>

#ifdef USE_NAMESPACE
using namespace std;
#endif

class x03 {

public:
  x03(int, char **);

private:
  // Class data
  plstream *pls;


};


x03::x03( int argc, char ** argv ) {

  int i, j;
  char text[4];
  PLFLT dtr, theta, dx, dy, r;

  PLFLT *x0 = new PLFLT[361];
  PLFLT *y0 = new PLFLT[361];
  PLFLT *x  = new PLFLT[361];
  PLFLT *y  = new PLFLT[361];

  dtr = M_PI / 180.0;
  for( i = 0; i <= 360; i++ )
    {
      x0[i] = cos(dtr * i);
      y0[i] = sin(dtr * i);
    }


  // plplot initialization

  pls = new plstream();

  // Parse and process command line arguments.
  pls->parseopts( &argc, argv, PL_PARSE_FULL );

  // Initialize PLplot.
  pls->init();

  // Set up viewport and window, but do not draw box.

  pls->env(-1.3, 1.3, -1.3, 1.3, 1, -2);
  for (i = 1; i <= 10; i++) {
    for (j = 0; j <= 360; j++) {
      x[j] = 0.1 * i * x0[j];
      y[j] = 0.1 * i * y0[j];
    }

    // Draw circles for polar grid.

    pls->line(361, x, y);
  }

  pls->col0(2);
  for (i = 0; i <= 11; i++) {
    theta = 30.0 * i;
    dx = cos(dtr * theta);
    dy = sin(dtr * theta);

    // Draw radial spokes for polar grid.
    pls->join(0.0, 0.0, dx, dy);
    sprintf(text, "%d", (int)rint(theta));

    // Write labels for angle.

    //Slightly off zero to avoid floating point logic flips at 90 and 270 deg.
    if (dx >= -0.00001)
      pls->ptex(dx, dy, dx, dy, -0.15, text);
    else
      pls->ptex(dx, dy, -dx, -dy, 1.15, text);
  }

  // Draw the graph.

  for (i = 0; i <= 360; i++) {
    r = sin(dtr * (5 * i));
    x[i] = x0[i] * r;
    y[i] = y0[i] * r;
  }
  pls->col0(3);
  pls->line(361, x, y);

  pls->col0(4);
  pls->mtex("t", 2.0, 0.5, 0.5, "#frPLplot Example 3 - r(#gh)=sin 5#gh");

  // Close the plot at end.

  // pls->end();
   
  delete[] x;
  delete[] y;
  delete[] x0;
  delete[] y0;

  delete pls;

}

int main( int argc, char ** argv ) {
  x03 *x = new x03( argc, argv );

  delete x;
}


//---------------------------------------------------------------------------//
//                              End of x03.cc
//---------------------------------------------------------------------------//
