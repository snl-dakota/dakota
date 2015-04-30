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

#include <utilib/fXMLSerialStream.h>

using std::endl;

using std::string;
using std::list;

using utilib::SerialObject;
using utilib::Serializer;
using utilib::Any;
using utilib::ifXMLSerialStream;
using utilib::ofXMLSerialStream;
using utilib::fXMLSerialStream;

#define OUT std::cout

template<typename T>
std::ostream& print(std::ostream& os, const T& stl)
{
   typename T::const_iterator it = stl.begin();
   os << "[ " << stl.size() << ": ";
   for ( ; it != stl.end(); ++it )
      os << *it << " ";
   os << "]";
   return os;
}

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const list<T1,T2>& stl)
  { return print(os, stl); }


namespace {

/// Defining a simple class to serialize
class A
{
public:
   A()
      : a(0), b(0), c()
   {}
   A(int x, float y)
      : a(x), b(y), c(x,y)
   {}
   std::ostream& print(std::ostream& os) const
   {
      os << "{" << a << ", " << b << ", " << c << "}";
      return os;
   }
   bool operator==(const A& rhs) const
   {
      return ( a == rhs.a ) && ( b == rhs.b ) && ( c == rhs.c );
   }
   bool operator<(const A& rhs) const
   {
      return a < rhs.a; // bogus... I know...
   }

private:
   static int serializer(SerialObject::elementList_t& serial, 
                         Any& data, 
                         bool serialize)
   {
      A& me = const_cast<A&>(data.expose<A>());
      serial_transform(serial, me.a, serialize);
      serial_transform(serial, me.b, serialize);
      serial_transform(serial, me.c, serialize);
      return 0;
   }

private:
   static const int serializer_registered;

   int a;
   float b;
   list<double> c;
};

// Register the serializer for class A
const int A::serializer_registered
   = Serializer().register_serializer<A>("fxss:A", A::serializer);

// support stream output for the testing macros
std::ostream& operator<<(std::ostream& os, const A& data)
   { return data.print(os); }


} // namespace local

//-----------------------------------------------------------------------

