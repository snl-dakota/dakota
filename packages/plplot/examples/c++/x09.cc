//---------------------------------------------------------------------------//
// $Id: x09.cc 3186 2006-02-15 18:17:33Z slbrow $
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
// Implementation of PLplot example 09 in C++.
//---------------------------------------------------------------------------//

#include "plstream.h"

#include <iostream>
#include <cmath>

#ifdef USE_NAMESPACE
using namespace std;
#endif

class x09 {
public:
  x09(int, char**);
  void polar();
  const void potential();

private:
  plstream *pls;

  static const int XPTS;
  static const int YPTS;

  // polar plot data
  static const int PERIMETERPTS;
  static const int RPTS;
  static const int THETAPTS;

  // potential plot data
  static const int PPERIMETERPTS;
  static const int PRPTS;
  static const int PTHETAPTS;
  static const int PNLEVEL;

  static PLFLT clevel[];
  // Transformation function
  //  static const PLFLT tr[];

public:
  static const PLFLT XSPA;
  static const PLFLT YSPA;


};

const int x09::XPTS = 35;
const int x09::YPTS = 46;
const PLFLT x09::XSPA =  2./(XPTS-1);
const PLFLT x09::YSPA =  2./(YPTS-1);

// polar plot data
const int x09::PERIMETERPTS = 100;
const int x09::RPTS = 40;
const int x09::THETAPTS = 40;

// potential plot data
const int x09::PPERIMETERPTS = 100;
const int x09::PRPTS = 40;
const int x09::PTHETAPTS = 64;
const int x09::PNLEVEL = 20;

PLFLT x09::clevel[] = {-1., -.8, -.6, -.4, -.2, 0, .2, .4, .6, .8, 1.};
  // Transformation function
//const PLFLT x09::tr[] = {XSPA, 0.0, -1.0, 0.0, YSPA, -1.0};


static const PLFLT tr[] = {x09::XSPA, 0.0, -1.0, 0.0, x09::YSPA, -1.0};

static void mypltr(PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, void *pltr_data){
  *tx = tr[0] * x + tr[1] * y + tr[2];
  *ty = tr[3] * x + tr[4] * y + tr[5];
}

// Does a large series of unlabelled and labelled contour plots.


