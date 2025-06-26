/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <gtest/gtest.h>

#include <exception>

#include "DakotaBuildInfo.hpp"

namespace DakotaUnitTest {
namespace MinTest {

void my_test0() {
  EXPECT_TRUE((0 == 0));
  EXPECT_TRUE((1 + 1 == 2));
}

void rev_test() {
  try {
    // It is desirable to split-up dakota into packages of re-useable components
    // but that is a significant re-factoring effort in its own right, so
    // link with the whole dakota library (for now)

    std::string rev_str = Dakota::DakotaBuildInfo::get_rev_number();
    // BMA: This check is from Subversion I'd guess and is somewhat
    // meaningless, but the short SHA1 should always satisfy this test
    // if converted as hexadecimal
    auto rev = std::stoll(rev_str, 0, 16);
    EXPECT_TRUE((rev > 2452));
  } catch (const std::logic_error& e) {
    std::cerr << '\n' << e.what() << std::endl;
  }
}

}  // namespace MinTest
}  // namespace DakotaUnitTest

//____________________________________________________________________________//

TEST(min_unit_test_tests, all_tests) {
  DakotaUnitTest::MinTest::my_test0();
  DakotaUnitTest::MinTest::rev_test();
}

//____________________________________________________________________________//

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
