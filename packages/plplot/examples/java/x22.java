//---------------------------------------------------------------------------//
// $Id: x22.java 3186 2006-02-15 18:17:33Z slbrow $
//    Simple vector plot example
//---------------------------------------------------------------------------//
//
//---------------------------------------------------------------------------//
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
// Implementation of PLplot example 22 in Java.
//---------------------------------------------------------------------------//

package plplot.examples;

import plplot.core.*;

import java.lang.Math;

class x22 {


    double MIN(double x, double y) { return (x<y?x:y);}
    double MAX(double x, double y) { return (x>y?x:y);}

    PLStream pls = new PLStream();
  
    double[][] u = null;
    double[][] v = null;
    double[][] xg = null;
    double[][] yg = null;
    int nx, ny;

    // Vector plot of the circulation about the origin
    void circulation() {
	int i,j;
	double dx, dy, x, y;
	double xmin, xmax, ymin, ymax;

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
		xg[i][j] = x;
		yg[i][j] = y;
		u[i][j] = y;
		v[i][j] = -x;
	    }
	}

	// Plot vectors with default arrows
	pls.env(xmin, xmax, ymin, ymax, 0, 0);
	pls.lab("(x)", "(y)", "#frPLplot Example 22 - circulation");
	pls.col0(2);
	pls.vect(u,v,0.0,xg,yg);
	pls.col0(1);

    }

    // Vector plot of flow through a constricted pipe
    void constriction() {
	int i,j;
	double dx, dy, x, y;
	double xmin, xmax, ymin, ymax;
	double Q, b, dbdx;

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
		xg[i][j] = x;
		yg[i][j] = y;
		b = ymax/4.0*(3-Math.cos(Math.PI*x/xmax));
		if (Math.abs(y) < b) {
		    dbdx = ymax/4.0*Math.sin(Math.PI*x/xmax)*
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

	pls.env(xmin, xmax, ymin, ymax, 0, 0);
	pls.lab("(x)", "(y)", "#frPLplot Example 22 - constriction");
	pls.col0(2);
	pls.vect(u,v,-0.5,xg,yg);
	pls.col0(1);

    }

    // Vector plot of the gradient of a shielded potential (see example 9)
    void potential() {
	
	final int nper = 100;
	final int nlevel = 10;

	int i,j, nr, ntheta;
	double eps, q1, d1, q1i, d1i, q2, d2, q2i, d2i;
	double div1, div1i, div2, div2i;
	double[][] z;
	double r, theta, x, y, dz, rmax;
	double[] xmaxmin = new double[2];
	double[] ymaxmin = new double[2];
	double[] zmaxmin = new double[2];
	double[] px = new double[nper];
	double[] py = new double[nper];
	double[] clevel = new double[nlevel];

	nr = nx;
	ntheta = ny;
  
	// Create data to be plotted
	z = new double[nr][ntheta];

	// Potential inside a conducting cylinder (or sphere) by method of images.
	// Charge 1 is placed at (d1, d1), with image charge at (d2, d2).
	// Charge 2 is placed at (d1, -d1), with image charge at (d2, -d2).
	// Also put in smoothing term at small distances.

	rmax = (double) nr;

	eps = 2.;

	q1 = 1.;
	d1 = rmax/4.;

	q1i = - q1*rmax/d1;
	d1i = Math.pow(rmax, 2.)/d1;

	q2 = -1.;
	d2 = rmax/4.;

	q2i = - q2*rmax/d2;
	d2i = Math.pow(rmax, 2.)/d2;

	for (i = 0; i < nr; i++) {
	    r = 0.5 + (double) i;
	    for (j = 0; j < ntheta; j++) {
		theta = 2.*Math.PI/(ntheta-1)*(0.5+(double)j);
		x = r*Math.cos(theta);
		y = r*Math.sin(theta);
		xg[i][j] = x;
		yg[i][j] = y;
		div1 = Math.sqrt(Math.pow(x-d1, 2.) + Math.pow(y-d1, 2.) + Math.pow(eps, 2.));
		div1i = Math.sqrt(Math.pow(x-d1i, 2.) + Math.pow(y-d1i, 2.) + Math.pow(eps, 2.));
		div2 = Math.sqrt(Math.pow(x-d2, 2.) + Math.pow(y+d2, 2.) + Math.pow(eps, 2.));
		div2i = Math.sqrt(Math.pow(x-d2i, 2.) + Math.pow(y+d2i, 2.) + Math.pow(eps, 2.));
		z[i][j] = q1/div1 + q1i/div1i + q2/div2 + q2i/div2i;
		u[i][j] = -q1*(x-d1)/Math.pow(div1,3.) - q1i*(x-d1i)/Math.pow(div1i,3.0) 
		    - q2*(x-d2)/Math.pow(div2,3.) - q2i*(x-d2i)/Math.pow(div2i,3.);
		v[i][j] = -q1*(y-d1)/Math.pow(div1,3.) - q1i*(y-d1i)/Math.pow(div1i,3.0) 
		    - q2*(y+d2)/Math.pow(div2,3.) - q2i*(y+d2i)/Math.pow(div2i,3.);
	    }
	}

	f2mnmx(xg, nr, ntheta, xmaxmin);
	f2mnmx(yg, nr, ntheta, ymaxmin);
	f2mnmx(z, nr, ntheta, zmaxmin);

	pls.env(xmaxmin[1], xmaxmin[0], ymaxmin[1], ymaxmin[0], 0, 0);
	pls.lab("(x)", "(y)", "#frPLplot Example 22 - potential gradient vector plot");
	// Plot contours of the potential
	dz = (zmaxmin[0]-zmaxmin[1])/(double) nlevel;
	for (i = 0; i < nlevel; i++) {
	    clevel[i] = zmaxmin[1] + ((double) i + 0.5)*dz;
	}
	pls.col0(3);
	pls.lsty(2);
	pls.cont(z,1,nr,1,ntheta,clevel,xg,yg);
	pls.lsty(1);
	pls.col0(1);
    
	// Plot the vectors of the gradient of the potential
	pls.col0(2);
	pls.vect(u,v,25.0,xg,yg);
	pls.col0(1);

	// Plot the perimeter of the cylinder
	for (i=0;i<nper;i++) {
	    theta = (2.*Math.PI/(nper-1))*(double)i;
	    px[i] = rmax*Math.cos(theta);
	    py[i] = rmax*Math.sin(theta);
	}
	pls.line(px,py);
    
    }

    void f2mnmx(double [][] f, int nx, int ny, double[] fmaxmin)
    {
	int i, j;

	fmaxmin[0] = f[0][0];
	fmaxmin[1] = fmaxmin[0];

	for (i = 0; i < nx; i++) {
	    for (j = 0; j < ny; j++) {
		fmaxmin[0] = MAX(fmaxmin[0], f[i][j]);
		fmaxmin[1] = MIN(fmaxmin[1], f[i][j]);
	    }
	}
    }


    public x22( String[] args) {

	int fill;

	// Set of points making a polygon to use as the arrow
	final double arrow_x[] = {-0.5, 0.5, 0.3, 0.5, 0.3, 0.5};
	final double arrow_y[] = {0.0, 0.0, 0.2, 0.0, -0.2, 0.0};
	final double arrow2_x[] = {-0.5, 0.3, 0.3, 0.5, 0.3, 0.3};
	final double arrow2_y[] = {0.0, 0.0,   0.2, 0.0, -0.2, 0.0};


	// Parse and process command line arguments 

	pls.parseopts(args, pls.PL_PARSE_FULL | pls.PL_PARSE_NOPROGRAM);

	// Initialize plplot 

	pls.init();

	nx = 20;
	ny = 20;

	// Allocate arrays
	u = new double[nx][ny];
	v = new double[nx][ny];
	xg = new double[nx][ny];
	yg = new double[nx][ny];

	circulation();

	fill = 0;

	// Set arrow style using arrow_x and arrow_y then 
	// plot uMath.sing these arrows.
	pls.svect(arrow_x, arrow_y, fill);
	constriction();

	// Set arrow style using arrow2_x and arrow2_y then 
	// plot using these filled arrows.
	fill = 1;
	pls.svect(arrow2_x, arrow2_y, fill);
	constriction();

	potential();

	pls.end();
    }

    public static void main( String[] args ) {
	x22 x = new x22( args );

    }
}



//---------------------------------------------------------------------------//
//                              End of x22.java
//---------------------------------------------------------------------------//

