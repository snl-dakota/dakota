//---------------------------------------------------------------------------//
// $Id: x10.java 3186 2006-02-15 18:17:33Z slbrow $
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
// Implementation of PLplot example 10 in Java.
//---------------------------------------------------------------------------//

package plplot.examples;

import plplot.core.*;

import java.lang.Math;

class x10 {

    public static void main( String[] args ) 
    {
        x10 x = new x10( args );
    }

    public x10( String[] args )
    {
       PLStream pls = new PLStream();

    // Parse and process command line arguments.

        pls.parseopts( args, pls.PL_PARSE_FULL | pls.PL_PARSE_NOPROGRAM );

    // Initialize plplot.

        pls.init();

        pls.adv(0);
        pls.vpor(0.0, 1.0, 0.0, 1.0);
        pls.wind(0.0, 1.0, 0.0, 1.0);
        pls.box("bc", 0.0, 0, "bc", 0.0, 0);

        pls.svpa(50.0, 150.0, 50.0, 100.0);
        pls.wind(0.0, 1.0, 0.0, 1.0);
        pls.box("bc", 0.0, 0, "bc", 0.0, 0);
        pls.ptex(0.5, 0.5, 1.0, 0.0, 0.5, "BOX at (50,150,50,100)");
        pls.end();
    }
}

//---------------------------------------------------------------------------//
//                              End of x10.java
//---------------------------------------------------------------------------//
