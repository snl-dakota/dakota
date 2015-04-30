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

#include <utilib/Property.h>

#ifdef UTILIB_HAVE_BOOST
#include <boost/bind.hpp>
#endif

namespace utilib {
namespace {

void increment_and_set(utilib::Any& data, const utilib::Any value)
{
   int i;
   utilib::TypeManager()->lexical_cast(value, i);
   data = ++i;
}

void int_fetch(const utilib::Any& data, utilib::Any& value)
{
   value = data;
}

void int_fetch_increment(const utilib::Any& data, utilib::Any& value)
{
   int i;
   utilib::TypeManager()->lexical_cast(data, i);
   value = i;
   const_cast<utilib::Any&>(data) = i+1;
}

class A
{
public:
   int value;
};
 
class B : public A
{
public:
   bool operator==(const B& rhs) const
   { return value == rhs.value; }

   bool operator<(const B& rhs) const
   { return value < rhs.value; }
};

int cast_A2B(const utilib::Any &src, utilib::Any &dest)
{
   dest.set<B>().value = src.expose<A>().value;
   return 0;
}
int cast_B2A(const utilib::Any &src, utilib::Any &dest)
{
   dest.set<A>().value = src.expose<B>().value;
   return 0;
}

} // namespace (local)

DEFINE_DEFAULT_ANY_COMPARATOR(B);



namespace unittest { class Test_Property; }
class utilib::unittest::Test_Property : public CxxTest::TestSuite
{
public:
   void test_bound_constructor()
   {
      int i = 5;
      Property prop(i);
      TS_ASSERT(!prop.get().empty());
      TS_ASSERT(!prop.get().is_immutable());
      TS_ASSERT(prop.get().is_type(typeid(int)));
      TS_ASSERT_EQUALS(prop.get().anyCount(), (size_t)0);
      TS_ASSERT_EQUALS(i, prop.get().expose<int>());
      prop.set(7);
      TS_ASSERT_EQUALS(i, 7);
      TS_ASSERT_EQUALS(prop.get().expose<int>(), 7);
   }

   void test_unbound_any_constructor()
   {
      Property prop;
      TS_ASSERT(prop.get().empty());
      TS_ASSERT(!prop.get().is_immutable());
      TS_ASSERT_EQUALS(prop.get().anyCount(), (size_t)0);
   }

   void test_unbound_typed_constructor()
   {
      Property prop(Property::Bind<int>());
      TS_ASSERT(!prop.get().empty());
      TS_ASSERT(!prop.get().is_immutable());
      TS_ASSERT_EQUALS(prop.get().anyCount(), (size_t)0);
      TS_ASSERT(prop.get().is_type(typeid(int)));
   }

   void test_copy_constructor()
   {
      Property p1;
      p1 = 5;

      Property p2 = p1;
      TS_ASSERT( p2.get().is_type(typeid(int)) );
      TS_ASSERT_EQUALS(p2, 5);
      TS_ASSERT_EQUALS(p1, 5);
      p1 = 6;
      TS_ASSERT_EQUALS(p2, 6);
      p2 = 7;
      TS_ASSERT_EQUALS(p1, 7);

      Property p3(p1);
      TS_ASSERT( p3.get().is_type(typeid(int)) );
      TS_ASSERT_EQUALS(p3, 7);
      TS_ASSERT_EQUALS(p1, 7);
      p1 = 8;
      TS_ASSERT_EQUALS(p3, 8);
      p3 = 9;
      TS_ASSERT_EQUALS(p1, 9);

      Property p4 = Property(utilib::Property::Bind<int>(5));
      TS_ASSERT( p4.get().is_type(typeid(int)) );
      TS_ASSERT_EQUALS(p4, 5);
      p4 = 6;
      TS_ASSERT_EQUALS(p4, 6);
   }

   void test_special_conversions()
   {
      Property p(Property::Bind<int>(42));
      static_cast<void>(p);

      utilib::Any a1 = p;
      TS_ASSERT( a1.is_type(typeid(int)) );
   }

   void test_implicit_any_ref_construction()
   {
#if defined(UTILIB_AIX_CC)
      //TS_SKIP(...);
      TS_WARN("Implicit construction of AnyRef/AnyFixedRef from Property "
              "is not available in AIX");
#else
      {
         typedef ReadOnly_Property prop_t;
         prop_t p = Property(Property::Bind<int>(42));

         utilib::AnyRef ar = p;
         TS_ASSERT( ar.is_type(typeid(prop_t)) );
         TS_ASSERT( ar.expose<prop_t>().get().is_type(typeid(int)) );
         utilib::AnyFixedRef afr = p;
         TS_ASSERT( afr.is_type(typeid(prop_t)) );
         TS_ASSERT( afr.expose<prop_t>().get().is_type(typeid(int)) );
      }
      {
         typedef Property prop_t;
         prop_t p(Property::Bind<int>(42));

         utilib::AnyRef ar = p;
         TS_ASSERT( ar.is_type(typeid(prop_t)) );
         TS_ASSERT( ar.expose<prop_t>().get().is_type(typeid(int)) );
         utilib::AnyFixedRef afr = p;
         TS_ASSERT( afr.is_type(typeid(prop_t)) );
         TS_ASSERT( afr.expose<prop_t>().get().is_type(typeid(int)) );
      }
      {
         typedef Privileged_Property prop_t;
         prop_t p(Property::Bind<int>(42));

         utilib::AnyRef ar = p;
         TS_ASSERT( ar.is_type(typeid(prop_t)) );
         TS_ASSERT( ar.expose<prop_t>().get().is_type(typeid(int)) );
         utilib::AnyFixedRef afr = p;
         TS_ASSERT( afr.is_type(typeid(prop_t)) );
         TS_ASSERT( afr.expose<prop_t>().get().is_type(typeid(int)) );
      }
#endif
   }

