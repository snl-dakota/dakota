/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_TOOLS_HPP
#define DAKOTA_SURROGATES_TOOLS_HPP

#include "SurrogatesBase.hpp"
#include "util_data_types.hpp"

namespace dakota {

/// namespace for new %Dakota surrogates module
namespace surrogates {

/**
 *  \param[in] num_dims Dimension of the feature space.
 *  \param[in] level Total order in each row of indices. Should be >= 1.
 *  \param[inout] index Vector of ints that specifies the powers for each
 *  term in the basis.
 *  \param[inout] extend Bool for whether to continue with the computation
 *  of basis indices.
 *  \param[inout] h Working variable for basis enumeration.
 *  \param[inout] t Working variable for basis enumeration.
 */
void compute_next_combination(int num_dims, int level, VectorXi& index,
                              bool& extend, int& h, int& t);

/**
 *  \brief Compute a matrix of basis indices for given dimension and level. Each
 * row of the matrix sums to level. \param[in] num_dims Dimension of the feature
 * space. \param[in] level Total order in each row of indices. Should be >= 1.
 *  \param[out] indices Matrix of indices - (num_terms by num_dims).
 */
void size_level_index_vector(int num_dims, int level, MatrixXi& indices);

/**
 *  \brief Compute a matrix of indices for a submatrix (i.e. up to the
 *  active dimensions column) of indices produced by
 *  size_level_index_vector(num_dims, level, indices) where each feature
 *  has a component > 0 and respects the p-norm cutoff.
 *  \param[in] num_dims Dimension of the feature space.
 *  \param[in] level Total order in each row of indices. Should be >= 1.
 *  \param[in] num_active_dims The # of active features and end index of the
 * submatrix. \param[in] p Real value for p-norm. \param[out] indices Matrix of
 * indices - (num_terms by num_active_dims)
 */
void compute_hyperbolic_subdim_level_indices(int num_dims, int level,
                                             int num_active_dims, double p,
                                             MatrixXi& indices);

/**
 *  \brief Compute the hyperbolic cross indices for a given level
 *  \param[in] num_dims Dimension of the feature space.
 *  \param[in] level Total order in each column of indices. Should be >= 0.
 *  \param[in] p Real value for p-norm.
 *  \param[out] indices Matrix of indices - (num_dims by num_terms)
 */
void compute_hyperbolic_level_indices(int num_dims, int level, double p,
                                      MatrixXi& indices);

/**
 *  \brief Compute the hyperbolic cross indices for all levels up to level.
 *  \param[in] num_dims Dimension of the feature space.
 *  \param[in] level Highest level to compute basis indices for.
 *  \param[in] p Real value for p-norm.
 *  \param[out] indices Matrix of indices - (num_dims by num_terms).
 */
void compute_hyperbolic_indices(int num_dims, int level, double p,
                                MatrixXi& indices);

/**
 *  \brief Compute the reduced indices for all levels up to level.
 *  \param[in] num_dims Dimension of the feature space.
 *  \param[in] level Highest level to compute basis indices for.
 *  \param[out] indices Matrix of indices - (num_dims by num_terms).
 */
void compute_reduced_indices(int num_dims, int level, MatrixXi& indices);

/**
 *  \brief Perform a centered finite difference check of a Surrogate's
 *  gradient method.
 *  \param[in] surr Reference to a Surrogate.
 *  \param[in] sample Point to evaluate the gradient at - (1 by numVariables).
 *  \param[out] fd_error Matrix of finite difference error for each component
 *  of the gradient - (num_steps by numVariables).
 *  \param[in] num_steps Number of increments (N) for the finite difference. The
 * increment vector h = 10**(-i), i = 1, ..., N.
 */
void fd_check_gradient(Surrogate& surr, const MatrixXd& sample,
                       MatrixXd& fd_error, const int num_steps = 10);

/**
 *  \brief Perform a centered finite difference check of a Surrogate's
 *  Hessian method.
 *  \param[in] surr Reference to a Surrogate.
 *  \param[in] sample Point to evaluate the Hessian at - (1 by numVariables).
 *  \param[out] fd_error Matrix of finite difference error for each
 *  independent component of the Hessian. There are
 * numVariables*(numVariables+1)/2 = numInd components - (num_steps by numInd).
 *  of the Hessian - (num_steps by numVariables).
 *  \param[in] num_steps Number of increments (N) for the finite difference. The
 * increment vector h = 10**(-i), i = 1, ..., N.
 */
void fd_check_hessian(Surrogate& surr, const MatrixXd& sample,
                      MatrixXd& fd_error, const int num_steps = 10);

}  // namespace surrogates
}  // namespace dakota

#endif  // include guard