int test_fxmlserialstream(int , char**)
{
   int     i1  = 5;
   double  d1  = 3.14159263;
   double  d1a = 1.234;
   char    c1  = 'A';
   A       a1(2, 5.5f);
   wchar_t w1  = 'X';
   string  s1  = "Hello World (&'\\\"<>\")";

   // Test 0: generate a test file
   try
   {
      ofXMLSerialStream ofss("fxmlserialstream.out");
      ofss << i1 << d1 << a1;
      ofss.close();

      OUT << "(file created)" << endl;
   }
   catch (std::exception &e)
   {
      OUT << "Caught unexpected exception: " << endl << e.what() << endl;
   }

   // Test 1: reading 1 at a time
   OUT << "Single reads:" << endl;
   try
   {
      int    i2 = 0;
      double d2 = 1.0;
      A      a2;
      
      ifXMLSerialStream ifss("fxmlserialstream.out");
      ifss >> i2;
      OUT << "   int:    " << ( i1 == i2 ? "[ OK ]" : "[FAIL]" ) << endl;
      ifss >> d2;
      OUT << "   double: " << ( d1 == d2 ? "[ OK ]" : "[FAIL]" ) << endl;
      ifss >> a2;
      OUT << "   A:      " << (( a1 == a2 )? "[ OK ]" : "[FAIL]" ) << endl;
      ifss.close();
   }
   catch (std::exception &e)
   {
      OUT << "Caught unexpected exception: " << endl << e.what() << endl;
   }

   // Test 2: chain reading 
   OUT << "Chain reading:" << endl;
   try
   {
      int    i2 = 0;
      double d2 = 1.0;
      A      a2;
      
      ifXMLSerialStream ifss("fxmlserialstream.out");
      ifss >> i2 >> d2 >> a2;
      ifss.close();

      OUT << "   int:    " << ( i1 == i2 ? "[ OK ]" : "[FAIL]" ) << endl;
      OUT << "   double: " << ( d1 == d2 ? "[ OK ]" : "[FAIL]" ) << endl;
      OUT << "   A:      " << (( a1 == a2 )? "[ OK ]" : "[FAIL]" ) << endl;
   }
   catch (std::exception &e)
   {
      OUT << "Caught unexpected exception: " << endl << e.what() << endl;
   }

   // Test 3: type conversions
   OUT << "Converting read:" << endl;
   try
   {
      list<double> l1;
      list<double> l2;
      list<double> l3;

      ifXMLSerialStream ifss("fxmlserialstream.out");
      ifss >> l1 >> l2;

      OUT << "   l1:     " << ( i1 == l1.front() ? "[ OK ]: " : "[FAIL]: " ) 
          << l1 << endl;
      OUT << "   l2:     " << ( d1 == l2.front() ? "[ OK ]: " : "[FAIL]: " ) 
          << l2 << endl;

      try {
         ifss >> l3;
         OUT << "[FAIL]: should NOT be able to read A as list<double>" << endl;
      }
      catch ( ... )
      {
         OUT << "   [ OK ]: get A as list<double> (correctly) threw exception"
             << endl;
      }
      ifss.close();      
   }
   catch (std::exception &e)
   {
      OUT << "Caught unexpected exception: " << endl << e.what() << endl;
   }

   // Test 4: appending to a file
   OUT << "Appending to an existing file" << endl;
   try
   {
      int    i2 = 0;
      double d2 = 1.0;
      A      a2;
      double d2a = 0.0;

      ofXMLSerialStream ofss("fxmlserialstream.out", 
                             utilib::XML::Default,
                             std::ios_base::app);
      ofss << d1a;
      ofss.close();
      OUT << "   (appended d1a)" << endl;

      ifXMLSerialStream ifss("fxmlserialstream.out");
      ifss >> i2 >> d2 >> a2 >> d2a;
      ifss.close();

      OUT << "   int:    " << ( i1 == i2 ? "[ OK ]" : "[FAIL]" ) << endl;
      OUT << "   double: " << ( d1 == d2 ? "[ OK ]" : "[FAIL]" ) << endl;
      OUT << "   A:      " << (( a1 == a2 )? "[ OK ]" : "[FAIL]" ) << endl;
      OUT << "   double: " << ( d1a == d2a ? "[ OK ]" : "[FAIL]" ) << endl;
   }
   catch (std::exception &e)
   {
      OUT << "Caught unexpected exception: " << endl << e.what() << endl;
   }

   // Test 5: appending information
   OUT << "fXMLSerialStream appending to an existing file" << endl;
   try
   {
      int    i2 = 0;
      double d2 = 1.0;
      A      a2;
      double d2a = 1.0;
      char   c2 = ' ';

      fXMLSerialStream fss("fxmlserialstream.out", 
                           utilib::XML::noindent | utilib::XML::typeids,
                           std::ios_base::ate);
      // read 2 ...
      fss >> i2 >> d2;
      OUT << "   int:    " << ( i1 == i2 ? "[ OK ]" : "[FAIL]" ) << endl;
      OUT << "   double: " << ( d1 == d2 ? "[ OK ]" : "[FAIL]" ) << endl;
      // write 1...
      fss << c1;
      OUT << "   (wrote c1)" << endl;
      fss.flush();
      // read 2 ...
      fss >> a2 >> d2a >> c2;
      OUT << "   A:      " << (( a1 == a2 )? "[ OK ]" : "[FAIL]" ) << endl;
      OUT << "   double: " << ( d1a == d2a ? "[ OK ]" : "[FAIL]" ) << endl;
      OUT << "   char    " << (( c1 == c2 )? "[ OK ]" : "[FAIL]" ) << endl;
      fss.close();
   }
   catch (std::exception &e)
   {
      OUT << "Caught unexpected exception: " << endl << e.what() << endl;
   }
      
   // Test 6: 
   OUT << "fXMLSerialStream to a new/truncated file" << endl;
   try
   {
      int     i2 = 0;
      double  d2 = 1.0;
      A       a2;
      char    c2 = ' ';
      wchar_t w2 = ' ';
      string  s2 = "";

      fXMLSerialStream fss("fxmlserialstream.out", 
                           utilib::XML::Default,
                           std::ios_base::trunc);
      fss << i1 << d1;
      fss >> i2 >> d2;
      OUT << "   int:    " << ( i1 == i2 ? "[ OK ]" : "[FAIL]" ) << endl;
      OUT << "   double: " << ( d1 == d2 ? "[ OK ]" : "[FAIL]" ) << endl;

      fss << a1 << c1;
      fss >> a2 >> c2;
      OUT << "   A:      " << (( a1 == a2 )? "[ OK ]" : "[FAIL]" ) << endl;
      OUT << "   char    " << (( c1 == c2 )? "[ OK ]" : "[FAIL]" ) << endl;

      fss << w1 << s1;
      fss >> w2 >> s2;
      OUT << "   wchar_t " << (( w1 == w2 )? "[ OK ]" : "[FAIL]" ) << endl;
      OUT << "   string  " << (( s1 == s2 )? "[ OK ]" : "[FAIL]" ) << endl;
      fss.close();
   }
   catch (std::exception &e)
   {
      OUT << "Caught unexpected exception: " << endl << e.what() << endl;
   }
   
   return 0;
}
