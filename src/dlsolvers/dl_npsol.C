#include <stdio.h>
#include "DLSolver.H"
#define NO_DAKOTA_DLSOLVER_FUNCS_INLINE
#include "DLSfuncs.H"
#include "NPSOLOptimizer.H"

 extern "C" void npoptn_(const char*, unsigned int);

namespace Dakota {

 static void
np_optn(const char *s)
{
	npoptn_(s, strlen(s));
	}

 static void
my_findopt(void *v, Optimizer1 *D, char *options)
{
	NPSOLOptimizer *T = (NPSOLOptimizer *)v;
	size_t i, nf;

	printf("\ndl_npsol calling find_optimum; options = \"%s\"\n\n", options);
	np_optn("NOLIST");
	np_optn("MA PR 0");
	np_optn("Verify Level = -1");
	T->find_optimum();
	(*D->bestVariables_()).continuous_variables(
		T->variables_results().continuous_variables());
	(*D->bestResponse_()).function_values(
		T->response_results().function_values());
	}

 static void
my_destructor(void **v)
{
	NPSOLOptimizer *T = *(NPSOLOptimizer **)v;
	printf("\ndl_npsol calling ~NPSOLOptimizer\n\n");
	*v = 0;
	delete T;
	}

 void*
dl_constructor(Optimizer1 *M, Dakota_funcs *df, dl_find_optimum_t *findopt, dl_destructor_t *dtor)
{
	NPSOLOptimizer *T = new NPSOLOptimizer(*M->M0);
	*findopt = my_findopt;
	*dtor = my_destructor;
	return (void*)T;
	}

} // namespace Dakota
