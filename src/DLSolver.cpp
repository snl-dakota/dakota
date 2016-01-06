#ifdef _WIN32
#include "dakota_windows.h"
#define dlopen(x,y) LoadLibrary(x)
#define find_dlsym(a,b,c) (a = (dl_constructor_t)GetProcAddress((HINSTANCE)(b),c))
#define dlclose(x) FreeLibrary((HMODULE)x)
#define NO_DLERROR
#else
#include <dlfcn.h>
#define find_dlsym(a,b,c) (a = (dl_constructor_t)dlsym(b,c))
#undef NO_DLERROR
#endif
#include <stdarg.h>
#include "DLSolver.hpp"
#include "ProblemDescDB.hpp"

#ifdef DLSOLVER_DEBUG_UNLOCK
// This is only needed for the simplest example, dlsolvers/dl_npsol.C,
// which uses as much of DAKOTA's machinery for NPSOL as possible.
// (This machinery is compiled into DAKOTA unless configured with --without-npsol).
// The corresponding dl_npsol.dll is not loaded until the database is locked,
// but dl_npsol.dll then invokes "new NPSOLOptimizer(...)", which needs to
// have the database unlocked.  Note that unlocking then may give wrong results
// in multi-specification runs.  The configure machinery does not know about
// compilation with -DDLSOLVER_DEBUG_UNLOCK, since it is only meant for use
// in special cases by knowledgeable developers.
#define DEBUG_unlock probDescDB.unlock();
#define DEBUG_lock probDescDB.lock();
#else
#define DEBUG_unlock /*default: do nothing*/
#define DEBUG_lock   /*default: do nothing*/
#endif

#define NO_DAKOTA_DLSOLVER_FUNCS_INLINE
#include "DLSfuncs.hpp"

namespace Dakota {

 static RealVector const *
continuous_lower_bounds1(Optimizer1 *o)
{ return &o->M->continuous_lower_bounds(); }

 static RealVector const *
continuous_upper_bounds1(Optimizer1 *o)
{ return &o->M->continuous_upper_bounds(); }

 static RealVector const *
nonlinear_ineq_constraint_lower_bounds1(Optimizer1 *o)
{ return &o->M->nonlinear_ineq_constraint_lower_bounds(); }

 static RealVector const *
nonlinear_ineq_constraint_upper_bounds1(Optimizer1 *o)
{ return &o->M->nonlinear_ineq_constraint_upper_bounds(); }

 static RealVector const *
nonlinear_eq_constraint_targets1(Optimizer1 *o)
{ return &o->M->nonlinear_eq_constraint_targets(); }

 static RealVector const *
linear_ineq_constraint_lower_bounds1(Optimizer1 *o)
{ return &o->M->linear_ineq_constraint_lower_bounds(); }

 static RealVector const *
linear_ineq_constraint_upper_bounds1(Optimizer1 *o)
{ return &o->M->linear_ineq_constraint_upper_bounds(); }

 static RealVector const *
linear_eq_constraint_targets1(Optimizer1 *o)
{ return &o->M->linear_eq_constraint_targets(); }

 static RealMatrix const *
linear_eq_constraint_coeffs1(Optimizer1 *o)
{ return &o->M->linear_eq_constraint_coeffs(); }

 static RealMatrix const *
linear_ineq_constraint_coeffs1(Optimizer1 *o)
{ return &o->M->linear_ineq_constraint_coeffs(); }

 static void
ComputeResponses1(Optimizer1 *o, int mode, int n, double *x)
{
	// without the printf, g++ generates incorrect code
	if (n < 0)
		printf("ComputeResponses1 has o = #%x, mode = %d, n = %d, x = #%x\n",
			o, mode, n, x);
	RealVector lx(n);
	copy_data(x, n, lx);
	o->M->continuous_variables(lx);
	o->activeSet_()->request_values(mode);
	o->M->evaluate(*o->activeSet_());
	}

 static void
GetFuncs1(Optimizer1 *o, int m0, int m1, double *f)
{
	Response const *R = &o->M->current_response();
	RealVector const *RV = &R->function_values();
	for(int i = m0; i < m1; i++)
		f[i-m0] = (*RV)[i];
	}

