/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/included/unit_test.hpp> // header-only, Boost.Test framework

#include "DakotaBuildInfo.hpp"

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

  try {
    // It is desirable to split-up dakota into packages of re-useable components
    // but that is a significant re-factoring effort in its own right, so
    // link with the whole dakota library (for now)

    std::string rev_str = Dakota::DakotaBuildInfo::get_rev_number();
    // BMA: This check is from Subversion I'd guess and is somewhat
    // meaningless, but the short SHA1 should always satisfy this test
    // if converted as hexadecimal
    auto rev = std::stoll(rev_str, 0, 16);
    BOOST_CHECK(rev > 2452);
  }
  catch (const std::logic_error& e) {
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

