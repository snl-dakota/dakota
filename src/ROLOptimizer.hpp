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
#include "ROL_OptimizationSolver.hpp"

#include "DakotaTraitsBase.hpp"

namespace Dakota {

  typedef double ScalarT;

  enum {AS_FUNC=1, AS_GRAD=2, AS_HESS=4};

/** ROLTraits specializes some traits accessors by over-riding the default 
accessors in TraitsBase. */

class ROLTraits: public TraitsBase
{
  public:

  /// default constructor
  ROLTraits();

  // TODO: data transfers cleanup once decide on std vs Teuchos
  /** Convenience function to set the variables from ROL into a Dakota Model */
  static void set_continuous_vars(Teuchos::RCP<const std::vector<Real>> x, Model& model);

  /// destructor
  virtual ~ROLTraits();

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

inline ROLTraits::ROLTraits()
{ }

inline ROLTraits::~ROLTraits()
{ }

inline void ROLTraits::set_continuous_vars(Teuchos::RCP<const std::vector<Real>> x, Model& model)
{
  size_t num_cv = model.cv();
  for(size_t i=0; i<num_cv; ++i)
    model.continuous_variable((*x)[i], i);
}

// --------------------------------------------------------------

template<class ScalarT> 
class DakotaToROLIneqConstraints : public ROL::Constraint<ScalarT>
{

  private:

    Teuchos::RCP<const std::vector<ScalarT>> getVector( const ROL::Vector<ScalarT>& x ) {
      using Teuchos::dyn_cast;
      return dyn_cast<const ROL::StdVector<ScalarT>>(x).getVector();
    }

    Teuchos::RCP<std::vector<ScalarT>> getVector( ROL::Vector<ScalarT>& x ) {
      using Teuchos::dyn_cast;
      return dyn_cast<ROL::StdVector<ScalarT>>(x).getVector();
    }

    // extract model parameters relevant for inequality constraint
    // evaluation
    void extract_model_params() {

      numContinuousVars = iteratedModel.cv();
      num_nln_ineq = iteratedModel.num_nonlinear_ineq_constraints();
      num_lin_ineq = iteratedModel.num_linear_ineq_constraints();
      num_nln_eq = iteratedModel.num_nonlinear_eq_constraints();

      // BMA: I don't think we need copies at all, could just access
      // model, but for now, assign instead of looping... until discuss with MK
      lin_ineq_coeffs.reshape((int)num_lin_ineq,(int)numContinuousVars);
      lin_ineq_coeffs.assign(iteratedModel.linear_ineq_constraint_coeffs());

      dakotaFns.resize((int)(1+num_nln_eq+num_nln_ineq));

      dakotaGrads.reshape((int)numContinuousVars,
          (int)(1+num_nln_eq+num_nln_ineq));

      update_called = false;
    }

    /// Number of continuous variables
    int numContinuousVars;

    /// Shallow copy of the model on which ROL will iterate.
    Model iteratedModel;

    /// Deep copy of linear inequaity coefficients
    RealMatrix lin_ineq_coeffs;

    /// Number of nonlinear inequalities
    size_t num_nln_ineq;

    /// Number of linear inequalities
    size_t num_lin_ineq;

    /// Number of nonlinear equalities
    size_t num_nln_eq;

    /// Copy of latest function (objective and nonlinear constraints) values
    RealVector dakotaFns;

    /// Copy of latest function (objective and nonlinear constraints) gradient values
    RealMatrix dakotaGrads;

    /// Used in checking whether or not update has been called yet
    bool update_called;

  public:

    DakotaToROLIneqConstraints(Model& dakota_model): iteratedModel(dakota_model)
  {
    extract_model_params();
  }


    void update( const ROL::Vector<ScalarT> &x, bool flag, int iter ) {

      // Speculative model evaluation, invoked by ROL's call to update(...)
      if ( flag && (num_nln_ineq > 0)) {
        ROLTraits::set_continuous_vars(getVector(x), iteratedModel);

        ActiveSet eval_set(iteratedModel.current_response().active_set());
        eval_set.request_values(0);
        for(size_t i=0;i<num_nln_ineq;++i)
          eval_set.request_value(AS_FUNC+AS_GRAD,i+1);

        iteratedModel.evaluate(eval_set);

        dakotaFns.assign(iteratedModel.current_response().function_values());

        dakotaGrads.assign(iteratedModel.current_response().function_gradients());
      }

      update_called = true;
    }


