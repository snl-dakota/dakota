//---------------------------------------------------------------------------//
// $Id: x05.cc 3186 2006-02-15 18:17:33Z slbrow $
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
// Implementation of PLplot example 5 in C++.
//---------------------------------------------------------------------------//

#include "plstream.h"

#include <iostream>
#include <cmath>

#ifdef USE_NAMESPACE
using namespace std;
#endif

class x05 {

public:
  x05(int, char **);

private:
  // Class data
  plstream *pls;

  static const int NPTS;

};

const int x05::NPTS = 2047;

x05::x05( int argc, char ** argv ) {

  int i;
  PLFLT *data = new PLFLT[NPTS];
  PLFLT delta;

  // plplot initialization

  pls = new plstream();

  // Parse and process command line arguments.
  pls->parseopts( &argc, argv, PL_PARSE_FULL );

  // Initialize PLplot.
  pls->init();

  // Fill up data points.

  delta = 2.0 * M_PI / (PLFLT) NPTS;
  for (i = 0; i < NPTS; i++)
    data[i] = sin(i * delta);

  pls->col0(1);
  pls->hist(NPTS, data, -1.1, 1.1, 44, 0);
  pls->col0(2);
  pls->lab( "#frValue", "#frFrequency",
	     "#frPLplot Example 5 - Probability function of Oscillator" );

  //pls.plend();

  delete[] data;

  delete pls;
}

int main( int argc, char ** argv ) {
  x05 *x = new x05( argc, argv );

  delete x;
}


//---------------------------------------------------------------------------//
//                              End of x05.cc
//---------------------------------------------------------------------------//
