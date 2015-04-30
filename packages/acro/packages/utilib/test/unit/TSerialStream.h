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

#include <utilib/sSerialStream.h>
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
= utilib::Serializer().register_serializer<A>("test:sss:A", A::serializer);

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

namespace unittest { class Test_sSerialStream; }
class utilib::unittest::Test_sSerialStream : public CxxTest::TestSuite
{
public:

   void test_basic_pod()
   {
      int    i  = 5;
      double d = 3.1415;
      char   c  = 'A';
      std::string s = "The quick brown fox";

      osSerialStream oss;
      oss << utilib::Serialize(i);
      oss << utilib::Serialize(d);
      oss << utilib::Serialize(c);
      oss << utilib::Serialize(s);

      utilib::SerialObject so;

      isSerialStream iss(oss.str());
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

   void test_explicit_constructor()
   {
      int    i  = 5;

      osSerialStream oss("");
      oss << utilib::Serialize(i);

      utilib::SerialObject so;
      isSerialStream iss(oss.str());
      iss >> so;
      TS_ASSERT(iss.good());
      TS_ASSERT_EQUALS(i, Deserialize(so).expose<int>());
   }

   void test_chained_pod()
   {
      std::string data;

      int    i  = 5;
      double d = 3.1415;
      char   c  = 'A';
      {
         osSerialStream oss;
         oss << i << d << c;
         data = oss.str();
      }

      int i2 = 0;
      double d2 = 0;
      char c2 = 0;

      {
         isSerialStream iss;
         iss.str(data);
         iss >> i2 >> d2 >> c2;
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

      osSerialStream oss;
      oss << a << any;
      isSerialStream iss(oss.str());
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

      sSerialStream ss;
      ss << i << d << c;
      ss >> i2 >> d2 >> c2;

      TS_ASSERT(ss.good());
      TS_ASSERT_EQUALS(i, i2);
      TS_ASSERT_EQUALS(d, d2);
      TS_ASSERT_EQUALS(c, c2);
   }

   void test_singleBuffer_explicit_constructor()
   {
      int i  = 5;
      int i2 = 0;

      sSerialStream ss("");
      ss << i;
      ss >> i2;

      TS_ASSERT(ss.good());
      TS_ASSERT_EQUALS(i, i2);
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

      sSerialStream ss;
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

      osSerialStream oss1;
      oss1 << i;
      osSerialStream oss2(oss1.str());
      oss2 << d;
      oss1.str(oss2.str());
      oss1 << c;

      int i2 = 0;
      double d2 = 0;
      char c2 = 0;

      isSerialStream iss(oss1.str());
      iss >> i2 >> d2 >> c2;

      TS_ASSERT(iss.good());
      TS_ASSERT(oss1.good());
      TS_ASSERT(oss2.good());
      TS_ASSERT_EQUALS(i, i2);
      TS_ASSERT_EQUALS(d, d2);
      TS_ASSERT_EQUALS(c, c2);
   }

   void test_ioAppend()
   {
      int    i  = 5;
      double d = 3.1415;
      char   c  = 'A';

      sSerialStream ss1;
      ss1 << i;
      sSerialStream ss2(ss1.str());
      ss2 << d;
      ss1.str(ss2.str());
      ss1 << c;

      int i2 = 0;
      double d2 = 0;
      char c2 = 0;

      ss1 >> i2 >> d2 >> c2;

      TS_ASSERT(ss1.good());
      TS_ASSERT(ss2.good());
      TS_ASSERT_EQUALS(i, i2);
      TS_ASSERT_EQUALS(d, d2);
      TS_ASSERT_EQUALS(c, c2);
   }
};

namespace unittest { class Test_BinarySerialStream; }
/** In a weird twist of fate, the design of the ios class hierarchy (and
 *  in particular, the SerialStream system built on top of it) is such
 *  that the lower-level classes (SerialStream and BinarySerialStream)
 *  are incomplete due to the virtual functions pointing to the actual
 *  storage system.  Instead of mocking up another derivative class so
 *  that we can test the base classes, we are just going to use the
 *  (already tested) sSerialStream.
 */
class utilib::unittest::Test_BinarySerialStream : public CxxTest::TestSuite
{
public:

   void test_streamHeaderValidation()
   {
      osSerialStream oss;
      std::string data = oss.str();
      isSerialStream iss;

      // Foul up the format marker
      ++data[1];
      TS_ASSERT_THROWS_ASSERT
         (iss.str(data), utilib::serialstream_error &e,
           TEST_WHAT(e, "iBinarySerialStream::init_stream(): "
                     "stream does not begin with the correct format marker") );
      --data[1];

      // Foul up the format version
      ++data[4];
      TS_ASSERT_THROWS_ASSERT
         ( iss.str(data), utilib::serialstream_error &e,
           TEST_WHAT(e, "iBinarySerialStream::init_stream(): stream version "
                     "(2) does not match the current version (1).") );
      --data[4];

      // Foul up the endianness flag
      data[5] = ~utilib::Serialization_Manager::Endian;
      TS_ASSERT_THROWS_ASSERT
         ( iss.str(data), utilib::serialstream_error &e,
           TEST_WHAT(e, "iBinarySerialStream::init_stream(): "
                     "stream endian flag") );
      data[5] = utilib::Serialization_Manager::Endian;

      // Foul up the format version
      ++data[6];
      TS_ASSERT_THROWS_ASSERT
         ( iss.str(data), utilib::serialstream_error &e,
           TEST_WHAT(e, "iBinarySerialStream::init_stream(): stream size_t") );
      --data[6];

      // NB: the basic iss does not validate until the first read attempt 
      isSerialStream iss2;
      utilib::Any a;
      TS_ASSERT_THROWS_ASSERT
         (iss2 >> a, utilib::serialstream_error &e,
           TEST_WHAT(e, "iBinarySerialStream::init_stream(): "
                     "stream does not begin with the correct format marker") );

   }

   void test_TypeKeyFlag()
   {
      osSerialStream oss;
      std::list<double> l;
      l.push_back(4);
      l.push_back(2);
      oss << int(5) << l;
      std::string referenceData = oss.str();

      utilib::Any a;
      isSerialStream iss;
      std::string data = referenceData;

      //std::cerr << "\"" << data <<"\"" <<std::endl;

      // check for unknown "type" (change "int" -> "xnt")
      data[7+3*sizeof(size_t)] = 'x';
      iss.str(data);
      {
         utilib::OStreamTee tee(std::cerr);
         TS_ASSERT_THROWS(iss >> a, utilib::serializer_unknown_type);
         std::stringstream ref;
         ref << "ERROR: iBinarySerialStream::get_object(): stream specifies "
             << "typeid/user-defined name that we can't resolve." << std::endl 
             <<"     TypeID: " << *(size_t*)(&data[7+1*sizeof(size_t)])
             << ", Name = \"xnt\"" << std::endl;
         TS_ASSERT_EQUALS(tee.out.str(), ref.str());
      }

      // foul up the POD flag (mark the int as non-pod)
      data = referenceData;
      data[7+3*sizeof(size_t)+3] = 0;
      iss.str(data);
      TS_ASSERT_THROWS_ASSERT
         ( iss >> a, utilib::serialstream_error &e,
           TEST_WHAT(e, "iBinarySerialStream::get_object(): SerialStream "
                     "POD flag disagrees with registered type for ") );

      // foul up the POD size (mark the int as 3-bytes)
      data = referenceData;
      data[7+3*sizeof(size_t)+3+1] = 0;
      iss.str(data);
      TS_ASSERT_THROWS_ASSERT
         ( iss >> a, utilib::serialstream_error &e,
           TEST_WHAT(e, "iBinarySerialStream::get_object(): "
                     "SerialStream POD size disagrees with "
                     "registered type size for ") );

      // foul up the data key (mark a key > known key)
      data = referenceData;
      std::stringstream ss;
      size_t &keyId = *reinterpret_cast<size_t*>
         (&data[0] + 7 + 3*sizeof(size_t) + 3 + 1 + sizeof(size_t));
      keyId += 1;
      ss << "iBinarySerialStream::get_object(): "
         "Stream returned an unknown type key (" << keyId <<")";
      iss.str(data);
      TS_ASSERT_THROWS_ASSERT
         ( iss >> a, utilib::serialstream_error &e, TEST_WHAT(e, ss.str()) );

      // foul up the data key (mark a key < known key, but not defined)
      keyId -= 2;
      ss.str("");
      ss << "iBinarySerialStream::get_object(): "
         "Stream returned an unknown type key (" << keyId <<")";
      iss.str(data);
      TS_ASSERT_THROWS_ASSERT
         ( iss >> a, utilib::serialstream_error &e, TEST_WHAT(e, ss.str()) );
   }


   void test_eofDuringRead()
   {
      osSerialStream oss;
      std::list<double> l;
      l.push_back(4);
      l.push_back(2);
      oss << int(5) << l;
      std::string referenceData = oss.str();

      utilib::Any a;
      isSerialStream iss;
      std::string data = referenceData;

      // EOF during POD read
      data.resize(7 + 3*sizeof(size_t) + 3 + 1 + 2*sizeof(size_t) + 2);
      iss.str(data);
      TS_ASSERT_THROWS_ASSERT
         ( iss >> a, utilib::serialstream_error &e,
           TEST_WHAT(e, "iBinarySerialStream::get_object(): "
                     "encountered EOF in middle of SerialPOD.") );

      // EOF during type definition
      data = referenceData;
      data.resize(7 + 3*sizeof(size_t) + 3 + 1 + 2*sizeof(size_t) + sizeof(int)
                  + 3*sizeof(size_t) + 6);
      iss.str(data);
      iss >> a;
      TS_ASSERT_THROWS_ASSERT
         ( iss >> a, utilib::serialstream_error &e,
           TEST_WHAT(e, "iBinarySerialStream::get_object(): "
                     "encountered EOF during type definition.") );
      
      // EOF during type definition (pod flag)
      data = referenceData;
      data.resize(7 + 3*sizeof(size_t) + 3 + 1 + 2*sizeof(size_t) + sizeof(int)
                  + 3*sizeof(size_t) + std::strlen("std::list<double,*>"));
      iss.str(data);
      iss >> a;
      TS_ASSERT_THROWS_ASSERT
         ( iss >> a, utilib::serialstream_error &e,
           TEST_WHAT(e, "iBinarySerialStream::get_object(): "
                     "encountered EOF during type definition POD flags.") );
      
      // EOF during POD read within object
      data = referenceData;
      data.resize(7 + 3*sizeof(size_t) + 3 + 1 + 2*sizeof(size_t) + sizeof(int)
                  + 3*sizeof(size_t) + std::strlen("std::list<double,*>") + 1
                  + 3*sizeof(size_t) + std::strlen("double") + 1
                  + sizeof(size_t)
                  + sizeof(size_t) + sizeof(size_t) + 2 );
      iss.str(data);
      iss >> a;
      TS_ASSERT_THROWS_ASSERT
         ( iss >> a, utilib::serialstream_error &e,
           TEST_WHAT(e, "iBinarySerialStream::get_object(): "
                     "encountered EOF in middle of SerialPOD.") );
      
      // EOF during Object read
      data = referenceData;
      data.resize(7 + 3*sizeof(size_t) + 3 + 1 + 2*sizeof(size_t) + sizeof(int)
                  + 3*sizeof(size_t) + std::strlen("std::list<double,*>") + 1
                  + 3*sizeof(size_t) + std::strlen("double") + 1 
                  + sizeof(size_t)
                  + sizeof(size_t) + sizeof(size_t) + sizeof(double) );
      iss.str(data);
      iss >> a;
      TS_ASSERT_THROWS_ASSERT
         ( iss >> a, utilib::serialstream_error &e,
           TEST_WHAT(e, "iBinarySerialStream::get_object(): "
                     "encountered EOF in middle of SerialObject container.") );
      
   }
};



class sSerialStream_Tester : public sSerialStream
{
public:
   sSerialStream_Tester() : sSerialStream() {}
   sSerialStream_Tester(const std::string & data) : sSerialStream(data) {}

   iSerialStream& seekg(off_type off, ios_base::seekdir ref)
   { return sSerialStream::seekg(off, ref); }

   iSerialStream& seekg(pos_t pos)
   { return sSerialStream::seekg(pos); }

   iSerialStream& read(char* s, std::streamsize n)
   { return sSerialStream::read(s, n); }

   oSerialStream& seekp(off_type off, ios_base::seekdir ref)
   { return sSerialStream::seekp(off, ref); }

   oSerialStream& seekp(pos_t pos)
   { return sSerialStream::seekp(pos); }

   oSerialStream& write(const char* s, const std::streamsize n)
   { return sSerialStream::write(s, n); }
};


namespace unittest { class Test_SerialStream; }
/** In a weird twist of fate, the design of the ios class hierarchy (and
 *  in particular, the SerialStream system built on top of it) is such
 *  that the lower-level classes (SerialStream and BinarySerialStream)
 *  are incomplete due to the virtual functions pointing to the actual
 *  storage system.  Instead of mocking up another derivative class so
 *  that we can test the protected API in the base classes, we are just
 *  going to use the (already tested) sSerialStream.
 */
class utilib::unittest::Test_SerialStream : public CxxTest::TestSuite
{
public:
   void test_iosFlags()
   {
      int i = 42;
      utilib::Any a;

      sSerialStream ss;
      TS_ASSERT(ss.good());
      TS_ASSERT(!ss.bad());
      TS_ASSERT(!ss.fail());
      TS_ASSERT(!ss.eof());

      ss << i;
      // Read a value from the stream
      ss >> a;
      TS_ASSERT_EQUALS(a, 42);
      TS_ASSERT(ss.good());
      TS_ASSERT(!ss.bad());
      TS_ASSERT(!ss.fail());
      TS_ASSERT(!ss.eof());

      // Read a nonexistant value: sets EOF
      ss >> a;
      TS_ASSERT(a.empty());
      TS_ASSERT(!ss.good());
      TS_ASSERT(!ss.bad());
      TS_ASSERT(ss.fail());
      TS_ASSERT(ss.eof());

      // Read another nonexistant value: sets EOF
      utilib::SerialObject so;
      a = 5;
      ss >> a;
      TS_ASSERT(a.empty());
      TS_ASSERT(!ss.good());
      TS_ASSERT(!ss.bad());
      TS_ASSERT(ss.fail());
      TS_ASSERT(ss.eof());
   }


   void test_ios_API()
   {
      isSerialStream iss;
      osSerialStream oss;

      TS_ASSERT_EQUALS(iss.tellg(), sSerialStream::pos_t(0));
      TS_ASSERT_EQUALS(oss.tellp(), sSerialStream::pos_t(7));

      sSerialStream ss;
      // the ss has already called init_stream()
      TS_ASSERT_EQUALS(ss.tellp(), sSerialStream::pos_t(7));

      // trigger a fail (buffer underrun)
      utilib::Any a;
      ss >> a;
      TS_ASSERT(ss.fail());
      TS_ASSERT(ss.eof());
      TS_ASSERT_EQUALS(ss.tellp(), sSerialStream::pos_t(-1));
   }


   void test_seek()
   {
      {
         sSerialStream_Tester ss;

         // seek to the beginning of the stream
         TS_ASSERT_EQUALS(ss.seekg(0).tellg(), sSerialStream::pos_t(0));
         TS_ASSERT(ss.good());
         TS_ASSERT_EQUALS(ss.seekp(0).tellp(), sSerialStream::pos_t(0));
         TS_ASSERT(ss.good());
      }
      
      // seek past the end of the stream (absolute)
      {
         sSerialStream_Tester ss;
         TS_ASSERT_EQUALS(ss.seekg(8).tellg(), sSerialStream::pos_t(-1));
         TS_ASSERT(!ss.good());
         TS_ASSERT(!ss.bad());
         TS_ASSERT(ss.fail());
         TS_ASSERT(!ss.eof());
      }
      {
         sSerialStream_Tester ss;
         TS_ASSERT_EQUALS(ss.seekp(8).tellp(), sSerialStream::pos_t(-1));
         TS_ASSERT(!ss.good());
         TS_ASSERT(!ss.bad());
         TS_ASSERT(ss.fail());
         TS_ASSERT(!ss.eof());
      }

      // seek past the end of the stream (relative)
      {
         sSerialStream_Tester ss;
         TS_ASSERT_EQUALS( ss.seekg(1, std::ios_base::cur).tellg(), 
                           sSerialStream::pos_t(-1) );
         TS_ASSERT(!ss.good());
         TS_ASSERT(!ss.bad());
         TS_ASSERT(ss.fail());
         TS_ASSERT(!ss.eof());
      }
      {
         sSerialStream_Tester ss;
         TS_ASSERT_EQUALS( ss.seekp(1, std::ios_base::cur).tellp(), 
                           sSerialStream::pos_t(-1) );
         TS_ASSERT(!ss.good());
         TS_ASSERT(!ss.bad());
         TS_ASSERT(ss.fail());
         TS_ASSERT(!ss.eof());
      }
   }
};

} // namespace utilib
