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

#include <utilib/ReferenceCounted.h>

namespace utilib {
namespace {

class ReferenceCounted_tester
{
public:
   ReferenceCounted_tester()
   {
      ++count();
   }

   ~ReferenceCounted_tester()
   {
      --count();
   }

   static int& count()
   {
      static int myCount = 0;
      return myCount;
   }
};

} // namespace (local)

namespace unittest { class ReferenceCounted; }
class utilib::unittest::ReferenceCounted : public CxxTest::TestSuite
{
public:
   void test_constructor()
   {
      TS_ASSERT_EQUALS(ReferenceCounted_tester::count(), 0);
      {
         utilib::ReferenceCounted<ReferenceCounted_tester> i;
         TS_ASSERT_EQUALS(i.use_count(), (size_t)1);
         TS_ASSERT_EQUALS(ReferenceCounted_tester::count(), 1);
      }
      TS_ASSERT_EQUALS(ReferenceCounted_tester::count(), 0);
   }


   void test_copy_constructor()
   {
      TS_ASSERT_EQUALS(ReferenceCounted_tester::count(), 0);
      {
         utilib::ReferenceCounted<ReferenceCounted_tester> i;
         TS_ASSERT_EQUALS(i.use_count(), (size_t)1);
         TS_ASSERT_EQUALS(ReferenceCounted_tester::count(), 1);
         {
            utilib::ReferenceCounted<ReferenceCounted_tester> j = i;
            TS_ASSERT_EQUALS(i.use_count(), (size_t)2);
            TS_ASSERT_EQUALS(ReferenceCounted_tester::count(), 1);
         }
         TS_ASSERT_EQUALS(i.use_count(), (size_t)1);
         TS_ASSERT_EQUALS(ReferenceCounted_tester::count(), 1);
      }
      TS_ASSERT_EQUALS(ReferenceCounted_tester::count(), 0);
   }


   void test_assignment()
   {
      TS_ASSERT_EQUALS(ReferenceCounted_tester::count(), 0);
      {
         utilib::ReferenceCounted<ReferenceCounted_tester> i;
         TS_ASSERT_EQUALS(i.use_count(), (size_t)1);
         TS_ASSERT_EQUALS(ReferenceCounted_tester::count(), 1);

         {
            utilib::ReferenceCounted<ReferenceCounted_tester> j;
            TS_ASSERT_EQUALS(j.use_count(), (size_t)1);
            TS_ASSERT_EQUALS(ReferenceCounted_tester::count(), 2);
            
            j = i;

            TS_ASSERT_EQUALS(i.use_count(), (size_t)2);
            TS_ASSERT_EQUALS(ReferenceCounted_tester::count(), 1);

            utilib::ReferenceCounted<ReferenceCounted_tester> k;
            TS_ASSERT_EQUALS(k.use_count(), (size_t)1);
            TS_ASSERT_EQUALS(ReferenceCounted_tester::count(), 2);
            
            i = k;

            TS_ASSERT_EQUALS(i.use_count(), (size_t)2);
            TS_ASSERT_EQUALS(j.use_count(), (size_t)1);
            TS_ASSERT_EQUALS(ReferenceCounted_tester::count(), 2);
         }

         TS_ASSERT_EQUALS(i.use_count(), (size_t)1);
         TS_ASSERT_EQUALS(ReferenceCounted_tester::count(), 1);
      }
      TS_ASSERT_EQUALS(ReferenceCounted_tester::count(), 0);
   }


   void test_accessors()
   {
      utilib::ReferenceCounted<int> i;
      utilib::ReferenceCounted<int> j;

      *i = 10;
      j = i;
      TS_ASSERT_EQUALS(*j, 10);

      utilib::ReferenceCounted<std::list<int> > v;
      TS_ASSERT(v->empty());
      (*v).push_back(5);
      TS_ASSERT_EQUALS(v->front(), 5);
   }
};

} // namespace utilib
