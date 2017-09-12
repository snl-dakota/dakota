#include <stdio.h>
#include "DLSolver.H"
#define NO_DAKOTA_DLSOLVER_FUNCS_INLINE
#include "DLSfuncs.H"
#include "SOLBase.H"
using std::endl;

extern "C" void flush_();
extern "C" void npoptn_(const char*, unsigned int);

#define NPSOL_F77   F77_FUNC(npsol,NPSOL)
#define NPOPTN2_F77 F77_FUNC(npoptn2,NPOPTN2)

extern "C" void NPSOL_F77( int& n, int& nclin, int& ncnln, int& nrowa, int& nrowj,
		int& nrowr, double* a, double* bl, double* bu,
		void (*funcon)(int& mode, int& ncnln, int& n, int& nrowj,
			       int* needc, double* x, double* c, double* cjac,
			       int& nstate),
		void (*funobj)(int& mode, int& n, double* x, double& f,
			       double* gradf, int& nstate),
		int& inform, int& iter, int* istate, double* c, double* cjac,
		double* clambda, double& objf, double* grad, double* r,
		double* x, int* iw, int& leniw, double* w, int& lenw );

namespace Dakota {

 class
NPSOL_stuff: public SOLBase {
 private:
	static NPSOL_stuff* npsolInstance;
	static void objective_eval(int& mode, int& n, double* x, double& f,
					double* gradf, int& nstate);
 public:
	Optimizer1 *Oz;
	char *options;
	~NPSOL_stuff() {}
	void find_optimum(Optimizer1*);
	NPSOL_stuff(Optimizer1 *O1): Oz(O1), options(0) {}
 };

NPSOL_stuff* NPSOL_stuff::npsolInstance(NULL);

 static void
np_optn(const char *s)
{
	npoptn_(s, strlen(s));
	}

void NPSOL_stuff::
objective_eval(int& mode, int& n1, double* x, double& f, double* gradf,
	       int& nstate)
{
	int	n	= n1;
	Optimizer1 *D	= npsolInstance->Oz;
	Model	*M	= D->M0;
	int	nnlc	= D->numNonlinearConstraints_();

	// NPSOL computes constraints first, then the objective function.  However,
	// Dakota assumes that the objective and constraint function values are all
	// computed in a single fn. evaluation. A numNonlinearConstraints check is
	// therefore needed to ensure that 1 and only 1 mapping occurs.

	// Handle special cases with asv_request (see SOLBase::constraint_eval)
	int asv_request = mode + 1; // default definition of asv_request

	if (D->vendorNumericalGradFlag_() && (asv_request & 2) ) {
	  asv_request -= 2; // downgrade request
	  if (nnlc == 0) { // else already printed
	    Cout << "NPSOL has requested objective gradient for case of vendor "
	   << "numerical gradients.\n";
	    if (asv_request)
	Cout << "Request will be downgraded to objective value alone.\n" <<endl;
	    else
	Cout << "Request will be ignored and no evaluation performed.\n" <<endl;
	  }
	}

	if (asv_request && nnlc == 0) {
	  // constraint_eval has not been called.  Therefore, set vars/asv
	  // and perform a compute_response prior to data recovery.
	  RealVector local_des_vars(n);
	  copy_data(x, n, local_des_vars);
	  M->continuous_variables(local_des_vars);
	  D->activeSet_()->request_values(asv_request);
	  M->compute_response(*D->activeSet_());
	  if (++npsolInstance->fnEvalCntr == D->maxFunctionEvals_()) {
	    mode = -1; // terminate NPSOL (see mode discussion in "User-Supplied
	         // Subroutines" section of NPSOL manual)
	    Cout << "Iteration terminated: max_function_evaluations limit has been "
	   << "met." << endl;
	  }
	}

	const Response& local_response = M->current_response();
	if (asv_request & 1)
	  f = local_response.function_values()[0];
	if (asv_request & 2)
		memcpy(gradf, local_response.function_gradients()[0], n*sizeof(Real));
}


