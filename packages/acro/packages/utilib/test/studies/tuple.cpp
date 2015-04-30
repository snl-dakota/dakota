/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */
#include <utilib_config.h>
#include <utilib/Tuple.h>
#include <utilib/CharString.h>
#include <utilib/CommonIO.h>

using namespace utilib;

int test_tuple(int,char**)
{
utilib::Tuple1<int> a(3);
utilib::Tuple2<int,utilib::CharString> b(4,"boo");
utilib::Tuple7<int,double,utilib::CharString,char*,int,int,int> e(1,2,"aa","b",3,4,5);
ucout << a << std::endl;
ucout << b << std::endl;
ucout << e << std::endl;
return 0;
}