x09::x09( int argc, char **argv ) {

  int i, j;

  PLFLT xg1[XPTS];
  PLFLT yg1[YPTS];
  PLcGrid cgrid1;
  PLcGrid2 cgrid2;
  PLFLT **z;
  PLFLT **w;

  PLFLT xx, yy, argx, argy, distort;
  static PLINT mark = 1500;
  static PLINT space = 1500;

  pls = new plstream();

  // Parse and process command line arguments.

  pls->parseopts( &argc, argv, PL_PARSE_FULL );
  /* Initialize plplot */

  pls->init();

  pls->Alloc2dGrid(&z,XPTS,YPTS);
  pls->Alloc2dGrid(&w,XPTS,YPTS);


  /* Set up function arrays */

  for (i = 0; i < XPTS; i++) {
    xx = (PLFLT) (i - (XPTS / 2)) / (PLFLT) (XPTS / 2);
    for (j = 0; j < YPTS; j++) {
      yy = (PLFLT) (j - (YPTS / 2)) / (PLFLT) (YPTS / 2) - 1.0;
      z[i][j] = xx * xx - yy * yy;
      w[i][j] = 2 * xx * yy;
    }
  }

  /* Set up grids */
  cgrid1.xg = xg1;
  cgrid1.yg = yg1;
  cgrid1.nx = XPTS;
  cgrid1.ny = YPTS;

  pls->Alloc2dGrid(&cgrid2.xg,XPTS,YPTS);
  pls->Alloc2dGrid(&cgrid2.yg,XPTS,YPTS);
  cgrid2.nx = XPTS;
  cgrid2.ny = YPTS;


  for (i = 0; i < XPTS; i++) {
    for (j = 0; j < YPTS; j++) {
      mypltr((PLFLT) i, (PLFLT) j, &xx, &yy, NULL);

      argx = xx * M_PI/2;
      argy = yy * M_PI/2;
      distort = 0.4;

      cgrid1.xg[i] = xx + distort * cos(argx);
      cgrid1.yg[j] = yy - distort * cos(argy);

      cgrid2.xg[i][j] = xx + distort * cos(argx) * cos(argy);
      cgrid2.yg[i][j] = yy - distort * cos(argx) * cos(argy);
    }
  }


  // Plot using scaled identity transform used to create xg0 and yg0
  /*	pls->_setcontlabelparam(0.006, 0.3, 0.1, 0);
	pls->env(-1.0, 1.0, -1.0, 1.0, 0, 0);
	pls->col0(2);
        pls->cont( z, XPTS, YPTS, 1, XPTS, 1, YPTS, clevel, 11, mypltr, NULL );
	pls->styl(1, &mark, &space);
	pls->col0(3);
	pls->cont(w, XPTS, YPTS, 1, XPTS, 1, YPTS, clevel, 11, mypltr, NULL );
	pls->styl(0, &mark, &space);
	pls->col0(1);
	pls->lab("X Coordinate", "Y Coordinate", "Streamlines of flow");
  */
  pls->setcontlabelparam(0.006, 0.3, 0.1, 1);
  pls->env(-1.0, 1.0, -1.0, 1.0, 0, 0);
  pls->col0(2);
  pls->cont(z, XPTS, YPTS, 1, XPTS, 1, YPTS, clevel, 11, mypltr, NULL );
  pls->styl(1, &mark, &space);
  pls->col0(3);
  pls->cont(w, XPTS, YPTS, 1, XPTS, 1, YPTS, clevel, 11, mypltr, NULL );
  pls->styl(0, &mark, &space);
  pls->col0(1);
  pls->lab("X Coordinate", "Y Coordinate", "Streamlines of flow");
  pls->setcontlabelparam(0.006, 0.3, 0.1, 0);

  // Plot using 1d coordinate transform
  pls->env(-1.0, 1.0, -1.0, 1.0, 0, 0);
  pls->col0(2);
  pls->cont(z, XPTS, YPTS, 1, XPTS, 1, YPTS, clevel, 11,
	    plstream::tr1, (void *) &cgrid1 );
  pls->styl(1, &mark, &space);
  pls->col0(3);
  pls->cont(w, XPTS, YPTS, 1, XPTS, 1, YPTS, clevel, 11,
	    plstream::tr1, (void *) &cgrid1 );
  pls->styl(0, NULL, NULL);
  pls->col0(1);
  pls->lab("X Coordinate", "Y Coordinate", "Streamlines of flow");

  /*	pls->_setcontlabelparam(0.006, 0.3, 0.1, 1);
	pls->env(-1.0, 1.0, -1.0, 1.0, 0, 0);
	pls->col0(2);
        pls->cont(z, XPTS, YPTS, 1, XPTS, 1, YPTS, clevel, 11,
	    pltr1, (void *) &cgrid1 );
	pls->styl(2, &mark, &space);
	pls->col0(3);
        pls->cont(w, XPTS, YPTS, 1, XPTS, 1, YPTS, clevel, 11,
	    pltr1, (void *) &cgrid1 );
	pls->styl(0, &mark, &space);
	pls->col0(1);
	pls->lab("X Coordinate", "Y Coordinate", "Streamlines of flow");
	pls->_setcontlabelparam(0.006, 0.3, 0.1, 0);
  */
  // Plot using 2d coordinate transform
  pls->env(-1.0, 1.0, -1.0, 1.0, 0, 0);
  pls->col0(2);
  pls->cont(z, XPTS, YPTS, 1, XPTS, 1, YPTS, clevel, 11,
	    plstream::tr2, (void *) &cgrid2 );
  pls->styl(1, &mark, &space);
  pls->col0(3);
  pls->cont(w, XPTS, YPTS, 1, XPTS, 1, YPTS, clevel, 11,
	    plstream::tr2, (void *) &cgrid2 );
  pls->styl(0, &mark, &space);
  pls->col0(1);
  pls->lab("X Coordinate", "Y Coordinate", "Streamlines of flow");

  /*	pls->_setcontlabelparam(0.006, 0.3, 0.1, 1);
	pls->env(-1.0, 1.0, -1.0, 1.0, 0, 0);
	pls->col0(2);
	pls->cont(z, XPTS, YPTS, 1, XPTS, 1, YPTS, clevel, 11,
	    pltr2, (void *) &cgrid2 );
	pls->styl(1, &mark, &space);
	pls->col0(3);
	pls->cont(w, XPTS, YPTS, 1, XPTS, 1, YPTS, clevel, 11,
	    pltr2, (void *) &cgrid2 );
	pls->styl(1, &mark0, &space0);
	pls->col0(1);
	pls->lab("X Coordinate", "Y Coordinate", "Streamlines of flow");
  */
  pls->setcontlabelparam(0.006, 0.3, 0.1, 0);
  polar();
  /*
  pls->setcontlabelparam(0.006, 0.3, 0.1, 1);
  polar();
  */
  pls->setcontlabelparam(0.006, 0.3, 0.1, 0);
  potential();
  /*
  pls->setcontlabelparam(0.006, 0.3, 0.1, 1);
  potential();
  */

  // pls->end();

  pls->Free2dGrid(cgrid2.xg,XPTS,YPTS);
  pls->Free2dGrid(cgrid2.yg,XPTS,YPTS);
  pls->Free2dGrid(z,XPTS,YPTS);
  pls->Free2dGrid(w,XPTS,YPTS);

  delete pls;
}

