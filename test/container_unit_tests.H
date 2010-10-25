/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "DakotaString.H"
#include "DakotaList.H"
#include "DakotaArray.H"
#include "DakotaVector.H"
#include "DakotaMatrix.H"

using namespace Dakota;

int write_precision = 10; ///< used in ostream data output functions

/**  Unit test suite for the String class.  Provides a quick way to
     test the basic functionality of the class.  Utilizes the assert
     function to test for correctness, will report if an unexpected
     answer is received. */
class DakotaStringTest : public CppUnit::TestFixture {
  ///< Register the test suite and its contents.
  CPPUNIT_TEST_SUITE( DakotaStringTest );
  CPPUNIT_TEST( testToUpper );
  CPPUNIT_TEST( testToLower );
  CPPUNIT_TEST( testAddString );
  CPPUNIT_TEST( testAssignChar );
  CPPUNIT_TEST( testAddChar );
  CPPUNIT_TEST( testContains );
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp();
  void testToUpper();
  void testToLower();
  void testAddString();
  void testAssignChar();
  void testAddChar();
  void testContains();
  void tearDown();
private:
  String test_string, test_string1, test_string2, test_string3, test_string4;
  char *char_string;
};

/**  Unit test fixture for the Array class.  Provides a quick way to
     test the basic functionallity of the class.  Utilizes the assert
     function to test for correctness, will report if an unexpected
     answer is received. */
class DakotaArrayTest : public CppUnit::TestFixture {
  ///< Register the test suite and its contents.
  CPPUNIT_TEST_SUITE( DakotaArrayTest );
  CPPUNIT_TEST( testData );
  CPPUNIT_TEST( testAssign );
  CPPUNIT_TEST( testReshape );
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp();
  void testData();
  void testAssign();
  void testReshape();
  void tearDown();
private:
  Array<int> temp_vec, temp_vec1, temp_vec2;
  int *point;
};

/** Unit test fixture for the List class.  Provides a quick way to
    test the basic functionality of the class.  Utilizes the assert
    function to test for correctness, will report if an unexpected
    answer is received. */
class DakotaListTest : public CppUnit::TestFixture {
  ///< Register the test suite and its contents.
  CPPUNIT_TEST_SUITE( DakotaListTest );
  CPPUNIT_TEST( testAssign );
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp();
  void testAssign();
  void tearDown();
private:
  List<int> intList, intList2;
  List<bool> intListBool;
};

/** Unit test fixture for the BaseVector class.  Provides a quick way
    to test the basic functionality of the class.  The assert function
    is used to test for correctness, will report if an unexpected
    answer is received. */
class DakotaBaseVectorTest : public CppUnit::TestFixture {
  ///< Register the test suite and its contents.
  CPPUNIT_TEST_SUITE( DakotaBaseVectorTest );
  CPPUNIT_TEST( testEquality );
  CPPUNIT_TEST( testReshape );
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp();
  void testEquality();
  void testReshape();
  void tearDown();
private:
  BaseVector<double> v1;
  BaseVector<double> v2;
  BaseVector<double> v3;
};

/**  Unit test fixture for the Vector class.  Provides a quick way to
     test the basic functionality of the class.  Utilizes the assert
     function to test for correctness, will report if an unexpected
     answer is received. */
class DakotaVectorTest : public CppUnit::TestFixture {
  ///< Register the test suite and its contents.
  CPPUNIT_TEST_SUITE( DakotaVectorTest );
  CPPUNIT_TEST( testData );
  CPPUNIT_TEST( testReshape );
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp();
  void testData();
  void testReshape();
  void tearDown();
private:
  Vector<double> temp_vec, temp_vec1;
  double *point;
};

/** Unit test fixture for the Matrix class.  Provides a quick way to
    test the basic functionality of the class.  The assert function is
    used to test for correctness, will report if an unexpected answer
    is received. */
class DakotaMatrixTest : public CppUnit::TestFixture {
  ///< Register the test suite and its contents.
  CPPUNIT_TEST_SUITE( DakotaMatrixTest );
  CPPUNIT_TEST( testEquality );
  CPPUNIT_TEST( testReshape2D );
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp();
  void testEquality();
  void testReshape2D();
  void tearDown();
private:
  Matrix<double> temp1;
  Matrix<double> temp2;
};
