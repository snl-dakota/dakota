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

#include <utilib/PropertyDict_YamlPrinter.h>

using std::endl;

namespace utilib {

namespace unittest { class Test_PropertyDict_YamlPrinter; }
class utilib::unittest::Test_PropertyDict_YamlPrinter \
   : public CxxTest::TestSuite
{
public:
   std::stringstream ss;
   PropertyDict_YamlPrinter printer;

   void setUp()
   {
      ss.str("");
      printer = PropertyDict_YamlPrinter();
   }

   void test_empty()
   {
      std::stringstream ref;
      ref << "---" << endl << "..." << endl;

      PropertyDict pd;
      printer.print(ss, pd);
      TS_ASSERT_EQUALS(ss.str(), ref.str());
   }

   void test_title()
   {
      std::stringstream ref;
      ref << "--- # A title" << endl << "..." << endl;

      PropertyDict pd;
      printer.print(ss, pd, "A title");
      TS_ASSERT_EQUALS(ss.str(), ref.str());
   }

   void test_scalars()
   {
      std::stringstream ref;
      ref << "---" << endl
          << "bar: 3.14159" << endl
          << "baz: 42" << endl
          << "foo: 5" << endl
          << "..." << endl;

      PropertyDict pd;
      pd.declare("foo") = 5;
      pd.declare("bar") = 3.14159;
      pd.declare("baz") = 42u;
      printer.print(ss, pd);
      TS_ASSERT_EQUALS(ss.str(), ref.str());
   }

   void test_doubles()
   {
      // Special case until we figure out extended-precision printing
      std::stringstream ref;
      ref << "---" << endl
          << "bar: 3.141592" << endl
          << "..." << endl;

      PropertyDict pd;
      pd.declare("bar") = 3.141592;
      printer.print(ss, pd);
      TS_ASSERT_EQUALS(ss.str(), ref.str());
   }

   void test_strings()
   {
      std::stringstream ref;
      ref << "---" << endl
          << "bar: <" << endl
          << "   A " << endl << endl
          << "   multiline string" << endl << endl
          << "   with a very, very, very, very, very, very, very, very, very, very, very, " << endl << "   very, very, long line" << endl << endl
          << "   " << endl << endl
          << "   and a blank line" << endl
          << endl
          << "baz: Hello World" << endl
          << "foo: |" << endl
          << "   A relatively short" << endl
          << "   multiline string" << endl
          << "   " << endl
          << "   with a blank line" << endl
          << "str: " << endl
          << "..." << endl;

      PropertyDict pd;
      pd.declare("str") = string();
      pd.declare("foo") = 
         string("A relatively short\n"
                "multiline string\n"
                "\n"
                "with a blank line");
      pd.declare("bar") =  
         string("A \n"
                "multiline string\n"
                "with a very, very, very, very, very, very, very, very, very, very, very, very, very, long line\n"
                "\n"
                "and a blank line");
      pd.declare("baz") = string("Hello World");
      printer.print(ss, pd);
      TS_ASSERT_EQUALS(ss.str(), ref.str());
   }

   void test_lists()
   {
      std::stringstream ref;
      ref << "---" << endl
          << "bar: " << endl 
          << "   - 5" << endl
          << "baz: " << endl 
          << "   - 5" << endl
          << "   - 3.1415" << endl
          << "foo: " << endl 
          << "nested: " << endl
          << "   - " << endl
          << "     - 1" << endl
          << "     - 2" << endl
          << "   - " << endl
          << "     - 3" << endl
          << "     - 4" << endl
          << "..." << endl;

      PropertyList pl;
      PropertyDict pd;
      pd.declare("foo") = pl;
      pl.push_back(Property(Property::Bind<int>(5)));
      pd.declare("bar") = pl;
      pl.push_back(Property(Property::Bind<double>(3.1415)));
      pd.declare("baz") = pl;

      pl.clear();
      PropertyList l;
      l.push_back(Property(Property::Bind<int>(1)));
      l.push_back(Property(Property::Bind<int>(2)));
      pl.push_back(Property(Property::Bind<PropertyList>(l)));
      l.clear();
      l.push_back(Property(Property::Bind<int>(3)));
      l.push_back(Property(Property::Bind<int>(4)));
      pl.push_back(Property(Property::Bind<PropertyList>(l)));
      pd.declare("nested") = pl;

      printer.print(ss, pd);
      TS_ASSERT_EQUALS(ss.str(), ref.str());
   }

   void test_dicts()
   {
      // Special case until we figure out extended-precision printing
      std::stringstream ref;
      ref << "---" << endl
          << "bar: " << endl
          << "   foo: Hello World" << endl
          << "   zzz: |" << endl
          << "      And a" << endl
          << "      multiline string" << endl
          << "baz: " << endl
          << "foo: " << endl
          << "   bar: 3.14159" << endl
          << "   baz: 42" << endl
          << "   foo: 5" << endl
          << "..." << endl;

      PropertyDict pd1;
      pd1.declare("foo") = 5;
      pd1.declare("bar") = 3.14159;
      pd1.declare("baz") = 42u;

      PropertyDict pd2;
      pd2.declare("foo") = string("Hello World");
      pd2.declare("zzz") = string("And a\nmultiline string");

      PropertyDict pd3;

      PropertyDict pd;
      pd.declare("foo", Property(Property::Bind<PropertyDict>(pd1)));
      pd.declare("bar") = pd2;
      pd.declare("baz") = pd3;
      printer.print(ss, pd);
      TS_ASSERT_EQUALS(ss.str(), ref.str());
   }

   void test_complex()
   {
      // Special case until we figure out extended-precision printing
      std::stringstream ref;
      ref << "---" << endl
          << "bar: " << endl
          << "   - Hello World" << endl
          << "   - |" << endl
          << "     And a" << endl
          << "     multiline string" << endl
          << "foo: " << endl
          << "   - " << endl
          << "     bar: 3.14159" << endl
          << "     baz: 42" << endl
          << "     foo: 5" << endl
          << "   - " << endl
          << "     bar: 1" << endl
          << "     baz: 2" << endl
          << "     foo: 3" << endl
          << "..." << endl;

      PropertyList l;
      l.push_back(Property(Property::Bind<string>("Hello World")));
      l.push_back(Property(Property::Bind<string>("And a\nmultiline string")));

      PropertyDict pd;
      pd.declare("bar") = l;
      l.clear();

      PropertyDict pd1;
      pd1.declare("foo") = 5;
      pd1.declare("bar") = 3.14159;
      pd1.declare("baz") = 42u;
      l.push_back(Property(Property::Bind<PropertyDict>(pd1)));

      pd1 = PropertyDict();
      pd1.declare("foo") = 3;
      pd1.declare("bar") = 1;
      pd1.declare("baz") = 2;
      l.push_back(Property(pd1));
      
      pd.declare("foo") = l;
      printer.print(ss, pd);
      TS_ASSERT_EQUALS(ss.str(), ref.str());
   }
};

} // namespace utilib
