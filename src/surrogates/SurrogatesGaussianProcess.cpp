/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "SurrogatesGaussianProcess.hpp"

#include "ROL_Solver.hpp"
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
#ifdef DISABLE_YAML_SURROGATES_CONFIG
  throw std::runtime_error(
      "Configuring a surrogate using a YAML file not supported by this build "
      "of Dakota");
#endif
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
#ifdef DISABLE_YAML_SURROGATES_CONFIG
  throw std::runtime_error(
      "Configuring a surrogate using a YAML file not supported by this build "
      "of Dakota");
#endif
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

  //  auto obj = std::make_shared<GP_Objective>(*this);
  auto obj_ptr = ROL::makePtr<GP_Objective>(*this);
  auto& obj = *obj_ptr;
  int dim = numVariables + 1 + numPolyTerms + numNuggetTerms;

  auto make_ROLVectorXd = [](auto&&... args) {
    return ROL::makePtr<ROLVectorXd>(std::forward<decltype(args)>(args)...);
  };

  /* set up parameter vectors and bounds */
  auto x_ptr = make_ROLVectorXd(dim, true);
  auto& x = *x_ptr;
  auto lo_ptr = make_ROLVectorXd(dim, true);
  auto& lo = *lo_ptr;
  auto hi_ptr = make_ROLVectorXd(dim, true);
  auto& hi = *hi_ptr;

  auto bounds_ptr = ROL::makePtr<ROL::Bounds<double>>(lo_ptr, hi_ptr);
  auto& bounds = *bounds_ptr;

  constexpr int LO{0}, HI{1};

  for (int bnd : {LO, HI}) {
    auto& vec = bnd == LO ? lo : hi;
    vec(0) = log(sigma_bounds(bnd));
    if (length_scale_bounds.rows() > 1) {
      vec.segment(1, numVariables) = length_scale_bounds.col(bnd).array().log();
    } else {
      vec.segment(1, numVariables)
          .setConstant(log(length_scale_bounds(0, bnd)));
    }

    if (estimateTrend) {
      vec.segment(numVariables + 1, numPolyTerms) = beta_bounds.col(bnd);
    }
    if (estimateNugget) {
      vec(dim - 1) = log(nugget_bounds(bnd));
    }
  }

  //  std::vector<std::string> output;

  objectiveFunctionHistory.resize(num_restarts);
  objectiveGradientHistory.resize(num_restarts, dim);
  thetaHistory.resize(num_restarts, dim);

  double final_obj_value;
  VectorXd final_obj_gradient(dim);

  auto prob_ptr = ROL::makePtr<ROL::Problem<double>>(obj_ptr, x_ptr);
  prob_ptr->addBoundConstraint(bounds_ptr);

  ROL::Solver<double> rol_solver(prob_ptr, *gp_mle_rol_params);

  for (int i = 0; i < num_restarts; i++) {
    x = initial_guesses.row(i);
    rol_solver.solve(*outStream, ROL::nullPtr, true);
    thetaValues = x.head(numVariables + 1);
    if (estimateTrend) {
      for (int j = 0; j < numPolyTerms; ++j) {
        betaValues(j) = x(numVariables + 1 + j);
      }
    }
    if (estimateNugget) {
      estimatedNuggetValue = x(numVariables + 1 + numPolyTerms);
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
    rol_solver.reset();
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

VectorXd GaussianProcess::value(const MatrixXd& eval_points) {
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

MatrixXd GaussianProcess::gradient(const MatrixXd& eval_points) {
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

MatrixXd GaussianProcess::hessian(const MatrixXd& eval_point) {
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
  auto& sb_list = configOptions.sublist("Sigma Bounds");
  sigma_bounds(0) = sb_list.get<double>("lower bound");
  sigma_bounds(1) = sb_list.get<double>("upper bound");

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

void GaussianProcess::set_opt_params(const VectorXd& opt_params) {
  // Copy the first numVariables + 1 elements to thetaValues
  thetaValues = opt_params.head(numVariables + 1);

  if (estimateTrend) {
    // Copy the next numPolyTerms elements to betaValues
    betaValues = opt_params.segment(numVariables + 1, numPolyTerms);
  }

  if (estimateNugget) {
    estimatedNuggetValue = opt_params(numVariables + 1 + numPolyTerms);
  }
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
  auto& nlist = defaultConfigOptions.sublist("Nugget");
  nlist.set("fixed nugget", 1.0e-10, "fixed nugget term");
  nlist.set("estimate nugget", false, "estimate a nugget term");
  nlist.sublist("Bounds").set("lower bound", nugget_lower_bound,
                              "nugget term lower bound");
  nlist.sublist("Bounds").set("upper bound", nugget_upper_bound,
                              "nugget term upper bound");

  /* Polynomial Trend */
  auto& trend_list = defaultConfigOptions.sublist("Trend");
  trend_list.set("estimate trend", false, "estimate a trend term");

  auto& opt_list = trend_list.sublist("Options");
  opt_list.set("max degree", 2, "Maximum polynomial order");
  opt_list.set("reduced basis", false, "Use Reduced Basis");
  opt_list.set("p-norm", 1.0, "P-Norm in hyperbolic cross");
  opt_list.set("scaler type", "none", "Type of data scaling");
  opt_list.set("regression solver type", "SVD", "Type of regression solver");
  opt_list.set("verbosity", 1, "console output verbosity");
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
  auto& sec_list = rol_params->sublist("General").sublist("Secant");
  sec_list.set("Type", "Limited-Memory BFGS");
  sec_list.set("Maximum Storage", 20);

  auto& step_list = rol_params->sublist("General").sublist("Step");
  step_list.set("Type", "Line Search");

  auto& ls_list = step_list.sublist("Line Search");
  ls_list.set("Function Evaluation Limit", 3);
  ls_list.set("Sufficient Decrease Tolerance", 1.0e-4);
  ls_list.set("Initial Step Size", 1.0);

  auto& dm_list = ls_list.sublist("Descent Method");
  dm_list.set("Type", "Quasi-Newton");
  dm_list.set("Nonlinear CG Type", "Hestenes-Stiefel");

  auto& cc_list = ls_list.sublist("Curvature Condition");
  cc_list.set("Type", "Strong Wolfe Conditions");
  cc_list.set("General Parameter", 0.9);
  cc_list.set("Generalized Wolfe Parameter", 0.6);

  auto& lsm_list = ls_list.sublist("Line-Search Method");
  lsm_list.set("Type", "Cubic Interpolation");
  lsm_list.set("Backtracking Rate", 0.5);
  lsm_list.set("Bracketing Tolerance", 1.0e-8);

  auto& st_list = rol_params->sublist("Status Test");
  st_list.set("Gradient Tolerance", 1.0e-4);
  st_list.set("Step Tolerance", 1.0e-8);
  st_list.set("Iteration Limit", 200);
}

}  // namespace surrogates
}  // namespace dakota

BOOST_CLASS_EXPORT_IMPLEMENT(dakota::surrogates::GaussianProcess)
