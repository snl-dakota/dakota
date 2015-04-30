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
// scolib.cpp
//

#if 1
#include <scolib/SCORegistrations.h>
#include <colin/ColinDriver.h>
#include <utilib/exception_mngr.h>

int main(int argc, char* argv[])
{
   if ( scolib::StaticInitializers::static_scolib_registrations )
      EXCEPTION_MNGR(std::runtime_error, "Coliny: main(): Acro solver "
                     "plugin library registrations failed :"
                     << scolib::StaticInitializers::static_scolib_registrations
                     );
   return colin::colin_driver(argc,argv,"scolib");
}

#else


#include <acro_config.h>

#if defined(ACRO_USING_AMPL ) && !defined(TFLOPS)

//#include <utilib/ParameterList.h>
#include <utilib/string_ops.h>
#include <colin/OptSetup.h>
#include <scolib/Factory.h>
#include <colin/OptSolverUtil.h>

using namespace utilib;
using namespace scolib;
using namespace std;
using colin::real;

#undef NO
extern "C" {
#include "getstub.h"
#include "asl.h"
};
#undef real
#undef getenv
#define asl cur_ASL

//
// Global data
//
static char xxxvers[] = "AMPL/Coliny\0\nAMPL/Coliny 1.0 Driver Version 20040903\n";
static char* usage_msg[] = {
"  TODO "
};
static Option_Info Oinfo = {
	"scolib", "Coliny Solver Library 1.0",
	"scolib_options", 0, 0, 0, xxxvers,
	usage_msg, 0, 0, 0, 0, 20040903
	};

//
// The function that computes the objective
//
void test_function(BasicArray<double>& vec, colin::AppResponse_Utilib& response)
{
//
// Compute the function value
//
fint retcd = 0; 
for (unsigned int i=0; i<n_obj; i++)
  response.function_value(i) = objval (i, vec.data(), &retcd); 
//
// Check for errors in the function value computation
//  
if (retcd != 0){
   EXCEPTION_MNGR(std::runtime_error,"The AMPL function objval returned a nonzero error state: " << retcd << " at point " << vec);
   }
//
// Compute the constraint functions
//
BasicArray<double> constraint(n_con);  
fint nerror = 0;
conval (vec.data(), constraint.data(), &nerror);
//
// Check for errors in the constraint value computation
//
if (nerror != 0) {
   EXCEPTION_MNGR(std::runtime_error, "The AMPL function conval returned a nonzero error state: " << nerror);
   }    
//
// Compute a penalized objective value 
//
for (int i = n_conjac[0]; i < n_conjac[1]; i++) {
  response.constraint_values()[i] = constraint[i];
  #if 0
  if (constraint[i] < Al[i])
     ans += 10000.0 * (Al[i] - constraint[i]);
  if (constraint[i] > Au[i])
     ans += 10000.0 * (constraint[i] - Au[i]);
  #endif
  }

for (unsigned int i=0; i<response.request_vector().size(); i++) {
  response.request_vector()[i] &= colin::mode_f;
  response.response_vector()[i] |= colin::mode_f;
  }

// ACRO_VALIDATING I/O
//cout << "COLINY.cpp\n" << response << endl;
//cout << "Point: " << vec << endl;
//cout << "Fval: " << response.function_value() << endl;
//cout << "Cvals: " << response.constraint_values() << endl;

}

