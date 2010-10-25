/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "stream_unit_tests.H"

CPPUNIT_TEST_SUITE_REGISTRATION( DakotaBinStreamTest );

void DakotaBinStreamTest::setUp() {
  ch = 'c';
  strcpy(cstr,"this is a char string!!\n");
  dbl = 1234567.89;
  flt = 1.23;
  nt = 16;
  lng = 890123;
  shrt = 1;
  uch = 'u';
  uin = 32;
  uln = 654321;
  ush = 4321;
}

void DakotaBinStreamTest::testEquality() {
  // Open a file for binary output
  BoStream bostream("data_test.bin", ios::out);
  CPPUNIT_ASSERT( bostream.good() );

  // Write binary data to file and close it
  bostream << ch << cstr << dbl << flt << nt << lng << shrt << uch
	   << uin << uln << ush;
  bostream.close();

  // Open the binary file for input
  BiStream bistream;
  bistream.open("data_test.bin", ios::in);

  // Read binary data from file and close it
  bistream >> ch2 >> cstr2 >> dbl2 >> flt2 >> nt2 >> lng2 >> shrt2 >> uch2
	   >> uin2 >> uln2 >> ush2;

  // Check that it opened correctly, then close it.
  CPPUNIT_ASSERT( bistream.good() );
  bistream.close();

  CPPUNIT_ASSERT( ch == ch2 );
  CPPUNIT_ASSERT( dbl == dbl2 );
  CPPUNIT_ASSERT( flt == flt2 );
  CPPUNIT_ASSERT( nt == nt2 );
  CPPUNIT_ASSERT( lng == lng2 );
  CPPUNIT_ASSERT( shrt == shrt2 );
  CPPUNIT_ASSERT( uch == uch2 );
  CPPUNIT_ASSERT( uin == uin2 );
  CPPUNIT_ASSERT( uln == uln2 );
  CPPUNIT_ASSERT( ush == ush2 );
}

void DakotaBinStreamTest::tearDown() {}

#ifdef USE_MPI
CPPUNIT_TEST_SUITE_REGISTRATION( DakotaPackBufferTest );

void DakotaPackBufferTest::setUp() {
  ch   = 'c';
  dbl  = 1234567.89;
  flt  = 1.23;
  nt   = 16;
  lng  = 890123;
  shrt = 1;
  uch = 'u';
  uin = 32;
  uln = 654321;
  ush = 4321;
  send_buffer; // buffer to be shared between methods
}

void DakotaPackBufferTest::testEquality() {
  // Create a send buffer and pack the data into it
  MPIPackBuffer send_buffer;
  send_buffer << ch << dbl << flt << nt << lng << shrt << uch
	      << uin << uln << ush;


  // Create a buffer to unpack that is a copy of send_buffer
  MPIUnpackBuffer recv_buffer(const_cast<char*>(send_buffer.buf()),
			      send_buffer.size(), false);
    
  // Unpack the data
  recv_buffer >> ch2 >> dbl2 >> flt2 >> nt2 >> lng2 >> shrt2 >> uch2
	      >> uin2 >> uln2 >> ush2; // >> cstr2 >> str2;

  // check for data matches using assert
  CPPUNIT_ASSERT( ch == ch2 );
  CPPUNIT_ASSERT( dbl == dbl2 );
  CPPUNIT_ASSERT( flt == flt2 );
  CPPUNIT_ASSERT( nt == nt2 );
  CPPUNIT_ASSERT( lng == lng2 );
  CPPUNIT_ASSERT( shrt == shrt2 );
  CPPUNIT_ASSERT( uch == uch2 );
  CPPUNIT_ASSERT( uin == uin2 );
  CPPUNIT_ASSERT( uln == uln2 );
  CPPUNIT_ASSERT( ush == ush2 );
}

void DakotaPackBufferTest::tearDown() {}
#endif

/** Main program executes each of the container unit tests. */
int main(int argc, char **argv) {
#ifdef HAVE_MPI
  // required for use of MPI_Pack()/MPI_Unpack()
  MPI_Init(&argc, &argv);
#endif

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

#ifdef HAVE_MPI
  // use of MPI_Pack()/MPI_Unpack() is complete
  MPI_Finalize();
#endif

  // Return error code 1 if even one of the tests fail...
  return result.wasSuccessful() ? 0 : 1;
}
