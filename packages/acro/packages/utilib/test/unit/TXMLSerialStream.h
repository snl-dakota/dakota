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

#include <utilib/fXMLSerialStream.h>
#include <utilib/OStreamTee.h>

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

   bool approx_equals(const A& rhs, double tol) const
   {
      if (a != rhs.a) return false;
      if (fabs(b-rhs.b) > tol) return false;

      std::list<double>::const_iterator curr = c.begin();
      std::list<double>::const_iterator end = c.end();
      std::list<double>::const_iterator rhs_curr = rhs.c.begin();
      while (curr != end) {
        if (fabs(*curr-*rhs_curr) > tol) return false;
        curr++;
        rhs_curr++;
      }
      return true;
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
= utilib::Serializer().register_serializer<A>("test:xss:A", A::serializer);

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

namespace unittest { class Test_fXMLSerialStream; }
class utilib::unittest::Test_fXMLSerialStream : public CxxTest::TestSuite
{
   void file2str(std::string fname, std::string &data)
   {
      std::string line;
      std::ifstream f(fname.c_str());
      data.clear();
      getline(f, line);
      while ( f )
      {
         data += line;
         getline(f, line);
      }
      f.close();
   }

public:

   void test_basic_pod()
   {
      int    i  = 5;
      double d = 3.1415;
      char   c  = 'A';
      std::string s = "The quick brown fox";

      // implicitly, ofXMLSerialStream opens with trunc
      ofXMLSerialStream oss("test.xss");
      oss << utilib::Serialize(i);
      oss << utilib::Serialize(d);
      oss << utilib::Serialize(c);
      oss << utilib::Serialize(s);
      oss.flush();

      utilib::SerialObject so;

      ifXMLSerialStream iss("test.xss");
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
         // implicitly, ofXMLSerialStream opens with trunc
         ofXMLSerialStream oss("test.xss");
         oss << c << d << i;
         // no flush needed since the oss is falling out of scope
      }
      
      int i2 = 0;
      double d2 = 0;
      char c2 = 0;

      {
         ifXMLSerialStream iss("test.xss");
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

      // implicitly, ofXMLSerialStream opens with trunc
      ofXMLSerialStream oss("test.xss");
      oss << a << any;
      oss.flush();
      ifXMLSerialStream iss("test.xss");
      iss >> a2 >> any2;
      TS_ASSERT(iss.good());
      //TS_ASSERT(a.approx_equals(a2,1e-8));
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

      fXMLSerialStream ss( "test.xss", utilib::XML::FormatFlags(0), 
                           std::ios::trunc );
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

      fXMLSerialStream ss( "test.xss", utilib::XML::FormatFlags(0), 
                           std::ios::trunc );
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
         ofXMLSerialStream oss("test.xss");
         oss << d;
      }
      {
         ofXMLSerialStream oss( "test.xss", utilib::XML::FormatFlags(0),
                                std::ios::app );
         oss << c;
      }
      {
         ofXMLSerialStream oss( "test.xss", utilib::XML::FormatFlags(0), 
                                std::ios::app );
         oss << i;
      }

      int i2 = 0;
      double d2 = 0;
      char c2 = 0;

      ifXMLSerialStream iss("test.xss");
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
         fXMLSerialStream ss1( "test.xss", utilib::XML::FormatFlags(0), 
                               std::ios::trunc );
         ss1 << i;
      }
      {
         fXMLSerialStream ss1("test.xss");
         ss1 << d;
      }
      fXMLSerialStream ss("test.xss");
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
      std::fstream f( "test.xss", std::ios::out | std::ios::trunc );
      f << "Some non-UXSS garbage";
      f.close();

      fXMLSerialStream ss( "test.xss", utilib::XML::FormatFlags(0), 
                           std::ios::ate );
      
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
      ofXMLSerialStream oss;
      ifXMLSerialStream iss;

      int    i  = 5;
      double d = 3.1415;
      char   c  = 'A';
      oss.open("test.xss");
      oss << c << d << i;
      oss.close();
      
      int i2 = 0;
      double d2 = 0;
      char c2 = 0;

      iss.open("test.xss");
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

      ofXMLSerialStream oss("test.xss");
      oss << d;

      oss.open("test.xss", utilib::XML::FormatFlags(0), std::ios::app);
      oss << c;

      oss.open("test.xss", utilib::XML::FormatFlags(0), std::ios::app);
      oss << i;
      oss.close();

      int i2 = 0;
      double d2 = 0;
      char c2 = 0;

      ifXMLSerialStream iss("test.xss");
      iss >> d2 >> c2 >> i2;

      TS_ASSERT(iss.good());
      TS_ASSERT_EQUALS(i, i2);
      TS_ASSERT_EQUALS(d, d2);
      TS_ASSERT_EQUALS(c, c2);

      // test implicit close on open...
      d2 = i2 = c2 = 0;
      iss.open("test.xss");
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

      fXMLSerialStream ss( "test.xss", utilib::XML::FormatFlags(0), 
                           std::ios::trunc );
      ss << i;

      ss.open("test.xss");
      ss << d;

      ss.open("test.xss");
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
      std::fstream f( "test.xss", std::ios::out | std::ios::trunc );
      f << "Some non-UXSS garbage";
      f.close();

      fXMLSerialStream ss;
      ss.open("test.xss", utilib::XML::FormatFlags(0), std::ios::ate);
      
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
      ifXMLSerialStream iss;
      iss.open("path/to/bogus/file.xss");
      TS_ASSERT(!iss.good());
      TS_ASSERT(!iss.bad());
      TS_ASSERT(iss.fail());
      TS_ASSERT(!iss.eof());

      ofXMLSerialStream oss;
      oss.open("path/to/bogus/file.xss");
      TS_ASSERT(!oss.good());
      TS_ASSERT(!oss.bad());
      TS_ASSERT(oss.fail());
      TS_ASSERT(!oss.eof());

      fXMLSerialStream ss;
      ss.open("path/to/bogus/file.xss");
      TS_ASSERT(!ss.good());
      TS_ASSERT(!ss.bad());
      TS_ASSERT(ss.fail());
      TS_ASSERT(!ss.eof());

      // test that you cannot read an object from an uninitialized
      // (i.e. unopened) stream:
      utilib::Any a;
      TS_ASSERT_THROWS_ASSERT
         ( ss >> a, utilib::xmlserialstream_error &e,
           TEST_WHAT(e, "iXMLSerialStream::get_object() "
                     "called on an uninitialized stream.") );
   }

   void test_failed_close()
   {
      ifXMLSerialStream iss;
      TS_ASSERT(iss.good());
      TS_ASSERT(!iss.bad());
      TS_ASSERT(!iss.fail());
      TS_ASSERT(!iss.eof());
      iss.close();
      TS_ASSERT(!iss.good());
      TS_ASSERT(!iss.bad());
      TS_ASSERT(iss.fail());
      TS_ASSERT(!iss.eof());

      ofXMLSerialStream oss;
      TS_ASSERT(oss.good());
      TS_ASSERT(!oss.bad());
      TS_ASSERT(!oss.fail());
      TS_ASSERT(!oss.eof());
      oss.close();
      TS_ASSERT(!oss.good());
      // NB: bad because of the failed write of the end-of-main-element tag
      TS_ASSERT(oss.bad());
      TS_ASSERT(oss.fail());
      TS_ASSERT(!oss.eof());

      fXMLSerialStream ss;
      TS_ASSERT(ss.good());
      TS_ASSERT(!ss.bad());
      TS_ASSERT(!ss.fail());
      TS_ASSERT(!ss.eof());
      ss.close();
      TS_ASSERT(!ss.good());
      // NB: bad because of the failed write of the end-of-main-element tag
      TS_ASSERT(ss.bad());
      TS_ASSERT(ss.fail());
      TS_ASSERT(!ss.eof());
   }


   void test_format_flags()
   {
      std::string s, line;

      int    i  = 5;
      double d = 3.1415;
      char   c  = 'A';
      A a(2, 3.1415);
      A a2;

      int i2 = 0;
      double d2 = 0;
      char c2 = 0;
      utilib::Any any;
      utilib::Any any2 = a;

      // indention suppression
      fXMLSerialStream ss( "test.xss", utilib::XML::noindent,
                           std::ios::trunc );
      ss << c << d << i << a << any;
      ss >> c2 >> d2 >> i2 >> a2 >> any2;

      TS_ASSERT(ss.good());
      TS_ASSERT_EQUALS(i, i2);
      TS_ASSERT_EQUALS(d, d2);
      TS_ASSERT_EQUALS(c, c2);
      //TS_ASSERT(a.approx_equals(a2,1e-8));
      TS_ASSERT_EQUALS(a, a2);
      TS_ASSERT_EQUALS(any, any2);

      file2str("test.xss", s);
      TS_ASSERT_EQUALS(s.find("  "), std::string::npos);
      TS_ASSERT_EQUALS(s.find("<TypeDef "), std::string::npos);      
      TS_ASSERT_EQUALS(s.find(" format=\"hex\" "), std::string::npos);      

      // type ids (not names)
      d2 = i2 = c2 = 0;
      a2 = A();
      any2 = a;
      ss.open( "test.xss", utilib::XML::typeids, std::ios::trunc );
      ss << c << d << i << a << any;
      ss >> c2 >> d2 >> i2 >> a2 >> any2;

      TS_ASSERT(ss.good());
      TS_ASSERT_EQUALS(i, i2);
      TS_ASSERT_EQUALS(d, d2);
      TS_ASSERT_EQUALS(c, c2);
      //TS_ASSERT(a.approx_equals(a2,1e-8));
      TS_ASSERT_EQUALS(a, a2);
      TS_ASSERT_EQUALS(any, any2);

      file2str("test.xss", s);
      TS_ASSERT_DIFFERS(s.find("  "), std::string::npos);      
      TS_ASSERT_DIFFERS(s.find("<TypeDef "), std::string::npos);      
      TS_ASSERT_EQUALS(s.find(" format=\"hex\" "), std::string::npos);      

      // force binary format
      d2 = i2 = c2 = 0;
      a2 = A();
      any2 = a;
      ss.open( "test.xss", utilib::XML::binary, std::ios::trunc );
      ss << c << d << i << a << any;
      ss >> c2 >> d2 >> i2 >> a2 >> any2;

      TS_ASSERT(ss.good());
      TS_ASSERT_EQUALS(i, i2);
      TS_ASSERT_EQUALS(d, d2);
      TS_ASSERT_EQUALS(c, c2);
      TS_ASSERT_EQUALS(a, a2);
      TS_ASSERT_EQUALS(any, any2);

      file2str("test.xss", s);
      TS_ASSERT_DIFFERS(s.find("  "), std::string::npos);      
      TS_ASSERT_EQUALS(s.find("<TypeDef "), std::string::npos);      
      TS_ASSERT_DIFFERS(s.find(" format=\"hex\""), std::string::npos);      
   }
};