 void
NPSOL_stuff::find_optimum(Optimizer1 *D)
{
	NPSOL_stuff* prev_instance	= npsolInstance;
	Minimizer *prev_minimizer	= optLSqInstance;

	char	*nextOpt, *Opt, *Opts, *val;
	int	i;
	int	num_cv  = D->numContinuousVars_();
	int	num_lc	= D->numLinearConstraints_();
	int	num_nlc = D->numNonlinearConstraints_();
	Model	*M	= D->M0;
	double	local_f_val = 0.;
	Opt_Info OI(options);
	RealVector local_f_grad(num_cv, 0.);
	size_t	namelen, vallen;

	npsolInstance	= this;
	solInstance	= this;
	optLSqInstance	= D;

	// Augmentation of bounds appears here rather than in the constructor because
	// set the constraint offset used in SOLBase::constraint_eval()
	constrOffset = D->numObjectiveFunctions_();

	fnEvalCntr = 0; // prevent current iterator from continuing previous counting

	allocate_arrays(num_cv, num_nlc,
		  M->linear_ineq_constraint_coeffs(),
		  M->linear_eq_constraint_coeffs());
	allocate_workspace(num_cv, num_nlc, num_lc, 0);

	// NPSOL requires a non-zero array size.  Therefore, size the local
	// constraint arrays and matrices to a size of 1 if there are no nonlinear
	// constraints and to the proper size otherwise.
	RealVector local_c_vals(nlnConstraintArraySize, 0.);

	// initialize local_des_vars with DB initial point.  Variables are updated
	// in constraint_eval/objective_eval
	RealVector local_des_vars = M->continuous_variables();

	// these bounds must be updated from model bounds each time an iterator is
	// run within the B&B strategy.
	RealVector augmented_l_bnds = M->continuous_lower_bounds();
	RealVector augmented_u_bnds = M->continuous_upper_bounds();
	augment_bounds(augmented_l_bnds, augmented_u_bnds,
		 M->linear_ineq_constraint_lower_bounds(),
		 M->linear_ineq_constraint_upper_bounds(),
		 M->linear_eq_constraint_targets(),
		 M->nonlinear_ineq_constraint_lower_bounds(),
		 M->nonlinear_ineq_constraint_upper_bounds(),
		 M->nonlinear_eq_constraint_targets());

	np_optn("NOLIST");
	np_optn("MA PR 0");
	while(dlsolver_option(&OI)) {
		printf("\n*** calling npoptn_(\"%.*s\", %d) ***\n", OI.all_len,
			OI.name, OI.all_len);
		npoptn_(OI.name, OI.all_len);
		}

	NPSOL_F77( num_cv, num_lc, num_nlc,
	     linConstraintArraySize, nlnConstraintArraySize, num_cv,
	     linConstraintMatrixF77, &augmented_l_bnds[0], &augmented_u_bnds[0],
	     constraint_eval, objective_eval, informResult,
	     numberIterations, constraintState.data(), &local_c_vals[0],
	     constraintJacMatrixF77, cLambda.data(), local_f_val,
	     &local_f_grad[0], upperFactorHessianF77, &local_des_vars[0], intWorkSpace.data(),
	     intWorkSpaceSize, realWorkSpace.data(), realWorkSpaceSize );
	flush_();	// flush Fortran buffers

	// NPSOL completed. Do post-processing/output of final NPSOL info and data:
	Cout << "\nNPSOL exits with INFORM code = " << informResult
	     << " (see \"Interpretation of output\" section in NPSOL manual)\n";

	// invoke SOLBase deallocate function (shared with NLSSOLLeastSq)
	deallocate_arrays();

	// Set bestVariables and bestResponse for use by strategy level.
	// local_des_vars, local_f_val, & local_c_vals contain the optimal design
	// (not the final fn. eval) since NPSOL performs this assignment internally
	// prior to exiting (see "Subroutine npsol" section of NPSOL manual).
	D->DF->SetBestContVars(D, num_cv, &local_des_vars[0]);
	if (!D->multiObjFlag_()) {
		Real best_fns[num_nlc + 1];
		best_fns[0] = local_f_val;
		for(i = 0; i < num_nlc; ++i)
			best_fns[i+1] = local_c_vals[i];
		D->DF->SetBestRespFns(D, num_nlc+1, best_fns);
		}

	std::ifstream npsol_fort_9( "fort.9" );
	if (npsol_fort_9) {
		char fort_9_line[255];
		Cout << "\nNPSOL's iteration output from fort.9 file:\n" << endl;
		while (npsol_fort_9) {
			npsol_fort_9.getline( fort_9_line, 255 );
			Cout << fort_9_line << endl;
			}
		Cout << endl;
		}

	// restore in case of recursion
	npsolInstance  = prev_instance;
	solInstance    = prev_instance;
	optLSqInstance = prev_minimizer;
	}

 static void
my_findopt(void *v, Optimizer1 *D, char *options)
{
	NPSOL_stuff *T = (NPSOL_stuff *)v;
	T->options = options;
	T->find_optimum(D);
	}

 static void
my_destructor(void **v)
{
	NPSOL_stuff *T = *(NPSOL_stuff **)v;
	printf("\ndl_npsol calling ~NPSOL_stuff\n\n");
	*v = 0;
	delete T;
	}

 void*
dl_constructor(Optimizer1 *D, Dakota_funcs *df, dl_find_optimum_t *findopt, dl_destructor_t *dtor)
{
	NPSOL_stuff *T;

	*findopt = my_findopt;
	*dtor = my_destructor;
	T = new NPSOL_stuff(D);
	return (void*)T;
	}

} // namespace Dakota
