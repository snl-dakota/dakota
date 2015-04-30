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

#include <utilib/Any.h>

namespace utilib {

namespace {

class FOO
{
public:
   FOO()
   { }
   FOO(const FOO &)
   { }
};

class NotCopyConstructable
{
public:
   NotCopyConstructable()
   { }
   NotCopyConstructable(const NotCopyConstructable &)
   {
      EXCEPTION_MNGR(std::runtime_error, 
                     "This class is not copy-constructable.");
   }
};

class State
{
public:
   State()
      : stack(utilib::exception_mngr::stack_trace())
   {
      utilib::exception_mngr::set_stack_trace(false);
   }

   ~State()
   {
      utilib::exception_mngr::set_stack_trace(stack);
   }

private:
   bool stack;
};

} // namespace utilib::(local)


namespace unittest { class Test_Any; }

class utilib::unittest::Test_Any : public CxxTest::TestSuite
{
public:

   /// empty any
   void test_default_constructor()
   {
      utilib::Any a;
      TS_ASSERT(a.empty());
      TS_ASSERT(!a.is_reference());
      TS_ASSERT(!a.is_immutable());
   }

   /// value any from explicit constructor
   void test_explicit_constructor1()
   {
      int i=3;
      utilib::Any a(i);
      TS_ASSERT(!a.empty());
      TS_ASSERT(!a.is_reference());
      TS_ASSERT(!a.is_immutable());

      TS_ASSERT_EQUALS(a, 3);
      i = 2;
      TS_ASSERT_EQUALS(a, 3);
      a = 5;
      TS_ASSERT_EQUALS(a, 5);
      TS_ASSERT_EQUALS(i, 2);
   }

   /// reference any from explicit constructor
   void test_explicit_constructor2()
   {
      int i=3;
      utilib::Any a(i,true);
      TS_ASSERT(!a.empty());
      TS_ASSERT(a.is_reference());
      TS_ASSERT(!a.is_immutable());

      TS_ASSERT_EQUALS(a, 3);
      i = 2;
      TS_ASSERT_EQUALS(a, 2);
      a = 5;
      TS_ASSERT_EQUALS(a, 5);
      TS_ASSERT_EQUALS(i, 2);
   }

   /// immutable value any from explicit constructor
   void test_explicit_constructor3()
   {
      int i=3;
      utilib::Any a(i,false,true);
      TS_ASSERT(!a.empty());
      TS_ASSERT(!a.is_reference());
      TS_ASSERT(a.is_immutable());

      TS_ASSERT_EQUALS(a, 3);
      i = 2;
      TS_ASSERT_EQUALS(a, 3);
      a = 5;
      TS_ASSERT_EQUALS(a, 5);
      TS_ASSERT_EQUALS(i, 2);
   }

   /// immutable reference any from explicit constructor
   void test_explicit_constructor4()
   {
      int i=3;
      utilib::Any a(i,true,true);
      TS_ASSERT(!a.empty());
      TS_ASSERT(a.is_reference());
      TS_ASSERT(a.is_immutable());

      TS_ASSERT_EQUALS(a, 3);
      i = 2;
      TS_ASSERT_EQUALS(a, 2);
      a = 5;
      TS_ASSERT_EQUALS(a, 5);
      TS_ASSERT_EQUALS(i, 5);
   }

   /// explicit construction from a reference any
   void test_explicit_constructor5()
   {
      int i = 3;
      utilib::Any a(i,true);
      TS_ASSERT_EQUALS(a, 3);
      utilib::Any b(a);
      TS_ASSERT(!b.empty());
      TS_ASSERT(b.is_reference());
      TS_ASSERT(!b.is_immutable());

      TS_ASSERT_EQUALS(b.anyCount(), size_t(1));
      TS_ASSERT_EQUALS(b, 3);
      i = 4;
      TS_ASSERT_EQUALS(a, 4);
      TS_ASSERT_EQUALS(b, 4);
      a = 5;
      TS_ASSERT_EQUALS(i, 4);
      TS_ASSERT_EQUALS(b, 4);
      TS_ASSERT_EQUALS(b.anyCount(), size_t(0));
   }

   /// explicit construction from an immutable reference any
   void test_explicit_constructor6()
   {
      int i = 3;
      utilib::Any a(i,true,true);
      TS_ASSERT_EQUALS(a, 3);
      utilib::Any b(a);
      TS_ASSERT(!b.empty());
      TS_ASSERT(b.is_reference());
      TS_ASSERT(b.is_immutable());

      TS_ASSERT_EQUALS(b.anyCount(), size_t(1));
      TS_ASSERT_EQUALS(b, 3);

      i = 4;
      TS_ASSERT_EQUALS(a, 4);
      TS_ASSERT_EQUALS(b, 4);
      a = 5;
      TS_ASSERT_EQUALS(i, 5);
      TS_ASSERT_EQUALS(b, 5);
      TS_ASSERT_EQUALS(b.anyCount(), size_t(1));
   }

   /// copy construction from a value
   void test_copy_constructor1()
   {
      int i = 3;
      utilib::Any a = i;
      TS_ASSERT(!a.empty());
      TS_ASSERT(!a.is_reference());
      TS_ASSERT(!a.is_immutable());

      TS_ASSERT_EQUALS(a, 3);
      i = 2;
      TS_ASSERT_EQUALS(a, 3);
      a = 5;
      TS_ASSERT_EQUALS(a, 5);
      TS_ASSERT_EQUALS(i, 2);
   }

