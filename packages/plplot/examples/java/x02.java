//---------------------------------------------------------------------------//
// $Id: x02.java 3186 2006-02-15 18:17:33Z slbrow $
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// Copyright (C) 2001  Geoffrey Furnish
// Copyright (C) 2002  Alan W. Irwin
// Copyright (C) 2004,2005  Andrew Ross
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
// Implementation of PLplot example 2 in Java.
//---------------------------------------------------------------------------//

package plplot.examples;

import plplot.core.*;

import java.lang.Math;
import java.text.*;

class x02 {

    private  PLStream pls;

    public static void main( String[] args ) 
    {
        x02 x = new x02( args );
    }

    x02( String[] args )
    {
        int i, j;
        double vmin, vmax;

        pls = new PLStream();

	// Parse and process command line arguments.

        pls.parseopts( args, pls.PL_PARSE_FULL | pls.PL_PARSE_NOPROGRAM );

	// Initialize plplot.

        pls.init();
	demo1();
	demo2();

        pls.end();
    }

    void demo1()
    {

	pls.bop();

	// Divide screen into 16 regions.

        pls.ssub(4, 4);

	draw_windows(16,0);

	pls.eop();
    }

    void demo2()
    {
	// Set up cmap0
	// Use 100 custom colors in addition to base 16
	int[] r = new int[116];
	int[] g = new int[116];
	int[] b = new int[116];
	int[] r1i = new int[1];
	int[] g1i = new int[1];
	int[] b1i = new int[1];
	
	// Min and max lightness values
	double lmin = 0.15, lmax = 0.85;

	int i;
	
	pls.bop();

	// Divide screen into 100 regions.
	pls.ssub(10,10);

	for (i = 0; i <= 99; i++) {
	    double h, l, s;
	    double[] r1 = new double[1];
	    double[] g1 = new double[1];
	    double[] b1 = new double[1];

    
	    // Bounds on HLS, from plhlsrgb() commentary --
	    //	hue		[0., 360.]	degrees
	    //	lightness	[0., 1.]	magnitude
	    //	saturation	[0., 1.]	magnitude
	    //
	    
	    // Vary hue uniformly from left to right
	    h = (360. / 10. ) * ( i % 10 );
	    // Vary lightness uniformly from top to bottom, between min & max
	    l = lmin + (lmax - lmin) * (i / 10) / 9.;
	    // Use max saturation
	    s = 1.0;
	    
	    pls.hlsrgb(h, l, s, r1, g1, b1);
    
	    r[i+16] = (int)(r1[0] * 255);
	    g[i+16] = (int)(g1[0] * 255);
	    b[i+16] = (int)(b1[0] * 255);
	}
  
	// Load default cmap0 colors into our custom set
	for (i = 0; i <= 15; i++) {
	    pls.gcol0(i, r1i, g1i, b1i);
	    r[i] = r1i[0];
	    g[i] = g1i[0];
	    b[i] = b1i[0];
	}
	

	// Now set cmap0 all at once (faster, since fewer driver calls)
	pls.scmap0(r, g, b);
  
	draw_windows( 100, 16 );
	
	pls.eop();

    }

    // --------------------------------------------------------------------
    // draw_windows
    //
    // Draws a set of numbered boxes with colors according to cmap0 entry.
    // --------------------------------------------------------------------
    void draw_windows(int nw, int cmap0_offset) {
	int i,j;
	double vmin, vmax;
        NumberFormat nf = NumberFormat.getNumberInstance();


        pls.schr(0.0, 3.5);
        pls.font(4);

        for (i = 0; i < nw; i++) {
            pls.col0(i+cmap0_offset);

            String text = nf.format(i);

            pls.adv(0);
            vmin = 0.1;
            vmax = 0.9;
            for (j = 0; j <= 2; j++) {
                pls.wid(j + 1);
                pls.vpor(vmin, vmax, vmin, vmax);
                pls.wind(0.0, 1.0, 0.0, 1.0);
                pls.box("bc", 0.0, 0, "bc", 0.0, 0);
                vmin = vmin + 0.1;
                vmax = vmax - 0.1;
            }
            pls.wid(1);
            pls.ptex(0.5, 0.5, 1.0, 0.0, 0.5, text);
        }
    }
}

//---------------------------------------------------------------------------//
//                              End of x02.java
//---------------------------------------------------------------------------//
