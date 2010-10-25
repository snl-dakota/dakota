//---------------------------------------------------------------------------//
// $Id: x13.cc 3186 2006-02-15 18:17:33Z slbrow $
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
// Implementation of PLplot example 13 in C++.
//---------------------------------------------------------------------------//

#include "plstream.h"

#include <iostream>
#include <cmath>

#ifdef USE_NAMESPACE
using namespace std;
#endif

static char* text[] = {"Maurice", "Geoffrey", "Alan", "Rafael", "Vince"};
static int per[] = {10, 32, 12, 30, 16};

class x13 {

public:
  x13(int, char **);

private:
  // Class data
  plstream *pls;


};


x13::x13( int argc, char ** argv ) {

  int i, j, dthet, theta0, theta1, theta;
  PLFLT just, dx, dy;
  PLFLT *x = new PLFLT[ 500 ];
  PLFLT *y = new PLFLT[ 500 ];


  // plplot initialization

  pls = new plstream();

  // Parse and process command line arguments.
  pls->parseopts( &argc, argv, PL_PARSE_FULL );

  // Initialize PLplot.
  pls->init();

  pls->env(0., 10., 0., 10., 1, -2);
  pls->col0(2);
  // n.b. all theta quantities scaled by 2*pi/500 to be integers to avoid
  // floating point logic problems.
  theta0 = 0;
  dthet = 1;
  for (i = 0; i <= 4; i++) {
    j = 0;
    x[j] = 5.;
    y[j++] = 5.;
    // n.b. the theta quantities multiplied by 2*pi/500 afterward so
    // in fact per is interpreted as a percentage.
    theta1 = theta0 + 5 * per[i];
    if (i == 4)
      theta1 = 500;
    for (theta = theta0; theta <= theta1; theta += dthet) {
      x[j] = 5 + 3 * cos((2.*M_PI/500.)*theta);
      y[j++] = 5 + 3 * sin((2.*M_PI/500.)*theta);
    }
    pls->col0(i + 1);
    pls->psty((i + 3) % 8 + 1);
    pls->fill(j,x, y);
    pls->col0(1);
    pls->line(j, x, y);
    just = (2.*M_PI/500.)*(theta0 + theta1)/2.;
    dx = .25 * cos(just);
    dy = .25 * sin(just);
    if ((theta0 + theta1)  < 250 || (theta0 + theta1) > 750)
      just = 0.;
    else
      just = 1.;

    pls->ptex((x[j / 2] + dx), (y[j / 2] + dy), 1.0, 0.0, just, text[i]);
    theta0 = theta - dthet;

  }
  pls->font(2);
  pls->schr(0., 1.3);
  pls->ptex(5.0, 9.0, 1.0, 0.0, 0.5, "Percentage of Sales");
  // pls->end();

  delete[] x;
  delete[] y;
  delete pls;

}

int main( int argc, char ** argv ) {
  x13 *x = new x13( argc, argv );

  delete x;
}


//---------------------------------------------------------------------------//
//                              End of x13.cc
//---------------------------------------------------------------------------//
