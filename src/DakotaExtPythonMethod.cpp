/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaExtPythonMethod.hpp"
#include "NonDSampling.hpp"
#include "ProblemDescDB.hpp"
#include "python_utils.hpp"

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
	Cout << "Python interpreter initialized for external methods."
	     << std::endl;
    }
    else {
      Cerr << "Error: Could not initialize Python for external methods."
	   << std::endl;
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
  for( auto const & req_at : req_attrs )
    is_module_valid = is_module_valid &&
                      PythonUtils::check_for_attr(pyMethod, req_at, moduleAndClassName);

  if( !is_module_valid ) {
      Cerr << "Error: Invalid python method module." << std::endl;
      abort_handler(-1);
  }

}

// -----------------------------------------------------------------

void ExtPythonMethod::initialize_run()
{
  // Create the callback executor (model wrapper)
  executor_ = std::make_shared<ModelExecutor>(iteratedModel);

  // Check and do if optional method exists
  std::string chk_attr = "initialize_run";
  if( PythonUtils::check_for_attr(pyMethod, chk_attr, moduleAndClassName) ) {
    py::object py_run = pyMethod.attr(chk_attr.c_str());
    py_run(*executor_);
  }
}

// -----------------------------------------------------------------

void ExtPythonMethod::pre_run()
{
  // Check and do if optional method exists
  std::string chk_attr = "pre_run";
  if( PythonUtils::check_for_attr(pyMethod, chk_attr, moduleAndClassName) ) {
    py::object py_run = pyMethod.attr(chk_attr.c_str());
    py_run(*executor_);
  }
}

// -----------------------------------------------------------------