   void test_implicit_any_ref_assignment()
   {
      {
         typedef ReadOnly_Property prop_t;
         prop_t p = Property(Property::Bind<int>(42));

         utilib::AnyRef ar;
         ar = p;
         TS_ASSERT( ar.is_type(typeid(prop_t)) );
         TS_ASSERT( ar.expose<prop_t>().get().is_type(typeid(int)) );
         // Comparing an RHS property wrapped within an immutable Any
         utilib::AnyFixedRef afr;
         afr = p;
         TS_ASSERT( afr.is_type(typeid(prop_t)) );
         TS_ASSERT( afr.expose<prop_t>().get().is_type(typeid(int)) );
      }
      {
         typedef Property prop_t;
         prop_t p(Property::Bind<int>(42));

         utilib::AnyRef ar;
         ar = p;
         TS_ASSERT( ar.is_type(typeid(prop_t)) );
         TS_ASSERT( ar.expose<prop_t>().get().is_type(typeid(int)) );
         // Comparing an RHS property wrapped within an immutable Any
         utilib::AnyFixedRef afr;
         afr = p;
         TS_ASSERT( afr.is_type(typeid(prop_t)) );
         TS_ASSERT( afr.expose<prop_t>().get().is_type(typeid(int)) );
      }
      {
         typedef Privileged_Property prop_t;
         prop_t p(Property::Bind<int>(42));

         utilib::AnyRef ar;
         ar = p;
         TS_ASSERT( ar.is_type(typeid(prop_t)) );
         TS_ASSERT( ar.expose<prop_t>().get().is_type(typeid(int)) );
         // Comparing an RHS property wrapped within an immutable Any
         utilib::AnyFixedRef afr;
         afr = p;
         TS_ASSERT( afr.is_type(typeid(prop_t)) );
         TS_ASSERT( afr.expose<prop_t>().get().is_type(typeid(int)) );
      }
   }

   void test_explicit_any_ref()
   {
      Property p(Property::Bind<int>(42));

      utilib::AnyRef a2 = p.as<utilib::AnyRef>();
      TS_ASSERT( a2.is_type(typeid(Property)) );
      TS_ASSERT( a2.expose<Property>().get().is_type(typeid(int)) );
      utilib::AnyFixedRef a3 = p.as<utilib::AnyFixedRef>();
      TS_ASSERT( a3.is_type(typeid(Property)) );
      TS_ASSERT( a3.expose<Property>().get().is_type(typeid(int)) );

      utilib::AnyRef a4;
      a4.set(p, true);
      TS_ASSERT( a4.is_type(typeid(Property)) );
      TS_ASSERT( a4.expose<Property>().get().is_type(typeid(int)) );
      utilib::AnyFixedRef a5;
      a5.set(p, true, true);
      TS_ASSERT( a5.is_type(typeid(Property)) );
      TS_ASSERT( a5.expose<Property>().get().is_type(typeid(int)) );
   }

   void test_implicit_use()
   {
      int i = 3;
      Property prop(i);
      int j = prop;
      TS_ASSERT_EQUALS(i, j);
      prop = 10;
      TS_ASSERT_EQUALS(i, 10);
      double d = prop;
      TS_ASSERT_EQUALS(d, 10.);

      j = 5;
      utilib::AnyFixedRef a(j);
      prop = a;
      TS_ASSERT_EQUALS(i, 5);
      TS_ASSERT_EQUALS(a.anyCount(), (size_t)0);
   }

   void test_implicit_any()
   {
      int i = 42;
      Property prop(i);

      utilib::Any a = prop;
      TS_ASSERT(a.is_type(typeid(int)));
      TS_ASSERT_EQUALS(prop.get().anyCount(), (size_t)0);
      TS_ASSERT_EQUALS(a.anyCount(), (size_t)0);
      TS_ASSERT_EQUALS(a, 42);
   }

   void test_assignment()
   {
      Property p1;
      Property p2;

      p1 = 5;
      p2 = p1;
      TS_ASSERT(p2.get().is_type(typeid(int)));
      TS_ASSERT_EQUALS(p2, 5);
      TS_ASSERT_EQUALS(p2, p1);
   }

