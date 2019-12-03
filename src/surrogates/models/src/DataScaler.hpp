/*  _______________________________________________________________________

    PECOS: Parallel Environment for Creation Of Stochastics
    Copyright (c) 2011, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Pecos directory.
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
 *         a 2D matrix of features.
 *
 *  There are currently 3 scaling options for the DataScaler class:
 *
 *  1. Standardization - transform each feature to have zero mean and unit
 *  variance.
 *
 *  2. Normalization - normalizies each feature uses the max and min
 *  value divivded by either the mean value (mean_normalization = true)
 *  or min value (mean_normalization = false)
 *  Also allows for a norm_factor scaling, required for the direct
 *  neural network.
 *
 *  3. No scaling - scaling coefficients amount to an identity operation
 */


class DataScaler {

  public: 
    DataScaler();

    ~DataScaler();

    VectorXd scaleFeatures(const VectorXd &x);
    MatrixXd scaleSamples(const MatrixXd &unscaled_samples);

    VectorXd getScalerFeaturesOffsets() {return scalerFeaturesOffsets;}
    VectorXd getScalerFeaturesScaleFactors() {return scalerFeaturesScaleFactors;}
    MatrixXd getScaledFeatures() {return scaledFeatures;}

    bool has_scaling = false;


  protected: 

    VectorXd scalerFeaturesOffsets;
    VectorXd scalerFeaturesScaleFactors;
    MatrixXd scaledFeatures;


};

class NormalizationScaler: public DataScaler {
  public:
    NormalizationScaler();

    ~NormalizationScaler();

    NormalizationScaler(const MatrixXd &features, const bool mean_normalization, 
                        const Real norm_factor = 1.0);
};

class StandardizationScaler: public DataScaler {
  public:
    StandardizationScaler();

    ~StandardizationScaler();

    StandardizationScaler(const MatrixXd &features, const Real norm_factor = 1.0);
};

class NoScaler: public DataScaler {
  public:
    NoScaler();

    ~NoScaler();

    NoScaler(const MatrixXd &features);
};


}  // namespace Surrogates

#endif  // include guard
