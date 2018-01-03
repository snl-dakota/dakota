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

  enum {AS_FUNC=1, AS_GRAD=2, AS_HESS=4};

/** ROLTraits specializes some traits accessors by over-riding the default 
accessors in TraitsBase. */

class ROLTraits: public TraitsBase
{
  public:

  /// default constructor
  ROLTraits();

  typedef std::vector<Real> VecT;

  // TODO: data transfers cleanup once decide on std vs Teuchos
  /** Convenience function to set the variables from ROL into a Dakota Model */
  static void set_continuous_vars(Teuchos::RCP<const std::vector<Real>> x, Model& model);

  /** Convenience function to convert from ROL to std::vector */
  static Teuchos::RCP<const std::vector<Real>> getVector( const ROL::Vector<Real>& x );

  /** Convenience function to convert from ROL to std::vector */
  static Teuchos::RCP<std::vector<Real>> getVector( ROL::Vector<Real>& x );

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

inline Teuchos::RCP<const std::vector<Real>> ROLTraits::getVector( const ROL::Vector<Real>& x )
{
  using Teuchos::dyn_cast;
  return dyn_cast<const ROL::StdVector<Real>>(x).getVector();
}

inline Teuchos::RCP<std::vector<Real>> ROLTraits::getVector( ROL::Vector<Real>& x )
{
  using Teuchos::dyn_cast;
  return dyn_cast<ROL::StdVector<Real>>(x).getVector();
}


// --------------------------------------------------------------

class DakotaROLModelWrapper
{
  public:

    ~DakotaROLModelWrapper()
    {
      Cout << "~DakotaROLModelWrapper() : numUpdateCalled = " << numUpdateCalled << std::endl;
      Cout << "~DakotaROLModelWrapper() : numMismatchedUpdatesCalled = " << numMismatchedUpdatesCalled << std::endl;
    }

    DakotaROLModelWrapper(Model& dakota_model) :
      wrappedModel(dakota_model),
      lastIterUpdateCalled(-1),
      numMismatchedUpdatesCalled(0),
      numUpdateCalled(0)
    {
      extract_model_params();
    }

    void check_model_is_updated( const ROL::Vector<Real> &x)
    {
      Teuchos::RCP<const std::vector<Real>> test_vec = ROLTraits::getVector(x);
      if( (*test_vec) != lastUpdateIterate )
      {
        numMismatchedUpdatesCalled++;
        update_model(x, true, 0);
      }
    }

    void update_model( const ROL::Vector<Real> &x, bool flag, int rol_iter )
    {
      if ( flag )
      {
        ROLTraits::set_continuous_vars(ROLTraits::getVector(x), wrappedModel);

        ActiveSet eval_set(wrappedModel.current_response().active_set());
        eval_set.request_values(AS_FUNC+AS_GRAD);

        wrappedModel.evaluate(eval_set);

        fnVal = wrappedModel.current_response().function_value(0);

        dakotaFns.resize((int)(1+numNonlinEq+numNonlinIneq));
        dakotaFns.assign(wrappedModel.current_response().function_values());

        dakotaGrads.reshape((int)numContinuousVars, (int)(1+numNonlinEq+numNonlinIneq));
        dakotaGrads.assign(wrappedModel.current_response().function_gradients());
        lastIterUpdateCalled = rol_iter;
        numUpdateCalled++;
      }
    }

    Real fn_value()
      { return fnVal; }

    void grad_value( ROL::Vector<Real> &g, const ROL::Vector<Real> &x, Real &tol )
    {
      Teuchos::RCP<std::vector<Real>> gp = ROLTraits::getVector(g);
      for (int i=0; i<numContinuousVars; ++i)
        (*gp)[i] = dakotaGrads(i, 0);
    }

    const Model & get_wrapped_model() const
      { return wrappedModel; }

    size_t get_num_continuous_vars() const
      { return numContinuousVars; }

    size_t get_num_linear_eq() const
      { return numLinearEq; }

