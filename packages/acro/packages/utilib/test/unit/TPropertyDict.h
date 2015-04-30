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

#include <utilib/PropertyDict.h>

namespace utilib {

namespace unittest { class Test_PropertyDict; }
class utilib::unittest::Test_PropertyDict : public CxxTest::TestSuite
{
public:
   void test_exists()
   {
      utilib::PropertyDict pd;

      TS_ASSERT( ! pd.exists("a") );
      TS_ASSERT( ! pd.exists("b") );

      pd.declare("a");

      TS_ASSERT( pd.exists("a") );
      TS_ASSERT( ! pd.exists("b") );

      pd.declare("b");

      TS_ASSERT( pd.exists("a") );
      TS_ASSERT( pd.exists("b") );

      TS_ASSERT_THROWS_ASSERT
         ( pd.declare("a"), 
           utilib::propertyDict_error &e,
           TEST_WHAT(e, "PropertyDict::declare(): "
                     "attempt to declare duplicate Property 'a'") );
   }

   void test_erase()
   {
      utilib::PropertyDict pd;

      TS_ASSERT( ! pd.exists("a") );
      pd.declare("a");
      TS_ASSERT( pd.exists("a") );
      pd.erase("a");
      TS_ASSERT( ! pd.exists("a") );
      TS_ASSERT_THROWS_ASSERT
         ( pd.erase("a"), 
           utilib::propertyDict_error &e,
           TEST_WHAT(e, "PropertyDict::erase(): "
                     "attempt to erase a nonexistent Property 'a'") );
   }

   void test_copy()
   {
      utilib::PropertyDict pd2;

      {
         utilib::PropertyDict pd1;
         pd1.declare("a");
         pd1.declare("b");
         pd2.declare("z");

         TS_ASSERT( pd1.exists("a") );
         TS_ASSERT( pd1.exists("b") );
         TS_ASSERT( pd2.exists("z") );
         
         pd2 = pd1;
         TS_ASSERT( pd1.exists("a") );
         TS_ASSERT( pd1.exists("b") );
         TS_ASSERT( pd2.exists("a") );
         TS_ASSERT( pd2.exists("b") );
         TS_ASSERT( ! pd2.exists("z") );
      }
      TS_ASSERT( pd2.exists("a") );
      TS_ASSERT( pd2.exists("b") );
      TS_ASSERT( ! pd2.exists("z") );
   }

   void test_get()
   {
      utilib::PropertyDict pd;

      int i = 5;
      utilib::Property p(i);
      pd.declare("a", p);

      TS_ASSERT_EQUALS(pd.get("a"), 5);

      pd.get("a") = 10;
      TS_ASSERT_EQUALS(i, 10);
      TS_ASSERT_EQUALS(pd.get("a"), 10);

      TS_ASSERT_THROWS_ASSERT
         ( pd.get("A"), 
           utilib::propertyDict_error &e,
           TEST_WHAT(e, "PropertyDict::get_impl(): "
                     "attempt to retrieve nonexistent Property 'A'") );

      pd.implicitDeclareIfDNE() = true;
      TS_ASSERT(pd.get("A").get().empty());
   }

   void test_const_get()
   {
      utilib::PropertyDict source_pd;
      int i = 5;
      utilib::Property p(i);
      source_pd.declare("a", p);

      {
         const utilib::PropertyDict &pd = source_pd;

         TS_ASSERT_EQUALS(pd.get("a"), 5);

         source_pd.get("a") = 10;
         TS_ASSERT_EQUALS(i, 10);
         TS_ASSERT_EQUALS(pd.get("a"), 10);

         TS_ASSERT_THROWS_ASSERT
            ( pd.get("A"), 
              utilib::propertyDict_error &e,
              TEST_WHAT(e, "PropertyDict::get_impl() const: "
                        "attempt to retrieve nonexistent Property 'A'") );
      }

      source_pd.implicitDeclareIfDNE() = true;
      
      {
         const utilib::PropertyDict &pd = source_pd;
         TS_ASSERT_THROWS_ASSERT
            ( pd.get("A"), 
              utilib::propertyDict_error &e,
              TEST_WHAT(e, "PropertyDict::get_impl() const: "
                        "attempt to retrieve nonexistent Property 'A'") );
      }
   }

