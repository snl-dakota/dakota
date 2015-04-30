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

#include <iostream>
#include <deque>

using utilib::Serializer;
using utilib::Serialize;
using utilib::SerialObject;
using utilib::SerialPOD;
using utilib::Any;

using utilib::BasicArray;
using utilib::NumArray;
using utilib::Ereal;
using utilib::MixedIntVars;

using std::endl;
using std::setw;
using std::ostream;
using std::string;

using std::pair;
using std::list;
using std::vector;
using std::deque;
using std::queue;
using std::stack;
using std::priority_queue;
using std::set;
using std::multiset;
using std::map;
using std::multimap;

#define OUT std::cout
#define QUOTE(X) _QUOTE(X)
#define _QUOTE(X) #X

#define DESCRIBE_TYPE(TYPE)                             \
   OUT << "    " << setw(22) << QUOTE(TYPE) << ":  "

#define DESCRIBE_SO(SO)                                                 \
   OUT << "(";                                                          \
   if ( print_type_keyid )                                              \
      OUT << setw(2) << SO.type << ",";                                 \
   else                                                                 \
      OUT << "--,";                                                     \
   if ( SO.data.is_type(typeid(SerialPOD)) )                            \
   {                                                                    \
      if ( print_type_keyid )                                           \
         OUT << setw(2) << SO.data.expose<SerialPOD>().size();          \
      else                                                              \
         OUT << "nn";                                                   \
   }                                                                    \
   else if ( SO.data.is_type(typeid(SerialObject::elementList_t)) )     \
      OUT << "C" << SO.data.expose<SerialObject::elementList_t>().size(); \
   else                                                                 \
      OUT << "--";                                                      \
   OUT << ")"

#define DESCRIBE_TEST()                                                 \
   OUT << ": [" << ( test_tmp == test_ans ? " OK ]  " : "FAIL]  " );    \
   OUT << "(";                                                          \
   print(OUT, test_ans);                                                \
   OUT << ")" << endl


#define TEST_1(VAL, TYPE)                                       \
   do {                                                         \
      SerialObject::elementList_t test_so;                      \
      TYPE test_tmp = VAL;                                      \
      TYPE test_ans;                                            \
      DESCRIBE_TYPE(TYPE);                                      \
      OUT << serial_transform(test_so, test_tmp, true) << ",";  \
      SerialObject &tmp_so = test_so.front();                   \
      DESCRIBE_SO(tmp_so);                                      \
      OUT << "," << serial_transform(test_so, test_ans, false); \
      DESCRIBE_TEST();                                          \
   } while ( false )

#define TEST_2(VAL, TYPE)                                               \
   do {                                                                 \
      TYPE test_tmp = VAL;                                              \
      DESCRIBE_TYPE(TYPE);                                              \
      OUT << "  ";                                                      \
      SerialObject test_so = Serialize(test_tmp);                       \
      DESCRIBE_SO(test_so);                                             \
      OUT << "  ";                                                      \
      TYPE test_ans                                                     \
         = utilib::Deserialize(test_so).expose<TYPE >();                \
      DESCRIBE_TEST();                                                  \
   } while ( false )

#define TEST_2_ANY(VAL, TYPE)                                   \
   do {                                                         \
      TYPE test_tmp = VAL;                                      \
      DESCRIBE_TYPE(TYPE);                                      \
      OUT << "  ";                                              \
      SerialObject test_so = Serialize(test_tmp);               \
      DESCRIBE_SO(test_so);                                     \
      OUT << "  ";                                              \
      TYPE test_ans = utilib::Deserialize(test_so);             \
      DESCRIBE_TEST();                                          \
   } while ( false )


#define TEST_POD_1(VAL, TYPE) TEST_1(VAL, TYPE)
#define TEST_POD_2(VAL, TYPE) TEST_2(VAL, TYPE)

#define TEST_STL_1(VAL, TYPE) TEST_1(VAL, TYPE)
#define TEST_STL_2(VAL, TYPE) TEST_2(VAL, TYPE)

