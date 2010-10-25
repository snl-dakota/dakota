//---------------------------------------------------------------------------//
// $Id: x08.cc 3186 2006-02-15 18:17:33Z slbrow $
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
// Implementation of PLplot example 8 in C++.
//---------------------------------------------------------------------------//

#include "plstream.h"

#include <iostream>
#include <cmath>

#ifdef USE_NAMESPACE
using namespace std;
#endif

class x08 {

public:
  x08(int, char**);
  void cmap1_init(int);

private:
  plstream *pls;

  static const int XPTS;
  static const int YPTS;
  static PLFLT alt[];
  static PLFLT az[];
  static char *title[];

  static int sombrero;

  static PLOptionTable options[];
};


const int x08::XPTS = 35;
const int x08::YPTS = 46;
PLFLT x08::alt[] = {60.0, 20.0};
PLFLT x08::az[] = {30.0, 60.0};

char *x08::title[] = {
  "#frPLplot Example 8 - Alt=60, Az=30",
  "#frPLplot Example 8 - Alt=20, Az=60",
};

PLOptionTable x08::options[] = {
  {
    "sombrero",                 /* Turns on use of Sombrero function */
    NULL,
    NULL,
    &x08::sombrero,
    PL_OPT_BOOL,
    "-sombrero",
    "Use the \"sombrero\" function." },
  {
    NULL,                       /* option */
    NULL,                       /* handler */
    NULL,                       /* client data */
    NULL,                       /* address of variable to set */
    0,                          /* mode flag */
      NULL,                       /* short syntax */
    NULL }                      /* long syntax */
};


int x08::sombrero = 0;

// cmap1_init1

// Initializes color map 1 in HLS space.
// Basic grayscale variation from half-dark (which makes more interesting
// looking plot compared to dark) to light.
// An interesting variation on this:
// s[1] = 1.0

