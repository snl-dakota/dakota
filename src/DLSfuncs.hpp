#ifndef DLSfuncs.hpp
#define DLSfuncs.hpp

#include <stdio.h>
#include <iostream>
#include "dakota_data_types.hpp"

namespace Dakota {

class Optimizer1;	// treated as anonymous here

struct Opt_Info {
	char *begin;	// I/O, updated by dlsolver_option for use in next dlsolver_option call
	char *name;	// out: option name
	char *val;	// out: start of value (null if no value found)
			// output _len values are int rather than size_t for format %.*s
	int name_len;	// out: length of name
	int val_len;	// out: length of value (0 if val is null)
	int all_len;	// out: length of entire name = value phrase
	int eq_repl;	// in: change '=' to this value
	int und_repl;	// in: change '_' to this value

	// constructor...

	inline Opt_Info(char *begin1, int undval = ' ', int eqval = ' '):
		begin(begin1), eq_repl(eqval), und_repl(undval) {}
	};

 struct
Dakota_probsize
{
	int n_var;	// number of continuous variables
	int n_linc;	// number of linear constraints
	int n_nlinc;	// number of nonlinear constraints
	int n_obj;	// number of objectives
	int maxfe;	// maximum function evaluations
	int numgflag;	// numerical gradient flag
	int objrecast;	// local objective recast flag
	};