    void value(ROL::Vector<ScalarT> &c, const ROL::Vector<ScalarT> &x, ScalarT &tol){

      using Teuchos::RCP;

      // Pointer to constraint vector
      RCP<std::vector<ScalarT>> cp = getVector(c);

      // Pointer to optimization vector     
      RCP<const std::vector<ScalarT>> xp = getVector(x);

      for(size_t i=0;i<num_lin_ineq;++i) {
        (*cp)[i] = 0.0;
        for (size_t j=0; j<numContinuousVars; j++)
          (*cp)[i] += lin_ineq_coeffs(i,j) * (*xp)[j];
      }

      if (num_nln_ineq > 0) {

        // makes sure that update(...) is called prior to first value(...) call
        if (!update_called)
          update( x, true, 0 );

        for(size_t i=0;i<num_nln_ineq;++i)
          (*cp)[i+num_lin_ineq] = dakotaFns[i+1];
      }
    }

    void applyJacobian(ROL::Vector<ScalarT> &jv,
        const ROL::Vector<ScalarT> &v, const ROL::Vector<ScalarT> &x, ScalarT &tol){

      using Teuchos::RCP;

      // Pointer to optimization vector     
      RCP<const std::vector<ScalarT>> xp = getVector(x);

      // Pointer to jv vector
      RCP<std::vector<ScalarT>> jvp = getVector(jv);

      // apply linear constraint Jacobian
      for(size_t i=0;i<num_lin_ineq;++i) {
        (*jvp)[i] = 0.0;
        for (size_t j=0; j<numContinuousVars; j++)
          (*jvp)[i] += lin_ineq_coeffs(i,j) * (*xp)[j];
      }

      // apply nonlinear constraint Jacobian
      if (num_nln_ineq > 0) {

        // makes sure that update(...) is called prior to first applyJacobian(...) call
        if (!update_called)
          update( x, true, 0 );

        for(size_t i=0;i<num_nln_ineq;++i){
          (*jvp)[i+num_lin_ineq] = 0.0;
          for (size_t j=0; j<numContinuousVars; j++)
            (*jvp)[i+num_lin_ineq] += dakotaGrads(j,i+1) * (*xp)[j];
        }
      }

    }

    // provide access to Dakota model
    void pass_model(Model& model) {

      iteratedModel = model;

      extract_model_params();
    }

}; // class DakotaToROLIneqConstraints

// --------------------------------------------------------------

template<class Real> 
class DakotaToROLEqConstraints : public ROL::Constraint<Real>
{

  private:

    Teuchos::RCP<const std::vector<Real>> getVector( const ROL::Vector<Real>& x ) {
      using Teuchos::dyn_cast;
      return dyn_cast<const ROL::StdVector<Real>>(x).getVector();
    }

    Teuchos::RCP<std::vector<Real>> getVector( ROL::Vector<Real>& x ) {
      using Teuchos::dyn_cast;
      return dyn_cast<ROL::StdVector<Real>>(x).getVector();
    }

    // extract model parameters relevant for equality constraint
    // evaluation
    void extract_model_params() {

      numContinuousVars = iteratedModel.cv();
      num_nln_eq = iteratedModel.num_nonlinear_eq_constraints();
      num_nln_ineq = iteratedModel.num_nonlinear_ineq_constraints();
      num_lin_eq = iteratedModel.num_linear_eq_constraints();

      // BMA: I don't think we need copies at all, could just access
      // model, but for now, assign instead of looping... until discuss with MK
      lin_eq_coeffs.reshape((int)num_lin_eq,(int)numContinuousVars);
      lin_eq_coeffs.assign(iteratedModel.linear_eq_constraint_coeffs());
      lin_eq_targets.resize((int)num_lin_eq);
      lin_eq_targets.assign(iteratedModel.linear_eq_constraint_targets());
      nln_eq_targets.resize((int)num_nln_eq);
      nln_eq_targets.assign(iteratedModel.nonlinear_eq_constraint_targets());

      dakotaFns.resize((int)(1+num_nln_eq+num_nln_ineq));

      dakotaGrads.reshape((int)numContinuousVars,
          (int)(1+num_nln_eq+num_nln_ineq));

      update_called = false;
    }

