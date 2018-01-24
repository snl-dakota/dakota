/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ROLOptimizer
//- Description: Implementation of the ROLOptimizer class, Only able to
//               call line search optimization for now (i.e. ROLOptimizer
//               is single-method TPL for this iterations)
//- Owner:       Moe Khalil
//- Checked by:
//- Version: $Id$

#ifndef ROL_OPTIMIZER_H
#define ROL_OPTIMIZER_H

#include "DakotaOptimizer.hpp"
#include "DakotaModel.hpp"
#include "ROL_StdObjective.hpp"
#include "ROL_StdConstraint.hpp"
#include "ROL_OptimizationSolver.hpp"

#include "DakotaTraitsBase.hpp"

namespace Dakota {

  enum {AS_FUNC=1, AS_GRAD=2, AS_HESS=4};

/** ROLTraits specializes some traits accessors by over-riding the default 
accessors in TraitsBase. */

class ROLTraits: public TraitsBase
{
  public:

  /// default constructor
  ROLTraits() { }

  typedef std::vector<Real> VecT;

  /// destructor
  virtual ~ROLTraits() { }

  /// A temporary query used in the refactor
  virtual bool is_derived() { return true; }

  /// Return the flag indicating whether method supports continuous variables
  bool supports_continuous_variables() { return true; }

  /// Return the flag indicating whether method supports linear equalities
  bool supports_linear_equality() { return true; }

  /// Return the flag indicating whether method supports linear inequalities
  bool supports_linear_inequality() { return true; }

  /// Return the flag indicating whether method supports nonlinear equalities
  bool supports_nonlinear_equality() { return true; }

  /// Return the format used for nonlinear equality constraints
  NONLINEAR_EQUALITY_FORMAT nonlinear_equality_format()
   { return NONLINEAR_EQUALITY_FORMAT::TPL_MANAGED; }

  /// Return the flag indicating whether method supports nonlinear inequalities
  bool supports_nonlinear_inequality() { return true; }

  /// Return the format used for nonlinear inequality constraints
  NONLINEAR_INEQUALITY_FORMAT nonlinear_inequality_format()
   { return NONLINEAR_INEQUALITY_FORMAT::TWO_SIDED; }

};


// --------------------------------------------------------------

/// A helper function for consolidating model callbacks
namespace {

  void update_model(Model & model, const std::vector<Real> & x)
  {
    // Could replace with an adapter call - RWH
    size_t num_cv = model.cv();
    for(size_t i=0; i<num_cv; ++i)
      model.continuous_variable(x[i], i);

    ActiveSet eval_set(model.current_response().active_set());
    eval_set.request_values(AS_FUNC+AS_GRAD);
    model.evaluate(eval_set);

    // now we can use the response currently in the model for any
    // obj/cons/grad/hess
  }

}


// --------------------------------------------------------------

class DakotaROLIneqConstraints : public ROL::StdConstraint<Real>
{

  public:

    DakotaROLIneqConstraints(Model & model) :
      dakotaModel(model)
    { }


    void value(std::vector<Real> &c, const std::vector<Real> &x, Real &tol) override
    {
      update_model(dakotaModel, x);
      apply_linear_constraints( dakotaModel, CONSTRAINT_EQUALITY_TYPE::INEQUALITY, x, c );
      get_nonlinear_ineq_constraints( dakotaModel, c );
    }

    void applyJacobian(std::vector<Real> &jv,
        const std::vector<Real> &v, const std::vector<Real> &x, Real &tol) override
    {
      size_t num_continuous_vars = dakotaModel.cv();
      size_t num_linear_ineq = dakotaModel.num_linear_ineq_constraints();
      size_t num_nonlinear_ineq = dakotaModel.num_nonlinear_ineq_constraints();
      const RealMatrix & lin_ineq_coeffs = dakotaModel.linear_ineq_constraint_coeffs();
      const RealMatrix & gradient_matrix = dakotaModel.current_response().function_gradients();

      // apply linear constraint Jacobian
      apply_matrix(lin_ineq_coeffs, v, jv);

      // apply nonlinear constraint Jacobian
      if (num_nonlinear_ineq > 0) {

        // makes sure that model is current
        update_model(dakotaModel, x);

        for(size_t i=0;i<num_nonlinear_ineq;++i){
          jv[i+num_linear_ineq] = 0.0;
          for (size_t j=0; j<num_continuous_vars; j++)
            jv[i+num_linear_ineq] += gradient_matrix(j,i+1) * v[j];
        }
      }
    }

