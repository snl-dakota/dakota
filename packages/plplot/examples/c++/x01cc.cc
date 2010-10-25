//---------------------------------------------------------------------------//
// $Id: x01cc.cc 3186 2006-02-15 18:17:33Z slbrow $
//---------------------------------------------------------------------------//
//
//---------------------------------------------------------------------------//
// Copyright (C) 1994 Geoffrey Furnish
// Copyright (C) 1995, 2000 Maurice LeBrun
// Copyright (C) 2002, 2002, 2003, 2004 Alan W. Irwin
// Copyright (C) 2004  Andrew Ross
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
// This example program demonstrates the use of the plstream C++ class, and
// some aspects of its improvements over the klunky C API, mostly those
// relating to 2-d plotting.
//---------------------------------------------------------------------------//

#ifdef HAVE_CMATH
#include <cmath>
#else
#include <math.h>
#endif

#include <iostream>

#include "plstream.h"

#ifdef USE_NAMESPACE
using namespace std;
#endif

//---------------------------------------------------------------------------//
// In the real world, the user has his own Matrix class, so he just includes
// the header for it.  Here we conjure up a dopey stand in.

class Matrix {
    int nx, ny;
    PLFLT *v;
  public:
    Matrix( int _nx, int _ny ) : nx(_nx), ny(_ny) { v = new PLFLT[nx*ny]; }
    ~Matrix() { delete[] v; }

    PLFLT& operator()( int i, int j )
    {
	// Should do bounds checking, pass for now.
	return v[ j*ny + i ];
    }

    PLFLT operator()( int i, int j ) const
    {
	// Should do bounds checking, pass for now.
	return v[ j*ny + i ];
    }

    void redim( int i, int j )
    {
	delete[] v;
	nx = i, ny = j;
	v = new PLFLT[nx*ny];
    }
};

//---------------------------------------------------------------------------//
// To perform contouring, we have to concretize the abstract contouring
// interface.  Do this by deriving from Contourable_Data, and implementing
// the indexing operator.

class ContourableMatrix : public Contourable_Data {
    int nx, ny;
    Matrix m;
    int wrapy;			// periodic in 2nd coord ?
  public:
    ContourableMatrix( int _nx, int _ny, int wy =0 )
	: Contourable_Data(_nx,_ny),
	  nx(_nx), ny(_ny), m(nx,ny), wrapy(wy)
    {}
    void elements( int& _nx, int& _ny ) const
    {
	_nx = nx;
	if (wrapy)
	    _ny = ny +1;
	else
	    _ny = ny;
    }
    PLFLT& operator()( int i, int j )
    {
	if (wrapy) j %= ny;
	return m(i,j);
    }
    PLFLT operator()( int i, int j ) const
    {
	if (wrapy) j %= ny;
	return m(i,j);
    }
};
	
//---------------------------------------------------------------------------//
// For general mesh plotting, we also need to concretize the abstract
// coordinate interface.  Do this by deriving from Coord_2d and filling in
// the blanks.

class CoordinateMatrix : public Coord_2d {
    int nx, ny;
    Matrix m;
    int wrapy;
  public:
    CoordinateMatrix( int _nx, int _ny, int wy =0 )
	: nx(_nx), ny(_ny), m(nx,ny), wrapy(wy)
    {}

    PLFLT operator() ( int ix, int iy ) const
    {
	if (wrapy) iy %= ny;
	return m(ix,iy);
    }

    PLFLT& operator() ( int ix, int iy )
    {
	if (wrapy) iy %= ny;
	return m(ix,iy);
    }

    void elements( int& _nx, int& _ny )
    {
	_nx = nx;
	if (wrapy)
	    _ny = ny + 1;
	else
	    _ny = ny;
    }

    void min_max( PLFLT& _min, PLFLT& _max )
    {
	_min = _max = m(0,0);
	for( int i=0; i < nx; i++ )
	    for( int j=0; j < ny; j++ ) {
		if (m(i,j) < _min) _min = m(i,j);
		if (m(i,j) > _max) _max = m(i,j);
	    }
    }
};

class x01cc {

public:
   x01cc(int, char**);
   void plot1();
   void plot2();


private:
   plstream *pls;
   
};

//---------------------------------------------------------------------------//
// Just a simple little routine to show simple use of the plstream object.
//---------------------------------------------------------------------------//

void x01cc::plot1()
{
    pls->col( Red );
    pls->env( 0., 1., 0., 1., 0, 0 );

    pls->col( Yellow );
    pls->lab("(x)", "(y)", "#frPLplot Example 1 - y=x#u2");

    PLFLT x[6], y[6];
    for( int i=0; i < 6; i++ ) {
	x[i] = .2 * i;
	y[i] = x[i] * x[i];
    }

    pls->col( Cyan );
    pls->poin( 6, x, y, 9 );

    pls->col( Green );
    pls->line( 6, x, y );
}

//---------------------------------------------------------------------------//
// Demonstration of contouring using the C++ abstract interface which does
// not impose fascist requirements on storage order/format of user data as
// the C and Fortran API's do.
//---------------------------------------------------------------------------//

