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

#include <utilib/TypeManager.h>
#include <utilib/BitArray.h>

#include "CommonTestUtils.h"

namespace {

class State
{
public:
   State( bool _err, bool _warn, bool _exact,
          utilib::Type_Manager* _tm = utilib::TypeManager() )
      : stack(utilib::exception_mngr::stack_trace()),
        tm(_tm),
        err(tm->errorExceptions()),
        warn(tm->warningExceptions()),
        exact(tm->defaultForceExact())
   {
      utilib::exception_mngr::set_stack_trace(false);
      tm->setErrorExceptions(_err);
      tm->setWarningExceptions(_warn);
      tm->setDefaultForceExact(_exact);
   }

   ~State()
   { 
      restore(); 
   }
   
   void restore()
   {
      utilib::exception_mngr::set_stack_trace(stack);
      tm->setErrorExceptions(err);
      tm->setWarningExceptions(warn);
      tm->setDefaultForceExact(exact);
   }

private:
   bool stack;
   utilib::Type_Manager* tm;
   bool err;
   bool warn;
   bool exact;
};

class A
{
public:
   A& operator=(const int& val)
   { value = val; return *this; }

   operator int() const
   { return value; }

private:
   int value;
};

class B
{
public:
   B& operator=(const int& val)
   { value = val; return *this; }

   operator int() const
   { return value; }

private:
   int value;
};

class C {};
class D {};

int cast_A2B(const utilib::Any &src, utilib::Any &dest)
{
   dest.set<B>() = (int)(src.expose<A>());
   return 0;
}

int cast_A2B_exception(const utilib::Any &src, utilib::Any &dest)
{
   dest.set<B>() = 10*(int)(src.expose<A>());
   EXCEPTION_MNGR(std::runtime_error, "An Exception");
   return 0;
}

int cast_A2B_int_exception(const utilib::Any &src, utilib::Any &dest)
{
   dest.set<B>() = 100*(int)(src.expose<A>());
   throw 10;
   return 0;
}

int cast_A2B_error(const utilib::Any &src, utilib::Any &dest)
{
   dest.set<B>() = 1000*(int)(src.expose<A>());
   return -1000;
}

} // namespace (local)


namespace utilib {

namespace unittest { class Test_TypeManager; }
class utilib::unittest::Test_TypeManager : public CxxTest::TestSuite
{
public:
   void test_cast_to_self()
   {
      int i = 1;
      int j = 0;

      // to self
      utilib::TypeManager()->lexical_cast(i,i);
      TS_ASSERT_EQUALS(i, 1);

      // to same type
      utilib::TypeManager()->lexical_cast(i,j);
      TS_ASSERT_EQUALS(i, 1);
      TS_ASSERT_EQUALS(j, 1);
   }

   void test_cast_empty()
   {
      State state(false, false, false);

      int j = 0;
      int ans = utilib::TypeManager()->lexical_cast(utilib::Any(), j);
      TS_ASSERT_EQUALS(ans, utilib::error::TypeManager_EmptySrc);

      utilib::TypeManager()->setErrorExceptions(true);
      TS_ASSERT_THROWS_ASSERT
         ( utilib::TypeManager()->lexical_cast(utilib::Any(), j),
           utilib::bad_lexical_cast &e,
           TEST_WHAT(e, "Type_Manager::lexical_cast(): empty source Any") );

      utilib::Any src;
      utilib::Any dest;
      utilib::TypeManager()->lexical_cast(src, dest);
      TS_ASSERT(dest.empty());
      TS_ASSERT_EQUALS(dest.anyCount(), 0);
   }

   void test_noCastRoute()
   {
      State state(false, false, false);

      int i = 1;
      utilib::Any a;
      int ans = 0;

      // with unknown source type
      a = i;
      ans = utilib::TypeManager()->lexical_cast(A(), a, typeid(int));
      TS_ASSERT_EQUALS(ans, utilib::error::TypeManager_NoCastRoute);
      TS_ASSERT(a.empty());

      // with known source type, but unknown dest
      a = i;
      ans = 0;
      ans = utilib::TypeManager()->lexical_cast(i, a, typeid(A));
      TS_ASSERT_EQUALS(ans, utilib::error::TypeManager_NoCastRoute);
      TS_ASSERT(a.empty());

      // same tests, but throw exceptions
      utilib::TypeManager()->setErrorExceptions(true);

      // with unknown source type
      TS_ASSERT_THROWS_ASSERT
         ( utilib::TypeManager()->lexical_cast(A(), a, typeid(int)),
           utilib::bad_lexical_cast &e,
           TEST_WHAT(e, "Type_Manager::lexical_cast(): no castable route from"));

      // with known source type, but unknown dest
      TS_ASSERT_THROWS_ASSERT
         ( utilib::TypeManager()->lexical_cast(i, a, typeid(A)),
           utilib::bad_lexical_cast &e,
           TEST_WHAT(e, "Type_Manager::lexical_cast(): no castable route from"));
   }


