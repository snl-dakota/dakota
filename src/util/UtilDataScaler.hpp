/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_UTIL_DATA_SCALER_HPP
#define DAKOTA_UTIL_DATA_SCALER_HPP

#include "util_data_types.hpp"

#include <boost/serialization/serialization.hpp>
#include <memory>

namespace dakota {
namespace util {

/**
 *  \brief The DataScaler class computes the scaling coefficients and scales
 *         a 2D data matrix with dimensions num_samples by num_features.
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
  /// Enumeration for supported types of DataScalers
  enum class SCALER_TYPE {
    NONE,
    STANDARDIZATION,
    MEAN_NORMALIZATION,
    MINMAX_NORMALIZATION
  };

  DataScaler();

  virtual ~DataScaler();

  /**
   *  \brief Apply scaling to a set of unscaled samples
   *  \param[in] unscaled_samples Unscaled matrix of samples
   *  \param[out] scaled_samples Scaled matrix of samples
   */
  void scale_samples(const MatrixXd& unscaled_samples,
                     MatrixXd& scaled_samples);

  /**
   *  \brief Apply scaling to a set of unscaled samples
   *  \param[in] unscaled_samples Unscaled matrix of samples
   *  \returns MatrixXd scaled_samples Scaled matrix of samples
   */
  MatrixXd scale_samples(const MatrixXd& unscaled_samples) {
    MatrixXd scaled_samples;
    scale_samples(unscaled_samples, scaled_samples);
    return scaled_samples;
  }

  /**
   *  \brief Get the vector of offsets
   *  \returns Vector of scaler offsets - (num_features)
   */
  inline const VectorXd& get_scaler_features_offsets() const {
    return scalerFeaturesOffsets;
  }

  /**
   *  \brief Get the vector of scaling factors
   *  \returns Vector of scaling factors - (num_features)
   */
  inline const VectorXd& get_scaler_features_scale_factors() const {
    return scalerFeaturesScaleFactors;
  }

  /**
   *  \brief Checks an individual scaler feature scale factor for being close to
   * zero;  If it is near zero, we can potentially run into a divide-by-zero
   * error if not handled appropriately. \param[in] index The scaler feature
   * index to check \returns True if the value is near zero
   */
  bool check_for_zero_scaler_factor(int index);

  /**
   *  \brief Convert scaler name to enum type
   *  \param[in] scaler_name DataScaler name to map
   *  \returns Corresponding DataScaler enum
   */
  static SCALER_TYPE scaler_type(const std::string& scaler_name);

 protected:
  /**
   *  \brief Bool for whether or not the the scaling coefficients
   *   have been computed
   */
  bool hasScaling;

  /// Vector for a single scaled sample - (num_features); avoids resize memory
  /// allocs
  RowVectorXd scaledSample;

  /// Vector of offsets - (num_features)
  VectorXd scalerFeaturesOffsets;

  /// Vector of scaling factors - (num_features)
  VectorXd scalerFeaturesScaleFactors;

 private:
  /// Allow serializers access to private class data
  friend class boost::serialization::access;
  /// Serializer for base class data (call from dervied with base_object)
  template <class Archive>
  void serialize(Archive& archive, const unsigned int version);
};

template <class Archive>
void DataScaler::serialize(Archive& archive, const unsigned int version) {
  silence_unused_args(version);

  archive& hasScaling;
  archive& scalerFeaturesOffsets;
  archive& scalerFeaturesScaleFactors;
}

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
 * Setting mean_normalization = false scales each feature to [0,1]
 */
class NormalizationScaler : public DataScaler {
 public:
  NormalizationScaler();

  ~NormalizationScaler();

  /**
   * \brief Main constructor for NormalizationScaler
   *
   * \param[in] features           Unscaled data matrix - (num_samples by
   * num_features) \param[in] mean_normalization Flag for whether to use mean or
   * min value as the offset \param[in] norm_factor        Optional scaling
   * factor applied to each feature Has a default value of 1.0
   */
  NormalizationScaler(const MatrixXd& features, bool mean_normalization,
                      double norm_factor = 1.0);
};

/**
 * \brief Standardizes the data so the each feature has zero mean and unit
 * variance.
 *
 * scaler_offsets = mean
 *
 * scale_factors = standard_deviation/norm_factor
 */
class StandardizationScaler : public DataScaler {
 public:
  StandardizationScaler();

  ~StandardizationScaler();

  /**
   * \brief Main constructor for StandardizationScaler
   *
   * \param[in] features           Unscaled data matrix - (num_samples by
   * num_features) \param[in] norm_factor        Optional scaling factor applied
   * to each feature Has a default value of 1.0
   */
  StandardizationScaler(const MatrixXd& features, double norm_factor = 1.0);
};

/**
 * \brief Leaves the data unscaled
 *
 * This DataScaler has fixed coefficients that amount to an identity operation.
 * It is useful when the data has already been scaled or scaling is desired.
 *
 * scaler_offsets = 0.0
 *
 * scale_factors = 1.0
 */
class NoScaler : public DataScaler {
 public:
  NoScaler();

  /**
   * \brief Main constructor for NoScaler
   *
   * \param[in] features Unscaled data matrix - (num_samples by num_features)
   */
  NoScaler(const MatrixXd& features);

  ~NoScaler();
};

// BMA TODO: Discuss whether we benefit from shared_ptr here, since
// Boost serialization < 1.56 can't handle std::shared_ptr...
/**
 * \brief Free function to construct DataScaler
 *
 * \param[in] scaler_type      Which scaler to construct
 * \param[in] unscaled_matrix  Unscaled data matrix - (num_samples by
 * num_features) \returns  Shared pointer to a DataScaler
 */
std::shared_ptr<DataScaler> scaler_factory(DataScaler::SCALER_TYPE scaler_type,
                                           const MatrixXd& unscaled_matrix);

}  // namespace util
}  // namespace dakota

#endif  // include guard
