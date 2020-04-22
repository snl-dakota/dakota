/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_SURROGATE_HPP
#define DAKOTA_SURROGATES_SURROGATE_HPP

#include "DataScaler.hpp"
#include "util_data_types.hpp"

namespace dakota {
namespace surrogates {

/**
 *  \brief Parent class for surrogate models.
 *
 *  The Surrogate class defines the API for surrogate models
 *  contained in the Dakota surrogates module.
 *
 *  Pure virtual functions include build, value, and
 *  default_options. Gradient and Hessian methods are optional.
 *
 *  Configuration options for a surrogate are set through the
 *  use of a Teuchos ParameterList named configOptions.
 */
class Surrogate {

public:

  /// Default constructor.
  Surrogate();

  /// Default destructor.
  virtual ~Surrogate();

  /**
   * \brief Build the Surrogate using specified build data.
   *
   * \param[in] samples Matrix of data for surrogate construction - (num_samples by num_features)
   * \param[in] response Vector of responses/targets for surrogate construction - (num_samples by num_qoi)
   */
  virtual void build(const MatrixXd &samples, const MatrixXd &response) = 0;

  /**
   *  \brief Evaluate the Surrogate at a set of prediction points.
   *  \param[in] samples Matrix of prediction points - (num_pts by num_features).
   *  \param[out] approx_values Values of the Surrogate at the prediction
   *  points - (num_pts by num_qoi) 
   */
  virtual void value(const MatrixXd &samples, MatrixXd &value) = 0;

  /**
   *  \brief Evaluate the gradient of the Surrogate at a set of prediction points.
   *  \param[in] samples Coordinates of the prediction points - (num_pts by num_features).
   *  \param[out] gradient Matrix of gradient vectors at the prediction points - 
   *  (num_pts by num_features).
   *  \param[in] qoi Index of the quantity of interest for gradient evaluation - 
   *  0 for scalar-valued surrogates.
   *
   */
  virtual void gradient(const MatrixXd &samples, MatrixXd &gradient, int qoi);

  /**
   *  \brief Evaluate the Hessian of the Surrogate at a single point.
   *  \param[in] sample Coordinates of the prediction point - (num_samples by num_features).
   *  \param[out] hessian Hessian matrix at the prediction point - 
   *  (num_features by num_features).
   *  \param[in] qoi Index of the quantity of interest for Hessian evaluation - 
   *  0 for scalar-valued surrogates.
   */
  virtual void hessian(const MatrixXd &samples, MatrixXd &hessian, int qoi);

  /**
   *  \brief Set the Surrogate's configOptions.
   *  \param[in] options ParameterList of configuration options.
   */
  void set_options(const ParameterList &options);

  /**
   *  \brief Get the Surrogate's configOptions.
   *  \param[out] options ParameterList of configuration options.
   */
  void get_options(ParameterList &options);

  /// Initialize the Surrogate's defaultConfigOptions.
  virtual void default_options() = 0;

  /// DataScaler class for a Surrogate's build samples.
  std::shared_ptr<util::DataScaler> dataScaler;

protected:
  /// Number of samples in the Surrogate's build samples.
  int numSamples;
  /// Number of features/variables in the Surrogate's build samples.
  int numVariables;
  /**
   *  \brief Number of quantities of interest predicted by the surrogate. For 
   *  scaler-valued surrogates numQOI = 1.
   */
  int numQOI;
  /// Default Key/value options to configure the surrogate.
  ParameterList defaultConfigOptions;
  /// Key/value options to configure the surrogate - will override defaultConfigOptions.
  ParameterList configOptions;

};

} // namespace surrogates
} // namespace dakota

#endif // include guard
