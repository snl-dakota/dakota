//---------------------------------------------------------------------------//
// $Id: x07.java 3186 2006-02-15 18:17:33Z slbrow $
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// Copyright (C) 2001  Geoffrey Furnish
// Copyright (C) 2001, 2002  Alan W. Irwin
// Copyright (C) 2001  Andrew Ross
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
// Implementation of PLplot example 7 in Java.
//---------------------------------------------------------------------------//

package plplot.examples;

import plplot.core.*;

import java.lang.Math;
import java.text.*;

class x07 {

    static int base[] = {
        0, 200, 500, 600, 700, 800, 900,
        2000, 2100, 2200, 2300, 2400, 2500, 2600, 2700, 2800, 2900
    };

    public static void main( String[] args ) 
    {
        x07 x = new x07( args );
    }

    public x07( String[] args )
    {
       PLStream pls = new PLStream();

//        NumberFormat nf = NumberFormat.getNumberInstance();
//        Change to this custom format to get stupid locale commas
//        separating hundreds and thousands place out of labels.
        DecimalFormat nf = new DecimalFormat("#####");

        int i, j, k, l;
        double x[] = new double[1];
        double y[] = new double[1];

    // Parse and process command line arguments.

        pls.parseopts( args, pls.PL_PARSE_FULL | pls.PL_PARSE_NOPROGRAM );

    // Initialize plplot.

        pls.init();

        pls.fontld(1);
        for( l = 0; l < base.length; l++ ) {
            pls.adv(0);

        // Set up viewport and window.

            pls.col0(2);
            pls.vpor(0.15, 0.95, 0.1, 0.9);
            pls.wind(0.0, 1.0, 0.0, 1.0);

        // Draw the grid using plbox.

            pls.box("bcg", 0.1, 0, "bcg", 0.1, 0);

        // Write the digits below the frame.

            pls.col0(15);
            for (i = 0; i <= 9; i++) {
                String text = nf.format(i);
                pls.mtex("b", 1.5, (0.1 * i + 0.05), 0.5, text);
            }

            k = 0;
            for( i = 0; i <= 9; i++ ) {

            // Write the digits to the left of the frame.

                String text = nf.format( base[l] + 10*i );
                pls.mtex("lv", 1.0, (0.95 - 0.1 * i), 1.0, text);
                for( j = 0; j <= 9; j++ ) {
                    x[0] = 0.1 * j + 0.05;
                    y[0] = 0.95 - 0.1 * i;

                // Display the symbols.
		// N.B. plsym expects arrays so that is what we give it.

                    pls.sym( x, y, base[l] + k );
                    k = k + 1;
                }
            }

            pls.mtex("t", 1.5, 0.5, 0.5, "PLplot Example 7 - PLSYM symbols");
        }
        pls.end();
    }
}

//---------------------------------------------------------------------------//
//                              End of x07.java
//---------------------------------------------------------------------------//