   void test_untyped_any_container()
   {
      UntypedAnyContainer src;
      src.m_data = 5;

      //
      // Untyped to Untyped
      //

      {
         UntypedAnyContainer dest;
         utilib::TypeManager()->lexical_cast(src, dest);
         TS_ASSERT( ! dest.m_data.empty() );
         TS_ASSERT_EQUALS( dest.m_data.expose<int>(), 5 );
         TS_ASSERT_EQUALS( dest.m_data.anyCount(), 1 );
      }

      {
         UntypedAnyContainer dest;
         utilib::TypeManager()->lexical_cast(src, dest, true);
         TS_ASSERT( ! dest.m_data.empty() );
         TS_ASSERT_EQUALS( dest.m_data.expose<int>(), 5 );
         TS_ASSERT_EQUALS( dest.m_data.anyCount(), 1 );
      }

      {
         UntypedAnyContainer dest;
         AnyFixedRef tmp(dest);
         utilib::TypeManager()->lexical_cast(src, tmp);
         TS_ASSERT( ! dest.m_data.empty() );
         TS_ASSERT_EQUALS( dest.m_data.expose<int>(), 5 );
         TS_ASSERT_EQUALS( dest.m_data.anyCount(), 1 );
      }

      {
         Any dest = UntypedAnyContainer();
         utilib::TypeManager()->lexical_cast(src, dest);
         TS_ASSERT( ! dest.empty() );
         TS_ASSERT_EQUALS
            ( dest.expose<UntypedAnyContainer>().m_data.expose<int>(), 5 );
         TS_ASSERT_EQUALS
            ( dest.expose<UntypedAnyContainer>().m_data.anyCount(), 1 );
      }

      //
      // Untyped to Typed
      //

      // no conversion
      {
         int i = 0;
         utilib::TypeManager()->lexical_cast(src, i);
         TS_ASSERT_EQUALS(i, 5);
      }
      // with conversion
      {
         double d = 0;
         utilib::TypeManager()->lexical_cast(src, d);
         TS_ASSERT_EQUALS(d, 5.);
      }

      //
      // Typed to Untyped
      //

      // no conversion
      {
         UntypedAnyContainer dest = src;
         TS_ASSERT_EQUALS( src.m_data.anyCount(), 1 );

         int i = 10;
         utilib::TypeManager()->lexical_cast(i, dest);
         
         TS_ASSERT( ! dest.m_data.empty() );
         TS_ASSERT_EQUALS( dest.m_data.expose<int>(), 10 );
         TS_ASSERT_EQUALS( dest.m_data.anyCount(), 0 );

         TS_ASSERT( ! src.m_data.empty() );
         TS_ASSERT_EQUALS( src.m_data.expose<int>(), 5 );
         TS_ASSERT_EQUALS( src.m_data.anyCount(), 0 );

      }

      // "with" conversion
      {
         UntypedAnyContainer dest = src;
         TS_ASSERT_EQUALS( src.m_data.anyCount(), 1 );

         double d = 10;
         utilib::TypeManager()->lexical_cast(d, dest);
         
         TS_ASSERT( ! dest.m_data.empty() );
         TS_ASSERT_EQUALS( dest.m_data.expose<double>(), 10. );
         TS_ASSERT_EQUALS( dest.m_data.anyCount(), 0 );

         TS_ASSERT( ! src.m_data.empty() );
         TS_ASSERT_EQUALS( src.m_data.expose<int>(), 5 );
         TS_ASSERT_EQUALS( src.m_data.anyCount(), 0 );
      }
   }