 struct
Dakota_funcs
{
	int (*Fprintf)(FILE*, const char*, ...);
	void (*abort_handler1)(int);
	int  (*dlsolver_option1)(Opt_Info*);
	RealVector const * (*continuous_lower_bounds)(Optimizer1*);
	RealVector const * (*continuous_upper_bounds)(Optimizer1*);
	RealVector const * (*nonlinear_ineq_constraint_lower_bounds)(Optimizer1*);
	RealVector const * (*nonlinear_ineq_constraint_upper_bounds)(Optimizer1*);
	RealVector const * (*nonlinear_eq_constraint_targets)(Optimizer1*);
	RealVector const * (*linear_ineq_constraint_lower_bounds)(Optimizer1*);
	RealVector const * (*linear_ineq_constraint_upper_bounds)(Optimizer1*);
	RealVector const * (*linear_eq_constraint_targets)(Optimizer1*);
	RealMatrix const * (*linear_ineq_constraint_coeffs)(Optimizer1*);
	RealMatrix const * (*linear_eq_constraint_coeffs)(Optimizer1*);
	void (*ComputeResponses1)(Optimizer1*, int mode, int n, double *x);
	// ComputeResponses uses x[i], 0 <= i < n
	void (*GetFuncs1)(Optimizer1*, int m0, int m1, double *f);
	// GetFuncs sets f[i] <-- response(m+i), m0 <= i < m1
	void (*GetGrads1)(Optimizer1*, int m0, int m1, int n, int is, int js, double *g);
	// g[(i-m0)*is + j*js] <-- partial(Response i)/partial(var j),
	// m0 <= i < m1, 0 <= j < n
	void (*GetContVars)(Optimizer1*, int n, double *x);
	void (*SetBestContVars)(Optimizer1*, int n, double *x);	// set best continuous var values
	void (*SetBestDiscVars)(Optimizer1*, int n, int *x);	// set best discrete var values
	void (*SetBestRespFns)(Optimizer1*, int n, double *x);	// set best resp func values
	double (*get_real)(Optimizer1*, const char*);	// for probDescDB.get_real()
	int    (*get_int) (Optimizer1*, const char*);	// for probDescDB.get_int()
	bool   (*get_bool)(Optimizer1*, const char*);	// for probDescDB.get_bool()
	Dakota_probsize *ps;
	std::ostream *dakota_cerr, *dakota_cout;
	FILE *Stderr;
	};

#ifdef NO_DAKOTA_DLSOLVER_FUNCS_INLINE

extern int dlsolver_option(Opt_Info *);

#else //!NO_DAKOTA_DLSOLVER_FUNCS_INLINE

extern Dakota_funcs *DF;

#undef Cout
#define Cout (*DF->dakota_cout)

#if 0 // "inline" is not inlined at low optimization levels,
// causing confusion in gdb.  It's less confusing to use #define;
// we get type checking in this case much as with inline.

inline void abort_handler(int n)
{ DF->abort_handler1(n); }

inline int dlsolver_option(Opt_Info *oi)
{ return DF->dlsolver_option1(oi); }

inline RealVector const * continuous_lower_bounds(Optimizer1 *o)
{ return DF->continuous_lower_bounds(o); }

inline RealVector const * continuous_upper_bounds(Optimizer1 *o)
{ return DF->continuous_upper_bounds(o); }

inline RealVector const * nonlinear_ineq_constraint_lower_bounds(Optimizer1 *o)
{ return DF->nonlinear_ineq_constraint_lower_bounds(o); }

inline RealVector const * nonlinear_ineq_constraint_upper_bounds(Optimizer1 *o)
{ return DF->nonlinear_ineq_constraint_upper_bounds(o); }

inline RealVector const * nonlinear_eq_constraint_targets(Optimizer1 *o)
{ return DF->nonlinear_eq_constraint_targets(o); }

inline RealVector const * linear_ineq_constraint_lower_bounds(Optimizer1 *o)
{ return DF->linear_ineq_constraint_lower_bounds(o); }

inline RealVector const * linear_ineq_constraint_upper_bounds(Optimizer1 *o)
{ return DF->linear_ineq_constraint_upper_bounds(o); }

inline RealVector const * linear_eq_constraint_targets(Optimizer1 *o)
{ return DF->linear_eq_constraint_targets(o); }

inline RealMatrix const * linear_ineq_constraint_coeffs(Optimizer1 *o)
{ return DF->linear_ineq_constraint_coeffs(o); }

inline RealMatrix const * linear_eq_constraint_coeffs(Optimizer1 *o)
{ return DF->linear_eq_constraint_coeffs(o); }

inline void ComputeResponses(Optimizer1 *o, int mode, int n, double *x)
{ DF->ComputeResponses1(o, mode, n, x); }

inline void GetFuncs(Optimizer1 *o, int m0, int m1, double *f)
{ DF->GetFuncs1(o, m0, m1, f); }

inline void GetGrads(Optimizer1 *o, int m0, int m1, int n, int is, int js, double *g)
{ DF->GetGrads1(o, m0, m1, n, is, js, g); }

inline void GetContVars(Optimizer1 *o, int n, double *x)
{ DF->GetContVars(o, n, x); }

inline void SetBestContVars(Optimizer1 *o, int n, double *x)
{ DF->SetBestContVars(o, n, x); }

inline void SetBestRespFns(Optimizer1 *o, int n, double *x)
{ DF->SetBestRespFns(o, n, x); }

#else // use #define to really inline

#define abort_handler(n) DF->abort_handler1(n)
#define dlsolver_option(oi) DF->dlsolver_option1(oi)
#define continuous_lower_bounds(o) DF->continuous_lower_bounds(o)
#define continuous_upper_bounds(o) DF->continuous_upper_bounds(o)
#define nonlinear_ineq_constraint_lower_bounds(o) DF->nonlinear_ineq_constraint_lower_bounds(o)
#define nonlinear_ineq_constraint_upper_bounds(o) DF->nonlinear_ineq_constraint_upper_bounds(o)
#define nonlinear_eq_constraint_targets(o) DF->nonlinear_eq_constraint_targets(o)
#define linear_ineq_constraint_lower_bounds(o) DF->linear_ineq_constraint_lower_bounds(o)
#define linear_ineq_constraint_upper_bounds(o) DF->linear_ineq_constraint_upper_bounds(o)
#define linear_eq_constraint_targets(o) DF->linear_eq_constraint_targets(o)
#define linear_ineq_constraint_coeffs(o) DF->linear_ineq_constraint_coeffs(o)
#define linear_eq_constraint_coeffs(o) DF->linear_eq_constraint_coeffs(o)
#define ComputeResponses(o,mode,n,x) DF->ComputeResponses1(o, mode, n, x)
#define GetFuncs(o,m0,m1,f) DF->GetFuncs1(o, m0, m1, f)
#define GetGrads(o,m0,m1,n,is,js,g) DF->GetGrads1(o, m0, m1, n, is, js, g)
#define GetContVars(o,n,x) DF->GetContVars(o, n, x)
#define SetBestContVars(o,n,x) DF->SetBestContVars(o, n, x)
#define SetBestDiscVars(o,n,x) DF->SetBestDiscVars(o, n, x)
#define SetBestRespFns(o,n,x) DF->SetBestRespFns(o, n, x)

#endif

#endif //NO_DAKOTA_DLSOLVER_FUNCS_INLINE

typedef void (*dl_core_run_t)(void*, Optimizer1*, char*);
typedef void (*dl_destructor_t)(void**);
extern "C" void *dl_constructor(Optimizer1 *, Dakota_funcs*, dl_core_run_t *, dl_destructor_t *);

} // namespace Dakota

#endif //DLSfuncs.hpp
