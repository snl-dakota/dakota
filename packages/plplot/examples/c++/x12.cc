//---------------------------------------------------------------------------//
// $Id: x12.cc 3186 2006-02-15 18:17:33Z slbrow $
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
// Implementation of PLplot example 12 in C++.
//---------------------------------------------------------------------------//

#include "plstream.h"

#include <iostream>
#include <cmath>

#ifdef USE_NAMESPACE
using namespace std;
#endif

class x12 {

public:
  x12(int, char**);
  void plfbox(PLFLT, PLFLT);
private:
  plstream *pls;

  static const PLFLT y0[];


};

const PLFLT x12::y0[10] = {5., 15., 12., 24., 28., 30., 20., 8., 12., 3.};

x12::x12( int argc, char **argv ) {

  int i;
  char string[20];

  pls = new plstream();

  // Parse and process command line arguments.

  pls->parseopts( &argc, argv, PL_PARSE_FULL );


  // Initialize plplot.

  pls->init();



  pls->adv(0);
  pls->vsta();
  pls->wind(1980.0, 1990.0, 0.0, 35.0);
  pls->box("bc", 1.0, 0, "bcnv", 10.0, 0);
  pls->col0(2);
  pls->lab("Year", "Widget Sales (millions)", "#frPLplot Example 12");
  for (i = 0; i < 10; i++) {
    pls->col0(i + 1);
    //             pls->col1((PLFLT) ((i + 1)/10.0));
    pls->psty(0);
    plfbox((1980. + i), y0[i]);
    sprintf(string, "%.0f", y0[i]);
    pls->ptex((1980. + i + .5), (y0[i] + 1.), 1.0, 0.0, .5, string);
    sprintf(string, "%d", 1980 + i);
    pls->mtex("b", 1.0, ((i + 1) * .1 - .05), 0.5, string);
  }
  //pls->end();
  delete pls;
}

void x12::plfbox(PLFLT x0, PLFLT y0) {
  PLFLT *x = new PLFLT[4];
  PLFLT *y = new PLFLT[4];

  x[0] = x0;
  y[0] = 0.;
  x[1] = x0;
  y[1] = y0;
  x[2] = x0 + 1.;
  y[2] = y0;
  x[3] = x0 + 1.;
  y[3] = 0.;
  pls->fill(4, x, y);
  pls->col0(1);
  pls->lsty(1);
  pls->line(4, x, y);

  delete[] x;
  delete[] y;
}



int main( int argc, char **argv )
{
  x12 *x = new x12( argc, argv );

  delete x;
}


//---------------------------------------------------------------------------//
//                              End of x12.cc
//---------------------------------------------------------------------------//
