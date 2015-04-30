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

#include <utilib/BitArray.h>

namespace utilib {
namespace unittest { class Test_BitArray; }

class utilib::unittest::Test_BitArray : public CxxTest::TestSuite
{
public:
   void test_internal_assign()
   {
      utilib::BitArray a(2);
      TS_ASSERT( ! a[0] );
      TS_ASSERT( ! a[1] );
      a[1] = 1;
      TS_ASSERT( ! a[0] );
      TS_ASSERT(   a[1] );
      a[0] = a[1];
      TS_ASSERT(   a[0] );
      TS_ASSERT(   a[1] );
   }

   void test_element_assign()
   {
      utilib::BitArray a(1);
      utilib::BitArray b(1);
      TS_ASSERT( ! a[0] );
      TS_ASSERT( ! b[0] );
      a[0] = 1;
      TS_ASSERT(   a[0] );
      TS_ASSERT( ! b[0] );
      b[0] = a[0];
      TS_ASSERT(   a[0] );
      TS_ASSERT(   b[0] );
   }

};

} // namespace utilib
