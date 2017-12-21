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

  /** Convenience function to convert from ROL to std::vector */
  static Teuchos::RCP<const std::vector<ScalarT>> getVector( const ROL::Vector<ScalarT>& x );

  /** Convenience function to convert from ROL to std::vector */
  static Teuchos::RCP<std::vector<ScalarT>> getVector( ROL::Vector<ScalarT>& x );

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

inline Teuchos::RCP<const std::vector<ScalarT>> ROLTraits::getVector( const ROL::Vector<ScalarT>& x )
{
  using Teuchos::dyn_cast;
  return dyn_cast<const ROL::StdVector<ScalarT>>(x).getVector();
}

inline Teuchos::RCP<std::vector<ScalarT>> ROLTraits::getVector( ROL::Vector<ScalarT>& x )
{
  using Teuchos::dyn_cast;
  return dyn_cast<ROL::StdVector<ScalarT>>(x).getVector();
}


// --------------------------------------------------------------

template<class ScalarT> 
class DakotaToROLIneqConstraints : public ROL::Constraint<ScalarT>
{

  private:

    // extract model parameters relevant for inequality constraint
    // evaluation
    void extract_model_params() {

      numContinuousVars = iteratedModel.cv();
      numNonlinIneq = iteratedModel.num_nonlinear_ineq_constraints();
      numLinearIneq = iteratedModel.num_linear_ineq_constraints();
      numNonlinEq = iteratedModel.num_nonlinear_eq_constraints();

      // BMA: I don't think we need copies at all, could just access
      // model, but for now, assign instead of looping... until discuss with MK
      linIneqCoeffs.reshape((int)numLinearIneq,(int)numContinuousVars);
      linIneqCoeffs.assign(iteratedModel.linear_ineq_constraint_coeffs());

      dakotaFns.resize((int)(1+numNonlinEq+numNonlinIneq));

      dakotaGrads.reshape((int)numContinuousVars,
          (int)(1+numNonlinEq+numNonlinIneq));

      updateCalled = false;
    }

    /// Number of continuous variables
    int numContinuousVars;

    /// Shallow copy of the model on which ROL will iterate.
    Model iteratedModel;

    /// Deep copy of linear inequaity coefficients
    RealMatrix linIneqCoeffs;

    /// Number of nonlinear inequalities
    size_t numNonlinIneq;

    /// Number of linear inequalities
    size_t numLinearIneq;

    /// Number of nonlinear equalities
    size_t numNonlinEq;

    /// Copy of latest function (objective and nonlinear constraints) values
    RealVector dakotaFns;

    /// Copy of latest function (objective and nonlinear constraints) gradient values
    RealMatrix dakotaGrads;

    /// Used in checking whether or not update has been called yet
    bool updateCalled;

  public:

    DakotaToROLIneqConstraints(Model& dakota_model): iteratedModel(dakota_model)
  {
    extract_model_params();
  }


    void update( const ROL::Vector<ScalarT> &x, bool flag, int iter ) {

      // Cout << "ROL: calling update in IneqConstraints." << std::endl;

      // Speculative model evaluation, invoked by ROL's call to update(...)
      if ( flag && (numNonlinIneq > 0)) {
        ROLTraits::set_continuous_vars(ROLTraits::getVector(x), iteratedModel);

        ActiveSet eval_set(iteratedModel.current_response().active_set());
        // eval_set.request_values(0);
        // for(size_t i=0;i<num_nln_ineq;++i)
        //   eval_set.request_value(AS_FUNC+AS_GRAD,i+1);

        // Speculative model evaluation, requesting function values and
        // gradients of all objectives and constraints
        eval_set.request_values(AS_FUNC+AS_GRAD);
       
        iteratedModel.evaluate(eval_set);

        dakotaFns.assign(iteratedModel.current_response().function_values());

        dakotaGrads.assign(iteratedModel.current_response().function_gradients());
      }

      updateCalled = true;
    }


    void value(ROL::Vector<ScalarT> &c, const ROL::Vector<ScalarT> &x, ScalarT &tol) override
    {

      // Cout << "ROL: calling value in IneqConstraints." << std::endl;

      using Teuchos::RCP;

      // Pointer to constraint vector
      RCP<std::vector<ScalarT>> cp = ROLTraits::getVector(c);

      // Pointer to optimization vector     
      RCP<const std::vector<ScalarT>> xp = ROLTraits::getVector(x);

      for(size_t i=0;i<numLinearIneq;++i) {
        (*cp)[i] = 0.0;
        for (size_t j=0; j<numContinuousVars; j++)
          (*cp)[i] += linIneqCoeffs(i,j) * (*xp)[j];
      }

      if (numNonlinIneq > 0) {

        // makes sure that update(...) is called prior to first value(...) call
        if (!updateCalled)
          update( x, true, 0 );

        for(size_t i=0;i<numNonlinIneq;++i)
          (*cp)[i+numLinearIneq] = dakotaFns[i+1];
      }
    }

