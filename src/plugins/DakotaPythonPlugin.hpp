/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
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

  void initialize() override {
    std::cout << "Dynamically loading " << PLUGIN_PYTHON_LIB << std::endl;
    boost::dll::shared_library python_lib(PLUGIN_PYTHON_LIB,
        boost::dll::load_mode::rtld_global);

    py::initialize_interpreter();
    py::print("DakotaPythonPlugin: interpreter is live.");
  }

  void finalize() override {
    py::print("DakotaPythonPlugin: interpreter shutting down.");
    py::finalize_interpreter();
  }

protected:

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
        "cv"_a = cv,
        "div"_a = div,
        "dsv"_a = dsv,
        "drv"_a = drv,
        "cv_labels"_a = cv_labels,
        "div_labels"_a = div_labels,
        "dsv_labels"_a = dsv_labels,
        "drv_labels"_a = drv_labels,
        "asv"_a = asv,
        "dvv"_a = dvv,
        "eval_id"_a = eval_id);

    return python_request;
  }

private:

  template<typename RetT, typename T>
  RetT copy_vector_to_pybind11(std::vector<T> const& src) const {
    return py::cast(src);
  }

};

}

#endif
