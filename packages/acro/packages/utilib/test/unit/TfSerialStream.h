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

#include <utilib/fSerialStream.h>

namespace {

class A
{
public:
   A()
      : a(0), b(0), c()
   {}
   A(int x, float y)
      : a(x), b(y), c(x,y)
   {}
   void print(std::ostream& os) const
   {
      os << "{" << a << ", " << b << ", " << c << "}";
   }
   bool operator==(const A& rhs) const
   {
      return ( a == rhs.a ) && ( b == rhs.b ) && ( c == rhs.c );
   }

private:
   static int serializer(utilib::SerialObject::elementList_t& serial, 
                         utilib::Any& data, 
                         bool serialize)
   {
      A& me = const_cast<A&>(data.expose<A>());
      utilib::serial_transform(serial, me.a, serialize);
      utilib::serial_transform(serial, me.b, serialize);
      utilib::serial_transform(serial, me.c, serialize);
      return 0;
   }

private:
   static const int serializer_registered;

   int a;
   float b;
   std::list<double> c;
};

// Register the serializer for class A
const int A::serializer_registered
= utilib::Serializer().register_serializer<A>("test:fss:A", A::serializer);

} // namespace (local)

namespace CxxTest {

/// Printing utility for use in CxxTest unit tests
CXXTEST_TEMPLATE_INSTANTIATION
class ValueTraits<A>
{
public:
   ValueTraits( const A &a )
   { 
      std::ostringstream ss;
      a.print(ss);
      str = ss.str();
   }

   const char* asString() const
   { return str.c_str(); }

private:
   std::string str;
};

} // namespace CxxTest



namespace utilib {
namespace unittest { class Test_fSerialStream; }
class utilib::unittest::Test_fSerialStream : public CxxTest::TestSuite
{
public:

   void test_basic_pod()
   {
      int    i  = 5;
      double d = 3.1415;
      char   c  = 'A';
      std::string s = "The quick brown fox";

      // implicitly, ofSerialStream opens with trunc
      ofSerialStream oss("test.bss");
      oss << utilib::Serialize(i);
      oss << utilib::Serialize(d);
      oss << utilib::Serialize(c);
      oss << utilib::Serialize(s);
      oss.flush();

      utilib::SerialObject so;

      ifSerialStream iss("test.bss");
      iss >> so;
      TS_ASSERT(iss.good());
      TS_ASSERT_EQUALS(i, Deserialize(so).expose<int>());
      iss >> so;
      TS_ASSERT(iss.good());
      TS_ASSERT_EQUALS(d, Deserialize(so).expose<double>());
      iss >> so;
      TS_ASSERT(iss.good());
      TS_ASSERT_EQUALS(c, Deserialize(so).expose<char>());
      iss >> so;
      TS_ASSERT(iss.good());
      TS_ASSERT_EQUALS(s, Deserialize(so).expose<std::string>());
   }

   void test_chained_pod()
   {
      int    i  = 5;
      double d = 3.1415;
      char   c  = 'A';
      {
         // implicitly, ofSerialStream opens with trunc
         ofSerialStream oss("test.bss");
         oss << c << d << i;
         // no flush needed since the oss is falling out of scope
      }
      
      int i2 = 0;
      double d2 = 0;
      char c2 = 0;

      {
         ifSerialStream iss("test.bss");
         iss >> c2 >> d2 >> i2;
         TS_ASSERT(iss.good());
      }
      TS_ASSERT_EQUALS(i, i2);
      TS_ASSERT_EQUALS(d, d2);
      TS_ASSERT_EQUALS(c, c2);
   }


   void test_basic_object()
   {
      A a(2, 3.1415);
      A a2;
      utilib::Any any;
      utilib::Any any2 = a;

      // implicitly, ofSerialStream opens with trunc
      ofSerialStream oss("test.bss");
      oss << a << any;
      oss.flush();
      ifSerialStream iss("test.bss");
      iss >> a2 >> any2;
      TS_ASSERT(iss.good());
      TS_ASSERT_EQUALS(a, a2);
      TS_ASSERT_EQUALS(any, any2);
   }


   void test_singleBuffer_WriteThenRead()
   {
      int    i  = 5;
      double d = 3.1415;
      char   c  = 'A';

      int i2 = 0;
      double d2 = 0;
      char c2 = 0;

      fSerialStream ss("test.bss", std::ios::trunc);
      ss << i << d << c;
      ss >> i2 >> d2 >> c2;

      TS_ASSERT(ss.good());
      TS_ASSERT_EQUALS(i, i2);
      TS_ASSERT_EQUALS(d, d2);
      TS_ASSERT_EQUALS(c, c2);
   }