   void test_equality()
   {
      Property p;
      Property p1;
      Property p2;

      p1 = 5;
      p2 = 5.0;

      TS_ASSERT(p1.get().is_type(typeid(int)));
      TS_ASSERT(p2.get().is_type(typeid(double)));

      // Comparing two properties...
      TS_ASSERT_EQUALS(p,  p);
      TS_ASSERT_EQUALS(p1, p1);
      TS_ASSERT_EQUALS(p2, p2);
      TS_ASSERT( !( p == p1) );
      TS_ASSERT( !( p1 == p) );
      TS_ASSERT_EQUALS(p1, p2);
      TS_ASSERT_EQUALS(p2, p1);

      // Comparing "special" properties
      Privileged_Property pp;
      pp = 5.0;
      TS_ASSERT_EQUALS(pp, pp);
      TS_ASSERT_EQUALS(p1, pp);
      TS_ASSERT_EQUALS(pp, p1);

      ReadOnly_Property rp = pp.set_readonly();
      TS_ASSERT_EQUALS(rp, rp);
      TS_ASSERT_EQUALS(p1, rp);
      TS_ASSERT_EQUALS(rp, p1);

      // Comparing an RHS property wrapped within an Any
      utilib::Any a1(pp);
      TS_ASSERT_EQUALS(p1, a1);
      a1 = pp.set_readonly();
      TS_ASSERT_EQUALS(p1, a1);
      a1.set(p2);
      TS_ASSERT_EQUALS(p1, a1);
      a1.set(5.0);
      TS_ASSERT_EQUALS(p1, a1);

      // Comparing an RHS property wrapped within an Any reference
      utilib::AnyRef a2 = p2.as<utilib::AnyRef>();
      TS_ASSERT_EQUALS(p1, a2);
      // Comparing an RHS property wrapped within an immutable Any
      utilib::AnyFixedRef a3 = p2.as<utilib::AnyFixedRef>();
      TS_ASSERT_EQUALS(p1, a3);

      /// Comparing to a general RHS Any
      TS_ASSERT( !( p == 5 ) );
      TS_ASSERT_EQUALS(p, utilib::Any());
      TS_ASSERT( !( p1 == utilib::Any() ) );
      TS_ASSERT_EQUALS(p1, 5);
      TS_ASSERT_EQUALS(p1, 5.0);
      TS_ASSERT_EQUALS(p2, 5.0);
      TS_ASSERT_THROWS_ASSERT
         ( p2 == B(), utilib::bad_lexical_cast &e,
           TEST_WHAT( e, "Type_Manager::lexical_cast(): no castable "
                      "route from " /* "double to string" */ ) );
      // NB: we do not test the actual types returned, because the
      // demangled type names are not always "double" and "int"

      // Test that an inexact cast (from double to int) will fall through
      // to equality_compare_any()
      p2 = 5.5;
      TS_ASSERT( ! (p2 == 5) );

      // Test that B is comparable, but A is not
      Property p3(Property::Bind<A>());
      utilib::Any a;
      a.set<A>();
      Property p4(Property::Bind<B>());
      utilib::Any b;
      b.set<B>();
      TS_ASSERT_THROWS( p3 == a, utilib::any_not_comparable);
      TS_ASSERT_EQUALS(p4, b);

      // Test that comparing A to B will convert to B for comparison
      utilib::TypeManager()->register_lexical_cast
         ( typeid(A), typeid(B), &cast_A2B );
      utilib::TypeManager()->register_lexical_cast
         ( typeid(B), typeid(A), &cast_B2A );

      TS_ASSERT_EQUALS(p3, b);
      TS_ASSERT_EQUALS(p4, a);

      utilib::TypeManager()->clear_lexical_casts( typeid(A), typeid(B) );
      utilib::TypeManager()->clear_lexical_casts( typeid(B), typeid(A) );
   }

