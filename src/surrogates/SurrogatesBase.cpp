/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "SurrogatesBase.hpp"

#include "util_math_tools.hpp"
#include "util_metrics.hpp"

namespace dakota {
namespace surrogates {

Surrogate::Surrogate() : numQOI(0) {}

Surrogate::Surrogate(const ParameterList& param_list) {
  numQOI = 0;
  silence_unused_args(param_list);
}

Surrogate::Surrogate(const MatrixXd& samples, const MatrixXd& response,
                     const ParameterList& param_list) {
  numQOI = 0;
  silence_unused_args(samples, response, param_list);
}

Surrogate::~Surrogate() {}

VectorXd Surrogate::value(const MatrixXd& eval_points) {
  silence_unused_args(eval_points);
  throw(std::runtime_error("Surrogate does not implement value(...)"));
}

VectorXd Surrogate::values(const MatrixXd& eval_points) {
  silence_unused_args(eval_points);
  throw(std::runtime_error("Surrogate does not implement values(...)"));
}

MatrixXd Surrogate::gradient(const MatrixXd& eval_points) {
  silence_unused_args(eval_points);
  throw(std::runtime_error("Surrogate does not implement gradient(...)"));
}

MatrixXd Surrogate::gradients(const MatrixXd& eval_points) {
  silence_unused_args(eval_points);
  throw(std::runtime_error("Surrogate does not implement gradients(...)"));
}

MatrixXd Surrogate::hessian(const MatrixXd& eval_point) {
  silence_unused_args(eval_point);
  throw(std::runtime_error("Surrogate does not implement hessian(...)"));
}

MatrixXd Surrogate::hessians(const MatrixXd& eval_point) {
  silence_unused_args(eval_point);
  throw(std::runtime_error("Surrogate does not implement hessians(...)"));
}

void Surrogate::variable_labels(const std::vector<std::string>& var_labels) {
  variableLabels = var_labels;
}

const std::vector<std::string>& Surrogate::variable_labels() const {
  return variableLabels;
}

void Surrogate::response_labels(const std::vector<std::string>& resp_labels) {
  responseLabels = resp_labels;
}

const std::vector<std::string>& Surrogate::response_labels() const {
  return responseLabels;
}

void Surrogate::set_options(const ParameterList& options) {
  configOptions = options;
}

void Surrogate::get_options(ParameterList& options) { options = configOptions; }

void Surrogate::print_options() { std::cout << configOptions << "\n"; }

std::shared_ptr<Surrogate> Surrogate::load(const std::string& infile,
                                           const bool binary) {
  std::shared_ptr<Surrogate> surr_in;
  load(infile, binary, surr_in);
  return surr_in;
}

VectorXd Surrogate::evaluate_metrics(const StringArray& mnames,
                                     const MatrixXd& points,
                                     const MatrixXd& ref_values) {
  const int num_metrics = mnames.size();
  /* Assuming numQOI = 1 */
  VectorXd surr_values(ref_values.size());
  VectorXd metrics(num_metrics);

  for (int m = 0; m < num_metrics; m++) {
    surr_values = this->value(points);
    metrics(m) =
        util::compute_metric(surr_values, ref_values.col(0), mnames[m]);
  }
  return metrics;
}

VectorXd Surrogate::cross_validate(const MatrixXd& samples,
                                   const MatrixXd& response,
                                   const StringArray& mnames,
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
  // ParameterList cv_surrogate_options;
  // cv_surrogate->get_options(cv_surrogate_options);
  // int verbosity_level = cv_surrogate_options.get<int>("verbosity");
  int verbosity_level = configOptions.get<int>("verbosity");

  for (int i = 0; i < num_folds; i++) {
    if (verbosity_level > 0) {
      std::cout << "\nCross-validation fold " << i + 1 << "/" << num_folds
                << "\n\n";
    }
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

}  // namespace surrogates
}  // namespace dakota

BOOST_CLASS_EXPORT_IMPLEMENT(dakota::surrogates::Surrogate)
