/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaExtPythonMethod.hpp"
#include "ProblemDescDB.hpp"

using namespace pybind11::literals; // to bring in the `_a` literal

namespace Dakota {

ExtPythonMethod::ExtPythonMethod() :
  Iterator(std::shared_ptr<ExtPythonTraits>(new ExtPythonTraits()))
{ }


ExtPythonMethod::ExtPythonMethod(ProblemDescDB& problem_db,
                                       ParallelLibrary& parallel_lib,
                                       std::shared_ptr<Dakota::Model> model):
  Iterator(problem_db, parallel_lib, std::shared_ptr<TraitsBase>(new ExtPythonTraits())),
  ownPython(false),
  py11Active(false)
{
  moduleAndClassName = problem_db.get_string("method.class_path_and_name");
  initialize_python();

  iteratedModel = model;
  update_from_model(*iteratedModel);
}

// -----------------------------------------------------------------

void ExtPythonMethod::initialize_python()
{
  if (!Py_IsInitialized()) {
    py::initialize_interpreter();
    ownPython = true;
    if (Py_IsInitialized()) {
      if (outputLevel >= NORMAL_OUTPUT)
	Cout << "Python interpreter initialized for direct function evaluation."
	     << std::endl;
    }
    else {
      Cerr << "Error: Could not initialize Python for direct function "
	   << "evaluation." << std::endl;
      abort_handler(-1);
    }
  }

  if( !py11Active )
  {
    try {
      size_t p = moduleAndClassName.find_last_of(".");
      if( std::string::npos == p ) {
        Cerr << "Invalid method python module_and_class_name."
             << "\n\tUse \"module.classname\"";
        abort_handler(-1);
      }
      auto module_name = moduleAndClassName.substr(0,p);
      auto method_name = moduleAndClassName.substr(p+1);

      py::object module = py::module_::import(module_name.c_str());
      pyMethod = module.attr(method_name.c_str())();
    }
    catch(py::error_already_set &e) {
      if (e.matches(PyExc_ModuleNotFoundError)) {
        Cerr << "Could not load the required module '"
             << moduleAndClassName << "'" << std::endl;
        abort_handler(-1);
      }
      else {
        Cerr << "Caught a python exception:\n"
             << e.what() << std::endl;
        abort_handler(-1);
      }
    }
    py11Active = true;
  }

  // Check that needed method(s) exist in the module
  std::vector<std::string> req_attrs = { "core_run" };
  bool is_module_valid = true;
  for( auto const & req_at : req_attrs ) {
    try {
      py::object py_fn = pyMethod.attr(req_at.c_str());
    }
    catch(py::error_already_set &e) {
      if (e.matches(PyExc_AttributeError)) {
        std::cerr << "Module '" << moduleAndClassName << "' does not "
          << "contain required method '" << req_at << "'"
          << std::endl;
      }
      is_module_valid = false;;
    }
  }
  if( !is_module_valid ) {
      Cerr << "Error: Invalid python method module." << std::endl;
      abort_handler(-1);
  }

}

// -----------------------------------------------------------------

void ExtPythonMethod::initialize_run()
{
  Iterator::initialize_run();
}

// -----------------------------------------------------------------

void ExtPythonMethod::core_run()
{
  //initialize_variables_and_constraints();

  assert( py11Active );
  assert( Py_IsInitialized() );

  ModelExecutor executor = ModelExecutor(iteratedModel);

  try  {

    ShortArray fnASV = {1};
    py::list asv = copy_array_to_pybind11<py::list,ShortArray,int>(fnASV);

    const RealVector& cv_initial_points = ModelUtils::continuous_variables(*iteratedModel);
    const RealVector& cv_lower_bounds = ModelUtils::continuous_lower_bounds(*iteratedModel);
    const RealVector& cv_upper_bounds = ModelUtils::continuous_upper_bounds(*iteratedModel);
    py::list initial_pts  = copy_array_to_pybind11<py::list>(cv_initial_points);
    py::list lower_bounds = copy_array_to_pybind11<py::list>(cv_lower_bounds);
    py::list upper_bounds = copy_array_to_pybind11<py::list>(cv_upper_bounds);

    py::dict kwargs = py::dict(
        "variables"_a             = iteratedModel->current_variables().cv() /* numVars */,
        "initial_values"_a        = initial_pts,
        "lower_bounds"_a          = lower_bounds,
        "upper_bounds"_a          = upper_bounds,
        "asv"_a                   = asv,
        "functions"_a             = ModelUtils::response_size(*iteratedModel)
        );

    // Call the method
    const std::string fn_name("core_run");
    py::object py_run = pyMethod.attr(fn_name.c_str());
    py::dict ret_val = py_run(kwargs, executor);

    // Extract the single scalar result and best parameter
    if (ret_val.contains("fns")) {
      int i=0;
      auto best_x = ret_val["best_x"].cast<std::vector<double>>();
      auto values = ret_val["fns"].cast<std::vector<double>>();
      for (auto const & f : values) {
	Cout << "ExtPythonMethod::core_run(), best_f --> " << f
             << " at best_x = " << best_x
             << std::endl;
      }
    }
  }
  catch (const std::exception& e) {
    // (py::error_already_set is caught here too)
    std::string err_msg("Error evaluating Python method ");
    err_msg += moduleAndClassName + ":\n";
    err_msg += e.what();
    Cerr << err_msg << std::endl;
    abort_handler(-1);
  }
  catch (...) {
    std::string err_msg("Error evaluating Python method ");
    err_msg += moduleAndClassName;
    Cerr << err_msg << std::endl;
    abort_handler(-1);
  }

} // core_run

// -----------------------------------------------------------------
// Model Executor
// -----------------------------------------------------------------

ModelExecutor::ModelExecutor(std::shared_ptr<Model> & model) :
  model_(model)
{ }

std::vector<double>
ModelExecutor::value(std::vector<double>& x)
{
  Real result = 0;
  for (int i=0; i<x.size(); ++i)
    model_->current_variables().continuous_variable(x[i], i);
  model_->evaluate();

  const Response& test_resp = model_->current_response();
  std::vector<double> resp(test_resp.num_functions());
  for (int i=0; i<resp.size(); ++i)
    resp[i] = test_resp.function_value(i);

  //Cout << "ModelExecutor::value : x = " << x << ", f = " << resp << std::endl;
  return resp;
}

PYBIND11_MODULE(ext_method, m) {

  // Executor wrapper
  py::class_<Dakota::ModelExecutor>(m, "Executor")
    .def("function_value", &Dakota::ModelExecutor::value
         , "Return function value for passed parameter value"
         , py::arg("x"))
    ;
}

} // namespace Dakota