    /// Number of continuous variables
    int numContinuousVars;

    /// Shallow copy of the model on which ROL will iterate.
    Model iteratedModel;

    /// Deep copy of linear equaity coefficients
    RealMatrix lin_eq_coeffs;

    /// Deep copy of linear equaity targets
    RealVector lin_eq_targets;

    /// Deep copy of nonlinear equaity targets
    RealVector nln_eq_targets;

    /// Number of nonlinear equalities
    size_t num_nln_eq;

    /// Number of nonlinear inequalities
    size_t num_nln_ineq;

    /// Number of linear equalities
    size_t num_lin_eq;

    /// Copy of latest function (objective and nonlinear constraints) values
    RealVector dakotaFns;

    /// Copy of latest function (objective and nonlinear constraints) gradient values
    RealMatrix dakotaGrads;

    /// Used in checking whether or not update has been called yet
    bool update_called;

  public:

    DakotaToROLEqConstraints(Model& dakota_model): iteratedModel(dakota_model)
  {
    extract_model_params();
  }

    // BMA TODO: don't we now have data adapters that convert linear to
    // nonlinear constraints and manage the indexing?
    void update( const ROL::Vector<Real> &x, bool flag, int iter ) {

      // Speculative model evaluation, invoked by ROL's call to update(...)
      if ( flag && (num_nln_eq > 0)) {
        ROLTraits::set_continuous_vars(getVector(x), iteratedModel);

        ActiveSet eval_set(iteratedModel.current_response().active_set());
        eval_set.request_values(0);
        for(size_t i=0;i<num_nln_eq;++i)
          eval_set.request_value(AS_FUNC+AS_GRAD,i+1+num_nln_ineq);

        iteratedModel.evaluate(eval_set);

        dakotaFns.assign(iteratedModel.current_response().function_values());

        dakotaGrads.assign(iteratedModel.current_response().function_gradients());
      }

      update_called = true;
    }

    void value(ROL::Vector<Real> &c, const ROL::Vector<Real> &x, Real &tol){

      using Teuchos::RCP;

      // Pointer to constraint vector
      RCP<std::vector<Real>> cp = getVector(c);

      // Pointer to optimization vector     
      RCP<const std::vector<Real>> xp = getVector(x);

      for(size_t i=0;i<num_lin_eq;++i) {
        (*cp)[i] = -lin_eq_targets(i);
        for (size_t j=0; j<numContinuousVars; j++)
          (*cp)[i] += lin_eq_coeffs(i,j) * (*xp)[j];
      }

      if (num_nln_eq > 0) {

        // makes sure that update(...) is called prior to first value(...) call
        if (!update_called)
          update( x, true, 0 );

        for(size_t i=0;i<num_nln_eq;++i)
          (*cp)[i+num_lin_eq] = -nln_eq_targets(i)+dakotaFns[i+1+num_nln_ineq];
      }

    }

    void applyJacobian(ROL::Vector<Real> &jv,
        const ROL::Vector<Real> &v, const ROL::Vector<Real> &x, Real &tol){

      using Teuchos::RCP;

      // Pointer to optimization vector     
      RCP<const std::vector<Real>> xp = getVector(x);

      // Pointer to jv vector
      RCP<std::vector<Real>> jvp = getVector(jv);

      // apply linear constraint Jacobian
      for(size_t i=0;i<num_lin_eq;++i) {
        (*jvp)[i] = 0.0;
        for (size_t j=0; j<numContinuousVars; j++)
          (*jvp)[i] += lin_eq_coeffs(i,j) * (*xp)[j];
      }

      // apply nonlinear constraint Jacobian
      if (num_nln_eq > 0) {

        // makes sure that update(...) is called prior to first applyJacobian(...) call
        if (!update_called)
          update( x, true, 0 );

        for(size_t i=0;i<num_nln_eq;++i){
          (*jvp)[i+num_lin_eq] = 0.0;
          for (size_t j=0; j<numContinuousVars; j++)
            (*jvp)[i+num_lin_eq] += dakotaGrads(j,i+1+num_nln_ineq) * (*xp)[j];
        }

      }

    }

