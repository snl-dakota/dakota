/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_BASE_HPP
#define DAKOTA_SURROGATES_BASE_HPP

#include "UtilDataScaler.hpp"
#include "util_data_types.hpp"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/codecvt_null.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/math/special_functions/nonfinite_num_facets.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <fstream>
#include <locale>

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
  /// Constructor that uses defaultConfigOptions and does not build.
  Surrogate();

  /**
   * \brief Constructor that sets configOptions but does not build.
   * \param[in] param_list List that overrides entries in defaultConfigOptions.
   */
  Surrogate(const ParameterList& param_list);

  /**
   * \brief Constructor for the Surrogate that sets configOptions
   *  and builds the surrogate (does nothing in the base class).
   * \param[in] samples Matrix of data for surrogate construction - (num_samples
   * by num_features). \param[in] response Vector of targets for surrogate
   * construction - (num_samples by num_qoi = 1; only 1 response is supported
   * currently). \param[in] param_list List that overrides entries in
   * defaultConfigOptions.
   */
  Surrogate(const MatrixXd& samples, const MatrixXd& response,
            const ParameterList& param_list);

  /// Default destructor.
  virtual ~Surrogate();

  /**
   * \brief Build the Surrogate using specified build data.
   *
   * \param[in] samples Matrix of data for surrogate construction - (num_samples
   * by num_features). \param[in] response Vector of responses/targets for
   * surrogate construction - (num_samples by num_qoi = 1).
   */
  virtual void build(const MatrixXd& samples, const MatrixXd& response) = 0;

  /**
   *  \brief Evaluate the Surrogate at a set of prediction points for a scalar
   * QoI. \param[in] eval_points Matrix of prediction points - (num_pts by
   * num_features). \returns Values of
   * the Surrogate at the prediction points - (num_pts).
   */
  virtual VectorXd value(const MatrixXd& eval_points);

  /**
   *  \brief Evaluate the Surrogate at a set of prediction points for field QoIs.
   *  \param[in] eval_points Vector of prediction points - (num_features).
   *  \returns Values of the Surrogate at the prediction points - (num_pts).
   */
  virtual VectorXd values(const MatrixXd& eval_points);

  /**
   *  \brief Evaluate the gradient of the scalar Surrogate at a set of prediction
   * points. \param[in] eval_points Matrix of prediction points - (num_pts by
   * num_features). \returns Matrix of
   * gradient vectors at the prediction points - (num_pts by num_features).
   */
  virtual MatrixXd gradient(const MatrixXd& eval_points);

  /**
   *  \brief Evaluate the gradient of the field Surrogate at a set of prediction
   * points. \param[in] eval_points Matrix of prediction points
   * - (num_pts by num_features). \returns Matrix of gradient vectors at the
   * prediction points - (num_pts by num_features).
   */
  MatrixXd gradients(const MatrixXd& eval_points);

  /**
   *  \brief Evaluate the Hessian of the scalar Surrogate at a set of prediction
   *  points. \param[in] eval_point Coordinates of the prediction point - (1 by
   * num_features). \returns Hessian matrix at the
   * prediction point - (num_features by num_features).
   */
  virtual MatrixXd hessian(const MatrixXd& eval_point);

  /**
   *  \brief Evaluate the Hessian of the field Surrogate at a set of prediction
   *  points.  \param[in] eval_point Coordinates of the prediction point - (1 by
   * num_features). \returns Hessian matrix at the
   * prediction point - (num_features by num_features).
   */
  virtual MatrixXd hessians(const MatrixXd& eval_point);

  /**
      \brief Set the variable/feature names
      \param[in] var_labels Vector of strings, one per input variable
   */
  void variable_labels(const std::vector<std::string>& var_labels);

  /**
      \brief Get the (possibly empty) variable/feature names
      \returns Vector of strings, one per input variable; empty if not set
   */
  const std::vector<std::string>& variable_labels() const;

  /**
      \brief Set the response/QoI names
      \param[in] resp_labels Vector of strings, one per surrogate response
   */
  void response_labels(const std::vector<std::string>& resp_labels);

  /**
      \brief Get the (possibly empty) response/QoI names
      \returns Vector of strings, one per surrogate response; empty if not set
   */
  const std::vector<std::string>& response_labels() const;

  /**
   *  \brief Set the Surrogate's configOptions.
   *  \param[in] options ParameterList of configuration options.
   */
  void set_options(const ParameterList& options);

  /**
   *  \brief Get the Surrogate's configOptions.
   *  \param[out] options ParameterList of configuration options.
   */
  void get_options(ParameterList& options);

  /// Print the Surrogate's configOptions.
  void print_options();

  /// Initialize the Surrogate's defaultConfigOptions.
  virtual void default_options() = 0;

  /// DataScaler class for a Surrogate's build samples.
  util::DataScaler dataScaler;

  /// Response offset
  double responseOffset = 0.;

  /// Response scale factor
  double responseScaleFactor = 1.;

  /// serialize Surrogate to file (typically through
  /// shared_ptr<Surrogate>, but Derived& or Derived* okay too)
  template <typename SurrHandle>
  static void save(const SurrHandle& surr_out, const std::string& outfile,
                   const bool binary);

  /// serialize Surrogate from file (typically through
  /// shared_ptr<Surrogate>, but Derived& or Derived* okay too)
  template <typename SurrHandle>
  static void load(const std::string& infile, const bool binary,
                   SurrHandle& surr_in);

  // member variant of save to save as shared_ptr(*this)
  // could enable if desired, but might require shared_from_this?
  // void save(const std::string& outfile, const bool binary);

  /// serialize Surrogate from file through pointer to base class
  /// (must have been saved via same data type)
  static std::shared_ptr<Surrogate> load(const std::string& infile,
                                         const bool binary);

  // also demo load via ctor
  //  Surrogate(infile, binary)

  // Allow diagnostics enable/disable
  virtual bool diagnostics_available() {
    return true;
  }

  /// Evalute metrics at specified points (within surrogates)
  VectorXd evaluate_metrics(const StringArray& mnames, const MatrixXd& points,
                            const MatrixXd& ref_values);

  /// Perform K-folds cross-validation (within surrogates)
  VectorXd cross_validate(const MatrixXd& samples, const MatrixXd& response,
                          const StringArray& mnames, const int num_folds = 5,
                          const int seed = 20);

 protected:
  /// Number of samples in the Surrogate's build samples.
  int numSamples;
  /// Number of features/variables in the Surrogate's build samples.
  int numVariables;
  /// Names of the variables/features; need not be populated
  std::vector<std::string> variableLabels;
  /**
   *  \brief Number of quantities of interest predicted by the surrogate. For
   *  scalar-valued surrogates numQOI = 1.
   */
  int numQOI;
  /// Names of the responses/QoIs; need not be populated
  std::vector<std::string> responseLabels;
  /// Default Key/value options to configure the surrogate.
  ParameterList defaultConfigOptions;
  /// Key/value options to configure the surrogate - will override
  /// defaultConfigOptions.
  ParameterList configOptions;

  // BMA: Could instead use virtual copy constructor idiom
  /// clone derived Surrogate class for use in cross-validation
  virtual std::shared_ptr<Surrogate> clone() const = 0;

 private:
  /// Allow serializers access to private class data
  friend class boost::serialization::access;
  /// Serializer for base class data (call from dervied with base_object)
  template <class Archive>
  void serialize(Archive& archive, const unsigned int version);
};