void x08::cmap1_init(int gray)
{
  PLFLT *i = new PLFLT[2];
  PLFLT *h = new PLFLT[2];
  PLFLT *l = new PLFLT[2];
  PLFLT *s = new PLFLT[2];
  PLINT *rev = new PLINT[2];

  i[0] = 0.0;         // left boundary
  i[1] = 1.0;         // right boundary

  if (gray==1) {
    h[0] = 0.0;         // hue -- low: red (arbitrary if s=0)
    h[1] = 0.0;         // hue -- high: red (arbitrary if s=0)

    l[0] = 0.5;         // lightness -- low: half-dark
    l[1] = 1.0;         // lightness -- high: light

    s[0] = 0.0;         // minimum saturation
    s[1] = 0.0;         // minimum saturation
  }
  else {
    h[0] = 240; /* blue -> green -> yellow -> */
    h[1] = 0;   /* -> red */

    l[0] = 0.6;
    l[1] = 0.6;

    s[0] = 0.8;
    s[1] = 0.8;
  }

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

// Does a series of 3-d plots for a given data set, with different viewing
// options in each plot.

x08::x08( int argc, char **argv ) {
  int i, j, k;
  const int LEVELS = 10;

  PLFLT *x = new PLFLT[ XPTS ];
  PLFLT *y = new PLFLT[ YPTS ];
  PLFLT **z;
  PLFLT *clevel = new PLFLT[LEVELS];
  bool rosen = true;

  PLFLT xx, yy, r;
  PLFLT zmin, zmax;

  int ifshade;

  pls = new plstream();

  // Parse and process command line arguments.
  pls->MergeOpts(options, "x08c options",  NULL);
  pls->parseopts( &argc, argv, PL_PARSE_FULL );
  if (sombrero) {
    rosen = false;
  }

  // Initialize plplot.

  pls->init();

  pls->Alloc2dGrid(&z,XPTS,YPTS);
  /*z = new PLFLT*[XPTS];*/

  /*
  for (i = 0; i < XPTS; i++) {
    z[i] = new PLFLT [YPTS];
    x[i] = ((double) (i - (XPTS / 2)) / (double) (XPTS / 2));
    if (rosen)
      x[i] *=  1.5;
  }
  */

  for( i=0; i < XPTS; i++ ) {
    x[i] = (PLFLT) (i - (XPTS/2)) / (PLFLT) (XPTS/2);
    if (rosen)
      x[i] *=  1.5;
  }

  for( j=0; j < YPTS; j++ ) {
    y[j] = (PLFLT) (j - (YPTS/2)) / (PLFLT) (YPTS/2);
    if (rosen)
      y[j] += 0.5;
  }

  for( i = 0; i < XPTS; i++ )
    {
      xx = x[i];
      for( j = 0; j < YPTS; j++ )
	{
	  yy = y[j];
	  if (rosen) {
	    z[i][j] = pow((double)(1. - xx),2.) + 100 * pow((double)(yy - pow((double)xx,2.)),2.);
	    /* The log argument may be zero for just the right grid.  */
	    if (z[i][j] > 0.)
	       z[i][j] = log(z[i][j]);
	    else
	      z[i][j] = -5.; /* -MAXFLOAT would mess-up up the scale */
	  }
	  else {
	    r = sqrt(xx * xx + yy * yy);
	    z[i][j] = exp(-r * r) * cos(2.0 * M_PI * r);
	  }
          if(i==0 && j==0) {
	     zmin = z[i][j];
	     zmax = zmin;
	  }
	  if (zmin > z[i][j])
	    zmin = z[i][j];
	  if (zmax < z[i][j])
	    zmax = z[i][j];
	}
    }

  PLFLT step = (zmax-zmin)/(LEVELS+1);
  for (i=0; i<LEVELS; i++)
    clevel[i] = zmin + step*(i+1);

  pls->lightsource( 1., 1., 1. );
  for( k = 0; k < 2; k++ )
    {
      for( ifshade = 0; ifshade < 4; ifshade++)
	{
	  pls->adv(0);
	  pls->vpor(0.0, 1.0, 0.0, 0.9);
	  pls->wind(-1.0, 1.0, -0.9, 1.1);
	  pls->col0(3);
	  pls->mtex("t", 1.0, 0.5, 0.5, title[k]);
	  pls->col0(1);
	  if(rosen)
	    pls->w3d( 1.0, 1.0, 1.0, -1.5, 1.5, -0.5, 1.5, zmin, zmax,
		       alt[k], az[k] );
	  else
	    pls->w3d( 1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, zmin, zmax,
		       alt[k], az[k] );
	  pls->box3( "bnstu", "x axis", 0.0, 0,
		      "bnstu", "y axis", 0.0, 0,
		      "bcdmnstuv", "z axis", 0.0, 0 );

	  pls->col0(2);

	  if (ifshade == 0) { /* diffuse light surface plot */
	    cmap1_init(1);
	    pls->surf3d( x, y, z, XPTS, YPTS, 0, NULL, 0 );
	  } else if (ifshade == 1) { /* magnitude colored plot */
	    cmap1_init(0);
	    pls->surf3d( x, y, z, XPTS, YPTS, MAG_COLOR, NULL, 0 );
	  } else if (ifshade == 2) { /*  magnitude colored plot with faceted squares */
	    cmap1_init(0);
	    pls->surf3d( x, y, z, XPTS, YPTS, MAG_COLOR | FACETED, NULL, 0 );
	  } else {                   /* magnitude colored plot with contours */
	    cmap1_init(0);
	    pls->surf3d( x, y, z, XPTS, YPTS, MAG_COLOR | SURF_CONT | BASE_CONT, clevel, LEVELS );
	  }
	}
    }

  pls->Free2dGrid(z,XPTS,YPTS);

  delete[] x;
  delete[] y;
  delete[] clevel;
  delete pls;
}


int main( int argc, char **argv )
{
  x08 *x = new x08( argc, argv );

  delete x;
}


//---------------------------------------------------------------------------//
//                              End of x08.cc
//---------------------------------------------------------------------------//
