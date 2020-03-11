/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "GaussianProcess.hpp"

#include "Teuchos_XMLParameterListHelpers.hpp"
#include "Teuchos_oblackholestream.hpp"

#include "GP_Objective.hpp"
#include "ROL_Algorithm.hpp"
#include "ROL_Bounds.hpp"

namespace dakota {

using util::NormalizationScaler;
using util::NoScaler;
using util::StandardizationScaler;

namespace surrogates {

GaussianProcess::GaussianProcess(){}
GaussianProcess::~GaussianProcess(){}

const VectorXd & GaussianProcess::get_posterior_std_dev() const { return *posteriorStdDev; }

const MatrixXd & GaussianProcess::get_posterior_covariance() const { return *posteriorCov; }

const VectorXd & GaussianProcess::get_theta_values() const { return *thetaValues; }

int GaussianProcess::get_num_variables() const { return numVariables; }

void GaussianProcess::set_theta(const std::vector<double> theta_new) {
  for (int i = 0; i < thetaValues->size(); i++)
    (*thetaValues)(i) = theta_new[i];
}

void GaussianProcess::generate_initial_guesses(MatrixXd &initial_guesses, int num_restarts,
                                               const VectorXd &sigma_bounds,
                                               const MatrixXd &length_scale_bounds) {
  initial_guesses = MatrixXd::Random(num_restarts,numVariables+1);
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
                                   *exp(-2.0*((*thetaValues)(index+1)));
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
                                   *exp(-2.0*((*thetaValues)(index_i+1) + (*thetaValues)(index_j+1)))
                                   - diagonal_factor*exp(-2.0*((*thetaValues)(index_i+1))));
    }
  }
}

double GaussianProcess::sq_exp_cov(const int i, const int j) {
  double inside = 0.0;
  for (int k = 0; k < numVariables; k++)
    inside += componentwiseDistances[k](i,j)*std::exp(-2.0*(*thetaValues)(k+1));

  return std::exp(2.0*(*thetaValues)(0))*exp(-0.5*inside);
}

double GaussianProcess::sq_exp_cov_pred(const VectorXd &x, const VectorXd &y) {
  VectorXd diff = x - y;
  double inside = 0.0;
  for (int k = 0; k < numVariables; k++)
    inside += std::pow(diff(k),2.0)*std::exp(-2.0*(*thetaValues)(k+1));

  return std::exp(2.0*(*thetaValues)(0))*std::exp(-0.5*inside);
}
                

void GaussianProcess::compute_gram(bool compute_derivs){
  for (int i = 0; i < numSamples; i++) {
    for (int j = i; j < numSamples; j++) {
      (*GramMatrix)(i,j) = sq_exp_cov(i,j);
      if (i != j)
        (*GramMatrix)(j,i) = (*GramMatrix)(i,j);
    }
  }

  if (compute_derivs) {
    GramMatrixDerivs[0] = 2.0*(*GramMatrix);
    for (int k = 1 ; k < numVariables + 1; k++) {
      GramMatrixDerivs[k] = GramMatrix->cwiseProduct(componentwiseDistances[k-1])
                            *std::exp(-2.0*(*thetaValues)(k));
    }
  }

  /* add in the nugget */
  for (int i = 0; i < numSamples; i++)
    (*GramMatrix)(i,i) += nuggetValue;
}

void GaussianProcess::negative_marginal_log_likelihood(double &obj_value, VectorXd &obj_gradient) {
  double logSum;
  VectorXd z;
  MatrixXd Q, eye_matrix;
  eye_matrix = MatrixXd::Identity(numSamples,numSamples);

  compute_gram(true);
  CholFact.compute(*GramMatrix);
  VectorXd D(CholFact.vectorD());

  z = CholFact.solve(*targetValues);
  logSum = 0.5*log(D.array()).matrix().sum();
  Q = -0.5*(z*z.transpose() - CholFact.solve(eye_matrix));

  obj_value = logSum + 0.5*targetValues->col(0).dot(z) + 
              static_cast<double>(numSamples)/2.0*log(2.0*PI);

  MatrixXd product(numSamples,numSamples);
  for (int k = 0; k < numVariables+1; k++)
    obj_gradient(k) = (GramMatrixDerivs[k].cwiseProduct(Q)).sum();
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
}