namespace unittest { class Test_XMLSerialStream; }
class utilib::unittest::Test_XMLSerialStream : public CxxTest::TestSuite
{
public:

   void test_main_element()
   {
      std::ofstream f("test.xss", std::ios::out | std::ios::trunc);
      f << "<badTag></badTag>";
      f.close();

      utilib::Any a;
      ifXMLSerialStream ss("test.xss");
      TS_ASSERT_THROWS_ASSERT
         ( ss >> a, utilib::xmlserialstream_error &e,
           TEST_WHAT(e, "iXMLSerialStream::ParserImplementation: "
                     "File begins with an invalid element (badTag)"));

      f.open("test.xss", std::ios::out | std::ios::trunc);
      f << "<UtilibSerialStream></UtilibSerialStream>";
      f.close();

      ss.open("test.xss");
      TS_ASSERT(ss.good());
      TS_ASSERT_THROWS_ASSERT
         ( ss >> a, utilib::xmlserialstream_error &e,
           TEST_WHAT(e, "iXMLSerialStream::ParserImplementation::"
                     "start_element(): inconsistent file version "
                     "(file=0, parser=1)"));

      f.open("test.xss", std::ios::out | std::ios::trunc);
      f << "<UtilibSerialStream version=\"1\"></UtilibSerialStream>";
      f.close();

      ss.open("test.xss");
      TS_ASSERT(ss.good());
      TS_ASSERT_THROWS_ASSERT
         ( ss >> a, utilib::xmlserialstream_error &e,
           TEST_WHAT(e, "iXMLSerialStream::ParserImplementation::"
                     "start_element(): endian marker not found.") );

      f.open("test.xss", std::ios::out | std::ios::trunc);
      f << "<UtilibSerialStream version=\"1\" endian=\"AG \">"
        << "</UtilibSerialStream>";
      f.close();

      ss.open("test.xss");
      TS_ASSERT(ss.good());
      TS_ASSERT_THROWS_ASSERT
         ( ss >> a, utilib::xmlserialstream_error &e,
           TEST_WHAT(e, "iXMLSerialStream::ParserImplementation::"
                     "start_element(): invalid endian marker character (G)."));

      f.open("test.xss", std::ios::out | std::ios::trunc);
      f << "<UtilibSerialStream version=\"1\" endian=\"00\">"
        << "</UtilibSerialStream>";
      f.close();

      ss.open("test.xss");
      TS_ASSERT(ss.good());
      TS_ASSERT_THROWS_ASSERT
         ( ss >> a, utilib::xmlserialstream_error &e,
           TEST_WHAT(e, "iXMLSerialStream::ParserImplementation::"
                     "start_element(): endian marker not valid."));
   }


