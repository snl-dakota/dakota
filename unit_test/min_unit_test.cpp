/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <boost/test/minimal.hpp>     // header-only minimal testing
//#include <boost/test/unit_test.hpp> // requires linkage -lboost_unit_test_framework

namespace DakotaUnitTest {
namespace MinTest {

void my_test0()
{
  BOOST_CHECK(0 == 0);
  BOOST_CHECK(1+1 == 2);
}

} // end of MinTest namespace
} // end of DakotaUnitTest namespace


// NOTE: Boost.Test framework provides the main program driver
//____________________________________________________________________________//

int test_main( int argc, char* argv[] )      // note the name!
{
  DakotaUnitTest::MinTest::my_test0();
  return boost::exit_success;
}

