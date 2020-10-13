/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_GAUSSIAN_PROCESS_HPP
#define DAKOTA_SURROGATES_GAUSSIAN_PROCESS_HPP

#include "UtilDataScaler.hpp"
#include "SurrogatesPolynomialRegression.hpp"
#include "SurrogatesBase.hpp"

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/vector.hpp>

namespace dakota {

namespace surrogates {

/**
 *  \brief The GaussianProcess constructs a Gaussian Process
 *  regressor surrogate given a matrix of data.
 *
 *  The Gaussian Process (GP) uses an anisotropic
 *  squared exponential kernel with a constant multiplicative
 *  scaling factor. This yields a total of num_features + 1
 *  kernel hyperparameters. These parameters are internally
 *  transformed to a log-space vector (theta) for optimization
 *  and evaluation of the GP. Polynomial trend and nugget
 *  estimation are optional.
 *
 *  The GP's parameters are determined through maximum
 *  likelihood estimation (MLE) via minimization of the negative
 *  marginal log-likelihood function. ROL's implementation of
 *  L-BFGS-B is used to solve the optimization problem, and the
 *  algorithm may be run from multiple random initial guesses
 *  to increase the chance of finding the global minimum.
 *
 *  Once the GP is constructed its mean, standard deviation,
 *  and covariance matrix can be computed for a set of prediction
 *  points. Gradients and Hessians are available.
 */
class GaussianProcess: public Surrogate {

public:

  /* Constructors and destructors */

  /// Constructor that uses defaultConfigOptions and does not build.
  GaussianProcess();

  /**
   * \brief Constructor that sets configOptions but does not build.
   * \param[in] param_list List that overrides entries in defaultConfigOptions.
   */
  GaussianProcess(const Teuchos::ParameterList& param_list);

  /**
   * \brief Constructor for the GaussianProcess that sets configOptions
   *        but does not build the GP.
   *
   * \param[in] param_list_xml_filename A ParameterList file (relative to the location of the
   * Dakota input file) that overrides entries in defaultConfigOptions.
   */
  GaussianProcess(const std::string &param_list_xml_filename);

  /**
   * \brief Constructor for the GaussianProcess that sets configOptions
   *        and builds the GP.
   * \param[in] samples Matrix of data for surrogate construction - (num_samples by num_features)
   * \param[in] response Vector of targets for surrogate construction - (num_samples by num_qoi = 1; only 1 response is supported currently).
   * \param[in] param_list List that overrides entries in defaultConfigOptions
   */
  GaussianProcess(const MatrixXd &samples, const MatrixXd &response,
                  const Teuchos::ParameterList &param_list);

  /**
   * \brief Constructor for the GaussianProcess that sets configOptions
   *        and builds the GP.
   *
   * \param[in] samples Matrix of data for surrogate construction - (num_samples by num_features)
   * \param[in] response Vector of targets for surrogate construction - (num_samples by num_qoi = 1;
   *  only 1 response is supported currently).
   * \param[in] param_list_xml_filename A ParameterList file (relative to the location of the
   *  Dakota input file) that overrides entries in defaultConfigOptions.
   */
  GaussianProcess(const MatrixXd &samples, const MatrixXd &response,
                  const std::string &param_list_xml_filename);

  /// Default destructor
  ~GaussianProcess();

  /* Public utility functions */

  /**
   * \brief Build the GP using specified build data.
   * \param[in] eval_points Matrix of data for surrogate construction - (num_samples by num_features)
   * \param[in] response Vector of targets for surrogate construction - (num_samples by num_qoi = 1; only 1 response is supported currently).
   */
  void build(const MatrixXd &eval_points, const MatrixXd &response) override;

  /**
   *  \brief Evaluate the Gaussian Process at a set of prediction points for a single qoi.
   *  \param[in] eval_points Matrix for prediction points - (num_points by num_features).
   *  \param[in] qoi Index for surrogate QoI.
   *  \returns Mean of the Gaussian process at the prediction points.
   */
  VectorXd value(const MatrixXd &eval_points, const int qoi) override;

