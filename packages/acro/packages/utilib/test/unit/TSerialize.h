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

#include <utilib/Serialize.h>

#include <utilib/BasicArray.h>
#include <utilib/NumArray.h>
#include <utilib/MixedIntVars.h>
#include <utilib/OStreamTee.h>

#include "CommonTestUtils.h"


// NB: we use #defines so that if a test fails, the reported line number
// is from the original TEST code, and not from the utility function
// code (the latter being very difficult to diagnose for large tests
// like test_pod_serializers).

#define TEST_POD_serial_transform(TYPE, VALUE, LEN)                     \
   if ( true ) {                                                        \
      SerialObject::elementList_t so_list;                              \
      TYPE init = VALUE;                                                \
                                                                        \
      /* serialize */                                                   \
      TS_ASSERT_EQUALS(serial_transform(so_list, init, true), 0);       \
      TS_ASSERT_EQUALS(so_list.size(), size_t(1));                      \
                                                                        \
      /* verify serial POD */                                           \
      SerialObject &so = so_list.front();                               \
      TS_ASSERT_EQUALS(Serializer().get_typeinfo(so.type), &typeid(TYPE)); \
      TS_ASSERT(so.data.is_type(typeid(SerialPOD)));                    \
      if ( ! POD_text_mode )                                            \
         TS_ASSERT_EQUALS(so.data.expose<SerialPOD>().size(), LEN);     \
                                                                        \
      /* deserialize */                                                 \
      TYPE final;                                                       \
      TYPE test = VALUE;                                                \
      TS_ASSERT_EQUALS(serial_transform(so_list, final, false), 0);     \
      TS_ASSERT_EQUALS(final, test);                                    \
   } else static_cast<void>(0)

#define TEST_POD_serialize(TYPE, VALUE, LEN)                            \
   if ( true ) {                                                        \
      /* serialize */                                                   \
      TYPE init = VALUE;                                                \
      SerialObject so = Serialize(init, POD_text_mode);                 \
                                                                        \
      /* verify serial POD */                                           \
      TS_ASSERT_EQUALS(Serializer().get_typeinfo(so.type), &typeid(TYPE)); \
      TS_ASSERT(so.data.is_type(typeid(SerialPOD)));                    \
      if ( ! POD_text_mode )                                            \
         TS_ASSERT_EQUALS(so.data.expose<SerialPOD>().size(), LEN);     \
                                                                        \
      /* deserialize */                                                 \
      TYPE final = Deserialize(so).expose<TYPE >();                     \
      TYPE test = VALUE;                                                \
      TS_ASSERT_EQUALS(final, test);                                    \
   } else static_cast<void>(0)


/// Composite test given type, value, and the expected serialized length
#define TEST_POD_LEN(TYPE, VALUE, LEN)                  \
      if ( true ) {                                     \
         TEST_POD_serial_transform(TYPE, VALUE, LEN);   \
         TEST_POD_serialize(TYPE, VALUE, LEN);          \
      } else static_cast<void>(0)

#define TEST_POD(TYPE, VALUE) TEST_POD_LEN(TYPE, VALUE, sizeof(TYPE))


#define TEST_STL_serial_transform(TYPE, VALUE, ADAPTOR, CMP)            \
   if ( true ){                                                         \
      SerialObject::elementList_t so_list;                              \
      TYPE init = VALUE;                                                \
                                                                        \
      /* serialize */                                                   \
      TS_ASSERT_EQUALS(serial_transform(so_list, init, true), 0);       \
      TS_ASSERT_EQUALS(so_list.size(), size_t(1));                      \
                                                                        \
      /* verify serialized STL */                                       \
      SerialObject &so = so_list.front();                               \
      TS_ASSERT_EQUALS(Serializer().get_typeinfo(so.type), &typeid(TYPE)); \
      TS_ASSERT(so.data.is_type(typeid(SerialObject::elementList_t)));  \
      const SerialObject::elementList_t &el =                           \
         so.data.expose<SerialObject::elementList_t>();                 \
      if ( ADAPTOR )                                                    \
      {                                                                 \
         TS_ASSERT_EQUALS(el.size(), size_t(1));                        \
         TS_ASSERT( el.front().data.is_type                             \
                    (typeid(SerialObject::elementList_t)) );            \
         TS_ASSERT_EQUALS( el.front().data.expose                       \
                           <SerialObject::elementList_t>().size(),      \
                           VALUE.size() );                              \
      }                                                                 \
      else                                                              \
         TS_ASSERT_EQUALS( el.size(), VALUE.size() );                   \
                                                                        \
      /* deserialize */                                                 \
      TYPE final;                                                       \
      TS_ASSERT_EQUALS(serial_transform(so_list, final, false), 0);     \
      CMP(final, VALUE);                                                \
   } else static_cast<void>(0)

