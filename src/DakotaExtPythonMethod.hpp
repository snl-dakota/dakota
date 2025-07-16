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
#include <pybind11/eigen.h>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
namespace py = pybind11;

// Dakota headers
#include "DakotaIterator.hpp"

namespace Dakota {

// -----------------------------------------------------------------

class ModelExecutor
{
  public:

    /// Constructor
    ModelExecutor(std::shared_ptr<Model> & model);

    /// model evaluator
    std::vector<double> value(std::vector<double> & x);

    /// compute and print response central moments
    void compute_and_print_moments(const std::vector<std::vector<double>> &);

    const Model& model() const
      { return *model_; }

  private:

    // wrapped model
    std::shared_ptr<Model> model_;

}; // class ModelExecutor

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

    /// Optional initialization
    void initialize_run() override;

    /// Optional
    void pre_run() override;

    /// REQUIRED method execution
    void core_run() override;

    /// Optional
    void post_run(std::ostream& s) override;

    /// Optional
    void finalize_run() override;

  protected:

    /// Initialize python interpreter and Method module
    void initialize_python();

    /// whether the user requested numpy data structures in the input file
    bool userNumpyFlag;
    /// true if this class created the interpreter instance
    bool ownPython;
    /// python Method class
    py::object pyMethod;

    /// Python module filename and class/function
    String moduleAndClassName;

    bool py11Active;

    std::shared_ptr<ModelExecutor> executor_;

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


} // namespace Dakota

#endif
