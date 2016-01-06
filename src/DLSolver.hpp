#ifndef DLSolver.hpp
#define DLSolver.hpp

#include "DakotaOptimizer.hpp"

namespace Dakota {

struct Dakota_funcs;

class Optimizer1: public Optimizer
{
public:
	char *options;
	Dakota_funcs *DF;
	Model *M, *M0;
	inline Optimizer1(Model &M1): Optimizer(M1), DF(0), M0(&M1) {}
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
