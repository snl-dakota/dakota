/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Pybind11Interface
//- Description:  Class implementation
//- Owner:        Russell Hooper


#include <pybind11/numpy.h>

#include "Pybind11Interface.hpp"
#include "dakota_global_defs.hpp"
#include "DataMethod.hpp"
#include "ProblemDescDB.hpp"

using namespace pybind11::literals; // to bring in the `_a` literal

namespace Dakota {

Pybind11Interface::Pybind11Interface(const ProblemDescDB& problem_db)
  : DirectApplicInterface(problem_db),
    userNumpyFlag(problem_db.get_bool("interface.python.numpy")),
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
    py::finalize_interpreter();
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Python interpreter terminated." << std::endl;
  }
}


/// Python specialization of derived analysis components
int Pybind11Interface::derived_map_ac(const String& ac_name)
{
#ifdef MPI_DEBUG
    Cout << "analysis server " << analysisServerId << " invoking " << ac_name
         << " within Pybind11Interface." << std::endl;
#endif // MPI_DEBUG

  int fail_code = pybind11_run(ac_name);

  // Failure capturing
  if (fail_code) {
    std::string err_msg("Error evaluating Python analysis_driver ");
    err_msg += ac_name;
    throw FunctionEvalFailure(err_msg);
  }

  return 0;
}


int Pybind11Interface::pybind11_run(const String& ac_name)
{
  // minimal error checking for now (or actually none ... but should be)
  int fail_code = 0;

  // If a python callback has not yet been registered (eg via
  // top-evel Dakota API) then try it here.  This is consistent with how
  // PythonInterface does it, ie a lazy initialization. - RWH
  if( !py11Active )
  {
    size_t pos = ac_name.find(":");
    std::string module_name = ac_name.substr(0,pos);
    std::string function_name = ac_name.substr(pos+1);

    py::module_ module = py::module_::import(module_name.c_str());
    py::function callback_fn = module.attr(function_name.c_str());
    register_pybind11_callback_fn(callback_fn);
  }

  assert( py11Active );
  assert( Py_IsInitialized() );

  py::dict kwargs;
  if( userNumpyFlag )
    kwargs = pack_kwargs<py::array>();
  else
    kwargs = pack_kwargs<py::list>();

  py::dict ret_val = py11CallBack(kwargs);

  size_t const numDerivs = directFnDVV.size();

  for (auto item : ret_val) {
    auto key = item.first.cast<std::string>();
    //Cout << "key: " << key << " = " << value[i] << std::endl;

    if (key == "fns") {
      auto values = item.second.cast<std::vector<double>>();
      if (values.size() != numFns) {
        throw(std::runtime_error("Pybind11 Direct Interface [\"fns\"]: "
                                 "incorrect size for # of functions"));
      }
      for (size_t i = 0; i < numFns; ++i) {
        fnVals[i] = values[i];
      }
    }

    else if (key == "fnGrads") {
      auto grads = item.second.cast<std::vector<std::vector<double>>>();
      if (grads.size() != numFns) {
        throw(std::runtime_error("Pybind11 Direct Interface [\"fnGrads\"]: "
                                 "incorrect size for # of functions"));
      }
      for (size_t i = 0; i < numFns; ++i) {
        if (grads[i].size() != numDerivs) {
          throw(std::runtime_error("Pybind11 Direct Interface [\"fnGrads\"]: "
                                   "gradient dimension != # of derivatives "
                                   "for response " + std::to_string(i)));
        }
        for (size_t j = 0; j < numDerivs; ++j) {
          fnGrads[i][j] = grads[i][j];
        }
      }
    }

    else if (key == "fnHessians") {
      auto hess = item.second.cast<
          std::vector<std::vector<std::vector<double>>>>();
      if (hess.size() != numFns) {
        throw(std::runtime_error("Pybind11 Direct Interface [\"fnHessians\"]: "
                                 "incorrect size for # of functions"));
      }
      for (size_t i = 0; i < numFns; ++i) {
        if (hess[i].size() != numDerivs) {
          throw(std::runtime_error(
              "Pybind11 Direct Interface [\"fnHessians\"]: "
              "Hessian # of rows != # of derivatives "
              "for response " + std::to_string(i)));
        }
        for (size_t j = 0; j < numDerivs; ++j) {
          if (hess[i][j].size() != numDerivs) {
            throw(std::runtime_error(
                "Pybind11 Direct Interface [\"fnHessians\"]: "
                "Hessian # of columns != # of derivatives "
                "for response " + std::to_string(i)));
          }
          for (size_t k = 0; k <= j; ++k) {
            fnHessians[i](j, k) = hess[i][j][k];
          }
        }
      }
    }
  }

  return(fail_code);
}

template<typename RetT, class ArrayT, typename T>
RetT Pybind11Interface::copy_array_to_pybind11(const ArrayT & src) const
{
  std::vector<T> tmp_vec;
  for( auto const & a : src )
    tmp_vec.push_back(a);
  return py::cast(tmp_vec);
}

template<typename RetT, class O, class S>
RetT Pybind11Interface::copy_array_to_pybind11(const Teuchos::SerialDenseVector<O,S> & src) const
{
  std::vector<S> tmp_vec;
  copy_data(src, tmp_vec);
  return py::cast(tmp_vec);
}

template<typename py_arrayT>
py::dict Pybind11Interface::pack_kwargs() const
{
  py::list  all_labels   = copy_array_to_pybind11<py::list,StringArray,String>(xAllLabels);
  py_arrayT cv           = copy_array_to_pybind11<py_arrayT>(xC);
  py::list  cv_labels    = copy_array_to_pybind11<py::list,StringMultiArray,String>(xCLabels);
  py_arrayT div          = copy_array_to_pybind11<py_arrayT>(xDI);
  py::list  div_labels   = copy_array_to_pybind11<py::list,StringMultiArray,String>(xDILabels);
  py_arrayT dsv          = copy_array_to_pybind11<py::list,StringMultiArray,String>(xDS);
  py::list  dsv_labels   = copy_array_to_pybind11<py::list,StringMultiArray,String>(xDSLabels);
  py_arrayT drv          = copy_array_to_pybind11<py_arrayT>(xDR);
  py::list  drv_labels   = copy_array_to_pybind11<py::list,StringMultiArray,String>(xDRLabels);
  py_arrayT asv          = copy_array_to_pybind11<py_arrayT,ShortArray,int>(directFnASV);
  py_arrayT dvv          = copy_array_to_pybind11<py_arrayT,SizetArray,size_t>(directFnDVV);
  py_arrayT an_comps     = (analysisComponents.size() > 0)
                          ?  copy_array_to_pybind11<py_arrayT,StringArray,String>(analysisComponents[analysisDriverIndex])
                          :  py_arrayT();

  py::dict kwargs = py::dict(
      "variables"_a             = numVars,
      "functions"_a             = numFns,
      "all_labels"_a            = all_labels,
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
      "currEvalId"_a            = currEvalId);

  return kwargs;
}

} //namespace Dakota
