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

  py::initialize_interpreter();
  py::print("DakotaPythonPlugin: interpreter is live.");
}

void DakotaPythonPlugin::finalize() {
  py::print("DakotaPythonPlugin: interpreter shutting down.");
  py::finalize_interpreter();
}

void DakotaPythonPlugin::set_python_names(std::string const& py_module_fn_str) {
  const size_t sep_pos = py_module_fn_str.find(":");
  const size_t fn_str_length = py_module_fn_str.size() - sep_pos - 1;

  py_module_name = py_module_fn_str.substr(0, sep_pos);
  py_function_name = py_module_fn_str.substr(sep_pos + 1, fn_str_length);
}

DakotaPlugins::EvalResponse DakotaPythonPlugin::evaluate(
    DakotaPlugins::EvalRequest const& request) {

  DakotaPlugins::EvalResponse response;
  resize_response_arrays(request, response);

  // DTS: set somewhere else
  std::string const py_mod_fn_str = "textbook:text_book_dict";
  set_python_names(py_mod_fn_str);

  py::dict py_request = pack_python_request<py::array>(request);
  py::module_ textbook = py::module_::import(py_module_name.c_str());

  size_t const num_fns = request.activeSet.size();
  size_t const num_derivs = request.derivativeVars.size();

  py::dict py_response = textbook.attr(py_function_name.c_str())(py_request);
  unpack_python_response(num_fns, num_derivs, py_response, response);

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

  // DTS: set somewhere else
  std::string const py_mod_fn_str = "textbook:text_book_batch";
  set_python_names(py_mod_fn_str);

  py::module_ textbook = py::module_::import(py_module_name.c_str());
  py::list py_responses = textbook.attr(py_function_name.c_str())(py_requests);

  for (size_t i = 0; i < num_requests; ++i) {
    // DTS: these should be the same for all requests
    size_t const num_fns = requests[i].activeSet.size();
    size_t const num_derivs = requests[i].derivativeVars.size();
    unpack_python_response(num_fns, num_derivs, py_responses[i], responses[i]);
  }

  return responses;
}

// May want to move this up to the parent class, but API for the
// send and return dicts is not consistent with Python direct
// interface at the moment.
void DakotaPythonPlugin::unpack_python_response(
    size_t const num_fns,
    size_t const num_derivs,
    py::dict const& py_response,
    DakotaPlugins::EvalResponse& response) {


  for (auto item : py_response) {
    auto key = item.first.cast<std::string>();

    if (key == "fns") {
      auto values = item.second.cast<Array1D>();
      if (values.size() != num_fns) {
        throw(std::runtime_error("Python Plugin [\"fns\"]: "
                                 "incorrect size for # of functions"));
      }
      for (size_t i = 0; i < num_fns; ++i) {
        response.functions[i] = values[i];
      }
    }

    else if (key == "fnGrads") {
      auto grads = item.second.cast<Array2D>();
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

    else if (key == "fnHessians") {
      auto hess = item.second.cast<Array3D>();
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
  }
}


extern "C" BOOST_SYMBOL_EXPORT DakotaPythonPlugin dakota_interface_plugin;
DakotaPythonPlugin dakota_interface_plugin;
}