   void test_privilegedGet()
   {
      utilib::Privileged_PropertyDict pd;

      pd.declare("a").set_readonly();
      TS_ASSERT( pd.privilegedGet("a").get().empty() );

      pd.privilegedGet("a") = 10;
      TS_ASSERT_EQUALS(pd.privilegedGet("a"), 10);

      utilib::Property p;
      pd.declare("b", p);
      TS_ASSERT_THROWS_ASSERT
         ( pd.privilegedGet("b"), 
           utilib::propertyDict_error &e,
           TEST_WHAT(e, "Privileged_PropertyDict::privilegedGet(): "
                     "Property 'b' is not Privileged") );      
   }

   void test_const_privilegedGet()
   {
      utilib::Privileged_PropertyDict source_pd;

      source_pd.declare("a").set_readonly();
      source_pd.privilegedGet("a") = 10;
      utilib::Property p;
      source_pd.declare("b", p);

      {
         const utilib::Privileged_PropertyDict &pd = source_pd;

         TS_ASSERT_EQUALS(pd.privilegedGet("a"), 10);

         TS_ASSERT_THROWS_ASSERT
            ( pd.privilegedGet("b"), 
              utilib::propertyDict_error &e,
              TEST_WHAT(e, "Privileged_PropertyDict::privilegedGet() const: "
                        "Property 'b' is not Privileged") );      
      }
   }

   void test_indexer()
   {
      utilib::PropertyDict pd;

      int i = 5;
      utilib::Property p(i);
      pd.declare("a", p);

      TS_ASSERT_EQUALS(pd["a"], 5);

      pd.get("a") = 10;
      TS_ASSERT_EQUALS(i, 10);
      TS_ASSERT_EQUALS(pd["a"], 10);

      TS_ASSERT_THROWS_ASSERT
         ( pd["A"],
           utilib::propertyDict_error &e,
           TEST_WHAT(e, "PropertyDict::get_impl(): "
                     "attempt to retrieve nonexistent Property 'A'") );

      pd.implicitDeclareIfDNE() = true;
      TS_ASSERT(pd["A"].get().empty());
   }

   void test_const_indexer()
   {
      utilib::PropertyDict source_pd;
      int i = 5;
      utilib::Property p(i);
      source_pd.declare("a", p);

      {
         const utilib::PropertyDict &pd = source_pd;

         TS_ASSERT_EQUALS(pd["a"], 5);

         source_pd["a"] = 10;
         TS_ASSERT_EQUALS(i, 10);
         TS_ASSERT_EQUALS(pd["a"], 10);

         TS_ASSERT_THROWS_ASSERT
            ( pd["A"], 
              utilib::propertyDict_error &e,
              TEST_WHAT(e, "PropertyDict::get_impl() const: "
                        "attempt to retrieve nonexistent Property 'A'") );
      }

      source_pd.implicitDeclareIfDNE() = true;
      
      {
         const utilib::PropertyDict &pd = source_pd;
         TS_ASSERT_THROWS_ASSERT
            ( pd["A"],
              utilib::propertyDict_error &e,
              TEST_WHAT(e, "PropertyDict::get_impl() const: "
                        "attempt to retrieve nonexistent Property 'A'") );
      }
   }