   void test_cast_int2double()
   {
      int i = 5;
      double j = 0;

      // the simple way (through template methods)
      utilib::TypeManager()->lexical_cast(i,j);
      TS_ASSERT_EQUALS(i, 5);
      TS_ASSERT_EQUALS(j, 5);

      // the simple way (but with an Any destination)
      utilib::Any a = j;
      utilib::TypeManager()->lexical_cast(4, a);
      TS_ASSERT_EQUALS(j, 5);
      TS_ASSERT_EQUALS(a, 4);

      // the simple way (but with an immutable Any destination)
      a.set(j, true, true);
      utilib::TypeManager()->lexical_cast(3, a);
      TS_ASSERT_EQUALS(j, 3);
      TS_ASSERT_EQUALS(a, 3.);
   }

   void test_cast_double2int()
   {
      State state(false, false, false);

      int i = 0;
      double j = 5;
      int ans = 0;

      ans = utilib::TypeManager()->lexical_cast(j,i);
      TS_ASSERT_EQUALS(i, 5);
      TS_ASSERT_EQUALS(j, 5);
      TS_ASSERT_EQUALS(ans, 0);

      j = 1.75;
      ans = utilib::TypeManager()->lexical_cast(j,i);
      TS_ASSERT_EQUALS(i, 1);
      TS_ASSERT_EQUALS(ans,utilib::Type_Manager::CastWarning::ValueTruncated);

      j = (double)INT_MAX + 1.;
      ans = utilib::TypeManager()->lexical_cast(j,i);
      TS_ASSERT_EQUALS(ans,utilib::Type_Manager::CastWarning::ValueOutOfRange);

      j = (double)INT_MIN - 1.;
      ans = utilib::TypeManager()->lexical_cast(j,i);
      TS_ASSERT_EQUALS(ans,utilib::Type_Manager::CastWarning::ValueOutOfRange);

      j = 6.75;
      utilib::TypeManager()->setWarningExceptions(true);
      TS_ASSERT_THROWS_ASSERT
         ( utilib::TypeManager()->lexical_cast(j,i),
           utilib::bad_lexical_cast &e,
           TEST_WHAT(e, "Type_Manager::lexical_cast(): cast function warning "
                     "during cast from "));
      TS_ASSERT_EQUALS(i, 6);

      j = 7;
      ans = 0;
      utilib::TypeManager()->setDefaultForceExact(true);
      TS_ASSERT_THROWS_NOTHING(ans = utilib::TypeManager()->lexical_cast(j,i));
      TS_ASSERT_EQUALS(i, 6);
      TS_ASSERT_EQUALS(ans, utilib::error::TypeManager_NoExactCastRoute);

      j = 8;
      utilib::TypeManager()->setErrorExceptions(true);
      TS_ASSERT_THROWS_ASSERT
           ( utilib::TypeManager()->lexical_cast(j,i),
             utilib::bad_lexical_cast &e,
             TEST_WHAT(e, "Type_Manager::lexical_cast(): no exact cast route") );
      TS_ASSERT_EQUALS(i, 6);
   }