#define TEST_STL_serialize(TYPE, VALUE, ADAPTOR, CMP)                   \
   if ( true ) {                                                        \
      /* serialize */                                                   \
      TYPE init = VALUE;                                                \
      SerialObject so = Serialize(init);                                \
                                                                        \
      /* verify serial POD */                                           \
      TS_ASSERT_EQUALS(Serializer().get_typeinfo(so.type), &typeid(TYPE)); \
      TS_ASSERT(so.data.is_type(typeid(SerialObject::elementList_t)));  \
      const SerialObject::elementList_t &el =                           \
         so.data.expose<SerialObject::elementList_t>();                 \
      if ( ADAPTOR )                                                    \
      {                                                                 \
         TS_ASSERT_EQUALS(el.size(), size_t(1));                        \
         TS_ASSERT( el.front().data.is_type                             \
                    (typeid(SerialObject::elementList_t)) );            \
         TS_ASSERT_EQUALS( el.front().data.expose                       \
                           <SerialObject::elementList_t>().size(),      \
                           VALUE.size() );                              \
      }                                                                 \
      else                                                              \
         TS_ASSERT_EQUALS( el.size(), VALUE.size() );                   \
                                                                        \
      /* deserialize */                                                 \
      CMP(Deserialize(so).expose<TYPE >(), VALUE);                      \
   } else static_cast<void>(0)


/// Composite test given type, value
#define TEST_STL(TYPE, VALUE, ADAPTOR, CMP)                     \
   if ( true ) {                                                \
      TEST_STL_serial_transform(TYPE, VALUE, ADAPTOR, CMP);     \
      TEST_STL_serialize(TYPE, VALUE, ADAPTOR, CMP);            \
   } else static_cast<void>(0)

#define TEST_STL_driver_1(TYPE, A, B, C)                \
   if ( true ) {                                        \
      TYPE val;                                         \
      TEST_STL(TYPE, val, false, TS_ASSERT_EQUALS);     \
                                                        \
      val.insert(val.end(), A);                         \
      val.insert(val.end(), B);                         \
      val.insert(val.end(), C);                         \
      TEST_STL(TYPE, val, false, TS_ASSERT_EQUALS);     \
   } else static_cast<void>(0)

#define TEST_STL_driver_2(TYPE, A, B, C, FCN, ADAPTOR)  \
   if ( true ) {                                        \
      TYPE val;                                         \
      TEST_STL(TYPE, val, ADAPTOR, TS_ASSERT_EQUALS);   \
                                                        \
      val.FCN(A);                                       \
      val.FCN(B);                                       \
      val.FCN(C);                                       \
      TEST_STL(TYPE, val, ADAPTOR, TS_ASSERT_EQUALS);   \
   } else static_cast<void>(0)



#define TEST_OBJ_serial_transform(TYPE, VALUE, CMP)                     \
   if ( true ){                                                         \
      SerialObject::elementList_t so_list;                              \
      TYPE init = VALUE;                                                \
                                                                        \
      /* serialize */                                                   \
      TS_ASSERT_EQUALS(serial_transform(so_list, init, true), 0);       \
      TS_ASSERT_EQUALS(so_list.size(), size_t(1));                      \
                                                                        \
      /* verify serialized OBJ */                                       \
      SerialObject &so = so_list.front();                               \
      TS_ASSERT_EQUALS(Serializer().get_typeinfo(so.type), &typeid(TYPE)); \
      TS_ASSERT(so.data.is_type(typeid(SerialObject::elementList_t)));  \
                                                                        \
      /* deserialize */                                                 \
      TYPE final;                                                       \
      TS_ASSERT_EQUALS(serial_transform(so_list, final, false), 0);     \
      CMP(final, VALUE);                                                \
   } else static_cast<void>(0)

#define TEST_OBJ_serialize(TYPE, VALUE, CMP)                            \
   if ( true ) {                                                        \
      /* serialize */                                                   \
      TYPE init = VALUE;                                                \
      SerialObject so = Serialize(init);                                \
                                                                        \
      /* verify serial POD */                                           \
      TS_ASSERT_EQUALS(Serializer().get_typeinfo(so.type), &typeid(TYPE)); \
      TS_ASSERT(so.data.is_type(typeid(SerialObject::elementList_t)));  \
                                                                        \
      /* deserialize */                                                 \
      CMP(Deserialize(so).expose<TYPE >(), VALUE);                      \
   } else static_cast<void>(0)


/// Composite test given type, value
#define TEST_OBJ(TYPE, VALUE)                                   \
   if ( true ) {                                                \
      TEST_OBJ_serial_transform(TYPE, VALUE, TS_ASSERT_EQUALS); \
      TEST_OBJ_serialize(TYPE, VALUE, TS_ASSERT_EQUALS);        \
   } else static_cast<void>(0)


namespace {

template<typename T>
void PQueue_Equals(std::priority_queue<T> a, std::priority_queue<T> b)
{
   while ( ! a.empty() && ! b.empty() )
   {
      TS_ASSERT_EQUALS(a.top(), b.top());
      a.pop();
      b.pop();
   }
   TS_ASSERT(a.empty());
   TS_ASSERT(b.empty());
}

void NullInitializer(utilib::Any& data)
{ data.clear(); }

class UnknownClass {};

class AnotherClass {};

} // namespace (local)



namespace utilib {
namespace unittest { class Test_Serialize; }

class utilib::unittest::Test_Serialize : public CxxTest::TestSuite
{
public:

   void setUp()
   {
      // As this TestSuite serializes and then deserializes data, it is
      // important that a serialization failure terminates the test.
      CxxTest::setAbortTestOnFail(true);
   }

