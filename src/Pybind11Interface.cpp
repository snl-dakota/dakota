/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <algorithm>
#include <pybind11/numpy.h>

#include "Pybind11Interface.hpp"
#include "dakota_global_defs.hpp"
#include "DataMethod.hpp"
#include "ProblemDescDB.hpp"
#include "python_utils.hpp"

using namespace pybind11::literals; // to bring in the `_a` literal

namespace Dakota {

Pybind11Interface::Pybind11Interface(const ProblemDescDB& problem_db, ParallelLibrary& parallel_lib)
  : DirectApplicInterface(problem_db, parallel_lib),
    userNumpyFlag(problem_db.get_bool("interface.python.numpy")),
    ownPython(false),
    py11Active(false)
{
  // Only supports bulk synchronous batch evals with a single driver
  if (asynchFlag) {
    Cerr << "\nError: Python interfaces support single or batch evaluations, "
	 << "but not\nasynchronous.\n";
    abort_handler(INTERFACE_ERROR);
  }
  if (batchEval && analysisDrivers.size() != 1) {
    Cerr << "\nError: interface > python only supports batch option with "
	 << "exactly one\nanalysis_driver string\n";
    abort_handler(INTERFACE_ERROR);
  }

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

  if (userNumpyFlag) {
#ifndef DAKOTA_PYTHON_NUMPY
    Cerr << "\nError: Direct Python interface 'numpy' option requested, but "
	 << "Dakota was not built with numpy support enabled."
         << std::endl;
    abort_handler(-1);
#endif
  }

  // prepend sys.path (env PYTHONPATH) with empty string to find module in pwd
  // This assumes any directory changing in the driver is reversed
  // between function evaluations
  PyRun_SimpleString("import sys\nsys.path.insert(0,\"\")");
}


Pybind11Interface::~Pybind11Interface() {
  if (ownPython && Py_IsInitialized()) {
    // This appears to cause a segfault when other classes use pybind11 objects
    //py::finalize_interpreter();
    //if (outputLevel >= NORMAL_OUTPUT)
    //  Cout << "Python interpreter terminated." << std::endl;
  }
}


/// Python specialization of derived analysis components
int Pybind11Interface::derived_map_ac(const String& ac_name)
{
#ifdef MPI_DEBUG
    Cout << "analysis server " << analysisServerId << " invoking " << ac_name
         << " within Pybind11Interface." << std::endl;
#endif // MPI_DEBUG

    try  {
      initialize_driver(ac_name);

      py::dict kwargs = params_to_dict();

      py::dict ret_val = py11CallBack(kwargs);

      unpack_python_response(directFnASV, directFnDVV.size(), ret_val,
			     fnVals, fnGrads, fnHessians, metaData);
    }
    catch (const std::runtime_error& e) {
      // (py::error_already_set is caught here too)
      std::string err_msg("Error evaluating Python analysis_driver ");
      err_msg += ac_name + ":\n";
      err_msg += e.what();
      Cerr << err_msg << std::endl;
      throw FunctionEvalFailure(err_msg);
    }
    catch (...) {
      std::string err_msg("Error evaluating Python analysis_driver ");
      err_msg += ac_name;
      Cerr << err_msg << std::endl;
      throw FunctionEvalFailure(err_msg);
    }

  return 0;
}


/** This is a no-op as all work takes place in wait/test_local_evaluations */
void Pybind11Interface::derived_map_asynch(const ParamResponsePair& pair)
{ /* no-op*/ }


void Pybind11Interface::wait_local_evaluations(PRPQueue& prp_queue)
{
  // TODO: refactor to avoid passing through local class members for
  // variables and responses (inherit directly from ApplicationInterface)

  ++batchIdCntr;
  
  initialize_driver(analysisDrivers[0]);
  // in this case the user's python function is to be called with
  // list<dict>, one list entry per eval

  py::list py_requests;
  for (const auto& prp : prp_queue) {
    set_local_data(prp.variables(), prp.active_set(), prp.response());
    currEvalId = prp.eval_id();
    py_requests.append(params_to_dict());
  }

  py::list py_responses = py11CallBack(py_requests);

  const size_t num_requests = prp_queue.size();
  size_t i = 0;
  for (auto& prp : prp_queue) {
    const ActiveSet& active_set = prp.active_set();
    // shallow copy technically violates const-ness
    unpack_python_response(active_set.request_vector(),
			   active_set.derivative_vector().size(),
			   py_responses[i], fnVals, fnGrads, fnHessians,
			   metaData);
    Response resp = prp.response();
    resp.update(fnVals, fnGrads, fnHessians, active_set);
    resp.metadata(metaData);
    completionSet.insert(prp.eval_id());
    ++i;
  }
}


void Pybind11Interface::test_local_evaluations(PRPQueue& prp_queue)
{
  wait_local_evaluations(prp_queue);
}


void Pybind11Interface::initialize_driver(const String& ac_name)
{
  // If a python callback has not yet been registered (eg via
  // top-evel Dakota API) then try it here.  This is consistent with how
  // PythonInterface does it, ie a lazy initialization. - RWH
  if( !py11Active )
  {
    size_t pos = ac_name.find(":");
    if ( pos != std::string::npos )
      Cerr << "Warning: delimiter \":\" in "
           << "\"python_module:analysis_function\" is deprecated.  Replace with "
           << "\".\" delimiter" << std::endl;

    else
      pos = ac_name.find(".");
    std::string module_name = ac_name.substr(0,pos);
    std::string function_name = ac_name.substr(pos+1);

    py::module_ module = py::module_::import(module_name.c_str());
    py::function callback_fn = module.attr(function_name.c_str());
    register_pybind11_callback_fn(callback_fn);
  }

  assert( py11Active );
  assert( Py_IsInitialized() );
}


py::dict Pybind11Interface::params_to_dict() const
{
  py::dict kwargs;
  if( userNumpyFlag )
    kwargs = pack_kwargs<py::array>();
  else
    kwargs = pack_kwargs<py::list>();
  return kwargs;
}


template<typename py_arrayT>
py::dict Pybind11Interface::pack_kwargs() const
{
  py::list  all_var_labels   = PythonUtils::copy_array_to_pybind11<py::list,StringArray,String>(xAllLabels);
  py_arrayT cv               = PythonUtils::copy_array_to_pybind11<py_arrayT>(xC);
  py::list  cv_labels        = PythonUtils::copy_array_to_pybind11<py::list,StringMultiArray,String>(xCLabels);
  py_arrayT div              = PythonUtils::copy_array_to_pybind11<py_arrayT>(xDI);
  py::list  div_labels       = PythonUtils::copy_array_to_pybind11<py::list,StringMultiArray,String>(xDILabels);
  py_arrayT dsv              = PythonUtils::copy_array_to_pybind11<py::list,StringMultiArray,String>(xDS);
  py::list  dsv_labels       = PythonUtils::copy_array_to_pybind11<py::list,StringMultiArray,String>(xDSLabels);
  py_arrayT drv              = PythonUtils::copy_array_to_pybind11<py_arrayT>(xDR);
  py::list  drv_labels       = PythonUtils::copy_array_to_pybind11<py::list,StringMultiArray,String>(xDRLabels);
  py_arrayT asv              = PythonUtils::copy_array_to_pybind11<py_arrayT,ShortArray,int>(directFnASV);
  py_arrayT dvv              = PythonUtils::copy_array_to_pybind11<py_arrayT,SizetArray,size_t>(directFnDVV);
  py_arrayT an_comps         = (analysisComponents.size() > 0)
                               ?  PythonUtils::copy_array_to_pybind11<py_arrayT,StringArray,String>(analysisComponents[analysisDriverIndex])
                               :  py_arrayT();
  py::list fn_labels         = PythonUtils::copy_array_to_pybind11<py::list,StringArray,String>(fnLabels);
  py::list md_labels         = PythonUtils::copy_array_to_pybind11<py::list,StringArray,String>(metaDataLabels);

  std::string eval_id(eval_id_string());

  py::dict kwargs = py::dict(
      "variables"_a             = numVars,
      "functions"_a             = numFns,
      "metadata"_a              = metaData.size(),
      "variable_labels"_a       = all_var_labels,
      "function_labels"_a       = fn_labels,
      "metadata_labels"_a       = md_labels,
      "cv"_a                    = cv,
      "cv_labels"_a             = cv_labels,
      "div"_a                   = div,
      "div_labels"_a            = div_labels,
      "dsv"_a                   = dsv,
      "dsv_labels"_a            = dsv_labels,
      "drv"_a                   = drv,
      "drv_labels"_a            = drv_labels,
      "asv"_a                   = asv,
      "dvv"_a                   = dvv,
      "analysis_components"_a   = an_comps,
      "eval_id"_a               = eval_id);

  return kwargs;
}


void Pybind11Interface::unpack_python_response
(const ShortArray& asv, const size_t num_derivs,
 const pybind11::dict& py_response, RealVector& fn_values,
 RealMatrix& gradients, RealSymMatrixArray& hessians,
 RealArray& metadata)
{
  size_t num_fns = asv.size();

  if (expect_derivative(asv, 1)) {
    if (py_response.contains("fns")) {
      auto values = py_response["fns"].cast<std::vector<double>>();
      if (values.size() != num_fns) {
	throw(std::runtime_error("Pybind11 Direct Interface [\"fns\"]: "
				 "incorrect size for # of functions"));
      }
      for (size_t i = 0; i < num_fns; ++i) {
	fn_values[i] = values[i];
      }
    }
    else {
      throw(std::runtime_error("Pybind11 Direct Interface: required key "
			       "[\"fns\"] absent in dict returned to Dakota"));
    }
  }

  if (expect_derivative(asv, 2)) {
    if (py_response.contains("fnGrads")) {
      auto grads = py_response["fnGrads"].cast<std::vector<std::vector<double>>>();
      if (grads.size() != num_fns) {
        throw(std::runtime_error("Pybind11 Direct Interface [\"fnGrads\"]: "
                                 "incorrect size for # of functions"));
      }
      for (size_t i = 0; i < num_fns; ++i) {
        if (grads[i].size() != num_derivs) {
          throw(std::runtime_error("Pybind11 Direct Interface [\"fnGrads\"]: "
                                   "gradient dimension != # of derivatives "
                                   "for response " + std::to_string(i)));
        }
        for (size_t j = 0; j < num_derivs; ++j) {
          gradients[i][j] = grads[i][j];
        }
      }

    }
    else {
      throw(std::runtime_error("Pybind11 Direct Interface: required key "
			       "[\"fnGrads\"] absent in dict returned to Dakota"));
    }
  }

  if (expect_derivative(asv, 4)) {
    if (py_response.contains("fnHessians")) {
      auto hess = py_response["fnHessians"].cast<
	std::vector<std::vector<std::vector<double>>>>();
      if (hess.size() != num_fns) {
        throw(std::runtime_error("Pybind11 Direct Interface [\"fnHessians\"]: "
                                 "incorrect size for # of functions"));
      }
      for (size_t i = 0; i < num_fns; ++i) {
        if (hess[i].size() != num_derivs) {
          throw(std::runtime_error(
              "Pybind11 Direct Interface [\"fnHessians\"]: "
              "Hessian # of rows != # of derivatives "
              "for response " + std::to_string(i)));
        }
        for (size_t j = 0; j < num_derivs; ++j) {
          if (hess[i][j].size() != num_derivs) {
            throw(std::runtime_error(
                "Pybind11 Direct Interface [\"fnHessians\"]: "
                "Hessian # of columns != # of derivatives "
                "for response " + std::to_string(i)));
          }
          for (size_t k = 0; k <= j; ++k) {
            hessians[i](j, k) = hess[i][j][k];
          }
        }
      }
    }
    else {
      throw(std::runtime_error("Pybind11 Direct Interface: required key "
			       "[\"fnHessians\"] absent in dict returned to Dakota"));
    }
  }

  size_t num_md = metadata.size();
  if (num_md > 0) {
    if (py_response.contains("metadata")) {
      auto md = py_response["metadata"].cast<std::vector<double>>();
      if (md.size() != num_md) {
	throw(std::runtime_error("Pybind11 Direct Interface [\"metadata\"]: "
				 "incorrect size for # of metadata"));
      }
      for (size_t i = 0; i < num_md; ++i) {
	metadata[i] = md[i];
      }
    }
    else {
      throw(std::runtime_error("Pybind11 Direct Interface: required key "
			       "[\"metadata\"] absent in dict returned to Dakota"));
    }
  }

}


bool Pybind11Interface::expect_derivative(const ShortArray& asv,
					  const short deriv_type) const
{
  return std::any_of(asv.begin(), asv.end(),
		     [deriv_type](short a){ return (a & deriv_type); });
}

std::string Pybind11Interface::eval_id_string() const {
  std::string eval_id;
  if(!evalTagPrefix.empty())
    eval_id = evalTagPrefix + ":";
  if(batchEval)
    eval_id += std::to_string(batchIdCntr) + ":";
  eval_id += std::to_string(currEvalId);
  return eval_id;
}


} //namespace Dakota