   void test_lessThan()
   {
      Property p;
      Property p1;
      Property p2;

      p1 = 6;
      p2 = 5.0;

      TS_ASSERT(p1.get().is_type(typeid(int)));
      TS_ASSERT(p2.get().is_type(typeid(double)));

      // Comparing two properties...
      TS_ASSERT( !( p1 < p2 ));
      TS_ASSERT(  ( p2 < p1 ));
      TS_ASSERT(  ( p  < p1 ));
      TS_ASSERT( !( p1 < p  ));
      TS_ASSERT( !( p  < p  ));
      TS_ASSERT( !( p1 < p1 ));
      TS_ASSERT( !( p2 < p2 ));

      // Comparing "special" properties
      Privileged_Property pp;
      pp = 6.0;
      TS_ASSERT( !( pp < p2 ));
      TS_ASSERT(  ( p2 < pp ));
      TS_ASSERT(  ( p  < pp ));
      TS_ASSERT( !( pp < p  ));
      TS_ASSERT( !( pp < pp ));

      ReadOnly_Property rp = pp.set_readonly();
      TS_ASSERT( !( rp < p2 ));
      TS_ASSERT(  ( p2 < rp ));
      TS_ASSERT(  ( p  < rp ));
      TS_ASSERT( !( rp < p  ));
      TS_ASSERT( !( rp < rp ));

      // Comparing an RHS property wrapped within an Any
      utilib::Any a1(pp);
      TS_ASSERT( !( p1 < a1 ));
      TS_ASSERT(  ( p2 < a1 ));
      a1 = pp.set_readonly();
      TS_ASSERT( !( p1 < a1 ));
      TS_ASSERT(  ( p2 < a1 ));
      a1.set(p1);
      TS_ASSERT( !( p1 < a1 ));
      TS_ASSERT(  ( p2 < a1 ));
      a1.set(6.0);
      TS_ASSERT( !( p1 < a1 ));
      TS_ASSERT(  ( p2 < a1 ));

      // Comparing an RHS property wrapped within an Any reference
      utilib::AnyRef a2a;
      a2a = p1;
      TS_ASSERT( !( p1 < a2a ));
      TS_ASSERT(  ( p2 < a2a ));
      // Comparing an RHS property wrapped within an immutable Any
      utilib::AnyFixedRef a3a;
      a3a = p1;
      TS_ASSERT( !( p1 < a3a ));
      TS_ASSERT(  ( p2 < a3a ));

      // Comparing an RHS property wrapped within an Any reference
      utilib::AnyRef a2 = p1.as<utilib::AnyRef>();
      TS_ASSERT( !( p1 < a2 ));
      TS_ASSERT(  ( p2 < a2 ));
      // Comparing an RHS property wrapped within an immutable Any
      utilib::AnyFixedRef a3 = p1.as<utilib::AnyFixedRef>();
      TS_ASSERT( !( p1 < a3 ));
      TS_ASSERT(  ( p2 < a3 ));

      /// Comparing to a general RHS Any
      TS_ASSERT( !( p1 < 5 ));
      TS_ASSERT( !( p1 < 5.0 ));
      TS_ASSERT(  ( p1 < 7 ));
      TS_ASSERT(  ( p1 < 7.0 ));

      TS_ASSERT( !( p2 < 5.0 ));
      TS_ASSERT(  ( p2 < 7.0 ));

      TS_ASSERT(  ( p < 5 ));
      TS_ASSERT( !( p  < utilib::Any() ));
      TS_ASSERT( !( p2 < utilib::Any() ));

      TS_ASSERT_THROWS_ASSERT
         ( p2 < B(), utilib::bad_lexical_cast &e,
           TEST_WHAT( e, "Type_Manager::lexical_cast(): no castable "
                      "route from " /* "double to string" */ ) );
      // NB: we do not test the actual types returned, because the
      // demangled type names are not always "double" and "int"

      // Test that an inexact cast (from double to int) will fall through
      // to lessThan_compare_any()
      p2 = 5.5;
      TS_ASSERT( ! (p2 < 5) )
         TS_ASSERT(   (p2 < 6) )


      // Test that B is comparable, but A is not
      Property p3(Property::Bind<A>());
      utilib::Any a;
      a.set<A>().value = 5;
      Property p4(Property::Bind<B>());
      utilib::Any b1;
      b1.set<B>().value = -5;
      utilib::Any b2;
      b2.set<B>().value = 5;
      TS_ASSERT_THROWS( p3 < a, utilib::any_not_comparable);
      TS_ASSERT( ! (p4 < b1) );
      TS_ASSERT(   (p4 < b2) );

      // Test that comparing A to B will convert to B for comparison
      utilib::TypeManager()->register_lexical_cast
         ( typeid(A), typeid(B), &cast_A2B );
      utilib::TypeManager()->register_lexical_cast
         ( typeid(B), typeid(A), &cast_B2A );

      TS_ASSERT( ! (p3 < b1) );
      TS_ASSERT(   (p3 < b2) );
      TS_ASSERT(   (p4 < a) );
      a.set<A>();
      TS_ASSERT( ! (p4 < a) );

      utilib::TypeManager()->clear_lexical_casts( typeid(A), typeid(B) );
      utilib::TypeManager()->clear_lexical_casts( typeid(B), typeid(A) );
   }

   void test_any_extraction()
   {
      utilib::Property p;
      p = 42;
      
      utilib::Any a = p;
      TS_ASSERT( ! a.empty() );
      TS_ASSERT( a.is_type(typeid(int)) );
      TS_ASSERT_EQUALS( a.anyCount(), 0u );
      TS_ASSERT_EQUALS( a, 42 );
      a.clear();

      a = p.get();
      TS_ASSERT( ! a.empty() );
      TS_ASSERT( a.is_type(typeid(int)) );
      TS_ASSERT_EQUALS( a.anyCount(), 0u );
      TS_ASSERT_EQUALS( a, 42 );
      a.clear();

      a = p.as<utilib::Any>();
      TS_ASSERT( ! a.empty() );
      TS_ASSERT( a.is_type(typeid(int)) );
      TS_ASSERT_EQUALS( a.anyCount(), 0u );
      TS_ASSERT_EQUALS( a, 42 );
      a.clear();
   }

   void test_derived_comparisons()
   {
      Property p1;
      Property p2;

      p1 = 5.0;
      p2 = 6;

      TS_ASSERT( ! ( p1 == p2 ) );
      TS_ASSERT(   ( p1 != p2 ) );
      TS_ASSERT(   ( p1 <  p2 ) );
      TS_ASSERT(   ( p1 <= p2 ) );
      TS_ASSERT( ! ( p1 >  p2 ) );
      TS_ASSERT( ! ( p1 >= p2 ) );

      TS_ASSERT( ! ( p2 == p1 ) );
      TS_ASSERT(   ( p2 != p1 ) );
      TS_ASSERT( ! ( p2 <  p1 ) );
      TS_ASSERT( ! ( p2 <= p1 ) );
      TS_ASSERT(   ( p2 >  p1 ) );
      TS_ASSERT(   ( p2 >= p1 ) );

      p2 = 5;

      TS_ASSERT(   ( p1 == p2 ) );
      TS_ASSERT( ! ( p1 != p2 ) );
      TS_ASSERT( ! ( p1 <  p2 ) );
      TS_ASSERT(   ( p1 <= p2 ) );
      TS_ASSERT( ! ( p1 >  p2 ) );
      TS_ASSERT(   ( p1 >= p2 ) );
   }

   void test_custom_set()
   {
      int i = 3;
#ifdef UTILIB_SOLARIS_CC
      Property prop( i, 
                     Property::set_fcn_t(increment_and_set), 
                     Property::get_fcn_t() );
#else
      Property prop( i, increment_and_set, Property::get_fcn_t() );
#endif
      int j = prop;
      TS_ASSERT_EQUALS(i, j);
      TS_ASSERT_EQUALS(i, 3);
      prop = 10;
      TS_ASSERT_EQUALS(i, 11);
      double d = prop;
      TS_ASSERT_EQUALS(d, 11.);
   }

