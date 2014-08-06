/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        PythonInterface

//- Description:  Derived interface class for performing analysis via
//-               linked Python API, permitting single initialization / 
//-               finalize of Python for the whole set of function evaluations.
//- Owner:        Brian Adams
//- Version: $Id$
#ifndef PYTHON_INTERFACE_H
#define PYTHON_INTERFACE_H

#include "DirectApplicInterface.hpp"

// The following to forward declare, but avoid clash with include
#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif

namespace Dakota {

/** Specialization of DirectApplicInterface to link to Python analysis 
    drivers.  Includes convenience functions to map data to/from Python */
class PythonInterface: public DirectApplicInterface
{

public:

  PythonInterface(const ProblemDescDB& problem_db); ///< constructor
  ~PythonInterface();                               ///< destructor

protected:

  /// execute an analysis code portion of a direct evaluation invocation
  virtual int derived_map_ac(const String& ac_name);

  /// direct interface to Python via API, BMA 07/02/07
  int python_run(const String& ac_name);

  /// whether the user requested numpy data structures in the input file
  bool userNumpyFlag;
  /// true if this class created the interpreter instance
  bool ownPython;

  /// convert arrays of integer types to Python list or numpy array
  template<class ArrayT, class Size>
  bool python_convert_int(const ArrayT& src, Size size, PyObject** dst);
  /// convert RealVector to Python list or numpy array
  bool python_convert(const RealVector& src, PyObject** dst);
  /// convert RealVector + IntVector + RealVector to Python mixed list 
  /// or numpy double array
  bool python_convert(const RealVector& c_src, const IntVector& di_src,
		      const RealVector& dr_src, PyObject** dst);
  /// convert labels
  template<class StringArrayT>
  bool python_convert_strlist(const StringArrayT& src, PyObject** dst);
  /// convert all labels to single list
  bool python_convert(const StringMultiArray& c_src, 
		      const StringMultiArray& di_src, 
		      const StringMultiArray& dr_src, PyObject** dst);
  /// convert python [list of int or float] or [numpy array of double] to 
  /// RealVector (for fns)
  bool python_convert(PyObject *pyv, RealVector& rv, const int& dim);
  /// convert python [list of int or float] or [numpy array of double] to 
  /// double[], for use as helper in converting gradients
  bool python_convert(PyObject *pyv, double *rv, const int& dim);
  /// convert python [list of lists of int or float] or [numpy array of dbl]
  /// to RealMatrix (for gradients)
  bool python_convert(PyObject *pym, RealMatrix &rm);
  /// convert python [list of lists of int or float] or [numpy array of dbl]
  /// to RealMatrix (used as helper in Hessian conversion)
  bool python_convert(PyObject *pym, RealSymMatrix &rm);
  /// convert python [list of lists of lists of int or float] or 
  /// [numpy array of double] to RealSymMatrixArray (for Hessians)
  bool python_convert(PyObject *pyma, RealSymMatrixArray &rma);

};

} // namespace Dakota

#endif  // PYTHON_INTERFACE_H
