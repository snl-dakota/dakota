#include "nidr.h"
#include "NIDRProblemDescDB.H"
#include "NPSOLOptimizer.H"
#include "NLSSOLLeastSq.H"

namespace Dakota {

 struct
Meth_Info {
	DataMethodRep *dme;
	DataMethod *dme0;
	};

 void NIDRProblemDescDB::
method_Real(const char *keyname, Values *val, void **g, void *v)
{
	(*(Meth_Info**)g)->dme->**(Real DataMethodRep::**)v = *val->r;
	}

 struct
Method_mp_lit {
	String DataMethodRep::* sp;
	const char *lit;
	};

 void NIDRProblemDescDB::
method_lit(const char *keyname, Values *val, void **g, void *v)
{
	(*(Meth_Info**)g)->dme->*((Method_mp_lit*)v)->sp = ((Method_mp_lit*)v)->lit;
	}

#define MP_(x) DataMethodRep::* method_mp_##x = &DataMethodRep::x
#define MP2(x,y) method_mp_##x##_##y = {&DataMethodRep::x,#y}

static Method_mp_lit
	MP2(methodName,nlssol_sqp),
	MP2(methodName,npsol_sqp);

static Real
	MP_(functionPrecision),
	MP_(lineSearchTolerance);

static int
	MP_(verifyLevel);

extern  NLSSOLLeastSq *(*new_NLSSOLLeastSq)(Model& model);
extern  NLSSOLLeastSq *(*new_NLSSOLLeastSq1)(NoDBBaseConstructor, Model &model);
extern  NPSOLOptimizer *(*new_NPSOLOptimizer)(Model& model);
extern  NPSOLOptimizer *(*new_NPSOLOptimizer1)(NoDBBaseConstructor, Model &model);

 static NPSOLOptimizer *
my_new_NPSOLOptimizer(Model& model)
{
	return new NPSOLOptimizer(model);
	}

 static NPSOLOptimizer *
my_new_NPSOLOptimizer1(NoDBBaseConstructor, Model& model)
{
	return new NPSOLOptimizer(NoDBBaseConstructor(), model);
	}

 static NPSOLOptimizer *
my_new_NPSOLOptimizer2(Model& model, const int& derivative_level, const Real& conv_tol)
{
	return new NPSOLOptimizer(model, derivative_level, conv_tol);
	}

 static NPSOLOptimizer *
my_new_NPSOLOptimizer3(const RealVector& initial_point,
    const RealVector& var_lower_bnds,
    const RealVector& var_upper_bnds,
    const RealMatrix& lin_ineq_coeffs,
    const RealVector& lin_ineq_lower_bnds,
    const RealVector& lin_ineq_upper_bnds,
    const RealMatrix& lin_eq_coeffs,
    const RealVector& lin_eq_targets,
    const RealVector& nonlin_ineq_lower_bnds,
    const RealVector& nonlin_ineq_upper_bnds,
    const RealVector& nonlin_eq_targets,
    void (*user_obj_eval) (int&, int&, double*, double&, double*, int&),
    void (*user_con_eval) (int&, int&, int&, int&, int*, double*, double*,
			   double*, int&),
    const int& derivative_level, const Real& conv_tol)
{
	return new NPSOLOptimizer(initial_point,
			var_lower_bnds,
			var_upper_bnds,
			lin_ineq_coeffs,
			lin_ineq_lower_bnds,
			lin_ineq_upper_bnds,
			lin_eq_coeffs,
			lin_eq_targets,
			nonlin_ineq_lower_bnds,
			nonlin_ineq_upper_bnds,
			nonlin_eq_targets,
			user_obj_eval,
			user_con_eval,
			derivative_level,
			conv_tol);
	}

 static NLSSOLLeastSq *
my_new_NLSSOLLeastSq(Model& model)
{
	return new NLSSOLLeastSq(model);
	}

 static NLSSOLLeastSq *
my_new_NLSSOLLeastSq1(NoDBBaseConstructor, Model& model)
{
	return new NLSSOLLeastSq(NoDBBaseConstructor(), model);
	}

 static void
stanford_start(const char *keyname, Values *val, void **g, void *v)
{
	new_NLSSOLLeastSq  = my_new_NLSSOLLeastSq;
	new_NLSSOLLeastSq1 = my_new_NLSSOLLeastSq1;
	new_NPSOLOptimizer  = my_new_NPSOLOptimizer;
	new_NPSOLOptimizer1 = my_new_NPSOLOptimizer1;
	}

#define N_mdm(x,y)	NIDRProblemDescDB::method_##x,&method_mp_##y

/** 6 distinct keywords (plus 2 aliases) **/

static KeyWordx
	kw_1[7] = {
		{{"function_precision",0x2000a,0,3,0,0,0.,0.,0,N_mdm(Real,functionPrecision)},7,9,"N_mdm(Real,functionPrecision)","{Function precision} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNPSOLDC"},
		{{"linesearch_tolerance",0x2000a,0,4,0,0,0.,0.,0,N_mdm(Real,lineSearchTolerance)},8,11,"N_mdm(Real,lineSearchTolerance)","{Line search tolerance} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNPSOLDC"},
		{{"nlssol",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,methodName_nlssol_sqp)},4,5,"N_mdm(lit,methodName_nlssol_sqp)",0,"Nonlinear Least Squares"},
		{{"nlssol_sqp",0x20000,0,1,1,0,0.,0.,-1,N_mdm(lit,methodName_nlssol_sqp)},5,4,"N_mdm(lit,methodName_nlssol_sqp)"},
		{{"npsol",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,methodName_npsol_sqp)},2,3,"N_mdm(lit,methodName_npsol_sqp)",0,"Optimization: Local, Derivative-based"},
		{{"npsol_sqp",0x20000,0,1,1,0,0.,0.,-1,N_mdm(lit,methodName_npsol_sqp)},3,2,"N_mdm(lit,methodName_npsol_sqp)"},
		{{"verify_level",0x20009,0,2,0,0,0.,0.,0,N_mdm(int,verifyLevel)},6,7,"N_mdm(int,verifyLevel)","{Gradient verification level} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodNPSOLDC"}
		},
	kw_2[1] = {
		{{"stanford",0x20008,7,1,1,(KeyWord*)kw_1,0.,0.,0,stanford_start},1,1,"stanford_start"}
		};

} // namespace Dakota

#ifdef __cplusplus
extern "C" {
KeyWord *keyword_add(void);
}
#endif

 KeyWord*
keyword_add(void) {
	return &Dakota::kw_2[0].kw;
	}
