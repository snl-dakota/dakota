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

#include <acro_config.h>
//#if !defined(TFLOPS)
#if 0

//#define USING_PROBE
//#define VERBOSE
#define FOCUSED_INIT

#include <sys/types.h>
#include <utilib/CharString.h>
//#include <colin/OptSetup.h>
#include <colin/OptSolverUtil.h>
#include <scolib/Factory.h>

using namespace std;
using namespace utilib;

void probe() {}

vector<double> target;
double target_z;

namespace {

void renormalize(double& nx, double& ny, double& nz)
{
double norm = sqrt(nx*nx + ny*ny + nz*nz);
nx /= norm;
ny /= norm;
nz /= norm;
}


double compute_z(double x, double y)
{
double tmp = 1.0 - x*x - y*y;
if (tmp < -1e-7)
   EXCEPTION_MNGR(runtime_error, "Bad x/y values!");
return sqrt(max(0.0,tmp));
}

}

#if 0
void test_starting_points(vector <vector<double> >& possiblepts, int spaces)
{
vector<double> tvec(3);
tvec << 0.0;

for (int temp=0; temp<spaces+1; temp++) {
  for (int t2=0; t2<=temp; t2++) {
    //
    // Setup a point on a triangle that defines a single quadrant of a sphere
    //
    tvec[0]  = 1.0 + (1.0/sqrt(2.0) - 1.0)*static_cast<double>(temp)/spaces + 
	       	(1.0/sqrt(2.0) - 1/sqrt(3.0))*static_cast<double>(t2)/spaces;
    tvec[1] = (1/sqrt(2.0))*static_cast<double>(temp)/spaces - 
		(1/sqrt(2.0) - 1/sqrt(3.0))*static_cast<double>(t2)/spaces;
    tvec[2] = (1/sqrt(3.0))*static_cast<double>(t2)/spaces;
    //
    // Normalize point to lie on the sphere
    //
    double norm = sqrt(tvec[0]*tvec[0]+tvec[1]*tvec[1]+tvec[2]*tvec[2]);
    tvec[0] /= norm;
    tvec[1] /= norm;
    tvec[2] /= norm;
    possiblepts.push_back(tvec);
    }
  }
}
#endif

void core_test_function(double x, double y, double z, colin::real& ans)
{
renormalize(x,y,z);
ans = 1 - max(
	 x*target[3] + y*target[4] + z*target_z,
	-x*target[3] - y*target[4] - z*target_z);
}




void dockingps_test_function(vector<double>& x, colin::real& ans)
{
#if 0
ans = 0.0;
ans += (x[0]-target[0])*(x[0]-target[0]);
ans += (x[1]-target[1])*(x[1]-target[1]);
ans += (x[2]-target[2])*(x[2]-target[2]);
ans += min( 
       (x[3]-target[3])*(x[3]-target[3]) + (x[4]-target[4])*(x[4]-target[4]),
       (x[3]+target[3])*(x[3]+target[3]) + (x[4]+target[4])*(x[4]+target[4]));
for (size_type i=6; i<x.size(); i++) {
  double tmp = x[i];
  if (tmp < 0.0)
     tmp += 2*M_PI;
  ans += (tmp-target[i])*(tmp-target[i]);
  }
#endif

core_test_function(x[3],x[4], compute_z(x[3],x[4]), ans);
}

void simple_test_function(vector<double>& x, colin::real& ans)
{
// renormalize the x,y quarternion parameters
if ((x[3]*x[3]+x[4]*x[4]) > 1.0) {
   double tmp = sqrt(x[3]*x[3]+x[4]*x[4]);
   x[3] /= tmp;
   x[4] /= tmp;
   }
// renormalize the quarternion angle
while (x[5] > 2*M_PI)
  x[5] -= 2*M_PI;
while (x[5] < 0.0)
  x[5] += M_PI;
// renormalize the torsion angles
for (size_type i=6; i<x.size(); i++) {
  while (x[i] > 2*M_PI)
    x[i] -= 2*M_PI;
  while (x[i] < 0.0)
    x[i] += 2*M_PI;
  }
//
// Call the docking test function, which assumes that bounds are
// preserved in the solution
//
core_test_function(x[3],x[4], compute_z(x[3],x[4]), ans);
}