 static void
GetGrads1(Optimizer1 *o, int m0, int m1, int n, int is, int js, double *g)
{
	int i, i1, j;
	Response const *R = &o->M->current_response();
	RealMatrix const *RM = &R->function_gradients();
	Real const *RV;

	for(i1 = 0, i = m0; i < m1; i++, i1 += is) {
		RV = (*RM)[i];
		for(j = 0; j < n; j++)
			g[i1 + j*js] = RV[j];
		}
	}

 static void
GetContVars1(Optimizer1 *o, int n, double *x)
{ memcpy(x, &o->M->continuous_variables()[0], n*sizeof(double)); }

 static void
SetBestContVars1(Optimizer1 *o, int n, double *x)
{
	int i;
	RealVector X(n);
	for(i = 0; i < n; i++)
		X[i] = x[i];
	o->bestVariables_()->continuous_variables(X);
	}

 static void
SetBestDiscVars1(Optimizer1 *o, int n, int *x)
{
	int i;
	IntVector X(n);
	for(i = 0; i < n; i++)
		X[i] = x[i];
	o->bestVariables_()->discrete_int_variables(X);
	}

 static void
SetBestRespFns1(Optimizer1 *o, int n, double *x)
{
	int i;
	RealVector X(n);
	for(i = 0; i < n; i++)
		X[i] = x[i];
	o->bestResponse_()->function_values(X);
	}

 static double
Get_Real1(Optimizer1*o, const char* name)
{
	return o->problem_description_db().get_real(name);
	}

 static int
Get_Int1(Optimizer1*o, const char* name)
{
	return o->problem_description_db().get_int(name);
	}

 static bool
Get_Bool1(Optimizer1*o, const char* name)
{
	return o->problem_description_db().get_bool(name);
	}

Dakota_funcs DakFuncs0 = {
	fprintf,
	abort_handler,
	dlsolver_option,
	continuous_lower_bounds1,
	continuous_upper_bounds1,
	nonlinear_ineq_constraint_lower_bounds1,
	nonlinear_ineq_constraint_upper_bounds1,
	nonlinear_eq_constraint_targets1,
	linear_ineq_constraint_lower_bounds1,
	linear_ineq_constraint_upper_bounds1,
	linear_eq_constraint_targets1,
	linear_ineq_constraint_coeffs1,
	linear_eq_constraint_coeffs1,
	ComputeResponses1,
	GetFuncs1,
	GetGrads1,
	GetContVars1,
	SetBestContVars1,
	SetBestDiscVars1,
	SetBestRespFns1,
	Get_Real1,
	Get_Int1,
	Get_Bool1
	};

 void
DLSolver::cleanup()
{
	void *h;
	if (h = dlLib) {
		dlLib = NULL;
		if (dl_destructor)
			(*dl_destructor)(&dl_Optimizer);
		dlclose(h);
		if (pdlLib)
			*pdlLib = 0;
		}
	if (details) {
		delete[] details;
		details = 0;
		}
	if (DF) {
		delete DF;
		DF = 0;
		}
	}

 void
DLSolver::botch(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	fprintf(stderr, "\nDLSolver Error: ");
	vfprintf(stderr, fmt, ap);
	putc('\n', stderr);
	va_end(ap);
	fflush(stderr);
	cleanup();
	abort_handler(-1);
	}

DLSolver::DLSolver(Model& model):
	Optimizer1(model), dl_core_run(0), dl_destructor(0), dlLib(0)
{
	const String &dlDetails = probDescDB.get_string("method.dl_solver.dlDetails");
	char *s, *s0;
	size_t L;
	void *h, **vp;

	DF = 0;
	L = strlen(s0 = dlDetails.data()) + 1;
	details = s = new char[L];
	if (!s)
		botch("new(%lu) failure in DLSolver::DLSolver.", (unsigned long) L);
	memcpy(s, s0, L);
	vp = probDescDB.get_voidss("method.dl_solver.dlLib");
	pdlLib = *vp ? vp : 0;
	}

DLSolver::~DLSolver()
{ cleanup(); }

