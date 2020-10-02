/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "SurrogatesBase.hpp"
#include "util_metrics.hpp"
#include "util_math_tools.hpp"


namespace dakota {
namespace surrogates {

Surrogate::Surrogate() : numQOI(0) {}

Surrogate::Surrogate(const ParameterList &param_list) {
  numQOI = 0;
  silence_unused_args(param_list);
}


Surrogate::Surrogate(const MatrixXd &samples,
                     const MatrixXd &response,
                     const ParameterList& param_list) {
  numQOI = 0;
  silence_unused_args(samples, response, param_list);
}

Surrogate::~Surrogate(){}

MatrixXd Surrogate::gradient(const MatrixXd &eval_points, int qoi) {
  silence_unused_args(eval_points, qoi);
  throw(std::runtime_error("Surrogate does not implement gradient(...)"));
}

MatrixXd Surrogate::hessian(const MatrixXd &eval_point, int qoi) {
  silence_unused_args(eval_point, qoi);
  throw(std::runtime_error("Surrogate does not implement hessian(...)"));
}

void Surrogate::set_options(const ParameterList &options) {
  configOptions = options;
}

void Surrogate::get_options(ParameterList &options) {
  options = configOptions;
}

void Surrogate::print_options() {
  std::cout << configOptions << "\n";
}

/// Evalute metrics at specified points (within surrogates)
VectorXd Surrogate::evaluate_metrics(const StringArray &mnames,
  const MatrixXd &points, const MatrixXd &ref_values) {

  const int num_metrics = mnames.size();
  /* Assuming numQOI = 1 */
  VectorXd surr_values(ref_values.size());
  VectorXd metrics(num_metrics);

  for (int m = 0; m < num_metrics; m++) {
    surr_values = this->value(points);
    metrics(m) = util::compute_metric(surr_values,
      ref_values.col(0), mnames[m]);
  }
  return metrics;
}

/// K-folds cross validation (within surrogates)
VectorXd Surrogate::cross_validate(const MatrixXd &samples,
    const MatrixXd &response, const StringArray &mnames,
    const int num_folds, const int seed) {

  const int num_metrics = mnames.size();
  VectorXd cv_results = VectorXd::Zero(num_metrics);
  VectorXd metrics_values(num_metrics);

  const int num_samples = samples.rows();
  const int num_features = samples.cols();
  std::vector<VectorXi> cv_folds;
  VectorXi fold_indices;
  MatrixXd train_samples, train_response;
  MatrixXd val_samples, val_response;

  int num_train_samples, num_val_samples;
  int samples_index, train_index;

  util::create_cv_folds(num_folds, num_samples, cv_folds, seed);

  // clone the surrogate's configuration so CV doesn't invalidate *this
  std::shared_ptr<Surrogate> cv_surrogate = this->clone();

  for (int i = 0; i < num_folds; i++) {
    /* validation samples */
    fold_indices = cv_folds[i];
    num_val_samples = fold_indices.size();
    val_samples.resize(num_val_samples, num_features);
    val_response.resize(num_val_samples, 1);
    for (int j = 0; j < num_val_samples; j++) {
      samples_index = fold_indices(j);
      val_samples.row(j) = samples.row(samples_index);
      val_response(j, 0) = response(samples_index, 0);
    }

    /* training samples */
    num_train_samples = num_samples - num_val_samples;
    train_samples.resize(num_train_samples, num_features);
    train_response.resize(num_train_samples, 1);
    train_index = 0;
    for (int k = 0; k < num_folds; k++) {
      if (k != i) {
        fold_indices = cv_folds[k];
        for (int j = 0; j < fold_indices.size(); j++) {
          samples_index = fold_indices(j);
          train_samples.row(train_index) = samples.row(samples_index);
          train_response(train_index, 0) = response(samples_index, 0);
          train_index++;
        }
      }
    }

    cv_surrogate->build(train_samples, train_response);
    metrics_values =
      cv_surrogate->evaluate_metrics(mnames, val_samples, val_response);
    cv_results += metrics_values;

  }

  cv_results /= double(num_folds);
  return cv_results;

}

} // namespace surrogates
} // namespace dakota

BOOST_CLASS_EXPORT_IMPLEMENT(dakota::surrogates::Surrogate)