  private:

    Model & dakotaModel;

}; // class DakotaROLIneqConstraints

// --------------------------------------------------------------

class DakotaROLEqConstraints : public ROL::StdConstraint<Real>
{

  public:

    DakotaROLEqConstraints(Model & model) :
      dakotaModel(model)
    { }

    void value(std::vector<Real> &c, const std::vector<Real> &x, Real &tol) override
    {
      update_model(dakotaModel, x);
      apply_linear_constraints( dakotaModel, CONSTRAINT_EQUALITY_TYPE::EQUALITY, x, c );
      get_nonlinear_eq_constraints( dakotaModel, c, -1.0 );
    }

    void applyJacobian(std::vector<Real> &jv,
        const std::vector<Real> &v, const std::vector<Real> &x, Real &tol) override
    {
      size_t num_continuous_vars = dakotaModel.cv();
      size_t num_linear_eq = dakotaModel.num_linear_eq_constraints();
      size_t num_nonlinear_ineq = dakotaModel.num_nonlinear_ineq_constraints();
      size_t num_nonlinear_eq = dakotaModel.num_nonlinear_eq_constraints();
      const RealMatrix & lin_eq_coeffs = dakotaModel.linear_eq_constraint_coeffs();
      const RealMatrix & gradient_matrix = dakotaModel.current_response().function_gradients();

      // apply linear constraint Jacobian
      apply_matrix(lin_eq_coeffs, v, jv);

      // apply nonlinear constraint Jacobian
      if (num_nonlinear_eq > 0) {

        // makes sure that model is current
        update_model(dakotaModel, x);

        for(size_t i=0;i<num_nonlinear_eq;++i){
          jv[i+num_linear_eq] = 0.0;
          for (size_t j=0; j<num_continuous_vars; j++)
            jv[i+num_linear_eq] += gradient_matrix(j,i+1+num_nonlinear_ineq) * v[j];
        }
      }
    }

  private:

    Model & dakotaModel;

}; // class DakotaROLEqConstraints


// --------------------------------------------------------------

class DakotaROLObjective : public ROL::StdObjective<Real>
{

  public:

    DakotaROLObjective(Model & model) :
      dakotaModel(model)
  { }

    Real value(const std::vector<Real> &x, Real &tol) override
    {
      update_model(dakotaModel, x);
      return dakotaModel.current_response().function_value(0);
    }

    void gradient( std::vector<Real> &g, const std::vector<Real> &x, Real &tol ) override
    {
      update_model(dakotaModel, x);
      copy_column_vector(dakotaModel.current_response().function_gradients(), 0, g);
    }

  private:

    Model & dakotaModel;

}; // class DakotaROLObjective

// --------------------------------------------------------------


class ROLOptimizer : public Optimizer
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  ROLOptimizer(ProblemDescDB& problem_db, Model& model);

  /// alternate constructor for Iterator instantiations by name
  ROLOptimizer(const String& method_name, Model& model);
  
  /// destructor
  ~ROLOptimizer() {}

  //
  //- Heading: Virtual member function redefinitions
  //

  /// iterates the ROL solver to determine the optimal solution
  void core_run();

protected:

  //
  //- Heading: constructor convenience member functions
  //

  /// Convenience function to map Dakota input and power-user parameters to ROL
  void set_rol_parameters();

  /// Helper function called after default construction to extract problem
  /// information from the Model and set it for ROL.
  void set_problem();

  //
  //- Heading: Data
  //

  /// Parameters for the ROL::OptimizationSolver
  Teuchos::ParameterList optSolverParams;

  /// Handle to ROL::OptimizationProblem, part of ROL's simplified interface 
  ROL::OptimizationProblem<Real> optProblem;

  /// Handle to ROL's solution vector 
  Teuchos::RCP<std::vector<Real> > rolX;
};

} // namespace Dakota

#endif
