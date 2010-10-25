//---------------------------------------------------------------------------//
// $Id: x06.java 3186 2006-02-15 18:17:33Z slbrow $
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
// Implementation of PLplot example 6 in Java.
//---------------------------------------------------------------------------//

package plplot.examples;

import plplot.core.*;

import java.lang.Math;
import java.text.*;

class x06 {

    public static void main( String[] args ) 
    {
        x06 x = new x06( args );
    }

    public x06( String[] args )
    {
       PLStream pls = new PLStream();

        NumberFormat nf = NumberFormat.getNumberInstance();

        int i, j, k;
        double x[] = new double[1];
        double y[] = new double[1];

    // Parse and process command line arguments.

        pls.parseopts( args, pls.PL_PARSE_FULL | pls.PL_PARSE_NOPROGRAM );

    // Initialize plplot

        pls.init();

        pls.adv(0);

    // Set up viewport and window.

        pls.col0(2);
        pls.vpor(0.1, 1.0, 0.1, 0.9);
        pls.wind(0.0, 1.0, 0.0, 1.3);

    // Draw the grid using plbox.

        pls.box("bcg", 0.1, 0, "bcg", 0.1, 0);

    // Write the digits below the frame.

        pls.col0(15);
        for( i = 0; i <= 9; i++ ) {
            String text = nf.format(i);
            pls.mtex("b", 1.5, (0.1 * i + 0.05), 0.5, text);
        }

        k = 0;
        for( i = 0; i <= 12; i++ ) {

        // Write the digits to the left of the frame.

            String text = nf.format( 10*i);
            pls.mtex("lv", 1.0, (1.0 - (2 * i + 1) / 26.0), 1.0, text);
            for( j = 0; j <= 9; j++ ) {
                x[0] = 0.1 * j + 0.05;
                y[0] = 1.25 - 0.1 * i;

            // Display the symbols (plpoin expects that x and y are arrays so
            // that is what we feed it).

                if (k < 128)
                    pls.poin( x, y, k );
                k = k + 1;
            }
        }

        pls.mtex("t", 1.5, 0.5, 0.5, "PLplot Example 6 - plpoin symbols");
        pls.end();
    }
}

//---------------------------------------------------------------------------//
//                              End of x06.java
//---------------------------------------------------------------------------//
