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

#include "DakotaBinStream.H"
#include "MPIPackBuffer.H"

#ifdef USE_MPI
#include "mpi.h"
#endif

using namespace Dakota;

int write_precision = 10; ///< used in ostream data output functions

/**  Unit test method for the Dakota::BiStream/BoStream classes.
     Provides a quick way to test the basic functionality of the
     classes.  Utilizes the assert function to test for correctness,
     will fail if an expected answer is not received. */
class DakotaBinStreamTest : public CppUnit::TestFixture {
  ///< Register the test suite and its contents.
  CPPUNIT_TEST_SUITE( DakotaBinStreamTest );
  CPPUNIT_TEST( testEquality );
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp();
  void testBoStreamConstructor();
  void testBiStreamConstructor();
  void testEquality();
  void tearDown();
private:
  // Basic types.  To do: add strings/vectors/matrices
  char   ch2,   ch;
  char   cstr2[125], cstr[25];
  double dbl2,  dbl;
  float	 flt2,  flt;
  int    nt2,   nt;
  long   lng2,  lng;
  short  shrt2, shrt;
  unsigned char  uch2, uch;
  unsigned int   uin2, uin;
  unsigned long  uln2, uln;
  unsigned short ush2, ush;
};

#ifdef USE_MPI
/**  Unit test method for the Dakota::MPIPackBuffer/MPIUnpackBuffer
     classes.  Provides a quick way to test the basic functionality of
     the classes.  Utilizes the assert function to test for
     correctness, will fail if an expected answer is not received. */
class DakotaPackBufferTest : public CppUnit::TestFixture {
  ///< Register the test suite and its contents.
  CPPUNIT_TEST_SUITE( DakotaPackBufferTest );
  CPPUNIT_TEST( testEquality );
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp();
  void testMPIPackBufferConstructor();
  void testMPIUnPackBufferConstructor();
  void testEquality();
  void tearDown();
private:
  // Basic types.  To do: add strings/vectors/matrices
  char   ch2,   ch;
  double dbl2,  dbl;
  float	 flt2,  flt;
  int    nt2,   nt;
  long   lng2,  lng;
  short  shrt2, shrt;
  unsigned char  uch2, uch;
  unsigned int   uin2, uin;
  unsigned long  uln2, uln;
  unsigned short ush2, ush;
  MPIPackBuffer send_buffer;
};
#endif
