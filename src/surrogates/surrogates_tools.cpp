/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "surrogates_tools.hpp"

#include "util_math_tools.hpp"

namespace dakota {
namespace surrogates {

void compute_next_combination(int num_dims, int level, VectorXi& index,
                              bool& extend, int& h, int& t) {
  if (!extend) {
    t = level;
    h = 0;
    index[0] = level;
    for (int d = 1; d < num_dims; d++) index[d] = 0;
  } else {
    if (1 < t) h = 0;

    t = index[h];
    index[h] = 0;
    index[0] = t - 1;
    index[h + 1] = index[h + 1] + 1;
    h += 1;
  }
  extend = (index[num_dims - 1] != level);
}

// ------------------------------------------------------------

void size_level_index_vector(int num_dims, int level, MatrixXi& indices) {
  int num_indices;
  if (level > 0) {
    num_indices = util::n_choose_k(num_dims + level, num_dims) -
                  util::n_choose_k(num_dims + level - 1, num_dims);
    indices.resize(num_indices, num_dims);
    bool extend = false;
    int h = 0, t = 0;
    VectorXi index(num_dims);  // important this is initialized to zero
    index *= 0;
    int i = 0;
    while (true) {
      compute_next_combination(num_dims, level, index, extend, h, t);
      for (int d = 0; d < num_dims; d++) indices(i, d) = index[d];
      i++;

      if (!extend) break;
    }
  } else {
    indices.resize(1, num_dims);
  }
}

// ------------------------------------------------------------

void compute_hyperbolic_subdim_level_indices(int num_dims, int level,
                                             int num_active_dims, double p,
                                             MatrixXi& indices) {
  silence_unused_args(num_dims);
  double eps = 100 * std::numeric_limits<double>::epsilon();

  MatrixXi working_indices(num_active_dims, 1000);
  int l = num_active_dims;
  int num_indices = 0;
  while (true) {
    MatrixXi level_data;
    size_level_index_vector(num_active_dims, l, level_data);
    level_data.transposeInPlace();
    for (int i = 0; i < level_data.cols(); ++i) {
      Eigen::Map<VectorXi> index(level_data.col(i).data(), num_active_dims);
      if (util::num_nonzeros(index) == num_active_dims) {
        double pnorm = util::p_norm(index, p);
        if ((pnorm > level - 1 + eps) && (pnorm < level + eps)) {
          if (num_indices >= working_indices.cols())
            working_indices.resize(working_indices.rows(), num_indices + 1000);
          Eigen::Map<VectorXi> col(working_indices.col(num_indices).data(),
                                   num_active_dims);
          col = index;
          num_indices++;
        }
      }
    }
    l++;
    if (l > level) break;
  }
  // Remove unwanted memory
  indices.resize(num_indices, num_active_dims);
  for (int row = 0; row < indices.rows(); ++row) {
    for (int col = 0; col < indices.cols(); ++col) {
      indices(row, col) = working_indices(col, row);
    }
  }
}

// ------------------------------------------------------------

void compute_hyperbolic_level_indices(int num_dims, int level, double p,
                                      MatrixXi& indices) {
  if (level == 0)
    indices = MatrixXi::Zero(num_dims, 1);
  else {
    indices.resize(num_dims, num_dims);
    for (int i = 0; i < num_dims; ++i) {
      VectorXi index;  // need to initialize values to zero
      index.setZero(num_dims);
      index(i) = level;
      for (int d = 0; d < num_dims; ++d) indices(d, i) = index(d);
    }

    for (int d = 2; d < std::min(level + 1, num_dims + 1); d++) {
      MatrixXi level_comb;
      compute_hyperbolic_subdim_level_indices(num_dims, level, d, p,
                                              level_comb);

      if (level_comb.rows() == 0) break;

      level_comb.transposeInPlace();

      MatrixXi dims_comb;
      size_level_index_vector(num_dims, d, dims_comb);
      MatrixXi dim_indices(dims_comb.cols(), dims_comb.rows());
      dims_comb.transposeInPlace();
      int num_dim_indices = 0;
      for (int i = 0; i < dims_comb.cols(); i++) {
        Eigen::Map<VectorXi> index(dims_comb.col(i).data(), num_dims);
        if (util::num_nonzeros(index) == d) {
          Eigen::Map<VectorXi> col(dim_indices.col(num_dim_indices).data(),
                                   num_dims);
          col = index;
          num_dim_indices++;
        }
      }
      // Chop off unused memory;
      dim_indices.conservativeResize(num_dims, num_dim_indices);

      MatrixXi new_indices(num_dims, num_dim_indices * level_comb.cols());
      int num_new_indices = 0;
      for (int i = 0; i < dim_indices.cols(); i++) {
        Eigen::Map<VectorXi> dim_index(dim_indices.col(i).data(), num_dims);
        VectorXi I;
        util::nonzero(dim_index, I);
        for (int j = 0; j < level_comb.cols(); j++) {
          auto index = new_indices.col(num_new_indices);
          index.setZero();
          for (int k = 0; k < I.size(); ++k) {
            index[I[k]] = level_comb(k, j);
          }
          num_new_indices++;
        }
      }
      util::append_columns(new_indices, indices);
    }
  }
}

// ------------------------------------------------------------

void compute_hyperbolic_indices(int num_dims, int level, double p,
                                MatrixXi& indices) {
  compute_hyperbolic_level_indices(num_dims, 0, p, indices);
  for (int l = 1; l < level + 1; ++l) {
    MatrixXi level_indices;
    compute_hyperbolic_level_indices(num_dims, l, p, level_indices);
    util::append_columns(level_indices, indices);
  }
}

// ------------------------------------------------------------

void compute_reduced_indices(int num_dims, int level, MatrixXi& indices) {
  indices = MatrixXi::Zero(num_dims, 1);
  for (int lev = 1; lev < level + 1; ++lev) {
    MatrixXi level_indices = lev * MatrixXi::Identity(num_dims, num_dims);
    util::append_columns(level_indices, indices);
  }
}

// ------------------------------------------------------------

void fd_check_gradient(Surrogate& surr, const MatrixXd& sample,
                       MatrixXd& fd_error, const int num_steps) {
  int num_vars = sample.cols();
  fd_error.resize(num_steps, num_vars);

  MatrixXd ref_sample_repeated(num_steps, num_vars);
  MatrixXd ref_grad;
  MatrixXd perturb_plus(num_steps, num_vars);
  MatrixXd perturb_minus(num_steps, num_vars);
  VectorXd value_perturb_plus, value_perturb_minus;
  VectorXd ref_grad_repeated(num_steps);
  ref_grad = surr.gradient(sample);
  VectorXd scale_factors = surr.dataScaler.get_scaler_features_scale_factors();

  /* create h array */
  VectorXd h(num_steps);
  for (int i = 0; i < num_steps; i++) h(i) = pow(10.0, -(i + 1));

  /* set up array of anchor points */
  for (int i = 0; i < num_vars; i++)
    ref_sample_repeated.col(i).setConstant(sample(0, i));

  for (int i = 0; i < num_vars; i++) {
    perturb_plus = ref_sample_repeated;
    perturb_minus = ref_sample_repeated;
    perturb_plus.col(i) += h * scale_factors(i);
    perturb_minus.col(i) -= h * scale_factors(i);
    value_perturb_plus = surr.value(perturb_plus);
    value_perturb_minus = surr.value(perturb_minus);
    ref_grad_repeated.setConstant(ref_grad(0, i));
    fd_error.col(i) =
        ((value_perturb_plus - value_perturb_minus).cwiseQuotient(2.0 * h) -
         ref_grad_repeated)
            .cwiseAbs();
  }
}

// ------------------------------------------------------------

void fd_check_hessian(Surrogate& surr, const MatrixXd& sample,
                      MatrixXd& fd_error, const int num_steps) {
  int num_vars = sample.cols();

  /* compute the fd error for each independent
   * component of the hessian */
  int num_hess_components = num_vars * (num_vars + 1) / 2;
  fd_error.resize(num_steps, num_hess_components);

  MatrixXd ref_sample_repeated(num_steps, num_vars);
  MatrixXd ref_hessian;
  MatrixXd perturb_plus_single(num_steps, num_vars);
  MatrixXd perturb_minus_single(num_steps, num_vars);
  MatrixXd perturb_plus_both(num_steps, num_vars);
  MatrixXd perturb_minus_both(num_steps, num_vars);
  MatrixXd perturb_plus_minus(num_steps, num_vars);
  MatrixXd perturb_minus_plus(num_steps, num_vars);
  VectorXd value_perturb_plus_single, value_perturb_minus_single;
  VectorXd value_perturb_plus_both, value_perturb_minus_both;
  VectorXd value_perturb_plus_minus, value_perturb_minus_plus;
  VectorXd ref_hessian_repeated(num_steps);
  VectorXd scale_factors = surr.dataScaler.get_scaler_features_scale_factors();
  VectorXd ref_value;
  VectorXd ref_value_repeated(num_steps);

  /* unperturbed value and hessian */
  ref_value = surr.value(sample);
  ref_hessian = surr.hessian(sample);

  /* create h and reference value arrays */
  VectorXd h(num_steps);
  for (int i = 0; i < num_steps; i++) {
    h(i) = pow(10.0, -(i + 1));
    ref_value_repeated.setConstant(ref_value(0, 0));
  }

  /* set up array of anchor points */
  for (int i = 0; i < num_vars; i++) {
    ref_sample_repeated.col(i).setConstant(sample(0, i));
  }

  int k = 0;
  for (int i = 0; i < num_vars; i++) {
    for (int j = i; j < num_vars; j++) {
      ref_hessian_repeated.setConstant(ref_hessian(i, j));
      perturb_plus_single = ref_sample_repeated;
      perturb_minus_single = ref_sample_repeated;

      perturb_plus_single.col(i) += h * scale_factors(i);
      perturb_minus_single.col(i) -= h * scale_factors(i);

      value_perturb_plus_single = surr.value(perturb_plus_single);
      value_perturb_minus_single = surr.value(perturb_minus_single);

      if (i == j) {
        fd_error.col(k) =
            ((value_perturb_plus_single - 2.0 * ref_value_repeated +
              value_perturb_minus_single)
                 .cwiseQuotient(h.cwiseProduct(h)) -
             ref_hessian_repeated)
                .cwiseAbs();
      } else {
        perturb_plus_both = ref_sample_repeated;
        perturb_minus_both = ref_sample_repeated;
        perturb_plus_minus = ref_sample_repeated;
        perturb_minus_plus = ref_sample_repeated;

        perturb_plus_both.col(i) += h * scale_factors(i);
        perturb_plus_both.col(j) += h * scale_factors(j);
        perturb_minus_both.col(i) -= h * scale_factors(i);
        perturb_minus_both.col(j) -= h * scale_factors(j);
        perturb_plus_minus.col(i) += h * scale_factors(i);
        perturb_plus_minus.col(j) -= h * scale_factors(j);
        perturb_minus_plus.col(i) -= h * scale_factors(i);
        perturb_minus_plus.col(j) += h * scale_factors(j);

        value_perturb_plus_both = surr.value(perturb_plus_both);
        value_perturb_minus_both = surr.value(perturb_minus_both);
        value_perturb_plus_minus = surr.value(perturb_plus_minus);
        value_perturb_minus_plus = surr.value(perturb_minus_plus);

        fd_error.col(k) = ((value_perturb_plus_both + value_perturb_minus_both -
                            value_perturb_plus_minus - value_perturb_minus_plus)
                               .cwiseQuotient(2.0 * h.cwiseProduct(2.0 * h)) -
                           ref_hessian_repeated)
                              .cwiseAbs();
      }
      k += 1;
    }
  }
}

// ------------------------------------------------------------

}  // namespace surrogates
}  // namespace dakota
