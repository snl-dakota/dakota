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

Python::Python(const std::string& module_name) :
  moduleFilename(module_name),
  ownPython(false),
  pyModuleActive(false)
{
  initialize_python();
}


Python::Python(const MatrixXd& samples,
               const MatrixXd& response,
               const std::string& module_name) :
  moduleFilename(module_name),
  ownPython(false),
  pyModuleActive(false)
{
  initialize_python();
  build(samples, response);
}


void Python::initialize_python()
{
  ownPython = false;
  pyModuleActive = false;
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
  if (!pyModuleActive) {
    pyModule = py::module_::import(moduleFilename.c_str());
    pyModuleActive = true;
  }
}

void Python::build(const MatrixXd& samples,
                   const MatrixXd& response)
{
  assert( pyModuleActive );
  assert( Py_IsInitialized() );

  // Hard-coded method for now; could expose to user - RWH
  const std::string fn_name("construct");
  py::function callback_fn = pyModule.attr(fn_name.c_str());
  pyModuleActive = true;
  callback_fn(samples, response);
}


VectorXd Python::value(const MatrixXd& eval_points,
                       const int qoi) {
  assert( pyModuleActive );
  assert( Py_IsInitialized() );

  /* Surrogate models don't yet support multiple responses */
  silence_unused_args(qoi);
  assert(qoi == 0);

  // Hard-coded method for now; could expose to user - RWH
  const std::string fn_name("predict");
  py::function callback_fn = pyModule.attr(fn_name.c_str());
  return callback_fn(eval_points).cast<VectorXd>();
}


MatrixXd Python::gradient(const MatrixXd& eval_points,
                          const int qoi) {
  assert( pyModuleActive );
  assert( Py_IsInitialized() );

  /* Surrogate models don't yet support multiple responses */
  silence_unused_args(qoi);
  assert(qoi == 0);

  // Hard-coded method for now; could expose to user - RWH
  const std::string fn_name("gradient");
  py::function callback_fn = pyModule.attr(fn_name.c_str());
  return callback_fn(eval_points).cast<MatrixXd>();
}


MatrixXd Python::hessian(const MatrixXd& eval_point,
                         const int qoi) {
  assert( pyModuleActive );
  assert( Py_IsInitialized() );

  silence_unused_args(eval_point);
  silence_unused_args(qoi);
  assert(qoi == 0);
  throw(std::runtime_error("hessian is not currently supported."));
}

}  // namespace surrogates
}  // namespace dakota

//BOOST_CLASS_EXPORT_IMPLEMENT(dakota::surrogates::Python)
