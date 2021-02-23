/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Pybind11Interface

//- Description:  Derived interface class for performing analysis via
//-               pybind11 API
//-               finalize of Python for the whole set of function evaluations.
//- Owner:        Russell Hooper
//- Version: $Id$
#ifndef PYBIND11_INTERFACE_H
#define PYBIND11_INTERFACE_H

#include "DirectApplicInterface.hpp"

// The following to forward declare, but avoid clash with include
#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif

namespace Dakota {

/** Specialization of DirectApplicInterface to link to Python analysis 
    drivers. */
class Pybind11Interface: public DirectApplicInterface
{

public:

  Pybind11Interface(const ProblemDescDB& problem_db); ///< constructor
  ~Pybind11Interface();                               ///< destructor

protected:

  /// execute an analysis code portion of a direct evaluation invocation
  virtual int derived_map_ac(const String& ac_name);

  /// direct interface to Pybind11 via API
  int pybind11_run();

  /// whether the user requested numpy data structures in the input file
  bool userNumpyFlag;
  /// true if this class created the interpreter instance
  bool ownPython;

//#ifdef DAKOTA_PYBIND11
//  py::object callBack;
//#endif
};

} // namespace Dakota

#endif  // PYBIND11_INTERFACE_H