   void test_empty_any()
   {
      // This should serialize as 'void'
      utilib::Any a;

      /* test 1: serial_transform */
      {
         SerialObject::elementList_t so_list;
         TS_ASSERT_EQUALS(serial_transform(so_list, a, true), 0);
         TS_ASSERT_EQUALS(so_list.size(), size_t(1));
         
         SerialObject &so = so_list.front();
         TS_ASSERT_EQUALS(so.type, size_t(0));
         TS_ASSERT(so.data.empty());
         
         /* deserialize */
         utilib::Any b = 1;
         TS_ASSERT_EQUALS(serial_transform(so_list, b, false), 0);
         TS_ASSERT(b.empty());
      }

      /* test 2: Serialize/Deserialize */
      {
         SerialObject so = Serialize(a);
         
         // void is the only type that is strictly guaranteed a specific
         // type number
         TS_ASSERT_EQUALS(so.type, size_t(0));
         TS_ASSERT(so.data.empty());
         
         TS_ASSERT(Deserialize(so).empty());
      }
   }


   /// Test all POD serializers [with both serial_transform() and
   /// Serialize() / Deserialize() interfaces]
   void test_pod_serializers_1()
   {
      bool POD_text_mode = false;

      TEST_POD(short, 0);
      TEST_POD(short, 42);
      TEST_POD(short, -21);

      TEST_POD(signed short, 0);
      TEST_POD(signed short, 42);
      TEST_POD(signed short, -21);

      TEST_POD(unsigned short, 0);
      TEST_POD(unsigned short, 42);

      TEST_POD(int, 0);
      TEST_POD(int, 42);
      TEST_POD(int, -21);
      TEST_POD(int, INT_MAX);
      TEST_POD(int, INT_MIN);

      TEST_POD(signed int, 0);
      TEST_POD(signed int, 42);
      TEST_POD(signed int, -21);

      TEST_POD(unsigned int, 0);
      TEST_POD(unsigned int, 42);

      TEST_POD(long, 0);
      TEST_POD(long, 42);
      TEST_POD(long, -21);
      TEST_POD(long, LONG_MAX);
      TEST_POD(long, LONG_MIN);

      TEST_POD(signed long, 0);
      TEST_POD(signed long, 42);
      TEST_POD(signed long, -21);

      TEST_POD(unsigned long, 0);
      TEST_POD(unsigned long, 42);
   }

   void test_pod_serializers_2()
   {
      bool POD_text_mode = false;

      TEST_POD(float, 0);
      TEST_POD(float, 3.1415f);

      TEST_POD(double, 0);
      TEST_POD(double, 3.1415);

      TEST_POD(long double, 0);
      TEST_POD(long double, 3.1415l);

      TEST_POD(bool, true);
      TEST_POD(bool, false);

      TEST_POD(char, 'A');
      TEST_POD(char, '\n');
      TEST_POD(char, 127);

      TEST_POD(signed char, 0);
      TEST_POD(signed char, 42);
      TEST_POD(signed char, -21);

      TEST_POD(unsigned char, 0);
      TEST_POD(unsigned char, 42);

      TEST_POD(wchar_t, 'A');
      TEST_POD(wchar_t, '\n');

      TEST_POD_LEN(std::string, "", size_t(0));
      TEST_POD_LEN(std::string, "hello, world", size_t(12));
   }

   
   /// Test all POD text serializers [with both serial_transform() and
   /// Serialize() / Deserialize() interfaces]
   void test_pod_text_serializers_1()
   {
      bool POD_text_mode = true;

      TEST_POD(short, 0);
      TEST_POD(short, 42);
      TEST_POD(short, -21);

      TEST_POD(signed short, 0);
      TEST_POD(signed short, 42);
      TEST_POD(signed short, -21);

      TEST_POD(unsigned short, 0);
      TEST_POD(unsigned short, 42);

      TEST_POD(int, 0);
      TEST_POD(int, 42);
      TEST_POD(int, -21);
      TEST_POD(int, INT_MAX);
      TEST_POD(int, INT_MIN);

      TEST_POD(signed int, 0);
      TEST_POD(signed int, 42);
      TEST_POD(signed int, -21);

      TEST_POD(unsigned int, 0);
      TEST_POD(unsigned int, 42);

      TEST_POD(long, 0);
      TEST_POD(long, 42);
      TEST_POD(long, -21);
      TEST_POD(long, LONG_MAX);
      TEST_POD(long, LONG_MIN);

      TEST_POD(signed long, 0);
      TEST_POD(signed long, 42);
      TEST_POD(signed long, -21);

      TEST_POD(unsigned long, 0);
      TEST_POD(unsigned long, 42);
   }

   void test_pod_text_serializers_2()
   {
      bool POD_text_mode = true;

      TEST_POD(float, 0);
      TEST_POD(float, 3.1415f);

      TEST_POD(double, 0);
      TEST_POD(double, 3.1415);

      TEST_POD(long double, 0);
      TEST_POD(long double, 3.1415l);

      TEST_POD(bool, true);
      TEST_POD(bool, false);

      TEST_POD(char, 'A');
      TEST_POD(char, '\n');
      TEST_POD(char, 31);
      TEST_POD(char, 32);
      TEST_POD(char, 126);
      TEST_POD(char, 127);

      TEST_POD(signed char, 0);
      TEST_POD(signed char, 42);
      TEST_POD(signed char, -21);

      TEST_POD(unsigned char, 0);
      TEST_POD(unsigned char, 42);

      TEST_POD(wchar_t, 'A');
      TEST_POD(wchar_t, '\n');

      TEST_POD(std::string, "");
      TEST_POD(std::string, "hello, world");
   }
   