   /// copy construction from an any
   void test_copy_constructor2()
   {
      // Copy an empty any...
      utilib::Any a;
      utilib::Any b = a;
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(b.anyCount(), size_t(0));

      // Copy a full any...
      a = 1;
      utilib::Any c = a;
      TS_ASSERT_EQUALS(a.anyCount(), size_t(1));
      TS_ASSERT_EQUALS(b.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(c.anyCount(), size_t(1));
   }

   /// copy construction from a reference any
   void test_copy_constructor3()
   {
      // Copy an empty any ref...
      utilib::AnyRef a;
      utilib::Any b = a;
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(b.anyCount(), size_t(0));

      // Copy a full any ref...
      int i = 3;
      a = i;
      TS_ASSERT_EQUALS(a, 3);
      utilib::Any c = a;
      TS_ASSERT(!c.empty());
      TS_ASSERT(c.is_reference());
      TS_ASSERT(!c.is_immutable());

      TS_ASSERT_EQUALS(c.anyCount(), size_t(1));
      TS_ASSERT_EQUALS(c, 3);
      i = 4;
      TS_ASSERT_EQUALS(a, 4);
      TS_ASSERT_EQUALS(c, 4);
      a = 5;
      TS_ASSERT_EQUALS(i, 4);
      TS_ASSERT_EQUALS(c, 4);
      TS_ASSERT_EQUALS(c.anyCount(), size_t(0));
   }

   /// copy construction from an immutable reference any
   void test_copy_constructor4()
   {
      // Copy an empty any fixed ref...
      utilib::AnyFixedRef a;
      utilib::Any b = a;
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(b.anyCount(), size_t(0));

      // Copy a full any ref...
      int i = 3;
      a = i;
      TS_ASSERT_EQUALS(a, 3);
      utilib::Any c = a;
      TS_ASSERT(!c.empty());
      TS_ASSERT(c.is_reference());
      TS_ASSERT(c.is_immutable());

      TS_ASSERT_EQUALS(c.anyCount(), size_t(1));
      TS_ASSERT_EQUALS(c, 3);

      i = 4;
      TS_ASSERT_EQUALS(a, 4);
      TS_ASSERT_EQUALS(c, 4);
      a = 5;
      TS_ASSERT_EQUALS(i, 5);
      TS_ASSERT_EQUALS(c, 5);
      TS_ASSERT_EQUALS(c.anyCount(), size_t(1));
   }

   /// test operator=(self)
   void test_operatorEQ1()
   {
      int i = 5;
      utilib::Any a = i;
      a = a;
      TS_ASSERT(!a.empty());
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(a, 5);
   }


   /// test operator=() to normal any
   void test_operatorEQ2()
   {
      utilib::Any a = 1;
      utilib::Any b = 2;
      utilib::Any c = 3;

      a = b;
      TS_ASSERT(!a.empty());
      TS_ASSERT_EQUALS(a.anyCount(), size_t(1));
      TS_ASSERT_EQUALS(a, 2);

      b = c;
      TS_ASSERT(!a.empty());
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(a, 2);
      TS_ASSERT(!b.empty());
      TS_ASSERT_EQUALS(b.anyCount(), size_t(1));
      TS_ASSERT_EQUALS(b, 3);
   }

   /// test operator=() to immutable value any
   void test_operatorEQ3()
   {
      utilib::Any a = 1;
      utilib::Any b(2, false, true);
      utilib::Any c = 3;

      a = b;
      TS_ASSERT(!a.empty());
      TS_ASSERT(a.is_immutable());
      TS_ASSERT_EQUALS(a.anyCount(), size_t(1));
      TS_ASSERT_EQUALS(a, 2);

      b = c;
      TS_ASSERT_EQUALS(a, 3);
      TS_ASSERT_EQUALS(a.anyCount(), size_t(1));
      TS_ASSERT_EQUALS(b.anyCount(), size_t(1));
      TS_ASSERT_EQUALS(c.anyCount(), size_t(0));
   }


   /// test operator=() to immutable value anys
   void test_operatorEQ4()
   {
      State state;

      utilib::Any a(1, false, true);
      utilib::Any b;
      
      // empty source yields exception
      TS_ASSERT_THROWS_ASSERT
         ( a = b, utilib::bad_any_typeid &e,
           TEST_WHAT(e, "Any::operator=(): assignment to immutable Any "
                     "from invalid type.") );

      // invalid type yields exception
      b = 3.3;
      TS_ASSERT_THROWS_ASSERT
         ( a = b, utilib::bad_any_typeid &e,
           TEST_WHAT(e, "Any::operator=(): assignment to immutable Any "
                     "from invalid type.") );
   }

   /// test operator=() for simple data
   void test_operatorEQ5()
   {
      utilib::Any a = 1;

      a = 5;
      TS_ASSERT(!a.empty());
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(a, 5);
   }

   /// test operator=() for immutable reference data
   void test_operatorEQ6()
   {
      int i = 1;
      utilib::Any a(i, true, true);

      a = 5;
      TS_ASSERT(!a.empty());
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(a, 5);
   }

   /// Tests written by Bill...
   void test_operatorEQ()
   {
      utilib::Any a;
      utilib::Any b;
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(b.anyCount(), size_t(0));
      a=1;
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(b.anyCount(), size_t(0));
      a=b;
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(b.anyCount(), size_t(0));
      b=1;
      a=b;
      TS_ASSERT_EQUALS(a.anyCount(), size_t(1));
      TS_ASSERT_EQUALS(b.anyCount(), size_t(1));
      a=1;
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(b.anyCount(), size_t(0));

      int i=3;
      utilib::Any c(i,true);
      a=c;
      TS_ASSERT_EQUALS(a.anyCount(), size_t(1));
      TS_ASSERT_EQUALS(c.anyCount(), size_t(1));
      a=1;
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(c.anyCount(), size_t(0));

      i=4;
      utilib::Any d(i,true,true);
      a=d;
      TS_ASSERT_EQUALS(a.anyCount(), size_t(1));
      TS_ASSERT_EQUALS(d.anyCount(), size_t(1));
      a=1;
      TS_ASSERT_EQUALS(a.anyCount(), size_t(1));
      TS_ASSERT_EQUALS(d.anyCount(), size_t(1));
   }


   /// test operator&=() for simple data
   void test_operatorAndEQ1()
   {
      int i = 1;
      utilib::Any a;

      a &= i;
      TS_ASSERT(!a.empty());
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(a, 1);
      i = 2;
      TS_ASSERT_EQUALS(a, 2);
   }

   /// test operator&=() for immutable Anys
   void test_operatorAndEQ2()
   {
      int i = 1;
      int j = 2;
      utilib::Any a(i, true, true);

      TS_ASSERT_THROWS_ASSERT
         ( a &= j, utilib::bad_any_typeid &e,
           TEST_WHAT(e, "Any::set(value): assigning reference to an "
                     "immutable Any.") );
   }


   /// basic use of set: set an any to point to a value
   void test_set1()
   {
      utilib::Any a = 5;
      
      a.set(5);
      TS_ASSERT_EQUALS(a, 5);
   }


   /// set any to be a reference to another value
   void test_set2()
   {
      utilib::Any a;
      int i = 1;
      
      a.set(i, true);
      TS_ASSERT_EQUALS(a, 1);
      i = 2;
      TS_ASSERT_EQUALS(a, 2);
   }


   /// set any to be an immutable reference to another value
   void test_set3()
   {
      utilib::Any a = 5;
      int i = 1;
      
      a.set(i, true, true);
      TS_ASSERT_EQUALS(a, 1);
      i = 2;
      TS_ASSERT_EQUALS(a, 2);
      a = 5;
      TS_ASSERT_EQUALS(i, 5);
      TS_ASSERT_EQUALS(a, 5);
   }


   /// we disallow setting the immutability flag on an already immutable Any
   void test_set4()
   {
      utilib::Any a(1, false, true);
      
      TS_ASSERT_THROWS_ASSERT
         ( a.set(5, true, true), utilib::bad_any_typeid &e,
           TEST_WHAT(e, "Any::set(value): assigning immutable to an "
                     "already immutable Any.") );
   }

   /// you cannot set an immutable Any to be a reference to an external value
   void test_set5()
   {
      utilib::Any a(1, false, true);
      
      TS_ASSERT_THROWS_ASSERT
         ( a.set(5, true, false), utilib::bad_any_typeid &e,
           TEST_WHAT(e, "Any::set(value): assigning reference to an "
                     "immutable Any.") );
   }


   /// simple test for setting an invalid type to an immutable Any
   void test_set6()
   {
      utilib::Any a(1, false, true);
      
      TS_ASSERT_THROWS_ASSERT
         ( a.set(5.5), utilib::bad_any_typeid &e,
           TEST_WHAT(e, "Any::set(value): assignment to immutable Any "
                     "from invalid type.") );
   }

   /// Set an Any without using the Copy Constructor
   void test_simple_set1()
   {
      utilib::Any a;
      
      a.set<int>() = 5;
      TS_ASSERT(!a.empty());
      TS_ASSERT_EQUALS(a, 5);
   }
   

   /// Set an Any without using the Copy Constructor
   void test_simple_set2()
   {
      utilib::Any a;
      
      a.set<NotCopyConstructable>();
      TS_ASSERT(!a.empty());
   }


   /// Set an immutable Any without using the Copy Constructor
   void test_simple_set3()
   {
      int i = 5;
      utilib::Any a = i;
      
      TS_ASSERT_EQUALS(a, 5);
      a.set<int>();
      TS_ASSERT(!a.empty());
      TS_ASSERT_EQUALS(a, 0);
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
   }


   /// Set an immutable Any without using the Copy Constructor
   void test_simple_set4()
   {
      int i = 5;
      utilib::Any a(i, true, true);
      
      TS_ASSERT_EQUALS(a, 5);
      a.set<int>();
      TS_ASSERT(!a.empty());
      TS_ASSERT_EQUALS(a, 0);
      TS_ASSERT_EQUALS(i, 0);
   }


   /// Set an immutable Any without the Copy Constructor to an invalid type
   void test_simple_set5()
   {
      int i = 5;
      utilib::Any a(i, true, true);
      
      TS_ASSERT_EQUALS(a, 5);
      TS_ASSERT_THROWS_ASSERT
         ( a.set<double>(), utilib::bad_any_typeid &e,
           TEST_WHAT(e, "Any::set<>(): assignment to immutable Any "
                     "from invalid type.") );
   }

   /// Expose the value of an Any
   void test_expose1()
   {
      int i = 1;
      utilib::Any a(i, true, true);

      TS_ASSERT_EQUALS(a.expose<int>(), 1);
   }


   /// Expose the value of an empty Any
   void test_expose2()
   {
      State state;
      utilib::Any a;

      TS_ASSERT_THROWS_ASSERT(a.expose<int>(), utilib::bad_any_cast &e,
                              TEST_WHAT(e, "Any::expose() - NULL data"));
   }

   /// Expose the value of an any of the wrong type
   void test_expose3()
   {
      utilib::Any a = 1;

      TS_ASSERT_THROWS_ASSERT
         ( a.expose<double>(), utilib::bad_any_cast &e,
           TEST_WHAT(e, "Any::expose() - failed conversion from '") );
   }


   /// Expose the value of an any as a non-const
   void test_expose4()
   {
      int i = 1;
      utilib::Any a(i, true, true);

      a.expose<int>() = 5;
      TS_ASSERT_EQUALS(i, 5);
   }


   /// Extract the value of an Any
   void test_extract1()
   {
      int i = 1;
      utilib::Any a = i;

      int j = 0;
      a.extract(j);
      TS_ASSERT_EQUALS(j, i);
   }


   /// Extract the value of an AnyRef
   void test_extract2()
   {
      int i = 1;
      utilib::AnyRef a = i;

      int j = 0;
      a.extract(j);
      TS_ASSERT_EQUALS(j, i);
   }


   /// Extract the value of an empty Any
   void test_extract3()
   {
      TS_ASSERT(utilib::Any::throwCastExceptions());

      utilib::Any a;
      int j;

      TS_ASSERT_THROWS_ASSERT(a.extract(j), utilib::bad_any_cast &e,
                              TEST_WHAT(e, "Any::extract() - NULL data"));
   }

   /// Extract the value of an any of the wrong type
   void test_extract4()
   {
      TS_ASSERT(utilib::Any::throwCastExceptions());
      utilib::Any a = 1;
      double d;

      TS_ASSERT_THROWS_ASSERT
         ( a.extract(d), utilib::bad_any_cast &e,
           TEST_WHAT(e, "Any::extract() - failed conversion from '") );
   }


   /// Extract the value of an empty Any
   void test_extract5()
   {
      bool exceptions = utilib::Any::throwCastExceptions();
      utilib::Any::throwCastExceptions() = false;

      utilib::Any a;
      int j;

      try {
         TS_ASSERT_EQUALS(a.extract(j), utilib::error::Any::Empty);
      } catch ( ... ) {
         TS_FAIL("Caught unexpected exception"); 
      }
      utilib::Any::throwCastExceptions() = exceptions;
   }

   /// Extract the value of an any of the wrong type
   void test_extract6()
   {
      bool exceptions = utilib::Any::throwCastExceptions();
      utilib::Any::throwCastExceptions() = false;

      utilib::Any a = 1;
      double d;

      try {
         TS_ASSERT_EQUALS(a.extract(d), utilib::error::Any::IncompatibleType);
      } catch ( ... ) {
         TS_FAIL("Caught unexpected exception"); 
      }
      utilib::Any::throwCastExceptions() = exceptions;
   }


   /// is_type()
   void test_is_type1()
   {
      int i = 0;
      utilib::Any a = i;
      TS_ASSERT(a.is_type(typeid(int)));
      TS_ASSERT(!a.is_type(typeid(double)));
   }


   /// is_type()
   void test_is_type2()
   {
      int i = 0;
      utilib::Any a = i;
      TS_ASSERT(a.is_type<int>());
      TS_ASSERT(!a.is_type<double>());
   }


   /// is_type() for empty anys should always be false
   void test_is_type3()
   {
      utilib::Any a;
      TS_ASSERT(!a.is_type(typeid(int)));
      TS_ASSERT(!a.is_type(typeid(double)));
   }


   /// test the operator==()
   void test_operatorEqEq1()
   {
      utilib::Any a = 1;
      utilib::Any b = 2.5;

      TS_ASSERT_DIFFERS(a, b);
      b = 2;
      TS_ASSERT_DIFFERS(a, b);
      b = 1;
      TS_ASSERT_EQUALS(a, b);
      TS_ASSERT(!a.references_same_data_as(b));
      b = a;
      TS_ASSERT_EQUALS(a, b);
      TS_ASSERT(a.references_same_data_as(b));
      b.clear();
      TS_ASSERT(b.empty());
      TS_ASSERT_DIFFERS(a, b);
      TS_ASSERT_DIFFERS(b, a);
      a.clear();
      TS_ASSERT(a.empty());
      TS_ASSERT_EQUALS(a, b);
      TS_ASSERT(!a.references_same_data_as(b));
   }

   /// test the operator==() for special comparators
   void test_operatorEqEq2()
   {
      std::pair<int,int> p;
      utilib::AnyRef a = p;
      utilib::Any b = std::pair<int,int>(5,6);

      TS_ASSERT_DIFFERS(a, b);
      p.first = 5;
      TS_ASSERT_DIFFERS(a, b);
      p.second = 6;
      TS_ASSERT_EQUALS(a, b);

      std::vector<int> v;
      v.push_back(1);
      v.push_back(2);
      v.push_back(3);
      a = v;
      b = v;
      TS_ASSERT_EQUALS(a, b);
      v.push_back(4);
      TS_ASSERT_DIFFERS(a, b);
      v.pop_back();
      TS_ASSERT_EQUALS(a, b);
      v.pop_back();
      TS_ASSERT_DIFFERS(a, b);
      v[1] = 4;
      TS_ASSERT_DIFFERS(a, b);

      v.clear();
      TS_ASSERT_DIFFERS(a, b);
      b = v;
      TS_ASSERT_EQUALS(a, b);
      v.push_back(1);
      TS_ASSERT_DIFFERS(a, b);
   }

   /// test the operator==() for non-comparable classes
   void test_operatorEqEq3()
   {
      utilib::Any a = FOO();
      utilib::Any b = FOO();

      TS_ASSERT_THROWS(a == b, utilib::any_not_comparable);
   }

   /// test the operator<()
   void test_operatorLt1()
   {
      utilib::Any a = 1;
      utilib::Any b = 2;

      TS_ASSERT(a < b);
      TS_ASSERT(!(b < a));

      b = 1;
      TS_ASSERT(!(a < b));
      TS_ASSERT(!(b < a));

      b = a;
      TS_ASSERT(!(a < b));
      TS_ASSERT(!(b < a));

      b.clear();
      TS_ASSERT(!(a < b));
      TS_ASSERT((b < a));
   }

   /// test the operator<() for special comparators
   void test_operatorLt2()
   {
      std::pair<int,int> p;
      utilib::AnyRef a = p;
      utilib::Any b = std::pair<int,int>(5,6);

      TS_ASSERT(a < b);
      TS_ASSERT(!(b < a));
      p.first = 5;
      TS_ASSERT(a < b);
      TS_ASSERT(!(b < a));
      p.second = 6;
      TS_ASSERT(!(a < b));
      TS_ASSERT(!(b < a));
      p.second = 7;
      TS_ASSERT(!(a < b));
      TS_ASSERT(b < a);
      p.first = 5;
      TS_ASSERT(!(a < b));
      TS_ASSERT(b < a);

      std::vector<int> c;
      c.push_back(1);
      c.push_back(2);
      c.push_back(3);
      a = c;
      b = c;
      TS_ASSERT(!(a < b));
      TS_ASSERT(!(b < a));
      c.push_back(4);
      TS_ASSERT(!(a < b));
      TS_ASSERT(b < a);
      c.pop_back();
      TS_ASSERT(!(a < b));
      TS_ASSERT(!(b < a));
      c.pop_back();
      TS_ASSERT(a < b);
      TS_ASSERT(!(b < a));
      c[1] = 4;
      TS_ASSERT(!(a < b));
      TS_ASSERT(b < a);
   }

   /// test the operator<() with different types
   void test_operatorLt3()
   {
      utilib::Any a = 1;
      utilib::Any b = 2.5;

      // int and double should have an ordering, but we don't know what it is
      TS_ASSERT((a < b) ^ (b < a));

      int test1 = a < b ? 1 : ( b < a ? 2 : 0 );

      // but it should be consistent
      b.set<int>();
      a.set<double>();
      int test2 = b < a ? 1 : ( a < b ? 2 : 0 );
      TS_ASSERT_EQUALS(test1, test2);
   }

   /// test the operator<() for non-comparable classes
   void test_operatorLt4()
   {
      utilib::Any a = FOO();
      utilib::Any b = FOO();

      TS_ASSERT_THROWS(a < b, utilib::any_not_comparable);
   }

   // test the type()
   void test_type1()
   {
      utilib::Any a = 1;
      TS_ASSERT(a.type() == typeid(int));
      TS_ASSERT(a.type() != typeid(double));
      TS_ASSERT(a.type() != typeid(void));
      TS_ASSERT(a.is_type(a.type()));
   }

   // test the type() for empty anys
   void test_type2()
   {
      utilib::Any a;
      TS_ASSERT(a.type() != typeid(int));
      TS_ASSERT(a.type() != typeid(double));
      TS_ASSERT(a.type() == typeid(void));
      TS_ASSERT(a.is_type(a.type()));
   }

   // test if clone() works
   void test_clone()
   {
      int i = 5;
      utilib::Any a = i;
      utilib::Any b = a.clone();
      TS_ASSERT(!b.empty());
      TS_ASSERT(!a.anyCount());
      TS_ASSERT(!b.anyCount());
      TS_ASSERT(!b.references_same_data_as(a));
      TS_ASSERT(a.is_type(typeid(int)));
      TS_ASSERT_EQUALS(b, 5);
   }


   // test AnyRefs
   void test_anyRef1()
   {
      int i = 1;
      utilib::AnyRef a(i);

      TS_ASSERT(!a.empty());
      TS_ASSERT(a.is_reference());
      TS_ASSERT(!a.is_immutable());

      TS_ASSERT_EQUALS(a, 1);
      i = 2;
      TS_ASSERT_EQUALS(a, 2);

      // This is dangerous, but logically correct
      a = 5;
      TS_ASSERT(!a.empty());
      TS_ASSERT(a.is_reference());
      TS_ASSERT(!a.is_immutable());
      TS_ASSERT_EQUALS(a, 5);
   }

   // test AnyRefs with anys
   void test_anyRef2()
   {
      int i = 1;
      utilib::Any a(i);
      utilib::AnyRef b(a);

      TS_ASSERT_EQUALS(a, 1);
      TS_ASSERT_EQUALS(b, 1);
      i = 2;
      TS_ASSERT_EQUALS(a, 1);
      TS_ASSERT_EQUALS(b, 1);
      a.expose<int>() = 3;
      TS_ASSERT_EQUALS(i, 2);
      TS_ASSERT_EQUALS(a, 3);
      TS_ASSERT_EQUALS(b, 3);
   }

   // test AnyRef::opertor=()
   void test_anyRef3()
   {
      utilib::Any a(1);
      utilib::Any b(2);
      utilib::AnyRef c(b);

      TS_ASSERT_EQUALS(a, 1);
      TS_ASSERT_EQUALS(b, 2);
      TS_ASSERT_EQUALS(c, 2);
      // NB: c is a "reference" to the data in "a" (since it's a
      // reference-counted object), but NOT a reference itself.
      TS_ASSERT(!c.is_reference());

      c = a;
      TS_ASSERT_EQUALS(a, 1);
      TS_ASSERT_EQUALS(b, 2);
      TS_ASSERT_EQUALS(c, 1);
      // NB: c is a "reference" to the data in "a" (since it's a
      // reference-counted object), but NOT a reference itself.
      TS_ASSERT(!c.is_reference());

      int i = 5;
      c = i;
      TS_ASSERT_EQUALS(a, 1);
      TS_ASSERT_EQUALS(b, 2);
      TS_ASSERT_EQUALS(c, 5);
      TS_ASSERT(c.is_reference());
   }

   // test AnyFixedRefs
   void test_anyFixedRef1()
   {
      int i = 1;
      utilib::AnyFixedRef a(i);

      TS_ASSERT(!a.empty());
      TS_ASSERT(a.is_reference());
      TS_ASSERT(a.is_immutable());

      TS_ASSERT_EQUALS(a, 1);
      i = 2;
      TS_ASSERT_EQUALS(a, 2);
      a = 5;
      TS_ASSERT_EQUALS(a, 5);
      TS_ASSERT_EQUALS(i, 5);
   }

   // test AnyFixedRefs with anys
   void test_anyFixedRef2()
   {
      int i = 1;
      utilib::Any a(i);
      utilib::AnyFixedRef b(a);

      TS_ASSERT_EQUALS(a, 1);
      TS_ASSERT_EQUALS(b, 1);
      i = 2;
      TS_ASSERT_EQUALS(a, 1);
      TS_ASSERT_EQUALS(b, 1);
      a.expose<int>() = 3;
      TS_ASSERT_EQUALS(i, 2);
      TS_ASSERT_EQUALS(a, 3);
      TS_ASSERT_EQUALS(b, 3);
      b = 4;
      TS_ASSERT_EQUALS(i, 2);
      TS_ASSERT_EQUALS(a, 4);
      TS_ASSERT_EQUALS(b, 4);
   }


   // test AnyFixedRef construction from an empty Any
   void test_anyFixedRef3()
   {
      utilib::Any a;
      utilib::AnyFixedRef b = a;
      TS_ASSERT(b.empty());
      TS_ASSERT_EQUALS(b.anyCount(), size_t(0));

      utilib::AnyRef c;
      utilib::AnyFixedRef d = c;
      TS_ASSERT(d.empty());
      TS_ASSERT_EQUALS(d.anyCount(), size_t(0));
   }


   // test AnyFixedRef construction from an immutable Any
   void test_anyFixedRef4()
   {
      utilib::Any a(1,false,true);
      utilib::AnyFixedRef b = a;
      TS_ASSERT(!b.empty());
      TS_ASSERT_EQUALS(b.anyCount(), size_t(1));
      TS_ASSERT_EQUALS(b, 1);

      int i = 1;
      utilib::AnyRef c(i, true);
      utilib::AnyFixedRef d = c;
      TS_ASSERT(!d.empty());
      TS_ASSERT_EQUALS(d.anyCount(), size_t(1));
      TS_ASSERT_EQUALS(d, 1);
   }


   // test AnyFixedRef::opertor=() [basic usage]
   void test_anyFixedRef5()
   {
      int i = 1;
      utilib::AnyRef a(i);
      utilib::Any b(2);
      utilib::AnyFixedRef c(a);

      TS_ASSERT_EQUALS(a, 1);
      TS_ASSERT_EQUALS(b, 2);
      TS_ASSERT_EQUALS(c, 1);
      // NB: c is a "fixed reference" to the data in "a"
      TS_ASSERT(!c.empty());
      TS_ASSERT(c.is_reference());
      TS_ASSERT(c.is_immutable());
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(b.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(c.anyCount(), size_t(0));

      c = 5;
      TS_ASSERT(!c.empty());
      TS_ASSERT(c.is_reference());
      TS_ASSERT(c.is_immutable());
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(b.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(c.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(c, 5);
      TS_ASSERT_EQUALS(a, 5);
      TS_ASSERT_EQUALS(i, 5);

      c = b;
      TS_ASSERT_EQUALS(a, 2);
      TS_ASSERT_EQUALS(b, 2);
      TS_ASSERT_EQUALS(c, 2);
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(b.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(c.anyCount(), size_t(0));
      TS_ASSERT(!c.empty());
      TS_ASSERT(c.is_reference());
      TS_ASSERT(c.is_immutable());
   }


   // test AnyFixedRef::opertor=() [advanced usage, part 1]
   void test_anyFixedRef6()
   {
      int i = 1;
      int j = 0;
      utilib::AnyRef a(i);
      utilib::Any b(2);
      utilib::AnyFixedRef c;

      c = j;

      TS_ASSERT_EQUALS(a, 1);
      TS_ASSERT_EQUALS(b, 2);
      TS_ASSERT_EQUALS(c, 0);
      // NB: c is a "fixed reference" to j
      TS_ASSERT(!c.empty());
      TS_ASSERT(c.is_reference());
      TS_ASSERT(c.is_immutable());
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(b.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(c.anyCount(), size_t(0));

      c = 5;
      TS_ASSERT(!c.empty());
      TS_ASSERT(c.is_reference());
      TS_ASSERT(c.is_immutable());
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(b.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(c.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(a, 1);
      TS_ASSERT_EQUALS(b, 2);
      TS_ASSERT_EQUALS(c, 5);
      TS_ASSERT_EQUALS(j, 5);

      // assignments to an initialized FixedRef are deep copies
      c = b;
      TS_ASSERT_EQUALS(a, 1);
      TS_ASSERT_EQUALS(b, 2);
      TS_ASSERT_EQUALS(c, 2);
      TS_ASSERT_EQUALS(j, 2);
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(b.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(c.anyCount(), size_t(0));
      TS_ASSERT(!c.empty());
      TS_ASSERT(c.is_reference());
      TS_ASSERT(c.is_immutable());

      c = a;
      TS_ASSERT_EQUALS(a, 1);
      TS_ASSERT_EQUALS(b, 2);
      TS_ASSERT_EQUALS(c, 1);
      TS_ASSERT_EQUALS(i, 1);
      TS_ASSERT_EQUALS(j, 1);
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(b.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(c.anyCount(), size_t(0));
      TS_ASSERT(!c.empty());
      TS_ASSERT(c.is_reference());
      TS_ASSERT(c.is_immutable());

      i = 10;
      TS_ASSERT_EQUALS(a, 10);
      TS_ASSERT_EQUALS(b, 2);
      TS_ASSERT_EQUALS(c, 1);
      TS_ASSERT_EQUALS(j, 1);
      TS_ASSERT_EQUALS(i, 10);

      // assignments to an EMPTY FixedRef create new FixedRefs
      c.clear();
      c = a;
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(b.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(c.anyCount(), size_t(0));
      TS_ASSERT(!c.empty());
      TS_ASSERT(c.is_reference());
      TS_ASSERT(c.is_immutable());
      TS_ASSERT_EQUALS(a, 10);
      TS_ASSERT_EQUALS(b, 2);
      TS_ASSERT_EQUALS(c, 10);
      TS_ASSERT_EQUALS(i, 10);
      TS_ASSERT_EQUALS(j, 1);

      c = 20;
      TS_ASSERT_EQUALS(a, 20);
      TS_ASSERT_EQUALS(b, 2);
      TS_ASSERT_EQUALS(c, 20);
      TS_ASSERT_EQUALS(i, 20);
      TS_ASSERT_EQUALS(j, 1);

      // (works for plain 'old anys, too)
      c.clear();
      c = b;
      TS_ASSERT_EQUALS(a.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(b.anyCount(), size_t(0));
      TS_ASSERT_EQUALS(c.anyCount(), size_t(0));
      TS_ASSERT(!c.empty());
      TS_ASSERT(c.is_reference());
      TS_ASSERT(c.is_immutable());
      TS_ASSERT_EQUALS(a, 20);
      TS_ASSERT_EQUALS(b, 2);
      TS_ASSERT_EQUALS(c, 2);
      TS_ASSERT_EQUALS(i, 20);
      TS_ASSERT_EQUALS(j, 1);

      c = 5;
      TS_ASSERT_EQUALS(a, 20);
      TS_ASSERT_EQUALS(b, 5);
      TS_ASSERT_EQUALS(c, 5);
      TS_ASSERT_EQUALS(i, 20);
      TS_ASSERT_EQUALS(j, 1);
   }

   // test AnyFixedRef::opertor=() [advanced usage, part 2]
   void test_anyFixedRef7()
   {
      // but if the Any is empty, then nothing can really happen
      utilib::Any a;
      utilib::AnyFixedRef b(a);

      TS_ASSERT(a.empty());
      TS_ASSERT(b.empty());

      // but if the Any is empty, then nothing can really happen
      utilib::AnyRef c;
      b = c;

      TS_ASSERT(c.empty());
      TS_ASSERT(b.empty());
   }

   // test downcasts
   void test_downcast()
   {
      int i = 1;
      int j = 2;
      utilib::AnyRef a = i;
      utilib::AnyFixedRef b = j;

      utilib::Any x = a;
      TS_ASSERT(x.is_reference());
      TS_ASSERT(!x.is_immutable());
      TS_ASSERT_EQUALS(x.anyCount(), size_t(1));
      TS_ASSERT_EQUALS(x, i);

      utilib::Any y = b;
      TS_ASSERT(y.is_reference());
      TS_ASSERT(y.is_immutable());
      TS_ASSERT_EQUALS(y.anyCount(), size_t(1));
      TS_ASSERT_EQUALS(y, j);

      x = b;
      TS_ASSERT(x.is_reference());
      TS_ASSERT(x.is_immutable());
      TS_ASSERT_EQUALS(x.anyCount(), size_t(2));
      TS_ASSERT_EQUALS(x, j);
   }


   // test streaming of Any contents to a stream
   void test_print1()
   {
      utilib::Any a;
      std::ostringstream os;
      os << a;
      
      TS_ASSERT_EQUALS(os.str(), "[Empty Any]");
   }

   // test streaming of scalar Any contents to a stream
   void test_print2()
   {
      utilib::Any a(5);
      std::ostringstream os;
      os << a;
      
      TS_ASSERT_EQUALS(os.str(), "5");
   }

   // test streaming of pair Any contents to a stream
   void test_print3()
   {
      utilib::Any a(std::pair<int,int>(5,6));
      std::ostringstream os;
      os << a;
      
      TS_ASSERT_EQUALS(os.str(), "(5, 6)");
   }

   // test streaming of sequence Any contents to a stream
   void test_print4()
   {
      std::vector<int> v;
      v.push_back(1);
      v.push_back(2);
      v.push_back(3);

      utilib::AnyRef a(v);
      std::ostringstream os;
      os << a;
      TS_ASSERT_EQUALS(os.str(), "[ 1, 2, 3 ]");

      os.str("");
      v.clear();
      os << a;
      TS_ASSERT_EQUALS(os.str(), "[ ]");
   }


   // test streaming of non-printable class
   void test_print5()
   {
      utilib::Any a = FOO();

      std::ostringstream os;
      os << a;

      std::string msg = os.str();
      TS_ASSERT_EQUALS( msg.substr(0, msg.find(",")),
                        "[utilib::Any contains non-printable object" );
   }


   // test streaming into an Any
   void test_read1()
   {
      utilib::Any a;
      std::stringstream ss("5");

      TS_ASSERT_THROWS(ss >> a, utilib::any_not_readable);
   }

   // test streaming into a scalar Any 
   void test_read2()
   {
      utilib::Any a(5);
      std::stringstream ss("55");

      ss >> a;
      TS_ASSERT_EQUALS(a, 55);
   }

   // test streaming into a pair Any 
   void test_read3()
   {
      typedef std::pair<int,int> pair_t;
      pair_t tmp;
      utilib::Any a(tmp);
      std::stringstream ss(" (5,6) ");

      TS_ASSERT_THROWS(ss >> a, utilib::any_not_readable);

      // Should be unreachable (until pair reader is defined)

      //pair_t ans(5,6);
      //TS_ASSERT_EQUALS(a.expose<pair_t>().first, 5);
      //TS_ASSERT_EQUALS(a.expose<pair_t>().second, 6);
      //TS_ASSERT_EQUALS(a, ans);
   }

   // test streaming into a sequence Any 
   void test_read4()
   {
      std::vector<int> v;
      utilib::AnyRef a(v);
      std::stringstream ss("[1, 2, 3]");

      TS_ASSERT_THROWS(ss >> a, utilib::any_not_readable);

      // Should be unreachable (until vector reader is defined)

      //std::vector<int> ans;
      //ans.push_back(1);
      //ans.push_back(2);
      //ans.push_back(3);
      //TS_ASSERT_EQUALS(a.expose<std::vector<int> >().size(), 3);
      //TS_ASSERT_EQUALS(a.expose<std::vector<int> >()[0], 1);
      //TS_ASSERT_EQUALS(a.expose<std::vector<int> >()[1], 2);
      //TS_ASSERT_EQUALS(a.expose<std::vector<int> >()[2], 3);
      //TS_ASSERT_EQUALS(a, ans);
   }

   // test the to to_value_any() method
   void test_to_value_any1()
   {
      int i = 5;
      utilib::AnyRef a = i;

      TS_ASSERT_EQUALS(a, 5);
      ++i;
      TS_ASSERT_EQUALS(a, 6);
      a.to_value_any();
      TS_ASSERT_EQUALS(a, 6);
      ++i;
      TS_ASSERT_EQUALS(a, 6);
   }

   // test the to to_value_any() method
   void test_to_value_any2()
   {
      int i = 5;
      utilib::Any a = i;

      TS_ASSERT_EQUALS(a, 5);
      ++i;
      TS_ASSERT_EQUALS(a, 5);
      a.to_value_any();
      TS_ASSERT_EQUALS(a, 5);
      ++i;
      TS_ASSERT_EQUALS(a, 5);

      utilib::Any b = a;
      TS_ASSERT_EQUALS(a, 5);
      TS_ASSERT_EQUALS(b, 5);
      b.expose<int>()++;
      TS_ASSERT_EQUALS(a, 6);
      a.to_value_any();
      TS_ASSERT_EQUALS(a, 6);
      b.expose<int>()++;
      TS_ASSERT_EQUALS(a, 7);
   }

   // test the to to_value_any() method
   void test_to_value_any3()
   {
      int i = 5;
      utilib::Any a(i,false,true);
      TS_ASSERT(a.is_immutable());

      TS_ASSERT_EQUALS(a, 5);
      ++i;
      TS_ASSERT_EQUALS(a, 5);
      a.to_value_any();
      TS_ASSERT_EQUALS(a, 5);
      ++i;
      TS_ASSERT_EQUALS(a, 5);

      a.set(i,false,true);
      utilib::Any b = a;
      TS_ASSERT(a.is_immutable());
      TS_ASSERT(b.is_immutable());

      TS_ASSERT_EQUALS(a, 7);
      TS_ASSERT_EQUALS(b, 7);
      b.expose<int>()++;
      TS_ASSERT_EQUALS(a, 8);
      a.to_value_any();
      TS_ASSERT(!a.is_immutable());
      TS_ASSERT(b.is_immutable());
      TS_ASSERT_EQUALS(a, 8);
      b.expose<int>()++;
      TS_ASSERT_EQUALS(a, 8);
   }

   // test the describe function
   void test_describe()
   {
      std::string s;

      utilib::Any a;
      s = a.describe();
      TS_ASSERT_EQUALS(s, "(Any): [empty]");

      a = 5;
      s = a.describe();
      std::string ref = "(Any): " + utilib::demangledName(typeid(int));
      TS_ASSERT_EQUALS(s, ref);

      int i = 10;
      utilib::AnyRef b = i;
      s = b.describe("Ref");
      ref = "Ref: " + utilib::demangledName(typeid(int)) + " [reference]";
      TS_ASSERT_EQUALS(s, ref);

      utilib::AnyFixedRef c = i;
      s = c.describe("Fixed");
      ref = "Fixed: " + utilib::demangledName(typeid(int)) + 
         " [immutable] [reference]";
      TS_ASSERT_EQUALS(s, ref);

      a.set(i, false, true);
      s = a.describe();
      ref = "(Any): " + utilib::demangledName(typeid(int)) + " [immutable]";
      TS_ASSERT_EQUALS(s, ref);
   }
};

} // namespace utilib