   void test_custom_get()
   {
      int i = 3;
#ifdef UTILIB_SOLARIS_CC
      Property prop1
	( i, 
	  Property::set_fcn_t(),
	  Property::get_fcn_t(&int_fetch_increment) );
#else
      Property prop1(i, Property::set_fcn_t(), &int_fetch_increment);
#endif
      int j = prop1;
      TS_ASSERT_EQUALS(i, 4);
      TS_ASSERT_EQUALS(j, 3);
      prop1 = 10;
      TS_ASSERT_EQUALS(i, 10);
      double d = prop1;
      TS_ASSERT_EQUALS(d, 10.);
      TS_ASSERT_EQUALS(i, 11);

      // test that the returned Any is unique
#ifdef UTILIB_SOLARIS_CC
      Property prop2
         ( i, Property::set_fcn_t(), Property::get_fcn_t(&int_fetch) );
#else
      Property prop2(i, Property::set_fcn_t(), &int_fetch);
#endif
      j = prop2;
      TS_ASSERT_EQUALS(i, 11);
      TS_ASSERT_EQUALS(j, 11);
      TS_ASSERT_EQUALS(prop2.get().anyCount(), 0u);
   }

   void test_unbound_any_property()
   {
      Property prop;

      TS_ASSERT(prop.get().empty());

      prop = 5;
      TS_ASSERT(prop.get().is_type(typeid(int)));
      TS_ASSERT_EQUALS((int)prop, 5);
      TS_ASSERT_EQUALS((double)prop, 5.0);
      // same tests, just implicit casts
      TS_ASSERT_EQUALS(prop, 5);
      TS_ASSERT_EQUALS(prop, 5.0);

      TS_ASSERT(! prop.get().empty());
      prop.set(utilib::Any());
      TS_ASSERT(prop.get().empty());

      prop = 'A';
      TS_ASSERT(prop.get().is_type(typeid(char)));
      TS_ASSERT_EQUALS((char)prop, 'A');
      TS_ASSERT_EQUALS((int)prop, 65);

      // Assigning an AnyRef to an unbound property should perform a deep copy
      int i = 5;
      utilib::AnyFixedRef a(i);
      prop = a;
      TS_ASSERT_EQUALS(prop, 5);
      TS_ASSERT_EQUALS(prop.get().anyCount(), (size_t)0);
      TS_ASSERT_EQUALS(a.anyCount(), (size_t)0);

      // Assigning an AnyRef to an unbound property should break the reference
      i = 6;
      TS_ASSERT_EQUALS(a, 6);
      TS_ASSERT_EQUALS(prop, 5);
   }

   void test_unbound_typed_property_1()
   {
      Property prop(Property::Bind<double>());

      TS_ASSERT(prop.get().is_type(typeid(double)));

      double d = 3.1415;
      prop = d;
      TS_ASSERT(prop.get().is_type(typeid(double)));
      TS_ASSERT_EQUALS(prop, d);

      TS_ASSERT_THROWS_ASSERT
         (prop.set(utilib::Any()), utilib::property_error &e,
           TEST_WHAT( e, "Property::set(): "
                         "attempt to empty a bound Property" ) );

      prop = 5;
      TS_ASSERT(prop.get().is_type(typeid(double)));
      TS_ASSERT_EQUALS(prop, 5.0);
   }

   void test_unbound_typed_property_2()
   {
      Property prop(Property::Bind<double>(3.1415));

      double d = 3.1415;
      TS_ASSERT(prop.get().is_type(typeid(double)));
      TS_ASSERT_EQUALS(prop, d);

      prop = 5;
      TS_ASSERT(prop.get().is_type(typeid(double)));
      TS_ASSERT_EQUALS(prop, 5.0);
   }

   void test_property_alias()
   {
      int i = 5;
      Property prop1(i);
      Property prop2;
      prop2 = prop1;
      
      prop1 = 6;
      TS_ASSERT_EQUALS(i, 6);
      TS_ASSERT_EQUALS(prop1, 6);
      TS_ASSERT_EQUALS(prop2, 6);
      TS_ASSERT_EQUALS(prop1.get().anyCount(), (size_t)0);
   }

   void test_unbound_property_alias()
   {
      Property prop1;
      Property prop2;
      prop2 = prop1;
      
      prop1 = 6;
      TS_ASSERT_EQUALS(prop1, 6);
      TS_ASSERT_EQUALS(prop2, 6);
      TS_ASSERT_EQUALS(prop1.get().anyCount(), (size_t)0);
   }

   void test_readonly_construction_and_assignment()
   {
      ReadOnly_Property r;
      TS_ASSERT_EQUALS(r, utilib::Any());

      Privileged_Property pp(Property::Bind<double>(5));
      r = pp.set_readonly();
      TS_ASSERT_EQUALS(r, 5);

      ReadOnly_Property r2;
      TS_ASSERT_EQUALS(r2, utilib::Any());

      r2 = r;
      TS_ASSERT_EQUALS(r2, 5);

      Property p(Property::Bind<int>(42));
      r = p;
      TS_ASSERT_EQUALS(r, 42);
      TS_ASSERT_EQUALS(r2, 5);
   }

