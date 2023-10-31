/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_UTIL_MATH_TOOLS_HPP
#define DAKOTA_UTIL_MATH_TOOLS_HPP

#include "util_data_types.hpp"

namespace dakota {
namespace util {

/**
 *  \brief Calculate Binomial coefficient n choose k
 *  \param[in] n Number of elements in set
 *  \param[in] k Number of elements in subset k where n >= k >= 0
 *  \returns Number of ways to choose an (unordered) subset of k elements from a
 * fixed set of n elements
 */
int n_choose_k(int n, int k);

/**
 *  \brief Caclulate and return number of nonzero entries in vector or matrix
 *  \param[in] mat Incoming vector or matrix
 *  \returns Number of nonzeros
 */
template <typename T>
int num_nonzeros(const T& mat) {
  return (mat.array() != 0).count();
}

/**
 *  \brief Create a vector of indices based on nonzero entries in input vector
 *  \param[in] v Incoming vector
 *  \param[out] result Vector having values at nonzero locations of incoming
 * vector and value equal to ordinal of occurrence
 */
template <typename T1, typename T2>
void nonzero(const T1& v, T2& result) {
  int num_nonzeros = 0;
  T2 working_vec = v;
  for (int i = 0; i < v.size(); ++i) {
    if (v[i] != 0) {
      working_vec[num_nonzeros] = i;
      num_nonzeros++;
    }
  }

  result.resize(num_nonzeros);
  for (int i = 0; i < num_nonzeros; ++i) result(i) = working_vec(i);
}

/**
 *  \brief Append columns of input matrix to existing matrix
 *  \param[in] new_cols Incoming matrix of column vectors to append
 *  \param[out] target Matrix to augment with appended columns
 */
template <typename T1, typename T2>
void append_columns(const T1& new_cols, T2& target) {
  if ((target.rows() != new_cols.rows()) && (new_cols.cols() > 0))
    throw std::runtime_error("Incompatible row dimensions for append_columns.");

  int num_orig_cols = target.cols();
  target.conservativeResize(Eigen::NoChange, num_orig_cols + new_cols.cols());
  for (int i = 0; i < new_cols.cols(); ++i)
    target.col(num_orig_cols + i) = new_cols.col(i);
}

/**
 *  \brief Caclulate and return p-norm of a vector
 *  \param[in] v Incoming vector
 *  \param[in] p Order or norm to compute
 *  \returns p-norm of incoming vector
 */
template <typename T>
double p_norm(const T& v, double p) {
  double sum = 0.;
  for (int i = 0; i < v.size(); ++i) sum += std::pow(std::abs((double)v(i)), p);

  return std::pow(sum, 1. / p);
}

/// Random permutation of int array
void random_permutation(const int num_pts, const unsigned int seed,
                        VectorXi& permutations);

/// Generate indices for cross validation folds
void create_cv_folds(const int num_folds, const int num_pts,
                     std::vector<VectorXi>& fold_indices, const int seed = 22);

/// Generate a real-valued matrix of uniformly distributed random values.
MatrixXd create_uniform_random_double_matrix(const int rows, const int cols,
                                             const unsigned int seed,
                                             bool transform,
                                             const double low,
                                             const double high);

/// Generate a real-valued matrix of uniformly distributed random values.
MatrixXd create_uniform_random_double_matrix(const int rows, const int cols,
                                             const unsigned int seed);

/// Generate a real-valued matrix of uniformly distributed random values.
MatrixXd create_uniform_random_double_matrix(const int rows, const int cols);


}  // namespace util
}  // namespace dakota

#endif  // include guard