   /// Test all STL container serializers [with both serial_transform()
   /// and Serialize() / Deserialize() interfaces]
   void test_stl_serializers_1()
   {
      // basic containers
      TEST_STL_driver_1(std::vector<int>, 0, 42, 21);
      TEST_STL_driver_1(std::list<int>, 0, 42, 21);

      // associative containers
      TEST_STL_driver_1(std::set<int>, 0, 42, 21);
      TEST_STL_driver_1(std::multiset<int>, 0, 42, 21);
      TEST_STL_driver_1(std::multiset<int>, 0, 42, 0);

      typedef std::map<int, int>  map_t;
      typedef std::multimap<int, int>  multimap_t;
      typedef std::pair<const int, int>  p_t;
      TEST_STL_driver_1( map_t, p_t(0,1), p_t(42, 43), p_t(21,20) );
      TEST_STL_driver_1( multimap_t, p_t(0,1), p_t(42, 43), p_t(21,20) );
      TEST_STL_driver_1( multimap_t, p_t(0,1), p_t(0, 43), p_t(0,20) );
   }

   void test_stl_serializers_3()
   {
      // special sequence containers
      TEST_STL_driver_2(std::deque<int>, 0, 42, 21, push_back, false);
      TEST_STL_driver_2(std::queue<int>, 0, 42, 21, push, true);
      TEST_STL_driver_2(std::stack<int>, 0, 42, 21, push, true);
   
      // priority_queue doesn't support operator==, so we have to do
      // this by hand
      {
         std::priority_queue<int> val;
         TEST_STL(std::priority_queue<int>, val, true, PQueue_Equals);
         
         val.push(0);
         val.push(42);
         val.push(21);
         TEST_STL(std::priority_queue<int>, val, true, PQueue_Equals);
      }

      // test a complex container
      {
         typedef std::map<std::pair<int, double>, std::vector<double> > 
            complex_t;
         complex_t val;
         TEST_STL(complex_t, val, false, TS_ASSERT_EQUALS);
         val[std::make_pair(0, 2)];
         val[std::make_pair(0, 3)].push_back(1);
         val[std::make_pair(0, 3)].push_back(10);
         val[std::make_pair(0, 3)].push_back(5);
         TEST_STL(complex_t, val, false, TS_ASSERT_EQUALS);
      }
   }

