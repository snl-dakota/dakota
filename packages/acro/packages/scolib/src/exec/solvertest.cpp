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
// solvertest.cpp
//
// Test solvers on various problem formulations
//

#include <acro_config.h>
#if !defined(TFLOPS)

#include <utilib/_math.h>
#include <utilib/PM_LCG.h>
#include <utilib/exception_mngr.h>
#include <utilib/mpiUtil.h>
//#include <scolib/scolib.h>

using namespace std;
using namespace colin;
using namespace utilib;

bool debug_flag=false;

void test_func0(BasicArray<double>& , real& ans)
{
ans = 1.0;
}


void test_func1(BasicArray<double>& x, real& ans)
{
ans=0.0;
for (unsigned int i=0; i<x.size(); i++)
  ans += x[i]*x[i];
}

double test_func2_value = 1.0;
void test_func2(BasicArray<double>& , real& ans)
{
ans = 1.0+1.0/(pow(2.0,test_func2_value));
test_func2_value += 1.0;
}

void test_func12(BasicArray<double>& x, real& ans)
{
ans=0.0;
for (unsigned int i=0; i<x.size(); i++)
  ans += x[i] + 10*sin(x[i])+1.0;
}

void test_rosenbrock(BasicArray<double>& x, real& ans)
{
           double          a, b, c;
           a = x[0];
           b = x[1];
           c = 100.0 * (b - (a * a)) * (b - (a * a));
           ans = (c + ((1.0 - a) * (1.0 - a)));

}


//
// x^2 + (y-2)^2 + z^2< 3
// (x-2)^2 + y^2 + z^2< 3
//
// Local min at (1-1/sqrt(2), 1-1/sqrt(2), 0)
//
void constr_func1(BasicArray<double>& x, BasicArray<real >& cvals)
{
cvals.resize(2);
cvals[0] = 3.0 - x[0]*x[0] - (x[1]-2.0)*(x[1]-2.0) - x[2]*x[2];
cvals[1] = 3.0 - x[1]*x[1] - (x[0]-2.0)*(x[0]-2.0) - x[2]*x[2];
}


//
// x^2 + (y-2)^2 + z^2 < 3
// (x-2)^2 + y^2 + z^2 < 3
// x + y + z < 2
//
// Local min at (1-1/sqrt(2), 1-1/sqrt(2), 0)
//
void constr_func2(BasicArray<double>& x, BasicArray<real >& cvals)
{
cvals.resize(3);
cvals[0] = 3.0 - x[0]*x[0] - (x[1]-2.0)*(x[1]-2.0) - x[2]*x[2];
cvals[1] = 3.0 - x[1]*x[1] - (x[0]-2.0)*(x[0]-2.0) - x[2]*x[2];
cvals[2] = -(x[0] + x[1] + x[2]-2);
}