   void test_iterator()
   {
      utilib::PropertyDict pd;
      pd.declare("x");
      pd.declare("a");
      pd.declare("foo");

      pd["a"] = 5;
      pd["foo"] = 3;
      pd["x"] = 1;

      utilib::PropertyDict::iterator it = pd.begin();
      TS_ASSERT( it != pd.end() );
      TS_ASSERT_EQUALS( it->first(), "a" );
      TS_ASSERT_EQUALS( it->second(), 5 );
      ++it;
      TS_ASSERT( it != pd.end() );
      TS_ASSERT_EQUALS( it->first(), "foo" );
      TS_ASSERT_EQUALS( it->second(), 3 );
      ++it;
      TS_ASSERT( it != pd.end() );
      TS_ASSERT_EQUALS( it->first(), "x" );
      TS_ASSERT_EQUALS( it->second(), 1 );
      ++it;
      TS_ASSERT( it == pd.end() );
      --it;
      TS_ASSERT( it != pd.end() );

      TS_ASSERT_EQUALS( (--it)->first(), "foo" );
      TS_ASSERT_EQUALS( (it--)->first(), "foo" );
      TS_ASSERT_EQUALS( (*it).first(), "a" );

      TS_ASSERT_EQUALS( (++it)->first(), "foo" );
      TS_ASSERT_EQUALS( (it++)->first(), "foo" );
      TS_ASSERT_EQUALS( (*it).first(), "x" );
   }

   void test_set_through_iterator()
   {
      utilib::Privileged_PropertyDict pd;
      int I(0);
      double D(0);
      pd.declare("i", utilib::Privileged_Property(I));
      pd.declare("d", utilib::Privileged_Property(D));

      {
         utilib::PropertyDict::iterator it = pd.find("i");
         TS_ASSERT_DIFFERS(it, pd.end());
         TS_ASSERT_EQUALS(it->second(), 0);
         it->second() = 5;
         TS_ASSERT_EQUALS(it->second(), 5);
         TS_ASSERT_EQUALS(I, 5);
      }
      {
         utilib::PropertyDict::iterator it = pd.find("d");
         TS_ASSERT_DIFFERS(it, pd.end());
         TS_ASSERT_EQUALS(it->second(), 0);
         it->second() = 5.5;
         TS_ASSERT_EQUALS(it->second(), 5.5);
         TS_ASSERT_EQUALS(D, 5.5);
      }
      {
         utilib::PropertyDict::iterator it = pd.find("x");
         TS_ASSERT_EQUALS(it, pd.end());
      }
   }

   void test_privileged_iterator()
   {
      utilib::Privileged_PropertyDict pd;
      pd.declare("x");
      pd.declare("a");
      pd.declare("foo");

      pd["a"] = 5;
      pd["foo"] = 3;
      pd["x"] = 1;

      utilib::Privileged_PropertyDict::privileged_iterator it 
         = pd.privilegedBegin();
      TS_ASSERT( it != pd.privilegedEnd() );
      TS_ASSERT_EQUALS( it->first, "a" );
      TS_ASSERT_EQUALS( it->second->property, 5 );
      ++it;
      TS_ASSERT( it != pd.privilegedEnd() );
      TS_ASSERT_EQUALS( it->first, "foo" );
      TS_ASSERT_EQUALS( it->second->property, 3 );
      ++it;
      TS_ASSERT( it != pd.privilegedEnd() );
      TS_ASSERT_EQUALS( it->first, "x" );
      TS_ASSERT_EQUALS( it->second->property, 1 );
      ++it;
      TS_ASSERT( it == pd.privilegedEnd() );
      --it;
      TS_ASSERT( it != pd.privilegedEnd() );

      TS_ASSERT_EQUALS( (--it)->first, "foo" );
      TS_ASSERT_EQUALS( (it--)->first, "foo" );
      TS_ASSERT_EQUALS( (*it).first, "a" );

      TS_ASSERT_EQUALS( (++it)->first, "foo" );
      TS_ASSERT_EQUALS( (it++)->first, "foo" );
      TS_ASSERT_EQUALS( (*it).first, "x" );
   }