void x01cc::plot2()
{
    pls->adv(0);

// First declare some objects to hold the data and the coordinates.  Note,
// if you don't want to go to the trouble of making these derived classes so
// easy to use (const and non-const indexing operators, etc), such as if you
// have existing code using a Matrix class, and all you want to do now is
// plot it, then you could just make these derived classes have a
// constructor taking a Matrix (previously calculated somewhere else) by
// reference through the constructor.  That way the calculation engine can
// continue to use the normal container class, and only the plotting code
// needs the auxiliary class to concretize the C++ abstract contouring
// interface.

// Since this is a "polar" plot ( :-), see below), we need to enable the
// "wrapy" option in our special purpose data and coordinate classes.  Note
// that this allows "reconnection" of lines, etc, with trivial effort, IFF
// done from C++.  For C-- and Dogtran, one would have to copy the data to a
// new buffer, and pad one side with an image copy of the other side.

    ContourableMatrix d(64,64,1);
    CoordinateMatrix  xg(64,64,1), yg(64,64,1);

    int i, j;
    PLFLT twopi = 2.*3.1415927;

// Set up the data and coordinate matrices.

    for( i=0; i < 64; i++ ) {
	PLFLT r = i/64.;
	for( j=0; j < 64; j++ ) {
	    PLFLT theta = twopi * j/64.;

	    xg(i,j) = r * cos(theta);
	    yg(i,j) = r * sin(theta);;
	    d(i,j) = exp(-r*r)*cos(twopi*2*r)*sin(3*theta);
	}
    }

// Now draw a normal shaded plot.

    PLFLT zmin = -1., zmax = 1.;
    int NCONTR = 20;
    PLFLT shade_min, shade_max, sh_color;
    int sh_cmap =1, sh_width;
    int min_color = 1, min_width = 0, max_color = 0, max_width = 0;

    pls->vpor( .1, .9, .1, .9 );
    pls->wind( 0., 1., 0., twopi );

    for (i = 0; i < NCONTR; i++) {
	shade_min = zmin + (zmax - zmin) * i / (PLFLT) NCONTR;
	shade_max = zmin + (zmax - zmin) * (i +1) / (PLFLT) NCONTR;
	sh_color = i / (PLFLT) (NCONTR-1);
	sh_width = 2;
	pls->psty(0);

	pls->shade( d, 0., 1., 0., twopi,
		   shade_min, shade_max, sh_cmap, sh_color, sh_width,
		   min_color, min_width, max_color, max_width,
		   1, NULL );
    }

    pls->col(Red);
    pls->box("bcnst", 0.0, 0, "bcnstv", 0.0, 0);

// Now do it again, but with the coordinate transformation taken into
// account. 

    pls->adv(0);

    cxx_pltr2 tr( xg, yg );

    pls->vpas( .1, .9, .1, .9, 1. );
    pls->wind( -1., 1., -1., 1. );

    for (i = 0; i < NCONTR; i++) {
	shade_min = zmin + (zmax - zmin) * i / (PLFLT) NCONTR;
	shade_max = zmin + (zmax - zmin) * (i +1) / (PLFLT) NCONTR;
	sh_color = i / (PLFLT) (NCONTR-1);
	sh_width = 2;
	pls->psty(0);

	pls->shade( d, 0., 1., 0., twopi,
		   shade_min, shade_max, sh_cmap, sh_color, sh_width,
		   min_color, min_width, max_color, max_width,
		   0, &tr );
    }

    pls->col(Red);

// Now draw the border around the drawing region.

    PLFLT x[65], y[65];

    for( i=0; i < 65; i++ ) {
	x[i] = xg(63,i);
	y[i] = yg(63,i);
    }

    pls->line( 65, x, y );

// Finally, let's "squoosh" the plot, and draw it all again.

    PLFLT X1 = 1., X2 = .1, Y1 = 1.2, Y2 = -.2;

    for( i=0; i < 64; i++ ) {
	PLFLT r = i/64.;
	for( j=0; j < 64; j++ ) {
	    PLFLT theta = twopi * j / 64.;

	    xg(i,j) = X1 * r * cos(theta) +
		X2 * r*r * cos(2*theta);

	    yg(i,j) = Y1 * r * sin(theta) +
		Y2 * r*r * sin(2*theta);
	}
    }

    PLFLT xmin, xmax, ymin, ymax;
    xg.min_max(xmin, xmax), yg.min_max(ymin, ymax);

    pls->adv(0);

    pls->vpas( .1, .9, .1, .9, 1. );
    pls->wind( xmin, xmax, ymin, ymax );

    for (i = 0; i < NCONTR; i++) {
	shade_min = zmin + (zmax - zmin) * i / (PLFLT) NCONTR;
	shade_max = zmin + (zmax - zmin) * (i +1) / (PLFLT) NCONTR;
	sh_color = i / (PLFLT) (NCONTR-1);
	sh_width = 2;
	pls->psty(0);

	pls->shade( d, 0., 1., 0., twopi,
		   shade_min, shade_max, sh_cmap, sh_color, sh_width,
		   min_color, min_width, max_color, max_width,
		   0, &tr );
    }

    pls->col(Red);

// Now draw the border around the drawing region.

    for( i=0; i < 65; i++ ) {
	x[i] = xg(63,i);
	y[i] = yg(63,i);
    }

    pls->line( 65, x, y );
}

x01cc::x01cc( int argc, char **argv ) {

   pls = new plstream();

   // Parse and process command line arguments.
  
   pls->parseopts( &argc, argv, PL_PARSE_FULL );

   // Initialize plplot.
  
   pls->init();
   plot1();
   plot2();
   delete pls;
}

//---------------------------------------------------------------------------//
// Finally!
//---------------------------------------------------------------------------//

int main( int argc, char **argv ) 
{
   x01cc *x = new x01cc( argc, argv );

   delete x;
}

//---------------------------------------------------------------------------//
//                              End of x01cc.cc
//---------------------------------------------------------------------------//