   void test_bad_element()
   {
      std::ofstream f("test.xss", std::ios::trunc);
      f << "<UtilibSerialStream version=\"1\" endian=\"" << std::hex 
        << static_cast<int>(utilib::Serialization_Manager::Endian) << "\">"
        << "<badTag/>"
        << "</UtilibSerialStream>";
      f.close();

      utilib::Any a;
      ifXMLSerialStream ss("test.xss");
      TS_ASSERT_THROWS_ASSERT
         ( ss >> a, utilib::xmlserialstream_error &e,
           TEST_WHAT(e, "iXMLSerialStream::ParserImplementation::"
                     "start_element(): unknown element (badTag)."));
   }


   void test_pod_subelement()
   {
      std::ofstream f("test.xss", std::ios::trunc);
      f << "<UtilibSerialStream version=\"1\" endian=\"" << std::hex 
        << static_cast<int>(utilib::Serialization_Manager::Endian) << "\">"
        << "<Pod type=\"int\">5<badTag/></Pod>/>"
        << "</UtilibSerialStream>";
      f.close();

      utilib::Any a;
      ifXMLSerialStream ss("test.xss");
      TS_ASSERT_THROWS_ASSERT
         ( ss >> a, utilib::xmlserialstream_error &e,
           TEST_WHAT(e, "iXMLSerialStream::ParserImplementation: "
                     "sub-element found where none are allowed") );
   }


