/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_PYTHON_HPP
#define DAKOTA_SURROGATES_PYTHON_HPP

#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include "SurrogatesBase.hpp"
#include "Teuchos_YamlParameterListCoreHelpers.hpp"
#include "UtilDataScaler.hpp"
#include "UtilLinearSolvers.hpp"
#include "util_data_types.hpp"

#include <boost/serialization/base_object.hpp>

namespace dakota {
namespace surrogates {
/**
 *  \brief The Python class constructs a surrogate via python and has
 *         it ready for Dakota use.
 */

class Python : public Surrogate {

 public:

  /**
   * \brief Constructor that sets moduleAndClassName and does not build.
   *
   * \param[in] module_and_class_name Name of python module file containing callback functions
   */
  Python(const std::string& module_and_class_name);

  /**
   * \brief Constructor sets moduleAndClassName and builds the python surrogate.
   *
   * \param[in] samples Matrix of data for surrogate construction - (num_samples
   * by num_features) \param[in] response Vector of targets for surrogate
   * construction - (num_samples by num_qoi = 1; only 1 response is supported
   * currently). \param[in] module_and_class_name Name of python module file
   * containing callback functions
   */
  Python(const MatrixXd& samples, const MatrixXd& response,
                       const std::string& module_and_class_name);

  /// Default destructor
  ~Python() { }

  /// Construct and populate the defaultConfigOptions.
  void default_options() override {
    return initialize_python();
  }

  // Allow disabling for field surrogates for now - RWH
  virtual bool diagnostics_available() override;

  /**
   * \brief Build the python surrogate using specified build data.
   *
   * \param[in] samples Matrix of data for surrogate construction - (num_samples
   * by num_features) \param[in] response Vector of targets for surrogate
   * construction - (num_samples by num_qoi = 1; only 1 response is supported
   * currently).
   */
  void build(const MatrixXd& samples, const MatrixXd& response) override;

  /**
   *  \brief Evaluate the scalar python surrogate at a set of prediction points.
   * \param[in] eval_points Matrix of prediction points - (num_pts
   * by num_features). \returns Values
   * of the python surrogate at the prediction points - (num_pts)
   */
  VectorXd value(const MatrixXd& eval_points) override;

  /**
   *  \brief Evaluate the field python surrogate at a set of prediction points.
   * \param[in] eval_points Matrix of prediction points - (num_pts
   * by num_features). \returns Values of the python surrogate at the
   * prediction points - (num_pts)
   */
  VectorXd values(const MatrixXd& eval_points) override;

  /**
   *  \brief Evaluate the gradient of the python scalar surrogate at a set of
   * prediction points. \param[in] eval_points Coordinates of
   * the prediction points - (num_pts by num_features). 
   * \returns Matrix of gradient
   * vectors at the prediction points - (num_pts by num_features).
   */
  MatrixXd gradient(const MatrixXd& eval_points) override;

  /**
   *  \brief Evaluate the Hessian of the python scalar surrogate at a single point.
   *  \param[in] eval_point Coordinates of the prediction point - (1 by
   * num_features). \returns Hessian matrix at the prediction point -
   *  (num_features by num_features).
   */
  MatrixXd hessian(const MatrixXd& eval_point) override;

  std::shared_ptr<Surrogate> clone() const override {
    return std::make_shared<Python>(moduleAndClassName);
  }

 private:

  // --------------- Python Setup --------------------

  /// Name of python callback module file
  std::string moduleAndClassName;

  /// true if this class created the interpreter instance
  bool ownPython;

  /// true if python callback module is valid
  bool pyModuleActive;

  /// python Surrogate class
  py::object pySurrogate;

  /// flag for field-based surrogates
  bool isField;


  // -------------------------------------------------

  /// Initialize python interpreter and callback module
  void initialize_python();

  /// Verbosity level.
  int verbosity;
};

}  // namespace surrogates
}  // namespace dakota

BOOST_CLASS_EXPORT_KEY(dakota::surrogates::Python)

#endif