   void test_readOnly()
   {
      int i = 1;
      utilib::Privileged_Property p(i);

      utilib::PropertyDict pd;

      pd.declare("A", p);
      TS_ASSERT_EQUALS(pd["A"], 1);

      p.set_readonly();
      TS_ASSERT_THROWS_ASSERT
         ( pd["A"] = 10, std::runtime_error &e,
           TEST_WHAT( e, "Property::set(): attempt to set "
                        "a Property declared read-only." ) );
      TS_ASSERT_EQUALS(p, 1);
      TS_ASSERT_EQUALS(pd["A"], 1);

      p.unset_readonly();
      TS_ASSERT_THROWS_NOTHING(pd["A"] = 11);
      TS_ASSERT_EQUALS(p, 11);
      TS_ASSERT_EQUALS(pd["A"], 11);
   }

   void test_basic_reference()
   {
#ifdef UTILIB_HAVE_BOOST
      utilib::PropertyDict pd1;
      utilib::PropertyDict pd3;
      utilib::Property p;
      utilib::Property pp;

      pd1.declare("local");
      pd1.declare("promote", p, utilib::Any(), true);
      pd1.declare("reference", pp);
      pd1.declare("z_local");
      pd1.declare("z_promote", p, utilib::Any(), true);
      
      pd3.declare("reference");

      // declare a temporary PropertyDict and form a 3-level chain
      {
         utilib::PropertyDict pd2;
         utilib::Privileged_Property pp2;
         pd2.declare("reference", pp2);
         pd2.declare("my_local");
         
         pd2.reference(pd1);
         pd3.reference(pd2);

         TS_ASSERT( ! pd2.exists("local") );
         TS_ASSERT(   pd2.exists("my_local") );
         TS_ASSERT(   pd2.exists("promote") );
         TS_ASSERT(   pd2.exists("reference") );
         TS_ASSERT( ! pd2.exists("z_local") );
         TS_ASSERT(   pd2.exists("z_promote") );
         
         TS_ASSERT( ! pd2["promote"].readonly() );
         TS_ASSERT(   pd2["reference"].readonly() );
         
         p = 5;
         TS_ASSERT_EQUALS( pd2["promote"], 5 );
         TS_ASSERT_EQUALS( pd3["promote"], 5 );
         
         pp = 15;
         TS_ASSERT_EQUALS( pp2, 15 );
         TS_ASSERT_EQUALS( pd2["reference"], 15 );
         TS_ASSERT_EQUALS( pd3["reference"], 15 );
      }

      // assrt that the chain is now broken (and that reference is
      // independent of pd1).
      TS_ASSERT( ! pd3.exists("promote") );
      TS_ASSERT(   pd3.exists("reference") );
      TS_ASSERT_EQUALS( pd3["reference"], 15 );

      pp = 10;
      TS_ASSERT_EQUALS( pd1["reference"], 10 );
      TS_ASSERT_EQUALS( pd3["reference"], 15 );

#else
      TS_WARN("UTILIB compiled without Boost support.  Test cannot run");
#endif
   }

