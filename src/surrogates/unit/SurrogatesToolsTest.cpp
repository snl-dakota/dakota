/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "surrogates_tools.hpp"
#include "util_common.hpp"

#define BOOST_TEST_MODULE surrogates_SurrogatesToolsTest
#include <boost/test/included/unit_test.hpp>

///////////////
// Namespace //
///////////////

using namespace dakota;
using namespace dakota::util;
using namespace dakota::surrogates;

// ------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_surrogates_size_level_index_vector) {
  const int ndims = 3;
  const int level = 2;

  MatrixXi gold_indices(
      6, 3);  // these come from exercising Pecos::util::compute_combinations
  gold_indices << 2, 0, 0, 1, 1, 0, 0, 2, 0, 1, 0, 1, 0, 1, 1, 0, 0, 2;

  Eigen::MatrixXi level_indices;
  size_level_index_vector(ndims, level, level_indices);

  BOOST_CHECK(matrix_equals(level_indices, gold_indices));
}

// ------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_surrogates_hyperbolic_subdim_level_indices) {
  // These values were used in manual comparisons with
  // Pecos::util::compute_hyperbolic_level_subdim_indices
  double p = 1.0;
  int ndims = 3, level = 2, active_dims = 2;
  Eigen::MatrixXi gold_indices(1, 2);
  gold_indices << 1, 1;

  Eigen::MatrixXi level_indices;
  compute_hyperbolic_subdim_level_indices(ndims, level, active_dims, p,
                                          level_indices);
  BOOST_CHECK(matrix_equals(level_indices, gold_indices));

  ndims = 3, level = 3, active_dims = 2;
  gold_indices.resize(2, 2);
  gold_indices << 2, 1, 1, 2;
  compute_hyperbolic_subdim_level_indices(ndims, level, active_dims, p,
                                          level_indices);
  BOOST_CHECK(matrix_equals(level_indices, gold_indices));

  ndims = 3, level = 3, active_dims = 3;
  gold_indices.resize(1, 3);
  gold_indices << 1, 1, 1;
  compute_hyperbolic_subdim_level_indices(ndims, level, active_dims, p,
                                          level_indices);
  BOOST_CHECK(matrix_equals(level_indices, gold_indices));
}

// ------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_surrogates_hyperbolic_level_indices) {
  // These values were used in manual comparisons with
  // Pecos::util::compute_hyperbolic_level_indices
  double p = 1.0;
  Eigen::MatrixXi level_indices;
  Eigen::MatrixXi gold_indices;

  int ndims = 3, level = 0;
  gold_indices.resize(3, 1);
  gold_indices << 0, 0, 0;
  compute_hyperbolic_level_indices(ndims, level, p, level_indices);
  BOOST_CHECK(matrix_equals(level_indices, gold_indices));

  ndims = 3, level = 1;
  gold_indices = Eigen::MatrixXi::Identity(3, 3);
  compute_hyperbolic_level_indices(ndims, level, p, level_indices);
  BOOST_CHECK(matrix_equals(level_indices, gold_indices));

  ndims = 3, level = 2;
  gold_indices = Eigen::MatrixXi::Identity(3, 6);
  gold_indices << 2, 0, 0, 1, 1, 0, 0, 2, 0, 1, 0, 1, 0, 0, 2, 0, 1, 1;
  compute_hyperbolic_level_indices(ndims, level, p, level_indices);
  BOOST_CHECK(matrix_equals(level_indices, gold_indices));

  ndims = 3, level = 3;
  gold_indices = Eigen::MatrixXi::Identity(3, 10);
  gold_indices << 3, 0, 0, 2, 1, 2, 1, 0, 0, 1, 0, 3, 0, 1, 2, 0, 0, 2, 1, 1, 0,
      0, 3, 0, 0, 1, 2, 1, 2, 1;
  compute_hyperbolic_level_indices(ndims, level, p, level_indices);
  BOOST_CHECK(matrix_equals(level_indices, gold_indices));
}

// ------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_surrogates_hyperbolic_indices) {
  // These values were used in manual comparisons with
  // Pecos::util::compute_hyperbolic_indices
  double p = 1.0;
  int ndims = 1, level = 1;

  Eigen::MatrixXi level_indices;
  Eigen::MatrixXi gold_indices;

  gold_indices.resize(1, 2);
  gold_indices << 0, 1;
  compute_hyperbolic_indices(ndims, level, p, level_indices);
  BOOST_CHECK(matrix_equals(level_indices, gold_indices));

  ndims = 3, level = 3;
  gold_indices = Eigen::MatrixXi::Identity(3, 20);
  gold_indices << 0, 1, 0, 0, 2, 0, 0, 1, 1, 0, 3, 0, 0, 2, 1, 2, 1, 0, 0, 1, 0,
      0, 1, 0, 0, 2, 0, 1, 0, 1, 0, 3, 0, 1, 2, 0, 0, 2, 1, 1, 0, 0, 0, 1, 0, 0,
      2, 0, 1, 1, 0, 0, 3, 0, 0, 1, 2, 1, 2, 1;
  compute_hyperbolic_indices(ndims, level, p, level_indices);
  BOOST_CHECK(matrix_equals(level_indices, gold_indices));
}

// ------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_surrogates_reduced_indices) {
  int ndims = 2, level = 3;

  Eigen::MatrixXi level_indices;
  Eigen::MatrixXi gold_indices;

  gold_indices.resize(ndims, 1 + ndims * level);
  gold_indices << 0, 1, 0, 2, 0, 3, 0, 0, 0, 1, 0, 2, 0, 3;
  compute_reduced_indices(ndims, level, level_indices);
  BOOST_CHECK(matrix_equals(level_indices, gold_indices));

  ndims = 3, level = 3;
  gold_indices.resize(ndims, 1 + ndims * level);
  gold_indices << 0, 1, 0, 0, 2, 0, 0, 3, 0, 0, 0, 0, 1, 0, 0, 2, 0, 0, 3, 0, 0,
      0, 0, 1, 0, 0, 2, 0, 0, 3;
  compute_reduced_indices(ndims, level, level_indices);
  BOOST_CHECK(matrix_equals(level_indices, gold_indices));
}

// ------------------------------------------------------------