    size_t get_num_linear_ineq() const
      { return numLinearIneq; }

    size_t get_num_nonlinear_eq() const
      { return numNonlinEq; }

    size_t get_num_nonlinear_ineq() const
      { return numNonlinIneq; }

    const RealMatrix & get_lin_eq_coeffs() const
      { return linEqCoeffs; }

    const RealMatrix & get_lin_ineq_coeffs() const
      { return linIneqCoeffs; }

    const RealMatrix & get_gradient_matrix() const
      { return dakotaGrads; }

  private:

    // extract model parameters relevant for inequality constraint evaluation
    void extract_model_params()
    {
      numContinuousVars = wrappedModel.cv();
      numNonlinIneq = wrappedModel.num_nonlinear_ineq_constraints();
      numLinearIneq = wrappedModel.num_linear_ineq_constraints();
      numNonlinEq = wrappedModel.num_nonlinear_eq_constraints();
      numLinearEq = wrappedModel.num_linear_eq_constraints();

      // BMA: I don't think we need copies at all, could just access
      // model, but for now, assign instead of looping... until discuss with MK
      linEqCoeffs.reshape((int)numLinearEq,(int)numContinuousVars);
      linEqCoeffs.assign(wrappedModel.linear_eq_constraint_coeffs());
      linEqTargets.resize((int)numLinearEq);
      linEqTargets.assign(wrappedModel.linear_eq_constraint_targets());
      nonlinEqTargets.resize((int)numNonlinEq);
      nonlinEqTargets.assign(wrappedModel.nonlinear_eq_constraint_targets());

      // BMA: I don't think we need copies at all, could just access
      // model, but for now, assign instead of looping... until discuss with MK
      linIneqCoeffs.reshape((int)numLinearIneq,(int)numContinuousVars);
      linIneqCoeffs.assign(wrappedModel.linear_ineq_constraint_coeffs());

      dakotaFns.resize((int)(1+numNonlinEq+numNonlinIneq));

      dakotaGrads.reshape((int)numContinuousVars, (int)(1+numNonlinEq+numNonlinIneq));
    }

    /// Number of continuous variables
    int numContinuousVars;

    /// Shallow copy of the model on which ROL will iterate.
    Model wrappedModel;

    /// Copy of latest obective function values
    Real fnVal;

    /// Deep copy of linear inequaity coefficients
    RealMatrix linIneqCoeffs;

    /// Deep copy of linear equaity coefficients
    RealMatrix linEqCoeffs;

    /// Deep copy of linear equaity targets
    RealVector linEqTargets;

    /// Deep copy of nonlinear equaity targets
    RealVector nonlinEqTargets;

    /// Number of nonlinear inequalities
    size_t numNonlinIneq;

    /// Number of linear inequalities
    size_t numLinearIneq;

    /// Number of nonlinear equalities
    size_t numNonlinEq;

    /// Number of linear equalities
    size_t numLinearEq;

    /// Copy of latest function (objective and nonlinear constraints) values
    RealVector dakotaFns;

    /// Copy of latest function (objective and nonlinear constraints) gradient values
    RealMatrix dakotaGrads;

    /// Also used in checking whether or not update has been called yet
    int lastIterUpdateCalled;

    /// And another datum used in checking whether or not update has been called yet
    ROLTraits::VecT lastUpdateIterate;

    /// Used to monitor times update has been called
    int numMismatchedUpdatesCalled;

    /// Used to monitor times update has been called
    int numUpdateCalled;


}; // class DakotaROLModelWrapper

// --------------------------------------------------------------

class DakotaToROLIneqConstraints : public ROL::Constraint<Real>
{

  public:

    DakotaToROLIneqConstraints(DakotaROLModelWrapper & wrapped_model) :
      dakotaROLModelWrapper(wrapped_model)
    { }


    void update( const ROL::Vector<Real> &x, bool flag, int iter ) override
    {
      dakotaROLModelWrapper.update_model(x, flag, iter);
    }


