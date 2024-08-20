/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

/** \file
    Python module wrapping surrogates modules
 */

#include <pybind11/eigen.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "SurrogatesGaussianProcess.hpp"
#include "SurrogatesPolynomialRegression.hpp"

namespace py = pybind11;

using dakota::ParameterList;

/// Convert Python dictionary to options list
ParameterList convert_options(pybind11::dict pydict) {
  ParameterList pl;
  for (const auto& di : pydict) {
    auto key = di.first.cast<std::string>();
    auto value = di.second;

    // basic data types
    if (pybind11::isinstance<pybind11::bool_>(value))
      pl.set(key, value.cast<bool>());
    else if (pybind11::isinstance<pybind11::int_>(value))
      pl.set(key, value.cast<int>());
    else if (pybind11::isinstance<pybind11::float_>(value))
      pl.set(key, value.cast<double>());
    else if (pybind11::isinstance<pybind11::str>(value))
      pl.set(key, value.cast<std::string>());
    // for nested ParameterLists
    else if (pybind11::isinstance<pybind11::dict>(value)) {
      pl.set(key, convert_options(value.cast<pybind11::dict>()));
    }
    // NumPy array to Eigen MatrixXd
    else if (pybind11::isinstance<pybind11::array>(value))
      pl.set(key, value.cast<Eigen::MatrixXd>());
    else
      throw std::runtime_error(
          "dict2optslist: key '" + key + "' has unknown type: " +
          pybind11::str(value.get_type()).cast<std::string>());
  }
  return pl;
}

#if 0

/// Extend PolynomialRegression with a new type for Python
/** Explore idea of extension as a way to specialize
    constructors. Permits mapping datatypes for any member functions
    or constructors that differ, while leaving most
    untouched. Downside is requires new class for each surrogates
    class. */
class PyPolyReg: public dakota::surrogates::PolynomialRegression
{
public:

  PyPolyReg():
    dakota::surrogates::PolynomialRegression()
  { }

  /// ctor that accepts a dictionary
  PyPolyReg(const pybind11::dict& pydict):
    dakota::surrogates::PolynomialRegression(convert_options(pydict))
  { }

  /// ctor that accepts a dictionary
  PyPolyReg(const Eigen::MatrixXd& samples,
	    const Eigen::MatrixXd& response,
	    const pybind11::dict& pydict):
    dakota::surrogates::PolynomialRegression(samples, response,
					     convert_options(pydict))
  { }

  /// Example workaround for default Eigen pass-by-copy semantics
  Eigen::MatrixXd value(const Eigen::MatrixXd& eval_points)
  {
    /*
    Eigen::MatrixXd approx_values;
    PolynomialRegression::value(eval_points, approx_values);
    return approx_values;
    */
    return dakota::surrogates::Surrogate::value(eval_points);
  }
};
#endif

