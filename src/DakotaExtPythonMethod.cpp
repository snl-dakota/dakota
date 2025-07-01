/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaExtPythonMethod.hpp"

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

  iteratedModel = model;
  update_from_model(*iteratedModel);
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

  // Hard-coded for now ... 
  const std::string dummy_ac_name("driver_text_book:demo_opt_fn");
  if( !py11Active )
  {
    size_t pos = dummy_ac_name.find(":");
    std::string module_name = dummy_ac_name.substr(0,pos);
    std::string function_name = dummy_ac_name.substr(pos+1);

    py::object module = py::module_::import(module_name.c_str());
    py11CallBack = module.attr(function_name.c_str());
    py11Active = true;
  }

  assert( py11Active );
  assert( Py_IsInitialized() );

  try  {

    ShortArray fnASV = {1};
    py::list asv = copy_array_to_pybind11<py::list,ShortArray,int>(fnASV);
    py::dict kwargs = py::dict(
        "variables"_a             = 1 /* numVars */,
        "asv"_a                   = asv,
        "functions"_a             = 1
        );

    // Call the method
    py::dict ret_val = py11CallBack(kwargs);

    // Exrtact the single scalar result and best parameter
    if (ret_val.contains("fns")) {
      int i=0;
      auto best_x = ret_val["best_x"].cast<std::vector<double>>();
      auto values = ret_val["fns"].cast<std::vector<double>>();
      for (auto const & f : values) {
	Cout << "ExtPythonMethod::core_run(), value --> " << f
             << " at best_x = " << best_x[i++]
             << std::endl;
      }
    }
  }
  catch (const std::runtime_error& e) {
    // (py::error_already_set is caught here too)
    std::string err_msg("Error evaluating Python analysis_driver ");
    err_msg += dummy_ac_name + ":\n";
    err_msg += e.what();
    Cerr << err_msg << std::endl;
    throw FunctionEvalFailure(err_msg);
  }
  catch (...) {
    std::string err_msg("Error evaluating Python analysis_driver ");
    err_msg += dummy_ac_name;
    Cerr << err_msg << std::endl;
    throw FunctionEvalFailure(err_msg);
  }

} // core_run

} // namespace Dakota