#define TEST_ANY_1(VAL, TYPE) TEST_1(VAL, TYPE)
#define TEST_ANY_2(VAL, TYPE) TEST_2_ANY(VAL, TYPE)

template<typename T1, typename T2>
std::ostream& print(std::ostream& os, const std::pair<T1, T2> &data);

template<typename T>
std::ostream& print(std::ostream& os, const T& stl)
{
   typename T::const_iterator it = stl.begin();
   os << "[ " << stl.size() << ": ";
   for ( ; it != stl.end(); ++it )
      print(os, *it) << " ";
   os << "]";
   return os;
}

template<>
std::ostream& print(std::ostream& os, const bool &data)
{ return os << data; }

template<>
std::ostream& print(std::ostream& os, const char &data)
{ return os << data; }

template<>
std::ostream& print(std::ostream& os, const wchar_t &data)
{ return os << data; }

template<>
std::ostream& print(std::ostream& os, const int &data)
{ return os << data; }

template<>
std::ostream& print(std::ostream& os, const long int &data)
{ return os << data; }

template<>
std::ostream& print(std::ostream& os, const float &data)
{ return os << data; }

template<>
std::ostream& print(std::ostream& os, const double &data)
{ return os << data; }

template<>
std::ostream& print(std::ostream& os, const std::string &data)
{ return os << data; }


template<typename T>
std::ostream& print(std::ostream& os, const utilib::Ereal<T> &data)
{ return os << data; }

template<>
std::ostream& print(std::ostream& os, const utilib::MixedIntVars &data)
{ return os << data; }

template<>
std::ostream& print(std::ostream& os, const utilib::Any &data)
{ return os << data; }


template<typename T1, typename T2>
std::ostream& print(std::ostream& os, const std::pair<T1, T2> &data)
{
   os << "(";
   print(os, data.first) << ",";
   return print(os, data.second) << ")";
}

template<typename T1, typename T2>
std::ostream& print(std::ostream& os, const queue<T1,T2>& stl)
{
   queue<T1,T2> tmp = stl;
   os << "[ ";
   for ( ; ! tmp.empty(); tmp.pop() )
      print(os, tmp.front()) << " ";
   os << "]";
   return os;
}

template<typename T1, typename T2>
std::ostream& print(std::ostream& os, const stack<T1,T2>& stl)
{
   stack<T1,T2> tmp = stl;
   os << "[ ";
   for ( ; ! tmp.empty(); tmp.pop() )
      print(os, tmp.top()) << " ";
   os << "]";
   return os;
}

template<typename T1, typename T2, typename T3>
std::ostream& print(std::ostream& os, const priority_queue<T1,T2,T3>& stl)
{
   priority_queue<T1,T2,T3> tmp = stl;
   os << "[ ";
   for ( ; ! tmp.empty(); tmp.pop() )
      print(os, tmp.top()) << " ";
   os << "]";
   return os;
}



template<typename T1, typename T2, typename T3>
bool operator==(const priority_queue<T1,T2,T3> &lhs,
                const priority_queue<T1,T2,T3> &rhs)
{
   priority_queue<T1,T2,T3> a = lhs;
   priority_queue<T1,T2,T3> b = rhs;
   while ( true )
   {
      if ( a.empty() && b.empty() )
         return true;
      if ( a.empty() || b.empty() )
         return false;
      if ( a.top() != b.top() )
         return false;
      a.pop();
      b.pop();
   }
}


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
   std::ostream& printer(std::ostream& os) const
   {
      os << "{";
      print(os, a) << ", ";
      print(os, b) << ", ";
      return print(os, c) << "}";
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
   = Serializer().register_serializer<A>("A", A::serializer);


/// Defining a class that has no default constructor and embeds another class
class B
{
public:
   B(int x)
      : a1(x, x+0.5f), a2(x+1, x+1.5f)
   {}
   std::ostream& printer(std::ostream& os) const
   {
      os << "{";
      a1.printer(os) << ", ";
      a2.printer(os) << "}";
      return os;
   }
   bool operator==(const B& rhs) const
   {
      return ( a1 == rhs.a1 ) && ( a2 == rhs.a2 );
   }
   bool operator<(const B& rhs) const
   {
      return a1 < rhs.a1; // bogus... I know...
   }

private:
   static void initializer(Any& value)
   {
      value.set(B(0));
   }

   static int serializer(SerialObject::elementList_t& serial, 
                         Any& data, 
                         bool serialize)
   {
      B& me = const_cast<B&>(data.expose<B>());
      serial_transform(serial, me.a1, serialize);
      serial_transform(serial, me.a2, serialize);
      return 0;
   }

private:
   static const int serializer_registered;

   A a1;
   A a2;
};

// Register the serializer & explicit initializer for class B
const int B::serializer_registered
   = Serializer().register_serializer<B>("B", B::serializer, B::initializer);

} // namespace (local)


