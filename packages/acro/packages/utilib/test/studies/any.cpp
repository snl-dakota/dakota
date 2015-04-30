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

#include <iostream>
#include <utilib/Any.h>

using std::cout;
using std::endl;
using std::setw;
using std::string;
using utilib::Any;
using utilib::AnyRef;
using utilib::AnyFixedRef;

#define OUT std::cout

namespace {

class FOO
  {
  public:
    FOO()
      { OUT << "Constructing FOO" << endl; }
    FOO(const FOO &rhs)
      {
      static_cast<void>(rhs);
      OUT << "Copy Constructing FOO" << endl; 
      }

    // equality test required by Any API
    //bool operator==(const FOO &rhs) const
    //  { return true; }
    // inequality test required by Any API
    //bool operator<(const FOO &rhs) const
    //  { return false; }
  };

}

/// Macro for incorporating macros into string constants
/** Macro pair to "stringify" any macro (Part 1).  This works by first
 *  quoting the macro name and then expanding the macro into it's
 *  actual value. [see _QUOT(x)]
 */
#define QUOTE(x) _QUOTE(x)
/** Macro pair to "stringify" any macro (Part 2).  This works by first
 *  quoting the macro name and then expanding the macro into it's
 *  actual value. [see QUOT(x)]
 */
#define _QUOTE(x) #x

/** Macro to "demangle" the type stored in the Any to a known (fixed)
 *  string.  That is, if the Any currently stores an instance of TYPE,
 *  then "TYPE" is returned, otherwise the mangled typename of the
 *  stored type is returned.  This prevents false failures caused by
 *  different platforms using different mangling rules.
 */
#define DEMANGLE_TYPE(ANY, TYPE) \
   ( strcmp( utilib::mangledName(ANY.type()), \
             utilib::mangledName(typeid(TYPE)) ) == 0   \
     ? QUOTE(TYPE) : utilib::mangledName(ANY.type()) )

#define PRINT_ANY(ANY, TYPE)      \
do {                              \
  int ans;                        \
  double d_val = 0;               \
  long   l_val = 0;               \
  int    i_val = 0;               \
  short  s_val = 0;               \
  OUT << QUOTE(ANY) ": type="  << setw(6) << DEMANGLE_TYPE(ANY,TYPE);   \
  ans = ANY.extract(d_val);       \
  OUT << ", d = " << setw(3) << d_val << " (" << setw(4) << ans << ")"; \
  ans = ANY.extract(i_val);       \
  OUT << ", i = " << setw(3) << i_val << " (" << setw(4) << ans << ")"; \
  ans = ANY.extract(s_val);       \
  OUT << ", s = " << setw(3) << s_val << " (" << setw(4) << ans << ")"; \
  ans = ANY.extract(l_val);       \
  OUT << ", l = " << setw(3) << l_val << " (" << setw(4) << ans << ")"; \
  OUT << endl;                   \
} while (false)

/** Macro to strip off the file name & line number from the exception
 *  message (so we don't have to edit the reference output
 *  (anyShallow.qa) every time we do *any* editing to the source code.
 *  Also strips off anything after and including the first "'" character
 *  to prevent comparison failures based on differing mangling rules
 */
#define CATCH(STR, EXCEPTION)   \
  catch (EXCEPTION err)         \
    {                           \
    string msg(err.what());     \
    size_t pos = msg.find_last_of("0123456789");  \
    if ( pos == string::npos )  \
      { pos = 0; }              \
    else                        \
      { ++pos; }                \
    size_t end = msg.substr(pos).find("'");  \
    OUT << STR << " failed" << msg.substr(pos, end) << endl;  \
    }                                        \
  catch (...)                                \
    { OUT << STR << " failed: (unexpected exception)" << endl; }  \
  do {} while (false)


void AnyRefTest(const AnyRef& myAny)
  {
  double &tmp = const_cast<double&>(myAny.expose<double>());
  OUT << tmp << " -> ";
  tmp++;
  OUT << tmp;
  }

void AnyFixedRefTest(AnyFixedRef myAny)
  {
  double &tmp = const_cast<double&>(myAny.expose<double>());
  OUT << tmp << " -> ";
  myAny = tmp + 2;
  OUT << tmp;
  }


