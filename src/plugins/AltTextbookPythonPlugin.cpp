#include <boost/config.hpp>
#include <boost/dll.hpp>
#include <pybind11/embed.h>

#include "AltTextbookPythonPlugin.hpp"

namespace py = pybind11;
namespace DP = DakotaPlugins;

using Array1D = std::vector<double>;
using Array2D = std::vector<std::vector<double>>;
using Array3D = std::vector<std::vector<std::vector<double>>>;

DP::EvalResponse AltTextbookPythonPlugin::evaluate(
    DP::EvalRequest const& request) {

  DP::EvalResponse response;
  resize_response_arrays(request, response);

  py::dict py_request = pack_python_request<py::array>(request);
  py::module_ textbook = py::module_::import("textbook");

  size_t const num_fns = request.activeSet.size();
  size_t const num_derivs = request.derivativeVars.size();

  py::dict py_response = textbook.attr("text_book_dict")(py_request);
  unpack_python_response(num_fns, num_derivs, py_response, response);

  return response;

}

// May want to move this up to the parent class, but API for the
// send and return dicts is not consistent with Python direct
// interface at the moment.
void AltTextbookPythonPlugin::unpack_python_response(
    size_t const num_fns,
    size_t const num_derivs,
    py::dict const& py_response,
    DP::EvalResponse& response) {


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

extern "C" BOOST_SYMBOL_EXPORT AltTextbookPythonPlugin dakota_interface_plugin;
AltTextbookPythonPlugin dakota_interface_plugin;
