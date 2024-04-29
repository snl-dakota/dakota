/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "util_common.hpp"
#include "util_math_tools.hpp"

#define BOOST_TEST_MODULE dakota_MathToolsTest
#include <boost/test/included/unit_test.hpp>

using namespace dakota;
using namespace dakota::util;

// ------------------------------------------------------------

BOOST_AUTO_TEST_CASE(util_N_choose_K) {
  std::vector<int> kvals = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  std::vector<int> gold = {10, 45, 120, 210, 252, 210, 120, 45, 10, 1};
  std::vector<int> fvals;

  for (auto const& k : kvals) fvals.push_back(n_choose_k(10, k));

  BOOST_CHECK(fvals == gold);
}

// ------------------------------------------------------------

BOOST_AUTO_TEST_CASE(util_num_nonzeros) {
  Eigen::MatrixXi test_indices(6, 3);
  test_indices << 2, 0, 0, 1, 1, 0, 0, 2, 0, 1, 0, 1, 0, 1, 1, 0, 0, 2;

  Eigen::Map<Eigen::VectorXi> column(test_indices.col(1).data(),
                                     test_indices.rows());

  BOOST_CHECK(9 == num_nonzeros(test_indices));
  BOOST_CHECK(3 == num_nonzeros(column));
}

// ------------------------------------------------------------

BOOST_AUTO_TEST_CASE(util_nonzeros) {
  Eigen::VectorXi working_vec(10);
  working_vec << 1, 2, 0, 0, 5, 0, 7, 0, 9, 10;

  Eigen::VectorXi gold_vec(6);
  gold_vec << 0, 1, 4, 6, 8, 9;

  Eigen::VectorXi test_vec;
  nonzero(working_vec, test_vec);

  BOOST_CHECK(matrix_equals(test_vec, gold_vec));
}

// ------------------------------------------------------------

BOOST_AUTO_TEST_CASE(util_append_cols) {
  Eigen::MatrixXi working_mat(3, 2);
  working_mat << 1, 4, 2, 5, 3, 6;

  Eigen::MatrixXi new_mat(3, 2);
  new_mat << 7, 10, 8, 11, 9, 12;

  Eigen::MatrixXi gold_mat(3, 4);
  gold_mat << 1, 4, 7, 10, 2, 5, 8, 11, 3, 6, 9, 12;

  append_columns(new_mat, working_mat);

  BOOST_CHECK(matrix_equals(working_mat, gold_mat));
}

// ------------------------------------------------------------

BOOST_AUTO_TEST_CASE(util_p_norm) {
  Eigen::VectorXd vals(10);
  vals << 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0;

  BOOST_CHECK_CLOSE(140.50736087767584, p_norm(vals, 0.7), 1.e-8);
}

BOOST_AUTO_TEST_CASE(util_random_permutation) {
  const int num_pts = 9;
  const int seed = 2;
  const int num_features = 3;

  VectorXi gold_permutation_indices(num_pts);
  gold_permutation_indices << 1, 8, 4, 2, 0, 5, 7, 3, 6;

  VectorXi linear_indices = VectorXi::LinSpaced(num_pts, 0, num_pts - 1);

  VectorXi permutation_indices = linear_indices;

  std::cout << "\n"
            << "original vector: ";
  std::cout << permutation_indices.transpose() << "\n";

  random_permutation(num_pts, (unsigned int)22, permutation_indices);

  std::cout << "permuted vector: ";
  std::cout << permutation_indices.transpose() << "\n";

  BOOST_CHECK(permutation_indices == gold_permutation_indices);
}

BOOST_AUTO_TEST_CASE(util_create_cross_validation_folds) {
  const int num_pts = 9;
  const int seed = 2;
  const int num_folds = 4;
  const int num_features = 3;

  std::vector<VectorXi> cv_folds, gold_cv_folds;
  gold_cv_folds.resize(num_folds);
  gold_cv_folds[0].resize(2);
  gold_cv_folds[0] << 1, 8;
  gold_cv_folds[1].resize(2);
  gold_cv_folds[1] << 4, 2;
  gold_cv_folds[2].resize(2);
  gold_cv_folds[2] << 0, 5;
  gold_cv_folds[3].resize(3);
  gold_cv_folds[3] << 7, 3, 6;

  create_cv_folds(num_folds, num_pts, cv_folds);
  std::cout << "\n\n";
  for (int i = 0; i < num_folds; i++) {
    std::cout << "fold indices " << i << ": ";
    std::cout << cv_folds[i].transpose() << "\n";
    BOOST_CHECK(cv_folds[i] == gold_cv_folds[i]);
  }
}

BOOST_AUTO_TEST_CASE(util_create_random_real_matrices) {
  double tight_tol = 1.0e-15;
  // std::cout << std::fixed << std::showpoint;
  // std::cout << std::setprecision(15);

  MatrixXd gold_random_matrix(3, 2);
  gold_random_matrix << 0.208460537066790, 0.701962978742542, 0.481681059226785,
      0.730506643823000, 0.420538031594022, 0.637479150350550;

  MatrixXd random_matrix = create_uniform_random_double_matrix(3, 2);
  // std::cout << "\n\n(0, 1) Random matrix:\n\n" << random_matrix << "\n";
  BOOST_CHECK(matrix_equals(random_matrix, gold_random_matrix, tight_tol));

  MatrixXd gold_scaled_random_matrix(2, 3);
  gold_scaled_random_matrix << 1.395270773068832, 1.254321804096532,
      -1.284416306597278, 0.365419989071186, -1.782547124610875,
      -1.556054711238494;

  MatrixXd scaled_random_matrix =
      create_uniform_random_double_matrix(2, 3, 15, true, -2.0, 2.0);
  // std::cout << "\n(-2, 2) Random matrix:\n\n" << scaled_random_matrix <<
  // "\n\n";
  BOOST_CHECK(matrix_equals(scaled_random_matrix, gold_scaled_random_matrix,
                            tight_tol));
}
