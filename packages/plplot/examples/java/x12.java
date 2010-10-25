//---------------------------------------------------------------------------//
// $Id: x12.java 3186 2006-02-15 18:17:33Z slbrow $
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// Copyright (C) 2001  Geoffrey Furnish
// Copyright (C) 2001, 2002  Alan W. Irwin
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
// Implementation of PLplot example 12 in Java.
//---------------------------------------------------------------------------//

package plplot.examples;

import plplot.core.*;

import java.lang.Math;
import java.text.*;

class x12 {

    static double y0[] = {5., 15., 12., 24., 28., 30., 20., 8., 12., 3.};
   
    PLStream pls = new PLStream();
   
    public static void main( String[] args ) 
    {
        x12 x = new x12( args );
    }

    public x12( String[] args )
    {
        NumberFormat nf = NumberFormat.getNumberInstance();
//        Change to this custom format to get stupid locale commas
//        separating hundreds and thousands place out of labels.
	DecimalFormat nf4 = new DecimalFormat("####");
	int i;

        pls.parseopts( args, pls.PL_PARSE_FULL | pls.PL_PARSE_NOPROGRAM );

        pls.init();

        pls.adv(0);
        pls.vsta();
        pls.wind(1980.0, 1990.0, 0.0, 35.0);
        pls.box("bc", 1.0, 0, "bcnv", 10.0, 0);
        pls.col0(2);
        pls.lab("Year", "Widget Sales (millions)", "#frPLplot Example 12");
        for (i = 0; i < 10; i++) {
            pls.col0(i + 1);
//             pls.col1((double) ((i + 1)/10.0));
            pls.psty(0);
            plfbox((1980. + i), y0[i]);
//	   sprintf(string, "%.0f", y0[i]);
            String text = nf.format( (int) (y0[i]+0.5));
            pls.ptex((1980. + i + .5), (y0[i] + 1.), 1.0, 0.0, .5, text);
//	   sprintf(string, "%d", 1980 + i);
            String text1 = nf4.format( 1980 + i );
            pls.mtex("b", 1.0, ((i + 1) * .1 - .05), 0.5, text1);
	}
        pls.end();
    }

    void plfbox(double x0, double y0)
    {
	double[] x = new double[4];
	double[] y = new double[4];
	
	x[0] = x0;
	y[0] = 0.;
	x[1] = x0;
	y[1] = y0;
	x[2] = x0 + 1.;
	y[2] = y0;
	x[3] = x0 + 1.;
	y[3] = 0.;
        pls.fill(x, y);
	pls.col0(1);
        pls.lsty(1);
	pls.line(x, y);
    }
}

//---------------------------------------------------------------------------//
//                              End of x12.java
//---------------------------------------------------------------------------//