   void test_pod_type_resolution()
   {
      std::ofstream f("test.xss", std::ios::out | std::ios::trunc);
      f << "<UtilibSerialStream version=\"1\" endian=\"" << std::hex 
        << static_cast<int>(utilib::Serialization_Manager::Endian) << "\">"
        << "<Pod type=\"foo\">5</Pod>/>"
        << "</UtilibSerialStream>";
      f.close();

      utilib::Any a;
      ifXMLSerialStream ss("test.xss");

      {
         utilib::OStreamTee tee(std::cerr);
         TS_ASSERT_THROWS(ss >> a, utilib::serializer_unknown_type);
         std::stringstream ref;
         ref << "ERROR: iXMLSerialStream::ParserImplementation: "
             << "stream specifies user-defined name that "
             << "we can't resolve." << std::endl << "     Name = \"foo\""
             << std::endl;
         TS_ASSERT_EQUALS(tee.out.str(), ref.str());
      }

      f.open("test.xss", std::ios::out | std::ios::trunc);
      f << "<UtilibSerialStream version=\"1\" endian=\"" << std::hex 
        << static_cast<int>(utilib::Serialization_Manager::Endian) << "\">"
        << "<Pod typeid=\"5\">5</Pod>/>"
        << "</UtilibSerialStream>";
      f.close();

      ss.open("test.xss");
      TS_ASSERT_THROWS_ASSERT
         ( ss >> a, utilib::xmlserialstream_error &e,
           TEST_WHAT(e, "iXMLSerialStream::ParserImplementation: "
                     "typeid refers to undefined ID") );


      f.open("test.xss", std::ios::out | std::ios::trunc);
      f << "<UtilibSerialStream version=\"1\" endian=\"" << std::hex 
        << static_cast<int>(utilib::Serialization_Manager::Endian) << "\">"
        << "<Pod type=\"test:xss:A\">5</Pod>/>"
        << "</UtilibSerialStream>";
      f.close();

      ss.open("test.xss");
      TS_ASSERT_THROWS_ASSERT
         ( ss >> a, utilib::xmlserialstream_error &e,
           TEST_WHAT(e, "iXMLSerialStream::ParserImplementation: "
                     "POD flag disagrees with registered type") );


      f.open("test.xss", std::ios::out | std::ios::trunc);
      f << "<UtilibSerialStream version=\"1\" endian=\"ff\">"
        << "<Pod type=\"int\">5</Pod>/>"
        << "<Pod type=\"int\" format=\"hex\">ff</Pod>/>"
        << "</UtilibSerialStream>";
      f.close();

      ss.open("test.xss");
      // text mode PODs are OK, even if the endianness doesn't match!
      ss >> a;
      TS_ASSERT_EQUALS(a, 5);
      TS_ASSERT_THROWS_ASSERT
         ( ss >> a, utilib::xmlserialstream_error &e,
           TEST_WHAT(e, "iXMLSerialStream::ParserImplementation: "
                     "POD contains binary data that does not match this "
                     "platform's endian flag: ") );
   }

