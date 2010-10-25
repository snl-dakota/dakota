//---------------------------------------------------------------------------//
// $Id: x10.cc 3186 2006-02-15 18:17:33Z slbrow $
//---------------------------------------------------------------------------//
//
//---------------------------------------------------------------------------//
// Copyright (C) 2004  Andrew Ross <andrewr@coriolis.greenend.org.uk>
// Copyright (C) 2004  Alan W. Irwin
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
// Implementation of PLplot example 10 in C++.
//---------------------------------------------------------------------------//

#include "plstream.h"

#include <iostream>
#include <cmath>

#ifdef USE_NAMESPACE
using namespace std;
#endif

class x10 {

public:
  x10(int, char**);

private:
  plstream *pls;

};

x10::x10( int argc, char **argv ) {

  pls = new plstream();

  // Parse and process command line arguments.

  pls->parseopts( &argc, argv, PL_PARSE_FULL );

  // Initialize plplot.

  pls->init();

  pls->adv(0);
  pls->vpor(0.0, 1.0, 0.0, 1.0);
  pls->wind(0.0, 1.0, 0.0, 1.0);
  pls->box("bc", 0.0, 0, "bc", 0.0, 0);

  pls->svpa(50.0, 150.0, 50.0, 100.0);
  pls->wind(0.0, 1.0, 0.0, 1.0);
  pls->box("bc", 0.0, 0, "bc", 0.0, 0);
  pls->ptex(0.5, 0.5, 1.0, 0.0, 0.5, "BOX at (50,150,50,100)");
  //  pls->end();
  delete pls;
}



int main( int argc, char **argv )
{
  x10 *x = new x10( argc, argv );

  delete x;
}


//---------------------------------------------------------------------------//
//                              End of x10.cc
//---------------------------------------------------------------------------//