int test_any(int,char**)
  {
  Any::throwCastExceptions() = false;
  utilib::exception_mngr::set_stack_trace(false);

  short s;
  int i;
  long l;
  double d;
  utilib::Any foo(5);
  utilib::Any bar = foo;
  const utilib::Any const_bar = 6.5;

  bar = 5.5;

  PRINT_ANY(foo, int);

  //foo.set(7.7);
  foo = 7.7;
  PRINT_ANY(foo, double);

  s = 7;
  //foo.set(s);
  foo = s;
  PRINT_ANY(foo, short);

  l = 8;
  //foo.set(l);
  foo = l;
  PRINT_ANY(foo, long);

  /* no longer valid as Any now stores CONST objects
  foo.expose<long>() = 9;
  PRINT_ANY(foo, long);
  */

  foo = bar;
  PRINT_ANY(foo, double);

  foo = const_bar;
  PRINT_ANY(foo, double);

  OUT << "typeof<int> = " << foo.is_type(typeid(int))
       << "; typeof<double> = " << foo.is_type(typeid(double)) << endl;

  // The following will throw an exception
  //foo.expose<int>() = 1;

  i = 5;
  bar &= i;
  Any bar1(bar, true);
  Any bar2(i, false);
  OUT << "reference tests: ";
  OUT << DEMANGLE_TYPE(bar, int) << "(" << bar.is_reference() << "), ";
  OUT << DEMANGLE_TYPE(bar1, Any) << "(" << bar1.is_reference() << "), ";
  OUT << DEMANGLE_TYPE(bar2, int) << "(" << bar2.is_reference() << ")" 
       << endl;

  OUT << "i = " << i << ", ";
  OUT << "Any(&i) = " << bar.expose<int>() << ", ";
  OUT << "Any(i) = " << bar2.expose<int>() << endl;

  ++i;
  OUT << "i = " << i << ", ";
  OUT << "Any(&i) = " << bar.expose<int>() << ", ";
  OUT << "Any(i) = " << bar2.expose<int>() << endl;

  ++const_cast<int&>(bar.expose<int>());
  ++const_cast<int&>(utilib::anyref_cast<int>(bar2));
  OUT << "i = " << i << ", ";
  OUT << "Any(&i) = " << bar.expose<int>() << ", ";
  OUT << "Any(i) = " << bar2.expose<int>() << endl;

  i = utilib::anyval_cast<int>(bar2);
  OUT << "i = " << i << ", ";
  OUT << "Any(&i) = " << bar.expose<int>() << ", ";
  OUT << "Any(i) = " << bar2.expose<int>() << endl;

  // This will call both constructor & copy constructor
  FOO myFOO;
  Any anyFoo1(myFOO);
  // This will only call the constructor
  Any anyFoo2;
  anyFoo2.set<FOO>();

  // Test immutable Anys
  int j = 1;
  foo.set(j,true,true);
  OUT << "j = " << j;
  foo = bar;
  OUT << "; j = " << j;
  foo.set(11);
  OUT << "; j = " << j << endl;
  try 
    { 
    bar = 10.5; 
    foo = bar;
    }
  CATCH("Assignment", utilib::bad_any_typeid);

  try 
    { foo = 10.5; }
  CATCH("Assignemnt", utilib::bad_any_typeid);
  
  try 
    { foo.set<long>(); }
  CATCH("Assignemnt", utilib::bad_any_typeid);

  Any::throwCastExceptions() = true;
  try
    { foo.extract(d); }
  CATCH("Extraction", utilib::bad_any_cast);


  // test implicit casting to reference anys
  {
  d = 1;
  AnyFixedRef tmp(d);
  OUT << "refcount = " << tmp.anyCount() << endl;
  AnyRef tmp4(tmp);
  OUT << "refcount = " << tmp.anyCount() << endl;
  AnyRef tmp3(tmp4);
  OUT << "refcount = " << tmp.anyCount() << endl;
  Any tmp5;
  tmp5 = tmp4;
  OUT << "refcount = " << tmp.anyCount() << endl;
  OUT << "Test AnyRef: (d = " << d << ") ";
  AnyRefTest(tmp3);
  OUT << " (d = " << d << ")" << endl;

  d = 5;
  OUT << "Test cast double -> AnyRef: (d = " << d << ") ";
  AnyRefTest(d);
  OUT << " (d = " << d << ")" << endl;

  d = 10;
  Any tmp2(d, true);
  OUT << "Test cast Any(is_ref = true) -> AnyRef: (d = " << d << ") ";
  AnyRefTest(tmp2);
  OUT << " (d = " << d << ", Any = " << tmp2.expose<double>() << ")" << endl;

  d = 20;
  tmp2 = d;
  OUT << "Test cast Any(is_ref = false) -> AnyRef: (d = " << d << ") ";
  AnyRefTest(tmp2);
  OUT << " (d = " << d << ", Any = " << tmp2.expose<double>() << ")" << endl;
  }

  // test implicit casting to immutable anys
  {
  d = 1;
  AnyRef tmp(d);
  OUT << "refcount = " << tmp.anyCount() << endl;
  AnyFixedRef tmp4(tmp);
  OUT << "refcount = " << tmp4.anyCount() << endl;
  AnyFixedRef tmp3(tmp4);
  OUT << "refcount = " << tmp4.anyCount() << endl;
  Any tmp5 = tmp4;
  OUT << "refcount = " << tmp4.anyCount() << endl;
  OUT << "Test AnyFixedRef: (d = " << d << ") ";
  AnyFixedRefTest(tmp3);
  OUT << " (d = " << d << ")" << endl;

  d = 5;
  OUT << "Test cast double -> AnyFixedRef: (d = " << d << ") ";
  AnyFixedRefTest(d);
  OUT << " (d = " << d << ")" << endl;

  d = 10;
  Any tmp2(d, true);
  OUT << "Test cast Any(is_ref = true) -> AnyFixedRef: (d = " << d << ") ";
  AnyFixedRefTest(tmp2);
  OUT << " (d = " << d << ", Any = " << tmp2.expose<double>() << ")" << endl;

  d = 20;
  tmp2 = d;
  OUT << "Test cast Any(is_ref = false) -> AnyFixedRef: (d = " << d << ") ";
  AnyFixedRefTest(tmp2);
  OUT << " (d = " << d << ", Any = " << tmp2.expose<double>() << ")" << endl;
  }

  // test equality tests
  {
     Any tmp_f = std::vector<FOO>(2);
     Any tmp_f1 = std::vector<FOO>(2);
     OUT << "Equality tests" << endl;

     Any tmp1 = i;
     Any tmp1a = tmp1;
     Any tmp2(i, true);
     Any tmp3(j, true);
     Any tmp4(d, true);
     OUT << "  test i   ref== i:  " << tmp1.references_same_data_as(tmp1a)
          << endl;
     OUT << "  test i   ref== &i: " << tmp1.references_same_data_as(tmp2)
          << endl;
     OUT << "  test i      == i:  " << (tmp1 == tmp1a) << endl;
     OUT << "  test i      == &i: " << (tmp1 == tmp2) << endl;
     OUT << "  test i      == j:  " << (tmp1 == tmp3) << endl;
     OUT << "  test i      == d:  " << (tmp1 == tmp4) << endl;
     j = i;
     OUT << "  test i      == j:  " << (tmp1 == tmp3) << endl;

     Any tmp_v1  = std::vector<int>(5,5);
     Any tmp_v1a = std::vector<int>(5,5);
     OUT << "  test v<i>   == v<i>:   " << ( tmp_v1 == tmp_v1a ) << endl;
     Any tmp_v1b = std::vector<int>(5,6);
     OUT << "  test v<i>   == v<i>':  " << ( tmp_v1 == tmp_v1b ) << endl;
     Any tmp_v1c = std::vector<double>(5,5);
     OUT << "  test v<i>   == v<d>:   " << ( tmp_v1 == tmp_v1c ) << endl;
     try
     {
        OUT << "  test v<FOO> == v<FOO>: ";
        OUT << ( tmp_f == tmp_f1 );
        OUT << endl;
     }
     catch (utilib::any_not_comparable& err)
     { OUT << "Comparing (correctly) failed" << endl; }
     catch (...)
     { OUT << "Comparing failed: (unexpected exception)" << endl; }

     std::map<int, double> m;
     m[1] = 4.4;
     m[3] = 3;
     Any tmp_m1 = m;
     Any tmp_m1a = m;
     OUT << "  test m<i,d> == m<i,d>: " << ( tmp_m1 == tmp_m1a ) << endl;
     m[4] = 2;
     Any tmp_m1b = m;
     OUT << "  test m<i,d> == m<i,d>: " << ( tmp_m1 == tmp_m1b ) << endl;

     std::list<char> l;
     l.push_back('a');
     l.push_back('c');
     l.push_back('b');
     Any tmp_l1 = l;
     Any tmp_l1a = l;
     OUT << "  test l<c>   == l<c>:   " << ( tmp_l1 == tmp_l1a ) << endl;

     Any tmp_d1 = std::deque<int>();
     Any tmp_d1a = tmp_d1;
     OUT << "  test d<i>   == d<i>:   " << ( tmp_d1 == tmp_d1a ) << endl;

     Any tmp_s1 = std::set<int>();
     Any tmp_s1a = tmp_s1;
     OUT << "  test s<i>   == s<i>:   " << ( tmp_s1 == tmp_s1a ) << endl;

     Any tmp_ms1 = std::multiset<int>();
     Any tmp_ms1a = tmp_ms1;
     OUT << "  test S<i>   == S<i>:   " << ( tmp_ms1 == tmp_ms1a ) << endl;

     Any tmp_mm1 = std::multimap<int,double>();
     Any tmp_mm1a = tmp_mm1;
     OUT << "  test M<i,d> == M<i,d>: " << ( tmp_mm1 == tmp_mm1a ) << endl;


     // test less than tests
     OUT << "LessThan tests" << endl;

     OUT << "  test i      <  i:  " << (tmp1 < tmp1a) << endl;
     OUT << "  test i      <  &i: " << (tmp1 < tmp2) << endl;
     j = i+1;
     OUT << "  test i      <  j:  " << (tmp1 < tmp3) << endl;
     OUT << "  test i      <  d:  " << (tmp1 < tmp4) << endl;
     j = i;
     OUT << "  test i      <  j:  " << (tmp1 < tmp3) << endl;

     OUT << "  test v<i>   <  v<i>:   " << ( tmp_v1 < tmp_v1a ) << endl;
     OUT << "  test v<i>   <  v<i>':  " << ( tmp_v1 < tmp_v1b ) << endl;
     // We test these as a pair because Any::operator<() for different
     // types is dependent on the (platform-dependent) definition of
     // std::type_info::before().
     OUT << "  (test v<i>  <  v<d>) + (test v<d>  <  v<i>):  " 
          << (( tmp_v1c < tmp_v1 ? 1 : 0 ) + ( tmp_v1 < tmp_v1c ? 1 : 0 )) 
          << endl;
     try
     {
        OUT << "  test v<FOO> <  v<FOO>: ";
        OUT << ( tmp_f < tmp_f1 );
        OUT << endl;
     }
     catch (utilib::any_not_comparable& err)
     { OUT << "Comparing (correctly) failed" << endl; }
     catch (...)
     { OUT << "Comparing failed: (unexpected exception)" << endl; }

     OUT << "  test m<i,d> <  m<i,d>: " << ( tmp_m1 < tmp_m1a ) << endl;
     OUT << "  test m<i,d> <  m<i,d>: " << ( tmp_m1 < tmp_m1b ) << endl;
     OUT << "  test l<c>   <  l<c>:   " << ( tmp_l1 < tmp_l1a ) << endl;
     OUT << "  test d<i>   <  d<i>:   " << ( tmp_d1 < tmp_d1a ) << endl;
     OUT << "  test s<i>   <  s<i>:   " << ( tmp_s1 < tmp_s1a ) << endl;
     OUT << "  test S<i>   <  S<i>:   " << ( tmp_ms1 < tmp_ms1a ) << endl;
     OUT << "  test M<i,d> <  M<i,d>: " << ( tmp_mm1 < tmp_mm1a ) << endl;
  }

  OUT << "Printing tests" << endl;
  {
     Any tmp = 1;
     OUT << "  (int) = " << tmp << endl;
     tmp = 5.5;
     OUT << "  (dbl) = " << tmp << endl;
     tmp = std::vector<int>(5,5);
     OUT << "  (vec) = " << tmp << endl;
     std::map<int,double> &m = tmp.set<std::map<int, double> >();
     m[2] = 2.2;
     m[4] = 4.4;
     m[8] = 8.8;
     OUT << "  (map) = " << tmp << endl;
     AnyRef tmp1 = i;
     OUT << "  (i&)  = " << tmp1 << endl;
  }

  return 0;
  }