void init_ampl(char* stub, 
	colin::Problem<BasicArray<double>,colin::AppResponse_Utilib>& problem,
	BasicArray<double>& initpt)
{
if (stub == NULL)
   EXCEPTION_MNGR(std::runtime_error, "No stub passed to init_ampl()");
//
// Read *.nl file
//  
ASL*  asl = ASL_alloc(ASL_read_fg);
FILE* nl = jac0dim(stub, (fint) strlen(stub));
initpt.resize(n_var);
initpt << 0.0;
X0 = initpt.data();
fg_read(nl,0);
//
// Setup lower and upper bounds on variables
//
BasicArray<real> vlower(n_var);
BasicArray<real> vupper(n_var);
for (int i=0; i<n_var; i++) {
  if (LUv[2*i] == negInfinity)
     vlower[i] = real::negative_infinity;
  else
     vlower[i] = LUv[2*i];
  if (LUv[2*i+1] == Infinity)
     vupper[i] = real::positive_infinity;
  else
     vupper[i] = LUv[2*i+1];
  }
//
// Setup lower and upper bounds on constraints
//
BasicArray<real> clower(n_con);
BasicArray<real> cupper(n_con);
double* b = LUrhs;
for ( int i = 0; i < n_con; i++, b +=2) {
  if (b[0] == negInfinity)
     clower[i] = real::negative_infinity;
  else
     clower[i] = b[0];
  if (b[1] == Infinity)
     cupper[i] = real::positive_infinity;
  else
     cupper[i] = b[1];
  }
ucout << "\nNumber of objectives: " << n_obj << endl;
ucout << "Number of variables: " << n_var << endl;
ucout << "\nVariable\tlower bound\tupper bound\n";
for (int i=0; i< n_var; i++){
   ucout << "  " << i << "\t\t" << vlower[i] << "\t" << vupper[i] << "\n" << Flush;
  }
ucout << "\nConstraint\tlower bound\tupper bound\n";
for (int i=0; i< n_con; i++){
   ucout << "  " << i << "\t\t" << clower[i] << "\t" << cupper[i] << "\n" << Flush;
  }
ucout << endl;
//
// Setup problem
//
colin::OptSetup(problem,&test_function,vlower,vupper,clower,cupper,n_obj);
///
/// Setup initpt
///
initpt.resize(vlower.size());
for (unsigned int i=0; i<initpt.size(); i++)
  initpt[i] = X0[i];
//
// TODO setup linear constraints
//
}


void process_options(CharString& envstr, char* s, ParameterList& plist,
							CharString& solverstr)
{
   ucout << "Processing AMPL Options: " << envstr << endl;
   //// Eliminate white space before and after '=' characters
   int slen = strlen(s);
   int offset=0;
   for (int i=0; i<slen; i++) {
     if (s[i] == '=') {
        int j = i-1;
        while ((j >= 0) && (s[j] == ' ')) { offset++; j--; }
        s[i-offset] = s[i];
        i++;
        while ((i<slen) && (s[i] == ' ')) { offset++; i++; }
        s[i-offset] = s[i];
        }
     else {
        s[i-offset] = s[i];
        }
     }
   s[slen-offset] = '\000';
   //// Process the options
   istringstream isstr(s);
   string str;
   while (isstr >> str) {
     const char* param = str.c_str();
     char* tmp = const_cast<char*>(param);
     unsigned int i=0;
     while ((i<str.size()) && (*tmp != '=')) {tmp++; i++;}
     if (i == str.size()) {
        ucout << "  OPTION: " << param << " true" << endl;
	plist.add_parameter(str,"true",false);
        }
     else {
	*tmp = '\000';
	tmp++;
        if (strcmp(param,"solver") == 0) {
           solverstr = tmp;
           }
        ucout << "  OPTION: " << param << " " << tmp << endl;
	plist.add_parameter(param,tmp,false);
        }
     }
   ucout << endl;
}



int main(int argc, char* argv[])
{
InitializeTiming();
double start_time = CPUSeconds();

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
   ucout << "scolib <*.nl> -AMPL" << endl;
   CommonIO::end();
   exit(1);
   }