   void test_utilib_adt()
   {
      utilib::Ereal<float> er = 5;
      TEST_OBJ(utilib::Ereal<float>, er);
      er = utilib::Ereal<float>::positive_infinity;
      TEST_OBJ(utilib::Ereal<float>, er);

      utilib::BasicArray<double> ba;
      TEST_OBJ(utilib::BasicArray<double>, ba);
      ba.push_back(1.1);
      ba.push_back(2.2);
      ba.push_back(3.3);
      TEST_OBJ(utilib::BasicArray<double>, ba);
   
      utilib::NumArray<int> na;
      TEST_OBJ(utilib::NumArray<int>, na);
      na.push_back(3);
      na.push_back(5);
      na.push_back(7);
      TEST_OBJ(utilib::NumArray<int>, na);

      utilib::MixedIntVars miv;
      TEST_OBJ(utilib::MixedIntVars, miv);
      miv.Binary().resize(3);
      miv.Binary().set(1);
      miv.Integer().push_back(3);
      miv.Integer().push_back(5);
      miv.Integer().push_back(7);
      miv.Real().push_back(3.1415);
      miv.Real().push_back(42);
      TEST_OBJ(utilib::MixedIntVars, miv);
   }

   
   void test_print()
   {
      std::stringstream ss;

      /*
       * A simple POD
       */
      int i = 5;
      Serialize(i, false).print(ss);
      if ( utilib::Serialization_Manager::Endian == 0xE4 )
      {
         TS_ASSERT_EQUALS(ss.str(), "type = int\n   POD: 4: 5 0 0 0\n");
      }
      else if ( utilib::Serialization_Manager::Endian == 0x1B )
      {
         TS_ASSERT_EQUALS(ss.str(), "type = int\n   POD: 4: 0 0 0 5\n");
      }
      else
         TS_FAIL("Unrecognized endianness");

      ss.str("");
      Serialize(i, true).print(ss);
      TS_ASSERT_EQUALS(ss.str(), "type = int: 5\n");

      /*
       * A simple STL container
       */

      std::list<int> l;
      ss.str("");
      Serialize(l, false).print(ss);
      TS_ASSERT_EQUALS(ss.str(), "type = std::list<int,*>\n");

      ss.str("");
      Serialize(l, true).print(ss);
      TS_ASSERT_EQUALS(ss.str(), "type = std::list<int,*>\n");

      l.push_back(5);
      l.push_back(10);

      ss.str("");
      Serialize(l, false).print(ss);
      if ( utilib::Serialization_Manager::Endian == 0xE4 )
      {
         TS_ASSERT_EQUALS(ss.str(), "type = std::list<int,*>\n"
                          "   type = int\n      POD: 4: 5 0 0 0\n"
                          "   type = int\n      POD: 4: 10 0 0 0\n");
      }
      else if ( utilib::Serialization_Manager::Endian == 0x1B )
      {
         TS_ASSERT_EQUALS(ss.str(), "type = std::list<int,*>\n"
                          "   type = int\n      POD: 4: 0 0 0 5\n"
                          "   type = int\n      POD: 4: 0 0 0 10\n");
      }
      else
         TS_FAIL("Unrecognized endianness");

      ss.str("");
      Serialize(l, true).print(ss);
      TS_ASSERT_EQUALS( ss.str(), "type = std::list<int,*>\n"
                        "   type = int: 5\n   type = int: 10\n" );

      /*
       * A complex STL container
       */

      std::map<std::pair<int,double>, std::string> cdt;
      ss.str("");
      Serialize(cdt, false).print(ss);
      TS_ASSERT_EQUALS(ss.str(), "type = std::map<std::pair<int,double>,"
                       "std::string,*,*>\n");

      ss.str("");
      Serialize(cdt, true).print(ss);
      TS_ASSERT_EQUALS(ss.str(), "type = std::map<std::pair<int,double>,"
                       "std::string,*,*>\n");

      cdt[std::make_pair(1, 5)] = "";
      cdt[std::make_pair(3, 3.14)] = "hello";

      ss.str("");
      Serialize(cdt, true).print(ss);
      TS_ASSERT_EQUALS
         (ss.str(), 
          "type = std::map<std::pair<int,double>,std::string,*,*>\n"
          "   type = std::pair<std::pair<int,double>,std::string>\n"
          "      type = std::pair<int,double>\n"
          "         type = int: 1\n"
          "         type = double: 5\n"
          "      type = std::string: \"\"\n"
          "   type = std::pair<std::pair<int,double>,std::string>\n"
          "      type = std::pair<int,double>\n"
          "         type = int: 3\n"
          "         type = double: 3.1400000000000001\n"
          "      type = std::string: \"hello\"\n");

      /*
       * A character
       */
      char c = 'A';
      ss.str("");
      Serialize(c, false).print(ss);
      TS_ASSERT_EQUALS(ss.str(), "type = char\n   POD: 1: 65\n");

      ss.str("");
      Serialize(c, true).print(ss);
      TS_ASSERT_EQUALS(ss.str(), "type = char: 'A'\n");

      c = ' ';
      ss.str("");
      Serialize(c, false).print(ss);
      TS_ASSERT_EQUALS(ss.str(), "type = char\n   POD: 1: 32\n");

      ss.str("");
      Serialize(c, true).print(ss);
      TS_ASSERT_EQUALS(ss.str(), "type = char: ' '\n");

      c = 31;
      ss.str("");
      Serialize(c, false).print(ss);
      TS_ASSERT_EQUALS(ss.str(), "type = char\n   POD: 1: 31\n");

      ss.str("");
      Serialize(c, true).print(ss);
      TS_ASSERT_EQUALS(ss.str(), "type = char: 31\n");

      c = '~';
      ss.str("");
      Serialize(c, false).print(ss);
      TS_ASSERT_EQUALS(ss.str(), "type = char\n   POD: 1: 126\n");

      ss.str("");
      Serialize(c, true).print(ss);
      TS_ASSERT_EQUALS(ss.str(), "type = char: '~'\n");

      c = 127;
      ss.str("");
      Serialize(c, false).print(ss);
      TS_ASSERT_EQUALS(ss.str(), "type = char\n   POD: 1: 127\n");

      ss.str("");
      Serialize(c, true).print(ss);
      TS_ASSERT_EQUALS(ss.str(), "type = char: 127\n");

      ss.str("");
      SerialObject so = Serialize(int(1));
      so.data = utilib::Any();
      so.print(ss);
      std::string ref = "type = int\n   UNKNOWN DATA! ("
         + utilib::demangledName(typeid(void)) + ")\n";
      TS_ASSERT_EQUALS(ss.str(), ref);
   }


