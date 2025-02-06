/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <boost/config.hpp>
#include <boost/dll.hpp>
#include <pybind11/embed.h>

#include "DakotaPythonPlugin.hpp"

namespace py = pybind11;

namespace DakotaPlugins {

using Array1D = std::vector<double>;
using Array2D = std::vector<std::vector<double>>;
using Array3D = std::vector<std::vector<std::vector<double>>>;

void DakotaPythonPlugin::initialize() {
  std::cout << "Dynamically loading " << PLUGIN_PYTHON_LIB << std::endl;
  boost::dll::shared_library python_lib(PLUGIN_PYTHON_LIB,
      boost::dll::load_mode::rtld_global);

  if (!Py_IsInitialized()) {
    py::initialize_interpreter();
    py::print("DakotaPythonPlugin: interpreter is live.");
    ownPython = true;
  }

  // DTS: assuming a single analysis driver
  const size_t num_drivers = analysisDrivers.size();
  if (num_drivers != 1) {
    throw(std::runtime_error("Python plugin supports "
        "a single analysis driver -- " +
        std::to_string(num_drivers) + " were provided."));
  }

  set_python_function(analysisDrivers[0]);
}

void DakotaPythonPlugin::finalize() {
  if (ownPython && Py_IsInitialized()) {
    py::print("DakotaPythonPlugin: interpreter shutting down.");
    py::finalize_interpreter();
    ownPython = false;
  }
}

void DakotaPythonPlugin::set_python_function(std::string const& py_module_fn_str) {
  const size_t sep_pos = py_module_fn_str.find(":");
  std::string const py_module_name = py_module_fn_str.substr(0, sep_pos);
  std::string const py_function_name = py_module_fn_str.substr(sep_pos + 1);

  py::module_ py_module = py::module_::import(py_module_name.c_str());
  python_function = py_module.attr(py_function_name.c_str());
}

DakotaPlugins::EvalResponse DakotaPythonPlugin::evaluate(
    DakotaPlugins::EvalRequest const& request) {

  DakotaPlugins::EvalResponse response;
  resize_response_arrays(request, response);

  py::dict py_request = pack_python_request<py::array>(request);
  py::dict py_response = python_function(py_request);

  size_t const num_derivs = request.derivativeVars.size();
  unpack_python_response(request.activeSet, num_derivs, py_response, response);

  return response;
}

std::vector<DakotaPlugins::EvalResponse> DakotaPythonPlugin::evaluate(
    std::vector<DakotaPlugins::EvalRequest> const& requests) {

  size_t const num_requests = requests.size();
  std::vector<DakotaPlugins::EvalResponse> responses(num_requests);
  py::list py_requests;

  for (size_t i = 0; i < num_requests; ++i) {
    resize_response_arrays(requests[i], responses[i]);
    py_requests.append(pack_python_request<py::array>(requests[i]));
  }

  py::list py_responses = python_function(py_requests);

  for (size_t i = 0; i < num_requests; ++i) {
    size_t const num_derivs = requests[i].derivativeVars.size();
    unpack_python_response(requests[i].activeSet, num_derivs, py_responses[i], responses[i]);
  }

  return responses;
}

void DakotaPythonPlugin::unpack_python_response(
    const std::vector<short>& active_set,
    size_t const num_derivs,
    py::dict const& py_response,
    DakotaPlugins::EvalResponse& response) {

  size_t num_fns = active_set.size();

  if (expect_derivative(active_set, 1)) {
    if (py_response.contains("fns")) {
      auto values = py_response["fns"].cast<Array1D>();
      if (values.size() != num_fns) {
        throw(std::runtime_error("Python Plugin [\"fns\"]: "
                                 "incorrect size for # of functions"));
      }
      for (size_t i = 0; i < num_fns; ++i) {
        response.functions[i] = values[i];
      }
    }
    else {
      throw(std::runtime_error("Python Plugin: required key "
			       "[\"fns\"] absent in dict returned to Dakota"));
    }
  }

  if (expect_derivative(active_set, 2)) {
    if (py_response.contains("fnGrads")) {
      auto grads = py_response["fnGrads"].cast<Array2D>();
      if (grads.size() != num_fns) {
        throw(std::runtime_error("Python Plugin [\"fnGrads\"]: "
                                 "incorrect size for # of functions"));
      }
      for (size_t i = 0; i < num_fns; ++i) {
        if (grads[i].size() != num_derivs) {
          throw(std::runtime_error("Python Plugin [\"fnGrads\"]: "
                                   "gradient dimension != # of derivatives "
                                   "for response " + std::to_string(i)));
        }
        for (size_t j = 0; j < num_derivs; ++j) {
          response.gradients[i][j] = grads[i][j];
        }
      }
    }
    else {
      throw(std::runtime_error("Python Plugin: required key "
			       "[\"fnGrads\"] absent in dict returned to Dakota"));
    }
  }

  if (expect_derivative(active_set, 4)) {
    if (py_response.contains("fnHessians")) {
      auto hess = py_response["fnHessians"].cast<Array3D>();
      if (hess.size() != num_fns) {
        throw(std::runtime_error("Python Plugin [\"fnHessians\"]: "
                                 "incorrect size for # of functions"));
      }
      for (size_t i = 0; i < num_fns; ++i) {
        if (hess[i].size() != num_derivs) {
          throw(std::runtime_error(
              "Python Plugin [\"fnHessians\"]: "
              "Hessian # of rows != # of derivatives "
              "for response " + std::to_string(i)));
        }
        for (size_t j = 0; j < num_derivs; ++j) {
          if (hess[i][j].size() != num_derivs) {
            throw(std::runtime_error(
                "Python Plugin [\"fnHessians\"]: "
                "Hessian # of columns != # of derivatives "
                "for response " + std::to_string(i)));
          }
          for (size_t k = 0; k <= j; ++k) {
            response.hessians[i][j][k] = hess[i][j][k];
          }
        }
      }
    }
    else {
      throw(std::runtime_error("Python Plugin: required key "
			       "[\"fnHessians\"] absent in dict returned to Dakota"));
    }
  }

}


bool DakotaPythonPlugin::expect_derivative(const std::vector<short>& asv,
					   const short deriv_type) const
{
  return std::any_of(asv.begin(), asv.end(),
		     [deriv_type](short a){ return (a & deriv_type); });
}


extern "C" BOOST_SYMBOL_EXPORT DakotaPythonPlugin dakota_interface_plugin;
DakotaPythonPlugin dakota_interface_plugin;

}