void basic_test_function(vector<double>& x, colin::real& ans)
{
// renormalize the x,y quarternion parameters
renormalize(x[3],x[4],x[5]);
// renormalize the quarternion angle
while (x[6] > 2*M_PI)
  x[6] -= 2*M_PI;
while (x[6] < 0.0)
  x[6] += M_PI;
// renormalize the torsion angles
for (size_type i=7; i<x.size(); i++) {
  while (x[i] > 2*M_PI)
    x[i] -= 2*M_PI;
  while (x[i] < 0.0)
    x[i] += 2*M_PI;
  }
// Call the docking test function, which assumes that bounds are
// preserved in the solution
core_test_function(x[3],x[4],x[5],ans);
}


void setup_problem(colin::Problem<vector<double> >& problem,
				const char* solver_str)
{
CharString solver = solver_str;
if (solver == "DockingPS")
   colin::OptSetup(problem,dockingps_test_function,"[0.0,1.0]^3 [-1.0,1.0]^2 [0.0,6.2832] [0.0,6.2832]");
else if (solver == "ps-new")
   colin::OptSetup(problem,simple_test_function,"[0.0,1.0]^3 [-1.0,1.0]^2 [-6.2832,12.5664] [-6.2832,12.5664]");
else
   colin::OptSetup(problem,basic_test_function,"[0.0,1.0]^3 [-1.0,1.0]^3 [-6.2832,12.5664] [-6.2832,12.5664]");
// TODO - make this target a function of the seed
//target.resize(7);
//target << 0.1;
//target[3] = 0.1;
//target[4] = 0.1;
}



int run_test1(string& filename, int seed)
{
//
// Set up the problem
// void test_function(vector<double>& x, double& answer);
// First three parameters are the position, the next two are the (x,y) of
// the quarternion, the next is the angle of the quarternion, and the rest
// are the torsion angles.
//
colin::Problem<vector<double> > problem;
setup_problem(problem,"DockingPS");
//
// Setup the solver
// Other names for solvers: direct, ms, ms.ps, ms.sw, sw, eareal, apps
//
colin::OptSolverHandle<vector<double>,colin::AppResponse<> > solver =
	scolib::Factory<vector<double>,colin::AppResponse<> >("DockingPS");
if (filename == "help") {
   solver->help_parameters(cout);
   exit(1);
   }
ifstream ifstr;
ifstr.open(filename.c_str());
if (ifstr)
   solver->read_parameter_values(ifstr);
//
// Create an initial point and minimize from it.
//
vector<double> initpt(7);
#ifndef FOCUSED_INIT
for (unsigned int i=0; i<initpt.size(); i++)
  initpt[i] = 1.0/(i+2.3);
#else
initpt << 0.1;
initpt[3] = 1.0/(3+2.3);
initpt[4] = 1.0/(4+2.3);
#endif
vector<double> best_point;
colin::real best_value;
#ifdef VERBOSE
solve(solver,problem, initpt, seed, true, true, best_point, best_value);
#else
solve(solver,problem, initpt, seed, false, false, best_point, best_value);
#endif
if (best_value > 1e-4) return -1;
return problem.neval();
}


int run_test2(string& filename, int seed)
{
//
// Set up the problem
// void test_function(vector<double>& x, double& answer);
// First three parameters are the position, the next two are the (x,y) of
// the quarternion, the next is the angle of the quarternion, and the rest
// are the torsion angles.
//
colin::Problem<vector<double> > problem;
setup_problem(problem,"ps-new");
//
// Setup the solver
// Other names for solvers: direct, ms, ms.ps, ms.sw, sw, eareal, apps
//
colin::OptSolverHandle<vector<double>,colin::AppResponse<> > solver =
	scolib::Factory<vector<double>,colin::AppResponse<> >("DockingPS");
if (filename == "help") {
   solver->help_parameters(cout);
   exit(1);
   }
ifstream ifstr;
ifstr.open(filename.c_str());
if (ifstr)
   solver->read_parameter_values(ifstr);
solver->set_parameter_with_string("basis","coordinate");
//
// Create an initial point and minimize from it.
//
vector<double> initpt(7);
#ifndef FOCUSED_INIT
for (unsigned int i=0; i<initpt.size(); i++)
  initpt[i] = 1.0/(i+2.3);
#else
initpt << 0.1;
initpt[3] = 1.0/(3+2.3);
initpt[4] = 1.0/(4+2.3);
#endif
vector<double> best_point;
colin::real best_value;
#ifdef VERBOSE
solve(solver, problem, initpt, seed, true, true, best_point, best_value);
#else
solve(solver, problem, initpt, seed, false, false, best_point, best_value);
#endif
if (best_value > 1e-4) return -1;
return problem.neval();
}


