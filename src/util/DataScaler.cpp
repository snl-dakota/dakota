/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DataScaler.hpp"

#include <boost/assign.hpp>
#include <boost/bimap.hpp>

namespace dakota {
namespace util {

using SCALER_TYPE = DataScaler::SCALER_TYPE;
using BimapScalertypeStr = boost::bimap<SCALER_TYPE, std::string>;

static BimapScalertypeStr type_name_bimap =
  boost::assign::list_of< BimapScalertypeStr::relation >
  (SCALER_TYPE::NONE, "none")
  (SCALER_TYPE::STANDARDIZATION, "standardization")
  (SCALER_TYPE::MEAN_NORMALIZATION, "mean normalization")
  (SCALER_TYPE::MINMAX_NORMALIZATION, "min-max normalization")
  ;


SCALER_TYPE DataScaler::scaler_type(const std::string& scaler_name)
{
  BimapScalertypeStr::right_const_iterator rc_iter
    = type_name_bimap.right.find(scaler_name);
  if (rc_iter == type_name_bimap.right.end()) {
    throw std::runtime_error("Invalid DataScaler scaler_name");
  }
  return rc_iter->second;
}


DataScaler::DataScaler() :
  hasScaling(false)
{ }

DataScaler::~DataScaler()
{ }

void DataScaler::scale_samples(const MatrixXd &unscaled_samples,
                               MatrixXd &scaled_samples) {
  const int num_features = unscaled_samples.cols();
  if (num_features != scalerFeaturesOffsets.size()) {
    throw(std::runtime_error("scaleSamples input is not consistent."
          "Number of features does not match."));
  }
  const int num_samples = unscaled_samples.rows();
  scaled_samples.resize(num_samples,num_features);
  for (int j = 0; j < num_features; j++) {
    if (check_for_zero_scaler_factor(j)) {
      for (int i = 0; i < num_samples; i++) {
        scaled_samples(i,j) = unscaled_samples(i,j)  - scalerFeaturesOffsets(j);
      }
    }
    else {
      for (int i = 0; i < num_samples; i++) {
        scaled_samples(i,j) = (unscaled_samples(i,j) - scalerFeaturesOffsets(j))/scalerFeaturesScaleFactors(j);
      }
    }
  }
}

std::shared_ptr<MatrixXd> DataScaler::scale_samples(const MatrixXd &unscaled_samples) {
  const int num_features = unscaled_samples.cols();
  if (num_features != scalerFeaturesOffsets.size()) {
    throw(std::runtime_error("scaleSamples input is not consistent."
          "Number of features does not match."));
  }
  const int num_samples = unscaled_samples.rows();
  MatrixXd scaledSamples(num_samples,num_features);
  for (int j = 0; j < num_features; j++) {
    if (check_for_zero_scaler_factor(j)) {
      for (int i = 0; i < num_samples; i++) {
        scaledSamples(i,j) = unscaled_samples(i,j)  - scalerFeaturesOffsets(j);
      }
    }
    else {
      for (int i = 0; i < num_samples; i++) {
        scaledSamples(i,j) = (unscaled_samples(i,j) - scalerFeaturesOffsets(j))/scalerFeaturesScaleFactors(j);
      }
    }
  }
  return std::make_shared<MatrixXd>(scaledSamples);
}

NormalizationScaler::NormalizationScaler(){}

NormalizationScaler::~NormalizationScaler(){}

NormalizationScaler::NormalizationScaler(const MatrixXd &features, 
                                         const bool mean_normalization, const double norm_factor) {

  const int num_samples  = features.rows();
  const int num_features = features.cols();

  scalerFeaturesOffsets.resize(num_features);
  scalerFeaturesScaleFactors.resize(num_features);
  scaledFeatures.resize(num_samples,num_features);

  double min_val, max_val, mean_val;
  
  for (int j = 0; j < num_features; j++) {
    min_val = features.col(j).minCoeff();
    max_val = features.col(j).maxCoeff();
    mean_val = features.col(j).mean();
    scalerFeaturesOffsets(j) = (mean_normalization) ? mean_val : min_val;
    scalerFeaturesScaleFactors(j) = (max_val - min_val)/norm_factor;
    if (check_for_zero_scaler_factor(j)) {
      for (int i = 0; i < num_samples; i++) {
        scaledFeatures(i,j) = features(i,j)  - scalerFeaturesOffsets(j);
      }
    }
    else {
      for (int i = 0; i < num_samples; i++) {
        scaledFeatures(i,j) = (features(i,j) - scalerFeaturesOffsets(j)) / scalerFeaturesScaleFactors(j);
      }
    }
  }

  hasScaling = true;
}


StandardizationScaler::StandardizationScaler(){}

StandardizationScaler::~StandardizationScaler(){}

StandardizationScaler::StandardizationScaler(const MatrixXd &features, 
                                             const double norm_factor) {

  const int num_samples = features.rows();
  const int num_features = features.cols();

  scalerFeaturesOffsets.resize(num_features);
  scalerFeaturesScaleFactors.resize(num_features);
  scaledFeatures.resize(num_samples,num_features);

  double mean_val, var_val;
  
  for (int j = 0; j < num_features; j++) {
    mean_val = features.col(j).mean();
    var_val = ((features.col(j).array() - mean_val).pow(2.0)).mean();
    scalerFeaturesOffsets(j) = mean_val;
    scalerFeaturesScaleFactors(j) = std::sqrt(var_val)/norm_factor;
    if( check_for_zero_scaler_factor(j) )
      for (int i = 0; i < num_samples; i++)
        scaledFeatures(i,j) = features(i,j) - scalerFeaturesOffsets(j);
    else
      for (int i = 0; i < num_samples; i++)
        scaledFeatures(i,j) = (features(i,j) - scalerFeaturesOffsets(j))/scalerFeaturesScaleFactors(j);      
  }

  hasScaling = true;
}

NoScaler::NoScaler(){}

NoScaler::~NoScaler(){}

NoScaler::NoScaler(const MatrixXd &features) {

  const int num_features = features.cols();
  scaledFeatures = features;
  scalerFeaturesOffsets = VectorXd::Zero(num_features);
  scalerFeaturesScaleFactors = VectorXd::Ones(num_features);

  hasScaling = true;
}

bool DataScaler::check_for_zero_scaler_factor(int index) {
  double value     = std::abs(scalerFeaturesScaleFactors(index));
  double near_zero = std::abs(100.0*std::numeric_limits<double>::min());
  return value < near_zero;
}

std::shared_ptr<DataScaler> scaler_factory(SCALER_TYPE scaler_type, const MatrixXd & unscaled_matrix) {
  if (scaler_type == util::SCALER_TYPE::STANDARDIZATION) {
    return std::make_shared<util::StandardizationScaler>(unscaled_matrix);
  }
  else if (scaler_type == util::SCALER_TYPE::MEAN_NORMALIZATION) {
    return std::make_shared<util::NormalizationScaler>(unscaled_matrix, true);
  }
  else if (scaler_type == util::SCALER_TYPE::MINMAX_NORMALIZATION) {
    return std::make_shared<util::NormalizationScaler>(unscaled_matrix, false); 
  }
  else if (scaler_type == util::SCALER_TYPE::NONE) {
    return std::make_shared<util::NoScaler>(unscaled_matrix); 
  }
  else {
    throw("Error: invalid data scaler type");
  }
}

}  // namespace util
}  // namespace dakota
