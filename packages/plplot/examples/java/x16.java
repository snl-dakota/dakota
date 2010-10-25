//---------------------------------------------------------------------------//
// $Id: x16.java 3186 2006-02-15 18:17:33Z slbrow $
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// Copyright (C) 2001  Geoffrey Furnish
// Copyright (C) 2002  Alan W. Irwin
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

//---------------------------------------------------------------------------//
// Implementation of PLplot example 16 in Java.
//---------------------------------------------------------------------------//

package plplot.examples;

import plplot.core.*;

import java.lang.Math;

class x16 {

   // number of shade levels, x data points, y data points, and perimeter points.
   static final int NSHADES = 20;
   static final int XPTS = 35;
   static final int YPTS = 46;
   static final int PERIMETERPTS = 100;

   // calculated constants and array that depends on them
   static final double XSPA =  2./(XPTS-1);
   static final double YSPA =  2./(YPTS-1);
   final double tr[] = {XSPA, 0.0, -1.0, 0.0, YSPA, -1.0};

   PLStream pls = new PLStream();

   double fmin, fmax;

   // Compute min and max value of a 2-d array.

   void f2mnmx( double[][] f, int nx, int ny )
     {
        fmax = f[0][0];
        fmin = fmax;
	
        for( int i=0; i < nx; i++ )
	  for( int j=0; j < ny; j++ ) {
	     if (f[i][j] < fmin) fmin = f[i][j];
	     if (f[i][j] > fmax) fmax = f[i][j];
	  }
     }

   // Does a variety of shade plots with continuous (cmap1) colours

   public static void main( String[] args ) 
     {
        x16 x = new x16( args );
     }