/// Define a Python module that wraps a few surrogates classes
/// Module name is really generic due to overly simple Python
///  packaging scheme we're using
PYBIND11_MODULE(dak_surrogates, m) {
  // free functions in module to save and load through base pointer
  m.def("save", static_cast<void (*)(
                    const std::shared_ptr<dakota::surrogates::Surrogate>&,
                    const std::string&, const bool)>(
                    &dakota::surrogates::Surrogate::save));

  m.def("load", static_cast<std::shared_ptr<dakota::surrogates::Surrogate> (*)(
                    const std::string&, const bool)>(
                    &dakota::surrogates::Surrogate::load));

  py::class_<dakota::surrogates::Surrogate,
             std::shared_ptr<dakota::surrogates::Surrogate>>(m, "Surrogate")
      // no init since this is a virtual base

      // This works, but doesn't yield an object of derived type
      //    .def(py::init([](const std::string& filename, bool binary)
      //	  {
      //	    return dakota::surrogates::Surrogate::load(filename,
      // binary);
      //	  }),
      //	  py::arg("filename"), py::arg("binary")
      //	)

      // qoi index 0
      .def("value", py::detail::overload_cast_impl<const Eigen::MatrixXd&>()(
                        &dakota::surrogates::Surrogate::value))

      .def("value", py::detail::overload_cast_impl<const Eigen::MatrixXd&>()(
                        &dakota::surrogates::Surrogate::value))

      // qoi index 0
      .def("gradient", py::detail::overload_cast_impl<const Eigen::MatrixXd&>()(
                           &dakota::surrogates::Surrogate::gradient))

      .def("gradient",
           py::detail::overload_cast_impl<const Eigen::MatrixXd&, int>()(
               &dakota::surrogates::Surrogate::gradient))

      // qoi index 0
      .def("hessian", py::detail::overload_cast_impl<const Eigen::MatrixXd&>()(
                          &dakota::surrogates::Surrogate::hessian))

      .def("hessian",
           py::detail::overload_cast_impl<const Eigen::MatrixXd&, int>()(
               &dakota::surrogates::Surrogate::hessian))

      .def("variable_labels",
           py::detail::overload_cast_impl<>()(
               &dakota::surrogates::Surrogate::variable_labels, py::const_))

      .def("variable_labels",
           py::detail::overload_cast_impl<const std::vector<std::string>&>()(
               &dakota::surrogates::Surrogate::variable_labels))

      .def("response_labels",
           py::detail::overload_cast_impl<>()(
               &dakota::surrogates::Surrogate::response_labels, py::const_))

      .def("response_labels",
           py::detail::overload_cast_impl<const std::vector<std::string>&>()(
               &dakota::surrogates::Surrogate::response_labels))

      .def("print_options",
           (&dakota::surrogates::Surrogate::print_options))

      ;  // Surrogate

  py::class_<dakota::surrogates::PolynomialRegression,
             std::shared_ptr<dakota::surrogates::PolynomialRegression>,
             dakota::surrogates::Surrogate>(m, "PolynomialRegression")
      .def(py::init<>())

      .def(py::init([](const pybind11::dict& d) {
        return dakota::surrogates::PolynomialRegression(convert_options(d));
      }))
      .def(py::init([](const Eigen::MatrixXd& samples,
                       const Eigen::MatrixXd& response,
                       const pybind11::dict& d) {
        return dakota::surrogates::PolynomialRegression(samples, response,
                                                        convert_options(d));
      }))

      .def(py::init([](const std::string& filename, bool binary) {
             return std::dynamic_pointer_cast<
                 dakota::surrogates::PolynomialRegression>(
                 dakota::surrogates::Surrogate::load(filename, binary));
           }),
           py::arg("filename"), py::arg("binary"))

      ;  // PolynomialRegression

  py::class_<dakota::surrogates::GaussianProcess,
             std::shared_ptr<dakota::surrogates::GaussianProcess>,
             dakota::surrogates::Surrogate>(m, "GaussianProcess")

      .def(py::init<>())

      .def(py::init([](const pybind11::dict& d) {
        return dakota::surrogates::GaussianProcess(convert_options(d));
      }))

      .def(py::init([](const Eigen::MatrixXd& samples,
                       const Eigen::MatrixXd& response,
                       const pybind11::dict& d) {
        return dakota::surrogates::GaussianProcess(samples, response,
                                                   convert_options(d));
      }))

      .def(py::init([](const std::string& filename, bool binary) {
             return std::dynamic_pointer_cast<
                 dakota::surrogates::GaussianProcess>(
                 dakota::surrogates::Surrogate::load(filename, binary));
           }),
           py::arg("filename"), py::arg("binary"))

      // qoi index 0
      .def("variance", py::detail::overload_cast_impl<const Eigen::MatrixXd&>()(
                           &dakota::surrogates::GaussianProcess::variance))

      .def("variance",
           py::detail::overload_cast_impl<const Eigen::MatrixXd&, int>()(
               &dakota::surrogates::GaussianProcess::variance))

      .def("objective_function_history", (&dakota::surrogates::GaussianProcess::
                                              get_objective_function_history))

      .def("objective_gradient_history", (&dakota::surrogates::GaussianProcess::
                                              get_objective_gradient_history))

      .def("theta_history",
           (&dakota::surrogates::GaussianProcess::get_theta_history))

      ;  // GaussianProcess
}
