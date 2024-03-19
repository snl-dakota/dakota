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
  // Consider adding meaningful parameters... RWH
  configOptions.set("verbosity", 1, "console output verbosity");

  ownPython = false;
  pyModuleActive = false;
  if (!Py_IsInitialized()) {
    py::initialize_interpreter();
    ownPython = true;
    if (!Py_IsInitialized())
      throw(std::runtime_error(
            "Error: Could not initialize Python for surrogates use."));
  }
  if (!pyModuleActive) {
    try {
      py::object pyModule = py::module_::import(moduleFilename.c_str());
      pySurrogate = pyModule.attr("Surrogate")(); // hard-coded python class name
    }
    catch(py::error_already_set &e) {
      if (e.matches(PyExc_ModuleNotFoundError)) {
        std::cout << "Could not load the required module '"
                  << moduleFilename << "'" << std::endl;
        //throw; // Do we want to throw this again? RWH
      }
    }
    pyModuleActive = true;
  }

  // Check that needed methods exist in the module
  // ... We could allow the user to register these... RWH
  std::vector<std::string> req_attrs = { "construct", "predict" };
  bool is_module_valid = true;
  for( auto const & req_at : req_attrs ) {
    try {
      py::object py_fn = pySurrogate.attr(req_at.c_str());
    }
    catch(py::error_already_set &e) {
      if (e.matches(PyExc_AttributeError)) {
        std::cout << "Module '" << moduleFilename << "' does not "
          << "contain required method '" << req_at << "'"
          << std::endl;
      }
      is_module_valid = false;;
    }
  }
  if( !is_module_valid )
    throw(std::runtime_error("Invalid python module for surrogates"));
}

void Python::build(const MatrixXd& samples,
                   const MatrixXd& response)
{
  assert( pyModuleActive );
  assert( Py_IsInitialized() );

  verbosity = configOptions.get<int>("verbosity");

  if (verbosity > 0) {
    if (verbosity == 1) {
      std::cout << "\nBuilding Python surrogate\n\n";
    } else if (verbosity == 2) {
      std::cout << "\nBuilding Python surrogate with module.method\n"
                << moduleFilename << "." << "construct" << "\n";
    } else
      throw(
          std::runtime_error("Invalid verbosity int for Python surrogate"));
  }
  // Hard-coded method for now; could expose to user - RWH
  const std::string fn_name("construct");
  py::object py_surr_builder = pySurrogate.attr(fn_name.c_str());
  py_surr_builder(samples, response);
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
  py::object py_surr_eval = pySurrogate.attr(fn_name.c_str());

  return py_surr_eval(eval_points).cast<VectorXd>();
}


MatrixXd Python::gradient(const MatrixXd& eval_points,
                          const int qoi) {
  assert( pyModuleActive );
  assert( Py_IsInitialized() );

  /* Surrogate models don't yet support multiple responses */
  silence_unused_args(qoi);
  assert(qoi == 0);

  // Hard-coded method for now; could expose to user - RWH
  // We could add a check for this method (attribute) above in the
  // req_attrs if we knew it was needed at the time of our construction.
  const std::string fn_name("gradient");
  py::object py_surr_grad;
  try {
    py_surr_grad = pySurrogate.attr(fn_name.c_str());
  }
  catch(py::error_already_set &e) {
    if (e.matches(PyExc_AttributeError)) {
      std::cout << "Module '" << moduleFilename << "' does not "
        << "contain required method '" << fn_name << "'"
        << std::endl;
      throw; // Do we want to throw this again? RWH
    }
  }

  return py_surr_grad(eval_points).cast<MatrixXd>();
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
