/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "common_utils.hpp"
#include "util_math_tools.hpp"

#include <Teuchos_UnitTestHarness.hpp>

using namespace dakota::util;

// ------------------------------------------------------------

TEUCHOS_UNIT_TEST(util, N_choose_K)
{
  std::vector<int> kvals = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  std::vector<int> gold  = { 10, 45, 120, 210, 252, 210, 120, 45, 10, 1 };
  std::vector<int> fvals;

  for (auto const & k : kvals)
    fvals.push_back(n_choose_k(10,k));

  TEST_ASSERT(fvals == gold);
}

// ------------------------------------------------------------

TEUCHOS_UNIT_TEST(util, num_nonzeros)
{
  Eigen::MatrixXi test_indices(6, 3);
  test_indices << 2, 0, 0,
                  1, 1, 0,
                  0, 2, 0,
                  1, 0, 1,
                  0, 1, 1,
                  0, 0, 2 ;

  Eigen::Map<Eigen::VectorXi> column(test_indices.col(1).data(), test_indices.rows());

  TEST_ASSERT(9 == num_nonzeros(test_indices));
  TEST_ASSERT(3 == num_nonzeros(column));
}

// ------------------------------------------------------------

TEUCHOS_UNIT_TEST(util, nonzeros)
{
  Eigen::VectorXi working_vec(10);
  working_vec << 1, 2, 0, 0, 5, 0, 7, 0, 9, 10;

  Eigen::VectorXi gold_vec(6);
  gold_vec << 0, 1, 4, 6, 8, 9;

  Eigen::VectorXi test_vec;
  nonzero(working_vec, test_vec);

  TEST_ASSERT(matrix_equals(test_vec, gold_vec));
}

// ------------------------------------------------------------

TEUCHOS_UNIT_TEST(util, append_cols)
{
  Eigen::MatrixXi working_mat(3,2);
  working_mat << 1, 4,
                 2, 5,
                 3, 6 ;

  Eigen::MatrixXi new_mat(3,2);
  new_mat << 7, 10,
             8, 11,
             9, 12 ;

  Eigen::MatrixXi gold_mat(3, 4);
  gold_mat << 1, 4, 7, 10,
              2, 5, 8, 11,
              3, 6, 9, 12 ;

  append_columns(new_mat, working_mat);

  TEST_ASSERT(matrix_equals(working_mat, gold_mat));
}

// ------------------------------------------------------------

TEUCHOS_UNIT_TEST(util, p_norm)
{
  Eigen::VectorXd vals(10);
  vals << 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0;

  TEST_FLOATING_EQUALITY(140.50736087767584, p_norm(vals, 0.7), 1.e-10);
}

// ------------------------------------------------------------