  /**
   *  \brief Evaluate the Gaussian Process at a set of prediction points for QoI index 0.
   *  \param[in] eval_points Matrix for prediction points - (num_points by num_features).
   *  \returns Mean of the Gaussian process at the prediction points.
   */
  VectorXd value(const MatrixXd &eval_points) {return value(eval_points, 0); }

  /**
   *  \brief Evaluate the gradient of the Gaussian process at a set of prediction points
   *  for a single QoI.
   *  \param[in] eval_points Coordinates of the prediction points - (num_pts by num_features).
   *  \param[in] qoi Index of response/QoI for which to compute derivatives.
   *  \returns Matrix of gradient vectors at the prediction points - 
   *  (num_pts by num_features).
   */
  MatrixXd gradient(const MatrixXd &eval_points, const int qoi) override;

  /**
   *  \brief Evaluate the gradient of the Gaussian process at a set of prediction points
   *  for QoI index 0.
   *  \param[in] eval_points Coordinates of the prediction points - (num_pts by num_features).
   *  \param[in] qoi Index of response/QoI for which to compute derivatives.
   *  \returns Matrix of gradient vectors at the prediction points - 
   *  (num_pts by num_features).
   */
  MatrixXd gradient(const MatrixXd &eval_points) { return gradient(eval_points, 0); }

  /**
   *  \brief Evaluate the Hessian of the Gaussian process at a single point for a single QoI.
   *  \param[in] eval_point Coordinates of the prediction point - (1 by num_features).
   *  \param[in] qoi Index of response/QoI for which to compute derivatives
   *  \returns Hessian matrix at the prediction point - 
   *  (num_features by num_features).
   */
  MatrixXd hessian(const MatrixXd &eval_point, const int qoi) override;

  /**
   *  \brief Evaluate the Hessian of the Gaussian process at a single point for QoI index 0.
   *  \param[in] eval_point Coordinates of the prediction point - (1 by num_features).
   *  \param[in] qoi Index of response/QoI for which to compute derivatives
   *  \returns Hessian matrix at the prediction point - 
   *  (num_features by num_features).
   */
  MatrixXd hessian(const MatrixXd &eval_point) { return hessian(eval_point, 0); }

  /**
   *  \brief Evaluate the covariance matrix for the Gaussian Process at a set of prediction points for a single QoI index.
   *  \param[in] eval_points Matrix for the prediction points - (num_points by num_features).
   *  \param[in] qoi Index of response/QoI for which to compute derivatives
   *  \returns[out] Covariance matrix for the Gaussian process at the prediction points.
   */
  MatrixXd covariance(const MatrixXd &eval_point, const int qoi);

  /**
   *  \brief Evaluate the covariance matrix for the Gaussian Process at a set of prediction points for QoI index 0.
   *  \param[in] eval_points Matrix for the prediction points - (num_points by num_features).
   *  \returns[out] Covariance of the Gaussian process at the prediction points.
   */
  MatrixXd covariance(const MatrixXd &eval_points) {return covariance(eval_points, 0);}

  /**
   *  \brief Evaluate the variance of the Gaussian Process at a set of prediction points for a given QoI index.
   *  \param[in] eval_points Matrix for the prediction points - (num_points by num_features).
   *  \param[in] qoi Index of response/QoI for which to compute derivatives
   *  \returns[out] Variance of the Gaussian process at the prediction points.
   */
  VectorXd variance(const MatrixXd &eval_points, const int qoi);

  /**
   *  \brief Evaluate the variance of the Gaussian Process at a set of prediction points for QoI index 0.
   *  \param[in] eval_points Matrix for the prediction points - (num_points by num_features).
   *  \returns[out] Variance of the Gaussian process at the prediction points.
   */
  VectorXd variance(const MatrixXd &eval_points) {return variance(eval_points, 0);}

  /**
   *  \brief Evaluate the negative marginal loglikelihood and its 
   *  gradient.
   *  \param[in] compute_grad Flag for computation of gradient.
   *  \param[in] compute_gram Flag for various Gram matrix calculations.
   *  \param[out] obj_value Value of the objection function.
   *  \param[out] obj_gradient Gradient of the objective function.
   */
  void negative_marginal_log_likelihood(bool compute_grad, bool compute_gram, double &obj_value, VectorXd &obj_gradient);

  /* Get/set functions */

