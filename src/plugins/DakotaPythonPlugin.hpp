/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_PYTHON_PLUGIN_INTERFACE_H
#define DAKOTA_PYTHON_PLUGIN_INTERFACE_H

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include "DakotaInterfaceAPI.hpp"

namespace py = pybind11;
using namespace pybind11::literals;

namespace DakotaPlugins {

class DakotaPythonPlugin : public DakotaInterfaceAPI {

public:

  DakotaPythonPlugin() {};
  virtual ~DakotaPythonPlugin() { finalize(); };

  void initialize() override; 
  void finalize() override;

  void set_python_function(std::string const& py_module_fn_str);

  DakotaPlugins::EvalResponse evaluate(
      DakotaPlugins::EvalRequest const& request) override;

  std::vector<DakotaPlugins::EvalResponse>
      evaluate(std::vector<DakotaPlugins::EvalRequest> const& requests)
      override;

protected:

  py::function python_function;

  template<typename RetT, typename T>
  RetT copy_vector_to_pybind11(std::vector<T> const& src) const {
    return py::cast(src);
  }

  template<typename py_arrayT>
      py::dict pack_python_request(
      EvalRequest const& request) const {

    py_arrayT cv  =
        copy_vector_to_pybind11<py_arrayT, double>(request.continuousVars);
    py_arrayT div =
        copy_vector_to_pybind11<py_arrayT, int>(request.discreteIntVars);
    py_arrayT dsv =
        copy_vector_to_pybind11<py_arrayT, std::string>(
        request.discreteStringVars);
    py_arrayT drv = copy_vector_to_pybind11<py_arrayT, double>(
        request.discreteRealVars);

    py_arrayT all_var_labels =
        copy_vector_to_pybind11<py_arrayT, std::string>(
        request.inputOrderedLabels);
    py_arrayT cv_labels =
        copy_vector_to_pybind11<py_arrayT, std::string>(
        request.continuousLabels);
    py_arrayT  div_labels =
        copy_vector_to_pybind11<py_arrayT, std::string>(
        request.discreteIntLabels);
    py_arrayT  dsv_labels =
        copy_vector_to_pybind11<py_arrayT, std::string>(
        request.discreteStringLabels);
    py_arrayT  drv_labels =
        copy_vector_to_pybind11<py_arrayT, std::string>(
        request.discreteRealLabels);

    py_arrayT asv  = copy_vector_to_pybind11<py_arrayT, short>(
        request.activeSet);
    py_arrayT dvv  = copy_vector_to_pybind11<py_arrayT, size_t>(
        request.derivativeVars);

    py::int_ eval_id = py::cast(request.functionEvalId);

    py::dict python_request = py::dict(
        // TODO: "variables"
        // TODO: "functions"
        // TODO: "metadata"
        // TODO: "function_labels"
        // TODO: "metadata_labels"
        "cv"_a = cv,
        "div"_a = div,
        "dsv"_a = dsv,
        "drv"_a = drv,
        "variable_labels"_a = all_var_labels,
        "cv_labels"_a = cv_labels,
        "div_labels"_a = div_labels,
        "dsv_labels"_a = dsv_labels,
        "drv_labels"_a = drv_labels,
        "asv"_a = asv,
        "dvv"_a = dvv,
        "eval_id"_a = eval_id);

    return python_request;
  }

  void unpack_python_response(const std::vector<short>& active_set,
      size_t const num_derivs,
      pybind11::dict const& py_response,
      DakotaPlugins::EvalResponse& response);

  bool expect_derivative(const std::vector<short>& active_set,
      const short deriv_type) const;

  bool ownPython = false;

};

}

#endif
