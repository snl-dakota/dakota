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
#include <utilib/std_headers.h>
#include <utilib/stl_auxiliary.h>
#include <utilib/Tuple.h>
#include <utilib/CharString.h>
#include <utilib/mpiUtil.h>
#include <cxxtest/TestSuite.h>

namespace utilib {

namespace unittest { class Test_Tuple; }
class utilib::unittest::Test_Tuple : public CxxTest::TestSuite
{
public:

   void test_printer()
      {
      std::ostringstream ss;
      utilib::Tuple1<int> a(3);
      ss << a;
      TS_ASSERT_EQUALS(ss.str(), "( 3 )");

      ss.str("");
      utilib::Tuple2<int, utilib::CharString> b(4, "boo");
      ss << b;
      TS_ASSERT_EQUALS(ss.str(), "( 4, boo )");

      ss.str("");
      utilib::Tuple7<int, double, utilib::CharString, char*, int, int, int> 
          e(1, 2, "aa", "b", 3, 4, 5);
      ss << e;
      TS_ASSERT_EQUALS(ss.str(), "( 1, 2, aa, b, 3, 4, 5 )");
   }

};

} // namespace utilib
