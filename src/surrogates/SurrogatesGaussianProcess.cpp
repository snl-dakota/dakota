/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "SurrogatesGaussianProcess.hpp"

#include "ROL_Algorithm.hpp"
#include "ROL_Bounds.hpp"
#include "ROL_LineSearchStep.hpp"
#include "SurrogatesGPObjective.hpp"
#include "Teuchos_oblackholestream.hpp"
#include "util_math_tools.hpp"

namespace dakota {
namespace surrogates {

GaussianProcess::GaussianProcess() { default_options(); }

GaussianProcess::GaussianProcess(const ParameterList& param_list) {
  default_options();
  configOptions = param_list;
  configOptions.validateParametersAndSetDefaults(defaultConfigOptions);
}

// Constructor that sets user-defined params but does not build.
GaussianProcess::GaussianProcess(const std::string& param_list_yaml_filename) {
  default_options();
  auto param_list =
      Teuchos::getParametersFromYamlFile(param_list_yaml_filename);
  configOptions = *param_list;
  configOptions.validateParametersAndSetDefaults(defaultConfigOptions);
}

// BMA NOTE: ParameterList::get() can throw, so direct delegation
// probably not good; might want to give a helpful message
GaussianProcess::GaussianProcess(const MatrixXd& samples,
                                 const MatrixXd& response,
                                 const ParameterList& param_list) {
  default_options();
  configOptions = param_list;
  build(samples, response);
}

GaussianProcess::GaussianProcess(const MatrixXd& samples,
                                 const MatrixXd& response,
                                 const std::string& param_list_yaml_filename) {
  default_options();
  auto param_list =
      Teuchos::getParametersFromYamlFile(param_list_yaml_filename);
  configOptions = *param_list;
  build(samples, response);
}

GaussianProcess::~GaussianProcess() {}

void GaussianProcess::build(const MatrixXd& samples, const MatrixXd& response) {
  configOptions.validateParametersAndSetDefaults(defaultConfigOptions);
  verbosity = configOptions.get<int>("verbosity");

  if (verbosity > 0) {
    if (verbosity == 1) {
      std::cout << "\nBuilding GaussianProcess\n\n";
    } else if (verbosity == 2) {
      std::cout << "\nBuilding GaussianProcess with configuration options\n"
                << configOptions << "\n";
    } else
      throw(std::runtime_error(
          "Invalid verbosity int for GaussianProcess surrogate"));
  }

  /* Standardize the response */
  bool standardize_response = configOptions.get<bool>("standardize response");
  if (standardize_response) {
    auto responseScaler = util::scaler_factory(
        util::DataScaler::scaler_type("standardization"), response);
    targetValues = responseScaler->scale_samples(response);
    responseOffset = responseScaler->get_scaler_features_offsets()(0);
    responseScaleFactor =
        responseScaler->get_scaler_features_scale_factors()(0);
  } else
    targetValues = response;

  numQOI = response.cols();
  numSamples = samples.rows();
  numVariables = samples.cols();
  eyeMatrix = MatrixXd::Identity(numSamples, numSamples);
  hasBestCholFact = false;
  kernel_type = configOptions.get<std::string>("kernel type");

  /* Kernel function */
  kernel = kernel_factory(kernel_type);

  /* Optimization-related data*/
  VectorXd sigma_bounds(2), nugget_bounds(2);
  MatrixXd length_scale_bounds;
  setup_hyperparameter_bounds(sigma_bounds, length_scale_bounds, nugget_bounds);
  const int num_restarts = configOptions.get<int>("num restarts");

  /* Scale the data and compute build squared distances */
  dataScaler =
      *(util::scaler_factory(util::DataScaler::scaler_type(
                                 configOptions.get<std::string>("scaler name")),
                             samples));
  dataScaler.scale_samples(samples, scaledBuildPoints);
  compute_build_dists();

  MatrixXd beta_bounds;
  estimateTrend = configOptions.sublist("Trend").get<bool>("estimate trend");
  if (estimateTrend) {
    polyRegression = std::make_shared<PolynomialRegression>(
        scaledBuildPoints, targetValues,
        configOptions.sublist("Trend").sublist("Options"));
    numPolyTerms = polyRegression->get_num_terms();
    polyRegression->compute_basis_matrix(scaledBuildPoints, basisMatrix);
    beta_bounds = MatrixXd::Ones(numPolyTerms, 2);
    beta_bounds.col(0) *= -betaBound;
    beta_bounds.col(1) *= betaBound;
  }

  /* size of thetaValues for squared exponential kernel and one QoI */
  thetaValues.resize(numVariables + 1);
  bestThetaValues.resize(numVariables + 1);
  betaValues.resize(numPolyTerms);
  bestBetaValues.resize(numPolyTerms);
  /* set the size of the GramMatrix and its derivatives */
  GramMatrix.resize(numSamples, numSamples);
  GramMatrixDerivs.resize(numVariables + 1);
  for (int k = 0; k < numVariables + 1; k++) {
    GramMatrixDerivs[k].resize(numSamples, numSamples);
  }

  /* DTS: if the nugget is being estimated, should the fixed value be set to
   * zero? */
  fixedNuggetValue =
      configOptions.sublist("Nugget").get<double>("fixed nugget");

  /* set up the initial guesses */
  // srand(configOptions.get<int>("gp seed"));
  MatrixXd initial_guesses;
  generate_initial_guesses(sigma_bounds, length_scale_bounds, nugget_bounds,
                           num_restarts, configOptions.get<int>("gp seed"),
                           initial_guesses);

  ROL::Ptr<std::ostream> outStream;
  Teuchos::oblackholestream bhs;
  outStream = ROL::makePtrFromRef(bhs);

  /* Uncomment if you'd like to print ROL's output to screen.
   * Useful for debugging */
  // outStream = ROL::makePtrFromRef(std::cout);

  /* No more reading in rol_params from an xml file
   * Set defaults in here instead */
  /*
  std::string paramfile = "rol_params.xml";
  auto rol_params = Teuchos::rcp(new ParameterList);
  Teuchos::updateParametersFromXmlFile(paramfile, rol_params.ptr());
  */

  auto gp_mle_rol_params =
      Teuchos::rcp(new ParameterList("GP_MLE_Optimization"));
  setup_default_optimization_params(gp_mle_rol_params);

  auto gp_objective = std::make_shared<GP_Objective>(*this);
  int dim = numVariables + 1 + numPolyTerms + numNuggetTerms;

  // Define algorithm
  ROL::Ptr<ROL::Step<double>> step =
      ROL::makePtr<ROL::LineSearchStep<double>>(*gp_mle_rol_params);
  ROL::Ptr<ROL::StatusTest<double>> status =
      ROL::makePtr<ROL::StatusTest<double>>(*gp_mle_rol_params);
  ROL::Algorithm<double> algo(step, status, false);

  /* set up parameter vectors and bounds */
  ROL::Ptr<std::vector<double>> x_ptr =
      ROL::makePtr<std::vector<double>>(dim, 0.0);
  ROL::StdVector<double> x(x_ptr);
  ROL::Ptr<ROL::Bounds<double>> bound;
  ROL::Ptr<std::vector<double>> lo_ptr =
      ROL::makePtr<std::vector<double>>(dim, 0.0);
  ROL::Ptr<std::vector<double>> hi_ptr =
      ROL::makePtr<std::vector<double>>(dim, 0.0);
  /* sigma bounds */
  (*lo_ptr)[0] = log(sigma_bounds(0));
  (*hi_ptr)[0] = log(sigma_bounds(1));
  /* length scale bounds */
  for (int i = 0; i < numVariables; i++) {
    if (length_scale_bounds.rows() > 1) {
      (*lo_ptr)[i + 1] = log(length_scale_bounds(i, 0));
      (*hi_ptr)[i + 1] = log(length_scale_bounds(i, 1));
    } else {
      (*lo_ptr)[i + 1] = log(length_scale_bounds(0, 0));
      (*hi_ptr)[i + 1] = log(length_scale_bounds(0, 1));
    }
  }
  if (estimateTrend) {
    for (int i = 0; i < numPolyTerms; i++) {
      (*lo_ptr)[numVariables + 1 + i] = beta_bounds(i, 0);
      (*hi_ptr)[numVariables + 1 + i] = beta_bounds(i, 1);
    }
  }
  if (estimateNugget) {
    (*lo_ptr)[dim - 1] = log(nugget_bounds(0));
    (*hi_ptr)[dim - 1] = log(nugget_bounds(1));
  }

  ROL::Ptr<ROL::Vector<double>> lop =
      ROL::makePtr<ROL::StdVector<double>>(lo_ptr);
  ROL::Ptr<ROL::Vector<double>> hip =
      ROL::makePtr<ROL::StdVector<double>>(hi_ptr);
  bound = ROL::makePtr<ROL::Bounds<double>>(lop, hip);

  std::vector<std::string> output;

  objectiveFunctionHistory.resize(num_restarts);
  objectiveGradientHistory.resize(num_restarts, dim);
  thetaHistory.resize(num_restarts, dim);

  double final_obj_value;
  VectorXd final_obj_gradient(dim);

  for (int i = 0; i < num_restarts; i++) {
    for (int j = 0; j < dim; ++j) {
      (*x_ptr)[j] = initial_guesses(i, j);
    }
    output = algo.run(x, *gp_objective, *bound, true, *outStream);
    for (int j = 0; j < thetaValues.size(); ++j) {
      (thetaValues)(j) = (*x_ptr)[j];
    }
    if (estimateTrend) {
      for (int j = 0; j < numPolyTerms; ++j) {
        betaValues(j) = (*x_ptr)[numVariables + 1 + j];
      }
    }
    if (estimateNugget) {
      estimatedNuggetValue = (*x_ptr)[numVariables + 1 + numPolyTerms];
    }
    /* get the final objective function value and gradient */
    negative_marginal_log_likelihood(true, true, final_obj_value,
                                     final_obj_gradient);
    if (final_obj_value < bestObjFunValue) {
      bestObjFunValue = final_obj_value;
      bestThetaValues = thetaValues;
      if (estimateTrend) bestBetaValues = betaValues;
      if (estimateNugget) bestEstimatedNuggetValue = estimatedNuggetValue;
    }
    objectiveFunctionHistory(i) = final_obj_value;
    objectiveGradientHistory.row(i) = final_obj_gradient;
    thetaHistory.row(i).head(numVariables + 1) = thetaValues;
    if (estimateTrend)
      thetaHistory.row(i).segment(numVariables + 1, numPolyTerms) = betaValues;
    if (estimateNugget) thetaHistory.row(i).tail(1)(0) = estimatedNuggetValue;
    algo.reset();
  }

  thetaValues = bestThetaValues;
  if (estimateTrend) {
    betaValues = bestBetaValues;
    /* set the betas in the polynomialRegression class */
    polyRegression->set_polynomial_coeffs(bestBetaValues);
  }
  if (estimateNugget) estimatedNuggetValue = bestEstimatedNuggetValue;

  /* compute and store best Cholesky factorization */
  compute_gram(cwiseDists2, true, false, GramMatrix);
  CholFact.compute(GramMatrix);
  hasBestCholFact = true;

  /* Useful info for debugging */
  /*
  std::cout << "\n";
  std::cout << objectiveFunctionHistory << "\n";
  std::cout << "optimal theta values in log-space:" << "\n";
  std::cout << bestThetaValues << "\n";
  std::cout << "best objective function value is " <<  bestObjFunValue << "\n";
  std::cout << "best objective function gradient norm is " <<
  final_obj_gradient.norm() << "\n";
  */
}

VectorXd GaussianProcess::value(const MatrixXd& eval_points, const int qoi) {
  /* Surrogate models don't yet support multiple responses */
  silence_unused_args(qoi);
  assert(qoi == 0);

  if (eval_points.cols() != numVariables) {
    throw(
        std::runtime_error("Gaussian Process value inputs are not consistent."
                           " Dimension of the feature space for the evaluation "
                           "point and Gaussian Process do not match"));
  }

  VectorXd approx_values;

  /* scale the eval_points (prediction points) */
  const MatrixXd& scaled_pred_points = dataScaler.scale_samples(eval_points);
  compute_pred_dists(scaled_pred_points);

  /* compute the Gram matrix and its Cholesky factorization */
  if (!hasBestCholFact) {
    compute_gram(cwiseDists2, true, false, GramMatrix);
    CholFact.compute(GramMatrix);
  }

  VectorXd resid, chol_solve_resid;
  compute_gram(cwiseMixedDists2, false, false, predMixedGramMatrix);

  if (estimateTrend) {
    resid = targetValues - basisMatrix * betaValues;
  } else
    resid = targetValues;

  chol_solve_resid = CholFact.solve(resid);
  approx_values = predMixedGramMatrix * chol_solve_resid;

  if (estimateTrend) {
    polyRegression->compute_basis_matrix(scaled_pred_points, predBasisMatrix);
    MatrixXd z = CholFact.solve(basisMatrix);
    approx_values += predBasisMatrix * betaValues;
  }
  return responseScaleFactor * approx_values.array() + responseOffset;
}

MatrixXd GaussianProcess::gradient(const MatrixXd& eval_points, const int qoi) {
  /* Surrogate models don't yet support multiple responses */
  silence_unused_args(qoi);
  assert(qoi == 0);

  if (eval_points.cols() != numVariables) {
    throw(std::runtime_error(
        "Gaussian Process gradient inputs are not consistent."
        " Dimension of the feature space for the evaluation points and "
        "Gaussian Process do not match"));
  }

  const int numPredictionPts = eval_points.rows();

  MatrixXd gradient(numPredictionPts, numVariables);

  /* scale the eval_points (prediction points) */
  MatrixXd scaled_pred_pts;
  dataScaler.scale_samples(eval_points, scaled_pred_pts);
  compute_pred_dists(scaled_pred_pts);

  /* compute the Gram matrix and its Cholesky factorization */
  if (!hasBestCholFact) {
    compute_gram(cwiseDists2, true, false, GramMatrix);
    CholFact.compute(GramMatrix);
  }

  MatrixXd chol_solve_resid, first_deriv_pred_gram, grad_components, resid;
  compute_gram(cwiseMixedDists2, false, false, predMixedGramMatrix);
  resid = targetValues;
  if (estimateTrend) resid -= basisMatrix * betaValues;
  chol_solve_resid = CholFact.solve(resid);

  for (int i = 0; i < numVariables; i++) {
    first_deriv_pred_gram = kernel->compute_first_deriv_pred_gram(
        predMixedGramMatrix, cwiseMixedDists, thetaValues, i);
    grad_components = first_deriv_pred_gram * chol_solve_resid;
    gradient.col(i) = grad_components.col(0);
  }

  /* extra terms for GP with a trend */
  if (estimateTrend) {
    MatrixXd poly_grad_pred_pts;
    gradient += polyRegression->gradient(scaled_pred_pts);
  }
  return responseScaleFactor * gradient;
}

MatrixXd GaussianProcess::hessian(const MatrixXd& eval_point, const int qoi) {
  /* Surrogate models don't yet support multiple responses */
  silence_unused_args(qoi);
  assert(qoi == 0);

  if (eval_point.rows() != 1) {
    throw(std::runtime_error(
        "Gaussian Process Hessian evaluation is for a single point."
        "The input contains more than one sample."));
  }

  MatrixXd hessian(numVariables, numVariables);

  /* scale the eval_point (prediction points) */
  MatrixXd scaled_pred_point;
  dataScaler.scale_samples(eval_point, scaled_pred_point);
  compute_pred_dists(scaled_pred_point);

  /* compute the Gram matrix and its Cholesky factorization */
  if (!hasBestCholFact) {
    compute_gram(cwiseDists2, true, false, GramMatrix);
    CholFact.compute(GramMatrix);
  }

  MatrixXd chol_solve_resid, second_deriv_pred_gram, resid;
  compute_gram(cwiseMixedDists2, false, false, predMixedGramMatrix);
  resid = targetValues;
  if (estimateTrend) resid -= basisMatrix * betaValues;
  chol_solve_resid = CholFact.solve(resid);

  /* Hessian */
  for (int i = 0; i < numVariables; i++) {
    for (int j = i; j < numVariables; j++) {
      second_deriv_pred_gram = kernel->compute_second_deriv_pred_gram(
          predMixedGramMatrix, cwiseMixedDists, thetaValues, i, j);
      hessian(i, j) = (second_deriv_pred_gram * chol_solve_resid)(0, 0);
      if (i != j) hessian(j, i) = hessian(i, j);
    }
  }

  if (estimateTrend) {
    MatrixXd poly_hessian_pred_pt;
    hessian += polyRegression->hessian(scaled_pred_point);
  }

  return responseScaleFactor * hessian;
}

MatrixXd GaussianProcess::covariance(const MatrixXd& eval_points,
                                     const int qoi) {
  /* Surrogate models don't yet support multiple responses */
  silence_unused_args(qoi);
  assert(qoi == 0);

  if (eval_points.cols() != numVariables) {
    throw(std::runtime_error(
        "Gaussian Process variance input has wrong dimension."
        " Dimension of the feature space for the evaluation point and Gaussian "
        "Process do not match"));
  }
  int num_eval_points = eval_points.rows();
  predCovariance.resize(num_eval_points, num_eval_points);
  /* scale the eval_points (prediction points) */
  const MatrixXd& scaled_pred_points = dataScaler.scale_samples(eval_points);
  compute_pred_dists(scaled_pred_points);

  /* compute the Gram matrix and its Cholesky factorization */
  if (!hasBestCholFact) {
    compute_gram(cwiseDists2, true, false, GramMatrix);
    CholFact.compute(GramMatrix);
  }

  VectorXd resid;
  MatrixXd chol_solve_pred_mat;
  compute_gram(cwiseMixedDists2, false, false, predMixedGramMatrix);

  if (estimateTrend)
    resid = targetValues - basisMatrix * betaValues;
  else
    resid = targetValues;

  chol_solve_pred_mat = CholFact.solve(predMixedGramMatrix.transpose());

  compute_gram(cwisePredDists2, true, false, predGramMatrix);
  predCovariance = predGramMatrix - predMixedGramMatrix * chol_solve_pred_mat;

  if (estimateTrend) {
    MatrixXd chol_solve_resid = CholFact.solve(resid);
    polyRegression->compute_basis_matrix(scaled_pred_points, predBasisMatrix);
    MatrixXd z = CholFact.solve(basisMatrix);
    MatrixXd R_mat = predBasisMatrix - predMixedGramMatrix * (z);
    MatrixXd h_mat = basisMatrix.transpose() * z;
    predCovariance += R_mat * (h_mat.ldlt().solve(R_mat.transpose()));
  }

  return pow(responseScaleFactor, 2) * predCovariance;
}

VectorXd GaussianProcess::variance(const MatrixXd& eval_points, const int qoi) {
  /* Surrogate models don't yet support multiple responses */
  silence_unused_args(qoi);
  assert(qoi == 0);

  VectorXd variance = covariance(eval_points).diagonal();

  for (int i = 0; i < variance.size(); i++) {
    if (variance(i) < 0.0 || std::isnan(variance(i))) {
      variance(i) = 0.0;
    }
  }

  return variance;
}

void GaussianProcess::negative_marginal_log_likelihood(bool compute_grad,
                                                       bool form_gram,
                                                       double& obj_value,
                                                       VectorXd& obj_gradient) {
  if (form_gram) {
    compute_gram(cwiseDists2, true, true, GramMatrix);
    CholFact.compute(GramMatrix);
    trendTargetResidual = targetValues;
    if (estimateTrend) trendTargetResidual -= basisMatrix * betaValues;
    GramResidualSolution = CholFact.solve(trendTargetResidual);
  }

  obj_value =
      0.5 * log(CholFact.vectorD().array()).matrix().sum() +
      0.5 * (trendTargetResidual.transpose() * GramResidualSolution)(0, 0) +
      static_cast<double>(numSamples) / 2.0 * log(2.0 * PI);

  if (compute_grad) {
    /* DTS: This Cholesky solve is much more expensive than the factorization!
     */
    MatrixXd Q =
        -0.5 * (GramResidualSolution * GramResidualSolution.transpose() -
                CholFact.solve(eyeMatrix));
    if (estimateTrend) {
      obj_gradient.segment(numVariables + 1, numPolyTerms) =
          -basisMatrix.transpose() * GramResidualSolution;
    }

    for (int k = 0; k < numVariables + 1; k++)
      obj_gradient(k) = (GramMatrixDerivs[k].cwiseProduct(Q)).sum();

    if (estimateNugget) {
      obj_gradient(numVariables + 1 + numPolyTerms) =
          2.0 * exp(2.0 * estimatedNuggetValue) * Q.trace();
    }
  }
}

void GaussianProcess::setup_hyperparameter_bounds(VectorXd& sigma_bounds,
                                                  MatrixXd& length_scale_bounds,
                                                  VectorXd& nugget_bounds) {
  sigma_bounds(0) =
      configOptions.sublist("Sigma Bounds").get<double>("lower bound");
  sigma_bounds(1) =
      configOptions.sublist("Sigma Bounds").get<double>("upper bound");

  if (length_scale_bounds.rows() == numVariables &&
      length_scale_bounds.cols() == 2)
    length_scale_bounds = configOptions.get<MatrixXd>("length-scale bounds");
  else if (length_scale_bounds.size() == 0) {
    length_scale_bounds.resize(1, 2);
    length_scale_bounds(0, 0) =
        configOptions.sublist("Length-scale Bounds").get<double>("lower bound");
    length_scale_bounds(0, 1) =
        configOptions.sublist("Length-scale Bounds").get<double>("upper bound");
  } else
    throw(std::runtime_error("Length-scale bounds incorrectly specified."));

  estimateNugget = configOptions.sublist("Nugget").get<bool>("estimate nugget");
  if (estimateNugget) {
    numNuggetTerms = 1;
    nugget_bounds(0) =
        configOptions.sublist("Nugget").sublist("Bounds").get<double>(
            "lower bound");
    nugget_bounds(1) =
        configOptions.sublist("Nugget").sublist("Bounds").get<double>(
            "upper bound");
  }
}

int GaussianProcess::get_num_opt_variables() {
  return numVariables + 1 + numPolyTerms + numNuggetTerms;
}

int GaussianProcess::get_num_variables() const { return numVariables; }

void GaussianProcess::set_opt_params(const std::vector<double>& opt_params) {
  for (int i = 0; i < numVariables + 1; i++) thetaValues(i) = opt_params[i];

  if (estimateTrend) {
    for (int i = 0; i < numPolyTerms; i++)
      betaValues(i) = opt_params[numVariables + 1 + i];
  }

  if (estimateNugget)
    estimatedNuggetValue = opt_params[numVariables + 1 + numPolyTerms];
}

void GaussianProcess::default_options() {
  // Scalar values for bound used by default. Advanced users can specify
  // ansiotropic legnth-scale bounds with an Eigen matrix in C++ or
  // Pybind11 with Numpy.
  double sigma_lower_bound, sigma_upper_bound, length_scale_lower_bounds,
      length_scale_upper_bounds, nugget_lower_bound, nugget_upper_bound;

  // Note: bound constraints will be converted to log-scale

  // sigma bounds - lower and upper
  sigma_lower_bound = 1.0e-2;
  sigma_upper_bound = 1.0e2;
  // length scale bounds - can be anisotropic (num_vars x 2 dimensions)
  // if given as a matrix to the ParameterList.
  // Otherwise these are bounds for all length-scales (i.e. isotropic)
  length_scale_lower_bounds = 1.0e-2;
  length_scale_upper_bounds = 1.0e2;

  // results in a nugget**2 betwen 1.0e-10 and 1.0e-4
  nugget_lower_bound = 1.0e-5;
  nugget_upper_bound = 1.0e-2;

  // Eigen containers are only used when they are non-empty
  // (i.e. user-supplied).
  VectorXd sigma_bounds, nugget_bounds;
  MatrixXd length_scale_bounds;

  defaultConfigOptions.set("kernel type", "squared exponential",
                           "kernel function specification");
  defaultConfigOptions.sublist("Sigma Bounds")
      .set("lower bound", sigma_lower_bound, "sigma term lower bound");
  defaultConfigOptions.sublist("Sigma Bounds")
      .set("upper bound", sigma_upper_bound, "sigma term lower bound");
  defaultConfigOptions.sublist("Length-scale Bounds")
      .set("lower bound", length_scale_lower_bounds,
           "length-scale lower bounds");
  defaultConfigOptions.sublist("Length-scale Bounds")
      .set("upper bound", length_scale_upper_bounds,
           "length-scale upper bounds");
  defaultConfigOptions.set("anisotropic length-scale bounds",
                           length_scale_bounds,
                           "Eigen Matrix of length-scale bounds: num_vars x "
                           "[lb, ub]. Will overide isotropic bounds settings.");
  defaultConfigOptions.set("scaler name", "standardization",
                           "scaler for variables");
  defaultConfigOptions.set("num restarts", 10,
                           "local optimizer number of initial iterates");
  defaultConfigOptions.set("gp seed", 42,
                           "random seed for initial iterate generation");
  defaultConfigOptions.set("standardize response", true,
                           "Make the response zero mean and unit variance");
  /* Verbosity levels
     2 - maximum level: print out config options and building notification
     1 - minimum level: print out building notification
     0 - no output */
  defaultConfigOptions.set("verbosity", 1, "console output verbosity");
  /* Nugget */
  defaultConfigOptions.sublist("Nugget").set("fixed nugget", 1.0e-10,
                                             "fixed nugget term");
  defaultConfigOptions.sublist("Nugget").set("estimate nugget", false,
                                             "estimate a nugget term");
  defaultConfigOptions.sublist("Nugget").sublist("Bounds").set(
      "lower bound", nugget_lower_bound, "nugget term lower bound");
  defaultConfigOptions.sublist("Nugget").sublist("Bounds").set(
      "upper bound", nugget_upper_bound, "nugget term upper bound");
  /* Polynomial Trend */
  defaultConfigOptions.sublist("Trend").set("estimate trend", false,
                                            "estimate a trend term");
  defaultConfigOptions.sublist("Trend").sublist("Options").set(
      "max degree", 2, "Maximum polynomial order");
  defaultConfigOptions.sublist("Trend").sublist("Options").set(
      "reduced basis", false, "Use Reduced Basis");
  defaultConfigOptions.sublist("Trend").sublist("Options").set(
      "p-norm", 1.0, "P-Norm in hyperbolic cross");
  defaultConfigOptions.sublist("Trend").sublist("Options").set(
      "scaler type", "none", "Type of data scaling");
  defaultConfigOptions.sublist("Trend").sublist("Options").set(
      "regression solver type", "SVD", "Type of regression solver");
  defaultConfigOptions.sublist("Trend").sublist("Options").set(
      "verbosity", 1, "console output verbosity");
}

void GaussianProcess::compute_build_dists() {
  cwiseDists2.resize(numVariables);

  for (int k = 0; k < numVariables; k++) {
    cwiseDists2[k].resize(numSamples, numSamples);
    for (int i = 0; i < numSamples; i++) {
      for (int j = i; j < numSamples; j++) {
        cwiseDists2[k](i, j) =
            pow(scaledBuildPoints(i, k) - scaledBuildPoints(j, k), 2);
        if (i != j) cwiseDists2[k](j, i) = cwiseDists2[k](i, j);
      }
    }
  }
}

void GaussianProcess::compute_pred_dists(const MatrixXd& scaled_pred_pts) {
  const int num_pred_pts = scaled_pred_pts.rows();
  cwiseMixedDists.resize(numVariables);
  cwiseMixedDists2.resize(numVariables);
  cwisePredDists2.resize(numVariables);

  for (int k = 0; k < numVariables; k++) {
    cwiseMixedDists[k].resize(num_pred_pts, numSamples);
    cwisePredDists2[k].resize(num_pred_pts, num_pred_pts);
    for (int i = 0; i < num_pred_pts; i++) {
      for (int j = 0; j < numSamples; j++) {
        cwiseMixedDists[k](i, j) =
            scaled_pred_pts(i, k) - scaledBuildPoints(j, k);
      }
      for (int j = i; j < num_pred_pts; j++) {
        cwisePredDists2[k](i, j) =
            pow(scaled_pred_pts(i, k) - scaled_pred_pts(j, k), 2);
        if (i != j) cwisePredDists2[k](j, i) = cwisePredDists2[k](i, j);
      }
    }
    cwiseMixedDists2[k] = cwiseMixedDists[k].array().square();
  }
}

void GaussianProcess::compute_gram(const std::vector<MatrixXd>& dists2,
                                   bool add_nugget, bool compute_derivs,
                                   MatrixXd& gram) {
  const int num_rows = dists2[0].rows();
  const int num_cols = dists2[0].cols();
  gram.resize(num_rows, num_cols);
  kernel->compute_gram(dists2, thetaValues, gram);

  if (compute_derivs)
    kernel->compute_gram_derivs(gram, dists2, thetaValues, GramMatrixDerivs);

  if (add_nugget) {
    /* add in the fixed nugget */
    gram.diagonal().array() += fixedNuggetValue;
    /* add in the estimated nugget */
    if (estimateNugget)
      gram.diagonal().array() += exp(2.0 * estimatedNuggetValue);
  }
}

void GaussianProcess::generate_initial_guesses(
    const VectorXd& sigma_bounds, const MatrixXd& length_scale_bounds,
    const VectorXd& nugget_bounds, const int num_restarts, const int seed,
    MatrixXd& initial_guesses) {
  initial_guesses = util::create_uniform_random_double_matrix(
      num_restarts, numVariables + 1 + numPolyTerms + numNuggetTerms, seed,
      true, -1.0, 1.0);

  double mean, span;
  for (int j = 0; j < numVariables + 1; j++) {
    if (j == 0) {
      span = 0.5 * (log(sigma_bounds(1)) - log(sigma_bounds(0)));
      mean = 0.5 * (log(sigma_bounds(1)) + log(sigma_bounds(0)));
    } else {
      if (length_scale_bounds.rows() > 1) {
        span = 0.5 * (log(length_scale_bounds(j - 1, 1)) -
                      log(length_scale_bounds(j - 1, 0)));
        mean = 0.5 * (log(length_scale_bounds(j - 1, 1)) +
                      log(length_scale_bounds(j - 1, 0)));
      } else {
        span = 0.5 * (log(length_scale_bounds(0, 1)) -
                      log(length_scale_bounds(0, 0)));
        mean = 0.5 * (log(length_scale_bounds(0, 1)) +
                      log(length_scale_bounds(0, 0)));
      }
    }
    for (int i = 0; i < num_restarts; i++) {
      initial_guesses(i, j) = span * initial_guesses(i, j) + mean;
    }
  }
  /* Initial guess for trend terms set to zero */
  if (estimateTrend) {
    int index_offset = numVariables + 1;
    for (int i = 0; i < num_restarts; ++i) {
      for (int j = 0; j < numPolyTerms; j++) {
        initial_guesses(i, index_offset + j) = 0.0;
      }
    }
  }
  if (estimateNugget) {
    int index_offset = numVariables + 1 + numPolyTerms;
    span = 0.5 * (log(nugget_bounds(1)) - log(nugget_bounds(0)));
    mean = 0.5 * (log(nugget_bounds(1)) + log(nugget_bounds(0)));
    for (int i = 0; i < num_restarts; ++i) {
      initial_guesses(i, index_offset) =
          span * initial_guesses(i, index_offset) + mean;
    }
  }
}

void GaussianProcess::setup_default_optimization_params(
    Teuchos::RCP<ParameterList> rol_params) {
  /* Secant */
  rol_params->sublist("General").sublist("Secant").set("Type",
                                                       "Limited-Memory BFGS");
  rol_params->sublist("General").sublist("Secant").set("Maximum Storage", 20);
  /* Step */
  rol_params->sublist("General")
      .sublist("Step")
      .sublist("Line Search")
      .set("Function Evaluation Limit", 3);

  rol_params->sublist("General")
      .sublist("Step")
      .sublist("Line Search")
      .set("Sufficient Decrease Tolerance", 1.0e-4);

  rol_params->sublist("General")
      .sublist("Step")
      .sublist("Line Search")
      .set("Initial Step Size", 1.0);

  rol_params->sublist("General")
      .sublist("Step")
      .sublist("Line Search")
      .sublist("Descent Method")
      .set("Type", "Quasi-Newton");

  rol_params->sublist("General")
      .sublist("Step")
      .sublist("Line Search")
      .sublist("Descent Method")
      .set("Nonlinear CG Type", "Hestenes-Stiefel");

  rol_params->sublist("General")
      .sublist("Step")
      .sublist("Line Search")
      .sublist("Curvature Condition")
      .set("Type", "Strong Wolfe Conditions");

  rol_params->sublist("General")
      .sublist("Step")
      .sublist("Line Search")
      .sublist("Curvature Condition")
      .set("General Parameter", 0.9);

  rol_params->sublist("General")
      .sublist("Step")
      .sublist("Line Search")
      .sublist("Curvature Condition")
      .set("Generalized Wolfe Parameter", 0.6);

  rol_params->sublist("General")
      .sublist("Step")
      .sublist("Line Search")
      .sublist("Line-Search Method")
      .set("Type", "Cubic Interpolation");

  rol_params->sublist("General")
      .sublist("Step")
      .sublist("Line Search")
      .sublist("Line-Search Method")
      .set("Backtracking Rate", 0.5);

  rol_params->sublist("General")
      .sublist("Step")
      .sublist("Line Search")
      .sublist("Line-Search Method")
      .set("Bracketing Tolerance", 1.0e-8);

  /* Status Test */
  rol_params->sublist("Status Test").set("Gradient Tolerance", 1.0e-4);

  rol_params->sublist("Status Test").set("Step Tolerance", 1.0e-8);

  rol_params->sublist("Status Test").set("Iteration Limit", 200);
}

}  // namespace surrogates
}  // namespace dakota

BOOST_CLASS_EXPORT_IMPLEMENT(dakota::surrogates::GaussianProcess)
