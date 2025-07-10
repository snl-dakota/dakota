/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#ifndef EXTERNAL_PYTHON_METHOD_H
#define EXTERNAL_PYTHON_METHOD_H

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>
namespace py = pybind11;

// Dakota headers
#include "DakotaIterator.hpp"

namespace Dakota {

// -----------------------------------------------------------------

class ExtPythonMethod : public Iterator
{
  public:

    /// Standard constructor
    ExtPythonMethod(ProblemDescDB &, ParallelLibrary &, std::shared_ptr<Dakota::Model>);

    /// Dummy constructor
    ExtPythonMethod();

    /// Destructor
    ~ExtPythonMethod() {}

    /// Initializes the Method
    void initialize_run() override;

    /// Executes the Method
    void core_run() override;

  protected:

  /// whether the user requested numpy data structures in the input file
  bool userNumpyFlag;
  /// true if this class created the interpreter instance
  bool ownPython;
  /// callback function for analysis driver
  py::function py11CallBack;

  /// Python module filename and class/function
  String moduleAndClassName;

  bool py11Active;

  template<typename RetT, class ArrayT, typename T>
  RetT copy_array_to_pybind11(const ArrayT & src) const
  {
    std::vector<T> tmp_vec;
    for( auto const & a : src )
      tmp_vec.push_back(a);
    return py::cast(tmp_vec);
  };

  template<typename RetT, class O, class S>
  RetT copy_array_to_pybind11(const Teuchos::SerialDenseVector<O,S> & src) const
  {
    std::vector<S> tmp_vec;
    copy_data(src, tmp_vec);
    return py::cast(tmp_vec);
  };

}; // class ExtPythonMethod


// -----------------------------------------------------------------

class ExtPythonTraits: public TraitsBase
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// Default constructor
  ExtPythonTraits() { }

  /// Destructor
  virtual ~ExtPythonTraits() { }

  /// Demo_Opt default data type to be used by Dakota data adapters
  typedef std::vector<double> VecT;

  /// This is needed to handle constraints
  inline static double noValue()
  { return std::numeric_limits<Real>::max(); }

  //
  //- Heading: Virtual member function redefinitions
  //

  /// Return the flag indicating whether method supports continuous variables
  bool supports_continuous_variables() override
    { return true; }

  /// Return the flag indicating whether method supports nonlinear equality constrinats
  bool supports_nonlinear_equality() override
    { return true; }

  /// Return the flag indicating whether method supports nonlinear inequality constrinats
  bool supports_nonlinear_inequality() override
    { return true; }

  /// Return format for nonlinear inequality constraints
  NONLINEAR_EQUALITY_FORMAT nonlinear_equality_format() override
    { return NONLINEAR_EQUALITY_FORMAT::TRUE_EQUALITY; }

  /// Return format for nonlinear inequality constraints
  NONLINEAR_INEQUALITY_FORMAT nonlinear_inequality_format() override
    { return NONLINEAR_INEQUALITY_FORMAT::ONE_SIDED_UPPER; }

}; // class ExtPythonTraits

// -----------------------------------------------------------------

class ModelExecutor
{
  public:

    /// Constructor
    ModelExecutor(std::shared_ptr<Model> & model);

    /// model evaluator
    std::vector<double> value(std::vector<double> & x);

  private:

    // wrapped model
    std::shared_ptr<Model> model_;

}; // class ModelExecutor

} // namespace Dakota

#endif
