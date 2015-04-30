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

#include <utilib/TypeManager.h>
#include <utilib/_math.h>

#include <iostream>
#include <set>
#include <list>
#include <vector>

using namespace utilib;

#define OUT std::cout

using std::endl;
using std::vector;
using std::list;
using std::set;

#define QUOTE(x) _QUOTE(x)

#define _QUOTE(x) #x

#define TEST_EXACT(NUM,SRC,DEST,EXACT)  \
do { \
  int ans = TypeManager()->lexical_cast(src, dest, typeid(DEST), EXACT);\
  OUT << "result[" << NUM << "]: " << ans << ";"; \
  printSTL<SRC >(QUOTE(SRC), src); \
  if ( ans >= 0 ) \
    { printSTL<DEST >(QUOTE(DEST), dest); } \
  else \
    { OUT << endl << "   [ " QUOTE(DEST) << " ]"; } \
  OUT << endl; \
} while ( false )

#define TEST(NUM,SRC,DEST)  TEST_EXACT(NUM,SRC,DEST,false)

#define TEST_CONTEXT_EXACT(NUM,SRC,DEST,S_TYPE,D_TYPE,EXACT)   \
do { \
  int ans = TypeManager()->context_cast( src, dest, SRC, DEST, \
                                         EXACT, typeid(D_TYPE));        \
  OUT << "result[" << NUM << "]: " << ans;  \
  printSTL<S_TYPE >(QUOTE(S_TYPE), src);  \
  printSTL<D_TYPE >(QUOTE(D_TYPE), dest);  \
  OUT << endl;  \
} while ( false )

#define TEST_CONTEXT(NUM,SRC,DEST,S_TYPE,D_TYPE) \
TEST_CONTEXT_EXACT(NUM,SRC,DEST,S_TYPE,D_TYPE,false)

template<typename T>
std::string print_val(T t)
{ return tostring(t); }


template<>
std::string print_val(double t)
{ return utilib::pscientific(t); }


template<typename T>
void printSTL(std::string name, Any ans)
  {
  if ( ans.is_type(typeid(T)) )
    {
    const T& data = ans.template expose<T>();
    OUT << endl << "   " << name << " == [";
    typename T::const_iterator it    = data.begin();
    typename T::const_iterator itEnd = data.end();
    while ( it != itEnd )
      {
      OUT << print_val(*it);
      ++it;
      OUT << ( it == itEnd ? "];  " : ", " );
      }
    }
  else
    { OUT << endl << "   { Any != " << name << " }"; }
  }


int double2vectorDouble(const Any &src, Any &dest)
  {
  const double& tmp = src.expose<double>();
  dest.set(std::vector<double>(1, tmp));
  return 0;
  }

int contextA2B_1(const Any &src, Any &dest)
  {
  const vector<int>& tmp = src.expose<vector<int> >();
  list<int> ans;
  size_t i = tmp.size();
  while ( i > 0 )
    { ans.push_back(2*tmp[--i]); }
  dest.set(ans);
  return 0;
  }

int contextA2B_2(const Any &src, Any &dest)
  {
  double tmp = 2.0 * src.expose<double>();
  dest.set(tmp);
  return 0;
  }

int contextB2C_1(const Any &src, Any &dest)
  {
  const vector<double>& tmp = src.expose<vector<double> >();
  dest.set(tmp[0]);
  return 0;
  }

int contextB2C_2(const Any &src, Any &dest)
  {
  const vector<double>& tmp = src.expose<vector<double> >();
  dest.set(vector<double>(1, tmp[0]));
  return 0;
  }