   void test_chained_reference()
   {
#ifdef UTILIB_HAVE_BOOST
      utilib::PropertyDict pd1;
      utilib::PropertyDict pd2;
      utilib::PropertyDict pd3;

      // construct & test a 3-level replication chain
      pd1.declare("promote", utilib::Property(), utilib::Any(), true);
      pd1.declare("reference");
      pd2.declare("reference");
      pd3.declare("reference");

      pd3.reference(pd2);
      TS_ASSERT( ! pd2["reference"].readonly() );
      TS_ASSERT(   pd3["reference"].readonly() );

      pd2.reference(pd1);
      TS_ASSERT( ! pd1["reference"].readonly() );
      TS_ASSERT(   pd2["reference"].readonly() );
      TS_ASSERT(   pd2.exists("promote") );
      TS_ASSERT(   pd3.exists("promote") );
      TS_ASSERT( ! pd1["promote"].readonly() );
      TS_ASSERT( ! pd2["promote"].readonly() );
      TS_ASSERT( ! pd3["promote"].readonly() );

      // Declaring a promoted property at the inner level should
      // reference throughout the chain
      pd1.declare("promote_2", utilib::Property(), 0, true);
      TS_ASSERT(   pd1.exists("promote_2") );
      TS_ASSERT(   pd2.exists("promote_2") );
      TS_ASSERT(   pd3.exists("promote_2") );
      pd1.erase("promote_2");
      TS_ASSERT( ! pd1.exists("promote_2") );
      TS_ASSERT( ! pd2.exists("promote_2") );
      TS_ASSERT( ! pd3.exists("promote_2") );

      // If I erase in the middle, the original should persist
      pd1.declare("promote_2", utilib::Property(), 0, true);
      pd2.erase("promote_2");
      TS_ASSERT(   pd1.exists("promote_2") );
      TS_ASSERT( ! pd2.exists("promote_2") );
      TS_ASSERT( ! pd3.exists("promote_2") );

      // Recreating the middle is allowable, but it should now be
      // logically decoupled from the original.
      pd2.declare("promote_2", utilib::Property(), 0, true);
      TS_ASSERT(   pd1.exists("promote_2") );
      TS_ASSERT(   pd2.exists("promote_2") );
      TS_ASSERT(   pd3.exists("promote_2") );
      pd1.erase("promote_2");
      TS_ASSERT( ! pd1.exists("promote_2") );
      TS_ASSERT(   pd2.exists("promote_2") );
      TS_ASSERT(   pd3.exists("promote_2") );

      // Recreating the middle is allowable, but it should now be
      // logically decoupled from the original (special test for
      // silently ignoring a missing promoted property in a child
      // PropertyDict)
      pd2.erase("promote_2");
      pd1.declare("promote_2", utilib::Property(), 0, true);
      pd2.erase("promote_2");
      pd3.declare("promote_2", utilib::Property(), 0, true);
      TS_ASSERT(   pd1.exists("promote_2") );
      TS_ASSERT( ! pd2.exists("promote_2") );
      TS_ASSERT(   pd3.exists("promote_2") );
      pd1.erase("promote_2");
      TS_ASSERT( ! pd1.exists("promote_2") );
      TS_ASSERT( ! pd2.exists("promote_2") );
      TS_ASSERT(   pd3.exists("promote_2") );

#else
      TS_WARN("UTILIB compiled without Boost support.  Test cannot run");
#endif
   }

   void test_chained_insertion_error()
   {
#ifdef UTILIB_HAVE_BOOST
      utilib::PropertyDict pd1;
      utilib::PropertyDict pd2;
      utilib::PropertyDict pd3a;
      utilib::PropertyDict pd3b;

      // construct & test a 3-level replication chain
      pd2.reference(pd1);
      pd3a.reference(pd2);
      pd3b.reference(pd2);

      // NB: we test both 3a and 3b because we cannot guarantee which
      // child (3a or 3b) pd2 will try and declare first; HOWEVER, the
      // order will be the same for both trials, so this will
      // effectively test the declare_impl() unroll mechanism.

      pd3b.declare("foo");

      TS_ASSERT_THROWS_ASSERT
         ( pd1.declare("foo", utilib::Property(), 0, true), 
           utilib::propertyDict_error &e,
           TEST_WHAT( e, "PropertyDict::declare(): attempt to declare "
                      "duplicate Property 'foo'" ) );
      TS_ASSERT( ! pd1.exists("foo") );
      TS_ASSERT( ! pd2.exists("foo") );
      TS_ASSERT( ! pd3a.exists("foo") );
      TS_ASSERT(   pd3b.exists("foo") );

      pd3b.erase("foo");
      pd3a.declare("foo");

      TS_ASSERT_THROWS_ASSERT
         ( pd1.declare("foo", utilib::Property(), 0, true), 
           utilib::propertyDict_error &e,
           TEST_WHAT( e, "PropertyDict::declare(): attempt to declare "
                      "duplicate Property 'foo'" ) );
      TS_ASSERT( ! pd1.exists("foo") );
      TS_ASSERT( ! pd2.exists("foo") );
      TS_ASSERT(   pd3a.exists("foo") );
      TS_ASSERT( ! pd3b.exists("foo") );

      pd3a.erase("foo");
      pd1.declare("foo", utilib::Property(), 0, true);
      TS_ASSERT(   pd1.exists("foo") );
      TS_ASSERT(   pd2.exists("foo") );
      TS_ASSERT(   pd3a.exists("foo") );
      TS_ASSERT(   pd3b.exists("foo") );

#else
      TS_WARN("UTILIB compiled without Boost support.  Test cannot run");
#endif
   }

