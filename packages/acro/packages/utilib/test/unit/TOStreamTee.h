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

#include "CommonTestUtils.h"

#include <utilib/OStreamTee.h>

namespace utilib {

namespace unittest { class Test_OStreamTee; }
class utilib::unittest::Test_OStreamTee : public CxxTest::TestSuite
{
public:
   void test_ostream_tee()
   {
      utilib::OStreamTee tee1(std::cout);
      std::cout << "A";
      TS_ASSERT_EQUALS(tee1.out.str(), "A");
      {
         utilib::OStreamTee tee2(std::cout);
         std::cout << "B";
         TS_ASSERT_EQUALS(tee2.out.str(), "B");
         TS_ASSERT_EQUALS(tee1.out.str(), "AB");
      }
      std::cout << "A";
      TS_ASSERT_EQUALS(tee1.out.str(), "ABA");
   }
};

} // namespace utilib
