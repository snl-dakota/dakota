//---------------------------------------------------------------------------//
// $Id: x15.cc 3186 2006-02-15 18:17:33Z slbrow $
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
// Implementation of PLplot example 15 in C++.
//---------------------------------------------------------------------------//

#include "plstream.h"

#include <iostream>
#include <cmath>

#ifdef USE_NAMESPACE
using namespace std;
#endif

class x15 {

public:
  x15(int, char **);
  void plot1( PLFLT **, PLFLT, PLFLT);
  void plot2( PLFLT **, PLFLT, PLFLT);

private:
  // Class data
  plstream *pls;

  const static int XPTS;
  const static int YPTS;
  // calculated constants and array that depends on them
  const static PLFLT XSPA;
  const static PLFLT YSPA;
  const static PLFLT tr[];



};

const int x15::XPTS = 35;
const int x15::YPTS = 46;
// calculated constants and array that depends on them
const PLFLT x15::XSPA =  2./(XPTS-1);
const PLFLT x15::YSPA =  2./(YPTS-1);
const PLFLT x15::tr[] = {XSPA, 0.0, -1.0, 0.0, YSPA, -1.0};


x15::x15( int argc, char ** argv ) {

  int i, j;

  PLFLT xx;
  PLFLT yy;
  PLFLT x;
  PLFLT y;
  PLFLT **z;
  PLFLT zmin, zmax;


  // plplot initialization

  pls = new plstream();

  // Parse and process command line arguments.
  pls->parseopts( &argc, argv, PL_PARSE_FULL );

  // Initialize PLplot.
  pls->init();
  // no need to fiddle with color maps since example uses default cmap0
  // and the cmap1 fiddling that x15c.c does is completely irrelevant
  // (although interesting).

  pls->Alloc2dGrid(&z,XPTS,YPTS);

  for( i = 0; i < XPTS; i++ ) {
    xx = (PLFLT) (i - (XPTS/2)) / (PLFLT) (XPTS/2);
    for( j = 0; j < YPTS; j++ )
      {
	yy = ((PLFLT) (j - (YPTS/2)) / (PLFLT) (YPTS/2)) - 1.;
	z[i][j] = xx*xx - yy*yy + (xx - yy)/(xx*xx+yy*yy + 0.1);
	x = tr[0] * i + tr[1] * j + tr[2];
	y = tr[3] * i + tr[4] * j + tr[5];
      }
  }

  pls->MinMax2dGrid(z,XPTS,YPTS,&zmax,&zmin);

  plot1( z, zmin, zmax );
  plot2( z, zmin, zmax );

  //pls->end();

  // Tidy up allocated grids
  pls->Free2dGrid(z,XPTS,YPTS);

  delete pls;

}

// Illustrates a single shaded region.

void x15::plot1( PLFLT **z, PLFLT zmin, PLFLT zmax) {
  PLFLT shade_min, shade_max, sh_color;
  int sh_cmap = 0, sh_width;
  int min_color = 0, min_width = 0, max_color = 0, max_width = 0;

  pls->adv(0);
  pls->vpor(0.1, 0.9, 0.1, 0.9);
  pls->wind(-1.0, 1.0, -1.0, 1.0);

  /* Plot using identity transform */

  shade_min = zmin + (zmax-zmin)*0.4;
  shade_max = zmin + (zmax-zmin)*0.6;
  sh_color = 7;
  sh_width = 2;
  min_color = 9;
  max_color = 2;
  min_width = 2;
  max_width = 2;

  pls->psty(8);

  pls->shade( z, XPTS, YPTS, NULL, -1., 1., -1., 1.,
	       shade_min, shade_max,
	       sh_cmap, sh_color, sh_width,
	       min_color, min_width, max_color, max_width,
	       plstream::fill, 1, NULL, NULL );

  pls->col0(1);
  pls->box("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
  pls->col0(2);
  pls->lab("distance", "altitude", "Bogon flux");
}

// Illustrates multiple adjacent shaded regions, using different fill
// patterns for each region.

void x15::plot2(PLFLT **z, PLFLT zmin, PLFLT zmax)
{
  PLFLT shade_min, shade_max, sh_color;
  int sh_cmap = 0, sh_width;
  int min_color = 0, min_width = 0, max_color = 0, max_width = 0;
  int i;
  sh_width = 2;

  pls->adv(0);
  pls->vpor(0.1, 0.9, 0.1, 0.9);
  pls->wind(-1.0, 1.0, -1.0, 1.0);

  /* Plot using identity transform */

  for (i = 0; i < 10; i++) {
    shade_min = zmin + (zmax - zmin) * i / 10.0;
    shade_max = zmin + (zmax - zmin) * (i +1) / 10.0;
    sh_color = i+6;
    pls->psty((i + 2) % 8 + 1);

    pls->shade( z, XPTS, YPTS, NULL, -1., 1., -1., 1.,
		 shade_min, shade_max,
		 sh_cmap, sh_color, sh_width,
		 min_color, min_width, max_color, max_width,
		 plstream::fill, 1, NULL, NULL );
  }

  pls->col0(1);
  pls->box("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
  pls->col0(2);
  pls->lab("distance", "altitude", "Bogon flux");
}

int main( int argc, char ** argv ) {
  x15 *x = new x15( argc, argv );

  delete x;
}


//---------------------------------------------------------------------------//
//                              End of x15.cc
//---------------------------------------------------------------------------//
