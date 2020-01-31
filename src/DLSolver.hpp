/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DLSolver.hpp
#define DLSolver.hpp

#include "DakotaOptimizer.hpp"

namespace Dakota {

/**
 * \brief A version of TraitsBase specialized for DLSolver
 *
 */

class DLSolverTraits: public TraitsBase
{
  public:

  /// default constructor
  DLSolverTraits() { }

  /// destructor
  virtual ~DLSolverTraits() { }

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

  /// Return the flag indicating whether method supports nonlinear inequalities
  bool supports_nonlinear_inequality() { return true; }

  /// Return the format used for nonlinear inequality constraints
  NONLINEAR_INEQUALITY_FORMAT nonlinear_inequality_format()
    { return NONLINEAR_INEQUALITY_FORMAT::TWO_SIDED; }

};

struct Dakota_funcs;

class Optimizer1: public Optimizer
{
public:
	char *options;
	Dakota_funcs *DF;
	Model *M, *M0;
	inline Optimizer1(Model &M1, std::shared_ptr<TraitsBase> traits):
		Optimizer(DL_SOLVER, M1, traits), DF(0), M0(&M1) {}
	inline ActiveSet *activeSet_() { return &activeSet; }
	inline Model *iteratedModel_() { return &iteratedModel; }
	inline Variables *bestVariables_() { return &bestVariablesArray.front(); }
	inline Response  *bestResponse_() { return &bestResponseArray.front(); }
	inline int maxFunctionEvals_() { return maxFunctionEvals; }
	inline int localObjectiveRecast_() { return localObjectiveRecast; }
	inline int numContinuousVars_() { return numContinuousVars; }
	inline int numFunctions_() { return numFunctions; }
	inline int numLinearConstraints_() { return numLinearConstraints; }
	inline int numNonlinearConstraints_() { return numNonlinearConstraints; }
	inline int numObjectiveFunctions_() { return numObjectiveFns; }
	inline int vendorNumericalGradFlag_() { return vendorNumericalGradFlag; }
	};

class DLSolver: public Optimizer1
{
public:
	DLSolver(Model& model);
	~DLSolver();
	void core_run();

private:
	char *details;
	void *dlLib;
	void **pdlLib;
	void *dl_Optimizer;
	void (*dl_core_run)(void*, Optimizer1*, char*);
	void (*dl_destructor)(void**);
	void botch(const char*,...);
	void cleanup();
};

} // namespace Dakota

#endif // DLSolver.hpp
