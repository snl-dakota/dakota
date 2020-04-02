/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "GaussianProcess.hpp"
#include "GP_Objective.hpp"

#include "ROL_Algorithm.hpp"
#include "ROL_Bounds.hpp"

#include "Teuchos_oblackholestream.hpp"
#include "Teuchos_XMLParameterListHelpers.hpp"


namespace dakota {

using util::NormalizationScaler;
using util::NoScaler;
using util::StandardizationScaler;

namespace surrogates {

// Constuctor with default parameters.
GaussianProcess::GaussianProcess(){
  default_options();
}

// Constructor that sets user-defined params but does not build.
GaussianProcess::GaussianProcess(const ParameterList& param_list) {
  default_options();
  configOptions = param_list;
}

// BMA NOTE: ParameterList::get() can throw, so direct delegation
// probably not good; might want to give a helpful message
// Constructor that will set user-defined parameters and build with
// given data.
GaussianProcess::GaussianProcess(const MatrixXd &samples,
				 const MatrixXd &response,
				 const ParameterList& param_list)
{
  default_options();
  configOptions = param_list;
  build(samples, response);
  //configOptions.validateParametersAndSetDefaults(defaultConfigOptions);

  // check that the passed parameters are valid for this surrogate
  // (doesn't allow unused parameters, which we might want)
  // (also overwrites the defaults, which we might want to preserve)
  /*
  try {
    configOptions.validateParametersAndSetDefaults(defaultConfigOptions);
  }
  catch (const Teuchos::Exceptions::InvalidParameter& e) {
    std::cerr << "Invalid parameter passed to GaussianProcess; details:\n"
	      << e.what() << std::endl;
    throw;
  }
  configOptions.setParameters(param_list);
  */
  //std::cout << "Building GaussianProcess with final parameters\n"
	//    << configOptions << std::endl;

  /*
  build(samples,
	response,
	configOptions.get<VectorXd>("sigma_bounds"),
	configOptions.get<MatrixXd>("length_scale_bounds"),
	configOptions.get<std::string>("scaler_name"),
	configOptions.get<int>("num_restarts"),
	configOptions.get<double>("nugget"),
	configOptions.get<int>("gp_seed")
	);
  */
}


/*
GaussianProcess::GaussianProcess(const MatrixXd &samples,
                                 const MatrixXd &response,
                                 const VectorXd &sigma_bounds,
                                 const MatrixXd &length_scale_bounds,
                                 const std::string scaler_name,
                                 const int num_restarts,
                                 const double nugget_val,
                                 const int seed)
{
  build(samples,
	response,
	sigma_bounds,
	length_scale_bounds,
	scaler_name,
	num_restarts,
	nugget_val,
	seed);
}
*/

GaussianProcess::~GaussianProcess(){}

const VectorXd & GaussianProcess::get_posterior_std_dev() const { return posteriorStdDev; }

const MatrixXd & GaussianProcess::get_posterior_covariance() const { return posteriorCov; }

const VectorXd & GaussianProcess::get_theta_values() const { return thetaValues; }

int GaussianProcess::get_num_opt_variables() {
  return numVariables + 1 + numPolyTerms + numNuggetTerms;
}

int GaussianProcess::get_num_variables() const { return numVariables; }

// TODO: Update the name, more parameters than theta now
void GaussianProcess::set_theta(const std::vector<double> theta_new) {
  /*
  for (int i = 0; i < thetaValues->size(); i++)
    (thetaValues)(i) = theta_new[i];
   */
  for (int i = 0; i < numVariables + 1; i++) 
    thetaValues(i) = theta_new[i];

  if (estimateTrend) {
    for (int i = 0; i < numPolyTerms; i++)
      betaValues(i) = theta_new[numVariables+1+i];
  }

  if (estimateNugget) {
    estimatedNuggetValue = theta_new[numVariables+1+numPolyTerms];
  }
}


void GaussianProcess::generate_initial_guesses(MatrixXd &initial_guesses, int num_restarts,
                                               const VectorXd &sigma_bounds,
                                               const MatrixXd &length_scale_bounds,
                                               const VectorXd &nugget_bounds) {
  initial_guesses = MatrixXd::Random(num_restarts,numVariables + 1
                    + numPolyTerms + numNuggetTerms);
  double mean, span;
  for (int j = 0; j < numVariables+1; j++) {
    if (j == 0) {
      span = 0.5*(log(sigma_bounds(1)) - log(sigma_bounds(0)));
      mean = 0.5*(log(sigma_bounds(1)) + log(sigma_bounds(0)));
    }
    else {
      span = 0.5*(log(length_scale_bounds(j-1,1)) - log(length_scale_bounds(j-1,0)));
      mean = 0.5*(log(length_scale_bounds(j-1,1)) + log(length_scale_bounds(j-1,0)));
    }
    for (int i = 0; i < num_restarts; i++) {
      initial_guesses(i,j) = span*initial_guesses(i,j) + mean;
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
    span = 0.5*(log(nugget_bounds(1)) - log(nugget_bounds(0)));
    mean = 0.5*(log(nugget_bounds(1)) + log(nugget_bounds(0)));
    for (int i = 0; i < num_restarts; ++i) {
      initial_guesses(i,index_offset) = span*initial_guesses(i,index_offset)
                                        + mean;
    }
  }
}


void GaussianProcess::compute_prediction_matrix(const MatrixXd &scaled_pred_pts, MatrixXd &pred_mat) {
  const int numPredictionPts = scaled_pred_pts.rows();
  pred_mat.resize(numPredictionPts,numSamples);
  MatrixXd scaled_samples = dataScaler->get_scaled_features();
  for (int i = 0; i < numPredictionPts; i++) {
    for (int j = 0; j < numSamples; j++) {
      pred_mat(i,j) = sq_exp_cov_pred(scaled_pred_pts.row(i),scaled_samples.row(j));
    }
  }
}

void GaussianProcess::compute_first_deriv_pred_mat(const MatrixXd &pred_mat, const MatrixXd & scaled_pred_pts,
                                                   const int index, MatrixXd &first_deriv_pred_mat) {
  const int numPredictionPts = scaled_pred_pts.rows();
  first_deriv_pred_mat.resize(numPredictionPts,numSamples);
  first_deriv_pred_mat.setZero();

  MatrixXd scaled_samples = dataScaler->get_scaled_features();

  for (int i = 0; i < numPredictionPts; i++) {
    for (int j = 0; j < numSamples; j++) {
      first_deriv_pred_mat(i,j) = -pred_mat(i,j)
                                   *(scaled_pred_pts(i,index) - scaled_samples(j,index))
                                   *exp(-2.0*((thetaValues)(index+1)));
    }
  }
}

void GaussianProcess::compute_second_deriv_pred_mat(const MatrixXd &pred_mat, const MatrixXd & scaled_pred_pts,
                                                    const int index_i, const int index_j,
                                                    MatrixXd &second_deriv_pred_mat) {
  const int numPredictionPts = 1; /* Hessian evaluations are at a single point */
  second_deriv_pred_mat.resize(numPredictionPts,numSamples);
  second_deriv_pred_mat.setZero();

  MatrixXd scaled_samples = dataScaler->get_scaled_features();

  double diagonal_factor = 0.0;

  if (index_i == index_j)
    diagonal_factor = 1.0;

  for (int i = 0; i < numPredictionPts; i++) {
    for (int j = 0; j < numSamples; j++) {
      second_deriv_pred_mat(i,j) = pred_mat(i,j)
                                   *((scaled_pred_pts(i,index_i) - scaled_samples(j,index_i))
                                   *(scaled_pred_pts(i,index_j) - scaled_samples(j,index_j))
                                   *exp(-2.0*((thetaValues)(index_i+1) + (thetaValues)(index_j+1)))
                                   - diagonal_factor*exp(-2.0*((thetaValues)(index_i+1))));
    }
  }
}

double GaussianProcess::sq_exp_cov(const int i, const int j) {
  double inside = 0.0;
  for (int k = 0; k < numVariables; k++)
    inside += componentwiseDistances[k](i,j)*std::exp(-2.0*(thetaValues)(k+1));

  return std::exp(2.0*(thetaValues)(0))*exp(-0.5*inside);
}

double GaussianProcess::sq_exp_cov_pred(const VectorXd &x, const VectorXd &y) {
  VectorXd diff = x - y;
  double inside = 0.0;
  for (int k = 0; k < numVariables; k++)
    inside += std::pow(diff(k),2.0)*std::exp(-2.0*(thetaValues)(k+1));

  return std::exp(2.0*(thetaValues)(0))*std::exp(-0.5*inside);
}
                

void GaussianProcess::compute_gram(bool compute_derivs){
  for (int i = 0; i < numSamples; i++) {
    for (int j = i; j < numSamples; j++) {
      GramMatrix(i,j) = sq_exp_cov(i,j);
      if (i != j)
        GramMatrix(j,i) = GramMatrix(i,j);
    }
  }

  if (compute_derivs) {
    GramMatrixDerivs[0] = 2.0*GramMatrix;
    for (int k = 1 ; k < numVariables + 1; k++) {
      GramMatrixDerivs[k] = GramMatrix.cwiseProduct(componentwiseDistances[k-1])
                            *std::exp(-2.0*(thetaValues)(k));
    }
  }

  /* add in the fixed nugget */
  for (int i = 0; i < numSamples; i++)
    GramMatrix(i,i) += fixedNuggetValue;

  /* add in the estimated nugget */
  if (estimateNugget) {
    for (int i = 0; i < numSamples; i++) {
      GramMatrix(i,i) += exp(2.0*estimatedNuggetValue);
    }
  }
}

void GaussianProcess::negative_marginal_log_likelihood(double &obj_value, VectorXd &obj_gradient) {
  double logSum;
  VectorXd z;
  MatrixXd Q, eye_matrix, resid;
  eye_matrix = MatrixXd::Identity(numSamples,numSamples);

  compute_gram(true);
  CholFact.compute(GramMatrix);
  VectorXd D(CholFact.vectorD());

  if (estimateTrend) {
    resid = targetValues - basisMatrix*betaValues;
    z = CholFact.solve(resid);
    obj_gradient.segment(numVariables+1, numPolyTerms) = 
                         -basisMatrix.transpose()*z;
  }
  else {
    resid = targetValues;
    z = CholFact.solve(resid);
  }
  logSum = 0.5*log(D.array()).matrix().sum();
  Q = -0.5*(z*z.transpose() - CholFact.solve(eye_matrix));

  obj_value = logSum + 0.5*(resid.transpose()*z)(0,0) + 
              static_cast<double>(numSamples)/2.0*log(2.0*PI);

  MatrixXd product(numSamples,numSamples);
  for (int k = 0; k < numVariables+1; k++)
    obj_gradient(k) = (GramMatrixDerivs[k].cwiseProduct(Q)).sum();

  if (estimateNugget) {
    obj_gradient(numVariables+1+numPolyTerms) = 2.0*exp(2.0*estimatedNuggetValue)*
                                                Q.trace();
  }
}

void GaussianProcess::compute_gram_pred(const MatrixXd &samples, MatrixXd &Gram_pred) {
  const int numPredictionPts = samples.rows();
  Gram_pred.resize(numPredictionPts,numPredictionPts);
  for (int i = 0; i < numPredictionPts; i++) {
    for (int j = i; j < numPredictionPts; j++) {
      Gram_pred(i,j) = sq_exp_cov_pred(samples.row(i),samples.row(j));
      if (i != j)
        Gram_pred(j,i) = Gram_pred(i,j);
    }
  }
  /* add in the fixed nugget */
  for (int i = 0; i < numPredictionPts; i++)
    Gram_pred(i,i) += fixedNuggetValue;

  /* add in the estimated nugget */
  if (estimateNugget) {
    for (int i = 0; i < numPredictionPts; i++) {
      Gram_pred(i,i) += exp(2.0*estimatedNuggetValue);
    }
  }
}


void GaussianProcess::default_options()
{
  // BMA TODO: with both approaches, how to publish a list of all valid options?

  // bound constraints -- will be converted to log-scale
  // sigma bounds - lower and upper
  VectorXd sigma_bounds(2);
  sigma_bounds(0) = 1.0e-2;
  sigma_bounds(1) = 1.0e2;
  // length scale bounds - num_vars x 2
  MatrixXd length_scale_bounds(1,2);
  length_scale_bounds(0,0) = 1.0e-2;
  length_scale_bounds(0,1) = 1.0e2;

  /* results in a nugget**2 betwen 1.0e-15 and 1.0e-8 */
  VectorXd nugget_bounds(2);
  nugget_bounds(0) = 3.17e-8;
  nugget_bounds(1) = 1.0e-2;

  defaultConfigOptions.set("sigma bounds", sigma_bounds, "sigma [lb, ub]");
  // BMA: Do we want to allow 1 x 2 always as a fallback?
  defaultConfigOptions.set("length-scale bounds", length_scale_bounds, "length scale num_vars x [lb, ub]");
  defaultConfigOptions.set("scaler name", "mean normalization", "scaler for variables");
  defaultConfigOptions.set("num restarts", 5, "local optimizer number of initial iterates");
  defaultConfigOptions.set("gp seed", 129, "random seed for initial iterate generation");
  /* Nugget */
  defaultConfigOptions.sublist("Nugget")
                      .set("fixed nugget", 0.0, "fixed nugget term");
  defaultConfigOptions.sublist("Nugget")
                      .set("estimate nugget", false, "estimate a nugget term");
  defaultConfigOptions.sublist("Nugget")
                      .set("nugget bounds", nugget_bounds, "nugget term [lb, ub]");
  /* Polynomial Trend */
  defaultConfigOptions.sublist("Trend").set("estimate trend", false, "estimate a trend term");
  defaultConfigOptions.sublist("Trend").sublist("Options")
                      .set("max degree", 2, "Maximum polynomial order");
  defaultConfigOptions.sublist("Trend").sublist("Options")
                      .set("p-norm", 1.0, "P-Norm in hyperbolic cross");
  defaultConfigOptions.sublist("Trend").sublist("Options")
                      .set("scaler type", "none", "Type of data scaling");
  defaultConfigOptions.sublist("Trend").sublist("Options")
                      .set("regression solver type", "SVD", "Type of regression solver");
}

void GaussianProcess::build(const MatrixXd &samples, const MatrixXd &response)
{
  configOptions.validateParametersAndSetDefaults(defaultConfigOptions);
  std::cout << "Building GaussianProcess with configuration options\n"
	  << configOptions << std::endl;

  numQOI = response.cols();
  numSamples = samples.rows();
  numVariables = samples.cols();
  targetValues = response;

  /* Optimization-related data*/
  VectorXd sigma_bounds = configOptions.get<VectorXd>("sigma bounds");
  MatrixXd length_scale_bounds = configOptions.get<MatrixXd>("length-scale bounds");

  /* Optional data*/
  estimateTrend = configOptions.sublist("Trend").get<bool>("estimate trend");
  estimateNugget = configOptions.sublist("Nugget").get<bool>("estimate nugget");
  MatrixXd beta_bounds;
  VectorXd nugget_bounds;


  const int num_restarts = configOptions.get<int>("num restarts");
  
  /* Scale the data */
  dataScaler = util::scaler_factory(
    util::DataScaler::scaler_type(configOptions.get<std::string>("scaler name")),
    samples);
  MatrixXd scaled_samples = dataScaler->get_scaled_features();

  if (estimateTrend) {
    polyRegression = std::make_shared<PolynomialRegression>
                     (scaled_samples, targetValues,
                      configOptions.sublist("Trend").sublist("Options"));
    numPolyTerms = polyRegression->get_num_terms();
    polyRegression->compute_basis_matrix(scaled_samples, basisMatrix);
    beta_bounds = MatrixXd::Ones(numPolyTerms,2);
    beta_bounds.col(0) *= -betaBound;
    beta_bounds.col(1) *= betaBound;
  }

  /* size of thetaValues for squared exponential kernel and one QoI */
  thetaValues.resize(numVariables+1);
  bestThetaValues.resize(numVariables+1);
  betaValues.resize(numPolyTerms);
  bestBetaValues.resize(numPolyTerms);
  /* set the size of the GramMatrix and its derivatives */
  GramMatrix.resize(numSamples, numSamples);
  GramMatrixDerivs.resize(numVariables+1);
  for (int k = 0; k < numVariables+1; k++) {
    GramMatrixDerivs[k].resize(numSamples, numSamples);
  }

  fixedNuggetValue = configOptions.sublist("Nugget").get<double>("fixed nugget");
  /* DTS: if the nugget is being estimated, should the fixed value be set to zero? */
  if (estimateNugget) {
    nugget_bounds = configOptions.sublist("Nugget").get<VectorXd>("nugget bounds");
    numNuggetTerms = 1;
  }

  /* compute Euclidean distances squared */
  componentwiseDistances.resize(numVariables);
  for (int k = 0; k < numVariables; k++) {
    componentwiseDistances[k].resize(numSamples,numSamples);
    for (int i = 0; i < numSamples; i++) {
      for (int j = i; j < numSamples; j++) {
        componentwiseDistances[k](i,j) = std::pow(scaled_samples(i,k) - scaled_samples(j,k), 2.0);
        if (i != j)
          componentwiseDistances[k](j,i) = componentwiseDistances[k](i,j);
      }
    }
  }



  /* set up the initial guesses */
  srand(configOptions.get<int>("gp seed"));
  MatrixXd initial_guesses;
  generate_initial_guesses(initial_guesses, num_restarts,
                           sigma_bounds, length_scale_bounds,
                           nugget_bounds);
  ROL::Ptr<std::ostream> outStream;
  Teuchos::oblackholestream bhs;
  outStream = ROL::makePtrFromRef(bhs);
  /* Uncomment if you'd like to print ROL's output to screen.
   * Useful for debugging */
  //outStream = ROL::makePtrFromRef(std::cout);

  /* No more reading in rol_params from an xml file
   * Set defaults in here instead.
   * TODO: Make some of ROL's parameters controllable by
   * a surrogates/Dakota user */

  /*
  std::string paramfile = "rol_params.xml";
  auto rol_params = Teuchos::rcp(new ParameterList);
  Teuchos::updateParametersFromXmlFile(paramfile, rol_params.ptr());
  */

  auto gp_mle_rol_params = Teuchos::rcp(new ParameterList("GP_MLE_Optimization"));
  set_default_optimization_params(gp_mle_rol_params);

  auto gp_objective = std::make_shared<GP_Objective>(*this);
  int dim = numVariables + 1 + numPolyTerms + numNuggetTerms;
  ROL::Algorithm<double> algo("Line Search",*gp_mle_rol_params);

  /* set up parameter vectors and bounds */
  ROL::Ptr<std::vector<double> > x_ptr = ROL::makePtr<std::vector<double>>(dim, 0.0);
  ROL::StdVector<double> x(x_ptr);
  ROL::Ptr<ROL::Bounds<double> > bound;
  ROL::Ptr<std::vector<double> > lo_ptr = ROL::makePtr<std::vector<double> >(dim,0.0);
  ROL::Ptr<std::vector<double> > hi_ptr = ROL::makePtr<std::vector<double> >(dim,0.0);
  /* sigma bounds */
  (*lo_ptr)[0] = log(sigma_bounds(0));
  (*hi_ptr)[0] = log(sigma_bounds(1));
  /* length scale bounds */
  for (int i = 0; i < numVariables; i++) {
    (*lo_ptr)[i+1] = log(length_scale_bounds(i,0));
    (*hi_ptr)[i+1] = log(length_scale_bounds(i,1));
  }
  if (estimateTrend) {
    for (int i = 0; i < numPolyTerms; i++) {
      (*lo_ptr)[numVariables+1+i] = beta_bounds(i,0);
      (*hi_ptr)[numVariables+1+i] = beta_bounds(i,1);
    }
  }
  if (estimateNugget) {
    (*lo_ptr)[dim-1] = log(nugget_bounds(0));
    (*hi_ptr)[dim-1] = log(nugget_bounds(1));
  }

  ROL::Ptr<ROL::Vector<double> > lop = ROL::makePtr<ROL::StdVector<double> >(lo_ptr);
  ROL::Ptr<ROL::Vector<double> > hip = ROL::makePtr<ROL::StdVector<double> >(hi_ptr);
  bound = ROL::makePtr<ROL::Bounds<double> >(lop,hip);

  std::vector<std::string> output;

  objectiveFunctionHistory.resize(num_restarts);
  bestObjFunValue = 1.0e300;

  double final_obj_value;
  VectorXd final_obj_gradient(dim);

  for (int i = 0; i < num_restarts; i++) {
    for (int j = 0; j < dim; ++j) {
      (*x_ptr)[j] = initial_guesses(i,j);
    }
    output = algo.run(x, *gp_objective, *bound, true, *outStream);
    for (int j = 0; j < thetaValues.size(); ++j) {
      (thetaValues)(j) = (*x_ptr)[j];
    }
    if (estimateTrend) {
      for (int j = 0; j < numPolyTerms; ++j) {
        betaValues(j) = (*x_ptr)[numVariables+1+j];
      }
    }
    if (estimateNugget) {
      estimatedNuggetValue = (*x_ptr)[numVariables+1+numPolyTerms];
    }
    /* get the final objective function value */
    negative_marginal_log_likelihood(final_obj_value, final_obj_gradient);
    if (final_obj_value < bestObjFunValue) {
      bestObjFunValue = final_obj_value;
      bestThetaValues = thetaValues;
      if (estimateTrend)
        bestBetaValues = betaValues;
      if (estimateNugget)
        bestEstimatedNuggetValue = estimatedNuggetValue;
    }
    objectiveFunctionHistory(i) = final_obj_value;
    algo.reset();
  }

  thetaValues = bestThetaValues;
  if (estimateTrend) {
    betaValues = bestBetaValues;
    /* set the betas in the polynomialRegression class */
    polyRegression->set_polynomial_coeffs(bestBetaValues);
  }
  if (estimateNugget)
    estimatedNuggetValue = bestEstimatedNuggetValue;

  /*
  std::cout << "\n";
  std::cout << objectiveFunctionHistory << std::endl;
  std::cout << "optimal theta values in log-space:" << std::endl;
  std::cout << bestThetaValues << std::endl;
  std::cout << "best objective function value is " <<  bestObjFunValue << std::endl;
  std::cout << "best objective function gradient norm is " <<  final_obj_gradient.norm() << std::endl;
  */
}

void GaussianProcess::value(const MatrixXd &samples, MatrixXd &approx_values) {
  if (samples.cols() != numVariables) {
    throw(std::runtime_error("Gaussian Process value inputs are not consistent."
          " Dimension of the feature space for the evaluation points and Gaussian Process do not match"));
  }

  /*
  if (numPredictionPts != approx_values.rows()) {
    throw(std::runtime_error("Gaussian Process value inputs are not consistent."
          " Number of samples and approximation sizes do not match"));
  }
  */

  const int numPredictionPts = samples.rows();
  approx_values.resize(numPredictionPts,numVariables);

  /* scale the samples (prediction points) */
  MatrixXd scaled_pred_pts = *(dataScaler->scale_samples(samples));

  /* compute the Gram matrix and its Cholesky factorization */
  compute_gram(false);
  CholFact.compute(GramMatrix);

  MatrixXd pred_mat, resid, pred_basis_matrix;
  compute_prediction_matrix(scaled_pred_pts,pred_mat);

  if (estimateTrend) {
    resid = targetValues - basisMatrix*betaValues;
  }
  else
    resid = targetValues;


  MatrixXd chol_solve_resid, chol_solve_pred_mat;
  chol_solve_resid = CholFact.solve(resid);
  chol_solve_pred_mat = CholFact.solve(pred_mat.transpose());

  /* value */
  approx_values = pred_mat*chol_solve_resid;

  /* compute the covariance matrix and standard deviation */
  MatrixXd Gram_pred;
  compute_gram_pred(scaled_pred_pts,Gram_pred);
  posteriorCov = Gram_pred - pred_mat*chol_solve_pred_mat;

  if (estimateTrend) {
    polyRegression->compute_basis_matrix(scaled_pred_pts, pred_basis_matrix);
    MatrixXd z = CholFact.solve(basisMatrix);
    MatrixXd R_mat = pred_basis_matrix - pred_mat*(z);
    MatrixXd h_mat = basisMatrix.transpose()*z;
    approx_values += pred_basis_matrix*betaValues;
    posteriorCov += R_mat*(h_mat.ldlt().solve(R_mat.transpose()));
  }

  posteriorStdDev.resize(numPredictionPts);
  for (int i = 0; i < numPredictionPts; i++) {
    posteriorStdDev(i) = sqrt(posteriorCov(i,i));
  }
}

void GaussianProcess::gradient(const MatrixXd &samples, MatrixXd &gradient,
                               const int qoi) {

  // Surrogate models don't yet support multiple responses
  assert(qoi == 0);

  if (samples.cols() != numVariables) {
    throw(std::runtime_error("Gaussian Process gradient inputs are not consistent."
          " Dimension of the feature space for the evaluation points and Gaussian Process do not match"));
  }

  const int numPredictionPts = samples.rows();

  /* resize the gradient */
  gradient.resize(numPredictionPts,numVariables);

  /* scale the samples (prediction points) */
  MatrixXd scaled_pred_pts = *(dataScaler->scale_samples(samples));

  /* compute the Gram matrix and its Cholesky factorization */
  compute_gram(false);
  CholFact.compute(GramMatrix);

  MatrixXd pred_mat, chol_solve_resid, first_deriv_pred_mat,
           grad_components, resid;
  compute_prediction_matrix(scaled_pred_pts,pred_mat);
  resid = targetValues;
  if (estimateTrend)
    resid -= basisMatrix*betaValues;
  chol_solve_resid = CholFact.solve(resid);

  for (int i = 0; i < numVariables; i++) {
    compute_first_deriv_pred_mat(pred_mat,scaled_pred_pts,i,first_deriv_pred_mat);
    grad_components = first_deriv_pred_mat*chol_solve_resid;
    gradient.col(i) = grad_components.col(0);
  }

  /* extra terms for GP with a trend */
  if (estimateTrend) {
    MatrixXd poly_grad_pred_pts;
    polyRegression->gradient(scaled_pred_pts, poly_grad_pred_pts);
    gradient += poly_grad_pred_pts;
  }

}

void GaussianProcess::hessian(const MatrixXd &sample, MatrixXd &hessian,
                              const int qoi) {

  // Surrogate models don't yet support multiple responses
  assert(qoi == 0);

  if (sample.rows() != 1) {
    throw(std::runtime_error("Gaussian Process Hessian evaluation is for a single point."
          "The input contains more than one sample."));
  }

  /* resize the Hessian */
  hessian.resize(numVariables,numVariables);

  /* scale the samples (prediction points) */
  MatrixXd scaled_pred_point = *(dataScaler->scale_samples(sample));

  /* compute the Gram matrix and its Cholesky factorization */
  compute_gram(false);
  CholFact.compute(GramMatrix);

  MatrixXd pred_mat, chol_solve_resid, second_deriv_pred_mat, resid;
  compute_prediction_matrix(scaled_pred_point,pred_mat);
  resid = targetValues;
  if (estimateTrend)
    resid -= basisMatrix*betaValues;
  chol_solve_resid = CholFact.solve(resid);

  /* Hessian */
  for (int i = 0; i < numVariables; i++) {
    for (int j = i; j < numVariables; j++) {
      compute_second_deriv_pred_mat(pred_mat,scaled_pred_point,i,j,second_deriv_pred_mat);
      hessian(i,j) = (second_deriv_pred_mat*chol_solve_resid)(0,0);
      if (i != j)
        hessian(j,i) = hessian(i,j);
    }
  }

  if (estimateTrend) {
    MatrixXd poly_hessian_pred_pt;
    polyRegression->hessian(scaled_pred_point, poly_hessian_pred_pt);
    hessian += poly_hessian_pred_pt;
  }

}

void GaussianProcess::set_default_optimization_params(Teuchos::RCP<ParameterList> rol_params) {
  /* Secant */
  rol_params->sublist("General").sublist("Secant").
             set("Type","Limited-Memory BFGS");
  rol_params->sublist("General").sublist("Secant").
             set("Maximum Storage",25);
  /* Step */ 
  rol_params->sublist("General").sublist("Step").
             sublist("Line Search").set("Function Evaluation Limit",20);

  rol_params->sublist("General").sublist("Step").
             sublist("Line Search").set("Sufficient Decrease Tolerance",1.0e-4);

  rol_params->sublist("General").sublist("Step").
             sublist("Line Search").set("Initial Step Size",1.0);

  rol_params->sublist("General").sublist("Step").
             sublist("Line Search").sublist("Descent Method").
             set("Type","Quasi-Newton");

  rol_params->sublist("General").sublist("Step").
             sublist("Line Search").sublist("Descent Method").
             set("Nonlinear CG Type","Hestenes-Stiefel");

  rol_params->sublist("General").sublist("Step").
             sublist("Line Search").sublist("Curvature Condition").
             set("Type","Strong Wolfe Conditions");

  rol_params->sublist("General").sublist("Step").
             sublist("Line Search").sublist("Curvature Condition").
             set("General Parameter",0.9);

  rol_params->sublist("General").sublist("Step").
             sublist("Line Search").sublist("Curvature Condition").
             set("Generalized Wolfe Parameter",0.6);

  rol_params->sublist("General").sublist("Step").
             sublist("Line Search").sublist("Line-Search Method").
             set("Type","Cubic Interpolation");

  rol_params->sublist("General").sublist("Step").
             sublist("Line Search").sublist("Line-Search Method").
             set("Backtracking Rate",0.5);

  rol_params->sublist("General").sublist("Step").
             sublist("Line Search").sublist("Line-Search Method").
             set("Bracketing Tolerance",1.0e-8);

  /* Status Test */
  rol_params->sublist("Status Test").
             set("Gradient Tolerance",1.0e-6);

  rol_params->sublist("Status Test").
             set("Step Tolerance",1.0e-8);

  rol_params->sublist("Status Test").
             set("Iteration Limit",200);
}



}  // namespace surrogates
}  // namespace dakota

