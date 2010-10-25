//---------------------------------------------------------------------------//
// $Id: x18.cc 3186 2006-02-15 18:17:33Z slbrow $
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
// Implementation of PLplot example 18 in C++.
//---------------------------------------------------------------------------//

#include "plstream.h"

#include <iostream>
#include <cmath>

#ifdef USE_NAMESPACE
using namespace std;
#endif

class x18 {

public:
  x18(int, char **);
  PLFLT THETA(int);
  PLFLT PHI(int);
  void test_poly(int);

private:
  // Class data
  plstream *pls;

  static const int NPTS;
  static const int opt[4];
  static const PLFLT alt[4];
  static const PLFLT az[4];

};


const int x18::NPTS = 1000;
const int x18::opt[] = { 1, 0, 1, 0 };
const PLFLT x18::alt[4] = {20.0, 35.0, 50.0, 65.0};
const PLFLT x18::az[4] = {30.0, 40.0, 50.0, 60.0};


x18::x18( int argc, char ** argv ) {

  int i, j, k;
  PLFLT r;
  char title[80];

  // plplot initialization

  pls = new plstream();

  // Parse and process command line arguments.
  pls->parseopts( &argc, argv, PL_PARSE_FULL );

  // Initialize PLplot.
  pls->init();

  for( k=0; k < 4; k++ )
    test_poly(k);

  PLFLT *x = new PLFLT[NPTS];
  PLFLT *y = new PLFLT[NPTS];
  PLFLT *z = new PLFLT[NPTS];

  // From the mind of a sick and twisted physicist...

  for (i = 0; i < NPTS; i++) {
    z[i] = -1. + 2. * i / NPTS;

    // Pick one ...

    // r    = 1. - ( (PLFLT) i / (PLFLT) NPTS );
    r = z[i];

    x[i] = r * cos( 2. * M_PI * 6. * i / NPTS );
    y[i] = r * sin( 2. * M_PI * 6. * i / NPTS );
  }

  for (k = 0; k < 4; k++) {
    pls->adv(0);
    pls->vpor(0.0, 1.0, 0.0, 0.9);
    pls->wind(-1.0, 1.0, -0.9, 1.1);
    pls->col0(1);
    pls->w3d(1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, alt[k], az[k]);
    pls->box3("bnstu", "x axis", 0.0, 0,
	       "bnstu", "y axis", 0.0, 0,
	       "bcdmnstuv", "z axis", 0.0, 0);

    pls->col0(2);

    if (opt[k]>0)
      pls->line3( NPTS, x, y, z );
    else
      pls->poin3( NPTS, x, y, z, 1 );

    pls->col0(3);

    sprintf(title, "#frPLplot Example 18 - Alt=%.0f, Az=%.0f",
	    alt[k], az[k]);
    pls->mtex("t", 1.0, 0.5, 0.5, title);
  }

  //pls->end();

  delete[] x;
  delete[] y;
  delete[] z;
  delete pls;

}

PLFLT x18::THETA(int a) {
  return 2. * M_PI * (PLFLT) a/20.;
}

PLFLT x18::PHI (int a) {
  return M_PI * (PLFLT) a/20.1;
}

void x18::test_poly(int k) {
  int i, j;
  PLINT draw[4][4]  = {
    { 1, 1, 1, 1 },
    { 1, 0, 1, 0 },
    { 0, 1, 0, 1 },
    { 1, 1, 0, 0 }
  };

  PLFLT *x = new PLFLT [5];
  PLFLT *y = new PLFLT [5];
  PLFLT *z = new PLFLT [5];

  pls->adv(0);
  pls->vpor(0.0, 1.0, 0.0, 0.9);
  pls->wind(-1.0, 1.0, -0.9, 1.1);
  pls->col0(1);
  pls->w3d(1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, alt[k], az[k]);
  pls->box3("bnstu", "x axis", 0.0, 0,
	     "bnstu", "y axis", 0.0, 0,
	     "bcdmnstuv", "z axis", 0.0, 0);

  pls->col0(2);

  // x = r sin(phi) cos(theta)
  // y = r sin(phi) sin(theta)
  // z = r cos(phi)
  // r = 1 :=)

  for( i=0; i < 20; i++ ) {
    for( j=0; j < 20; j++ ) {
      x[0] = sin( PHI(j) ) * cos( THETA(i) );
      y[0] = sin( PHI(j) ) * sin( THETA(i) );
      z[0] = cos( PHI(j) );

      x[1] = sin( PHI(j+1) ) * cos( THETA(i) );
      y[1] = sin( PHI(j+1) ) * sin( THETA(i) );
      z[1] = cos( PHI(j+1) );

      x[2] = sin( PHI(j+1) ) * cos( THETA(i+1) );
      y[2] = sin( PHI(j+1) ) * sin( THETA(i+1) );
      z[2] = cos( PHI(j+1) );

      x[3] = sin( PHI(j) ) * cos( THETA(i+1) );
      y[3] = sin( PHI(j) ) * sin( THETA(i+1) );
      z[3] = cos( PHI(j) );

      x[4] = sin( PHI(j) ) * cos( THETA(i) );
      y[4] = sin( PHI(j) ) * sin( THETA(i) );
      z[4] = cos( PHI(j) );

      pls->poly3(5, x, y, z, draw[k], 1 );
    }
  }

  pls->col0(3);
  pls->mtex("t", 1.0, 0.5, 0.5, "unit radius sphere" );

  delete[] x;
  delete[] y;
  delete[] z;
}

int main( int argc, char ** argv ) {
  x18 *x = new x18( argc, argv );

  delete x;
}


//---------------------------------------------------------------------------//
//                              End of x18.cc
//---------------------------------------------------------------------------//
