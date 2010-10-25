//---------------------------------------------------------------------------//
// $Id: x11.java 3186 2006-02-15 18:17:33Z slbrow $
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// Copyright (C) 2001  Geoffrey Furnish
// Copyright (C) 2001, 2002, 2003  Alan W. Irwin
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
// Implementation of PLplot example 11 in Java.
//---------------------------------------------------------------------------//

package plplot.examples;

import plplot.core.*;

import java.lang.Math;

class x11 {

    PLStream pls = new PLStream();

    static final int XPTS = 35;
    static final int YPTS = 46;
    static final int LEVELS = 10;

    static int opt[] = {3, 3};
    static double alt[] = {33.0, 17.0};
    static double az[] = {24.0, 115.0};

    static String[] title =
    {
        "#frPLplot Example 11 - Alt=33, Az=24, Opt=3",
        "#frPLplot Example 11 - Alt=17, Az=115, Opt=3"
    };

    void cmap1_init()
    {
	double [] i = new double[2];
	double [] h = new double[2];
	double [] l = new double[2];
	double [] s = new double[2];
	int [] rev = new int[2];
	  
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
	  
        pls.scmap1n(256);
        pls.scmap1l(0, i, h, l, s, rev);
    }

// Does a series of mesh plots for a given data set, with different viewing
// options in each plot.

    public static void main( String[] args ) 
    {
        x11 x = new x11( args );
    }

    public x11( String[] args )
    {
        int i, j, k;

        double[] x = new double[ XPTS ];
        double[] y = new double[ YPTS ];
        double[][] z = new double[XPTS][YPTS];
	double zmin=Double.MAX_VALUE, zmax=Double.MIN_VALUE;

        double xx, yy;
        int nlevel = LEVELS;
        double[] clevel = new double[LEVELS];
        double step;
       

    // Parse and process command line arguments.

        pls.parseopts( args, pls.PL_PARSE_FULL | pls.PL_PARSE_NOPROGRAM );

    // Initialize plplot.

        pls.init();

        for( i=0; i < XPTS; i++ )
            x[i] = 3.* (double) (i - (XPTS/2)) / (double) (XPTS/2);

        for( j=0; j < YPTS; j++ )
            y[j] = 3.* (double) (j - (YPTS/2)) / (double) (YPTS/2);

        for( i = 0; i < XPTS; i++ )
        {
            xx = x[i];
            for( j = 0; j < YPTS; j++ )
            {
                yy = y[j];
                z[i][j] = 3. * (1.-xx)*(1.-xx) * Math.exp(-(xx*xx) - (yy+1.)*(yy+1.)) -
		         10. * (xx/5. - Math.pow(xx,3.) - Math.pow(yy,5.)) * Math.exp(-xx*xx-yy*yy) -
		         1./3. * Math.exp(-(xx+1)*(xx+1) - (yy*yy));
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
	       pls.adv(0);
	       pls.col0(1);
	       pls.vpor(0.0, 1.0, 0.0, 0.9);
	       pls.wind(-1.0, 1.0, -1.0, 1.5);

	       pls.w3d( 1.0, 1.0, 1.2, -3.0, 3.0, -3.0, 3.0, zmin, zmax,
			  alt[k], az[k] );
	       pls.box3( "bnstu", "x axis", 0.0, 0,
			   "bnstu", "y axis", 0.0, 0,
			   "bcdmnstuv", "z axis", 0.0, 4 );

	       pls.col0(2);

	       /* wireframe plot */
	       if (i==0)
		 pls.mesh(x, y, z, opt[k]);
	       
	       /* magnitude colored wireframe plot */
	       else if (i==1)
		 pls.mesh(x, y, z, opt[k] | pls.MAG_COLOR);
	       
	       /* magnitude colored wireframe plot with sides */
	       else if (i==2)
		 pls.plot3d(x, y, z, opt[k] | pls.MAG_COLOR, 1);
	       
	       /* magnitude colored wireframe plot with base contour */
	       else if (i==3)
		 pls.meshc(x, y, z, opt[k] | pls.MAG_COLOR | pls.BASE_CONT,
			 clevel);
	       
	       
	       pls.col0(3);
	       pls.mtex("t", 1.0, 0.5, 0.5, title[k]);
	    }
	}

        pls.end();
    }
}

//---------------------------------------------------------------------------//
//                              End of x11.java
//---------------------------------------------------------------------------//