   void test_reference_error()
   {
#ifdef UTILIB_HAVE_BOOST
      utilib::PropertyDict pd1;
      pd1.declare("a_property") = 5;
      pd1.declare("promote", utilib::Property(), 0, true);
      pd1.declare("reference") = 10;

      // Test duplicate replication
      {
         utilib::PropertyDict pd2;
         pd2.reference(pd1);
         TS_ASSERT_THROWS_ASSERT
            ( pd2.reference(pd1), utilib::propertyDict_error &e,
              TEST_WHAT( e, "PropertyDict::reference(): "
                         "duplicate reference to an external PropertyDict" ) );
      }

      // Test invalid replication
      {
         utilib::PropertyDict pd2;
         pd2.declare("reference", utilib::Property());
         TS_ASSERT_THROWS_ASSERT
            ( pd2.reference(pd1), utilib::propertyDict_error &e,
              TEST_WHAT( e, "PropertyDict::reference(): "
                         "non-privileged property, 'reference' cannot "
                         "reference remote value." ) );
         // test the promotion unwinding...
         TS_ASSERT( ! pd2.exists("promote") );
      }

      // Test duplicate reference
      {
         utilib::PropertyDict pd;
         pd.declare("reference");
         utilib::PropertyDict pd2;
         pd2.declare("reference");
         pd2.reference(pd);

         TS_ASSERT_THROWS_ASSERT
            ( pd2.reference(pd1), utilib::propertyDict_error &e,
              TEST_WHAT( e, "PropertyDict::reference(): "
                         "attempt to reference property 'reference', "
                         "which is already connected to a remote value." ) );
      }

      // Test blocking promotion
      {
         utilib::PropertyDict pd2;
         pd2.declare("promote");
         TS_ASSERT_THROWS_ASSERT
            ( pd2.reference(pd1), utilib::propertyDict_error &e,
              TEST_WHAT( e, "PropertyDict::reference(): "
                         "local property 'promote', masks a remote "
                         "promotable property." ) );
      }

      // Test unrolling failed refeences
      {
         utilib::PropertyDict pd2;
         pd2.declare("a_property") = 0;
         pd2.declare("reference") = 0;

         propRef() = utilib::Property();
         propRef() = 5;
         pd2["reference"].validate().connect(&value_equals_propRef);

         TS_ASSERT_THROWS_ASSERT
            ( pd2.reference(pd1), utilib::propertyDict_error &e,
              TEST_WHAT( e, "PropertyDict::reference(): exception caught "
                         "while replicating value of 'reference':") ) ;

         // test the promotion unwinding...
         TS_ASSERT( ! pd2.exists("promote") );
         TS_ASSERT_EQUALS( pd1["a_property"], 5 );
         TS_ASSERT_EQUALS( pd2["a_property"], 5 );
         TS_ASSERT_EQUALS( pd2["reference"], 0 );
         pd1["a_property"] = 0;
         TS_ASSERT_EQUALS( pd2["a_property"], 5 );
      }
#else
      TS_WARN("UTILIB compiled without Boost support.  Test cannot run");
#endif
   }