   void test_typeRegistry()
   {
      // check for simple int
      size_t int_key = Serializer().get_keyid("int");
      TS_ASSERT_EQUALS(int_key, Serializer().get_keyid(typeid(int)));
      TS_ASSERT_EQUALS("int", Serializer().get_username(int_key));
      TS_ASSERT(Serializer().is_pod(int_key));
      TS_ASSERT_EQUALS(Serializer().get_pod_length(int_key), (int)sizeof(int));
      TS_ASSERT_EQUALS(Serializer().get_typeinfo(int_key), &typeid(int));

      // check for complex type
      typedef std::map<int,double> cdt_t;
      // declare & use the variable so it gets registered...
      cdt_t foo;
      Serialize(foo);

      std::string cdt_name = "std::map<int,double,*,*>";
      size_t cdt_key = Serializer().get_keyid(typeid(cdt_t));
      TS_ASSERT_EQUALS( cdt_name, Serializer().get_username(cdt_key) );
      TS_ASSERT_EQUALS(cdt_key, Serializer().get_keyid(cdt_name));
      TS_ASSERT(! Serializer().is_pod(cdt_key));
      TS_ASSERT_EQUALS(Serializer().get_pod_length(cdt_key), -1);
      TS_ASSERT_EQUALS(Serializer().get_typeinfo(cdt_key), &typeid(cdt_t));

      // check for unknown type
      typedef UnknownClass u_t;
      std::string u_name = "UnknownClass";
      size_t u_key = 1000000;
      TS_ASSERT_THROWS_ASSERT
         ( Serializer().get_keyid(typeid(u_t)), 
           utilib::serializer_unknown_type &e,
           TEST_WHAT(e, "Serialization_Manager::get_keyid(): "
                     "unknown type_info name, \"") );
      TS_ASSERT_EQUALS(Serializer().get_username(u_key), "");
      TS_ASSERT_THROWS_ASSERT
         ( Serializer().get_keyid(u_name), 
           utilib::serializer_unknown_type &e,
           TEST_WHAT(e, "Serialization_Manager::get_keyid(): "
                     "unknown user-defined type name, \"UnknownClass\"") );
      TS_ASSERT(! Serializer().is_pod(u_key));
      TS_ASSERT_EQUALS(Serializer().get_pod_length(u_key), -1);
      TS_ASSERT( ! Serializer().get_typeinfo(u_key) );
   }

   
   void test_listSerializers()
   {
      utilib::Serialization_Manager sm;
      std::stringstream ss;
      sm.list_serializers(ss);

      // we need to strip out all the mangled names, as those can change
      // across compilers/platforms.
      std::string list = ss.str();
      size_t start;
      while ( (start  = list.find("[ ")) != std::string::npos )
         list.erase(start+1, list.find(" ]") - start);

      TS_ASSERT_EQUALS(list, std::string("Known serializers:\n"
                                         "   bool             []\n"
                                         "   char             []\n"
                                         "   double           []\n"
                                         "   float            []\n"
                                         "   int              []\n"
                                         "   long             []\n"
                                         "   long double      []\n"
                                         "   short            []\n"
                                         "   signed char      []\n"
                                         "   std::string      []\n"
                                         "   unsigned char    []\n"
                                         "   unsigned int     []\n"
                                         "   unsigned long    []\n"
                                         "   unsigned short   []\n"
                                         "   void             []\n"
                                         "   wchar_t          []\n"));
   }