   void test_cast_signed()
   {
      State state(false, false, false);

      {
         char s = 10;
         unsigned char u = 0;
         unsigned char max = CHAR_MAX;

         TS_ASSERT_EQUALS(utilib::TypeManager()->lexical_cast(s, u), 0);
         TS_ASSERT_EQUALS(s, 10);
         TS_ASSERT_EQUALS(u, 10);

         u = 20;
         TS_ASSERT_EQUALS(utilib::TypeManager()->lexical_cast(u, s), 0);
         TS_ASSERT_EQUALS(s, 20);
         TS_ASSERT_EQUALS(u, 20);
         
         s = max;
         TS_ASSERT_EQUALS(utilib::TypeManager()->lexical_cast(s, u), 0);
         TS_ASSERT_EQUALS(s, max);
         TS_ASSERT_EQUALS(u, max);
         
         s = -1;
         TS_ASSERT_EQUALS( utilib::TypeManager()->lexical_cast(s, u), 
                           utilib::Type_Manager::CastWarning::ValueOutOfRange );
         TS_ASSERT_EQUALS(s, -1);
         TS_ASSERT_EQUALS(u, 0);

         u = max + 1;
         TS_ASSERT_EQUALS( utilib::TypeManager()->lexical_cast(u, s), 
                           utilib::Type_Manager::CastWarning::ValueOutOfRange );
         TS_ASSERT_EQUALS(s, 0);
         TS_ASSERT_EQUALS(u, max+1);
      }

      {
         short s = 10;
         unsigned short u = 0;
         unsigned short max = SHRT_MAX;

         TS_ASSERT_EQUALS(utilib::TypeManager()->lexical_cast(s, u), 0);
         TS_ASSERT_EQUALS(s, 10);
         TS_ASSERT_EQUALS(u, 10);

         u = 20;
         TS_ASSERT_EQUALS(utilib::TypeManager()->lexical_cast(u, s), 0);
         TS_ASSERT_EQUALS(s, 20);
         TS_ASSERT_EQUALS(u, 20);
         
         s = max;
         TS_ASSERT_EQUALS(utilib::TypeManager()->lexical_cast(s, u), 0);
         TS_ASSERT_EQUALS(s, max);
         TS_ASSERT_EQUALS(u, max);
         
         s = -1;
         TS_ASSERT_EQUALS( utilib::TypeManager()->lexical_cast(s, u), 
                           utilib::Type_Manager::CastWarning::ValueOutOfRange );
         TS_ASSERT_EQUALS(s, -1);
         TS_ASSERT_EQUALS(u, 0);

         u = max + 1;
         TS_ASSERT_EQUALS( utilib::TypeManager()->lexical_cast(u, s), 
                           utilib::Type_Manager::CastWarning::ValueOutOfRange );
         TS_ASSERT_EQUALS(s, 0);
         TS_ASSERT_EQUALS(u, max+1);
      }

      {
         int s = 10;
         unsigned int u = 0;
         unsigned int max = INT_MAX;

         TS_ASSERT_EQUALS(utilib::TypeManager()->lexical_cast(s, u), 0);
         TS_ASSERT_EQUALS(s, 10);
         TS_ASSERT_EQUALS(u, 10);

         u = 20;
         TS_ASSERT_EQUALS(utilib::TypeManager()->lexical_cast(u, s), 0);
         TS_ASSERT_EQUALS(s, 20);
         TS_ASSERT_EQUALS(u, 20);
         
         s = max;
         TS_ASSERT_EQUALS(utilib::TypeManager()->lexical_cast(s, u), 0);
         TS_ASSERT_EQUALS(s, max);
         TS_ASSERT_EQUALS(u, max);
         
         s = -1;
         TS_ASSERT_EQUALS( utilib::TypeManager()->lexical_cast(s, u), 
                           utilib::Type_Manager::CastWarning::ValueOutOfRange );
         TS_ASSERT_EQUALS(s, -1);
         TS_ASSERT_EQUALS(u, 0);

         u = max + 1;
         TS_ASSERT_EQUALS( utilib::TypeManager()->lexical_cast(u, s), 
                           utilib::Type_Manager::CastWarning::ValueOutOfRange );
         TS_ASSERT_EQUALS(s, 0);
         TS_ASSERT_EQUALS(u, max+1);
      }

      {
         long s = 10;
         unsigned long u = 0;
         unsigned long max = LONG_MAX;

         TS_ASSERT_EQUALS(utilib::TypeManager()->lexical_cast(s, u), 0);
         TS_ASSERT_EQUALS(s, 10);
         TS_ASSERT_EQUALS(u, 10);

         u = 20;
         TS_ASSERT_EQUALS(utilib::TypeManager()->lexical_cast(u, s), 0);
         TS_ASSERT_EQUALS(s, 20);
         TS_ASSERT_EQUALS(u, 20);
         
         s = max;
         TS_ASSERT_EQUALS(utilib::TypeManager()->lexical_cast(s, u), 0);
         TS_ASSERT_EQUALS(s, max);
         TS_ASSERT_EQUALS(u, max);
         
         s = -1;
         TS_ASSERT_EQUALS( utilib::TypeManager()->lexical_cast(s, u), 
                           utilib::Type_Manager::CastWarning::ValueOutOfRange );
         TS_ASSERT_EQUALS(s, -1);
         TS_ASSERT_EQUALS(u, 0);

         u = max + 1;
         TS_ASSERT_EQUALS( utilib::TypeManager()->lexical_cast(u, s), 
                           utilib::Type_Manager::CastWarning::ValueOutOfRange );
         TS_ASSERT_EQUALS(s, 0);
         TS_ASSERT_EQUALS(u, max+1);
      }

   }

