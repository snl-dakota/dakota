//---------------------------------------------------------------------------//
// $Id: x22.cc 3186 2006-02-15 18:17:33Z slbrow $
//    Simple vector plot example
//---------------------------------------------------------------------------//
//
//---------------------------------------------------------------------------//
// Copyright (C) 2004  Andrew Ross <andrewross@users.sourceforge.net>
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
// Implementation of PLplot example 22 in C++.
//---------------------------------------------------------------------------//

#include "plstream.h"
#include "plevent.h"

#include <iostream>
#include <cmath>

#ifdef USE_NAMESPACE
using namespace std;
#endif

class x22 {

public:
    x22(int, char **);

private:
    void circulation();
    void constriction();
    void potential();
    void f2mnmx(PLFLT **f, PLINT nx, PLINT ny, PLFLT *fmin, PLFLT *fmax);

    PLFLT MIN(PLFLT x, PLFLT y) { return (x<y?x:y);};
    PLFLT MAX(PLFLT x, PLFLT y) { return (x>y?x:y);};
  
    plstream *pls;
  
    PLFLT **u, **v;
    PLcGrid2 cgrid2;
    int nx, ny;

};

// Vector plot of the circulation about the origin
void
x22::circulation() {
    int i,j;
    PLFLT dx, dy, x, y;
    PLFLT xmin, xmax, ymin, ymax;

    dx = 1.0;
    dy = 1.0;

    xmin = -nx/2*dx;
    xmax = nx/2*dx;
    ymin = -ny/2*dy;
    ymax = ny/2*dy;


    // Create data - cirulation around the origin.
    for (i = 0; i<nx; i++) {
	for (j = 0; j<ny; j++) {
	    x = (i-nx/2+0.5)*dx;
	    y = (j-ny/2+0.5)*dy;
	    cgrid2.xg[i][j] = x;
	    cgrid2.yg[i][j] = y;
	    u[i][j] = y;
	    v[i][j] = -x;
	}
    }

    // Plot vectors with default arrows
    pls->env(xmin, xmax, ymin, ymax, 0, 0);
    pls->lab("(x)", "(y)", "#frPLplot Example 22 - circulation");
    pls->col0(2);
    pls->vect(u,v,nx,ny,0.0,plstream::tr2,(void *)&cgrid2);
    pls->col0(1);

}

// Vector plot of flow through a constricted pipe
void
x22::constriction() {
    int i,j;
    PLFLT dx, dy, x, y;
    PLFLT xmin, xmax, ymin, ymax;
    PLFLT Q, b, dbdx;

    dx = 1.0;
    dy = 1.0;

    xmin = -nx/2*dx;
    xmax = nx/2*dx;
    ymin = -ny/2*dy;
    ymax = ny/2*dy;

    Q = 2.0;
    for (i = 0; i<nx; i++) {
	for (j = 0; j<ny; j++) {
	    x = (i-nx/2+0.5)*dx;
	    y = (j-ny/2+0.5)*dy;
	    cgrid2.xg[i][j] = x;
	    cgrid2.yg[i][j] = y;
	    b = ymax/4.0*(3-cos(M_PI*x/xmax));
	    if (fabs(y) < b) {
		dbdx = ymax/4.0*sin(M_PI*x/xmax)*
		    y/b;
		u[i][j] = Q*ymax/b;
		v[i][j] = dbdx*u[i][j];
	    }
	    else {
		u[i][j] = 0.0;
		v[i][j] = 0.0;
	    }
	}
    }

    pls->env(xmin, xmax, ymin, ymax, 0, 0);
    pls->lab("(x)", "(y)", "#frPLplot Example 22 - constriction");
    pls->col0(2);
    pls->vect(u,v,nx,ny,-0.5,plstream::tr2,(void *)&cgrid2);
    pls->col0(1);

}