   void test_POD_serializer_errors()
   {
      SerialObject::elementList_t so_list;
      char c = 0;
      utilib::Any a;

      /*
       * int 
       */
      SerialObject so = Serialize(5);

      // make sure that the text serializer correctly handles whitespace
      so.data.expose<SerialPOD>().set(std::string(" 2  "));
      TS_ASSERT_EQUALS(Deserialize(so).expose<int>(), 2);

      // change the size of the int
      so.data.expose<SerialPOD>().set(&c, sizeof(char));
      TS_ASSERT_THROWS_ASSERT
         ( Deserialize(so), utilib::serializer_bad_pod_size &e,
           TEST_WHAT(e, "POD_serializer(): "
                     "SerialPOD data size does not match destination type") );

      // change the int to text mode with an invalid string
      so.data.expose<SerialPOD>().set(std::string("a"));
      so_list.push_back(so);
      TS_ASSERT_EQUALS(serial_transform(so_list, a, false), 
                       utilib::error::Serialization::BadPODTextConversion);
      TS_ASSERT_THROWS_ASSERT
         ( Deserialize(so), utilib::serialization_error &e,
           TEST_WHAT(e, "Serialization_Manager::Deserialize()"
                     ": Deserialization failed for '") );

      so.data.expose<SerialPOD>().set(std::string("1.5"));
      so_list.push_back(so);
      TS_ASSERT_EQUALS(serial_transform(so_list, a, false), 
                       utilib::error::Serialization::UnconvertedPODText);
      a.clear();
      TS_ASSERT_THROWS_ASSERT
         ( Deserialize(so), utilib::serialization_error &e,
           TEST_WHAT(e, "Serialization_Manager::Deserialize()"
                     ": Deserialization failed for '") );

      /*
       * float
       */

      so = Serialize(3.14f);

      so.data.expose<SerialPOD>().set(std::string("  2.718  "));
      TS_ASSERT_EQUALS(Deserialize(so).expose<float>(), 2.718f);

      so.data.expose<SerialPOD>().set(std::string("a"));
      so_list.push_back(so);
      TS_ASSERT_EQUALS(serial_transform(so_list, a, false), 
                       utilib::error::Serialization::BadPODTextConversion);
      TS_ASSERT_THROWS_ASSERT
         ( Deserialize(so), utilib::serialization_error &e,
           TEST_WHAT(e, "Serialization_Manager::Deserialize()"
                     ": Deserialization failed for '") );

      so.data.expose<SerialPOD>().set(std::string("1.5b"));
      so_list.push_back(so);
      TS_ASSERT_EQUALS(serial_transform(so_list, a, false), 
                       utilib::error::Serialization::UnconvertedPODText);
      a.clear();
      TS_ASSERT_THROWS_ASSERT
         ( Deserialize(so), utilib::serialization_error &e,
           TEST_WHAT(e, "Serialization_Manager::Deserialize()"
                     ": Deserialization failed for '") );

      /*
       * double
       */

      so = Serialize(3.14);

      so.data.expose<SerialPOD>().set(std::string("  2.718  "));
      TS_ASSERT_EQUALS(Deserialize(so).expose<double>(), 2.718);

      so.data.expose<SerialPOD>().set(std::string("a"));
      so_list.push_back(so);
      TS_ASSERT_EQUALS(serial_transform(so_list, a, false), 
                       utilib::error::Serialization::BadPODTextConversion);
      TS_ASSERT_THROWS_ASSERT
         ( Deserialize(so), utilib::serialization_error &e,
           TEST_WHAT(e, "Serialization_Manager::Deserialize()"
                     ": Deserialization failed for '") );

      so.data.expose<SerialPOD>().set(std::string("1.5b"));
      so_list.push_back(so);
      TS_ASSERT_EQUALS(serial_transform(so_list, a, false), 
                       utilib::error::Serialization::UnconvertedPODText);
      a.clear();
      TS_ASSERT_THROWS_ASSERT
         ( Deserialize(so), utilib::serialization_error &e,
           TEST_WHAT(e, "Serialization_Manager::Deserialize()"
                     ": Deserialization failed for '") );

      /*
       * long double
       */

      so = Serialize(3.14l);

      so.data.expose<SerialPOD>().set(std::string("  2.718  "));
      TS_ASSERT_EQUALS(Deserialize(so).expose<long double>(), 2.718l);

      so.data.expose<SerialPOD>().set(std::string("a"));
      so_list.push_back(so);
      TS_ASSERT_EQUALS(serial_transform(so_list, a, false), 
                       utilib::error::Serialization::BadPODTextConversion);
      TS_ASSERT_THROWS_ASSERT
         ( Deserialize(so), utilib::serialization_error &e,
           TEST_WHAT(e, "Serialization_Manager::Deserialize()"
                     ": Deserialization failed for '") );

      so.data.expose<SerialPOD>().set(std::string("1.5b"));
      so_list.push_back(so);
      TS_ASSERT_EQUALS(serial_transform(so_list, a, false), 
                       utilib::error::Serialization::UnconvertedPODText);
      a.clear();
      TS_ASSERT_THROWS_ASSERT
         ( Deserialize(so), utilib::serialization_error &e,
           TEST_WHAT(e, "Serialization_Manager::Deserialize()"
                     ": Deserialization failed for '") );


      /*
       * std::string
       */
      so = Serialize(std::string(""));

      so.data.expose<SerialPOD>().set(std::string());
      so_list.push_back(so);
      TS_ASSERT_EQUALS(serial_transform(so_list, a, false), 
                       utilib::error::Serialization::MissingStringQuote);
      a.clear();

      so.data.expose<SerialPOD>().set(std::string("A"));
      so_list.push_back(so);
      TS_ASSERT_EQUALS(serial_transform(so_list, a, false), 
                       utilib::error::Serialization::MissingStringQuote);
      a.clear();

      so.data.expose<SerialPOD>().set(std::string("\""));
      so_list.push_back(so);
      TS_ASSERT_EQUALS(serial_transform(so_list, a, false), 
                       utilib::error::Serialization::MissingStringQuote);
      a.clear();

      so.data.expose<SerialPOD>().set(std::string("\"a"));
      so_list.push_back(so);
      TS_ASSERT_EQUALS(serial_transform(so_list, a, false), 
                       utilib::error::Serialization::MissingStringQuote);
      a.clear();

      /*
       * char
       */

      so = Serialize('a');

      so.data.expose<SerialPOD>().set(std::string());
      so_list.push_back(so);
      TS_ASSERT_EQUALS(serial_transform(so_list, a, false), 
                       utilib::error::Serialization::BadPODData);
      a.clear();

      so.data.expose<SerialPOD>().set(std::string("'A"));
      so_list.push_back(so);
      TS_ASSERT_EQUALS(serial_transform(so_list, a, false), 
                       utilib::error::Serialization::BadPODData);
      a.clear();

      so.data.expose<SerialPOD>().set(std::string("'Ab"));
      so_list.push_back(so);
      TS_ASSERT_EQUALS(serial_transform(so_list, a, false), 
                       utilib::error::Serialization::BadPODData);
      a.clear();

      so.data.expose<SerialPOD>().set(std::string("257"));
      so_list.push_back(so);
      TS_ASSERT_EQUALS(serial_transform(so_list, a, false), 
                       utilib::error::Serialization::BadPODTextConversion);
      a.clear();

      so.data.expose<SerialPOD>().set(std::string("\"a\""));
      so_list.push_back(so);
      TS_ASSERT_EQUALS(serial_transform(so_list, a, false), 
                       utilib::error::Serialization::BadPODTextConversion);
      a.clear();

      so.data.expose<SerialPOD>().set(std::string("32a"));
      so_list.push_back(so);
      TS_ASSERT_EQUALS(serial_transform(so_list, a, false), 
                       utilib::error::Serialization::UnconvertedPODText);
      a.clear();
   }