/**
 * \brief Serialize a derived (i.e. non-base) surrogate model.
 * \param[in] surr_out Surrogate to seralize.
 * \param[in] outfile Name of the output text or binary file.
 * \param[in] binary Flag for binary or text format.
 */
template <typename DerivedSurr>
void Surrogate::save(const DerivedSurr& surr_out, const std::string& outfile,
                     const bool binary) {
  if (binary) {
    std::ofstream model_ostream(outfile, std::ios::out | std::ios::binary);
    if (!model_ostream.good())
      throw std::runtime_error("Failure opening model file '" + outfile +
                               "' for binary save.");

    boost::archive::binary_oarchive output_archive(model_ostream);
    output_archive << surr_out;
    std::cout << "Model saved to binary file '" << outfile << "'." << std::endl;
  } else {
    std::ofstream model_ostream(outfile, std::ios::out);
    if (!model_ostream.good())
      throw std::runtime_error("Failure opening model file '" + outfile +
                               "' for save.");

    // enable portable write/read of nan/inf, per
    // https://www.boost.org/doc/libs/1_58_0/libs/math/example/nonfinite_serialization_archives.cpp
    std::locale default_locale(std::locale::classic(),
                               new boost::archive::codecvt_null<char>);
    std::locale nonfinite_locale(default_locale,
                                 new boost::math::nonfinite_num_put<char>);
    model_ostream.imbue(nonfinite_locale);
    boost::archive::text_oarchive output_archive(model_ostream,
                                                 boost::archive::no_codecvt);

    output_archive << surr_out;
    std::cout << "Model saved to text file '" << outfile << "'." << std::endl;
  }
}

/**
 * \brief Load a derived (i.e. non-base) surrogate model.
 * \param[in] infile Filename for serialized surrogate.
 * \param[in] binary Flag for binary or text format.
 * \param[in] surr_in Derived surrogate class to be populated with serialized
 * data.
 */
template <typename DerivedSurr>
void Surrogate::load(const std::string& infile, const bool binary,
                     DerivedSurr& surr_in) {
  if (binary) {
    std::ifstream model_istream(infile, std::ios::in | std::ios::binary);
    if (!model_istream.good())
      throw std::string("Failure opening model file for load.");

    boost::archive::binary_iarchive input_archive(model_istream);
    input_archive >> surr_in;
    std::cout << "Model loaded from binary file '" << infile << "'."
              << std::endl;
  } else {
    std::ifstream model_istream(infile, std::ios::in);
    if (!model_istream.good())
      throw std::string("Failure opening model file for load.");

    // enable portable write/read of nan/inf, per
    // https://www.boost.org/doc/libs/1_58_0/libs/math/example/nonfinite_serialization_archives.cpp
    std::locale default_locale(std::locale::classic(),
                               new boost::archive::codecvt_null<char>);
    std::locale nonfinite_locale(default_locale,
                                 new boost::math::nonfinite_num_get<char>);
    model_istream.imbue(nonfinite_locale);
    boost::archive::text_iarchive input_archive(model_istream,
                                                boost::archive::no_codecvt);
    input_archive >> surr_in;
    std::cout << "Model loaded from text file." << std::endl;
  }
}

template <class Archive>
void Surrogate::serialize(Archive& archive, const unsigned int version) {
  silence_unused_args(version);
  // For future extension such as archiving final config options (will
  // require writing a serializer that maps to ParameterList
  // serialization)
  archive& dataScaler;
  archive& numSamples;
  archive& numVariables;
  archive& variableLabels;
  archive& responseLabels;
  archive& responseOffset;
  archive& responseScaleFactor;
  // archive & configOptions;
}

}  // namespace surrogates
}  // namespace dakota

BOOST_CLASS_EXPORT_KEY(dakota::surrogates::Surrogate)
BOOST_SERIALIZATION_ASSUME_ABSTRACT(dakota::surrogates::Surrogate)

#endif  // include guard