   void test_stl_casts()
   {
      std::vector<int> v;
      std::list<int> l;

      int i = 10;
      utilib::TypeManager()->lexical_cast(i,v);
      TS_ASSERT_EQUALS(v.size(), size_t(1));
      TS_ASSERT_EQUALS(v[0], i);
      utilib::TypeManager()->lexical_cast(i,l);
      TS_ASSERT_EQUALS(l.size(), size_t(1));
      TS_ASSERT_EQUALS(l.front(), i);

      v.push_back(1);
      v.push_back(3);
      v.push_back(2);

      utilib::TypeManager()->lexical_cast(v,l);
      utilib::Any a;
      utilib::TypeManager()->lexical_cast(l, a, typeid(v));
      TS_ASSERT_EQUALS(a, v);

      State state(false, false, false);
      int ans = 0;

      // int -> double -> int for valid ints
      std::vector<double> vd;
      ans = utilib::TypeManager()->lexical_cast(v,vd);
      TS_ASSERT_EQUALS(ans, 0);

      a.clear();
      ans = utilib::TypeManager()->lexical_cast(vd, a, typeid(v));
      TS_ASSERT_EQUALS(ans, 0);
      TS_ASSERT_EQUALS(a, v);

      // double -> int for invalid ints
      vd[1] = (double)INT_MAX + 1.0;
      vd[2] = 0.5;
      ans = utilib::TypeManager()->lexical_cast(vd, v);
      TS_ASSERT_EQUALS(ans,
                       ( utilib::Type_Manager::CastWarning::ValueTruncated +
                         utilib::Type_Manager::CastWarning::ValueOutOfRange ));
      TS_ASSERT_EQUALS(v.size(), size_t(4));

      // Also check < INT_MIN (and simultaneously truncated)
      vd[1] = (double)INT_MIN - 0.1;
      vd[2] = 0;
      ans = utilib::TypeManager()->lexical_cast(vd, v);
      TS_ASSERT_EQUALS(ans,
                       ( utilib::Type_Manager::CastWarning::ValueTruncated +
                         utilib::Type_Manager::CastWarning::ValueOutOfRange ));
      TS_ASSERT_EQUALS(v.size(), size_t(4));
   }

   void test_bool_conversions()
   {
      bool b = true;
      std::vector<bool> vb;
      utilib::TypeManager()->lexical_cast(b, vb);
      TS_ASSERT_EQUALS(vb.size(), 1u);
      TS_ASSERT_EQUALS(vb[0], true);

      b = false;
      utilib::TypeManager()->lexical_cast(b, vb);
      TS_ASSERT_EQUALS(vb.size(), 1u);
      TS_ASSERT_EQUALS(vb[0], false);

      utilib::BitArray ba;
      utilib::TypeManager()->lexical_cast(b, ba);
      TS_ASSERT_EQUALS(ba.size(), 1u);
      TS_ASSERT_EQUALS(ba[0], false);

      b = true;
      utilib::TypeManager()->lexical_cast(b, ba);
      TS_ASSERT_EQUALS(ba.size(), 1u);
      TS_ASSERT_EQUALS(ba[0], true);

      vb.resize(3);
      vb[0] = vb[2] = true;
      vb[1] = false;
      utilib::TypeManager()->lexical_cast(vb, ba);
      TS_ASSERT_EQUALS(ba.size(), 3u);
      TS_ASSERT_EQUALS(ba[0], true);
      TS_ASSERT_EQUALS(ba[1], false);
      TS_ASSERT_EQUALS(ba[2], true);

      ba.flip();
      utilib::TypeManager()->lexical_cast(ba, vb);
      TS_ASSERT_EQUALS(vb.size(), 3u);
      TS_ASSERT_EQUALS(vb[0], false);
      TS_ASSERT_EQUALS(vb[1], true);
      TS_ASSERT_EQUALS(vb[2], false);
   }

   void test_string_casts()
   {
      char c = 'a';
      std::string  s;
      utilib::TypeManager()->lexical_cast(c, s);
      TS_ASSERT_EQUALS(s.size(), size_t(1));
      TS_ASSERT_EQUALS(s[0], c);
   }

