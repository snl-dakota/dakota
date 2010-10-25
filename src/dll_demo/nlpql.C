#include "nidr.h"
#include "NIDRProblemDescDB.H"
#include "NLPQLPOptimizer.H"

namespace Dakota {

 struct
Meth_Info {
	DataMethodRep *dme;
	DataMethod *dme0;
	};

extern NLPQLPOptimizer *(*new_NLPQLPOptimizer)(Model& model);
extern  NLPQLPOptimizer *(*new_NLPQLPOptimizer1)(NoDBBaseConstructor, Model &model);

 static NLPQLPOptimizer *
my_new_NLPQLPOptimizer(Model& model)
{
	return new NLPQLPOptimizer(model);
	}

 static NLPQLPOptimizer *
my_new_NLPQLPOptimizer1(NoDBBaseConstructor, Model& model)
{
	return new NLPQLPOptimizer(NoDBBaseConstructor(), model);
	}

 static void
nlpql_start(const char *keyname, Values *val, void **g, void *v)
{
	(*(Meth_Info**)g)->dme->methodName = "nlpql_sqp";
	new_NLPQLPOptimizer  = my_new_NLPQLPOptimizer;
	new_NLPQLPOptimizer1 = my_new_NLPQLPOptimizer1;
	}

#define N_mdm(x,y)	NIDRProblemDescDB::method_##x,&method_mp_##y

/** 1 distinct keywords (plus 0 aliases) **/

static KeyWordx
	kw_1[1] = {
		{{"nlpql_sqp",0x20008,0,1,1,0,0.,0.,0,nlpql_start},1,1,"N_mdm(lit,methodName_nlpql_sqp)"}
		};

} // namespace Dakota

#ifdef __cplusplus
extern "C" {
KeyWord *keyword_add(void);
}
#endif

 KeyWord*
keyword_add(void) {
	return &Dakota::kw_1[0].kw;
	}
