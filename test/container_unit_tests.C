/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "container_unit_tests.H"

CPPUNIT_TEST_SUITE_REGISTRATION( DakotaStringTest );

void DakotaStringTest::setUp() {
  test_string = "";
  test_string1 = "foo";
  test_string2 = "bar";
  test_string3 = "FOO";
  test_string4 = "BAR";
  char_string = "123456789";
}

void DakotaStringTest::testToUpper() {
  test_string = test_string1;
  test_string.toUpper();
  test_string += "bar";
  CPPUNIT_ASSERT( test_string == "FOObar" );
}

void DakotaStringTest::testToLower() {
  test_string = test_string3;
  test_string.toLower();
  test_string += "bar";
  CPPUNIT_ASSERT( test_string == "foobar" );
}

void DakotaStringTest::testAddString() {
  test_string = test_string1 + test_string2;
  test_string += test_string3 + test_string4;
  CPPUNIT_ASSERT( test_string == "foobarFOOBAR" );
}

void DakotaStringTest::testAssignChar() {
  test_string = char_string;
  CPPUNIT_ASSERT( test_string == char_string );
}

void DakotaStringTest::testAddChar() {
  test_string += char_string;
  CPPUNIT_ASSERT( test_string == char_string );
}

void DakotaStringTest::testContains() {
  test_string = char_string;
  test_string += char_string;
  CPPUNIT_ASSERT( test_string.contains(char_string) );
}

void DakotaStringTest::tearDown() {}

CPPUNIT_TEST_SUITE_REGISTRATION( DakotaArrayTest );

void DakotaArrayTest::setUp() {
  temp_vec.reshape(50); temp_vec = 99;
  temp_vec1.reshape(50); temp_vec1 = 99;
  temp_vec2.reshape(50); temp_vec2 = 99;
  for (int i = 0; i < temp_vec.length(); i++)
    temp_vec[i] = -22;
}

void DakotaArrayTest::testData() {
  point = const_cast<int *>(temp_vec.data());
  CPPUNIT_ASSERT( *point == temp_vec[0] );
}

void DakotaArrayTest::testAssign() {
  temp_vec1 = temp_vec;
  temp_vec1 = -100;
  CPPUNIT_ASSERT( temp_vec1[49] == -100 );
}

void DakotaArrayTest::testReshape() {
  temp_vec2 = temp_vec;
  temp_vec2.reshape(40);
  CPPUNIT_ASSERT( temp_vec2.length() == 40 );
}

void DakotaArrayTest::tearDown() {}

CPPUNIT_TEST_SUITE_REGISTRATION( DakotaListTest );

void DakotaListTest::setUp() {
  for (int i = 0; i < 10; i++)
    intList.insert(i * 100);
}

void DakotaListTest::testAssign() {
  intList2 = intList;
  for (int i = 0; i < 10; i++)
    intListBool.insert( intList[i]  ==  intList2[i] );
  CPPUNIT_ASSERT( !(intListBool.contains(false)) );
}

void DakotaListTest::tearDown() {}

CPPUNIT_TEST_SUITE_REGISTRATION( DakotaBaseVectorTest );

void DakotaBaseVectorTest::setUp() {
  v1.reshape(10);
  v2.reshape(10);
  for (int i = 0; i < 10; i++) {
    v1[i] = 3.0;
    v2[i] = 3.0;
  }
}

void DakotaBaseVectorTest::testEquality() {
  CPPUNIT_ASSERT( v1 == v2 );
}

void DakotaBaseVectorTest::testReshape() {
  v3.reshape(3);
  CPPUNIT_ASSERT( v3.length() == 3 );
}

void DakotaBaseVectorTest::tearDown() {}

CPPUNIT_TEST_SUITE_REGISTRATION( DakotaVectorTest );

void DakotaVectorTest::setUp() {
  temp_vec.resize(50); temp_vec = 99.;
  temp_vec1.resize(50); temp_vec1 = 99.;    
  for (int i = 0; i < temp_vec.length(); i++)
    temp_vec[i] = i*100.0;
}

void DakotaVectorTest::testData() {
  point = (double *) temp_vec.data();
  CPPUNIT_ASSERT( *point == temp_vec[0] );
}

void DakotaVectorTest::testReshape(){
  temp_vec1 = temp_vec;
  temp_vec1.reshape(40);
  temp_vec1 = -100;
  CPPUNIT_ASSERT( temp_vec1.length() == 40 );
}

void DakotaVectorTest::tearDown() {}

CPPUNIT_TEST_SUITE_REGISTRATION( DakotaMatrixTest );

void DakotaMatrixTest::setUp() {
  temp1.reshape_2d(10,10);
  temp2.reshape_2d(10,10);
  for (int i=0; i<10; i++) {
    for (int j=0; j<10; j++) {
      temp1[i][j] = 10.0;
      temp2[i][j] = 10.0;
    }
  }
}

void DakotaMatrixTest::testEquality() {
  CPPUNIT_ASSERT( temp1 == temp2 );
}

void DakotaMatrixTest::testReshape2D() {
  temp1.reshape_2d(20,20);
  CPPUNIT_ASSERT( temp1.num_rows() == 20 );
  CPPUNIT_ASSERT( temp1.num_columns() == 20 );
}

void DakotaMatrixTest::tearDown() {}

/** Main program executes each of the container unit tests. */
int main(int argc, char **argv) {
  // Create the event manager and test controller...
  CppUnit::TestResult controller;
 
  // Add a listener that collects test results...
  CppUnit::TestResultCollector result;
  controller.addListener( &result );        
 
  // Add a listener that prints detailed status as test runs...
  CppUnit::BriefTestProgressListener progress;
  controller.addListener( &progress );      
 
  // Add the top suite to the test runner...
  CppUnit::TestRunner runner;
  runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
  runner.run(controller, "");
 
  // Print test in a compiler compatible format...
  CppUnit::CompilerOutputter outputter( &result, std::cerr );
  outputter.write();                      

  // Return error code 1 if even one of the tests fail...
  return result.wasSuccessful() ? 0 : 1;
}