   public x16( String[] args )
     {
	int i, j;
	double x, y, argx, argy, distort, r, t, zmin, zmax;
	double[] px = new double[PERIMETERPTS];
	double[] py = new double[PERIMETERPTS];
	double[][] xg0 = new double[XPTS][YPTS];
	double[][] yg0 = new double[XPTS][YPTS];
	double[][] xg1 = new double[XPTS][YPTS];
	double[][] yg1 = new double[XPTS][YPTS];
	double[][] z = new double[XPTS][YPTS];
	double[][] w = new double[XPTS][YPTS];
	double[][] xg2 = new double[XPTS][YPTS];
	double[][] yg2 = new double[XPTS][YPTS];
	double[] clevel = new double[NSHADES];
	double[] shedge = new double[NSHADES+1];
	final int fill_width = 2, cont_color = 0, cont_width = 0;

	// Parse and process command line arguments.
	pls.parseopts( args, pls.PL_PARSE_FULL | pls.PL_PARSE_NOPROGRAM );

	// Reduce colors in cmap 0 so that cmap 1 is useful on a 
	//16-color display
	pls.scmap0n(3);

	//Initialize plplot
         
          pls.init();
	// Set up data array

	for (i = 0; i < XPTS; i++) {
	   x = (double) (i - (XPTS / 2)) / (double) (XPTS / 2);
	   for (j = 0; j < YPTS; j++) {
	      y = (double) (j - (YPTS / 2)) / (double) (YPTS / 2) - 1.0;

	      z[i][j] = - Math.sin(7.*x) * Math.cos(7.*y) + x*x - y*y;
	      w[i][j] = - Math.cos(7.*x) * Math.sin(7.*y) + 2 * x * y;
	   }
	}

	f2mnmx( z, XPTS, YPTS );
	zmin = fmin;
	zmax = fmax;
	
	for (i = 0; i < NSHADES; i++)
	  clevel[i] = zmin + (zmax - zmin) * (i + 0.5) / (double) NSHADES;

	for (i = 0; i < NSHADES+1; i++)
	  shedge[i] = zmin + (zmax - zmin) * (double) i / (double) NSHADES;

	for (i = 0; i < XPTS; i++) {
	   for (j = 0; j < YPTS; j++) {
	      // Replacement for mypltr of x16c.c
	      x = tr[0] * i + tr[1] * j + tr[2];
	      y = tr[3] * i + tr[4] * j + tr[5];

	      argx = x * Math.PI/2;
	      argy = y * Math.PI/2;
	      distort = 0.4;

	      // Note xg0 ==> yg1 are one-dimensional because of arrangement of
	      //zeros in the final tr definition above.  However, for now
	      //we are using raw interface here so must nominally treat them
	      //as two-dimensional.
	      xg0[i][j] = x;
	      yg0[i][j] = y;

	      xg1[i][j] = x + distort * Math.cos(argx);
	      yg1[i][j] = y - distort * Math.cos(argy);

	      xg2[i][j] = x + distort * Math.cos(argx) * Math.cos(argy);
	      yg2[i][j] = y - distort * Math.cos(argx) * Math.cos(argy);
	   }
	}

	// Plot using identity transform

	pls.adv(0);
	pls.vpor(0.1, 0.9, 0.1, 0.9);
	pls.wind(-1.0, 1.0, -1.0, 1.0);

	pls.psty(0);

	pls.shades(z, -1., 1., -1., 1., 
		 shedge, fill_width,
		 cont_color, cont_width,
		 1, xg0, yg0);

	pls.col0(1);
	pls.box("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
	pls.col0(2);
	pls.lab("distance", "altitude", "Bogon density");

	// Plot using 1d coordinate transform
    
	pls.adv(0);
	pls.vpor(0.1, 0.9, 0.1, 0.9);
	pls.wind(-1.0, 1.0, -1.0, 1.0);

	pls.psty(0);

	pls.shades(z, -1., 1., -1., 1., 
	     shedge, fill_width,
	     cont_color, cont_width,
	     1, xg1, yg1);

	pls.col0(1);
	pls.box("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
	pls.col0(2);

	pls.lab("distance", "altitude", "Bogon density");

	// Plot using 2d coordinate transform

	pls.adv(0);
	pls.vpor(0.1, 0.9, 0.1, 0.9);
	pls.wind(-1.0, 1.0, -1.0, 1.0);

	pls.psty(0);

	pls.shades(z, -1., 1., -1., 1., 
		 shedge, fill_width,
		 cont_color, cont_width,
		 0, xg2, yg2);

	pls.col0(1);
	pls.box("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
	pls.col0(2);
	pls.cont(w, 1, XPTS, 1, YPTS, clevel, xg2, yg2);

	pls.lab("distance", "altitude", "Bogon density, with streamlines");

	// Plot using 2d coordinate transform

	pls.adv(0);
	pls.vpor(0.1, 0.9, 0.1, 0.9);
	pls.wind(-1.0, 1.0, -1.0, 1.0);

	pls.psty(0);

	pls.shades(z, -1., 1., -1., 1., 
		 shedge, fill_width,
		 2, 3,
		 0, xg2, yg2);

	pls.col0(1);
	pls.box("bcnst", 0.0, 0, "bcnstv", 0.0, 0);
	pls.col0(2);

	pls.lab("distance", "altitude", "Bogon density");

	// Note this exclusion API will probably change so don't bother
	// with it for x16.java example.


	//Example with polar coordinates.

	pls.adv(0);
	pls.vpor( .1, .9, .1, .9 );
	pls.wind( -1., 1., -1., 1. );

	pls.psty(0);

	// Build new coordinate matrices.
    
	for (i = 0; i < XPTS; i++) {
	   r = ((double) i)/ (XPTS-1);
	   for (j = 0; j < YPTS; j++) {
	      t = (2.*Math.PI/(YPTS-1.))*j;
	      xg2[i][j] = r*Math.cos(t);
	      yg2[i][j] = r*Math.sin(t);
	      z[i][j] = Math.exp(-r*r)*Math.cos(5.*Math.PI*r)*Math.cos(5.*t);
	   }
	}

	//Need a new shedge to go along with the new data set.

	f2mnmx( z, XPTS, YPTS );
	zmin = fmin;
	zmax = fmax;

	for (i = 0; i < NSHADES+1; i++)
	  shedge[i] = zmin + (zmax - zmin) * (double) i / (double) NSHADES;

	pls.shades(z, -1., 1., -1., 1., 
		 shedge, fill_width,
		 cont_color, cont_width,
		 0, xg2, yg2);

// Now we can draw the perimeter.  (If do before, shade stuff may overlap.)
	for (i = 0; i < PERIMETERPTS; i++) {
	   t = (2.*Math.PI/(PERIMETERPTS-1))*(double)i;
	   px[i] = Math.cos(t);
	   py[i] = Math.sin(t);
	}
	pls.col0(1);
	pls.line(px, py);
                  
	// And label the plot.

	pls.col0(2);
	pls.lab( "", "",  "Tokamak Bogon Instability" );

	// Clean up 

	pls.end();

     }
}

//---------------------------------------------------------------------------//
//                              End of x16.java
//---------------------------------------------------------------------------//

