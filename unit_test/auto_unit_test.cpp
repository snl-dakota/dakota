/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#define BOOST_TEST_MODULE dakota_collection_unit
#include <boost/test/included/unit_test.hpp>

#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>
#include <boost/range/combine.hpp>


// Consider eliminating these tests since not much added value
// perhaps useful as a template, and historically, the old CppUnit
// tests had some similar features, so maintain for a short time
// until authoring tests becomes part of the natural, Dakota
// development routine.

//____________________________________________________________________________//

BOOST_AUTO_TEST_CASE( test_int_vec )
{
  using namespace boost::assign;
  std::vector<int> v1, v2;

  v1 += 1, 2, 3, 4, 5, 6, 7, 8, 9;
  v2 += 1, 2, 3, 4, 5, 6, 7, 8, 9;

  BOOST_CHECK_EQUAL_COLLECTIONS( v1.begin(), v1.end(),
                                 v2.begin(), v2.end() );
}

//____________________________________________________________________________//

namespace DakotaAutoTest {

// ToDo:  templatize on floating-point type
// Surprised NOT provided by Boost??

inline void check_close_vecs(const std::vector<double>& v1,
                             const std::vector<double>& v2, double tol)
{
  std::vector<double>::value_type e1, e2;

  BOOST_REQUIRE_EQUAL( v1.size(), v2.size() );
  BOOST_FOREACH( boost::tie(e1, e2), boost::combine(v1, v2) )
    BOOST_CHECK_CLOSE(e1, e2, tol);
}

}

//____________________________________________________________________________//

BOOST_AUTO_TEST_CASE( test_double_vec )
{
  using namespace boost::assign;
  std::vector<double> v1, v2;

  v1 += 1, 2, 3, 4, 5, 6, 7, 8, 9;
  v2 += 1, 2, 3, 4, 5, 6, 7, 8, 9.00001;

  DakotaAutoTest::check_close_vecs(v1, v2, 0.001);
}