    void value(ROL::Vector<Real> &c, const ROL::Vector<Real> &x, Real &tol) override
    {
      // Pointer to constraint vector
      Teuchos::RCP<std::vector<Real>> cp = ROLTraits::getVector(c);

      // Pointer to optimization vector     
      Teuchos::RCP<const std::vector<Real>> xp = ROLTraits::getVector(x);

      apply_linear_ineq_constraints( dakotaROLModelWrapper.get_wrapped_model(), *xp, *cp );
      get_nonlinear_ineq_constraints( dakotaROLModelWrapper.get_wrapped_model(), (*cp) );
    }

    // -------------------------------------------------------
    // ------------------- CAUTION ---------------------------
    // ------------ THIS CODE IS NOT TESTED ------------------
    // -------------------------------------------------------
    void applyJacobian(ROL::Vector<Real> &jv,
        const ROL::Vector<Real> &v, const ROL::Vector<Real> &x, Real &tol) override
    {
      // Pointer to optimization vector     
      Teuchos::RCP<const std::vector<Real>> xp = ROLTraits::getVector(x);

      // Pointer to jv vector
      Teuchos::RCP<std::vector<Real>> jvp = ROLTraits::getVector(jv);

      size_t num_continuous_vars = dakotaROLModelWrapper.get_num_continuous_vars();
      size_t num_linear_ineq = dakotaROLModelWrapper.get_num_linear_ineq();
      size_t num_nonlinear_ineq = dakotaROLModelWrapper.get_num_nonlinear_ineq();
      const RealMatrix & lin_ineq_coeffs = dakotaROLModelWrapper.get_lin_ineq_coeffs();
      const RealMatrix & gradient_matrix = dakotaROLModelWrapper.get_gradient_matrix();

      // apply linear constraint Jacobian
      apply_matrix(lin_ineq_coeffs, *xp, *jvp);

      // apply nonlinear constraint Jacobian
      if (num_nonlinear_ineq > 0) {

        // makes sure that update(...) is called prior to first applyJacobian(...) call
        dakotaROLModelWrapper.check_model_is_updated(x);

        for(size_t i=0;i<num_nonlinear_ineq;++i){
          (*jvp)[i+num_linear_ineq] = 0.0;
          for (size_t j=0; j<num_continuous_vars; j++)
            (*jvp)[i+num_linear_ineq] += gradient_matrix(j,i+1) * (*xp)[j];
        }
      }
    }
    // -------------------------------------------------------
    // ------------------- CAUTION ---------------------------
    // -------------------------------------------------------

  private:

    DakotaROLModelWrapper & dakotaROLModelWrapper;

}; // class DakotaToROLIneqConstraints

// --------------------------------------------------------------

class DakotaToROLEqConstraints : public ROL::Constraint<Real>
{

  public:

    DakotaToROLEqConstraints(DakotaROLModelWrapper & wrapped_model) :
      dakotaROLModelWrapper(wrapped_model)
    { }

    // BMA TODO: don't we now have data adapters that convert linear to
    // nonlinear constraints and manage the indexing?
    void update( const ROL::Vector<Real> &x, bool flag, int iter ) override
    {
      dakotaROLModelWrapper.update_model(x, flag, iter);
    }

    void value(ROL::Vector<Real> &c, const ROL::Vector<Real> &x, Real &tol) override
    {
      // Pointer to constraint vector
      Teuchos::RCP<std::vector<Real>> cp = ROLTraits::getVector(c);

      // Pointer to optimization vector     
      Teuchos::RCP<const std::vector<Real>> xp = ROLTraits::getVector(x);

      apply_linear_eq_constraints( dakotaROLModelWrapper.get_wrapped_model(), *xp, *cp );
      get_nonlinear_eq_constraints( dakotaROLModelWrapper.get_wrapped_model(), (*cp), -1.0 );
    }

