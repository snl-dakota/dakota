//---------------------------------------------------------------------------//
// $Id: x07.cc 3186 2006-02-15 18:17:33Z slbrow $
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
// Implementation of PLplot example 07 in C++.
//---------------------------------------------------------------------------//

#include "plstream.h"

#include <iostream>
#include <cmath>

#ifdef USE_NAMESPACE
using namespace std;
#endif

static int base[17] = {
  0, 200, 500, 600, 700, 800, 900,
  2000, 2100, 2200, 2300, 2400, 2500, 2600, 2700, 2800, 2900
};

class x07 {

public:
  x07(int, char **);

private:
  // Class data
  plstream *pls;


};


x07::x07( int argc, char ** argv ) {

  int i, j, k, l;
  PLFLT x, y;
  char text[10];

  // plplot initialization

  pls = new plstream();

  // Parse and process command line arguments.
  pls->parseopts( &argc, argv, PL_PARSE_FULL );

  // Initialize PLplot.
  pls->init();

  pls->fontld(1);
  for( l = 0; l < 17; l++ ) {
    pls->adv(0);

    // Set up viewport and window.

    pls->col0(2);
    pls->vpor(0.15, 0.95, 0.1, 0.9);
    pls->wind(0.0, 1.0, 0.0, 1.0);

    // Draw the grid using plbox.

    pls->box("bcg", 0.1, 0, "bcg", 0.1, 0);

    // Write the digits below the frame.

    pls->col0(15);
    for (i = 0; i <= 9; i++) {
      sprintf(text,"%d",i);
      pls->mtex("b", 1.5, (0.1 * i + 0.05), 0.5, text);
    }

    k = 0;
    for( i = 0; i <= 9; i++ ) {

      // Write the digits to the left of the frame.

      sprintf(text,"%d",base[l]+10*i);
      pls->mtex("lv", 1.0, (0.95 - 0.1 * i), 1.0, text);
      for( j = 0; j <= 9; j++ ) {
	x = 0.1 * j + 0.05;
	y = 0.95 - 0.1 * i;

	// Display the symbols.
	// N.B. plsym expects arrays so that is what we give it.

	pls->sym( 1, &x, &y, base[l] + k );
	k = k + 1;
      }
    }

    pls->mtex("t", 1.5, 0.5, 0.5, "PLplot Example 7 - PLSYM symbols");
  }
  // pls->end();
  delete pls;
}

int main( int argc, char ** argv ) {
  x07 *x = new x07( argc, argv );

  delete x;
}


//---------------------------------------------------------------------------//
//                              End of x07.cc
//---------------------------------------------------------------------------//