   void test_print()
   {
      std::stringstream ss1;
      std::stringstream ss2;
      Property prop;

      utilib::Any a;
      ss1 << a;
      ss2 << prop;
      TS_ASSERT_EQUALS(ss1.str(), ss2.str());

      prop = 42;
      ss2.str("");
      ss2 << prop;
      TS_ASSERT_EQUALS("42", ss2.str());

      // Also test the cxxtest ValueTraits class (for coverage)
      utilib::Privileged_Property pp;
      pp = 12;
      CxxTest::ValueTraits<utilib::Privileged_Property> vt(pp);
      TS_ASSERT_EQUALS("12", std::string(vt.asString()));
   }

   void test_onChange()
   {
      cb1_value() = -1;
      cb2_value() = -1;
      int i = 0;
      Property prop(i);

      prop = 1;
      TS_ASSERT_EQUALS(cb1_value(), -1);
      TS_ASSERT_EQUALS(cb2_value(), -1);

      prop.onChange().connect(&cb1);

      prop = 2;
      TS_ASSERT_EQUALS(cb1_value(), 12);
      TS_ASSERT_EQUALS(cb2_value(), -1);

      prop.onChange().connect(&cb2);

      prop = 3;
      TS_ASSERT_EQUALS(cb1_value(), 13);
      TS_ASSERT_EQUALS(cb2_value(), 103);

      prop.onChange().disconnect(&cb2);

      prop = 4;
      TS_ASSERT_EQUALS(cb1_value(), 14);
      TS_ASSERT_EQUALS(cb2_value(), 103);

      prop.onChange().connect(&cb2);
      prop.onChange().disconnect(&cb1);

      prop = 5;
      TS_ASSERT_EQUALS(cb1_value(), 14);
      TS_ASSERT_EQUALS(cb2_value(), 105);

      prop.onChange().disconnect(&cb2);

      prop = 6;
      TS_ASSERT_EQUALS(cb1_value(), 14);
      TS_ASSERT_EQUALS(cb2_value(), 105);
   }

   void test_onChange_cascade()
   {
#ifdef UTILIB_HAVE_BOOST
      Property p1(Property::Bind<int>());
      Property p2;

      p1.onChange().connect(p2.bind_set());

      TS_ASSERT(p2.get().empty());
      p1 = 5;
      TS_ASSERT(p2.get().is_type(typeid(int)));
      TS_ASSERT_EQUALS(p2, 5);

      Property p3(Property::Bind<int>(1));
      p2.onChange().connect(p3.bind_set());

      TS_ASSERT(p3.get().is_type(typeid(int)));
      TS_ASSERT_EQUALS(p3, 1);
      p1 = 10;
      TS_ASSERT(p3.get().is_type(typeid(int)));
      TS_ASSERT_EQUALS(p3, 10);

      
#else
      TS_WARN("UTILIB compiled without Boost support.  Test cannot run");
#endif
   }

   void test_onChange_loop_detection()
   {
      cb3_value().onChange().connect(&cb4);
      cb3_value() = 5;
      TS_ASSERT_EQUALS(cb4_value(), 5.0);

      cb4_value().onChange().connect(&cb3);
      TS_ASSERT_THROWS_ASSERT
         ( cb3_value() = 5, utilib::property_error &e,
           TEST_WHAT( e, "Property::set(): "
                      "onChange callback loop detected "
                      "(onChange triggers changes which trigger itself)." ) );

      cb3_value().onChange().disconnect(&cb4);
      cb4_value() = 10;
      TS_ASSERT_EQUALS(cb3_value(), 10.0);
      cb4_value().onChange().disconnect(&cb3);

#ifdef UTILIB_HAVE_BOOST
      Property p1;
      Property p2;

      p1.onChange().connect(p2.bind_set());
      p1 = 5;
      TS_ASSERT(p2.get().is_type(typeid(int)));
      TS_ASSERT_EQUALS(p2, 5);

      p2.onChange().connect(p1.bind_set());
      TS_ASSERT_THROWS_ASSERT
         ( p1 = 15, utilib::property_error &e,
           TEST_WHAT( e, "Property::set(): "
                      "onChange callback loop detected "
                      "(onChange triggers changes which trigger itself)." ) );
#endif
   }


   void test_onChange_loop_override()
   {
      cb3_value().onChange().connect(&cb5_guarded);
      cb3_value() = 5;
      TS_ASSERT_EQUALS(cb5_value(), 6.0);
      TS_ASSERT_EQUALS(cb5_guard_count(), 1);
      cb5_guard_count() = 0;

      cb5_value().onChange().connect(&cb3);
      cb3_value() = 10;
      TS_ASSERT_EQUALS(cb5_value(), 12.0);
      TS_ASSERT_EQUALS(cb5_guard_count(), 3);
      cb5_guard_count() = 0;

      cb3_value().onChange().disconnect(&cb5_guarded);
      cb3_value().onChange().connect(&cb5_unguarded);
      TS_ASSERT_THROWS_ASSERT
         ( cb3_value() = 5, utilib::property_error &e,
           TEST_WHAT( e, "Privileged_Property::allowOnChangeRecursion: "
                      "exceeded maximum number of loops (2 >= 2)") );
      TS_ASSERT_EQUALS(cb5_guard_count(), 3);

      cb3_value().onChange().disconnect(&cb5_unguarded);
      cb5_value().onChange().disconnect(&cb3);
   }