// support stream output for the testing macros
template<>
std::ostream& print(std::ostream& os, const B& data)
   { return data.printer(os); }

// support stream output for the testing macros
template<>
std::ostream& print(std::ostream& os, const A& data)
   { return data.printer(os); }

int test_serialize(int argc, char**)
{
   bool print_type_keyid = ( argc > 1 );
   if ( argc > 1 )
      Serializer().list_serializers(OUT);

   OUT << "Testing POD data:" << endl;
   TEST_POD_1(true, bool);
   TEST_POD_1(false, bool);
   TEST_POD_1('A', char);
   TEST_POD_1('A', wchar_t);
   TEST_POD_1(42, int);
   TEST_POD_1(123456, long);
   TEST_POD_1(3.1415f, float);
   TEST_POD_1(3.1415, double);
   TEST_POD_1("hello, world", string);

   OUT << "Testing POD data:" << endl;
   TEST_POD_2(true, bool);
   TEST_POD_2(false, bool);
   TEST_POD_2('A', char);
   TEST_POD_2('B', wchar_t);
   TEST_POD_2(42, int);
   TEST_POD_2(123456, long);
   TEST_POD_2(3.1415f, float);
   TEST_POD_2(3.1415, double);
   TEST_POD_2("hello, world", string);

   OUT << "Testing STL:" << endl;

   list<int> l;
   l.push_back(2);
   l.push_back(4);
   l.push_back(6);
   TEST_STL_1(l, list<int>);
   TEST_STL_2(l, list<int>);
   l.clear();
   TEST_STL_1(l, list<int>);
   TEST_STL_2(l, list<int>);

   vector<string> v;
   v.push_back("hello");
   v.push_back("world");
   TEST_STL_1(v, vector<string>);
   TEST_STL_2(v, vector<string>);

   deque<double> d;
   d.push_back(1.1);
   d.push_back(2.2);
   d.push_back(3.3);
   TEST_STL_1(d, deque<double>);
   TEST_STL_2(d, deque<double>);

   queue<double> q1;
   q1.push(10.1);
   q1.push(20.2);
   q1.push(30.3);
   TEST_STL_1(q1, queue<double>);
   TEST_STL_2(q1, queue<double>);

   queue<float> q2;
   q2.push(100.1f);
   q2.push(200.2f);
   q2.push(300.3f);
   TEST_STL_1(q2, queue<float>);
   TEST_STL_2(q2, queue<float>);

   stack<int> st;
   st.push(1);
   st.push(3);
   st.push(5);
   TEST_STL_1(st, stack<int>);
   TEST_STL_2(st, stack<int>);

   priority_queue<int> pq;
   pq.push(1);
   pq.push(3);
   pq.push(5);
   TEST_STL_1(pq, priority_queue<int>);
   TEST_STL_2(pq, priority_queue<int>);

   set<int> s;
   s.insert(1);
   s.insert(3);
   s.insert(5);
   TEST_STL_1(s, set<int>);
   TEST_STL_2(s, set<int>);

   multiset<int> ms;
   ms.insert(1);
   ms.insert(3);
   ms.insert(3);
   ms.insert(3);
   ms.insert(5);
   ms.insert(5);
   TEST_STL_1(ms, multiset<int>);
   TEST_STL_2(ms, multiset<int>);

   typedef map<int,double> map_int_double_t;
   map_int_double_t m;
   m[3] = 0.3;
   m[1] = 0.1;
   m[2] = 0.2;
   TEST_STL_1(m, map_int_double_t);
   TEST_STL_2(m, map_int_double_t);

   typedef multimap<int,double> multimap_int_double_t;
   multimap_int_double_t mm;
   mm.insert(pair<int,double>(3,0.3));
   mm.insert(pair<int,double>(3,0.31));
   mm.insert(pair<int,double>(3,0.29));
   mm.insert(pair<int,double>(1,0.1));
   mm.insert(pair<int,double>(5,0.5));
   mm.insert(pair<int,double>(5,0.5));
   TEST_STL_1(mm, multimap_int_double_t);
   TEST_STL_2(mm, multimap_int_double_t);

   OUT << "Testing Utilib ADT:" << endl;

   Ereal<float> er = 5;
   TEST_POD_1(er, Ereal<float>);
   TEST_POD_2(er, Ereal<float>);
   er = Ereal<float>::positive_infinity;
   TEST_POD_1(er, Ereal<float>);
   TEST_POD_2(er, Ereal<float>);

   BasicArray<double> ba;
   ba.push_back(1.1);
   ba.push_back(2.2);
   ba.push_back(3.3);
   TEST_STL_1(ba, BasicArray<double>);
   TEST_STL_2(ba, BasicArray<double>);
   
   NumArray<int> na;
   na.push_back(3);
   na.push_back(5);
   na.push_back(7);
   TEST_STL_1(na, NumArray<int>);
   TEST_STL_2(na, NumArray<int>);

   MixedIntVars miv;
   TEST_STL_1(miv, MixedIntVars);
   TEST_STL_2(miv, MixedIntVars);
   miv.Binary().resize(3);
   miv.Binary().set(1);
   miv.Integer().push_back(3);
   miv.Integer().push_back(5);
   miv.Integer().push_back(7);
   miv.Real().push_back(3.1415);
   miv.Real().push_back(42);
   TEST_STL_1(miv, MixedIntVars);
   TEST_STL_2(miv, MixedIntVars);

   OUT << "Testing Anys:" << endl;

   Any a1 = 5;
   TEST_ANY_1(a1, Any);
   TEST_ANY_2(a1, Any);

   float f = 25;
   utilib::AnyFixedRef a2 = f;
   TEST_ANY_1(a2, Any);
   TEST_ANY_2(a2, Any);

   Any a3;
   TEST_ANY_1(a3, Any);
   TEST_ANY_2(a3, Any);
   {
      // Make sure that deserializing VOID clears the result Any
      SerialObject::elementList_t test_so; 
      Any test_tmp = a3;
      Any test_ans = '?'; 
      DESCRIBE_TYPE(Any); 
      OUT << serial_transform(test_so, test_tmp, true) << ","; 
      SerialObject &tmp_so = test_so.front(); 
      DESCRIBE_SO(tmp_so); 
      OUT << "," << serial_transform(test_so, test_ans, false); 
      DESCRIBE_TEST(); 
   }

   OUT << "Testing nested classes:" << endl;

   typedef map<pair<int,float>, list<char> >  xx_t;
   xx_t xx;
   xx[pair<int,float>(1,0.5f)];
   xx[pair<int,float>(2,1.5f)].push_back('h');
   xx[pair<int,float>(2,1.5f)].push_back('i');
   xx[pair<int,float>(2,2.5f)].push_back('?');
   xx[pair<int,float>(3,33)].push_back('!');
   TEST_STL_1(xx, xx_t);
   TEST_STL_2(xx, xx_t);

   A o1(2,5.5);
   TEST_STL_1(o1, A);
   TEST_STL_2(o1, A);

   B o2(1);
   TEST_STL_2(o2, B);
   {
      OUT << "SerialObject::print():" << endl;
      SerialObject so = Serialize(o2, true);
      so.print(OUT, "   ");
      Any tmp = Deserialize(so);
   }
   {
      OUT << "SerialObject::print():" << endl;
      SerialObject so = Serialize(o2, false);
      so.print(OUT, "   ");
      Any tmp = Deserialize(so);
   }

   return 0;
}