   void test_singleBuffer_WriteAndRead()
   {
      int    i  = 5;
      double d = 3.1415;
      char   c  = 'A';
      std::string s = "The quick brown fox";

      int i2 = 0;
      double d2 = 0;
      char c2 = 0;
      std::string s2 = "error";

      fSerialStream ss("test.bss", std::ios::trunc);
      ss << i << d;
      ss >> i2;
      ss << c;
      ss >> d2 >> c2;
      ss << s;
      ss >> s2;

      TS_ASSERT(ss.good());
      TS_ASSERT_EQUALS(i, i2);
      TS_ASSERT_EQUALS(d, d2);
      TS_ASSERT_EQUALS(c, c2);
      TS_ASSERT_EQUALS(s, s2);
   }

   void test_outputAppend()
   {
      int    i  = 5;
      double d = 3.1415;
      char   c  = 'A';

      {
         ofSerialStream oss("test.bss");
         oss << d;
      }
      {
         ofSerialStream oss("test.bss", std::ios::app);
         oss << c;
      }
      {
         ofSerialStream oss("test.bss", std::ios::app);
         oss << i;
      }

      int i2 = 0;
      double d2 = 0;
      char c2 = 0;

      ifSerialStream iss("test.bss");
      iss >> d2 >> c2 >> i2;

      TS_ASSERT(iss.good());
      TS_ASSERT_EQUALS(i, i2);
      TS_ASSERT_EQUALS(d, d2);
      TS_ASSERT_EQUALS(c, c2);

      utilib::Any a;
      iss >> a;
      TS_ASSERT(a.empty());
      TS_ASSERT(!iss.good());
      TS_ASSERT(!iss.bad());
      TS_ASSERT(iss.fail());
      TS_ASSERT(iss.eof());
   }

   void test_ioAppend()
   {
      int    i  = 5;
      double d = 3.1415;
      char   c  = 'A';

      {
         fSerialStream ss1("test.bss", std::ios::trunc);
         ss1 << i;
      }
      {
         fSerialStream ss1("test.bss");
         ss1 << d;
      }
      fSerialStream ss("test.bss");
      ss << c;

      int i2 = 0;
      double d2 = 0;
      char c2 = 0;

      ss >> i2 >> d2 >> c2;

      TS_ASSERT(ss.good());
      TS_ASSERT_EQUALS(i, i2);
      TS_ASSERT_EQUALS(d, d2);
      TS_ASSERT_EQUALS(c, c2);

      utilib::Any a;
      ss >> a;
      TS_ASSERT(a.empty());
      TS_ASSERT(!ss.good());
      TS_ASSERT(!ss.bad());
      TS_ASSERT(ss.fail());
      TS_ASSERT(ss.eof());
   }

   void test_ate()
   {
      std::fstream f("test.bss", std::ios::out | std::ios::trunc);
      f << "Some non-UBSS garbage";
      f.close();

      fSerialStream ss("test.bss", std::ios::ate);
      
      int    i  = 5;
      double d = 3.1415;
      char   c  = 'A';

      int i2 = 0;
      double d2 = 0;
      char c2 = 0;

      ss << i << d << c;
      ss >> i2 >> d2 >> c2;

      TS_ASSERT(ss.good());
      TS_ASSERT(!ss.bad());
      TS_ASSERT(!ss.fail());
      TS_ASSERT(!ss.eof());

      TS_ASSERT_EQUALS(i, i2);
      TS_ASSERT_EQUALS(d, d2);
      TS_ASSERT_EQUALS(c, c2);
   }

   void test_delayed_open_chained_pod()
   {
      ofSerialStream oss;
      ifSerialStream iss;

      int    i  = 5;
      double d = 3.1415;
      char   c  = 'A';
      oss.open("test.bss");
      oss << c << d << i;
      oss.close();
      
      int i2 = 0;
      double d2 = 0;
      char c2 = 0;

      iss.open("test.bss");
      iss >> c2 >> d2 >> i2;
      TS_ASSERT(iss.good());
      iss.close();

      TS_ASSERT_EQUALS(i, i2);
      TS_ASSERT_EQUALS(d, d2);
      TS_ASSERT_EQUALS(c, c2);
   }