   void test_validateListeners()
   {
      Property prop(Property::Bind<int>());
      prop = 1;
      TS_ASSERT_EQUALS(prop, 1);

      prop.onChange().connect(&counter);
      prop.validate().connect(&validate1);
      
      counter_value() = 0;
      prop = 1;
      TS_ASSERT_EQUALS(prop, 1);
      TS_ASSERT_EQUALS(counter_value(), 1);
      prop = 3;
      TS_ASSERT_EQUALS(prop, 3);
      TS_ASSERT_EQUALS(counter_value(), 2);
      TS_ASSERT_THROWS_ASSERT
         ( prop = 5, std::runtime_error &e,
           TEST_WHAT( e, "Property::set(): set disallowed "
                      "by validator callback for new value = 5" ) );
      TS_ASSERT_EQUALS(prop, 3);
      TS_ASSERT_EQUALS(counter_value(), 2);

      // test that all validators are called, even when one fails
      prop.validate().connect(&validate2);
      prop = 3;
      TS_ASSERT_EQUALS(prop, 3);
      TS_ASSERT_EQUALS(counter_value(), 4);
      TS_ASSERT_THROWS_ASSERT
         ( prop = 5, std::runtime_error &e,
           TEST_WHAT( e, "Property::set(): set disallowed "
                      "by validator callback for new value = 5" ) );
      TS_ASSERT_EQUALS(prop, 3);
      TS_ASSERT_EQUALS(counter_value(), 5);
   }

   void test_validateValueCast()
   {
      double d = 1.0;
      int i = 5;

      Property p0;
      p0.validate().connect(&validateDouble);

      // bad validation cast when the property is empty
      TS_ASSERT_THROWS_ASSERT
         ( p0 = i, utilib::bad_any_cast &e,
           TEST_WHAT( e, "Any::expose() - failed conversion from " ) );

      // OK: set the property to be a double
      TS_ASSERT_THROWS_NOTHING(p0 = d);
      TS_ASSERT_EQUALS(p0, 1.0);
      TS_ASSERT(p0.get().is_type(typeid(double)));

      // bad validation cast when the property is unbound, but a double
      TS_ASSERT_THROWS_ASSERT
         ( p0 = i, utilib::bad_any_cast &e,
           TEST_WHAT( e, "Any::expose() - failed conversion from " ) );
      TS_ASSERT_EQUALS(p0, 1.0);


      Property p1(Property::Bind<double>());
      p1.validate().connect(&validateDouble);

      TS_ASSERT_THROWS_NOTHING(p1 = d);
      TS_ASSERT_EQUALS(p1, 1.0);

      // OK validation cast when the property is bound to a double
      TS_ASSERT_THROWS_NOTHING(p1 = i);
      TS_ASSERT_EQUALS(p1, 5.0);

      // But the validation cast can fail on bad data...
      TS_ASSERT_THROWS_ASSERT
         ( p1 = 3.5, utilib::property_error &e,
           TEST_WHAT( e, "Property::set(): set disallowed "
                      "by validator callback for new value = 3.5" ) );
      TS_ASSERT_EQUALS(p1, 5.0);


      double ref = 0;
      Property p2(ref);
      p2.validate().connect(&validateDouble);

      TS_ASSERT_THROWS_NOTHING(p2 = d);
      TS_ASSERT_EQUALS(p2, 1.0);

      TS_ASSERT_THROWS_NOTHING(p2 = i);
      TS_ASSERT_EQUALS(p2, 5.0);

      TS_ASSERT_THROWS_ASSERT
         ( p1 = 3.5, utilib::property_error &e,
           TEST_WHAT( e, "Property::set(): set disallowed "
                      "by validator callback for new value = 3.5" ) );
      TS_ASSERT_EQUALS(p1, 5.0);
   }


private:
   static void cb1(const ReadOnly_Property &prop)
   {
      cb1_value() = 10 + (int)prop;
   }
   static int& cb1_value()
   {
      static int x = 0;
      return x;
   }

   static void cb2(const ReadOnly_Property &prop)
   {
      cb2_value() = 100 + (int)prop;
   }
   static int& cb2_value()
   {
      static int x = 0;
      return x;
   }

   static void cb3(const ReadOnly_Property &prop)
   {
      cb3_value().set(prop.get());
   }
   static Property& cb3_value()
   {
      static Property x;
      return x;
   }

   static void cb4(const ReadOnly_Property &prop)
   {
      cb4_value().set(prop.get());
   }
   static Property& cb4_value()
   {
      static Property x;
      return x;
   }

   static int& cb5_guard_count()
   {
      static int ans = 0;
      return ans;
   }
   static void cb5_unguarded(const ReadOnly_Property &prop)
   {
      cb5_guard_count()++;
      *cb5_value().allowOnChangeRecursion(2)->property() = 1 + (int)prop;
   }
   static void cb5_guarded(const ReadOnly_Property &prop)
   {
      if ( cb5_guard_count()++ >= 2 )
         return;
      *cb5_value().allowOnChangeRecursion(2)->property() = 1 + (int)prop;
   }
   static Privileged_Property& cb5_value()
   {
      static Privileged_Property x;
      return x;
   }

   static void counter(const ReadOnly_Property& /*prop*/)
   {
      counter_value()++;
   }
   static int& counter_value()
   {
      static int x = 0;
      return x;
   }