// Vector plot of the gradient of a shielded potential (see example 9)
void
x22::potential() {
	
    const int nper = 100;
    const int nlevel = 10;

    int i,j, nr, ntheta;
    PLFLT eps, q1, d1, q1i, d1i, q2, d2, q2i, d2i;
    PLFLT div1, div1i, div2, div2i;
    PLFLT **z, r, theta, x, y, dz;
    PLFLT xmin, xmax, ymin, ymax, rmax, zmax, zmin;
    PLFLT px[nper], py[nper], clevel[nlevel];

    nr = nx;
    ntheta = ny;
  
    // Create data to be plotted
    pls->Alloc2dGrid(&z,nr,ntheta);

    // Potential inside a conducting cylinder (or sphere) by method of images.
    // Charge 1 is placed at (d1, d1), with image charge at (d2, d2).
    // Charge 2 is placed at (d1, -d1), with image charge at (d2, -d2).
    // Also put in smoothing term at small distances.

    rmax = (double) nr;

    eps = 2.;

    q1 = 1.;
    d1 = rmax/4.;

    q1i = - q1*rmax/d1;
    d1i = pow((double)rmax, 2.)/d1;

    q2 = -1.;
    d2 = rmax/4.;

    q2i = - q2*rmax/d2;
    d2i = pow((double)rmax, 2.)/d2;

    for (i = 0; i < nr; i++) {
	r = 0.5 + (double) i;
	for (j = 0; j < ntheta; j++) {
	    theta = 2.*M_PI/(ntheta-1)*(0.5+(double)j);
	    x = r*cos(theta);
	    y = r*sin(theta);
	    cgrid2.xg[i][j] = x;
	    cgrid2.yg[i][j] = y;
	    div1 = sqrt(pow((double)(x-d1), 2.) + pow((double)(y-d1), 2.) + pow((double)eps, 2.));
	    div1i = sqrt(pow((double)(x-d1i), 2.) + pow((double)(y-d1i), 2.) + pow((double)eps, 2.));
	    div2 = sqrt(pow((double)(x-d2), 2.) + pow((double)(y+d2), 2.) + pow((double)eps, 2.));
	    div2i = sqrt(pow((double)(x-d2i), 2.) + pow((double)(y+d2i), 2.) + pow((double)eps, 2.));
	    z[i][j] = q1/div1 + q1i/div1i + q2/div2 + q2i/div2i;
	    u[i][j] = -q1*(x-d1)/pow((double)div1,3.) - q1i*(x-d1i)/pow((double)div1i,3.0) 
		- q2*(x-d2)/pow((double)div2,3.) - q2i*(x-d2i)/pow((double)div2i,3.);
	    v[i][j] = -q1*(y-d1)/pow((double)div1,3.) - q1i*(y-d1i)/pow((double)div1i,3.0) 
		- q2*(y+d2)/pow((double)div2,3.) - q2i*(y+d2i)/pow((double)div2i,3.);
	}
    }

    f2mnmx(cgrid2.xg, nr, ntheta, &xmin, &xmax);
    f2mnmx(cgrid2.yg, nr, ntheta, &ymin, &ymax);
    f2mnmx(z, nr, ntheta, &zmin, &zmax);

    pls->env(xmin, xmax, ymin, ymax, 0, 0);
    pls->lab("(x)", "(y)", "#frPLplot Example 22 - potential gradient vector plot");
    // Plot contours of the potential
    dz = (zmax-zmin)/(double) nlevel;
    for (i = 0; i < nlevel; i++) {
	clevel[i] = zmin + ((double) i + 0.5)*dz;
    }
    pls->col0(3);
    pls->lsty(2);
    pls->cont(z,nr,ntheta,1,nr,1,ntheta,clevel,nlevel,plstream::tr2,(void *) &cgrid2);
    pls->lsty(1);
    pls->col0(1);
    
    // Plot the vectors of the gradient of the potential
    pls->col0(2);
    pls->vect(u,v,nr,ntheta,25.0,plstream::tr2,(void *)&cgrid2);
    pls->col0(1);

    // Plot the perimeter of the cylinder
    for (i=0;i<nper;i++) {
	theta = (2.*M_PI/(nper-1))*(double)i;
	px[i] = rmax*cos(theta);
	py[i] = rmax*sin(theta);
    }
    pls->line(nper,px,py);
    
    pls->Free2dGrid(z,nr,ntheta);

}

void
x22::f2mnmx(PLFLT **f, PLINT nx, PLINT ny, PLFLT *fmin, PLFLT *fmax)
{
    int i, j;

    *fmax = f[0][0];
    *fmin = *fmax;

    for (i = 0; i < nx; i++) {
	for (j = 0; j < ny; j++) {
	    *fmax = MAX(*fmax, f[i][j]);
	    *fmin = MIN(*fmin, f[i][j]);
	}
    }
}


x22::x22( int argc, char ** argv ) {

    PLINT narr, fill;

    // Set of points making a polygon to use as the arrow
    PLFLT arrow_x[6] = {-0.5, 0.5, 0.3, 0.5, 0.3, 0.5};
    PLFLT arrow_y[6] = {0.0, 0.0, 0.2, 0.0, -0.2, 0.0};
    PLFLT arrow2_x[6] = {-0.5, 0.3, 0.3, 0.5, 0.3, 0.3};
    PLFLT arrow2_y[6] = {0.0, 0.0,   0.2, 0.0, -0.2, 0.0};


    // Create new plstream
    pls = new plstream();

    // Parse and process command line arguments 

    pls->parseopts(&argc, argv, PL_PARSE_FULL);

    // Initialize plplot 

    pls->init();

    nx = 20;
    ny = 20;

    // Allocate arrays
    pls->Alloc2dGrid(&cgrid2.xg,nx,ny);
    pls->Alloc2dGrid(&cgrid2.yg,nx,ny);
    pls->Alloc2dGrid(&u,nx,ny);
    pls->Alloc2dGrid(&v,nx,ny);

    cgrid2.nx = nx;
    cgrid2.ny = ny;

    circulation();

    narr = 6;
    fill = 0;

    // Set arrow style using arrow_x and arrow_y then 
    // plot using these arrows.
    pls->svect(arrow_x, arrow_y, narr, fill);
    constriction();

    // Set arrow style using arrow2_x and arrow2_y then 
    // plot using these filled arrows.
    fill = 1;
    pls->svect(arrow2_x, arrow2_y, narr, fill);
    constriction();

    potential();

    pls->Free2dGrid(cgrid2.xg,nx,ny);
    pls->Free2dGrid(cgrid2.yg,nx,ny);
    pls->Free2dGrid(u,nx,ny);
    pls->Free2dGrid(v,nx,ny);

    delete pls;
    exit(0);
}

int main( int argc, char ** argv ) {
    x22 *x = new x22( argc, argv );
    delete x;

}


//---------------------------------------------------------------------------//
//                              End of x22.cc
//---------------------------------------------------------------------------//

