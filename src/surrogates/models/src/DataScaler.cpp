/*  _______________________________________________________________________

    PECOS: Parallel Environment for Creation Of Stochastics
    Copyright (c) 2011, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Pecos directory.
    _______________________________________________________________________ */

// Two scalers:
// - Normalizing scaler - performs mean or max-min normalization
// - Standarization - makes each feature zero mean and unit variance
#include <DataScaler.hpp>

namespace Surrogates {

DataScaler::DataScaler(){}

DataScaler::~DataScaler(){}

VectorXd DataScaler::scaleFeatures(const VectorXd &unscaled_x) {
  int M = unscaled_x.size();
  VectorXd scaledInput(M);
  for (int i = 0; i < M; i++) {
    scaledInput(i) = (unscaled_x(i) - scalerFeaturesOffsets(i))/
                     scalerFeaturesScaleFactors(i);
  }
  return scaledInput;
}

MatrixXd DataScaler::scaleSamples(const MatrixXd &unscaled_samples) {
  const int num_features = unscaled_samples.rows();
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
                                         const bool mean_normalization, const Real norm_factor) {

  int M = features.rows();
  int K = features.cols();

  scalerFeaturesOffsets.resize(M);
  scalerFeaturesScaleFactors.resize(M);
  scaledFeatures.resize(M, K);

  Real min_val, max_val, mean_val;
  
  for (int i = 0; i < M; i++) {
    min_val = features.row(i).minCoeff();
    max_val = features.row(i).maxCoeff();
    mean_val = features.row(i).mean();
    scalerFeaturesOffsets(i) = (mean_normalization) ? mean_val : min_val;
    scalerFeaturesScaleFactors(i) = (max_val - min_val)/norm_factor;
    for (int j = 0; j < K; j++) {
      scaledFeatures(i,j) = (features(i,j) - scalerFeaturesOffsets(i))/
                            scalerFeaturesScaleFactors(i);
   }
  }

  has_scaling = true;
}


StandardizationScaler::StandardizationScaler(){}

StandardizationScaler::~StandardizationScaler(){}

StandardizationScaler::StandardizationScaler(const MatrixXd &features, 
                                             const Real norm_factor) {

  int M = features.rows();
  int K = features.cols();

  scalerFeaturesOffsets.resize(M);
  scalerFeaturesScaleFactors.resize(M);
  scaledFeatures.resize(M,K);

  Real mean_val, var_val;
  
  for (int i = 0; i < M; i++) {
    mean_val = features.row(i).mean();
    var_val = ((features.row(i).array() - mean_val).pow(2.0)).mean();
    scalerFeaturesOffsets(i) = mean_val;
    scalerFeaturesScaleFactors(i) = std::sqrt(var_val)/norm_factor;
    for (int j = 0; j < K; j++) {
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


}  // namespace Surrogates