   void test_castThrowsError()
   {
      utilib::Type_Manager tm(false);
      tm.register_lexical_cast( typeid(A), typeid(B), &cast_A2B_error );

      // returns error
      tm.setErrorExceptions(false);
      utilib::Any b;
      int ans = tm.lexical_cast(A(), b, typeid(B));
      TS_ASSERT_EQUALS(ans, -1000);
      TS_ASSERT( b.empty() );

      // returns exception
      tm.setErrorExceptions(true);
      TS_ASSERT_THROWS_ASSERT
         ( tm.lexical_cast(A(), b, typeid(B)),
           utilib::bad_lexical_cast &e, 
           TEST_WHAT(e, "Type_Manager::lexical_cast(): cast function error "
                     "during cast from ");
           TS_ASSERT(std::string::npos != std::string(e.what()).find
                     ("Cast function error (-1000) at cast 1: { "));
           TS_ASSERT( b.empty() ) );
   }

   void test_castThrowsException()
   {
      utilib::Type_Manager tm(false);
      utilib::Any b;

      // exception based on std::exception
      tm.register_lexical_cast( typeid(A), typeid(B), &cast_A2B_exception );
      TS_ASSERT_THROWS_ASSERT
         ( tm.lexical_cast(A(), b, typeid(B)),
           utilib::bad_lexical_cast &e,
           TEST_WHAT(e, "Type_Manager::lexical_cast(): cast function error "
                     "during cast from ");
           TS_ASSERT(std::string::npos != std::string(e.what()).find
                     ("Cast function threw exception at cast 1: { "));
           TS_ASSERT( b.empty() ) );

      // unknown exception
      tm.clear_lexical_casts( typeid(A), typeid(B) );
      tm.register_lexical_cast
         ( typeid(A), typeid(B), &cast_A2B_int_exception );
      TS_ASSERT_THROWS_ASSERT
         ( tm.lexical_cast(A(), b, typeid(B)), utilib::bad_lexical_cast &e,
           TEST_WHAT(e, "Type_Manager::lexical_cast(): cast function error "
                     "during cast from ");
           TS_ASSERT(std::string::npos != std::string(e.what()).find
                     ("Cast function threw unknown exception at cast 1: { "));
           TS_ASSERT( b.empty() ) );
   }

   void test_duplicate_tm()
   {
      utilib::TypeManager()->register_lexical_cast
         ( typeid(A), typeid(B), &cast_A2B );

      // inheriting the registrations from the global caster
      utilib::Type_Manager new_tm1(*TypeManager(), false);
      TS_ASSERT(new_tm1.lexical_castable(typeid(A), typeid(B)));

      // ignoring the registrations from the global caster
      utilib::Type_Manager new_tm2;
      TS_ASSERT(!new_tm2.lexical_castable(typeid(A), typeid(B)));

      utilib::TypeManager()->clear_lexical_casts( typeid(A), typeid(B) );
   }