  /**
   *  \brief Get the number of optimization variables.
   *  \returns Number of total optimization variables (hyperparameters + trend coefficients + nugget)
   */
  int get_num_opt_variables();

  /**
   *  \brief Get the dimension of the feature space.
   *  \returns numVariables The dimension of the feature space.
   */
  int get_num_variables() const;

  /**
   *  \brief Update the vector of optimization parameters.
   *  \param[in] opt_params Vector of optimization parameter values.
   */
  void set_opt_params(const std::vector<double> &opt_params);


  std::shared_ptr<Surrogate> clone() const override
  { return std::make_shared<GaussianProcess>(configOptions); }

private:
  /* Private utility functions */

  /// Construct and populate the defaultConfigOptions.
  void default_options() override;

  /// Compute squared distances between the scaled build points.
  void compute_build_dists();

  /**
   *  \brief Compute distances between build and prediction points. This includes
   *  build-prediction and prediction-prediction distance matrices.
   *  \param[in] scaled_pred_pts Matrix of scaled prediction points.
   */
  void compute_pred_dists(const MatrixXd &scaled_pred_pts);

  /**
   *  \brief Compute a Gram matrix given a vector of squared distances and
   *  optionally compute its derivatives and/or adds nugget terms.
   *  \param[in] dists2 Vector of squared distance matrices.
   *  \param[in] add_nugget Bool for whether or add nugget terms.
   *  \param[in] compute_derivs Bool for whether or not to compute the 
   *  derivatives of the Gram matrix.
   *  \param[out] gram Gram matrix.
   */
  void compute_gram(const std::vector<MatrixXd> &dists2, bool add_nugget, 
                    bool compute_derivs, MatrixXd &gram);

  /**
   *  \brief Compute the first derivatve of the prediction matrix for a given component.
   *  \param[in] pred_gram Prediction Gram matrix - Rectangular matrix of kernel
   *  evaluations between the surrogate and prediction points.
   *  \param[in] index Specifies the component of the derivative.
   *  \param[out] first_deriv_pred_gram First derivative of the prediction matrix 
   *  for a given component.
   */
  void compute_first_deriv_pred_gram(const MatrixXd &pred_gram, const int index,
                                     MatrixXd &first_deriv_pred_gram);

  /**
   *  \brief Compute the second derivatve of the prediction matrix for a pair of components.
   *  \param[in] pred_gram Prediction Gram matrix - Rectangular matrix of kernel evaluations 
   *  between the surrogate and prediction points.
   *  \param[in] index_i Specifies the first component of the second derivative.
   *  \param[in] index_j Specifies the second component of the second derivative.
   *  \param[out] second_deriv_pred_gram Second derivative of the prediction 
   *  matrix for a pair of components.
   */
  void compute_second_deriv_pred_gram(const MatrixXd &pred_gram,
                                      const int index_i, const int index_j,
                                      MatrixXd &second_deriv_pred_gram);

  /**
   *  \brief Randomly generate initial guesses for the optimization routine.
   *  \param[in] sigma_bounds Bounds for the scaling hyperparameter (sigma).
   *  \param[in] length_scale_bounds Bounds for the length scales for each feature (l).
   *  \param[in] nugget_bounds Bounds for the nugget term.
   *  \param[in] num_restarts Number of restarts for the optimizer.
   *  \param[out] initial_guesses Matrix of initial guesses.
  */
  void generate_initial_guesses(const VectorXd &sigma_bounds,
                                const MatrixXd &length_scale_bounds,
                                const VectorXd &nugget_bounds,
                                const int num_restarts,
                                MatrixXd &initial_guesses);

  /**
   *  \brief Set the default optimization parameters for ROL for GP hyperparameter
   *  estimation.
   *  \param[in] rol_params RCP to a Teuchos::ParameterList of ROL's options.
   */
  void setup_default_optimization_params(Teuchos::RCP<Teuchos::ParameterList> rol_params);

  /* Private member variables */

  /// Small constant added to the diagonal of the Gram matrix to avoid ill-conditioning.
  double fixedNuggetValue;

  /// Identity matrix for the build points space.
  MatrixXd eyeMatrix;

