/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "SurrogatesPython.hpp"
#include "surrogates_tools.hpp"

namespace dakota {
namespace surrogates {

Python::Python(const ParameterList& param_list) {
  default_options();
  configOptions = param_list;
}


Python::Python(const MatrixXd& samples,
               const MatrixXd& response,
               const ParameterList& param_list)
{
  default_options();
  configOptions = param_list;
  build(samples, response);
}


void Python::build(const MatrixXd& samples,
                   const MatrixXd& response)
{
  // Hard-coded module and method for developement purposes - RWH
  const std::string dummy_spec("driver_surrogates:construct");
  if( !py11Active )
  {
    size_t pos = dummy_spec.find(":");
    std::string module_name = dummy_spec.substr(0,pos);
    std::string function_name = dummy_spec.substr(pos+1);

    //py::module_ module = py::module_::import(module_name.c_str());
    py::object module = py::module_::import(module_name.c_str());
    py::function callback_fn = module.attr(function_name.c_str());
    py11CallBack = callback_fn;
    py11Active = true;
  }
  assert( py11Active );
  assert( Py_IsInitialized() );
  //std::cout << "samples: " << samples << std::endl;
  py11CallBack(samples, response);
}


VectorXd Python::value(const MatrixXd& eval_points,
                       const int qoi)
{
  /* Surrogate models don't yet support multiple responses */
  silence_unused_args(qoi);
  assert(qoi == 0);

  const std::string dummy_spec("driver_surrogates:predict");
  size_t pos = dummy_spec.find(":");
  std::string module_name = dummy_spec.substr(0,pos);
  std::string function_name = dummy_spec.substr(pos+1);

  py::object module = py::module_::import(module_name.c_str());
  py::function callback_fn = module.attr(function_name.c_str());

  return callback_fn(eval_points).cast<VectorXd>();
}


void Python::default_options() {
  ownPython = false;
  py11Active = false;
  if (!Py_IsInitialized()) {
    py::initialize_interpreter();
    ownPython = true;
    if (Py_IsInitialized()) {
      //if (outputLevel >= NORMAL_OUTPUT)
      //  Cout << "Python interpreter initialized for surrogates use."
      //       << std::endl;
    }
    else {
      throw(std::runtime_error(
        "Error: Could not initialize Python for surrogates use."));
    }
  }
}

MatrixXd Python::gradient(const MatrixXd& eval_points,
                                        const int qoi) {
  /* Surrogate models don't yet support multiple responses */
  silence_unused_args(qoi);
  assert(qoi == 0);

  const std::string dummy_spec("driver_surrogates:gradient");
  size_t pos = dummy_spec.find(":");
  std::string module_name = dummy_spec.substr(0,pos);
  std::string function_name = dummy_spec.substr(pos+1);

  py::object module = py::module_::import(module_name.c_str());
  py::function callback_fn = module.attr(function_name.c_str());

  return callback_fn(eval_points).cast<MatrixXd>();
}

MatrixXd Python::hessian(const MatrixXd& eval_point,
                                       const int qoi) {
  silence_unused_args(eval_point);
  silence_unused_args(qoi);
  assert(qoi == 0);
  throw(std::runtime_error("hessian is not currently supported."));
}

}  // namespace surrogates
}  // namespace dakota

//BOOST_CLASS_EXPORT_IMPLEMENT(dakota::surrogates::Python)
