#include <boost/config.hpp>
#include <boost/dll.hpp>
#include <pybind11/embed.h>

#include "TextbookPythonPlugin.hpp"

namespace py = pybind11;
namespace DP = DakotaPlugins;

using Array1D = std::vector<double>;
using Array2D = std::vector<std::vector<double>>;

DP::EvalResponse TextbookPythonPlugin::evaluate(DP::EvalRequest const& request) {

  DP::EvalResponse response;
  resize_response_arrays(request, response);

  auto const& asv = request.activeSet;
  size_t const num_fns = asv.size();

  py::dict py_request = pack_python_request<py::array>(request);
  py::module_ textbook = py::module_::import("textbook");

  if (asv[0] & 1) {
    response.functions[0] =
        textbook.attr("function_value")(py_request).cast<double>();
  }
  if (asv[0] & 2) {
    response.gradients[0] =
        textbook.attr("function_gradient")(py_request).cast<Array1D>();
  }
  if (asv[0] & 4) {
    response.hessians[0] =
        textbook.attr("function_hessian")(py_request).cast<Array2D>();
  }

  if (num_fns == 3) {
    int constraint_idx = -1;
    for (int i = 1; i < num_fns; ++i) {
      constraint_idx = i - 1;
      std::string constraint_str =
          "constraint_" + std::to_string(constraint_idx);

      if (asv[i] & 1) {
        response.functions[i] =
            textbook.attr((constraint_str + "_value").c_str())(py_request)
            .cast<double>();
      }
      if (asv[i] & 2) {
        response.gradients[i] =
            textbook.attr((constraint_str + "_gradient").c_str())(py_request)
            .cast<Array1D>();
      }
      if (asv[i] & 4) {
        response.hessians[i] =
            textbook.attr((constraint_str + "_hessian").c_str())
            (py_request).cast<Array2D>();
      }

    }
  }

  return response;

}

extern "C" BOOST_SYMBOL_EXPORT TextbookPythonPlugin dakota_interface_plugin;
TextbookPythonPlugin dakota_interface_plugin;