  /// Basis matrix for the sample points in polynomial regression.
  MatrixXd basisMatrix;

  /// Target values for the surrogate dataset.
  MatrixXd targetValues;

  /// The scaled build points for the surrogate dataset.
  MatrixXd scaledBuildPoints;

  /// Vector of log-space hyperparameters.
  VectorXd thetaValues;

  /// Vector of polynomial coefficients.
  VectorXd betaValues;

  /// Estimated nugget term.
  double estimatedNuggetValue;

  /// Vector of best hyperparameters from MLE with restarts.
  VectorXd bestThetaValues;

  /// Vector of best polynomial coefficients from MLE with restarts.
  VectorXd bestBetaValues;

  /// Best estimated nugget value from MLE with restarts.
  double bestEstimatedNuggetValue;

  /// Final objective function values for each optimization run.
  VectorXd objectiveFunctionHistory;

  /// Gram matrix for the build points
  MatrixXd GramMatrix;

  /// Difference between target values and trend predictions.
  VectorXd trendTargetResidual;

  /// Cholesky solve for Gram matrix with trendTargetResidual rhs.
  VectorXd GramResidualSolution;

  /// Derivatives of the Gram matrix w.r.t. the hyperparameters.
  std::vector<MatrixXd> GramMatrixDerivs;

  /// Squared component-wise distances between points in the surrogate dataset.
  std::vector<MatrixXd> cwiseDists2;

  /// Component-wise distances between prediction and build points.
  std::vector<MatrixXd> cwiseMixedDists;

  /// Squared component-wise distances between prediction and build points.
  std::vector<MatrixXd> cwiseMixedDists2;

  /// Component-wise distances between prediction points.
  std::vector<MatrixXd> cwisePredDists2;

  /// Pivoted Cholesky factorization.
  Eigen::LDLT<MatrixXd> CholFact;

  /// Flag for recomputation of the best Cholesky factorization.
  bool hasBestCholFact;

  /// PolynomialRegression for trend function.
  std::shared_ptr<PolynomialRegression> polyRegression;

  /// Large constant for polynomial coefficients upper/lower bounds.
  const double betaBound = 1.0e20;

  /// Bool for polynomial trend (i.e. semi-parametric GP) estimation.
  bool estimateTrend;

  /// Number of terms in polynomial trend.
  int numPolyTerms = 0;

  /// Number of terms for the (estimated) nugget parameter.
  int numNuggetTerms = 0;

  /// Bool for nugget estimation.
  bool estimateNugget;

  /// Verbosity level.
  int verbosity;

  /// Final objective function value.
  double bestObjFunValue = std::numeric_limits<double>::max();

  /// Numerical constant -- needed for negative marginal log-likelihood.
  const double PI = 3.14159265358979323846;

private:

  /// Allow serializers access to private class data
  friend class boost::serialization::access;
  /// Serializer for save/load
  template<class Archive>
  void serialize(Archive& archive, const unsigned int version);

}; // class GaussianProcess


template<class Archive>
void GaussianProcess::serialize(Archive& archive, const unsigned int version)
{
  silence_unused_args(version);

  archive & boost::serialization::base_object<Surrogate>(*this);

  // BMA: Initial cut is aggressive, serializing most members
  archive & cwiseDists2;
  archive & thetaValues;
  archive & fixedNuggetValue;
  archive & estimateNugget;
  archive & estimatedNuggetValue;
  archive & estimateTrend;
  archive & scaledBuildPoints;
  archive & targetValues;
  archive & basisMatrix;
  archive & betaValues;
  archive & verbosity;
  // BMA TODO: leaving this as shared_ptr pending discussion as it seems natural
  // BMA NOTE: If serializing through shared_ptr, wouldn't have to
  // trap the nullptr case here...
  if (estimateTrend) {
    if (Archive::is_loading::value)
      polyRegression.reset(new PolynomialRegression());
    archive & *polyRegression;
  }
  // DTS: Set false so that the Cholesky factorization is recomputed after load
  hasBestCholFact = false;
  archive & hasBestCholFact;
}

}  // namespace surrogates
}  // namespace dakota


BOOST_CLASS_EXPORT_KEY(dakota::surrogates::GaussianProcess)

#endif  // include guard