int run_test3(string& filename, int seed)
{
//
// Set up the problem
// void test_function(vector<double>& x, double& answer);
// First three parameters are the position, the next three are the (x,y,z) of
// the quarternion, the next is the angle of the quarternion, and the rest
// are the torsion angles.
//
colin::Problem<vector<double> > problem;
setup_problem(problem,"ps-old");
//
// Setup the solver
// Other names for solvers: direct, ms, ms.ps, ms.sw, sw, eareal, apps
//
colin::OptSolverHandle<vector<double>,colin::AppResponse<> > solver =
	scolib::Factory<vector<double>,colin::AppResponse<> >("ps");
if (filename == "help") {
   solver->help_parameters(cout);
   exit(1);
   }
ifstream ifstr;
ifstr.open(filename.c_str());
if (ifstr)
   solver->read_parameter_values(ifstr);
//
// Create an initial point and minimize from it.
//
vector<double> initpt(8);
#ifndef FOCUSED_INIT
initpt[0] = 1.0/2.3;
initpt[1] = 1.0/3.3;
initpt[2] = 1.0/4.3;
initpt[3] = 1.0/5.3;
initpt[4] = 1.0/6.3;
initpt[5] = compute_z(initpt[3],initpt[4]);
for (unsigned int i=6; i<initpt.size(); i++)
  initpt[i] = 1.0/(i+1.3);
#else
initpt << 0.1;
initpt[3] = 1.0/(3+2.3);
initpt[4] = 1.0/(4+2.3);
initpt[5] = compute_z(initpt[3],initpt[4]);
#endif
vector<double> best_point;
colin::real best_value;
#ifdef VERBOSE
solve(solver, problem, initpt, seed, true, true, best_point, best_value);
#else
solve(solver, problem, initpt, seed, false, false, best_point, best_value);
#endif
if (best_value > 1e-4) return -1;
return problem.neval();
}


void do_comparison(string& filename, int seed)
{
target_z = compute_z(target[3],target[4]);
int neval1 = run_test1(filename,seed);
ucout << flush;
//int neval2 = run_test2(filename,seed);
//ucout << flush;
int neval3 = run_test3(filename,seed);
ucout << flush;
ucout << "Comparison:  " << " DockingPS " << neval1 << " ps-old " << neval3 << 
		" Point: " << target[3] << " " << target[4] << " " << target_z << endl;

target_z = -compute_z(target[3],target[4]);
neval1 = run_test1(filename,seed);
ucout << flush;
//neval2 = run_test2(filename,seed);
//ucout << flush;
neval3 = run_test3(filename,seed);
ucout << flush;
ucout << "Comparison:  " << " DockingPS " << neval1 << " ps-old " << neval3 << 
		" Point: " << target[3] << " " << target[4] << " " << target_z << endl;
}


int main(int argc, char* argv[])
{
//
// Misc setup
// 
//utilib::exception_mngr::set_exit_function(&exit_fn);
utilib::exception_mngr::set_mode(utilib::exception_mngr::Abort);
CommonIO::begin();
//
// Process arguments
//
if (argc < 2) {
   cout << "docking <seed> [<parameter-file>]" << endl;
   CommonIO::end();
   exit(1);
   }
int seed = atoi(argv[1]);
string filename = "scolib.in";
if (argc > 2)
   filename = argv[2];

target.resize(8);
target << 0.1;

try {

int k=100;
#if 1
//int ctr=0;
for (int i=0; i<k; i++) {
  for (int j=0; j<(k-i); j++) {
    //ctr++;
    //if (ctr == 100) return 0;
    target << 0.1;
    double x = static_cast<double>(j)/(k-1);
    double y = static_cast<double>(i)/(k-1);
    double z = static_cast<double>(k-1-i-j)/(k-1);
    double norm = sqrt(x*x+y*y+z*z);
    target[3] = x/norm;
    target[4] = y/norm;
    do_comparison(filename, seed);
    }
  }
#else

target << 0.1;
target[3] = 0.25;
target[4] = 0.25;
double tmp = 0.0;
renormalize(target[3],target[4],tmp);
//double norm = sqrt(target[3]*target[3] + target[4]*target[4] + 
			//(k-1-6-68)/(99*99.0));
//target[3] /= norm;
//target[4] /= norm;
do_comparison(filename,seed);
#endif
}

catch (std::exception& err) {
  cerr << "Caught STL exception: " << err.what() << endl;
  }

catch (const char* err) {
  cerr << "Caught string exception: " << err << endl;
  }

ucout << flush;
CommonIO::end();

return 0;
}

#else

int main()
{
return -1;
}


#endif
