/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "util_math_tools.hpp"

#include "util_common.hpp"

#include <boost/version.hpp>
#if (BOOST_VERSION < 107000) && !defined(BOOST_ALLOW_DEPRECATED_HEADERS)
//could alternately use: #define BOOST_PENDING_INTEGER_LOG2_HPP 1
#define BOOST_ALLOW_DEPRECATED_HEADERS 1
#include <boost/random/mersenne_twister.hpp>
#undef BOOST_ALLOW_DEPRECATED_HEADERS
#else
#include <boost/random/mersenne_twister.hpp>
#endif
#include <boost/random/uniform_int_distribution.hpp>
#include <random>

namespace dakota {
namespace util {

// ------------------------------------------------------------

int n_choose_k(int n, int k) {
  double value = 1.0;
  for (int i = 0; i < n - k; i++)
    value *= (double)(n - i) / (double)(n - k - i);

  return (int)round(value);
}

// ------------------------------------------------------------

void random_permutation(const int num_pts, const unsigned int seed,
                        VectorXi& permutations) {
  // Using Boost MT since need it anyway for unif int dist
  boost::random::mt19937 generator(seed);
  // Using Boost unif int dist for cross-platform stability
  boost::random::uniform_int_distribution<> dist(0, num_pts - 1);
  int index, tmp;
  for (int i = 0; i < num_pts; i++) {
    index = dist(generator);
    tmp = permutations(i);
    permutations(i) = permutations(index);
    permutations(index) = tmp;
  }
}

// ------------------------------------------------------------

void create_cv_folds(const int num_folds, const int num_pts,
                     std::vector<VectorXi>& fold_indices, const int seed) {
  if (num_pts < num_folds) {
    error("Number of cv folds > number of points");
  }

  fold_indices.resize(num_folds);
  VectorXi permutation_indices = VectorXi::LinSpaced(num_pts, 0, num_pts - 1);

  if (seed == 0) {
    random_permutation(num_pts, (unsigned int)std::time(0),
                       permutation_indices);
  } else if (seed > 0) {
    random_permutation(num_pts, (unsigned int)seed, permutation_indices);
  }

  int start_ind, end_ind, max_fold_size, fold_size;
  max_fold_size = std::round(double(num_pts) / double(num_folds));
  start_ind = 0;

  for (int k = 1; k <= num_folds; k++) {
    if (k != num_folds)
      end_ind = start_ind + max_fold_size;
    else
      end_ind = num_pts;
    fold_size = end_ind - start_ind;
    fold_indices[k - 1] = permutation_indices.segment(start_ind, fold_size);
    start_ind = end_ind;
  }
}


MatrixXd create_uniform_random_double_matrix(const int num_rows,
                                             const int num_cols)
{
  return create_uniform_random_double_matrix(num_rows, num_cols, 22);
}


MatrixXd create_uniform_random_double_matrix(const int num_rows,
                                             const int num_cols,
                                             const unsigned int seed)
{
  return create_uniform_random_double_matrix(num_rows, num_cols, seed, false, 0.0, 1.0);
}


MatrixXd create_uniform_random_double_matrix(const int num_rows,
                                             const int num_cols,
                                             const unsigned int seed,
                                             bool transform, const double low,
                                             const double high) {
  if (num_rows < 1 || num_cols < 1)
    throw std::runtime_error("Number of rows and columns must > 1.");

  MatrixXd random_matrix(num_rows, num_cols);
  boost::mt19937 rng(seed);

  for (int i = 0; i < num_rows; ++i)
    for (int j = 0; j < num_cols; ++j) random_matrix(i, j) = rng();

  random_matrix /= static_cast<double>(rng.max());  // now in (0, 1)

  if (transform) {
    if (low > high)
      throw std::runtime_error("Lower limit > upper limit for random array.");
    random_matrix *= high - low;
    random_matrix.array() += low;
  }

  return random_matrix;
}

}  // namespace util
}  // namespace dakota
