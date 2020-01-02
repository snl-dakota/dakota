/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DataScaler.hpp"

namespace dakota {
namespace util {

DataScaler::DataScaler(){}

DataScaler::~DataScaler(){}

MatrixXd DataScaler::scaleSamples(const MatrixXd &unscaled_samples) {
  const int num_features = unscaled_samples.rows();
  if (num_features != scalerFeaturesOffsets.size()) {
    throw(std::runtime_error("scaleSamples input is not consistent."
          "Number of features does not match."));
  }
  const int num_samples = unscaled_samples.cols();
  MatrixXd scaledSamples(num_features, num_samples);
  for (int i = 0; i < num_features; i++) {
    for (int j = 0; j < num_samples; j++) {
      scaledSamples(i,j) = (unscaled_samples(i,j) - scalerFeaturesOffsets(i))/
                            scalerFeaturesScaleFactors(i);
    }
  }
  return scaledSamples;
}

NormalizationScaler::NormalizationScaler(){}

NormalizationScaler::~NormalizationScaler(){}

NormalizationScaler::NormalizationScaler(const MatrixXd &features, 
                                         const bool mean_normalization, const double norm_factor) {

  const int num_features = features.rows();
  const int num_samples = features.cols();

  scalerFeaturesOffsets.resize(num_features);
  scalerFeaturesScaleFactors.resize(num_features);
  scaledFeatures.resize(num_features,num_samples);

  double min_val, max_val, mean_val;
  
  for (int i = 0; i < num_features; i++) {
    min_val = features.row(i).minCoeff();
    max_val = features.row(i).maxCoeff();
    mean_val = features.row(i).mean();
    scalerFeaturesOffsets(i) = (mean_normalization) ? mean_val : min_val;
    scalerFeaturesScaleFactors(i) = (max_val - min_val)/norm_factor;
    for (int j = 0; j < num_samples; j++) {
      scaledFeatures(i,j) = (features(i,j) - scalerFeaturesOffsets(i))/
                            scalerFeaturesScaleFactors(i);
   }
  }

  has_scaling = true;
}


StandardizationScaler::StandardizationScaler(){}

StandardizationScaler::~StandardizationScaler(){}

StandardizationScaler::StandardizationScaler(const MatrixXd &features, 
                                             const double norm_factor) {

  const int num_features = features.rows();
  const int num_samples = features.cols();

  scalerFeaturesOffsets.resize(num_features);
  scalerFeaturesScaleFactors.resize(num_features);
  scaledFeatures.resize(num_features,num_samples);

  double mean_val, var_val;
  
  for (int i = 0; i < num_features; i++) {
    mean_val = features.row(i).mean();
    var_val = ((features.row(i).array() - mean_val).pow(2.0)).mean();
    scalerFeaturesOffsets(i) = mean_val;
    scalerFeaturesScaleFactors(i) = std::sqrt(var_val)/norm_factor;
    for (int j = 0; j < num_samples; j++) {
      scaledFeatures(i,j) = (features(i,j) - scalerFeaturesOffsets(i))/
                            scalerFeaturesScaleFactors(i);
   }
  }

  has_scaling = true;
}

NoScaler::NoScaler(){}

NoScaler::~NoScaler(){}

NoScaler::NoScaler(const MatrixXd &features) {
  const int num_features = features.rows();
  const int num_samples = features.cols();
  scaledFeatures = features;

  scalerFeaturesOffsets.resize(num_features);
  scalerFeaturesScaleFactors.resize(num_features);
  scalerFeaturesOffsets.setZero();
  scalerFeaturesScaleFactors.setOnes();
  has_scaling = true;
}


}  // namespace util
}  // namespace dakota