// BMA NOTE: ParameterList::get() can throw, so direct delegation
// probably not good
GaussianProcess::GaussianProcess(const MatrixXd &samples,
				 const MatrixXd &response,
				 const Teuchos::ParameterList& param_list):
  GaussianProcess(samples, response,
		  param_list.get<VectorXd>("sigma_bounds"),
		  param_list.get<MatrixXd>("length_scale_bounds"),
		  param_list.get<std::string>("scaler_type"),
		  param_list.get<int>("num_restarts"),
		  param_list.get<double>("nugget"),
		  param_list.get<int>("gp_seed")
		  )
{
}


GaussianProcess::GaussianProcess(const MatrixXd &samples, 
                                 const MatrixXd &response, 
                                 const VectorXd &sigma_bounds,
                                 const MatrixXd &length_scale_bounds,
                                 const std::string scaler_type,
                                 const int num_restarts,
                                 const double nugget_val,
                                 const int seed) {

  numSamples = samples.rows();
  numVariables = samples.cols();
  targetValues = std::make_shared<MatrixXd>(response);
  
  /* Scale the data */
  if (scaler_type == "mean normalization")
    dataScaler = std::make_shared<NormalizationScaler>(samples,true);
  else if (scaler_type == "min-max normalization")
    dataScaler = std::make_shared<NormalizationScaler>(samples,false);
  if (scaler_type == "standardization")
    dataScaler = std::make_shared<StandardizationScaler>(samples);
  else if (scaler_type == "none" )
    dataScaler = std::make_shared<NoScaler>(samples);
  else
    throw(std::string("Invalid scaler type"));

  MatrixXd scaled_samples = dataScaler->get_scaled_features();

  /* size of thetaValues for squared exponential kernel and one QoI */
  thetaValues = std::make_shared<VectorXd>(numVariables+1);
  bestThetaValues = std::make_shared<VectorXd>(numVariables+1);
  /* set the size of the GramMatrix and its derivatives */
  GramMatrix = std::make_shared<MatrixXd>(numSamples, numSamples);
  GramMatrixDerivs.resize(numVariables+1);
  for (int k = 0; k < numVariables+1; k++) {
    GramMatrixDerivs[k].resize(numSamples, numSamples);
  }

  /* set the nugget */
  nuggetValue = nugget_val;

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
  srand(seed);
  MatrixXd initial_guesses;
  generate_initial_guesses(initial_guesses, num_restarts,
                           sigma_bounds, length_scale_bounds);

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
  auto rol_params = Teuchos::rcp(new Teuchos::ParameterList);
  Teuchos::updateParametersFromXmlFile(paramfile, rol_params.ptr());
  */

  auto gp_mle_rol_params = Teuchos::rcp(new Teuchos::ParameterList("GP_MLE_Optimization"));
  set_default_optimization_params(gp_mle_rol_params);

  auto gp_objective = std::make_shared<GP_Objective>(this);
  int dim = numVariables + 1;
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

  ROL::Ptr<ROL::Vector<double> > lop = ROL::makePtr<ROL::StdVector<double> >(lo_ptr);
  ROL::Ptr<ROL::Vector<double> > hip = ROL::makePtr<ROL::StdVector<double> >(hi_ptr);
  bound = ROL::makePtr<ROL::Bounds<double> >(lop,hip);

  std::vector<std::string> output;

  objectiveFunctionHistory = std::make_shared<VectorXd>(num_restarts);
  bestObjFunValue = 1.0e300;

  double final_obj_value;
  VectorXd final_obj_gradient(dim);

  for (int i = 0; i < num_restarts; i++) {
    for (int j = 0; j < dim; ++j) {
      (*x_ptr)[j] = initial_guesses(i,j);
    }
    output = algo.run(x, *gp_objective, *bound, true, *outStream);
    for (int j = 0; j < thetaValues->size(); ++j) {
      (*thetaValues)(j) = (*x_ptr)[j];
    }
    /* get the final objective function value */
    negative_marginal_log_likelihood(final_obj_value, final_obj_gradient);
    if (final_obj_value < bestObjFunValue) {
      bestObjFunValue = final_obj_value;
      *bestThetaValues = *thetaValues;
    }
    (*objectiveFunctionHistory)(i) = final_obj_value;
    algo.reset();
  }

  *thetaValues = *bestThetaValues;

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
  CholFact.compute(*GramMatrix);

  MatrixXd pred_mat;
  compute_prediction_matrix(scaled_pred_pts,pred_mat);

  MatrixXd chol_solve_target, chol_solve_pred_mat;
  chol_solve_target = CholFact.solve(*targetValues);
  chol_solve_pred_mat = CholFact.solve(pred_mat.transpose());

  /* value */
  approx_values = pred_mat*chol_solve_target;

  /* compute the covariance matrix and standard deviation */
  MatrixXd Gram_pred;
  compute_gram_pred(scaled_pred_pts,Gram_pred);
  posteriorCov = std::make_shared<MatrixXd>( Gram_pred - pred_mat*chol_solve_pred_mat );

  posteriorStdDev = std::make_shared<VectorXd>(numPredictionPts);
  for (int i = 0; i < numPredictionPts; i++) {
    (*posteriorStdDev)(i) = sqrt((*posteriorCov)(i,i));
  }
}