    // -------------------------------------------------------
    // ------------------- CAUTION ---------------------------
    // ------------ THIS CODE IS NOT TESTED ------------------
    // -------------------------------------------------------
    void applyJacobian(ROL::Vector<Real> &jv,
        const ROL::Vector<Real> &v, const ROL::Vector<Real> &x, Real &tol)
    {
      // Pointer to optimization vector     
      Teuchos::RCP<const std::vector<Real>> xp = ROLTraits::getVector(x);

      // Pointer to jv vector
      Teuchos::RCP<std::vector<Real>> jvp = ROLTraits::getVector(jv);

      size_t num_continuous_vars = dakotaROLModelWrapper.get_num_continuous_vars();
      size_t num_linear_eq = dakotaROLModelWrapper.get_num_linear_eq();
      size_t num_nonlinear_ineq = dakotaROLModelWrapper.get_num_nonlinear_ineq();
      size_t num_nonlinear_eq = dakotaROLModelWrapper.get_num_nonlinear_eq();
      const RealMatrix & lin_eq_coeffs = dakotaROLModelWrapper.get_lin_eq_coeffs();
      const RealMatrix & gradient_matrix = dakotaROLModelWrapper.get_gradient_matrix();

      // apply linear constraint Jacobian
      apply_matrix(lin_eq_coeffs, *xp, *jvp);

      // apply nonlinear constraint Jacobian
      if (num_nonlinear_eq > 0) {

        // makes sure that update(...) is called prior to first applyJacobian(...) call
        dakotaROLModelWrapper.check_model_is_updated(x);

        for(size_t i=0;i<num_nonlinear_eq;++i){
          (*jvp)[i+num_linear_eq] = 0.0;
          for (size_t j=0; j<num_continuous_vars; j++)
            (*jvp)[i+num_linear_eq] += gradient_matrix(j,i+1+num_nonlinear_ineq) * (*xp)[j];
        }
      }
    }
    // -------------------------------------------------------
    // ------------------- CAUTION ---------------------------
    // -------------------------------------------------------

  private:

    DakotaROLModelWrapper & dakotaROLModelWrapper;

}; // class DakotaToROLEqConstraints


// --------------------------------------------------------------

class DakotaToROLObjective : public ROL::Objective<Real>
{

  public:

    DakotaToROLObjective(DakotaROLModelWrapper & wrapped_model) :
      dakotaROLModelWrapper(wrapped_model),
      gradVals(wrapped_model.get_num_continuous_vars())
  { }

    void update( const ROL::Vector<Real> &x, bool flag, int iter ) override
    {
      if( flag )
      {
        dakotaROLModelWrapper.update_model(x, flag, iter);
        Real tol = 0.0;
        fnVal = dakotaROLModelWrapper.fn_value();

        const RealMatrix & dakota_grads = dakotaROLModelWrapper.get_gradient_matrix();
        for( int i=0; i<dakotaROLModelWrapper.get_num_continuous_vars(); ++i )
          gradVals[i] = dakota_grads(i, 0);
      }
    }

    Real value(const ROL::Vector<Real> &x, Real &tol) override
    {
      return fnVal;
    }

    void gradient( ROL::Vector<Real> &g, const ROL::Vector<Real> &x, Real &tol ) override
    {
      Teuchos::RCP<std::vector<Real>> gp = ROLTraits::getVector(g);
      *gp = gradVals;
    }


  private:

    DakotaROLModelWrapper & dakotaROLModelWrapper;

    /// Copy of latest obective function values
    Real fnVal;

    /// Copy of latest function (objective) gradient values
    std::vector<Real> gradVals;

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

  /// A wrapper to the Dakota Model which allows ROL to do efficient callbacks
  std::shared_ptr<DakotaROLModelWrapper> wrappedDakotaModel;

  /// Handle to ROL::OptimizationProblem, part of ROL's simplified interface 
  ROL::OptimizationProblem<Real> optProblem;

  /// Handle to ROL's solution vector 
  Teuchos::RCP<std::vector<Real> > rolX;
};

} // namespace Dakota

#endif