   void test_register_cast()
   {
      utilib::Type_Manager tm(false);
      tm.setErrorExceptions(false);
      tm.setWarningExceptions(false);
      tm.setDefaultForceExact(false);

      size_t len = 0;
      bool exact = false;
      A a;
      B b;

      // check that things start off empty
      TS_ASSERT(!tm.lexical_castable(typeid(A), typeid(B)));

      // the identity cast always works
      exact = false;
      TS_ASSERT(tm.lexical_castable(typeid(A), typeid(A), exact, len));
      TS_ASSERT( exact );
      TS_ASSERT_EQUALS(len, 0);

      // ... the same holds for the UntypedAnyContainer
      exact = false;
      TS_ASSERT(tm.lexical_castable( typeid(A), typeid(UntypedAnyContainer), 
                                     exact, len ));
      TS_ASSERT( exact );
      TS_ASSERT_EQUALS(len, 0);

      // while casting out of an UntypedAnyContainer is supported, we
      // cannot tell if there is an available cast chain without the
      // actual container.
      TS_ASSERT( !tm.lexical_castable( typeid(UntypedAnyContainer), typeid(A),
                                       exact, len ));

      // register a cast
      tm.register_lexical_cast(typeid(A), typeid(B), &cast_A2B, 1);
      TS_ASSERT(tm.lexical_castable(typeid(A), typeid(B), exact, len));
      TS_ASSERT(!exact);
      TS_ASSERT_EQUALS(len, 1);
      // make sure the registration took
      a = 1;
      TS_ASSERT_EQUALS(0, tm.lexical_cast(a, b));
      TS_ASSERT_EQUALS(1, (int)b);

      // but no casts from a resistered type to a non-registered type
      TS_ASSERT(!tm.lexical_castable(typeid(A), typeid(int)));

      // test forcably-exact casts
      TS_ASSERT(tm.lexical_castable(typeid(A), typeid(B), false));
      TS_ASSERT(!tm.lexical_castable(typeid(A), typeid(B), true));

      // Attempt to re-register
      tm.register_lexical_cast(typeid(A), typeid(B), &cast_A2B_error);
      //TS_ASSERT_EQUALS(ans, utilib::warning::Typemanager_OverrideCastFcn);
      TS_ASSERT(tm.lexical_castable(typeid(A), typeid(B), exact, len));
      TS_ASSERT(exact);
      TS_ASSERT_EQUALS(len, size_t(1));
      // make sure casting a->b returns an error for exact casts (the
      // registration took)
      a = 2;
      TS_ASSERT_EQUALS(-1000, tm.lexical_cast(a, b, true));
      TS_ASSERT_EQUALS(2000, (int)b);
      // make sure casting a->b returns NO error for inexact casts (the
      // registration took,  failed, and the inexact route was used)
      a = 2;
      TS_ASSERT_EQUALS(0, tm.lexical_cast(a, b));
      TS_ASSERT_EQUALS(2, (int)b);


#if 0 // Since the new TypeManager supports multiple casts between two
      // types, this test is no longer needed or valid.
      tm.setWarningExceptions(true);
      // Attempt to re-register
      TS_ASSERT_THROWS_ASSERT
         ( tm.register_lexical_cast( typeid(A), typeid(B), &cast_A2B),
           utilib::typeManager_error &e,
           TEST_WHAT(e, "TypeManager::register_lexical_cast - overriding "
                     "cast function from ") );
      // make sure the re-registration took
      a = 3;
      TS_ASSERT_EQUALS(0, tm.lexical_cast(a, b));
      TS_ASSERT_EQUALS(3, (int)b);
#endif

      TS_ASSERT_THROWS_ASSERT
         ( tm.register_lexical_cast( typeid(A), typeid(B), NULL),
           utilib::typeManager_error &e,
           TEST_WHAT(e, "Type_Manager::register_lexical_cast(): NULL cast "
                     "function for cast { ") );
           
      TS_ASSERT_THROWS_ASSERT
         ( tm.register_lexical_cast( typeid(A), typeid(A), &cast_A2B),
           utilib::typeManager_error &e,
           TEST_WHAT(e, "Type_Manager::register_lexical_cast(): cannot "
                     "register cast to the same type { ") );
           
   }


   void test_clear_registrations()
   {
      // clearing everything works...
      Type_Manager tm1;
      TS_ASSERT( tm1.lexical_castable(typeid(int), typeid(double)) );
      tm1.clear_lexical_casts();
      TS_ASSERT( !tm1.lexical_castable(typeid(int), typeid(double)) );


      // clearing key routes works
      Type_Manager tm;
      TS_ASSERT( tm.lexical_castable(typeid(int), typeid(double), true) );
      TS_ASSERT_EQUALS
         ( 1, tm.clear_lexical_casts(typeid(long), typeid(double)) );
      TS_ASSERT_EQUALS
         ( 1, tm.clear_lexical_casts(typeid(unsigned long),typeid(double)) );
      TS_ASSERT( !tm.lexical_castable(typeid(int), typeid(double), true) );

      // clearing non-existant routes fails
      tm.setWarningExceptions(false);
      TS_ASSERT_EQUALS
         ( 0, tm.clear_lexical_casts(typeid(long), typeid(double)) );
      TS_ASSERT_EQUALS( tm.lastError(),
                        utilib::warning::Typemanager_NonexistentCastFcn );

      tm.setWarningExceptions(true);
      TS_ASSERT_THROWS_ASSERT
         ( tm.clear_lexical_casts(typeid(long), typeid(double)),
           utilib::typeManager_error &e,
           TEST_WHAT(e, "Type_Manager::clear_lexical_cast(): attempt to "
                     "clear nonexistent cast function from ") );
   }


