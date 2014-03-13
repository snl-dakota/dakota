#include "nidr.h"
#include "NIDRProblemDescDB.H"
#include "DOTOptimizer.H"

namespace Dakota {

 struct
Meth_Info {
	DataMethodRep *dme;
	DataMethod *dme0;
	};

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

#define MP2(x,y) method_mp_##x##_##y = {&DataMethodRep::x,#y}
static Method_mp_lit
	MP2(minMaxType,maximize),
	MP2(minMaxType,minimize),
	MP2(methodName,dot_bfgs),
	MP2(methodName,dot_frcg),
	MP2(methodName,dot_mmfd),
	MP2(methodName,dot_slp),
	MP2(methodName,dot_sqp);

extern DOTOptimizer *(*new_DOTOptimizer)(Model& model);
extern  DOTOptimizer *(*new_DOTOptimizer1)(NoDBBaseConstructor, Model &model);

 static DOTOptimizer *
my_new_DOTOptimizer(Model& model)
{
	return new DOTOptimizer(model);
	}

 static DOTOptimizer *
my_new_DOTOptimizer1(NoDBBaseConstructor, Model& model)
{
	return new DOTOptimizer(NoDBBaseConstructor(), model);
	}

 static void
dot_start(const char *keyname, Values *val, void **g, void *v)
{
	new_DOTOptimizer  = my_new_DOTOptimizer;
	new_DOTOptimizer1 = my_new_DOTOptimizer1;
	}

#define N_mdm(x,y)	NIDRProblemDescDB::method_##x,&method_mp_##y

/** 9 distinct keywords (plus 5 aliases) **/

static KeyWordx
	kw_1[2] = {
		{{"maximize",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,minMaxType_maximize)},14,17,"N_mdm(lit,minMaxType_maximize)","[CHOOSE optimization sense]"},
		{{"minimize",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,minMaxType_minimize)},13,15,"N_mdm(lit,minMaxType_minimize)","@"}
		},
	kw_2[11] = {
		{{"bfgs",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,methodName_dot_bfgs)},6,7,"N_mdm(lit,methodName_dot_bfgs)",0,"Optimization: Local, Derivative-based"},
		{{"dot_bfgs",0x20000,0,1,1,0,0.,0.,-1,N_mdm(lit,methodName_dot_bfgs)},7,6,"N_mdm(lit,methodName_dot_bfgs)"},
		{{"dot_frcg",0x20000,0,1,1,0,0.,0.,4,N_mdm(lit,methodName_dot_frcg)},3,2,"N_mdm(lit,methodName_dot_frcg)"},
		{{"dot_mmfd",0x20000,0,1,1,0,0.,0.,4,N_mdm(lit,methodName_dot_mmfd)},5,4,"N_mdm(lit,methodName_dot_mmfd)"},
		{{"dot_slp",0x20000,0,1,1,0,0.,0.,5,N_mdm(lit,methodName_dot_slp)},9,8,"N_mdm(lit,methodName_dot_slp)"},
		{{"dot_sqp",0x20000,0,1,1,0,0.,0.,5,N_mdm(lit,methodName_dot_sqp)},11,10,"N_mdm(lit,methodName_dot_sqp)"},
		{{"frcg",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,methodName_dot_frcg)},2,3,"N_mdm(lit,methodName_dot_frcg)",0,"Optimization: Local, Derivative-based"},
		{{"mmfd",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,methodName_dot_mmfd)},4,5,"N_mdm(lit,methodName_dot_mmfd)",0,"Optimization: Local, Derivative-based"},
		{{"optimization_type",0x20008,2,2,0,(KeyWord*)kw_1},12,13,0,"{Optimization type} http://www.cs.sandia.gov/dakota/licensing/votd/html-ref/MethodCommands.html#MethodDOTDC"},
		{{"slp",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,methodName_dot_slp)},8,9,"N_mdm(lit,methodName_dot_slp)",0,"Optimization: Local, Derivative-based"},
		{{"sqp",0x20008,0,1,1,0,0.,0.,0,N_mdm(lit,methodName_dot_sqp)},10,11,"N_mdm(lit,methodName_dot_sqp)",0,"Optimization: Local, Derivative-based"}
		},
	kw_3[1] = {
		{{"dot",0x20008,11,1,1,(KeyWord*)kw_2,0.,0.,0,dot_start},1,1,"dot_start"}
		};

} // namespace Dakota

#ifdef __cplusplus
extern "C" {
KeyWord *keyword_add(void);
}
#endif

 KeyWord*
keyword_add(void) {
	return &Dakota::kw_3[0].kw;
	}