   static bool validate1( const ReadOnly_Property& /*prop*/, 
                          const utilib::Any &new_value )
   {
      int i;
      utilib::TypeManager()->lexical_cast(new_value, i);
      return i != 5;
   }
   static bool validate2( const ReadOnly_Property& prop,
                          const utilib::Any& /*new_value*/ )
   {
      counter(prop);
      return true;
   }
   static bool validateDouble( const ReadOnly_Property& /*prop*/,
                               const utilib::Any& new_value )
   {
      double val = new_value.expose<double>();
      int test = (int)val;
      return val == (double)test;
   }
};


namespace unittest { class Test_PrivilegedProperty; }
class utilib::unittest::Test_PrivilegedProperty : public CxxTest::TestSuite
{
public:
   void test_constructors()
   {
      MyClass mc;

      mc.set(10);
      TS_ASSERT_EQUALS(mc.Foo, 10.0);
   }

   void test_bound_constructor()
   {
      int i = 5;
      Privileged_Property prop(i);
      TS_ASSERT(!prop.get().empty());
      TS_ASSERT(!prop.get().is_immutable());
      TS_ASSERT(prop.get().is_type(typeid(int)));
      TS_ASSERT_EQUALS(prop.get().anyCount(), (size_t)0);
      TS_ASSERT_EQUALS(i, prop.get().expose<int>());
      prop.set(7);
      TS_ASSERT_EQUALS(i, 7);
      TS_ASSERT_EQUALS(prop.get().expose<int>(), 7);
   }

   void test_unbound_any_constructor()
   {
      Privileged_Property prop;
      TS_ASSERT(prop.get().empty());
      TS_ASSERT(!prop.get().is_immutable());
      TS_ASSERT_EQUALS(prop.get().anyCount(), (size_t)0);
   }

   void test_unbound_typed_constructor()
   {
      Privileged_Property prop(Property::Bind<int>());
      TS_ASSERT(!prop.get().empty());
      TS_ASSERT(!prop.get().is_immutable());
      TS_ASSERT_EQUALS(prop.get().anyCount(), (size_t)0);
      TS_ASSERT(prop.get().is_type(typeid(int)));
   }

   void test_copy_constructor()
   {
      Privileged_Property p1;
      p1 = 5;

      Privileged_Property p2 = p1;
      TS_ASSERT( p2.get().is_type(typeid(int)) );
      TS_ASSERT_EQUALS(p2, 5);
      TS_ASSERT_EQUALS(p1, 5);
      p1 = 6;
      TS_ASSERT_EQUALS(p2, 6);
      p2 = 7;
      TS_ASSERT_EQUALS(p1, 7);

      Privileged_Property p3(p1);
      TS_ASSERT( p3.get().is_type(typeid(int)) );
      TS_ASSERT_EQUALS(p3, 7);
      TS_ASSERT_EQUALS(p1, 7);
      p1 = 8;
      TS_ASSERT_EQUALS(p3, 8);
      p3 = 9;
      TS_ASSERT_EQUALS(p1, 9);
   }

   void test_downcast()
   {
      Privileged_Property pp(Property::Bind<int>());

      TS_ASSERT( static_cast<ReadOnly_Property*>(&pp)
                 ->get().is_type(typeid(int)) );
      TS_ASSERT( static_cast<Property*>(&pp)
                 ->get().is_type(typeid(int)) );

      TS_ASSERT( static_cast<ReadOnly_Property>(pp)
                 .get().is_type(typeid(int)) );
      TS_ASSERT( static_cast<Property>(pp)
                 .get().is_type(typeid(int)) );

      Property p(pp);
      TS_ASSERT( p.get().is_type(typeid(int)) );
     
   }

   void test_set()
   {
      double d;
      Privileged_Property pp(d);
      Property p = pp;

      pp.set(1);
      TS_ASSERT_EQUALS(p, 1.0);

      TS_ASSERT_THROWS_NOTHING(p.set(10));
      TS_ASSERT_EQUALS(p, 10.0);
      TS_ASSERT_EQUALS(pp, 10.0);

      pp.set_readonly();
      TS_ASSERT_THROWS_ASSERT
         ( p.set(11), std::runtime_error &e,
           TEST_WHAT( e, "Property::set(): attempt to set "
                        "a Property declared read-only." ) );
      TS_ASSERT_EQUALS(p, 10.0);

      pp.unset_readonly();
      TS_ASSERT_THROWS_NOTHING(p.set(12));
      TS_ASSERT_EQUALS(p, 12.0);
      TS_ASSERT_EQUALS(pp, 12.0);
   }

   void test_changeListener_cascade()
   {
#ifdef UTILIB_HAVE_BOOST
      Privileged_Property p1(Property::Bind<int>());
      Privileged_Property p2;

      p1.onChange().connect(p2.bind_set());

      TS_ASSERT(p2.get().empty());
      p1 = 5;
      TS_ASSERT(p2.get().is_type(typeid(int)));
      TS_ASSERT_EQUALS(p2, 5);

      Privileged_Property p3(Property::Bind<int>(1));
      p2.onChange().connect(p3.bind_set());

      TS_ASSERT(p3.get().is_type(typeid(int)));
      TS_ASSERT_EQUALS(p3, 1);
      p1 = 10;
      TS_ASSERT(p3.get().is_type(typeid(int)));
      TS_ASSERT_EQUALS(p3, 10);

      
#else
      TS_WARN("UTILIB compiled without Boost support.  Test cannot run");
#endif
   }

private:
   class MyClass
   {
   protected:
      Privileged_Property _Foo;

   public:
      ReadOnly_Property Foo;

      MyClass()
         : _Foo(Property::Bind<double>()),
           Foo(_Foo.set_readonly())
      {}

      void set(double d)
      {
         _Foo = d;
      }
   };

};

} // namespace utilib