 void
DLSolver::core_run()
{
	Dakota_funcs *df;
	Dakota_probsize ps;
	char *s, *s0;
	typedef void* (*dl_constructor_t)(Optimizer1*, Dakota_funcs*,
			dl_core_run_t*, dl_destructor_t*);
	dl_constructor_t dl_constructor;
	void *h;
	if (!dl_core_run) {	// Load the shared library if this is
				// the first core_run() invocation.
		df = DF = new Dakota_funcs;
		if (!df)
			botch("new Dakota_Funcs failure");
		memcpy(df, &DakFuncs0, sizeof(Dakota_funcs));
		for(s0 = s = details; *s > ' '; s++);
		if (s == s0)
			botch("dlDetails not given for dl_solver.");
		if (*s)
			*s++ = 0;
		if (pdlLib)
			h = *pdlLib;
		else
			h = dlopen(s0, RTLD_NOW/*RTLD_LAZY*/);
		if (!h)
#ifdef NO_DLERROR
			botch("dlopen(\"%s\") failure", s0);
#else
			botch("dlopen(\"%s\") failure:\n%s", s0, dlerror());
#endif
		dlLib = h;
		if (!(find_dlsym(dl_constructor, h, "dl_constructor")))
			botch("dlsym(\"dl_constructor\") failed in \"%s\"", s0);
		while(*s && *s <= ' ')
			++s;
		options = s;
		df->Stderr = stderr;
		df->dakota_cerr = dakota_cerr;
		df->dakota_cout = dakota_cout;
		DEBUG_unlock
		dl_Optimizer = (*dl_constructor)(this, df, &dl_core_run, &dl_destructor);
		DEBUG_lock
		}
	if (dl_core_run) {
		df = DF;
		df->Stderr = stderr;
		df->dakota_cerr = dakota_cerr;
		df->dakota_cout = dakota_cout;
		ps.n_var = numContinuousVars_();
		ps.n_linc = numLinearConstraints_();
		ps.n_nlinc = numNonlinearConstraints_();
		ps.n_obj = numObjectiveFunctions_();
		ps.maxfe = maxFunctionEvals_();
		ps.numgflag = vendorNumericalGradFlag_();
		ps.objrecast = localObjectiveRecast_();
		df->ps = &ps;
		M = iteratedModel_();
		(*dl_core_run)(dl_Optimizer, this, options);
		}
	else
		botch("dl_core_run is null in core_run");
	}

 int
dlsolver_option(Opt_Info *Oi)
{
	char *in, *in0, *s, *val0;
	int c;

	if (!(in = Oi->begin))
		return 0;
	while(*in <= ' ')
		if (!*in++)
			return 0;
	Oi->name = in0 = in;
	for(; (c = *in) > ' '; ++in) {
		if (c == '_')
			*in = Oi->und_repl;
		else if (c == '=') {
			*in = Oi->eq_repl;
			break;
			}
		}
	Oi->name_len = in - in0;
	if (c == '=')
		++in;
	if (!*in) {
 no_val:
		Oi->val = 0;
		Oi->val_len = 0;
		Oi->begin = in;
		Oi->all_len = Oi->name_len;
		return 1;
		}
	for(; (c = *in) <= ' '; ++in) {
		if (!c)
			goto no_val;
		}
	if (c == '=') {
		*in++ = Oi->eq_repl;
		while((c = *++in) <= ' ') {
			if (!c)
				goto no_val;
			}
		}
	Oi->val = val0 = in;
	if (c == '\'') {
		for(s = in;;) {
			c = *++in;
			if (c == '\'') {
				if (*++in != '\'')
					break;
				}
			else if (!c)
				break;
			*s++ = c;
			}
		*s = 0;
		Oi->val_len = s - val0;
		}
	else {
		while(*++in > ' ');
		Oi->val_len = in - val0;
		}
	while((c = *in) && c <= ' ')
		++in;
	Oi->begin = in;
	Oi->all_len = in - in0;
	return 1;
	}

} // namespace Dakota