   void test_print_lexical_table()
   {
      utilib::Type_Manager tm(false);
      std::stringstream ss;

      std::string ref = "\n"
         " \\TO\n"
         "FR\\.\n";
      tm.printLexicalCastingTable(ss);
      TS_ASSERT_EQUALS(ss.str(), ref);

      // register a (fake) int -> double caster
      tm.register_lexical_cast(typeid(int), typeid(double), &cast_A2B);

      // print a new copy of the table
      ss.str("");
      tm.printLexicalCastingTable(ss);
      // NB: this test may fail if the demangled names alphabetize in a
      // different order (not likely)
      ref = "1 = " + utilib::demangledName(typeid(double));
      ref += "\n2 = " + utilib::demangledName(typeid(int));
      ref += "\n"
         "\n"
         " \\TO 1  2 \n"
         "FR\\.------\n"
         " 1 | 0  - \n"
         " 2 | 1  0 \n";
      TS_ASSERT_EQUALS(ss.str(), ref);
      
      // register a (fake) char -> double caster
      tm.register_lexical_cast(typeid(char), typeid(double), &cast_A2B, 1);

      // print a new copy of the table
      ss.str("");
      tm.printLexicalCastingTable(ss);
      //std::cerr << "!" << ss.str() << "!" << std::endl;
      // NB: this test may fail if the demangled names alphabetize in a
      // different order (not likely)
      ref = "1 = " + utilib::demangledName(typeid(char));
      ref += "\n2 = " + utilib::demangledName(typeid(double));
      ref += "\n3 = " + utilib::demangledName(typeid(int));
      ref += "\n"
         "\n"
         " \\TO 1  2  3 \n"
         "FR\\.---------\n"
         " 1 | 0  1* - \n"
         " 2 | -  0  - \n"
         " 3 | -  1  0 \n";
      TS_ASSERT_EQUALS(ss.str(), ref);

      // make sure that we restore the previous stream flags
      ss.str("");
      ss.setf(std::ios::left);
      tm.printLexicalCastingTable(ss);
      TS_ASSERT_EQUALS(ss.str(), ref);
      TS_ASSERT_DIFFERS(( ss.flags() & std::ios::left ), 0);
   }

   void test_chainSelection()
   {
      utilib::Type_Manager tm(false);
      size_t len = 0;
      bool exact = true;

      TS_ASSERT(!tm.lexical_castable(typeid(A), typeid(D)));

      // register a 3-step approximate cast: a -> b ~> c -> d
      tm.register_lexical_cast(typeid(A), typeid(B), &cast_A2B);
      tm.register_lexical_cast(typeid(B), typeid(C), &cast_A2B, 1);
      tm.register_lexical_cast(typeid(C), typeid(D), &cast_A2B);
      
      TS_ASSERT( tm.lexical_castable(typeid(A), typeid(D), false));
      TS_ASSERT(!tm.lexical_castable(typeid(A), typeid(D), true));
      tm.setDefaultForceExact(false);
      TS_ASSERT(!tm.lexical_castable(typeid(A), typeid(D), true));
      TS_ASSERT(tm.lexical_castable(typeid(A), typeid(D), exact, len));
      TS_ASSERT(!exact);
      TS_ASSERT_EQUALS(len, size_t(3));

      // register a 2-step route
      exact = true;
      tm.register_lexical_cast(typeid(B), typeid(D), &cast_A2B, 1);
      TS_ASSERT(!tm.lexical_castable(typeid(A), typeid(D), true));
      TS_ASSERT(tm.lexical_castable(typeid(A), typeid(D), exact, len));
      TS_ASSERT(!exact);
      TS_ASSERT_EQUALS(len, size_t(2));
      
      // re-register the longer route so it is exact
      tm.register_lexical_cast(typeid(B), typeid(C), &cast_A2B);
      TS_ASSERT(tm.lexical_castable(typeid(A), typeid(D), true));
      TS_ASSERT(tm.lexical_castable(typeid(A), typeid(D), exact, len));
      TS_ASSERT(exact);
      TS_ASSERT_EQUALS(len, size_t(3));

      // register a 1-step cast
      exact = false;
      tm.register_lexical_cast(typeid(A), typeid(D), &cast_A2B);
      TS_ASSERT(tm.lexical_castable(typeid(A), typeid(D), true));
      TS_ASSERT(tm.lexical_castable(typeid(A), typeid(D), exact, len));
      TS_ASSERT(exact);
      TS_ASSERT_EQUALS(len, size_t(1));

   }

};

} // namespace utilib
