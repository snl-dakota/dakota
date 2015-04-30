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
// TCC.cpp
//

#include <acro_config.h>
#ifdef USING_TCC

#include <utilib/Uniform.h>
#include <interfaces/TCC.h>

namespace interfaces {

using colin::real;

extern double (*urandom)();
extern void (*cluster_gopt_fn)(BasicArray<double>&, real&, int ,int );
extern int cluster_gopt(BasicArray<double>& amin, BasicArray<double>& amax, const int nparm,
                const int m, int n100, int ng0, ostream& ostr,
                const int nsig, const int max_num_clusters,
                DoubleMatrix& x0,
                int& nc, BasicArray<double>& f0, int& nfe, BasicArray<double>& work);

static Uniform urnd;
double Dummy_urandom() {return urnd();}

//
// Junk for computing the function
//
#ifdef ACRO_HAVE_TEMPLATES_AS_TEMPLATE_ARGUMENTS
static colin::Problem<BasicArray<double>,BasicArray > *localFn;
#else
static colin::Problem<BasicArray<double> >* localFn;
#endif
static BasicArray<double> vec;

void DummyFunc(BasicArray<double>& x, real& f, int nparam, int)
{
vec.set_data(nparam,&(x[1]));
localFn->EvalF(vec,f);
//ucout << "TCC Feval: " << f << ": " << vec << endl;
}

//
// Constructor
//
TCC::TCC() 
{
cluster_gopt_fn = &DummyFunc;
urandom=Dummy_urandom;
num_samples=300;
max_nmins=20;
}


void TCC::reset()
{
urnd.generator(&rng);
}


void TCC::minimize()
{
//localFn = &problem;
opt_init();

if (problem.numConstraints() > 0)
   EXCEPTION_MNGR(runtime_error, "TCCluster::minimize -- no constraints can be handled by this method!");

int m=1;
int nfe;
int nc;
work.resize(problem.num_real_vars() * (problem.num_real_vars()+1)/2 + 1);
 
problem.get_real_bounds(lower,upper);
x0.resize((problem.num_real_vars()+1),max_nmins+1);
f0.resize(max_nmins+1);
 
int nparams=problem.num_real_vars();
/*
cluster_gopt(lower,upper,nparams,m,num_samples,
        num_selected,ucout,
        nsig, max_nmins, x0, nc,f0,nfe,work);
*/

//
// Setup best value of function
//
int ndx=0;
for (int i=1; i<nc; i++)
  if (f0[i] < f0[ndx]) ndx=i;
for (unsigned int j=0; j<problem.num_real_vars(); j++)
  best_point[j] = x0[j][ndx];
best_val  = f0[ndx];
}

} // namespace interfaces

#endif
