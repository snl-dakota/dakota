//---------------------------------------------------------------------------//
// $Id: x11.cc 3186 2006-02-15 18:17:33Z slbrow $
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
// Implementation of PLplot example 11 in C++.
//---------------------------------------------------------------------------//

#include "plstream.h"

#include <iostream>
#include <cmath>

#ifdef USE_NAMESPACE
using namespace std;
#endif

class x11 {

public:
  x11(int, char**);
  void cmap1_init();

private:
  plstream *pls;

  static const int XPTS;
  static const int YPTS;
  static const int LEVELS;
  static const int opt[];
  static const PLFLT alt[];
  static const PLFLT az[];

  static const char *title[];

};


const int x11::XPTS = 35;
const int x11::YPTS = 46;
const int x11::LEVELS = 10;
const int x11::opt[] = {3, 3};
const PLFLT x11::alt[] = {33.0, 17.0};
const PLFLT x11::az[] = {24.0, 115.0};

const char *x11::title[] = {
  "#frPLplot Example 11 - Alt=33, Az=24, Opt=3",
  "#frPLplot Example 11 - Alt=17, Az=115, Opt=3"
};



void x11::cmap1_init()
{
  PLFLT *i = new PLFLT[2];
  PLFLT *h = new PLFLT[2];
  PLFLT *l = new PLFLT[2];
  PLFLT *s = new PLFLT[2];
  PLINT *rev = new PLINT[2];

  i[0] = 0.0;         // left boundary
  i[1] = 1.0;         // right boundary

  h[0] = 240; /* blue -> green -> yellow -> */
  h[1] = 0;   /* -> red */

  l[0] = 0.6;
  l[1] = 0.6;

  s[0] = 0.8;
  s[1] = 0.8;

  rev[0] = 0;         // interpolate on front side of colour wheel.
  rev[1] = 0;         // interpolate on front side of colour wheel.

  pls->scmap1n(256);
  pls->scmap1l(0, 2, i, h, l, s, rev);

  delete[] i;
  delete[] h;
  delete[] l;
  delete[] s;
  delete[] rev;
}


x11::x11( int argc, char **argv ) {

  int i, j, k;

  PLFLT *x = new PLFLT[ XPTS ];
  PLFLT *y = new PLFLT[ YPTS ];
  PLFLT **z;
  PLFLT zmin = 1E9, zmax = -1E-9;

  PLFLT xx, yy;
  int nlevel = LEVELS;
  PLFLT *clevel = new PLFLT[LEVELS];
  PLFLT step;



  pls = new plstream();

  // Parse and process command line arguments.

  pls->parseopts( &argc, argv, PL_PARSE_FULL );


  // Initialize plplot.

  pls->init();

  //  pls->Alloc2dGrid(&z, XPTS, YPTS);
  pls->Alloc2dGrid(&z, XPTS, YPTS);

  for( i=0; i < XPTS; i++ )
    x[i] = 3.* (PLFLT) (i - (XPTS/2)) / (PLFLT) (XPTS/2);

  for( j=0; j < YPTS; j++ )
    y[j] = 3.* (PLFLT) (j - (YPTS/2)) / (PLFLT) (YPTS/2);

  for( i = 0; i < XPTS; i++ )
    {
      xx = x[i];
      for( j = 0; j < YPTS; j++ )
	{
	  yy = y[j];
	  z[i][j] = 3. * (1.-xx)*(1.-xx) * exp(-(xx*xx) - (yy+1.)*(yy+1.)) -
	    10. * (xx/5. - pow((double)xx,3.) - pow((double)yy,5.)) * exp(-xx*xx-yy*yy) -
	    1./3. * exp(-(xx+1)*(xx+1) - (yy*yy));
	  if(false) { /* Jungfraujoch/Interlaken */
	    if (z[i][j] < -1.)
	      z[i][j] = -1.;
	  }
	  if (zmin > z[i][j])
	    zmin = z[i][j];
	  if (zmax < z[i][j])
	    zmax = z[i][j];
	}
    }

  step = (zmax - zmin)/(nlevel+1);
  for (i=0; i<nlevel; i++)
    clevel[i] = zmin + step + step*i;

  cmap1_init();
  for( k = 0; k < 2; k++ ) {
    for ( i = 0; i < 4; i++) {
      pls->adv(0);
      pls->col0(1);
      pls->vpor(0.0, 1.0, 0.0, 0.9);
      pls->wind(-1.0, 1.0, -1.0, 1.5);

      pls->w3d( 1.0, 1.0, 1.2, -3.0, 3.0, -3.0, 3.0, zmin, zmax,
		alt[k], az[k] );
      pls->box3( "bnstu", "x axis", 0.0, 0,
		 "bnstu", "y axis", 0.0, 0,
		 "bcdmnstuv", "z axis", 0.0, 4 );

      pls->col0(2);

      /* wireframe plot */
      if (i==0)
	pls->mesh(x, y, z, XPTS, YPTS, opt[k]);

      /* magnitude colored wireframe plot */
      else if (i==1)
	pls->mesh(x, y, z, XPTS, YPTS, opt[k] | MAG_COLOR);

      /* magnitude colored wireframe plot with sides */
      else if (i==2) {
	pls->plot3d(x, y, z, XPTS, YPTS, opt[k] | MAG_COLOR, 1);
      }

      /* magnitude colored wireframe plot with base contour */
      else if (i==3)
	pls->meshc(x, y, z, XPTS, YPTS, opt[k] | MAG_COLOR | BASE_CONT,
		   clevel, nlevel);


      pls->col0(3);
      pls->mtex("t", 1.0, 0.5, 0.5, title[k]);
    }
  }

  //pls->end();

  pls->Free2dGrid(z, XPTS, YPTS);

  delete[] x;
  delete[] y;
  delete[] clevel;

  delete pls;
}

// Does a series of mesh plots for a given data set, with different viewing
// options in each plot.


int main( int argc, char **argv )
{
  x11 *x = new x11( argc, argv );

  delete x;
}


//---------------------------------------------------------------------------//
//                              End of x11.cc
//---------------------------------------------------------------------------//