void ExtPythonMethod::core_run()
{
  //initialize_variables_and_constraints();

  assert( py11Active );
  assert( Py_IsInitialized() );

  try  {

    ShortArray fnASV = {1};
    py::list asv = PythonUtils::copy_array_to_pybind11<py::list,ShortArray,int>(fnASV);

    // Call the method
    const std::string fn_name("core_run");
    py::object py_run = pyMethod.attr(fn_name.c_str());
    py::dict ret_val = py_run(*executor_);

    // A special case for an optimizer
    //     extract the single scalar result and best parameters
    if (ret_val.contains("fns") && ret_val.contains("best_x")) {
      int i=0;
      auto best_x = ret_val["best_x"].cast<std::vector<double>>();
      auto values = ret_val["fns"].cast<std::vector<double>>();
      StringMultiArrayConstView cv_labels =
        ModelUtils::continuous_variable_labels(*iteratedModel);
      for (auto const & f : values) {
        Cout << "ExtPythonMethod::core_run()\n"
             << "\tbest_f: " << f << ", at:" << std::endl;
        for( size_t i=0; i<cv_labels.size(); ++i )
          Cout << "\t" << cv_labels[i].data() << ": " << best_x[i] << std::endl;
      }
    }
    // For debugging output utility
    //if (ret_val.contains("fns")) {
    //  executor_.compute_and_print_moments(ret_val["fns"].cast<std::vector<std::vector<double>>>());
    //}
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

void ExtPythonMethod::post_run(std::ostream& s)
{
  // Check and do if optional method exists
  std::string chk_attr = "post_run";
  if( PythonUtils::check_for_attr(pyMethod, chk_attr, moduleAndClassName) ) {
    py::object py_run = pyMethod.attr(chk_attr.c_str());
    py_run(*executor_);
  }
}

// -----------------------------------------------------------------

void ExtPythonMethod::finalize_run()
{
  // Check and do if optional method exists
  std::string chk_attr = "finalize_run";
  if( PythonUtils::check_for_attr(pyMethod, chk_attr, moduleAndClassName) ) {
    py::object py_run = pyMethod.attr(chk_attr.c_str());
    py_run(*executor_);
  }
}


// -----------------------------------------------------------------
// Model Executor
// -----------------------------------------------------------------

ModelExecutor::ModelExecutor(std::shared_ptr<Model> & model) :
  model_(model)
{ }

// -----------------------------------------------------------------

std::vector<double>
ModelExecutor::value(std::vector<double>& x)
{
  for (int i=0; i<x.size(); ++i)
    ModelUtils::continuous_variable(*model_, x[i], i);
  model_->evaluate();

  const Response& test_resp = model_->current_response(); // No ModelUtils helper for Response's
  std::vector<double> resp(test_resp.num_functions());
  for (int i=0; i<resp.size(); ++i)
    resp[i] = test_resp.function_value(i);

  return resp;
}

// -----------------------------------------------------------------

std::vector<double>
ModelExecutor::value(py::dict & vars)
{
  if (vars.contains("cv")) {
    auto cv = vars["cv"].cast<std::vector<double>>();
    //Cout << "ModelExecutor::value: cv : " << cv << std::endl;
    for (int i=0; i<cv.size(); ++i)
      ModelUtils::continuous_variable(*model_, cv[i], i);
  }
  if (vars.contains("div")) {
    auto div = vars["div"].cast<std::vector<int>>();
    //Cout << "ModelExecutor::value: div : " << div << std::endl;
    for (int i=0; i<div.size(); ++i)
      ModelUtils::discrete_int_variable(*model_, div[i], i);
  }
  if (vars.contains("dsv")) {
    auto dsv = vars["dsv"].cast<std::vector<String>>();
    //Cout << "ModelExecutor::value: dsv : " << dsv << std::endl;
    for (int i=0; i<dsv.size(); ++i)
      ModelUtils::discrete_string_variable(*model_, dsv[i], i);
  }
  if (vars.contains("drv")) {
    auto drv = vars["drv"].cast<std::vector<Real>>();
    //Cout << "ModelExecutor::value: drv : " << drv << std::endl;
    for (int i=0; i<drv.size(); ++i)
      ModelUtils::discrete_real_variable(*model_, drv[i], i);
  }

  model_->evaluate();

  const Response& test_resp = model_->current_response(); // No ModelUtils helper for Response's
  std::vector<double> resp(test_resp.num_functions());
  for (int i=0; i<resp.size(); ++i)
    resp[i] = test_resp.function_value(i);

  return resp;
}

// -----------------------------------------------------------------

void
ModelExecutor::compute_and_print_moments(const std::vector<std::vector<double>> & resp)
{
  if( resp.empty() )
    return;

  RealMatrix resp_mat(resp[0].size(), resp.size());
  //copy_data(resp, resp_mat); // this flavor does not appear to exist - RWH
  for( size_t i=0; i<resp[0].size(); ++i )
    for( size_t j=0; j<resp.size(); ++j )
      resp_mat(i,j) = resp[j][i]; // need to transpose layout - RWH
  //Cout << resp_mat << std::endl;

  RealMatrix stats;
  NonDSampling::compute_moments(resp_mat, stats, Pecos::STANDARD_MOMENTS);

  NonDSampling::print_moments(Cout, stats, RealMatrix(), 
      "external python method response",
      Pecos::STANDARD_MOMENTS, ModelUtils::response_labels(*model_), false); 

  return;
}

// -----------------------------------------------------------------

PYBIND11_MODULE(ext_method, m) {

  // Executor evaluator helper
  py::class_<Dakota::ModelExecutor>(m, "Executor")
    .def("function_value"
         , static_cast<std::vector<double> (Dakota::ModelExecutor::*)(std::vector<double>&)>
                        (&Dakota::ModelExecutor::value)
         , "Return function value for continuous values"
         , py::arg("x"))
    
    .def("function_value"
         , static_cast<std::vector<double> (Dakota::ModelExecutor::*)(py::dict &)>
                        (&Dakota::ModelExecutor::value)
         , "Return function value for mixed parameter values"
         , py::arg("vars"))
    
  // Executor output helper
    .def("output_central_moments", &Dakota::ModelExecutor::compute_and_print_moments
         , "Compute and print central moments for response array"
         , py::arg("resp"))

  // Executor helper alias
    .def("initial_values",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list>(
                ModelUtils::continuous_variables(me.model()));
        })


  // --------------
  // Model wrappers
  // --------------

    .def("tv",
        [](const Dakota::ModelExecutor &me) {
            return ModelUtils::tv(me.model()); }, "returns total number of vars")
        
    .def("cv",
        [](const Dakota::ModelExecutor &me) {
        return ModelUtils::cv(me.model()); }, "returns number of continuous variables")

    .def("div",
        [](const Dakota::ModelExecutor &me) {
        return ModelUtils::div(me.model()); }, "returns number of discrete integer vars")

    .def("dsv",
        [](const Dakota::ModelExecutor &me) {
        return ModelUtils::dsv(me.model()); }, "returns number of discrete string vars")

    .def("drv",
        [](const Dakota::ModelExecutor &me) {
        return ModelUtils::drv(me.model()); }, "returns number of discrete real vars")

    .def("continuous_variables",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list>(
                ModelUtils::continuous_variables(me.model()));
        }, "return the continuous variables")

    .def("discrete_int_variables",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list>(
                ModelUtils::discrete_int_variables(me.model()));
        }, "return the discrete integer variables")

    .def("discrete_string_variables",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list,StringMultiArrayConstView,String>(
                ModelUtils::discrete_string_variables(me.model()));
        }, "return the discrete string variables")

    .def("discrete_real_variables",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list>(
                ModelUtils::discrete_real_variables(me.model()));
        }, "return the discrete real variables")

    .def("continuous_variable_labels",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list,StringMultiArray,String>(
                ModelUtils::continuous_variable_labels(me.model()));
        }, "return the continuous variable labels")

    .def("discrete_int_variable_labels",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list,StringMultiArray,String>(
                ModelUtils::discrete_int_variable_labels(me.model()));
        }, "return the discrete int variable labels")

    .def("discrete_string_variable_labels",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list,StringMultiArray,String>(
                ModelUtils::discrete_string_variable_labels(me.model()));
        }, "return the discrete string variable labels")

    .def("discrete_real_variable_labels",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list,StringMultiArray,String>(
                ModelUtils::discrete_real_variable_labels(me.model()));
        }, "return the discrete real variable labels")

    .def("response_size",
        [](const Dakota::ModelExecutor &me) {
            return ModelUtils::response_size(me.model());
        }, "return the number of responses")

    .def("response_labels",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list,StringArray,String>(
                ModelUtils::response_labels(me.model()));
        }, "return the response labels")

    .def("continuous_lower_bounds",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list>(
                ModelUtils::continuous_lower_bounds(me.model()));
        }, "return the continuous lower bounds")

    .def("continuous_upper_bounds",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list>(
                ModelUtils::continuous_upper_bounds(me.model()));
        }, "return the continuous upper bounds")

    .def("discrete_int_lower_bounds",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list>(
                ModelUtils::discrete_int_lower_bounds(me.model()));
        }, "return the discrete int lower bounds")

    .def("discrete_int_upper_bounds",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list>(
                ModelUtils::discrete_int_upper_bounds(me.model()));
        }, "return the discrete int upper bounds")

    .def("discrete_real_lower_bounds",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list>(
                ModelUtils::discrete_real_lower_bounds(me.model()));
        }, "return the discrete real lower bounds")

    .def("discrete_real_upper_bounds",
        [](const Dakota::ModelExecutor &me) {
            return PythonUtils::copy_array_to_pybind11<py::list>(
                ModelUtils::discrete_real_upper_bounds(me.model()));
        }, "return the discrete real upper bounds")
    ;
}

} // namespace Dakota