   void test_serialization_errors()
   {
      // This is going to work with a non-global serializer, so we will
      // bypass the normal Serialize(), Deserialize(), and
      // serial_transform functions...

      utilib::Serialization_Manager sm;
      SerialObject::elementList_t so_list;

      // Attempt to serialize an unregistered class
      UnknownClass uc;
      utilib::AnyFixedRef data = uc;
      TS_ASSERT_THROWS_ASSERT
         ( sm.transform_impl(data.type(), so_list, data, true),
           utilib::serializer_unknown_type &e,
           TEST_WHAT(e, "Serialization_Manager::transform_impl(): "
                     "cannot serialize unknown type '") );

      // Attempt to deserialize an empty list
      TS_ASSERT_THROWS_ASSERT
         ( sm.transform_impl(data.type(), so_list, data, false),
           utilib::serialization_error &e,
           TEST_WHAT(e, "Serialization_Manager::transform_impl(): "
                     "SerialObject missing required element.") );
      

      // Register a completely bogus serialization
      sm.register_serializer<UnknownClass>( "MyClass", NULL, NULL, -1, NULL );

      // Still cannot serialize
      TS_ASSERT_THROWS_ASSERT
         ( sm.transform_impl(data.type(), so_list, data, true),
           utilib::serialization_error &e,
           TEST_WHAT(e, "Serialization_Manager::transform_impl(): "
                     "NULL serialization function for type ") );
      // ...nor deserialize
      TS_ASSERT_THROWS_ASSERT
         ( sm.transform_impl(data.type(), so_list, data, false),
           utilib::serialization_error &e,
           TEST_WHAT(e, "Serialization_Manager::transform_impl(): "
                     "NULL initialization function for id ") );

      // try to re-register with a valid initializer...
      utilib::OStreamTee tee(std::cerr);
      std::stringstream ref;
      ref << "WARNING: Serialization_Manager::register_serializer():  "
         "discarding duplicate registration for '" << 
         utilib::mangledName(typeid(UnknownClass)) << "'" << std::endl;
      TS_ASSERT_EQUALS( sm.register_serializer<UnknownClass>
                        ( "MyClass", NULL, &NullInitializer ),
                        utilib::error::Serialization::DuplicateRegistration );
      TS_ASSERT_EQUALS(tee.out.str(), ref.str())

      // OK, how about an invalid initializer?
      sm = utilib::Serialization_Manager();
      sm.register_serializer<UnknownClass>
         ( "MyClass", NULL, &NullInitializer );

      // ...still can't deserialize
      tee.out.str("");
      ref.str("");
      ref << "WARNING: Serialization_Manager::transform_impl(): initialization function for " << 
         utilib::mangledName(typeid(UnknownClass)) << " resulted in an empty Any." << std::endl << "         This will likely cause random segmentation faults." << std::endl;
      TS_ASSERT_THROWS_ASSERT
         ( sm.transform_impl(data.type(), so_list, data, false),
           utilib::serialization_error &e,
           TEST_WHAT(e, "Serialization_Manager::transform_impl(): "
                     "NULL deserialization function for id ") );
      TS_ASSERT_EQUALS(tee.out.str(), ref.str())
    
      // OK, how about a reasonable initializer?
      sm = utilib::Serialization_Manager();
      sm.register_serializer<UnknownClass>
         ( "MyClass", NULL,
           &utilib::Serialization_Manager::DefaultInitializer<UnknownClass> );

      // ...still can't deserialize
      TS_ASSERT_THROWS_ASSERT
         ( sm.transform_impl(data.type(), so_list, data, false),
           utilib::serialization_error &e,
           TEST_WHAT(e, "Serialization_Manager::transform_impl(): "
                     "NULL deserialization function for id ") );
    
      // what about conflicting usernames?
      sm.register_serializer<AnotherClass>
         ( "MyClass", NULL, 
           &utilib::Serialization_Manager::DefaultInitializer<UnknownClass> );

      tee.out.str("");
      ref.str("");
      ref << "WARNING: Serialization_Manager::resolve_user_name(): " 
          << std::endl << "     Multiple mangled type names map to the same "
         "user-defined name.  It is" << std::endl << "     likely that you "
         "forgot to register a name or a serialization function" << std::endl 
          << "     for a template argument.  If you attempt to serialize "
         "either type," << std::endl << "     you will get an exception." 
          << std::endl << "  User name: MyClass" << std::endl << "  Mangled: "
          << utilib::mangledName(typeid(UnknownClass)) << std::endl <<
         "           " 
          << utilib::mangledName(typeid(AnotherClass)) << std::endl;

      TS_ASSERT_THROWS_ASSERT
         ( sm.get_keyid("MyClass"), utilib::serializer_unknown_type &e,
           TEST_WHAT(e, "Serialization_Manager::get_keyid(): "
                     "user-defined type name, \"MyClass\" "
                     "maps to multiple types") );
      TS_ASSERT_EQUALS(tee.out.str(), ref.str())

      TS_ASSERT_THROWS_ASSERT
         ( sm.transform_impl(data.type(), so_list, data, true),
           utilib::serialization_error &e,
           TEST_WHAT(e, "Serialization_Manager::transform_impl(): attempt "
                     "to serialize an object with a nonunique username."));

      TS_ASSERT_THROWS_ASSERT
         ( sm.transform_impl(data.type(), so_list, data, false),
           utilib::serialization_error &e,
           TEST_WHAT(e, "Serialization_Manager::transform_impl(): attempt "
                     "to deserialize an object with a nonunique username."));

      // create an invalid type id
      so_list.front().type+=2;
      TS_ASSERT_THROWS_ASSERT
         ( sm.transform_impl(data.type(), so_list, data, false),
           utilib::serialization_error &e,
           TEST_WHAT(e, "Serialization_Manager::transform_impl(): "
                     "SerialObject contains unknown type id.") );
   }
};

} // namespace utilib
