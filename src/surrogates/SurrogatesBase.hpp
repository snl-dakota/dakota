/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_BASE_HPP
#define DAKOTA_SURROGATES_BASE_HPP

#include "UtilDataScaler.hpp"
#include "util_data_types.hpp"

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/assume_abstract.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <fstream>


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
  Surrogate(const Teuchos::ParameterList& param_list);

  /**
   * \brief Constructor for the Surrogate that sets configOptions
   *        and builds the GP.
   * \param[in] samples Matrix of data for surrogate construction - (num_samples by num_features)
   * \param[in] response Vector of targets for surrogate construction - (num_samples by num_qoi = 1; only 1 response is supported currently).
   * \param[in] param_list List that overrides entries in defaultConfigOptions
   */
  Surrogate(const MatrixXd &samples, const MatrixXd &response,
            const Teuchos::ParameterList &param_list);

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
   *  \param[out] value Values of the Surrogate at the prediction
   *  points - (num_pts by num_qoi) 
   */
  virtual void value(const MatrixXd &samples, MatrixXd &value) = 0;

  /* DTS: Should be able to make this non-virtual and keep in base class */
  /* Motivation for this call is pybind11 */
  virtual MatrixXd value(const MatrixXd &samples) = 0;

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

  /* DTS: Should be able to make this non-virtual and keep in base class */
  /* Motivation for this call is pybind11 */
  virtual MatrixXd gradient(const MatrixXd &samples, int qoi);

  /**
   *  \brief Evaluate the Hessian of the Surrogate at a single point.
   *  \param[in] samples Coordinates of the prediction point - (num_samples by num_features).
   *  \param[out] hessian Hessian matrix at the prediction point - 
   *  (num_features by num_features).
   *  \param[in] qoi Index of the quantity of interest for Hessian evaluation - 
   *  0 for scalar-valued surrogates.
   */
  virtual void hessian(const MatrixXd &samples, MatrixXd &hessian, int qoi);

  /* DTS: Should be able to make this non-virtual and keep in base class */
  /* Motivation for this call is pybind11 */
  virtual MatrixXd hessian(const MatrixXd &samples, int qoi);

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
  util::DataScaler dataScaler;

  /// serialize Surrogate (derived type only) to file
  template<typename DerivedSurr>
  static void save(const DerivedSurr& surr_out, const std::string& outfile,
		   const bool binary);

  /// serialize Surrogate (derived type only) from file
  template<typename DerivedSurr>
  static void load(const std::string& infile, const bool binary,
		   DerivedSurr& pr4);

  /// Evalute metrics at specified points (within surrogates)
  VectorXd evaluate_metrics(const StringArray &mnames,
                            const MatrixXd &points,
                            const MatrixXd &ref_values);

  /// Evalute metrics at specified points (from Dakota)
  RealMatrix evaluate_metrics(const StringArray &mnames,
                              const RealMatrix &points,
                              const RealMatrix &ref_values);

  /// Perform K-folds cross-validation (within surrogates)
  VectorXd cross_validate(const MatrixXd &samples,
    const MatrixXd &response, const StringArray &mnames,
    const int num_folds = 5, const int seed = 20);

  /// Perform K-folds cross-validation (from Dakota)
  RealMatrix cross_validate(const RealMatrix &samples,
    const RealMatrix &response, const StringArray &mnames,
    const int num_folds = 5, const int seed = 20);

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

  // BMA: Could instead use virtual copy constructor idiom
  /// clone derived Surrogate class for use in cross-validation
  virtual std::shared_ptr<Surrogate> clone() const = 0;


private:

  /// Allow serializers access to private class data
  friend class boost::serialization::access;
  /// Serializer for base class data (call from dervied with base_object)
  template<class Archive>
  void serialize(Archive& archive, const unsigned int version);

};


template<typename DerivedSurr>
void Surrogate::save(const DerivedSurr& surr_out, const std::string& outfile,
		     const bool binary)
{
  if (binary) {
    std::ofstream model_ostream(outfile, std::ios::out|std::ios::binary);
    if (!model_ostream.good())
      throw std::runtime_error("Failure opening model file '" + outfile +
			       "' for binary save.");

    boost::archive::binary_oarchive output_archive(model_ostream);
    output_archive << surr_out;
    std::cout << "Model saved to binary file '" << outfile << "'."
	      << std::endl;
  }
  else {
    std::ofstream model_ostream(outfile, std::ios::out);
    if (!model_ostream.good())
      throw std::runtime_error("Failure opening model file '" + outfile +
			       "' for save.");
    boost::archive::text_oarchive output_archive(model_ostream);
    output_archive << surr_out;
    std::cout << "Model saved to text file '" << outfile << "'."
	      << std::endl;
  }
}


template<typename DerivedSurr>
void Surrogate::load(const std::string& infile, const bool binary,
		     DerivedSurr& surr_in)
{
  if (binary) {
    std::ifstream model_istream(infile, std::ios::in|std::ios::binary);
    if (!model_istream.good())
      throw std::string("Failure opening model file for load.");

    boost::archive::binary_iarchive input_archive(model_istream);
    input_archive >> surr_in;
    std::cout << "Model loaded from binary file '" << infile << "'."
	      << std::endl;
  }
  else {
    std::ifstream model_istream(infile, std::ios::in);
    if (!model_istream.good())
      throw std::string("Failure opening model file for load.");

    boost::archive::text_iarchive input_archive(model_istream);
    input_archive >> surr_in;
    std::cout << "Model loaded from text file." << std::endl;
  }
}


template<class Archive>
void Surrogate::serialize(Archive& archive, const unsigned int version)
{
  // For future extension such as archiving final config options (will
  // require writing a serializer that maps to ParameterList
  // serialization)
  archive & dataScaler;
  archive & numSamples;
  archive & numVariables;
  //archive & configOptions;
}

} // namespace surrogates
} // namespace dakota


BOOST_SERIALIZATION_ASSUME_ABSTRACT(dakota::surrogates::Surrogate)

#endif // include guard