try {
//utilib::exception_mngr::set_exit_function(&exit_fn);

//
// Set up the problem and initialize AMPL
//
string fname = argv[1];
colin::Problem<BasicArray<double>,colin::AppResponse_Utilib > problem;
BasicArray<double> initpt;
init_ampl(const_cast<char*>(fname.c_str()), problem, initpt);
//
// Process solver options
//
getstub(&argv,&Oinfo);
//getopts(argv,&Oinfo);
ParameterList plist;
BasicArray<ParameterList> plist_subsolvers;
CharString envstr = "scolib_options";
char* s = getenv(envstr.data());
if (!s) {
   envstr = "COLINY_OPTIONS";
   s = getenv(envstr.data());
   }
if (s) {
   CharString solverstr;
   process_options(envstr,s,plist,solverstr);
   if (solverstr != "") {
      CharString dummy;
      BasicArray<CharString> strarray = split(solverstr,'-');
      strarray[0] += "_options";
      s = getenv(strarray[0].data());
      if (s) process_options(strarray[0], s, plist, dummy);
      if (strarray.size() > 1) {
        plist_subsolvers.resize(strarray.size()-1);
        for (size_t i=1; i<strarray.size(); i++) {
          strarray[i] += "_options";
          s = getenv(strarray[i].data());
          if (!s) {
             strarray[i] = upper_case(strarray[i]);
             s = getenv( strarray[i].data() );
             }
          if (s) process_options(strarray[i], s, plist_subsolvers[i-1], dummy);
          }
        }
      }
   }
//
//
//
bool   use_abort=false;
bool   verbose=false;
bool   debug_solver_params=false;
string solver_str="unknown";
int    repetitions=0;

utilib::ParameterSet params;
params.create_parameter("verbose",verbose,
	"<bool>","false","TODO");
params.create_parameter("debug-solver-params",debug_solver_params,
	"<bool>","false",
	"If true, print solver parameters");
params.create_parameter("solver",solver_str,
	"<string>","ps",
	"The name of the Coliny solver");
params.create_parameter("repetitions",repetitions,
	"<int>","0",
	"The number of times that the solver is run (to check that solver reset() methods work).");
/*
params.create_parameter("param-file",paramfile,
	"<string>","",
	"The name of the file of parameter values");
*/
params.create_parameter("use-abort",use_abort,
	"<bool>","false","If true, then force an abort when an error occurs.");

params.set_parameters(plist,false);
string paramfile;
params.get_parameter<string>("param-file",paramfile);
if (use_abort)
    utilib::exception_mngr::set_mode(utilib::exception_mngr::Abort);
if (solver_str == "unknown") {
   EXCEPTION_MNGR(runtime_error, "ERROR: option 'solver' not specified");
   CommonIO::end();
   exit(1);
   }

CharString tmp;
tmp += "cp ";
tmp += fname.c_str();
tmp += ".nl dummy.nl";
system(tmp.data());


//
// Setup the solver
// Other names for solvers: direct, ms, ms.ps, ms.sw, sw, eareal, apps
//
colin::OptSolverHandle<BasicArray<double>,colin::AppResponse_Utilib> solver = scolib::Factory<BasicArray<double>,colin::AppResponse_Utilib>(solver_str.c_str());

if (paramfile != "") {
   ifstream ifstr;
   ifstr.open(paramfile.c_str());
   if (ifstr)
      solver->read_parameter_values(ifstr);
   }
solver->set_parameters(plist);
solver->set_subsolver_parameters(plist_subsolvers);

if (params.get_parameter<bool>("help")) {
   //solver->initialize("help");
   solver->help_parameters(ucout);
   CommonIO::end();
   exit(1);
   }
//
// Minimize and generate summary statistics about the optimizer
//
BasicArray<double> best_point;
real best_value;
if (initpt.size() > 0)
   colin::solve(solver, problem, initpt, -1, verbose, debug_solver_params, best_point, best_value);
else
   colin::solve(solver, problem, -1, verbose, debug_solver_params, best_point, best_value);
ucout << "Total-Time: " << (CPUSeconds()- start_time) << endl;
ucout << Flush;

for (int i=0; i<repetitions; i++) {
  real tmp_best_value;
  BasicArray<double> tmp_best_point;
  problem.reset_neval();
  if (initpt.size() > 0)
     colin::solve(solver, problem, initpt, -1, verbose, debug_solver_params, tmp_best_point, tmp_best_value);
  else
     colin::solve(solver, problem, -1, verbose, debug_solver_params, tmp_best_point, tmp_best_value);
  if (fabs(tmp_best_value - best_value) > 1e-6) {
     ucout << "Repetition-Check: Fail" << endl;
     EXCEPTION_MNGR(std::runtime_error, "Value in repetition " << i << " is different: orig=" << best_value << " rep=" << tmp_best_value);
     }
  for (unsigned int i=0; i<best_point.size(); i++) {
    if (std::fabs(tmp_best_point[i] - best_point[i]) > 1e-6) {
       ucout << "Repetition-Check: Fail" << endl;
       EXCEPTION_MNGR(std::runtime_error, "Point in repetition " << i << " is different: orig=" << best_point << " rep=" << tmp_best_point);
       }
    }
  }
if (repetitions > 0)
   ucout << "Repetition-Check: Pass" << endl;
CharString msg;
msg += "Coliny Solver: ";
msg += solver_str;
msg += "  final f = ";
msg += best_value;
need_nl = 0;
write_sol(msg.data(), best_point.data(), NULL, NULL);
}

catch (std::exception& err) {
  cerr << "Caught STL exception: " << err.what() << endl;
  }

catch (const char* err) {
  cerr << "Caught string exception: " << err << endl;
  }

ucout << Flush;
CommonIO::end();

#if defined(ACRO_HAVE_MPI)
uMPI::done();
#endif
return 0;
}


#else

int main()
{
	std::cout << "Doing nothing!" << std::endl; 
	return -1;
}

#endif

#endif
