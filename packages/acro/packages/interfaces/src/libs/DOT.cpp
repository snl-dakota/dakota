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
// DOT.cpp
//

#include <acro_config.h>
#if 0
#ifndef COLINY_WITHOUT_DOT

#include <interfaces/DOT.h>


#if !defined(RS6K)
#define dot dot_
#endif

extern "C" {
 
void dot( int& info, int& ngotoz, int& method, int& iprint, int& ndv,
        int& ncon, double* x, double* xl, double* xu, double& obj,
        int& minmax, double* g, double* rprm, int* iprm, double* wk,
        int& nrwk, int* iwk, int& nriwk );
 
}


namespace interfaces {

using colin::real;

DOT_CG::DOT_CG()
  : realWorkSpace(1000),
    intWorkSpace(300),
    constraintArray(1),
    realCntlParmArray(20),
    intCntlParmArray(20)
{
ftol=1e-8;
fdss=1e-4;
}


void DOT_CG::reset()
{
numConstraints=0;
 
dotInfo=0;
dotMethod=2;
if (debug <= 0)
   printControl=0;
else printControl=2;
optimizationType=0;
 
realCntlParmArray << 0.0;
realCntlParmArray[3] = ftol;
realCntlParmArray[8] = fdss;
intCntlParmArray << 0;
}


void DOT_CG::minimize()
{
int N = problem.num_real_vars();
if (N == 0) return;

opt_init();

int WR=1000, WI=300;
{
problem.EvalF(best_point,best_val);
}
X << best_point;
double objf_value = best_val;

lowerBoundsArray.resize(N);
upperBoundsArray.resize(N);
vector<real> tmpl(N), tmpu(N);
problem.get_real_bounds(tmpl,tmpu);
for (int i=0; i<N; i++) {
  lowerBoundsArray[i] = tmpl[i];
  upperBoundsArray[i] = tmpu[i];
  }

  for( ; ; )
  {
    // dotFDSinfo accepts the NGOTOZ parameter from DOT.  This involves a
    // change to the DOT source (adding NGOTOZ to the SUBROUTINE DOT
    // parameter list in ddot1.f).  dotFDSinfo is nonzero when DOT is
    // calculating finite difference gradients, and is zero otherwise.
 
    dotFDSinfo = 0; // Vanderplaats: Initialize to 0 before DOT call
 
    DEBUGPR(10,
      ucout << dotInfo << endl;
      ucout << dotFDSinfo << endl;
      ucout << dotMethod << endl;
      ucout << printControl << endl;
      ucout << N << endl;
      ucout << numConstraints << endl;
      ucout << X << endl;
      ucout << lowerBoundsArray << endl;
      ucout << upperBoundsArray << endl;
      ucout << objf_value << endl;
      ucout << optimizationType << endl;
      ucout << constraintArray << endl;
      ucout << realCntlParmArray << endl;
      ucout << intCntlParmArray << endl;
      ucout << WR << endl;
      ucout << WI << endl;
      ucout.flush();
            );

    dot(dotInfo, dotFDSinfo, dotMethod, printControl, N, 
	numConstraints, &(X[0]), 
	&(lowerBoundsArray[0]), &(upperBoundsArray[0]),
	objf_value, optimizationType,
        &(constraintArray[0]), &(realCntlParmArray[0]),
        &(intCntlParmArray[0]), &(realWorkSpace[0]),
        WR, &(intWorkSpace[0]), WI);
 
    if (dotInfo == 0) break;

    DEBUGPR(10,
      ucout << dotInfo << endl;
      ucout << dotFDSinfo << endl;
      ucout << dotMethod << endl;
      ucout << printControl << endl;
      ucout << N << endl;
      ucout << numConstraints << endl;
      ucout << X << endl;
      ucout << bestX << endl;
      ucout << lowerBoundsArray << endl;
      ucout << "B: " << lowerBoundsArray;
      ucout.flush();
      ucout << upperBoundsArray << endl;
      ucout << objf_value << endl;
      ucout << optimizationType << endl;
      ucout << constraintArray << endl;
      ucout << realCntlParmArray << endl;
      ucout << intCntlParmArray << endl;
      ucout << intWorkSpace << endl;
      ucout << WR << endl;
      ucout << WI << endl;
      ucout.flush();
            );

    real tmp_value;
    problem.EvalF(X,tmp_value);
    if (best_val > tmp_value) {
       best_val = tmp_value;
       best_point << X;
       DEBUGPR(10,
          ucout << "New Best: " << best_val << "::" << X << "\n";
               );
       }
    objf_value = tmp_value;

    if (check_convergence(best_val)) break;
  }

best().point << x;
problem.Eval(x, best().response, colin::mode_f | colin::mode_cf);
compute_response_info(best().response,problem.state->constraint_lower_bounds,  problem.state->constraint_upper_bounds, best().value,best().constraint_violation);
}

} // namespace interfaces

#endif
#endif