   void test_delayed_open_outputAppend()
   {
      int    i  = 5;
      double d = 3.1415;
      char   c  = 'A';

      ofSerialStream oss("test.bss");
      oss << d;

      oss.open("test.bss", std::ios::app);
      oss << c;

      oss.open("test.bss", std::ios::app);
      oss << i;
      oss.close();

      int i2 = 0;
      double d2 = 0;
      char c2 = 0;

      ifSerialStream iss("test.bss");
      iss >> d2 >> c2 >> i2;

      TS_ASSERT(iss.good());
      TS_ASSERT_EQUALS(i, i2);
      TS_ASSERT_EQUALS(d, d2);
      TS_ASSERT_EQUALS(c, c2);

      // test implicit close on open...
      d2 = i2 = c2 = 0;
      iss.open("test.bss");
      iss >> d2 >> c2 >> i2;

      TS_ASSERT(iss.good());
      TS_ASSERT_EQUALS(i, i2);
      TS_ASSERT_EQUALS(d, d2);
      TS_ASSERT_EQUALS(c, c2);

      utilib::Any a;
      iss >> a;
      TS_ASSERT(a.empty());
      TS_ASSERT(!iss.good());
      TS_ASSERT(!iss.bad());
      TS_ASSERT(iss.fail());
      TS_ASSERT(iss.eof());
   }

   void test_delayed_open_ioAppend()
   {
      int    i  = 5;
      double d = 3.1415;
      char   c  = 'A';

      fSerialStream ss("test.bss", std::ios::trunc);
      ss << i;

      ss.open("test.bss");
      ss << d;

      ss.open("test.bss");
      ss << c;

      int i2 = 0;
      double d2 = 0;
      char c2 = 0;

      ss >> i2 >> d2 >> c2;

      TS_ASSERT(ss.good());
      TS_ASSERT_EQUALS(i, i2);
      TS_ASSERT_EQUALS(d, d2);
      TS_ASSERT_EQUALS(c, c2);

      utilib::Any a;
      ss >> a;
      TS_ASSERT(a.empty());
      TS_ASSERT(!ss.good());
      TS_ASSERT(!ss.bad());
      TS_ASSERT(ss.fail());
      TS_ASSERT(ss.eof());
   }

   void test_delayed_open_ate()
   {
      std::fstream f("test.bss", std::ios::out | std::ios::trunc);
      f << "Some non-UBSS garbage";
      f.close();

      fSerialStream ss;
      ss.open("test.bss", std::ios::ate);
      
      int    i  = 5;
      double d = 3.1415;
      char   c  = 'A';

      int i2 = 0;
      double d2 = 0;
      char c2 = 0;

      ss << i << d << c;
      ss >> i2 >> d2 >> c2;

      TS_ASSERT(ss.good());
      TS_ASSERT(!ss.bad());
      TS_ASSERT(!ss.fail());
      TS_ASSERT(!ss.eof());

      TS_ASSERT_EQUALS(i, i2);
      TS_ASSERT_EQUALS(d, d2);
      TS_ASSERT_EQUALS(c, c2);
   }

   void test_failed_open()
   {
      ifSerialStream iss;
      iss.open("path/to/bogus/file.bss");
      TS_ASSERT(!iss.good());
      TS_ASSERT(!iss.bad());
      TS_ASSERT(iss.fail());
      TS_ASSERT(!iss.eof());

      ofSerialStream oss;
      oss.open("path/to/bogus/file.bss");
      TS_ASSERT(!oss.good());
      TS_ASSERT(!oss.bad());
      TS_ASSERT(oss.fail());
      TS_ASSERT(!oss.eof());

      fSerialStream ss;
      ss.open("path/to/bogus/file.bss");
      TS_ASSERT(!ss.good());
      TS_ASSERT(!ss.bad());
      TS_ASSERT(ss.fail());
      TS_ASSERT(!ss.eof());
   }

   void test_failed_close()
   {
      ifSerialStream iss;
      TS_ASSERT(iss.good());
      TS_ASSERT(!iss.bad());
      TS_ASSERT(!iss.fail());
      TS_ASSERT(!iss.eof());
      iss.close();
      TS_ASSERT(!iss.good());
      TS_ASSERT(!iss.bad());
      TS_ASSERT(iss.fail());
      TS_ASSERT(!iss.eof());

      ofSerialStream oss;
      TS_ASSERT(oss.good());
      TS_ASSERT(!oss.bad());
      TS_ASSERT(!oss.fail());
      TS_ASSERT(!oss.eof());
      oss.close();
      TS_ASSERT(!oss.good());
      TS_ASSERT(!oss.bad());
      TS_ASSERT(oss.fail());
      TS_ASSERT(!oss.eof());

      fSerialStream ss;
      TS_ASSERT(ss.good());
      TS_ASSERT(!ss.bad());
      TS_ASSERT(!ss.fail());
      TS_ASSERT(!ss.eof());
      ss.close();
      TS_ASSERT(!ss.good());
      TS_ASSERT(!ss.bad());
      TS_ASSERT(ss.fail());
      TS_ASSERT(!ss.eof());
   }
};

} // namespace utilib