   void test_object_type_resolution()
   {
      std::ofstream f("test.xss", std::ios::out | std::ios::trunc);
      f << "<UtilibSerialStream version=\"1\" endian=\"" << std::hex 
        << static_cast<int>(utilib::Serialization_Manager::Endian) << "\">"
        << "<Object typeid=\"5\">5</Pod>/>"
        << "</UtilibSerialStream>";
      f.close();

      utilib::Any a;
      ifXMLSerialStream ss("test.xss");
      TS_ASSERT_THROWS_ASSERT
         ( ss >> a, utilib::xmlserialstream_error &e,
           TEST_WHAT(e, "iXMLSerialStream::ParserImplementation: "
                     "typeid refers to undefined ID") );


      f.open("test.xss", std::ios::out | std::ios::trunc);
      f << "<UtilibSerialStream version=\"1\" endian=\"" << std::hex 
        << static_cast<int>(utilib::Serialization_Manager::Endian) << "\">"
        << "<Object type=\"int\">5</Pod>/>"
        << "</UtilibSerialStream>";
      f.close();

      ss.open("test.xss");
      TS_ASSERT_THROWS_ASSERT
         ( ss >> a, utilib::xmlserialstream_error &e,
           TEST_WHAT(e, "iXMLSerialStream::ParserImplementation: "
                     "POD flag disagrees with registered type") );
   
   }


