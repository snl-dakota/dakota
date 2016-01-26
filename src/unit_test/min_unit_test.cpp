/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/included/unit_test.hpp> // header-only, Boost.Test framework

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
    BOOST_CHECK(rev > 2452);
  }
  catch (const boost::bad_lexical_cast& e) {
    std::cerr << '\n' << e.what() << std::endl;
  }
}

} // end of MinTest namespace
} // end of DakotaUnitTest namespace


// NOTE: Boost.Test framework provides the main program driver
//____________________________________________________________________________//

bool init_unit_test()
{
  boost::unit_test::framework::master_test_suite().add(
    BOOST_TEST_CASE( &DakotaUnitTest::MinTest::my_test0 ) );
  boost::unit_test::framework::master_test_suite().add(
    BOOST_TEST_CASE( &DakotaUnitTest::MinTest::rev_test ) );

  return true;
}