    // provide access to Dakota model
    void pass_model(Model& model) {

      iteratedModel = model;

      extract_model_params();
    }

}; // class DakotaToROLEqConstraints


// --------------------------------------------------------------

template<class Real> 
class DakotaToROLObjective : public ROL::Objective<Real>
{

  private:

    Teuchos::RCP<const std::vector<Real>> getVector( const ROL::Vector<Real>& x ) {
      using Teuchos::dyn_cast;
      return dyn_cast<const ROL::StdVector<Real>>(x).getVector();
    }

    Teuchos::RCP<std::vector<Real>> getVector( ROL::Vector<Real>& x ) {
      using Teuchos::dyn_cast;
      return dyn_cast<ROL::StdVector<Real>>(x).getVector();
    }

    // extract model parameters relevant for objective
    // function evaluation
    void extract_model_params() {
      numContinuousVars = iteratedModel.cv();

      num_nln_eq = iteratedModel.num_nonlinear_eq_constraints();
      num_nln_ineq = iteratedModel.num_nonlinear_ineq_constraints();

      update_called = false;
    }

    /// Number of continuous variables
    int numContinuousVars;

    /// Shallow copy of the model on which ROL will iterate.
    Model iteratedModel;

    /// Copy of latest obective function values
    Real fnVal;

    /// Copy of latest function (objective and nonlinear constraints) gradient values
    RealMatrix dakotaGrads;

    /// Number of nonlinear equalities
    size_t num_nln_eq;

    /// Number of nonlinear inequalities
    size_t num_nln_ineq;

    /// Used in checking whether or not update has been called yet
    bool update_called;

  public:

    DakotaToROLObjective(Model& dakota_model): iteratedModel(dakota_model)
  {
    extract_model_params();
  }

    void update( const ROL::Vector<Real> &x, bool flag, int iter ) {

      // Speculative model evaluation, invoked by ROL's call to update(...)
      if ( flag ) {
        ROLTraits::set_continuous_vars(getVector(x), iteratedModel);

        ActiveSet eval_set(iteratedModel.current_response().active_set());
        eval_set.request_values(0);
        eval_set.request_value(AS_FUNC+AS_GRAD,0);

        iteratedModel.evaluate(eval_set);

        fnVal = iteratedModel.current_response().function_value(0);

        dakotaGrads.reshape((int)numContinuousVars,
            (int)(1+num_nln_eq+num_nln_ineq));
        dakotaGrads.assign(iteratedModel.current_response().function_gradients());
      }

      update_called = true;
    }

    Real value(const ROL::Vector<Real> &x, Real &tol) {

      // makes sure that update(...) is called prior to first value(...) call
      if (!update_called)
        update( x, true, 0 );

      return fnVal;
    }

    void gradient( ROL::Vector<Real> &g, const ROL::Vector<Real> &x, Real &tol ) {

      // makes sure that update(...) is called prior to first value(...) call
      if (!update_called)
        update( x, true, 0 );

      Teuchos::RCP<std::vector<Real>> gp = getVector(g);
      for (int i=0; i<numContinuousVars; ++i)
        (*gp)[i] = dakotaGrads(i, 0);
    }

    // provide access to Dakota model
    void pass_model(Model& model) {

      iteratedModel = model;

      extract_model_params();
    }

}; // class DakotaToROLObjective

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
  ROL::OptimizationProblem<ScalarT> optProblem;

  /// Handle to ROL's solution vector 
  Teuchos::RCP<std::vector<ScalarT> > rolX;
};

} // namespace Dakota

#endif
