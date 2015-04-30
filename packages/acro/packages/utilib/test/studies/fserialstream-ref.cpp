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

#include <utilib/fSerialStream.h>

using std::endl;

using utilib::SerialObject;
using utilib::Serializer;
using utilib::Any;

#define OUT std::cout

namespace {

class RefData 
{
public:
   RefData()
      : i(), d(), us(), c(),
        s1(), s2(), 
        l(), m()
   {}
 
   RefData(int ver)
      : i(ver), d(), us(), c(),
        s1(), s2(), 
        l(), m()
   {
      if ( ver == 1 )
      {
         d = 3.1415;
         us = 42;
         c = 'A';
         s1 = "Hello, World";
         s2 = "Another string";
         l.push_back("Hi");
         l.push_back("Mom");
         m[1] = 1.1;
         m[3] = 3.3;
         m[2] = 2.2;
         m[4] = 4.4;
      }
      else if ( ver == 2 )
      {
         d = 1.23456;
         us = 21;
         c = 'X';
         s1 = "The quick brown fox jumped over the Lazy Dog";
         s2 = "utilib::test_fserialstream";
         l.push_back("This");
         l.push_back("is");
         l.push_back("a");
         l.push_back("list");
         l.push_back("of");
         l.push_back("words");
         m.clear();
      }
      else
         EXCEPTION_MNGR(std::logic_error, "RefData: unknown version");
   }
  
   bool operator==(const RefData& rhs) const
   {
      return ( i == rhs.i ) && ( d == rhs.d ) && ( c == rhs.c ) && 
         ( s1 == rhs.s1 ) && ( s2 == rhs.s2 ) && ( us == rhs.us ) && 
         ( l == rhs.l ) && ( m == rhs.m );
   }

   int  i;
   double  d;
   unsigned short  us;
   char  c;
   std::string  s1;
   std::string  s2;
   std::list<std::string>  l;
   std::map<int,double>    m;

private:
   static int serializer(SerialObject::elementList_t& serial, 
                         Any& data, 
                         bool serialize)
   {
      RefData& me = const_cast<RefData&>(data.expose<RefData>());
      serial_transform(serial, me.i, serialize);
      serial_transform(serial, me.d, serialize);
      serial_transform(serial, me.us, serialize);
      serial_transform(serial, me.c, serialize);
      serial_transform(serial, me.s1, serialize);
      serial_transform(serial, me.s2, serialize);
      serial_transform(serial, me.l, serialize);
      serial_transform(serial, me.m, serialize);
      return 0;
   }

   static const int serializer_registered;
};

// Register the serializer for class RefData
const int RefData::serializer_registered 
    = Serializer().register_serializer<RefData>( "fss::RefData", 
                                                 RefData::serializer );

} // namespace (local)


int test_fserialstream_ref(int argc, char**)
{
   if ( argc > 1 )
   {
      try
      {
         OUT << "Generating reference data file" << endl;
         utilib::ofSerialStream ofss("fserialstream.ref");
         ofss << RefData(1) << RefData(2);
         ofss.close();
         OUT << "    (DONE)" << endl;
      }
      catch (std::exception &e)
      {
         OUT << "Caught unexpected exception: " << endl << e.what() << endl;
      }
   }


   OUT << "Load and test a reference solution" << endl;
   try
   {
      RefData r;

      utilib::ifSerialStream ifss("fserialstream.ref");

      ifss >> r;
      OUT << "   Ref Data: " << ((r == RefData(1)) ? "[ OK ]" : "[FAIL]") 
          << endl;

      ifss >> r;
      OUT << "   Ref Data: " << ((r == RefData(2)) ? "[ OK ]" : "[FAIL]") 
          << endl;

      ifss.close();
   }
   catch (std::exception &e)
   {
      OUT << "Caught unexpected exception: " << endl << e.what() << endl;
   }


   return 0;
}