void x09::polar()
  // polar contour plot example.
{
  int i,j;
  PLFLT *px = new PLFLT[PERIMETERPTS];
  PLFLT *py = new PLFLT[PERIMETERPTS];
  PLcGrid2 cgrid2;
  PLFLT **z;
  PLFLT t, r, theta;
  PLFLT *lev = new PLFLT[10];

  pls->Alloc2dGrid(&cgrid2.xg,RPTS,THETAPTS);
  pls->Alloc2dGrid(&cgrid2.yg,RPTS,THETAPTS);
  pls->Alloc2dGrid(&z,RPTS,THETAPTS);
  cgrid2.nx = RPTS;
  cgrid2.ny = THETAPTS;

  pls->env(-1., 1., -1., 1., 0, -2);
  pls->col0(1);

  // Perimeter
  for (i = 0; i < PERIMETERPTS; i++) {
    t = (2.*M_PI/(PERIMETERPTS-1))*(PLFLT)i;
    px[i] = cos(t);
    py[i] = sin(t);
  }
  pls->line(PERIMETERPTS, px, py);

  // Create data to be contoured.

  for (i = 0; i < RPTS; i++) {
    r = i/(PLFLT)(RPTS-1);
    for (j = 0; j < THETAPTS; j++) {
      theta = (2.*M_PI/(PLFLT)(THETAPTS-1))*(PLFLT)j;
      cgrid2.xg[i][j] = r*cos(theta);
      cgrid2.yg[i][j] = r*sin(theta);
      z[i][j] = r;
    }
  }

  for (i = 0; i < 10; i++) {
    lev[i] = 0.05 + 0.10*(PLFLT) i;
  }

  pls->col0(2);
  pls->cont( z, RPTS, THETAPTS, 1, RPTS, 1, THETAPTS, lev, 10,
	     plstream::tr2, (void *) &cgrid2);
  pls->col0(1);
  pls->lab("", "", "Polar Contour Plot");

  pls->Free2dGrid(cgrid2.xg,RPTS,THETAPTS);
  pls->Free2dGrid(cgrid2.yg,RPTS,THETAPTS);
  pls->Free2dGrid(z,RPTS,THETAPTS);

  delete[] px;
  delete[] py;
  delete[] lev;
}