   void test_hex_import()
   {
      double d = 3.14159263;
      std::string hex;

      {
         std::stringstream hex_ss;
         hex_ss << std::hex << std::setfill('0');
         const unsigned char *c = reinterpret_cast<const unsigned char*>(&d);
         size_t pod_size = sizeof(double);
         for( size_t i = 0; i < pod_size; ++i )
            hex_ss << std::setw(2) << (int)*(c+i);
         hex = hex_ss.str();
      }

      if ( hex.find_first_of("abcdef") == std::string::npos )
         TS_FAIL("Test string contains no alpha hex characters");


      std::ofstream f("test.xss", std::ios::out | std::ios::trunc);
      f << "<UtilibSerialStream version=\"1\" endian=\"" << std::hex 
        << static_cast<int>(utilib::Serialization_Manager::Endian) << "\">"
        << "<Pod type=\"double\" format=\"hex\">" << hex << "</Pod>"
        << "</UtilibSerialStream>";
      f.close();

      utilib::Any a;
      ifXMLSerialStream ss("test.xss");
      ss >> a;
      TS_ASSERT_EQUALS(a, d);


      std::transform(hex.begin(), hex.end(), hex.begin(), toupper);
      if ( hex.find_first_of("abcdef") != std::string::npos )
         TS_FAIL("Uppercasing Test string failed");
      if ( hex.find_first_of("ABCDEF") == std::string::npos )
         TS_FAIL("Test string contains no alpha hex characters");
      

      f.open("test.xss", std::ios::out | std::ios::trunc);
      f << "<UtilibSerialStream version=\"1\" endian=\"" << std::hex 
        << static_cast<int>(utilib::Serialization_Manager::Endian) << "\">"
        << "<Pod type=\"double\" format=\"hex\">" << hex << "</Pod>"
        << "</UtilibSerialStream>";
      f.close();

      a.clear();
      ss.open("test.xss");
      ss >> a;
      TS_ASSERT_EQUALS(a, d);


      hex[hex.size()-1] = 'x';
      f.open("test.xss", std::ios::out | std::ios::trunc);
      f << "<UtilibSerialStream version=\"1\" endian=\"" << std::hex 
        << static_cast<int>(utilib::Serialization_Manager::Endian) << "\">"
        << "<Pod type=\"double\" format=\"hex\">" << hex << "</Pod>"
        << "</UtilibSerialStream>";
      f.close();

      ss.open("test.xss");
      TS_ASSERT_THROWS_ASSERT
         ( ss >> a, utilib::xmlserialstream_error &e,
           TEST_WHAT(e, "iXMLSerialStream::ParserImplementation: "
                     "invalid character in hexidecimal binary data") );

      
      hex.resize(hex.size()-1);
      f.open("test.xss", std::ios::out | std::ios::trunc);
      f << "<UtilibSerialStream version=\"1\" endian=\"" << std::hex 
        << static_cast<int>(utilib::Serialization_Manager::Endian) << "\">"
        << "<Pod type=\"double\" format=\"hex\">" << hex << "</Pod>"
        << "</UtilibSerialStream>";
      f.close();

      ss.open("test.xss");
      TS_ASSERT_THROWS_ASSERT
         ( ss >> a, utilib::xmlserialstream_error &e,
           TEST_WHAT(e, "iXMLSerialStream::ParserImplementation: "
                     "hexidecimal binary data ended with a partial byte") );
   }


   void test_bad_xml()
   {
      // Special case for reading empty strings (coverage test)
      std::ofstream f("test.xss", std::ios::out | std::ios::trunc);
      f << "<UtilibSerialStream version=\"1\" endian=\"" << std::hex 
        << static_cast<int>(utilib::Serialization_Manager::Endian) << "\">"
        << "<Pod type=\"int\">5</POD>"
        << "</UtilibSerialStream>";
      f.close();

      utilib::Any a;
      ifXMLSerialStream ss("test.xss");
      TS_ASSERT_THROWS_ASSERT
         (ss >> a, utilib::xmlserialstream_error &e,
          TEST_WHAT(e, "iXMLSerialStream::get_object(): parser threw error "));
   }
};

} // namespace utilib
