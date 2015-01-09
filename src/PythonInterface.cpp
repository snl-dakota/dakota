/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        PythonInterface
//- Description:  Class implementation
//- Owner:        Brian Adams

// Include Python headers first to avoid _POSIX_C_SOURCE redefinition warnings

// This hacks around lack of a packaged Python debug lib by default on Windows.
#if defined(_DEBUG) && defined(_MSC_VER)
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

#ifdef DAKOTA_PYTHON_NUMPY
#include <arrayobject.h>
#endif

#include "PythonInterface.hpp"
#include "dakota_global_defs.hpp"
#include "DataMethod.hpp"
#include "ProblemDescDB.hpp"


namespace Dakota {

PythonInterface::PythonInterface(const ProblemDescDB& problem_db)
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
    import_array();
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


PythonInterface::~PythonInterface() {
  if (ownPython && Py_IsInitialized()) {
    Py_Finalize();
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Python interpreter terminated." << std::endl;
  }
}


/// Python specialization of derived analysis components
int PythonInterface::derived_map_ac(const String& ac_name)
{
#ifdef MPI_DEBUG
    Cout << "analysis server " << analysisServerId << " invoking " << ac_name
         << " within PythonInterface." << std::endl;
#endif // MPI_DEBUG

  int fail_code = python_run(ac_name);

  // Failure capturing
  if (fail_code) {
    std::string err_msg("Error evaluating Python analysis_driver ");
    err_msg += ac_name;
    throw FunctionEvalFailure(err_msg);
  }

  return 0;
}


int PythonInterface::python_run(const String& ac_name)
{
  // probably need to convert all of the following with SWIG or Boost!!
  // (there is minimal error checking for now)
  // need to cleanup ref counts on Python objects
  int fail_code = 0;

  // probably want to load the modules and functions at construction time, incl.
  // validation and store the objects for later, but need to resolve use of
  // analysisDriverIndex

  // must use empty tuple here to pass to function taking only kwargs
  PyObject *pArgs = PyTuple_New(0);
  PyObject *pDict = PyDict_New();

  // convert DAKOTA data types to Python objects (lists and/or numpy arrays)
  PyObject *cv, *cv_labels, *div, *div_labels, *drv, *drv_labels,
    *av, *av_labels, *asv, *dvv, *an_comps;
  python_convert(xC, &cv);
  python_convert_strlist(xCLabels, &cv_labels);
  python_convert_int(xDI, xDI.length(), &div);
  python_convert_strlist(xDILabels, &div_labels);
  python_convert(xDR, &drv);
  python_convert_strlist(xDRLabels, &drv_labels);
  python_convert(xC, xDI, xDR, &av);
  python_convert(xCLabels, xDILabels, xDRLabels, &av_labels);
  python_convert_int(directFnASV, directFnASV.size(), &asv);
  python_convert_int(directFnDVV, directFnDVV.size(), &dvv);
  // send analysis components, or an empty list
  if (analysisComponents.size() > 0)
    python_convert_strlist(analysisComponents[analysisDriverIndex], &an_comps);
  else
    an_comps = PyList_New(0);

  // assemble everything into a dictionary to pass to user function
  // this should eat references to the objects declared above
  PyDict_SetItem(pDict, PyString_FromString("variables"), 
		 PyInt_FromLong((long) numVars));
  PyDict_SetItem(pDict, PyString_FromString("functions"), 
		 PyInt_FromLong((long) numFns)); 
  PyDict_SetItem(pDict, PyString_FromString("cv"), cv);
  PyDict_SetItem(pDict, PyString_FromString("cv_labels"), cv_labels);
  PyDict_SetItem(pDict, PyString_FromString("div"), div);
  PyDict_SetItem(pDict, PyString_FromString("div_labels"), div_labels);
  PyDict_SetItem(pDict, PyString_FromString("drv"), drv);
  PyDict_SetItem(pDict, PyString_FromString("drv_labels"), drv_labels);
  PyDict_SetItem(pDict, PyString_FromString("av"), av);
  PyDict_SetItem(pDict, PyString_FromString("av_labels"), av_labels);
  PyDict_SetItem(pDict, PyString_FromString("asv"), asv);
  PyDict_SetItem(pDict, PyString_FromString("dvv"), dvv);
  PyDict_SetItem(pDict, PyString_FromString("analysis_components"), an_comps);
  PyDict_SetItem(pDict, PyString_FromString("currEvalId"), 
		 PyInt_FromLong((long) currEvalId));


  // The active analysis_driver is passed in ac_name (in form
  // module:function); could make module optional.  We pass any
  // analysis components as string arguments to the Python function.
  size_t pos = ac_name.find(":");
  std::string module_name = ac_name.substr(0,pos);
  std::string function_name = ac_name.substr(pos+1);
  if (module_name.size() == 0 || function_name.size() == 0) {
    Cerr << "\nError: invalid Python analysis_driver '" << ac_name
	 << "'\n       Should have form 'module:function'." << std::endl;
    Py_DECREF(pDict);
    Py_DECREF(pArgs);
    abort_handler(INTERFACE_ERROR);
  }

  // import the module and function and test for callable
  PyObject *pModule = PyImport_Import(PyString_FromString(module_name.c_str()));
  if (pModule == NULL) {
    Cerr << "Error (PythonInterface): Failure importing module '" 
	 << module_name  << "'.\n                         Consider setting "
	 << "PYTHONPATH." << std::endl;
    Py_DECREF(pDict);
    Py_DECREF(pArgs);
    abort_handler(INTERFACE_ERROR);
  }

  PyObject *pFunc = PyObject_GetAttrString(pModule, function_name.c_str());
  if (!pFunc || !PyCallable_Check(pFunc)) {
    Cerr << "Error (PythonInterface): Function '" << function_name  
	 << "' not found or not callable" << std::endl;
    Py_DECREF(pDict);
    Py_DECREF(pArgs);
    Py_DECREF(pModule);
    abort_handler(INTERFACE_ERROR);
  }

  // perform analysis
  if (outputLevel > NORMAL_OUTPUT)
    Cout << "Info (PythonInterface): Calling function " << function_name 
	 << " in module " << module_name << "." << std::endl;
  PyErr_Clear();
  PyObject *retVal = PyObject_Call(pFunc, pArgs, pDict);

  Py_DECREF(pDict);
  Py_DECREF(pArgs);    
  Py_DECREF(pModule);
  Py_DECREF(pFunc);

  if (!retVal) {
    if (PyErr_Occurred()) {
      if (abort_mode == ABORT_EXITS)  // If abort throws, caller must report.
        PyErr_Print();
    }
    else {
      Cerr << "Error (PythonInterface): Unknown error evaluating python "
          << "function." << std::endl;
    }
    // abort_handler manages exit vs. throw
    //    abort_handler(INTERFACE_ERROR);
    // by throwing, we allow manage_failures to try again if desired
    std::string err_msg("Error evaluating Python analysis_driver ");
    err_msg += ac_name;
    throw FunctionEvalFailure(err_msg);
  }


  // process the return data

  bool fn_flag = false;
  for (size_t i=0; i<numFns; ++i)
    if (directFnASV[i] & 1) {
      fn_flag = true;
      break;
    }

  // process return type as dictionary, else assume list of functions only
  if (PyDict_Check(retVal)) {
    // or the user may return a dictionary containing entires fns, fnGrads,
    // fnHessians, fnLabels, failure (int)
    // fnGrads, e.g. is a list of lists of doubles
    // this is where Boost or SWIG could really help
    // making a lot of assumptions on types being returned
    PyObject *obj;
    if (fn_flag) {
      if ( !(obj = PyDict_GetItemString(retVal, "fns")) ) {
	Cerr << "Python dictionary must contain list 'fns'" << std::endl;
	Py_DECREF(retVal);
	abort_handler(INTERFACE_ERROR);
      }
      if (!python_convert(obj, fnVals, numFns)) {
	Py_DECREF(retVal);
	abort_handler(INTERFACE_ERROR);
      }
    }
    if (gradFlag) {
      if ( !(obj = PyDict_GetItemString(retVal, "fnGrads")) ) {
	Cerr << "Python dictionary must contain list 'fnGrads'" << std::endl;
	Py_DECREF(retVal);
	abort_handler(INTERFACE_ERROR);
      }
      if (!python_convert(obj, fnGrads)) {
	Py_DECREF(retVal);
	abort_handler(INTERFACE_ERROR);
      }
    }
    if (hessFlag) {
      if ( !(obj = PyDict_GetItemString(retVal, "fnHessians")) ) {
	Cerr << "Python dictionary must contain list 'fnHessians'" << std::endl;
	Py_DECREF(retVal);
	abort_handler(INTERFACE_ERROR);
      }
      if (!python_convert(obj, fnHessians)){
	Py_DECREF(retVal);
	abort_handler(INTERFACE_ERROR);
      }
    }
    // optional returns
    if (obj = PyDict_GetItemString(retVal, "failure"))
      fail_code = PyInt_AsLong(obj);

    if (obj = PyDict_GetItemString(retVal, "fnLabels")) {
      if (!PyList_Check(obj) || PyList_Size(obj) != numFns) {
	Cerr << "'fnLabels' must be list of length numFns." << std::endl;
	Py_DECREF(retVal);
	abort_handler(INTERFACE_ERROR);
      }
      for (size_t i=0; i<numFns; ++i)
	fnLabels[i] = PyString_AsString(PyList_GetItem(obj, i));
    }
  }
  else {
    // asssume list/numpy array containing only functions
    if (fn_flag)
      python_convert(retVal, fnVals, numFns);
  }
  Py_DECREF(retVal);

  return(fail_code);
}


/** convert all integer array types including IntVector, ShortArray,
    and SizetArray to Python list of ints or numpy array of ints */
template<class ArrayT, class Size>
bool PythonInterface::
python_convert_int(const ArrayT& src, Size sz, PyObject** dst)
{
#ifdef DAKOTA_PYTHON_NUMPY
  if (userNumpyFlag) {
    npy_intp dims[1];
    dims[0] = sz;
    if (!(*dst = PyArray_SimpleNew(1, dims, PyArray_INT))) {
      Cerr << "Error creating Python numpy array." << std::endl;
      return(false);
    }
    PyArrayObject *pao = (PyArrayObject *) *dst;
    for (Size i=0; i<sz; ++i)
      *(int *)(pao->data + i*(pao->strides[0])) = (int) src[i];
  }
  else 
#endif
  {
    if (!(*dst = PyList_New(sz))) {
      Cerr << "Error creating Python list." << std::endl;
      return(false);
    }
    for (Size i=0; i<sz; ++i)
      PyList_SetItem(*dst, i, PyInt_FromLong((long) src[i]));
  }
  return(true);
}


// convert RealVector to list of floats or numpy array of doubles
bool PythonInterface::
python_convert(const RealVector& src, PyObject** dst)
{
  int sz = src.length();
#ifdef DAKOTA_PYTHON_NUMPY
  if (userNumpyFlag) {
    npy_intp dims[1];
    dims[0] = sz;
    if (!(*dst = PyArray_SimpleNew(1, dims, PyArray_DOUBLE))) {
      Cerr << "Error creating Python numpy array." << std::endl;
      return(false);
    }
    PyArrayObject *pao = (PyArrayObject *) *dst;
    for (int i=0; i<sz; ++i)
      *(double *)(pao->data + i*(pao->strides[0])) = src[i];
  }
  else
#endif
  {
    if (!(*dst = PyList_New(sz))) {
      Cerr << "Error creating Python list." << std::endl;
      return(false);
    }
    for (int i=0; i<sz; ++i)
      PyList_SetItem(*dst, i, PyFloat_FromDouble(src[i]));
  }
  return(true);
}


// helper for converting xC, xDI, and xDR to single Python array of all variables
bool PythonInterface::
python_convert(const RealVector& c_src, const IntVector& di_src,
	       const RealVector& dr_src, PyObject** dst)
{
  int c_sz = c_src.length();
  int di_sz = di_src.length();
  int dr_sz = dr_src.length();
#ifdef DAKOTA_PYTHON_NUMPY
  if (userNumpyFlag) {
    npy_intp dims[1];
    dims[0] = c_sz + di_sz + dr_sz;
    if (!(*dst = PyArray_SimpleNew(1, dims, PyArray_DOUBLE))) {
      Cerr << "Error creating Python numpy array." << std::endl;
      return(false);
    }
    PyArrayObject *pao = (PyArrayObject *) *dst;
    for (int i=0; i<c_sz; ++i)
      *(double *)(pao->data + i*(pao->strides[0])) = c_src[i];
    for (int i=0; i<di_sz; ++i)
      *(double *)(pao->data + (c_sz+i)*(pao->strides[0])) = (double) di_src[i];
    for (int i=0; i<dr_sz; ++i)
      *(double *)(pao->data + (c_sz+di_sz+i)*(pao->strides[0])) = dr_src[i];
  }
  else
#endif
  {
    if (!(*dst = PyList_New(c_sz + di_sz + dr_sz))) {
      Cerr << "Error creating Python list." << std::endl;
      return(false);
    }
    for (int i=0; i<c_sz; ++i)
      PyList_SetItem(*dst, i, PyFloat_FromDouble(c_src[i]));
    for (int i=0; i<di_sz; ++i)
      PyList_SetItem(*dst, c_sz + i, PyInt_FromLong((long) di_src[i]));
    for (int i=0; i<dr_sz; ++i)
      PyList_SetItem(*dst, c_sz + di_sz + i, 
		     PyFloat_FromDouble(dr_src[i]));
  }
  return(true);
}


// convert StringArray/MultiArray to list of strings
template<class StringArrayT>
bool PythonInterface::
python_convert_strlist(const StringArrayT& src, PyObject** dst)
{
  int sz = src.size();
  if (!(*dst = PyList_New(sz))) {
      Cerr << "Error creating Python list." << std::endl;
      return(false);
  }
  for (int i=0; i<sz; ++i)
    PyList_SetItem(*dst, i, PyString_FromString(src[i].c_str()));

  return(true);
}


// convert continuous and discrete label strings to single list
bool PythonInterface::
python_convert(const StringMultiArray& c_src, const StringMultiArray& di_src,
	       const StringMultiArray& dr_src, PyObject** dst)
{
  int c_sz = c_src.size();
  int di_sz = di_src.size();
  int dr_sz = dr_src.size();
  if (!(*dst = PyList_New(c_sz + di_sz + dr_sz))) {
    Cerr << "Error creating Python list." << std::endl;
    return(false);
  }
  for (int i=0; i<c_sz; ++i)
    PyList_SetItem(*dst, i, PyString_FromString(c_src[i].c_str()));
  for (int i=0; i<di_sz; ++i)
    PyList_SetItem(*dst, c_sz+i, PyString_FromString(di_src[i].c_str()));
  for (int i=0; i<dr_sz; ++i)
    PyList_SetItem(*dst, c_sz+di_sz+i, PyString_FromString(dr_src[i].c_str()));

  return(true);
}


// Accepts python list or numpy array, DAKOTA RealVector,
// expected dimension. Returns false if conversion failed.
bool PythonInterface::
python_convert(PyObject *pyv, RealVector& rv, const int& dim)
{
#ifdef DAKOTA_PYTHON_NUMPY
  // could automatically detect return type instead of throwing error
  if (userNumpyFlag) {
    if (!PyArray_Check(pyv) || PyArray_NDIM(pyv) != 1 || 
	PyArray_DIM(pyv,0) != dim) {
      Cerr << "Python numpy array not 1D of size " << dim << "." << std::endl;
      return(false);
    }
    PyArrayObject *pao = (PyArrayObject *) pyv;
    for (int i=0; i<dim; ++i)
      rv[i] = *(double *)(pao->data + i*(pao->strides[0]));
  }
  else
#endif
  {
    PyObject *val;
    if (!PyList_Check(pyv) || PyList_Size(pyv) != dim) {
      Cerr << "Python vector must have length " << dim << "." << std::endl;
      return(false);
    }
    for (int i=0; i<dim; ++i) {
      val = PyList_GetItem(pyv, i);
      if (PyFloat_Check(val))
	rv[i] = PyFloat_AsDouble(val);
      else if (PyInt_Check(val))
	rv[i] = (double) PyInt_AsLong(val);
      else {
	Cerr << "Unsupported Python data type converting vector." << std::endl;
	Py_DECREF(val);
	return(false);
      }
    }
  }
  return(true);
}

// Accepts python list or numpy array, pointer to double, e.g., view
// of a Teuchose::SerialDenseVector, expected dimension.  Returns
// false if conversion failed.
bool PythonInterface::
python_convert(PyObject *pyv, double *rv, const int& dim)
{
#ifdef DAKOTA_PYTHON_NUMPY
  // could automatically detect return type instead of throwing error
  if (userNumpyFlag) {
    if (!PyArray_Check(pyv) || PyArray_NDIM(pyv) != 1 || 
	PyArray_DIM(pyv,0) != dim) {
      Cerr << "Python numpy array not 1D of size " << dim << "." << std::endl;
      return(false);
    }
    PyArrayObject *pao = (PyArrayObject *) pyv;
    for (int i=0; i<dim; ++i)
      rv[i] = *(double *)(pao->data + i*(pao->strides[0]));
  }
  else
#endif
  {
    PyObject *val;
    if (!PyList_Check(pyv) || PyList_Size(pyv) != dim) {
      Cerr << "Python vector must have length " << dim << "." << std::endl;
      return(false);
    }
    for (int i=0; i<dim; ++i) {
      val = PyList_GetItem(pyv, i);
      if (PyFloat_Check(val))
	rv[i] = PyFloat_AsDouble(val);
      else if (PyInt_Check(val))
	rv[i] = (double) PyInt_AsLong(val);
      else {
	Cerr << "Unsupported Python data type converting vector." << std::endl;
	Py_DECREF(val);
	return(false);
      }
    }
  }
  return(true);
}

// assume we're converting numFns x numDerivVars to numDerivVars x
// numFns (gradients) returns false if conversion failed
bool PythonInterface::python_convert(PyObject *pym, RealMatrix &rm)
{
#ifdef DAKOTA_PYTHON_NUMPY
  if (userNumpyFlag) {
    if (!PyArray_Check(pym) || PyArray_NDIM(pym) != 2 || 
	PyArray_DIM(pym,0) != numFns  ||  PyArray_DIM(pym,1) != numDerivVars) {
      Cerr << "Python numpy array not 2D of size " << numFns << "x"
	   << numDerivVars << "." << std::endl;
      return(false);
    }
    PyArrayObject *pao = (PyArrayObject *) pym;
    for (size_t i=0; i<numFns; ++i)
      for (size_t j=0; j<numDerivVars; ++j)
	rm(j,i) = *(double *)(pao->data + i*(pao->strides[0]) + 
			      j*(pao->strides[1]));
  }
  else
#endif
  {
    PyObject *val;
    if (!PyList_Check(pym) || PyList_Size(pym) != numFns) {
      Cerr << "Python matrix must have " << numFns << "rows." << std::endl;
      return(false);
    }
    for (size_t i=0; i<numFns; ++i) {
      val = PyList_GetItem(pym, i);
      if (PyList_Check(val)) {
	// use the helper to convert this column of the gradients
	if (!python_convert(val, rm[i], numDerivVars))
	  return(false);
      }
      else {
	Cerr << "Each row of Python matrix must be a list." << std::endl;
	Py_DECREF(val);
	return(false);
      }
    }
  }
  return(true);
}

// assume numDerivVars x numDerivVars as helper in Hessian conversion
// and lower triangular storage in Hessians
// returns false if conversion failed
bool PythonInterface::python_convert(PyObject *pym, 
					   RealSymMatrix &rm)
{
  // for now, the numpy case isn't called (since handled in calling
  // Hessian array convert)
#ifdef DAKOTA_PYTHON_NUMPY
  if (userNumpyFlag) {
    if (!PyArray_Check(pym) || PyArray_NDIM(pym) != 2 || 
	PyArray_DIM(pym,0) != numDerivVars  ||
	PyArray_DIM(pym,1) != numDerivVars) {
      Cerr << "Python numpy array not 2D of size " << numDerivVars << "x" 
	   << numDerivVars << "." << std::endl;
      return(false);
    }
    PyArrayObject *pao = (PyArrayObject *) pym;
    for (size_t i=0; i<numDerivVars; ++i)
      for (size_t j=0; j<=i; ++j)
	rm(i,j) = *(double *)(pao->data + i*(pao->strides[0]) + 
			       j*(pao->strides[1]));
  }
  else
#endif
  {
    if (!PyList_Check(pym) || PyList_Size(pym) != numDerivVars) {
      Cerr << "Python matrix must have " << numDerivVars << "rows." <<std::endl;
      return(false);
    }
    PyObject *pyv, *val;
    for (size_t i=0; i<numDerivVars; ++i) {
      pyv = PyList_GetItem(pym, i);
      if (!PyList_Check(pyv) || PyList_Size(pyv) != numDerivVars) {
	Cerr << "Python vector must have length " << numDerivVars << "." 
	     << std::endl;
	return(false);
      }
      for (int j=0; j<=i; ++j) {
	val = PyList_GetItem(pyv, j);
	if (PyFloat_Check(val))
	  rm(i,j) = PyFloat_AsDouble(val);
	else if (PyInt_Check(val))
	  rm(i,j) = (double) PyInt_AsLong(val);
	else {
	  Cerr << "Unsupported Python data type converting vector." 
	       << std::endl;
	  Py_DECREF(val);
	  return(false);
	}
      }
    }

  }
  return(true);
}


// assume numFns x numDerivVars x numDerivVars
// returns false if conversion failed
bool PythonInterface::
python_convert(PyObject *pyma, RealSymMatrixArray &rma)
{
#ifdef DAKOTA_PYTHON_NUMPY
  if (userNumpyFlag) {
    // cannot recurse in this case as we now have a symmetric matrix 
    // (clearer this way anyway)
    if (!PyArray_Check(pyma) || PyArray_NDIM(pyma) != 3 || 
	PyArray_DIM(pyma,0) != numFns || PyArray_DIM(pyma,1) != numDerivVars ||
	PyArray_DIM(pyma,2) != numDerivVars ) {
      Cerr << "Python numpy array not 3D of size " << numFns << "x"
	   << numDerivVars << "x" << numDerivVars << "." << std::endl;
      return(false);
    }
    PyArrayObject *pao = (PyArrayObject *) pyma;
    for (size_t i=0; i<numFns; ++i)
      for (size_t j=0; j<numDerivVars; ++j)
	for (size_t k=0; k<=j; ++k)
	  rma[i](j,k) = *(double *)(pao->data + i*(pao->strides[0]) + 
				    j*(pao->strides[1]) +
				    k*(pao->strides[2]));
  }
  else
#endif
  {
    PyObject *val;
    if (!PyList_Check(pyma) || PyList_Size(pyma) != numFns) {
      Cerr << "Python matrix array must have " << numFns << " rows."
	   << std::endl;
      return(false);
    }
    for (size_t i=0; i<numFns; ++i) {
      val = PyList_GetItem(pyma, i);
      if (PyList_Check(val)) {
	if (!python_convert(val, rma[i]))
	  return(false);
      }
      else {
	Cerr << "Each row of Python matrix must be a list." << std::endl;
	Py_DECREF(val);
	return(false);
      }
    }
  }
  return(true);
}


} //namespace Dakota