   void test_dereference()
   {
#ifdef UTILIB_HAVE_BOOST
      utilib::PropertyDict pd1;
      pd1.declare("promote", utilib::Property(), 0, true);
      pd1.declare("reference");

      utilib::Privileged_PropertyDict pd2;
      pd2.declare("reference");

      pd2.reference(pd1);
      TS_ASSERT( pd2.exists("reference") );
      TS_ASSERT( pd2.exists("promote") );

      //std::set<utilib::PropertyDict*> sources = pd2.referenced_sources();
      //TS_ASSERT_EQUALS(sources.size(), 1u);
      //TS_ASSERT_EQUALS(*sources.begin(), &pd1);

      utilib::Privileged_PropertyDict::privileged_iterator it 
         = pd2.privilegedFind("reference");
      TS_ASSERT( it->second->source.connected() );

      pd2.dereference(pd1);

      //sources = pd2.referenced_sources();
      //TS_ASSERT_EQUALS(sources.size(), 0u);
      TS_ASSERT( pd2.exists("reference") );
      TS_ASSERT( ! pd2.exists("promote") );

      TS_ASSERT( ! it->second->source.connected() );

      TS_ASSERT_THROWS_ASSERT
         ( pd2.dereference(pd1), utilib::propertyDict_error &e,
           TEST_WHAT( e, "PropertyDict::dereference(): "
                      "specified source not found in reference_sources()" ) );
      
#else
      TS_WARN("UTILIB compiled without Boost support.  Test cannot run");
#endif
   }


   void test_reference_order()
   {
#ifdef UTILIB_HAVE_BOOST
      utilib::PropertyDict pd;

      pd.declare("a C") = 0;
      pd.declare("b A") = 0;
      pd.declare("c B") = 0;
      pd.declare("d D") = 0;

      propRef() = pd["b A"];

      pd["a C"].validate().connect(&value_equals_propRef);
      pd["c B"].validate().connect(&value_equals_propRef);
      pd["d D"].validate().connect(&value_equals_propRef);

      utilib::PropertyDict pd_ref;
      pd_ref.declare("b A") = 5;
      pd_ref.declare("c B") = 5;
      pd_ref.declare("d D") = 5;

      pd.reference(pd_ref);
      TS_ASSERT_EQUALS(pd["a C"], 0);
      TS_ASSERT_EQUALS(pd["b A"], 5);
      TS_ASSERT_EQUALS(pd["c B"], 5);
      TS_ASSERT_EQUALS(pd["d D"], 5);

      pd.dereference(pd_ref);
      pd_ref.erase("c B");
      pd_ref.declare("a C") = 10;
      pd_ref["b A"] = 10;
      pd_ref["d D"] = 10;
      TS_ASSERT_THROWS_ASSERT
         ( pd.reference(pd_ref), utilib::propertyDict_error &e,
           TEST_WHAT( e, "PropertyDict::reference(): exception caught "
                      "while replicating value of 'a C':" ) );

#else
      TS_WARN("UTILIB compiled without Boost support.  Test cannot run");
#endif
   }


   void test_write_values()
   {
      utilib::PropertyDict pd;
      pd.declare("A") = 1;
      pd.declare("CC") = 2;
      pd.declare("BBB") = 3;
      pd.declare("DDDD") = 4;

      std::stringstream ref;
      ref << "   A   : 1" << std::endl
          << "   BBB : 3" << std::endl
          << "   CC  : 2" << std::endl
          << "   DDDD: 4" << std::endl;

      std::stringstream ss;
      pd.write(utilib::PropertyDict::ValueWriter(ss));
      TS_ASSERT_EQUALS(ss.str(), ref.str());
   }


   void test_write_description()
   {
      utilib::PropertyDict pd;
      pd.declare("A", "An 'A' property") = 1;
      pd.declare("CC", "A 'C' property") = 2;
      pd.declare("BBB") = 3;
      pd.declare("DDDD") = 4;

      std::stringstream ref;
      ref << "   A   : An 'A' property" << std::endl
          << "   BBB : " << std::endl
          << "   CC  : A 'C' property" << std::endl
          << "   DDDD: " << std::endl;

      std::stringstream ss;
      pd.write(utilib::PropertyDict::DescriptionWriter(ss));
      TS_ASSERT_EQUALS(ss.str(), ref.str());
   }


   static utilib::Property& propRef()
   {
      static utilib::Property x;
      return x;
   }

   static bool value_equals_propRef( const utilib::ReadOnly_Property&, 
                                     const utilib::Any &new_value )
   {
      return propRef() == new_value;
   }
};

} // namespace utilib
