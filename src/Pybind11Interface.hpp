/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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

    // override base class prohibition on asynch (needs tightening)
    void init_communicators_checks(int max_eval_concurrency) {}
    // override base class prohibition on asynch (needs tightening)
    void set_communicators_checks(int max_eval_concurrency) {}

    // load and cache the Python module:function specified by ac_name
    void initialize_driver(const String& ac_name);

    /// execute an analysis code portion of a direct evaluation invocation
    virtual int derived_map_ac(const String& ac_name);

    /// Python supports batch only, not true asynch (this is no-op)
    virtual void derived_map_asynch(const ParamResponsePair& pair);
    /// Python supports batch only, not true asynch (this does the work)
    virtual void wait_local_evaluations(PRPQueue& prp_queue);
    /// Python supports batch only, not true asynch, so this blocks
    virtual void test_local_evaluations(PRPQueue& prp_queue);

  private: 
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
    template<typename RetT, class ArrayT, typename T>
    RetT copy_array_to_pybind11(const ArrayT & src) const;

    /// specialized copy Dakota arrays to pybind11 lists via std::vector<> copy
    template<typename RetT, typename OrdinalType, typename ScalarType> 
    RetT copy_array_to_pybind11(const Teuchos::SerialDenseVector<OrdinalType,ScalarType> & src) const;

    /// Translate Dakota parameters into returned Python dictionary in
    /// numpy or array format.
    py::dict params_to_dict() const;

    /// generalized Python dictionary packing to support either lists
    /// or numpy arrays
    template<typename T>
    py::dict pack_kwargs() const;

    /// populate values, gradients, Hessians from Python to Dakota
    void unpack_python_response
    (const ShortArray& asv, const size_t num_derivs,
     const pybind11::dict& py_response, RealVector& fn_values,
     RealMatrix& gradients, RealSymMatrixArray& hessians,
     RealArray& metadata);

    /// return true if the passed asv value is requested for any function
    bool expect_derivative(const ShortArray& asv, const short deriv_type) const;

    /// Compute the evaluation id
    std::string eval_id_string() const;
};


inline void Pybind11Interface::register_pybind11_callback_fn(py::function callback)
{
  py11CallBack = callback;
  py11Active = true;
}

} // namespace Dakota

#endif  // PYBIND11_INTERFACE_H
