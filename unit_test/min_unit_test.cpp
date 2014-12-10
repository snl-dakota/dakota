/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <boost/test/minimal.hpp>     // header-only minimal testing
//#include <boost/test/unit_test.hpp> // requires linkage -lboost_unit_test_framework

#include "DakotaBuildInfo.hpp"

#include <boost/lexical_cast.hpp>
#include <exception>


namespace DakotaUnitTest {
namespace MinTest {

void my_test0()
{
  BOOST_CHECK(0 == 0);
  BOOST_CHECK(1+1 == 2);
}


void rev_test()
{
  using boost::lexical_cast;

  try {
    // It is desirable to split-up dakota into packages of re-useable components
    // but that is a significant re-factoring effort in its own right, so
    // link with the whole dakota library (for now)

    std::string rev_str = Dakota::DakotaBuildInfo::get_rev_number();
    short rev = lexical_cast<short>(rev_str);
    BOOST_CHECK(rev > 2852);
  }
  catch (const boost::bad_lexical_cast &) {
    boost::exit_failure;
  }
}

} // end of MinTest namespace
} // end of DakotaUnitTest namespace


// NOTE: Boost.Test framework provides the main program driver
//____________________________________________________________________________//

int test_main( int argc, char* argv[] )      // note the name!
{
  DakotaUnitTest::MinTest::my_test0();
  DakotaUnitTest::MinTest::rev_test();

  return boost::exit_success;
}