const void x09::potential()
  // Shielded potential contour plot example.
{
  int i,j;

  PLFLT rmax, xmin, xmax, x0, ymin, ymax, y0, zmin, zmax;
  PLFLT peps, xpmin, xpmax, ypmin, ypmax;
  PLFLT eps, q1, d1, q1i, d1i, q2, d2, q2i, d2i;
  PLFLT div1, div1i, div2, div2i;
  PLcGrid2 cgrid2;
  PLFLT **z;
  int nlevelneg, nlevelpos;
  PLFLT dz, clevel;
  PLFLT *clevelneg = new PLFLT[PNLEVEL];
  PLFLT *clevelpos = new PLFLT[PNLEVEL];
  int  ncollin, ncolbox, ncollab;
  PLFLT *px = new PLFLT[PPERIMETERPTS];
  PLFLT *py = new PLFLT[PPERIMETERPTS];
  PLFLT t, r, theta;

  // Create data to be contoured.
  pls->Alloc2dGrid(&cgrid2.xg,PRPTS,PTHETAPTS);
  pls->Alloc2dGrid(&cgrid2.yg,PRPTS,PTHETAPTS);
  pls->Alloc2dGrid(&z,PRPTS,PTHETAPTS);
  cgrid2.nx = PRPTS;
  cgrid2.ny = PTHETAPTS;

  //  r = 0.;
  for (i = 0; i < PRPTS; i++) {
    r = 0.5 + (PLFLT) i;
    for (j = 0; j < PTHETAPTS; j++) {
      theta = (2.*M_PI/(PLFLT)(PTHETAPTS-1))*(0.5 + (PLFLT) j);
      cgrid2.xg[i][j] = r*cos(theta);
      cgrid2.yg[i][j] = r*sin(theta);
    }
  }

  rmax = r;

  pls->MinMax2dGrid( cgrid2.xg, PRPTS, PTHETAPTS, &xmax, &xmin );
  pls->MinMax2dGrid( cgrid2.yg, PRPTS, PTHETAPTS, &ymax, &ymin );

  x0 = (xmin + xmax)/2.;
  y0 = (ymin + ymax)/2.;

  // Expanded limits
  peps = 0.05;
  xpmin = xmin - abs(xmin)*peps;
  xpmax = xmax + abs(xmax)*peps;
  ypmin = ymin - abs(ymin)*peps;
  ypmax = ymax + abs(ymax)*peps;

  // Potential inside a conducting cylinder (or sphere) by method of images.
  // Charge 1 is placed at (d1, d1), with image charge at (d2, d2).
  // Charge 2 is placed at (d1, -d1), with image charge at (d2, -d2).
  // Also put in smoothing term at small distances.

  eps = 2.;

  q1 = 1.;
  d1 = rmax/4.;

  q1i = - q1*rmax/d1;
  d1i = pow(rmax,2)/d1;

  q2 = -1.;
  d2 = rmax/4.;

  q2i = - q2*rmax/d2;
  d2i = pow(rmax,2)/d2;

  for (i = 0; i < PRPTS; i++) {
    for (j = 0; j < PTHETAPTS; j++) {
      div1 = sqrt(pow(cgrid2.xg[i][j]-d1,2) + pow(cgrid2.yg[i][j]-d1,2) + pow(eps,2));
      div1i = sqrt(pow(cgrid2.xg[i][j]-d1i,2) + pow(cgrid2.yg[i][j]-d1i,2) + pow(eps,2));
      div2 = sqrt(pow(cgrid2.xg[i][j]-d2,2) + pow(cgrid2.yg[i][j]+d2,2) + pow(eps,2));
      div2i = sqrt(pow(cgrid2.xg[i][j]-d2i,2) + pow(cgrid2.yg[i][j]+d2i,2) + pow(eps,2));
      z[i][j] = q1/div1 + q1i/div1i + q2/div2 + q2i/div2i;
    }
  }


  pls->MinMax2dGrid( z, PRPTS, PTHETAPTS, &zmax, &zmin );

  /*	printf("%.15g %.15g %.15g %.15g %.15g %.15g %.15g %.15g \n",
        q1, d1, q1i, d1i, q2, d2, q2i, d2i);
  	printf("%.15g %.15g %.15g %.15g %.15g %.15g\n",
        xmin,xmax,ymin,ymax,zmin,zmax);*/

  // Positive and negative contour levels.
  dz = (zmax-zmin)/(PLFLT) PNLEVEL;
  nlevelneg = 0;
  nlevelpos = 0;
  for (i = 0; i < PNLEVEL; i++) {
    clevel = zmin + ((PLFLT) i + 0.5)*dz;
    if (clevel <= 0.)
      clevelneg[nlevelneg++] = clevel;
    else
      clevelpos[nlevelpos++] = clevel;
  }
  // Colours!
  ncollin = 11;
  ncolbox = 1;
  ncollab = 2;

  // Finally start plotting this page!
  pls->adv(0);
  pls->col0(ncolbox);

  pls->vpas(0.1, 0.9, 0.1, 0.9, 1.0);
  pls->wind(xpmin, xpmax, ypmin, ypmax);
  pls->box("", 0., 0, "", 0., 0);

  pls->col0(ncollin);
  if(nlevelneg >0) {
    // Negative contours
    pls->lsty(2);
    pls->cont( z, PRPTS, PTHETAPTS, 1, PRPTS, 1, PTHETAPTS,
	       clevelneg, nlevelneg, plstream::tr2, (void *) &cgrid2 );
  }

  if(nlevelpos >0) {
    // Positive contours
    pls->lsty(1);
    pls->cont( z, PRPTS, PTHETAPTS, 1, PRPTS, 1, PTHETAPTS,
	       clevelpos, nlevelpos, plstream::tr2, (void *) &cgrid2 );
  }

  // Draw outer boundary
  for (i = 0; i < PPERIMETERPTS; i++) {
    t = (2.*M_PI/(PPERIMETERPTS-1))*(PLFLT)i;
    px[i] = x0 + rmax*cos(t);
    py[i] = y0 + rmax*sin(t);
  }

  pls->col0(ncolbox);
  pls->line(PPERIMETERPTS, px, py);

  pls->col0(ncollab);
  pls->lab("", "", "Shielded potential of charges in a conducting sphere");

  pls->Free2dGrid(cgrid2.xg,RPTS,THETAPTS);
  pls->Free2dGrid(cgrid2.yg,RPTS,THETAPTS);
  pls->Free2dGrid(z,RPTS,THETAPTS);

  delete[] clevelneg;
  delete[] clevelpos;
  delete[] px;
  delete[] py;
}

int main( int argc, char **argv )
{
  x09 *x = new x09( argc, argv );

  delete x;
}


//---------------------------------------------------------------------------//
//                              End of x09.cc
//---------------------------------------------------------------------------//
