/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

//
// NSGA2.cpp
//

#include <acro_config.h>
#ifdef ACRO_USING_NSGA2

#include <scolib/NSGA2.h>
#include <scolib/NSGA2_objective.h>

using namespace std;
using namespace utilib;

namespace scolib {

namespace nsga2 {

class scolib_objective:public objective {
public:
    void evaluate(double *, int , double *, int);
    scolib_objective(colin::Problem<BasicArray<double>,colin::AppResponse_Utilib>* p) : problem(p) {}
        
colin::Problem<BasicArray<double>,colin::AppResponse_Utilib>* problem;
};


void scolib_objective::evaluate(double *x, int n, double *f, int m) {
   BasicArray<double> point(x, n, DataNotOwned);
   BasicArray<double> objectives(f, m, DataNotOwned);

   problem->EvalF(point, objectives);
}

}

NSGA2::NSGA2() {

// func.solver = this;
// func.problem = &problem;
// solver->set_func(&func);

//option.disable("accuracy");
properties.erase("sufficient_objective_value");
//option.disable("ftol");
properties.erase("function_value_tolerance");

}

void NSGA2::reset()
{
if (!problem)
   return;

if (problem.has_all_bounds())
   problem.get_real_bounds(lower,upper);
// TODO: throw exception if lower/upper unknown

}


void NSGA2::minimize() {

    if (problem.num_real_vars() == 0) {
       best().termination_info = "No-Real-Params";
       return;
    }
    if (!problem.has_all_bounds()) {
       best().termination_info = "Missing-Bound-Constraints";
       return;
    }

    opt_init();

    int tmp_neval = max(max_neval-problem.neval(),0);
    if (max_neval_curr != 0)
       tmp_neval = min(tmp_neval, max_neval_curr);

    try {
	const int GeneLen=30;
	int NGenes=10;		// number of genes in the chromosome
	int N = 100;		// populations
	int GEN = 200;		// generation
	int SEED = 1050;         
	double pc=0.15,pm=0.06;
	int objID=1;
	int NObjs=2;			// number of objectives

	pc=0.9;
	pm=0.01;
	operators_NSGA2 opNSGA2(pc,pm,0);

	int *GSize = new int[NGenes];	// length of each gene
	for (int i=0; i< NGenes; i++) {
	  GSize[i] = GeneLen;
	}

	opNSGA2.obj = new scolib_objective(&problem);
	opNSGA2.dev=0;
	// opNSGA2.obj.setObjfunc(&foo); // syc test
	opNSGA2.Evolve(N,GEN,SEED,NGenes,NObjs,GSize,Ub,Lb); 
    }
    STD_CATCH(;)

    best().point << x;
    if (problem.numConstraints() > 0)
       problem.Eval(x, best().response, colin::mode_f | colin::mode_cf);
    else
       problem.Eval(x, best().response, colin::mode_f);
    compute_response_info(best().response,problem.state->constraint_lower_bounds,  problem.state->constraint_upper_bounds, best().value(),best().constraint_violation);
}

} 
#endif