void test_solver(const char* probid, const char* name, int seed,
						const char* infile=0)
{
//
// Information
//
ucout << "Solver: " << name << endl;
ucout << "Problem: " << probid << endl;
//
// Setup initial point and bounds
//
BasicArray<double> pt(3);
BasicArray<double> best_point(3);
BasicArray<double> optimum(3);
BasicArray<real> lb(3), ub(3);
for (unsigned int i=0; i<pt.size(); i++)
  pt[i] = 30.0*(1.0-1.0/sqrt(i+2.0));
lb << 3.0;
ub << 30.0;

Problem<BasicArray<double>, colin::AppResponse_Utilib > prob;

//
// Problem definitions
//
if (strcmp(probid,"0") == 0) {
   OptSetup(prob,&test_func0);
   prob.init_real_params(pt.size());
   optimum = pt;
   ucout << "Problem-Summary: constant function" << endl;
   }
else if (strcmp(probid,"1") == 0) {
   OptSetup(prob,&test_func1);
   prob.init_real_params(pt.size());
   optimum << 0.0;
   ucout << "Problem-Summary: quadratic function, unconstrained" << endl;
   }
else if (strcmp(probid,"2") == 0) {
   OptSetup(prob,&test_func1);
   prob.set_real_bounds(lb,ub);
   optimum << 3.0;
   ucout << "Problem-Summary: quadratic function, bound-constrained" << endl;
   }
else if (strcmp(probid,"3") == 0) {
   OptSetup(prob,&test_func1,&constr_func1,2);
   optimum[0] = 1.0 - 1.0/sqrt(2.0);
   optimum[1] = 1.0 - 1.0/sqrt(2.0);
   optimum[2] = 0.0;
   ucout << "Problem-Summary: quadratic function, nonlinear inequality constraints" << endl;
   }
else if (strcmp(probid,"4") == 0) {
   OptSetup(prob,&test_func1,&constr_func1,2);
   lb << 1.0;
   prob.set_real_bounds(lb,ub);
   prob.real_lower_bound_type(0,colin::no_bound);
   prob.real_lower_bound_type(2,colin::no_bound);
   optimum[0] = 2.0 - sqrt(2.0);
   optimum[1] = 1.0;
   optimum[2] = 0.0;
   ucout << "Problem-Summary: quadratic function, nonlinear inequality constraints + lower bound on variable x2" << endl;
   }
else if (strcmp(probid,"5") == 0) {
   OptSetup(prob,&test_func1,&constr_func2,2,1);
   optimum[0] = 2.0/3.0;
   optimum[1] = 2.0/3.0;
   optimum[2] = 2.0/3.0;
   ucout << "Problem-Summary: quadratic function, nonlinear inequality constraints + nonlinear equality constraint" << endl;
   }
else if (strcmp(probid,"6") == 0) {
   OptSetup(prob,&test_func0);
   prob.init_real_params(pt.size());
   lb << -1.0;
   ub << 30.0;
   prob.set_real_bounds(lb,ub);
   optimum << 14.5;
   ucout << "Problem-Summary: constant function + nonbinding bound constraints" << endl;
   }
else if (strcmp(probid,"7") == 0) {
   OptSetup(prob,&test_func1);
   prob.init_real_params(pt.size());
   lb << -1.0;
   ub << 30.0;
   prob.set_real_bounds(lb,ub);
   optimum << 0.0;
   ucout << "Problem-Summary: quadratic function + nonbinding bound constraints" << endl;
   }
else if (strcmp(probid,"8") == 0) {
   OptSetup(prob,&test_func1,&constr_func1,2);
   lb << -1.0;
   ub << 30.0;
   prob.set_real_bounds(lb,ub);
   optimum[0] = 1.0 - 1.0/sqrt(2.0);
   optimum[1] = 1.0 - 1.0/sqrt(2.0);
   optimum[2] = 0.0;
   ucout << "Problem-Summary: quadratic function, nonlinear inequality constraints + nonbinding bound constraints" << endl;
   }
else if (strcmp(probid,"9") == 0) {
   OptSetup(prob,&test_func1,&constr_func2,2,1);
   lb << -1.0;
   ub << 30.0;
   prob.set_real_bounds(lb,ub);
   optimum[0] = 2.0/3.0;
   optimum[1] = 2.0/3.0;
   optimum[2] = 2.0/3.0;
   ucout << "Problem-Summary: quadratic function, nonlinear inequality constraints + nonlinear equality constraint + nonbinding bound constraints" << endl;
   }
else if (strcmp(probid,"10") == 0) {
   OptSetup(prob,&test_func2);
   test_func2_value = 1.0;
   lb << -1.0;
   ub << 30.0;
   prob.set_real_bounds(lb,ub);
   optimum << 0.0;
   ucout << "Problem-Summary: a test function for which each function evaluation is better" << endl;
   }
else if (strcmp(probid,"11") == 0) {
   OptSetup(prob,&test_func1);
   pt.resize(20);
   for (unsigned int i=0; i<pt.size(); i++)
     pt[i] = 30.0*(1.0-1.0/sqrt(i+2.0));
   lb.resize(20);
   ub.resize(20);
   optimum.resize(20);
   prob.init_real_params(pt.size());
   lb << -0.01;
   ub << 30.0;
   prob.set_real_bounds(lb,ub);
   optimum << 0.0;
   ucout << "Problem-Summary: quadratic function + nearly-binding bound constraints" << endl;
   }
else if (strcmp(probid,"12") == 0) {
   OptSetup(prob,&test_func12);
   pt.resize(10);
   lb.resize(10);
   ub.resize(10);
   optimum.resize(10);
   prob.init_real_params(pt.size());
   pt << 10.0;
   lb << 10.0;
   ub << 50.0;
   prob.set_real_bounds(lb,ub);
   optimum << 10.8954;
   ucout << "Problem-Summary: linear + sinusoidal + bound constraints" << endl;
   }
else if (strcmp(probid,"13") == 0) {
   OptSetup(prob,&test_func1);
   prob.set_real_bounds(lb,ub);
   optimum[0] = 0.0;
   optimum[1] = 3.0;
   optimum[2] = 0.0;
   prob.real_lower_bound_type(0,colin::no_bound);
   prob.real_lower_bound_type(2,colin::no_bound);
   ucout << "Problem-Summary: quadratic function, partially bound-constrained" << endl;
   }
else if (strcmp(probid,"14") == 0) {
   OptSetup(prob,&test_rosenbrock);
   pt.resize(2);
   optimum.resize(2);
   prob.init_real_params(pt.size());
   pt[0] = -1.2;
   pt[1] = 9.0;
   optimum[0] = 1.0;
   optimum[1] = 1.0;
   ucout << "Problem-Summary: rosenbrock's parabolic valley" << endl;
   }

ifstream ifstr;
if (infile) {
   ifstr.open(infile);
   if (!ifstr)
      EXCEPTION_MNGR(runtime_error, "Missing parameter file: " << infile);
   ucout << "Param-File: " << infile << endl;
   }
else {
   ucout << "Param-File: none" << endl;
   }

//
// Local searchers
//
real best_value;
if ((strcmp(name,"SolisWets") == 0) || (strcmp(name,"sw")==0) ||
#ifndef COLINY_WITHOUT_COBYLA
    (strcmp(name,"Cobyla") == 0) || (strcmp(name,"cobyla")==0) ||
#endif
#ifndef COLINY_WITHOUT_APPS
    (strcmp(name,"APPS") == 0) || (strcmp(name,"apps")==0) ||
#endif
    (strcmp(name,"PatternSearch") == 0) || (strcmp(name,"ps")==0)) {
   scolib::solve(name,prob,pt,seed,ifstr,debug_flag,true,optimum,best_point,best_value);
   }

else 
   scolib::solve(name,prob,seed,ifstr,debug_flag,true,optimum,best_point,best_value);
}



