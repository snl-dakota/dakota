//---------------------------------------------------------------------------//
// $Id: x04.cc 3186 2006-02-15 18:17:33Z slbrow $
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
// Implementation of PLplot example 4 in C++.
//---------------------------------------------------------------------------//

#include "plstream.h"

#include <iostream>
#include <cmath>

#ifdef USE_NAMESPACE
using namespace std;
#endif

class x04 {

public:
  x04(int, char **);
  void plot1(int);

private:
  // Class data
  plstream *pls;


};


x04::x04( int argc, char ** argv ) {

  // plplot initialization

  pls = new plstream();

  // Parse and process command line arguments.
  pls->parseopts( &argc, argv, PL_PARSE_FULL );

  // Initialize PLplot.
  pls->init();
  pls->font(2);

  // Make log plots using two different styles.

  plot1(0);
  plot1(1);

  // pls->end();
  delete pls;
}

// Log-linear plot.

void x04::plot1( int type ) {
  int i;
  PLFLT *freql = new PLFLT[101];
  PLFLT *ampl  = new PLFLT[101];
  PLFLT *phase = new PLFLT[101];
  PLFLT f0, freq;

  pls->adv(0);

  // Set up data for log plot.

  f0 = 1.0;
  for (i = 0; i <= 100; i++) {
    freql[i] = -2.0 + i / 20.0;
    freq = pow(10.0, (double)freql[i]);
    ampl[i] = 20.0 * log10(1.0 / sqrt(1.0 + pow((double)(freq / f0), 2.)));
    phase[i] = -(180.0 / M_PI) * atan((double)(freq / f0));
  }

  pls->vpor(0.15, 0.85, 0.1, 0.9);
  pls->wind(-2.0, 3.0, -80.0, 0.0);

  // Try different axis and labelling styles.

  pls->col0(1);
  switch (type) {
  case 0:
    pls->box("bclnst", 0.0, 0, "bnstv", 0.0, 0);
    break;
  case 1:
    pls->box("bcfghlnst", 0.0, 0, "bcghnstv", 0.0, 0);
    break;
  }

  // Plot ampl vs freq.

  pls->col0(2);
  pls->line(101,freql, ampl);
  pls->col0(1);
  pls->ptex(1.6, -30.0, 1.0, -20.0, 0.5, "-20 dB/decade");

  // Put labels on.

  pls->col0(1);
  pls->mtex("b", 3.2, 0.5, 0.5, "Frequency");
  pls->mtex("t", 2.0, 0.5, 0.5, "Single Pole Low-Pass Filter");
  pls->col0(2);
  pls->mtex("l", 5.0, 0.5, 0.5, "Amplitude (dB)");

  // For the gridless case, put phase vs freq on same plot.

  if (type == 0) {
    pls->col0(1);
    pls->wind(-2.0, 3.0, -100.0, 0.0);
    pls->box("", 0.0, 0, "cmstv", 30.0, 3);
    pls->col0(3);
    pls->line(101,freql, phase);
    pls->col0(3);
    pls->mtex("r", 5.0, 0.5, 0.5, "Phase shift (degrees)");
  }

  delete[] freql;
  delete[] ampl;
  delete[] phase;
}


int main( int argc, char ** argv ) {
  x04 *x = new x04( argc, argv );

  delete x;
}


//---------------------------------------------------------------------------//
//                              End of x04.cc
//---------------------------------------------------------------------------//
