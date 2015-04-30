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

#if 0
#include <acro_config.h>
#if !defined(TFLOPS)

#define USING_PROBE

//#include <scolib/Factory.h>

#include <colin/OptApplications.h>
//#include <colin/OptSolverHandle.h>
#include <colin/OptSolverUtil.h>
#include <colin/SolverMngr.h>
#include <colin/SolverHandle.h>

#include <sys/types.h>

using namespace std;
using namespace utilib;

void probe() {}


void test_function(vector<double>& x, colin::real& ans)
{
ans = 0.0;
for (size_type i=0; i<x.size(); i++)
  ans += x[i]*x[i];
}

int main(int argc, char* argv[])
{
utilib::exception_mngr::set_mode(utilib::exception_mngr::Abort);
#if defined(ACRO_HAVE_MPI)
uMPI::init(&argc,&argv,MPI_COMM_WORLD);

#if 0
int tmp;
if (uMPI::rank == 0) {
   cerr << "Processor 0 waiting: pid = " << getpid() << endl;
   cin >> tmp;
   }
if (uMPI::rank == 1) {
   cerr << "Processor 1 waiting: pid = " << getpid() << endl;
   cin >> tmp;
   }
uMPI::barrier();
#endif
#endif

CommonIO::begin();
#if defined(ACRO_HAVE_MPI)
if (uMPI::rank == 0)
   CommonIO::end_tagging();
#endif


if (argc == 1) {
   cout << "simple [<solver>] [<parameter-file>] [<seed>]" << endl;
   CommonIO::end();
   exit(1);
   }

string solver_str;
string filename;
int seed;

filename = "scolib.in";
seed = 1234;

solver_str = argv[1];
if (argc > 2)
   filename = argv[2];
if (argc > 3)
   seed = atoi(argv[3]);


try {

//
// Set up the problem
// void test_function(vector<double>& x, double& answer);
//
colin::ConfigurableApplication<colin::NLP0_problem> *app
   = colin::new_application("docking_app",&test_function);
app->set_num_real_vars(3);
app->set_bounds("[1.0,10.0]^3");

colin::Problem<colin::NLP0_problem> problem;
problem.set_application( app );
vector<double> initpt(3);
initpt << 9.0;

//
// Setup the solver
// Other names for solvers: direct, ms, ms.ps, ms.sw, sw, eareal, apps
//
colin::SolverHandle solver = colin::SolverMngr().create_solver(solver_str.c_str());
//scolib::ColinySolver<colin::Problem<vector<double> >,vector<double> > solver;
//solver.initialize(solver_str.c_str());

if (solver_str == "help") {
   colin::SolverMngr().list_solver_types(cerr);
   exit(1);
   }

if (filename == "help") {
   solver.solver()->help_parameters(cout);
   exit(1);
   }

ifstream ifstr;
ifstr.open(filename.c_str());
if (ifstr)
   solver.solver()->read_parameter_values(ifstr);

//
// Minimize and generate summary statistics about the optimizer
//
vector<double> best_point;
colin::real best_value;

solver.solver()->set_initial_point(initpt);
#if defined(ACRO_HAVE_MPI)
solve(solver, seed, true, true, utilib::uMPI::comm);
#else
int dummy = 0;
solve(solver, seed, true, true, dummy);
#endif

//colin::solve(*(solver->solver()), problem, initpt, seed, true, true, best_point, best_value);
}

catch (std::exception& err) {
  cerr << "Caught STL exception: " << err.what() << endl;
  }

catch (const char* err) {
  cerr << "Caught string exception: " << err << endl;
  }

ucout << flush;
CommonIO::end();

#if defined(ACRO_HAVE_MPI)
uMPI::done();
#endif
return 0;
}

#endif
#else
int main()
{return 0;}
#endif