int main(int argc, char* argv[])
{
#if defined(ACRO_HAVE_MPI)
uMPI::init(&argc,&argv,MPI_COMM_WORLD);
#endif

CommonIO::begin();
#if defined(ACRO_HAVE_MPI)
if (uMPI::rank == 0)
   CommonIO::end_tagging();
#endif

if (argc == 1) {
   ucerr << "solvertest [-debug] <prob-id> <solver1> <seed> [<input-file>]" << endl;
   ucerr << "\toptimizers: sw ps direct pdirect apps cobyla eareal ms ms-sw ms-ps" << endl;
   ucerr << "Problems:" << endl;
   ucerr << "\t0  Constant" << endl;
   ucerr << "\t1  Unconstrained quadratic" << endl;
   ucerr << "\t2  Bound-constrained quadratic" << endl;
   ucerr << "\t3  Partially bound-constrained quadratic" << endl;
   ucerr << "\t4  Quadratic function, nonlinear inequality constraints +\n\t\t lower bound on variable x2" << endl;
   ucerr << "\t5  Quadratic function, nonlinear inequality constraints +\n\t\t nonlinear equality constraint" << endl;
   ucerr << "\t6  Constant function + nonbinding bound constraints" << endl;
   ucerr << "\t7  Quadratic function + nonbinding bound constraints" << endl;
   ucerr << "\t8  Quadratic function, nonlinear inequality constraints +\n\t\t nonbinding bound constraints" << endl;
   ucerr << "\t9  Quadratic function, nonlinear inequality constraints +\n\t\t nonlinear equality constraint + nonbinding bound constraints" << endl;
   ucerr << "\t10 A test function for which each function evaluation is better" << endl;
   ucerr << "\t11 Quadratic function + nearly-binding bound constraints" << endl;
   ucerr << "\t12 Linear + sinusoidal + bound constraints" << endl;
   ucerr << "\t13 Quadratic function, partially bound-constrained" << endl;
   ucerr << "\t14 Rosenbrocks parabolic valley" << endl;
   CommonIO::end();
   return 1;
   }

try {
  int ndx = 1;
  //utilib::exception_mngr::set_exit_function(&exit_fn);
  if (strcmp(argv[ndx],"-debug") == 0) {
     debug_flag = true;
     ndx++;
     utilib::exception_mngr::set_mode(utilib::exception_mngr::Abort);
     }

  int seed = atoi(argv[ndx+2]);
  if (argc == (ndx+3))
     test_solver(argv[ndx],argv[ndx+1],seed);
  else if (argc == (ndx+4))
     test_solver(argv[ndx],argv[ndx+1],seed,argv[ndx+3]);
  else
     EXCEPTION_MNGR(runtime_error,"Number of arguments does not work: " << argc);
  ucout << "Status: success" << endl;
  }
catch (const char* str){
  ucout << "Error: " << str << endl;
  ucout << "Status: failed" << endl;
  }
catch (const std::exception& err){
  ucout << "Error: " << err.what() << endl;
  ucout << "Status: failed" << endl;
  }

ucout << flush;

CommonIO::end();
#if defined(ACRO_HAVE_MPI)
uMPI::done();
#endif

return 0;
}


#else

int main()
{
return -1;
}

#endif
