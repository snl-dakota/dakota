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


//// This hacks around lack of a packaged Python debug lib by default on Windows.
//#if defined(_DEBUG) && defined(_MSC_VER)
//#undef _DEBUG
//#include <Python.h>
//#define _DEBUG
//#else
//#include <Python.h>
//#endif
//
//#ifdef DAKOTA_PYTHON_NUMPY
//#include <numpy/arrayobject.h>
//#endif

#ifdef DAKOTA_PYBIND11
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;
using namespace pybind11::literals; // to bring in the `_a` literal
#endif

// Python 2/3 compatibility layer
// BMA: Using preprocessor defines (only in this compilation unit) per
// submitted patch since not likely to support Python 2 for long and
// then they'll go away
#if PY_MAJOR_VERSION >= 3
#define DAKPY_PYSTR_FROMSTR PyUnicode_FromString
#define DAKPY_PYSTR_ASSTR PyUnicode_AsUTF8
#define DAKPY_PYINT_FROMLONG PyLong_FromLong
#define DAKPY_PYINT_ASLONG PyLong_AsLong
#define DAKPY_PYINT_CHECK PyLong_Check
#ifdef DAKOTA_PYTHON_NUMPY
#define DAKPY_IMPORT_ARRAY import_array1
#endif
#else
#define DAKPY_PYSTR_FROMSTR PyString_FromString
#define DAKPY_PYSTR_ASSTR PyString_AsString
#define DAKPY_PYINT_FROMLONG PyInt_FromLong
#define DAKPY_PYINT_ASLONG PyInt_AsLong
#define DAKPY_PYINT_CHECK PyInt_Check
#ifdef DAKOTA_PYTHON_NUMPY
#define DAKPY_IMPORT_ARRAY import_array
#endif
#endif

#include "Pybind11Interface.hpp"
#include "dakota_global_defs.hpp"
#include "DataMethod.hpp"
#include "ProblemDescDB.hpp"


namespace Dakota {

Pybind11Interface::Pybind11Interface(const ProblemDescDB& problem_db)
  : DirectApplicInterface(problem_db),
    userNumpyFlag(problem_db.get_bool("interface.python.numpy")),
    ownPython(false)
{
  if (!Py_IsInitialized()) {
    Py_Initialize();
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
#ifdef DAKOTA_PYTHON_NUMPY
    DAKPY_IMPORT_ARRAY();
#else
    Cerr << "\nError: Direct Python interface 'numpy' option requested, but "
	 << "not available." << std::endl;
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
    Py_Finalize();
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

  int fail_code = pybind11_run();

  // Failure capturing
  if (fail_code) {
    std::string err_msg("Error evaluating Python analysis_driver ");
    err_msg += ac_name;
    throw FunctionEvalFailure(err_msg);
  }

  return 0;
}


int Pybind11Interface::pybind11_run()
{
  // minimal error checking for now (or actually none ... but should be)
  int fail_code = 0;

  assert( py11Active );

  assert( Py_IsInitialized() );

  std::vector<double> tmp_cv;
  copy_data(xC, tmp_cv);
  py::list cv = py::cast(tmp_cv);

  std::vector<int> tmp_asv;
  for( auto const & a : directFnASV )
    tmp_asv.push_back(a);
  py::list asv = py::cast(tmp_asv);

  py::dict kwargs = py::dict(
      "variables"_a = numVars,
      "functions"_a = numFns,
      "cv"_a        = cv,
      "asv"_a       = asv      );

  py::dict ret_val = py11CallBack(kwargs);

  for (auto item : ret_val)
  {
    auto key = item.first.cast<std::string>();
    auto value = item.second.cast<std::vector<double>>();
    //Cout << "key: " << key << " = " << value[i] << std::endl;

    // Hard-coded for a single response
    if( key == "fns" )
      fnVals[0] = value[0];
  }

  return(fail_code);
}

} //namespace Dakota
