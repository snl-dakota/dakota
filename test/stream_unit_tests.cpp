/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_data_io.hpp"
#include "dakota_global_defs.hpp"

// Boost.Test
#include <boost/test/minimal.hpp>

#include <sstream>


namespace Dakota {
namespace TestBinStream {

struct DataBundle
{
  //static std::string astr;
  static char ch;
  static double dbl;
  static float flt;
  static int nt;
  static long lng;
  static short shrt;
  static unsigned char uch;
  static unsigned int uin;
  static unsigned long uln;
  static unsigned short ush;
};


// DataBundle initialization (pre-main); in old cppUnit done in setUp()
  //std::strcpy(cstr,"this is a char string!!\n");
//std::string DataBundle::astr = std::string("this is a standard string!!");
char DataBundle::ch = 'c';
double DataBundle::dbl = 1234567.89;
float DataBundle::flt = 1.23;
int DataBundle::nt = 16;
long DataBundle::lng = 890123;
short DataBundle::shrt = 1;
unsigned char DataBundle::uch = 'u';
unsigned int DataBundle::uin = 32;
unsigned long DataBundle::uln = 654321;
unsigned short DataBundle::ush = 4321;


void test_write_read()
{
  // WJB:  question the value of this test; already performed by Boost!
#if 0
  DataBundle dat_bundle;
  std::stringstream io_sstream;

  // Output archive stream to serialize data
  boost::archive::binary_oarchive oa(io_sstream);

  oa << dat_bundle.ch << dat_bundle.dbl << dat_bundle.flt << dat_bundle.nt
     << dat_bundle.lng << dat_bundle.shrt << dat_bundle.uch
     << dat_bundle.uin << dat_bundle.uln << dat_bundle.ush;

  // Input archive stream to unserialize data
  boost::archive::binary_iarchive ia(io_sstream);

  char ch2; double dbl2; float flt2; int nt2; long lng2; short shrt2;
  unsigned char uch2; unsigned int uin2; unsigned long uln2; unsigned short ush2;

  // Unserialize the data
  ia >> ch2 >> dbl2 >> flt2 >> nt2 >> lng2 >> shrt2 >> uch2
     >> uin2 >> uln2 >> ush2;

  // check for data matches using assert
  BOOST_CHECK( dat_bundle.ch == ch2 );
  BOOST_CHECK( dat_bundle.dbl == dbl2 );
  BOOST_CHECK( dat_bundle.flt == flt2 );
  BOOST_CHECK( dat_bundle.nt == nt2 );
  BOOST_CHECK( dat_bundle.lng == lng2 );
  BOOST_CHECK( dat_bundle.shrt == shrt2 );
  BOOST_CHECK( dat_bundle.uch == uch2 );
  BOOST_CHECK( dat_bundle.uin == uin2 );
  BOOST_CHECK( dat_bundle.uln == uln2 );
  BOOST_CHECK( dat_bundle.ush == ush2 );
#endif
}


#ifdef DAKOTA_HAVE_MPI
void test_mpi_send_receive()
{
  DataBundle dat_bundle;

  // Create a send buffer and pack the data into it
  Dakota::MPIPackBuffer send_buffer;
  send_buffer << dat_bundle.ch << dat_bundle.dbl << dat_bundle.flt
              << dat_bundle.nt << dat_bundle.lng << dat_bundle.shrt
              << dat_bundle.uch << dat_bundle.uin << dat_bundle.uln
              << dat_bundle.ush;

  // Create a buffer to unpack that is a copy of send_buffer
  Dakota::MPIUnpackBuffer recv_buffer(const_cast<char*>(send_buffer.buf()),
                                      send_buffer.size(), false);

  char ch2; double dbl2; float flt2; int nt2; long lng2; short shrt2;
  unsigned char uch2; unsigned int uin2; unsigned long uln2; unsigned short ush2;

  // Unpack the data
  recv_buffer >> ch2 >> dbl2 >> flt2 >> nt2 >> lng2 >> shrt2 >> uch2
              >> uin2 >> uln2 >> ush2;

  // check for data matches using assert
  BOOST_CHECK( dat_bundle.ch == ch2 );
  BOOST_CHECK( dat_bundle.dbl == dbl2 );
  BOOST_CHECK( dat_bundle.flt == flt2 );
  BOOST_CHECK( dat_bundle.nt == nt2 );
  BOOST_CHECK( dat_bundle.lng == lng2 );
  BOOST_CHECK( dat_bundle.shrt == shrt2 );
  BOOST_CHECK( dat_bundle.uch == uch2 );
  BOOST_CHECK( dat_bundle.uin == uin2 );
  BOOST_CHECK( dat_bundle.uln == uln2 );
  BOOST_CHECK( dat_bundle.ush == ush2 );
}
#endif

} // end namespace TestBinStream
} // end namespace Dakota


#ifdef DAKOTA_HAVE_MPI
#include "mpi.h"
#endif

// NOTE: Boost.Test framework provides the main program driver

//____________________________________________________________________________//

int test_main( int argc, char* argv[] )      // note the name!
{
  using namespace Dakota::TestBinStream;

#ifdef DAKOTA_HAVE_MPI
  MPI_Init(&argc, &argv);
  test_mpi_send_receive();
  MPI_Finalize();
#endif

  test_write_read();
  return boost::exit_success;
}