int test_typeManager(int argc, char** )
  {
  // Disable exceptions so this will actually run
  TypeManager()->setErrorExceptions(false);
  TypeManager()->setWarningExceptions(false);

  // An update to the typemanager added additional inexact casts that
  // foul up the logic of this test routine.  We will disable them so
  // things still work...
  TypeManager()->clear_lexical_casts(typeid(vector<int>), typeid(int));
  TypeManager()->clear_lexical_casts(typeid(vector<long>), typeid(long));
  TypeManager()->clear_lexical_casts(typeid(vector<double>), typeid(double));

  // only display the casting table if the user asks for it... doing it
  // as part of a unit test is problematic as registering additional
  // tests will (of course) change the table
  if ( argc > 1 )
    {
    OUT << "lexical_cast table:" << endl;
    TypeManager()->printLexicalCastingTable(OUT);
    OUT << endl;
    }

  // OK, I am about to do a whole set of tests... to cut down on
  // cut-and-paste errors, I am going to use a fancy set of macros and
  // templates.  BUT, for clarity's sake, the macro expands to something
  // similar to:
  /*
  // Setup...
  std::set<int> aSet;
  aSet.insert(1);
  aSet.insert(3);
  Any src(aSet);
  Any dest;

  // Convert...
  int ans = TypeManager()->lexical_cast(src, dest, typeid(std::vector<int>));

  // Print...
  std::cout << "result: " << ans << ";"; 
  printSTL<std::set<int> >("std::set<int>", src); 
  if ( ans >= 0 ) 
    { printSTL<std::vector<int> >("std::vector<int>", dest); } 
  else 
    { std::cout << endl << "   [ std::vector<int> ]"; } 
  std::cout << endl; 
  */

  vector<double> aDVec;
  
  std::set<int> aSet;
  aSet.insert(1);
  aSet.insert(2);
  aSet.insert(5);

  TypeManager()->register_lexical_cast( typeid(double), typeid(vector<double>),
                                        &double2vectorDouble, 1 );
  if ( argc > 1 )
    {
    OUT << "modified lexical_cast table:" << endl;
    TypeManager()->printLexicalCastingTable(OUT);
    OUT << endl;
    }

  OUT << "LEXICAL TESTS" << endl;

  Any src;
  Any dest;

  // Test fail due to empty source
  TEST(1, std::set<int>, std::list<int>);

  src = Any(aSet);
  TEST(2, std::set<int>, std::list<int>);

  aSet.erase(2);
  src = aSet;
  TEST_EXACT(3, std::set<int>, std::vector<int>, true);

  aSet.insert(10);
  src = aSet;
  TEST(4, std::set<int>, vector<double>);

  src = dest;
  TEST(5, vector<double>, vector<int>);

  src.extract(aDVec);
  aDVec[1] = 6.75;
  src.set(aDVec);
  // Test warning for truncation
  TEST(6, vector<double>, vector<int>);
 
  aDVec[2] = 5.5E9;
  src.set(aDVec);
  // Test warning for out of bounds
  TEST(7, vector<double>, vector<int>);

  // Test error for required exact cast
  TEST_EXACT(8, vector<double>, vector<int>, true);

  // Test warning for non-existant cast
  TEST(9, std::vector<double>, std::set<int>);

  // Test the convenience function (also tests Any immutable)
  src=aSet;
  std::vector<int> aIVec;
  int ans = TypeManager()->lexical_cast(src, aIVec);
  OUT << "result[10]: " << ans << ";";
  printSTL<set<int> >("set<int>", src);
  if ( ans >= 0 ) 
    { 
    OUT << endl << "   vector<int> == [";
    vector<int>::const_iterator it    = aIVec.begin();
    vector<int>::const_iterator itEnd = aIVec.end();
    while ( it != itEnd )
      {
      OUT << print_val(*it);
      ++it;
      OUT << ( it == itEnd ? "];  " : ", " );
      }
    }
  else 
    { OUT << endl << "   [ vector<int> ]"; } 
  OUT << endl; 

#if 0
  OUT << endl << "CONTEXT TESTS" << endl;
  OUT << "context registrations:  ";
  OUT << TypeManager()->register_context("Context A") << ", ";
  OUT << TypeManager()->register_context("Context B") << ", ";
  OUT << TypeManager()->register_context("Context C") << ", ";
  OUT << TypeManager()->register_context("Context B") << endl;

  OUT << "function registrations: ";
  OUT << TypeManager()->register_context_cast("Context A", 
                                              typeid(vector<int>),
                                              "Context B", 
                                              typeid(list<int>),
                                              &contextA2B_1) << ", ";
  OUT << TypeManager()->register_context_cast("Context A", 
                                              typeid(double),
                                              "Context B", 
                                              typeid(double),
                                              &contextA2B_2, false) << ", ";
  OUT << TypeManager()->register_context_cast("Context B", 
                                              typeid(vector<double>),
                                              "Context C", 
                                              typeid(double),
                                              &contextB2C_1) << ", ";
  OUT << TypeManager()->register_context_cast("Context B", 
                                              typeid(vector<double>),
                                              "Context C", 
                                              typeid(vector<double>),
                                              &contextB2C_2);
  OUT << endl;

  if ( argc > 1 )
    {
    OUT << "context_cast table:" << endl;
    TypeManager()->printContextCastingTable(OUT);
    OUT << endl;
    }


  src = aSet;
  TEST_CONTEXT(1, "Context X", "Context C", set<int>, list<double>);
  TEST_CONTEXT(2, "Context C", "Context B", set<int>, list<double>);
  TEST_CONTEXT(3, "Context A", "Context C", set<int>, list<double>);

  aDVec[2] = 7.75;
  src = aDVec;
  TEST_CONTEXT(4, "Context A", "Context C", vector<double>, vector<double>);

  src = aDVec;
  TEST_CONTEXT_EXACT(5, "Context A", "Context C", vector<double>, 
                     vector<double>, true);

  src = (double)7.75;
  OUT << "result[6]: " << TypeManager()->context_cast
     (src, dest, "Context A", "Context C", false, typeid(vector<double>));
  OUT << endl << "   double == " << src.expose<double>();
  printSTL<vector<double> >("vector<double>", dest);
  OUT << endl;

  OUT << "result[7]: " << TypeManager()->context_cast
     (src, dest, "Context A", "Context C", true, typeid(vector<double>));
  OUT << endl << "   double == " << src.expose<double>();
  printSTL<vector<double> >("vector<double>", dest);
  OUT << endl;
#endif

  // Enable exceptions ...
  OUT << endl << "EXCEPTION TESTS" << endl;
  TypeManager()->setErrorExceptions(true);
  TypeManager()->setWarningExceptions(true);

  aDVec[2] = 5.5E9;
  src.set(aDVec);
  try 
  {
     // Test warning for out of bounds
     TEST(1, vector<double>, vector<int>);
  }
  catch ( bad_lexical_cast & )
  {
     OUT << "Caught expected bad_lexical_cast exception:" << endl;
  }
  catch ( std::exception &e )
  {
     OUT << "Caught unexpected exception:" << endl << "  " << e.what() << endl;
  }

  try 
  {
     // Test error for required exact cast
     TEST_EXACT(2, vector<double>, vector<int>, true);
  }
  catch ( bad_lexical_cast & )
  {
     OUT << "Caught expected bad_lexical_cast exception:" << endl;
  }
  catch ( std::exception &e )
  {
     OUT << "Caught unexpected exception:" << endl << "  " << e.what() << endl;
  }

#if 0
  try 
  {
     src = aSet;
     TEST_CONTEXT(3, "Context X", "Context C", set<int>, list<double>);
  }
  catch ( bad_context_cast & )
  {
     OUT << "Caught expected bad_context_cast exception:" << endl;
  }
  catch ( std::exception &e )
  {
     OUT << "Caught unexpected exception:" << endl << "  " << e.what() << endl;
  }

  try 
  {
     TypeManager()->register_lexical_cast( typeid(double), 
                                           typeid(vector<double>),
                                           &double2vectorDouble, 1 );
  }
  catch ( typeManager_error & )
  {
     OUT << "Caught expected typeManager_error exception:" << endl;
  }
  catch ( std::exception &e )
  {
     OUT << "Caught unexpected exception:" << endl << "  " << e.what() << endl;
  }
#endif

  return 0;
  }
