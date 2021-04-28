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

#ifndef PYBIND11_INTERFACE_H
#define PYBIND11_INTERFACE_H

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include "DirectApplicInterface.hpp"

namespace Dakota {

/** Specialization of DirectApplicInterface to link to Python analysis 
    drivers. */
class Pybind11Interface: public DirectApplicInterface
{

  public:

    Pybind11Interface(const ProblemDescDB& problem_db); ///< constructor
    ~Pybind11Interface();                               ///< destructor

    /// register a python callback function
    void register_pybind11_callback_fn(py::function callback);

    /// register a collection of python callback functions
    void register_pybind11_callback_fns(const std::map<String, py::function>& callbacks);

  protected:

    /// execute an analysis code portion of a direct evaluation invocation
    virtual int derived_map_ac(const String& ac_name);

    /// direct interface to Pybind11 via API
    int pybind11_run(const String& ac_name);

    /// whether the user requested numpy data structures in the input file
    bool userNumpyFlag;
    /// true if this class created the interpreter instance
    bool ownPython;
    /// callback function for analysis driver
    py::function py11CallBack;

    bool py11Active;

    /// copy Dakota arrays to pybind11 lists via std::vector<> copy
    template<class ArrayT, typename T>
    py::list copy_array_to_pybind11(const ArrayT & src);

    /// specialized copy Dakota arrays to pybind11 lists via std::vector<> copy
    template<typename OrdinalType, typename ScalarType> 
    py::list copy_array_to_pybind11(const Teuchos::SerialDenseVector<OrdinalType,ScalarType> & src);
};


inline void Pybind11Interface::register_pybind11_callback_fn(py::function callback)
{
  py11CallBack = callback;
  py11Active = true;
}

} // namespace Dakota

#endif  // PYBIND11_INTERFACE_H