void GaussianProcess::gradient(const MatrixXd &samples, MatrixXd &gradient) {

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
  CholFact.compute(*GramMatrix);

  MatrixXd pred_mat, chol_solve_target, first_deriv_pred_mat;
  compute_prediction_matrix(scaled_pred_pts,pred_mat);
  chol_solve_target = CholFact.solve(*targetValues);

  /* gradient */
  for (int i = 0; i < numPredictionPts; i++) {
    for (int j = 0; j < numVariables; j++) {
      compute_first_deriv_pred_mat(pred_mat,scaled_pred_pts,j,first_deriv_pred_mat);
      auto grad_components = first_deriv_pred_mat*chol_solve_target;
      gradient(i,j) = grad_components(i,0);
    }
  }

}

void GaussianProcess::hessian(const MatrixXd &sample, MatrixXd &hessian) {

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
  CholFact.compute(*GramMatrix);

  MatrixXd pred_mat, chol_solve_target, second_deriv_pred_mat;
  compute_prediction_matrix(scaled_pred_point,pred_mat);
  chol_solve_target = CholFact.solve(*targetValues);

  /* Hessian */
  for (int i = 0; i < numVariables; i++) {
    for (int j = i; j < numVariables; j++) {
      compute_second_deriv_pred_mat(pred_mat,scaled_pred_point,i,j,second_deriv_pred_mat);
      auto hessian_component = second_deriv_pred_mat*chol_solve_target;
      hessian(i,j) = hessian_component(0,0);
      if (i != j)
        hessian(j,i) = hessian(i,j);
    }
  }

}

void GaussianProcess::set_default_optimization_params(Teuchos::RCP<Teuchos::ParameterList> rol_params) {
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
  rol_params->sublist("General").sublist("Status Test").
             set("Gradient Tolerance",1.0e-10);

  rol_params->sublist("General").sublist("Status Test").
             set("Step Tolerance",1.0e-8);

  rol_params->sublist("General").sublist("Status Test").
             set("Iteration Limit",100);
}


}  // namespace surrogates
}  // namespace dakota

