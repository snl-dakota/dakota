/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef SURROGATES_DATA_SCALER_HPP
#define SURROGATES_DATA_SCALER_HPP

#include "teuchos_data_types.hpp"
#include "Eigen/Dense"

using Eigen::MatrixXd;
using Eigen::VectorXd;

namespace Surrogates {

/**
 *  \brief The DataScaler class computes the scaling coefficients and scales
 *         a 2D data matrix with dimensions num_features by num_samples.
 *
 *  There are currently 3 scaling options for the DataScaler class:
 *
 *  1. StandardizationScaler - transform each feature to have zero mean 
 *  and unit variance.
 *
 *  2. NormalizationScaler - normalizies each feature uses the max and min
 *  value divided by either the mean value (mean_normalization = true)
 *  or min value (mean_normalization = false)
 *  Also allows for a norm_factor scaling, required for the direct
 *  neural network.
 *
 *  3. NoScaler - scaling coefficients amount to an identity operation
 */


class DataScaler {

  public: 
    DataScaler();

    ~DataScaler();
/**
 *  \brief Apply scaling to a set of unscaled samples.
 *  \param[in] Unscaled matrix of samples.
 *  \returns Scaled matrix of samples.
*/
    MatrixXd scaleSamples(const MatrixXd &unscaled_samples);

/**
 *  \brief Get the vector of offsets.
 *  \returns Vector of scaler offsets - (num_features).
*/
    VectorXd getScalerFeaturesOffsets() {return scalerFeaturesOffsets;}

/**
 *  \brief Get the vector of scaling factors
 *  \returns Vector of scaling factors - (num_features).
*/
    VectorXd getScalerFeaturesScaleFactors() {return scalerFeaturesScaleFactors;}

/**
 *  \brief Get the the scaled data matrix.
 *  \returns Scaled features - (num_features by num_samples).
*/
    MatrixXd getScaledFeatures() {return scaledFeatures;}

/**
 *  \brief Bool for whether or not the the scaling coefficients
 *   have been computed.
 *
*/
    bool has_scaling = false;

  protected: 

    /// Vector of offsets - (num_features).
    VectorXd scalerFeaturesOffsets;
    /// Vector of scaling factors - (num_features).
    VectorXd scalerFeaturesScaleFactors;
    /// Scaled surrogate data matrix - (num_features by num_samples).
    MatrixXd scaledFeatures;


};

/**
 * \brief Normalizes the data using max and min feature
 * values.
 * 
 * if (mean_normalization):
 *   scaler_offsets = mean
 * else:
 *   scaler_offsets = min
 * 
 * scale_factors = (max - min)/norm_factor
 *
 * Setting mean_normalization = false scales each feature to [0,1].
 */
class NormalizationScaler: public DataScaler {
  public:
    NormalizationScaler();

    ~NormalizationScaler();

/**
 * \brief Main constructor for NormalizationScaler
 *
 * \param[in] features           Unscaled data matrix - (num_features by num_samples).
 * \param[in] mean_normalization Flag for whether to use mean or min value
 *                               as the offset.
 * \param[in] norm_factor        Optional scaling factor applied to each feature.
 *                               Has a default value of 1.0 .
 */
    NormalizationScaler(const MatrixXd &features, const bool mean_normalization, 
                        const Real norm_factor = 1.0);
};

/**
 * \brief Standardizes the data so the each feature has zero mean and
 * unit variance.
 * 
 * 
 * scaler_offsets = mean
 * 
 * scale_factors = standard_deviation/norm_factor
 */
class StandardizationScaler: public DataScaler {
  public:
    StandardizationScaler();

    ~StandardizationScaler();

/**
 * \brief Main constructor for StandardizationScaler
 *
 * \param[in] features           Unscaled data matrix - (num_features by num_samples).
 * \param[in] norm_factor        Optional scaling factor applied to each feature.
 *                               Has a default value of 1.0 .
 */
    StandardizationScaler(const MatrixXd &features, const Real norm_factor = 1.0);
};

/**
 * \brief Leaves the data unscaled.
 *
 * This DataScaler has fixed coefficients that amount to an identity operation.
 * It is useful when the data has already been scaled or scaling is desired.
 * 
 * scaler_offsets = 0.0
 * 
 * scale_factors = 1.0
 */
class NoScaler: public DataScaler {
  public:
    NoScaler();

    ~NoScaler();

    NoScaler(const MatrixXd &features);
/**
 * \brief Main constructor for NoScaler.
 *
 * \param[in] features           Unscaled data matrix - (num_features by num_samples).
 */
};


}  // namespace Surrogates

#endif  // include guard