    void applyJacobian(ROL::Vector<ScalarT> &jv,
        const ROL::Vector<ScalarT> &v, const ROL::Vector<ScalarT> &x, ScalarT &tol){

      // Cout << "ROL: calling applyJacobian in IneqConstraints." << std::endl;


      using Teuchos::RCP;

      // Pointer to optimization vector     
      RCP<const std::vector<ScalarT>> xp = ROLTraits::getVector(x);

      // Pointer to jv vector
      RCP<std::vector<ScalarT>> jvp = ROLTraits::getVector(jv);

      // apply linear constraint Jacobian
      for(size_t i=0;i<numLinearIneq;++i) {
        (*jvp)[i] = 0.0;
        for (size_t j=0; j<numContinuousVars; j++)
          (*jvp)[i] += linIneqCoeffs(i,j) * (*xp)[j];
      }

      // apply nonlinear constraint Jacobian
      if (numNonlinIneq > 0) {

        // makes sure that update(...) is called prior to first applyJacobian(...) call
        if (!updateCalled)
          update( x, true, 0 );

        for(size_t i=0;i<numNonlinIneq;++i){
          (*jvp)[i+numLinearIneq] = 0.0;
          for (size_t j=0; j<numContinuousVars; j++)
            (*jvp)[i+numLinearIneq] += dakotaGrads(j,i+1) * (*xp)[j];
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

template<class ScalarT> 
class DakotaToROLEqConstraints : public ROL::Constraint<ScalarT>
{

  private:

    // extract model parameters relevant for equality constraint
    // evaluation
    void extract_model_params() {

      numContinuousVars = iteratedModel.cv();
      numNonlinEq = iteratedModel.num_nonlinear_eq_constraints();
      numNonlinIneq = iteratedModel.num_nonlinear_ineq_constraints();
      numLinearEq = iteratedModel.num_linear_eq_constraints();

      // BMA: I don't think we need copies at all, could just access
      // model, but for now, assign instead of looping... until discuss with MK
      linEqCoeffs.reshape((int)numLinearEq,(int)numContinuousVars);
      linEqCoeffs.assign(iteratedModel.linear_eq_constraint_coeffs());
      linEqTargets.resize((int)numLinearEq);
      linEqTargets.assign(iteratedModel.linear_eq_constraint_targets());
      nonlinEqTargets.resize((int)numNonlinEq);
      nonlinEqTargets.assign(iteratedModel.nonlinear_eq_constraint_targets());

      dakotaFns.resize((int)(1+numNonlinEq+numNonlinIneq));

      dakotaGrads.reshape((int)numContinuousVars,
          (int)(1+numNonlinEq+numNonlinIneq));

      updateCalled = false;
    }

    /// Number of continuous variables
    int numContinuousVars;

    /// Shallow copy of the model on which ROL will iterate.
    Model iteratedModel;

    /// Deep copy of linear equaity coefficients
    RealMatrix linEqCoeffs;

    /// Deep copy of linear equaity targets
    RealVector linEqTargets;

    /// Deep copy of nonlinear equaity targets
    RealVector nonlinEqTargets;

    /// Number of nonlinear equalities
    size_t numNonlinEq;

    /// Number of nonlinear inequalities
    size_t numNonlinIneq;

    /// Number of linear equalities
    size_t numLinearEq;

    /// Copy of latest function (objective and nonlinear constraints) values
    RealVector dakotaFns;

    /// Copy of latest function (objective and nonlinear constraints) gradient values
    RealMatrix dakotaGrads;

    /// Used in checking whether or not update has been called yet
    bool updateCalled;

  public:

    DakotaToROLEqConstraints(Model& dakota_model): iteratedModel(dakota_model)
  {
    extract_model_params();
  }

    // BMA TODO: don't we now have data adapters that convert linear to
    // nonlinear constraints and manage the indexing?
    void update( const ROL::Vector<ScalarT> &x, bool flag, int iter ) {

      // Cout << "ROL: calling update in EqConstraints." << std::endl;

      // Speculative model evaluation, invoked by ROL's call to update(...)
      if ( flag && (numNonlinEq > 0)) {
        ROLTraits::set_continuous_vars(ROLTraits::getVector(x), iteratedModel);

        ActiveSet eval_set(iteratedModel.current_response().active_set());
        // eval_set.request_values(0);
        // for(size_t i=0;i<num_nln_eq;++i)
        //   eval_set.request_value(AS_FUNC+AS_GRAD,i+1+num_nln_ineq);

        // Speculative model evaluation, requesting function values and
        // gradients of all objectives and constraints
        eval_set.request_values(AS_FUNC+AS_GRAD);

        iteratedModel.evaluate(eval_set);

        dakotaFns.assign(iteratedModel.current_response().function_values());

        dakotaGrads.assign(iteratedModel.current_response().function_gradients());
      }

      updateCalled = true;
    }

    void value(ROL::Vector<ScalarT> &c, const ROL::Vector<ScalarT> &x, ScalarT &tol) override
    {

      // Cout << "ROL: calling value in EqConstraints." << std::endl;

      using Teuchos::RCP;

      // Pointer to constraint vector
      RCP<std::vector<ScalarT>> cp = ROLTraits::getVector(c);

      // Pointer to optimization vector     
      RCP<const std::vector<ScalarT>> xp = ROLTraits::getVector(x);

      for(size_t i=0;i<numLinearEq;++i) {
        (*cp)[i] = -linEqTargets(i);
        for (size_t j=0; j<numContinuousVars; j++)
          (*cp)[i] += linEqCoeffs(i,j) * (*xp)[j];
      }

      if (numNonlinEq > 0) {

        // makes sure that update(...) is called prior to first value(...) call
        if (!updateCalled)
          update( x, true, 0 );

        for(size_t i=0;i<numNonlinEq;++i)
          (*cp)[i+numLinearEq] = -nonlinEqTargets(i)+dakotaFns[i+1+numNonlinIneq];
      }

    }

    void applyJacobian(ROL::Vector<ScalarT> &jv,
        const ROL::Vector<ScalarT> &v, const ROL::Vector<ScalarT> &x, ScalarT &tol){

      // Cout << "ROL: calling applyJacobian in EqConstraints." << std::endl;

      using Teuchos::RCP;

      // Pointer to optimization vector     
      RCP<const std::vector<ScalarT>> xp = ROLTraits::getVector(x);

      // Pointer to jv vector
      RCP<std::vector<ScalarT>> jvp = ROLTraits::getVector(jv);

      // apply linear constraint Jacobian
      for(size_t i=0;i<numLinearEq;++i) {
        (*jvp)[i] = 0.0;
        for (size_t j=0; j<numContinuousVars; j++)
          (*jvp)[i] += linEqCoeffs(i,j) * (*xp)[j];
      }

      // apply nonlinear constraint Jacobian
      if (numNonlinEq > 0) {

        // makes sure that update(...) is called prior to first applyJacobian(...) call
        if (!updateCalled)
          update( x, true, 0 );

        for(size_t i=0;i<numNonlinEq;++i){
          (*jvp)[i+numLinearEq] = 0.0;
          for (size_t j=0; j<numContinuousVars; j++)
            (*jvp)[i+numLinearEq] += dakotaGrads(j,i+1+numNonlinIneq) * (*xp)[j];
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

template<class ScalarT> 
class DakotaToROLObjective : public ROL::Objective<ScalarT>
{

  private:

    // extract model parameters relevant for objective
    // function evaluation
    void extract_model_params() {
      numContinuousVars = iteratedModel.cv();

      numNonlinEq = iteratedModel.num_nonlinear_eq_constraints();
      numNonlinIneq = iteratedModel.num_nonlinear_ineq_constraints();

      updateCalled = false;
    }

    /// Number of continuous variables
    int numContinuousVars;

    /// Shallow copy of the model on which ROL will iterate.
    Model iteratedModel;

    /// Copy of latest obective function values
    ScalarT fnVal;

    /// Copy of latest function (objective and nonlinear constraints) gradient values
    RealMatrix dakotaGrads;

    /// Number of nonlinear equalities
    size_t numNonlinEq;

    /// Number of nonlinear inequalities
    size_t numNonlinIneq;

    /// Used in checking whether or not update has been called yet
    bool updateCalled;

  public:

    DakotaToROLObjective(Model& dakota_model): iteratedModel(dakota_model)
  {
    extract_model_params();
  }

    void update( const ROL::Vector<ScalarT> &x, bool flag, int iter ) {

      // Cout << "ROL: calling update in Objective." << std::endl;

      // Speculative model evaluation, invoked by ROL's call to update(...)
      if ( flag ) {
        ROLTraits::set_continuous_vars(ROLTraits::getVector(x), iteratedModel);

        ActiveSet eval_set(iteratedModel.current_response().active_set());
        // eval_set.request_values(0);
        // eval_set.request_value(AS_FUNC+AS_GRAD,0);

        // Speculative model evaluation, requesting function values and
        // gradients of all objectives and constraints
        eval_set.request_values(AS_FUNC+AS_GRAD);

        iteratedModel.evaluate(eval_set);

        fnVal = iteratedModel.current_response().function_value(0);

        dakotaGrads.reshape((int)numContinuousVars,
            (int)(1+numNonlinEq+numNonlinIneq));
        dakotaGrads.assign(iteratedModel.current_response().function_gradients());
      }

      updateCalled = true;
    }

    ScalarT value(const ROL::Vector<ScalarT> &x, ScalarT &tol) override
    {
      // Cout << "ROL: calling value in Objective." << std::endl;

      // makes sure that update(...) is called prior to first value(...) call
      if (!updateCalled)
        update( x, true, 0 );

      return fnVal;
    }

    void gradient( ROL::Vector<ScalarT> &g, const ROL::Vector<ScalarT> &x, ScalarT &tol ) {

      // Cout << "ROL: calling gradient in Objective." << std::endl;
   
      // makes sure that update(...) is called prior to first value(...) call
      if (!updateCalled)
        update( x, true, 0 );

      Teuchos::RCP<std::vector<ScalarT>> gp = ROLTraits::getVector(g);
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
