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

std::shared_ptr<MatrixXd> DataScaler::scale_samples(const MatrixXd &unscaled_samples) {
  const int num_features = unscaled_samples.cols();
  if (num_features != scalerFeaturesOffsets->size()) {
    throw(std::runtime_error("scaleSamples input is not consistent."
          "Number of features does not match."));
  }
  const int num_samples = unscaled_samples.rows();
  MatrixXd scaledSamples(num_samples,num_features);
  for (int j = 0; j < num_features; j++) {
    if( check_for_zero_scaler_factor(j) )
      for (int i = 0; i < num_samples; i++)
        scaledSamples(i,j) = unscaled_samples(i,j)  - (*scalerFeaturesOffsets)(j);
    else
      for (int i = 0; i < num_samples; i++)
        scaledSamples(i,j) = (unscaled_samples(i,j) - (*scalerFeaturesOffsets)(j))/(*scalerFeaturesScaleFactors)(j);
  }
  return std::make_shared<MatrixXd>(scaledSamples);
}

NormalizationScaler::NormalizationScaler(){}

NormalizationScaler::~NormalizationScaler(){}

NormalizationScaler::NormalizationScaler(const MatrixXd &features, 
                                         const bool mean_normalization, const double norm_factor) {

  const int num_samples  = features.rows();
  const int num_features = features.cols();

  scalerFeaturesOffsets = std::make_shared<VectorXd>(num_features);
  scalerFeaturesScaleFactors = std::make_shared<VectorXd>(num_features);
  scaledFeatures = std::make_shared<MatrixXd>(num_samples,num_features);

  double min_val, max_val, mean_val;
  
  for (int j = 0; j < num_features; j++) {
    min_val = features.col(j).minCoeff();
    max_val = features.col(j).maxCoeff();
    mean_val = features.col(j).mean();
    (*scalerFeaturesOffsets)(j) = (mean_normalization) ? mean_val : min_val;
    (*scalerFeaturesScaleFactors)(j) = (max_val - min_val)/norm_factor;
    if( check_for_zero_scaler_factor(j) )
      for (int i = 0; i < num_samples; i++)
        (*scaledFeatures)(i,j) = features(i,j)  - (*scalerFeaturesOffsets)(j);
    else
      for (int i = 0; i < num_samples; i++)
        (*scaledFeatures)(i,j) = (features(i,j) - (*scalerFeaturesOffsets)(j)) / (*scalerFeaturesScaleFactors)(j);
  }

  has_scaling = true;
}


StandardizationScaler::StandardizationScaler(){}

StandardizationScaler::~StandardizationScaler(){}

StandardizationScaler::StandardizationScaler(const MatrixXd &features, 
                                             const double norm_factor) {

  const int num_samples = features.rows();
  const int num_features = features.cols();

  scalerFeaturesOffsets = std::make_shared<VectorXd>(num_features);
  scalerFeaturesScaleFactors = std::make_shared<VectorXd>(num_features);
  scaledFeatures = std::make_shared<MatrixXd>(num_samples,num_features);

  double mean_val, var_val;
  
  for (int j = 0; j < num_features; j++) {
    mean_val = features.col(j).mean();
    var_val = ((features.col(j).array() - mean_val).pow(2.0)).mean();
    (*scalerFeaturesOffsets)(j) = mean_val;
    (*scalerFeaturesScaleFactors)(j) = std::sqrt(var_val)/norm_factor;
    if( check_for_zero_scaler_factor(j) )
      for (int i = 0; i < num_samples; i++)
        (*scaledFeatures)(i,j) = features(i,j) - (*scalerFeaturesOffsets)(j);
    else
      for (int i = 0; i < num_samples; i++)
        (*scaledFeatures)(i,j) = (features(i,j) - (*scalerFeaturesOffsets)(j))/(*scalerFeaturesScaleFactors)(j);      
  }

  has_scaling = true;
}

NoScaler::NoScaler(){}

NoScaler::~NoScaler(){}

NoScaler::NoScaler(const MatrixXd &features) {

  const int num_features = features.cols();
  scaledFeatures = std::make_shared<MatrixXd>(features);
  scalerFeaturesOffsets = std::make_shared<VectorXd>(num_features);
  scalerFeaturesScaleFactors = std::make_shared<VectorXd>(num_features);

  has_scaling = true;
}

bool DataScaler::check_for_zero_scaler_factor(int index) {
  double value     = std::abs((*scalerFeaturesScaleFactors)(index));
  double near_zero = std::abs(100.0*std::numeric_limits<double>::min());
  return value < near_zero;
}

std::shared_ptr<DataScaler> scaler_factory(SCALER_TYPE scaler_type, const MatrixXd & unscaled_matrix) {
  if(scaler_type == util::SCALER_TYPE::STANDARDIZATION) {
    return std::make_shared<util::StandardizationScaler>(unscaled_matrix);
  } else if(scaler_type == util::SCALER_TYPE::NORMALIZATION) {
    return std::make_shared<util::NormalizationScaler>(unscaled_matrix, true); //TODO: Mean normalization should be an option
  }
  return std::make_shared<util::NoScaler>(unscaled_matrix);
}

}  // namespace util
}  // namespace dakota
