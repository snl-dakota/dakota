/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <gtest/gtest.h>

#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>
#include <boost/range/combine.hpp>

// Consider eliminating these tests since not much added value
// perhaps useful as a template, and historically, the old CppUnit
// tests had some similar features, so maintain for a short time
// until authoring tests becomes part of the natural, Dakota
// development routine.

//____________________________________________________________________________//

TEST(auto_unit_test_tests, test_int_vec) {
  using namespace boost::assign;
  std::vector<int> v1, v2;

  v1 += 1, 2, 3, 4, 5, 6, 7, 8, 9;
  v2 += 1, 2, 3, 4, 5, 6, 7, 8, 9;

  EXPECT_EQ(v1, v2);
}

//____________________________________________________________________________//

namespace DakotaAutoTest {

// ToDo:  templatize on floating-point type

inline void check_close_vecs(const std::vector<double>& v1,
                             const std::vector<double>& v2, double tol) {
  std::vector<double>::value_type e1, e2;

  EXPECT_EQ(v1.size(), v2.size());
  BOOST_FOREACH (boost::tie(e1, e2), boost::combine(v1, v2))
    EXPECT_NEAR(e1, e2, tol);
}

}  // namespace DakotaAutoTest

//____________________________________________________________________________//

TEST(auto_unit_test_tests, test_double_vec) {
  using namespace boost::assign;
  std::vector<double> v1, v2;

  v1 += 1, 2, 3, 4, 5, 6, 7, 8, 9;
  v2 += 1, 2, 3, 4, 5, 6, 7, 8, 9.00001;

  DakotaAutoTest::check_close_vecs(v1, v2, 0.001);
}

//____________________________________________________________________________//

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
