/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

#include <acro_config.h>
#include <colin/ObjectType.h>
#include <cxxtest/TestSuite.h>

namespace colin {
namespace unittest { class ObjectType; }

class colin::unittest::ObjectType : public CxxTest::TestSuite
{
public:

   void test_get()
   {
      TS_TRACE("Testing colin::ObjectType construction");
      colin::ObjectType tmp1 = colin::ObjectType::get<int>();
      int i;
      colin::ObjectType tmp2 = colin::ObjectType::get(&i);
      TS_ASSERT_EQUALS(tmp1,tmp2);
      float j;
      colin::ObjectType tmp3 = colin::ObjectType::get(&j);
      TS_ASSERT(tmp1 != tmp3);
   }

   void test_less_than()
   {
      TS_TRACE("Testing colin::ObjectType less-than comparison");
      int i;
      colin::ObjectType ot_i = colin::ObjectType::get(&i);
      float j;
      colin::ObjectType ot_f = colin::ObjectType::get(&j);
      TS_ASSERT( (ot_f < ot_i) ^ (ot_i < ot_f) );

      int k;
      colin::ObjectType ot_i2 = colin::ObjectType::get(&k);
      TS_ASSERT( ! ( ot_i2 < ot_i ) );
      TS_ASSERT( ! ( ot_i < ot_i2 ) );
   }

   void test_name()
   {
      TS_TRACE("Testing the name() method");
      std::string demangledName = utilib::demangledName(typeid(int*));

      colin::ObjectType i = colin::ObjectType::get<int>();
      TS_ASSERT_EQUALS(i.name(), demangledName);

      colin::ObjectType iPtr = colin::ObjectType::get<int*>();
      TS_ASSERT_EQUALS(iPtr.name(), demangledName);
   }

   void test_mangledName()
   {
      TS_TRACE("Testing the mangledName() method");
      std::string mangledName = utilib::mangledName(typeid(int*));

      colin::ObjectType i = colin::ObjectType::get<int>();
      TS_ASSERT_EQUALS(i.mangledName(), mangledName);

      colin::ObjectType iPtr = colin::ObjectType::get<int*>();
      std::string s = iPtr.mangledName();
      TS_ASSERT_EQUALS(s, mangledName);
   }

   void test_typeinfo()
   {
      TS_TRACE("Testing the typeinfo() method");
      colin::ObjectType i1 = colin::ObjectType::get<int>();
      colin::ObjectType i2 = colin::ObjectType::get<int>();
      TS_ASSERT(i1.type() != 0);
      TS_ASSERT_EQUALS(i1.type(), i2.type());

      colin::ObjectType iPtr = colin::ObjectType::get<int*>();
      TS_ASSERT_EQUALS(i1.type(), iPtr.type());

      colin::ObjectType d = colin::ObjectType::get<double>();
      TS_ASSERT(i1.type() != d.type());
   }

};

} // namespace colin
