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
// OPTpp.cpp
//
#include <limits>

#include <acro_config.h>
#ifdef ACRO_USING_OPTPP

#include <OptNewton.h>
#include <OptBCNewton.h>
#include <OptBCFDNewton.h>
#include <OptBCQNewton.h>
#include <OptQNewton.h>
#include <OptFDNewton.h>
#include <OptNIPS.h>
#include <OptQNIPS.h>
#include <OptFDNIPS.h>
#include <OptCG.h>
#include <OptLBFGS.h>
#include <OptPDS.h>
#include <NLP1.h>
#include <colin/SolverMngr.h>
#include <interfaces/OPTpp.h>

using namespace std;
using namespace utilib;

#if 0 // disable OPT++ interface

namespace interfaces {

namespace {

int lexical_cast_from_teuchos(const Any& from, Any& to)
{
   const TeuchosVector& data = from.expose<TeuchosVector>();
   vector<double>& ans = to.set<vector<double> >();
   ans.reserve(data.length());
   for (size_t i=0; i<data.length(); i++)
     ans.push_back(data(i));
   return OK;
}

int lexical_cast_to_teuchos(const Any& from, Any& to)
{
   const vector<double>& data = from.expose<vector<double> >();
   TeuchosVector& ans = to.set<TeuchosVector>();
   ans.sizeUninitialized(data.size());
   for (size_t i=0; i<data.size(); i++)
     ans(i) = data[i];
   return OK;
}

bool optpp_lexical_casts()
{
   utilib::TypeManager()->register_lexical_cast
      (typeid(TeuchosVector),
       typeid(std::vector<double>),
       &lexical_cast_from_teuchos);
   utilib::TypeManager()->register_lexical_cast
      (typeid(std::vector<double>),
       typeid(TeuchosVector),
       &lexical_cast_to_teuchos);
   return true;
}

bool dummy1 = optpp_lexical_casts();

}


class OptppNLP0::NLF0 : public OPTPP::NLF0
{
public:

    ///
    NLF0() : OPTPP::NLF0() {}

    ///
    virtual ~NLF0() {}

    ///
    void initFcn();

    ///
    colin::Problem<colin::UNLP0_problem> problem;

    ///
    TeuchosVector initial_point;

    ///
    OPTPP::real evalF();

    ///
    double evalF(const TeuchosVector& x);

    ///
    TeuchosVector evalG()
        { return OPTPP::NLF0::evalG(); }

    ///
    TeuchosVector evalG(const TeuchosVector& x);

    ///
    void my_fcn_v(const TeuchosVector& x, double& fvalue, int& result)
        {
        result=OPTPP::NLPFunction;
        colin::AppRequest req = problem->set_domain(x);
        problem->Request_F(req, fvalue);
        colin::AppResponse r1 = problem->eval_mngr().perform_evaluation(req);
        }
};


void OptppNLP0::NLF0::initFcn() // Initialize Function
{
  if (init_flag == false)  {
    utilib::TypeManager()->lexical_cast(initial_point, mem_xc);
    init_flag = true;
  }
  else  {
    cerr << "NLF0:initFcn: Warning - initialization called twice\n";
    utilib::TypeManager()->lexical_cast(initial_point, mem_xc);
  }
}

double OptppNLP0::NLF0::evalF() // Evaluate Function
{
  int result = 0;
  double time0 = get_wall_clock_time();

  if (SpecFlag == OPTPP::NoSpec) {
    if (!application.getF(mem_xc,fvalue)) {
      my_fcn_v(mem_xc, fvalue, result);
      application.update(OPTPP::NLPFunction,dim,mem_xc,fvalue);
      nfevals++;
    }
  }
  else {
    SpecFlag = OPTPP::Spec1;
    (void) evalG();
    SpecFlag = OPTPP::Spec2;
  }

  function_time = get_wall_clock_time() - time0;
  return fvalue;
}

double OptppNLP0::NLF0::evalF(const TeuchosVector& x) // Evaluate Function at x
{
  double fx;
  int result = 0;
  double time0 = get_wall_clock_time();

  if (SpecFlag == OPTPP::NoSpec) {
    if (!application.getF(x,fx)) {
      my_fcn_v(x, fx, result);
      application.update(OPTPP::NLPFunction,dim,x,fx);
      nfevals++;
    }
  }
  else {
    SpecFlag = OPTPP::Spec1;
    (void) evalG(x);
    fx = specF;
    SpecFlag = OPTPP::Spec2;
  }

  function_time = get_wall_clock_time() - time0;
  return fx;
}

TeuchosVector OptppNLP0::NLF0::evalG(const TeuchosVector& x)
{
  TeuchosVector gx(dim);
  TeuchosVector sx(dim);
  sx = 1.0;

  // Since NLF0 objects do not have analytic gradients supply
  // one by using finite differences

  if (SpecFlag == OPTPP::NoSpec) {
    int result = 0;
    if (!application.getF(x, specF)) {
      my_fcn_v(x, specF, result);
      nfevals++;
    }
  }

  gx = FDGrad(sx, x, specF, partial_grad);
  return gx;
}


class OptppNLP1::NLF1 : public OPTPP::NLF1
{
public:

    ///
    NLF1() : OPTPP::NLF1() {}

    ///
    virtual ~NLF1() {}

    ///
    void initFcn();

    ///
    colin::Problem<colin::UNLP1_problem> problem;

    ///
    TeuchosVector initial_point;

    ///
    void eval();

    ///
    OPTPP::real evalF();

    ///
    double evalF(const TeuchosVector& x);

    ///
    TeuchosVector evalG();

    ///
    TeuchosVector evalG(const TeuchosVector& x);

    ///
    void my_fcn_v(int request, const TeuchosVector& x, double& fvalue, const TeuchosVector& grad, int& result)
        {
        result=request;
        colin::AppRequest req = problem->set_domain(x);
        if (request | OPTPP::NLPFunction)
            problem->Request_F(req, fvalue);
        if (request | OPTPP::NLPGradient)
            problem->Request_G(req, grad);
        colin::AppResponse r1 = problem->eval_mngr().perform_evaluation(req);
        }
};

void OptppNLP1::NLF1::initFcn() // Initialize Function
{
  if (init_flag == false)  {
    utilib::TypeManager()->lexical_cast(initial_point, mem_xc);
    init_flag = true;
  }
  else  {
    cerr << "NLF1:initFcn: Warning - initialization called twice\n";
    utilib::TypeManager()->lexical_cast(initial_point, mem_xc);
  }
}

double OptppNLP1::NLF1::evalF() // Evaluate Function
{
  int result = 0;
  TeuchosVector gtmp(dim);

  double time0 = get_wall_clock_time();
  // *** CHANGE *** //
  if (!application.getF(mem_xc,fvalue)) {
    my_fcn_v(OPTPP::NLPFunction, mem_xc, fvalue, gtmp, result);
    application.update(result,dim,mem_xc,fvalue,gtmp);
    nfevals++;
  }
  // *** CHANGE *** //
  function_time = get_wall_clock_time() - time0;

  if (debug_)
  cout << "NLF1::evalF()\n"
    << "nfevals       = " << nfevals << "\n"
    << "fvalue        = " << fvalue << "\n"
    << "function time = " << function_time << "\n";
  return fvalue;
}

double OptppNLP1::NLF1::evalF(const TeuchosVector& x) // Evaluate Function at x
{
  int    result = 0;
  double fx;
  TeuchosVector gtmp(dim);

  double time0 = get_wall_clock_time();
  // *** CHANGE *** //
  if (!application.getF(x,fx)) {
    my_fcn_v(OPTPP::NLPFunction, x, fx, gtmp, result);
    application.update(result,dim,x,fx,gtmp);
    nfevals++;
  }
  // *** CHANGE *** //
  function_time = get_wall_clock_time() - time0;

  if (debug_)
  cout << "NLF1::evalF(x)\n"
    << "nfevals       = " << nfevals << "\n"
    << "fvalue        = " << fx << "\n"
    << "function time = " << function_time << "\n";
  return fx;
}

TeuchosVector OptppNLP1::NLF1::evalG() // Evaluate the gradient
{
  int    result = 0;
  double fx;

  // *** CHANGE *** //
  if (!application.getGrad(mem_xc,mem_grad)) {
    my_fcn_v(OPTPP::NLPGradient, mem_xc, fx, mem_grad, result);
    application.update(result,dim,mem_xc,fx,mem_grad);
    ngevals++;
  }
  // *** CHANGE *** //
  return mem_grad;
}

TeuchosVector OptppNLP1::NLF1::evalG(const TeuchosVector& x) // Evaluate the gradient at x
{
  int    result = 0 ;
  double fx;
  TeuchosVector gx(dim);

  // *** CHANGE *** //
  if (!application.getGrad(x,gx)) {
    my_fcn_v(OPTPP::NLPGradient, x, fx, gx, result);
    application.update(result,dim,x,fx,gx);
    ngevals++;
  }
  // *** CHANGE *** //
  return gx;
}

void OptppNLP1::NLF1::eval() // Evaluate Function and Gradient
{
  int mode = OPTPP::NLPFunction | OPTPP::NLPGradient, result = 0;

  double time0 = get_wall_clock_time();
  // *** CHANGE *** //
  if (!application.getF(mem_xc,fvalue) || !application.getGrad(mem_xc,mem_grad)) {
    my_fcn_v(mode, mem_xc, fvalue, mem_grad, result);
    application.update(result,dim,mem_xc,fvalue,mem_grad);
    nfevals++; ngevals++;
  }
  // *** CHANGE *** //

  function_time = get_wall_clock_time() - time0;

  if (debug_)
  cout << "NLF1::eval()\n"
    << "mode          = " << mode   << "\n"
    << "nfevals       = " << nfevals << "\n"
    << "fvalue        = " << fvalue << "\n"
    << "function time = " << function_time << "\n";
}


class OptppNLP2::NLF2 : public OPTPP::NLF2
{
public:

    ///
    NLF2() : OPTPP::NLF2() {}

    ///
    virtual ~NLF2() {}

    ///
    void initFcn();

    ///
    colin::Problem<colin::UNLP2_problem> problem;

    ///
    TeuchosVector initial_point;

    ///
    void eval();

    ///
    OPTPP::real evalF();

    ///
    double evalF(const TeuchosVector& x);

    ///
    TeuchosVector evalG();

    ///
    TeuchosVector evalG(const TeuchosVector& x);

    ///
    TeuchosSymMatrix evalH();

    ///
    TeuchosSymMatrix evalH(TeuchosVector& x);

    ///
    void my_fcn_v(int request, const TeuchosVector& x, double& fvalue, const TeuchosVector& grad, TeuchosSymMatrix& Hessian, int& result)
        {
        result=request;
        colin::AppRequest req = problem->set_domain(x);
        if (request | OPTPP::NLPFunction)
            problem->Request_F(req, fvalue);
        if (request | OPTPP::NLPGradient)
            problem->Request_G(req, grad);
	// BMA: this appears a typo; also no converters exist for the Hessian
        if (request | OPTPP::NLPHessian)
            problem->Request_H(req, grad);
        colin::AppResponse r2 = problem->eval_mngr().perform_evaluation(req);
        }
};

void OptppNLP2::NLF2::initFcn() // Initialize Function
{
  if (init_flag == false)  {
    utilib::TypeManager()->lexical_cast(initial_point, mem_xc);
    init_flag = true;
  }
  else  {
    cerr << "NLF2:initFcn: Warning - initialization called twice\n";
    utilib::TypeManager()->lexical_cast(initial_point, mem_xc);
  }
}

double OptppNLP2::NLF2::evalF() // Evaluate Function
{
  int  result = 0;
  TeuchosVector gtmp(dim);
  TeuchosSymMatrix Htmp(dim);
  //cout << "NLF2:evalF \n";

  double time0 = get_wall_clock_time();
  // *** CHANGE *** //
  if (!application.getF(mem_xc,fvalue)) {
    my_fcn_v(OPTPP::NLPFunction, mem_xc, fvalue, gtmp, Htmp,result);
    application.update(result,dim,mem_xc,fvalue,gtmp,Htmp);
    nfevals++;
  }
  // *** CHANGE *** //
  function_time = get_wall_clock_time() - time0;

  if (debug_)  cout << "NLF2::evalF()\n"
                   << "nfevals       = " << nfevals   << "\n"
                   << "fvalue        = " << fvalue << "\n"
                   << "function time = " << function_time << "\n";
  return fvalue;
}

double OptppNLP2::NLF2::evalF(const TeuchosVector& x) // Evaluate Function at x
{
  int    result = 0;
  double fx;
  TeuchosVector gtmp(dim);
  TeuchosSymMatrix Htmp(dim);

  double time0 = get_wall_clock_time();
  // *** CHANGE *** //
  if (!application.getF(x,fx)) {
    my_fcn_v(OPTPP::NLPFunction, x, fx, gtmp, Htmp,result);
    application.update(result,dim,x,fx,gtmp,Htmp);
    nfevals++;
  }
  // *** CHANGE *** //
  function_time = get_wall_clock_time() - time0;

  if (debug_) cout << "NLF2::evalF(x)\n"
                  << "nfevals       = " << nfevals   << "\n"
                  << "fvalue        = " << fvalue << "\n"
                  << "function time = " << function_time << "\n";
  return fx;
}

TeuchosVector OptppNLP2::NLF2::evalG() // Evaluate the gradient
{
  int    result = 0;
  double fx;
  TeuchosSymMatrix Htmp(dim);

  // *** CHANGE *** //
  if (!application.getGrad(mem_xc,mem_grad)) {
    my_fcn_v(OPTPP::NLPGradient, mem_xc, fx, mem_grad, Htmp,result);
    application.update(result,dim,mem_xc,fx,mem_grad,Htmp);
    ngevals++;
  }
  // *** CHANGE *** //
  return mem_grad;
}

TeuchosVector OptppNLP2::NLF2::evalG(const TeuchosVector& x) // Evaluate the gradient at x
{
  int    result = 0;
  double fx;
  TeuchosVector gx(dim);
  TeuchosSymMatrix Htmp(dim);

  // *** CHANGE *** //
  if (!application.getGrad(x,gx)) {
    my_fcn_v(OPTPP::NLPGradient, x, fx, gx, Htmp, result);
    application.update(result,dim,x,fx,gx,Htmp);
    ngevals++;
  }
  // *** CHANGE *** //
  return gx;
}

TeuchosSymMatrix OptppNLP2::NLF2::evalH() // Evaluate the Hessian
{
  int    result = 0;
  double fx;
  TeuchosVector gtmp(dim);

  // *** CHANGE *** //
  if (!application.getHess(mem_xc,Hessian)) {
    my_fcn_v(OPTPP::NLPHessian, mem_xc, fx, gtmp, Hessian, result);
    application.update(result,dim,mem_xc,fx,gtmp,Hessian);
    nhevals++;
  }
  // *** CHANGE *** //
  return Hessian;
}

TeuchosSymMatrix OptppNLP2::NLF2::evalH(TeuchosVector& x) // Evaluate the hessian at x
{
  int    result = 0;
  double fx;
  TeuchosVector gx(dim);
  TeuchosSymMatrix Hx(dim);

  // *** CHANGE *** //
  if (!application.getHess(x,Hx)) {
    my_fcn_v(OPTPP::NLPHessian, x, fx, gx, Hx, result);
    application.update(result,dim,x,fx,gx,Hx);
    nhevals++;
  }
  // *** CHANGE *** //
  return Hx;
}

void OptppNLP2::NLF2::eval() // Evaluate Function and Gradient
{
  int mode = OPTPP::NLPFunction | OPTPP::NLPGradient | OPTPP::NLPHessian, result = 0;

  double time0 = get_wall_clock_time();
  // *** CHANGE *** //
  if (!application.getF(mem_xc,fvalue) || !application.getGrad(mem_xc,mem_grad) ||
      !application.getHess(mem_xc,Hessian)) {
    my_fcn_v(mode, mem_xc, fvalue, mem_grad, Hessian,result);
    application.update(result,dim,mem_xc,fvalue,mem_grad,Hessian);
    nfevals++; ngevals++; nhevals++;
  }
  // *** CHANGE *** //
  function_time = get_wall_clock_time() - time0;

  if (debug_)  cout << "NLF2::eval()\n"
                   << "mode          = " << mode   << "\n"
                   << "nfevals       = " << nfevals   << "\n"
               << "fvalue        = " << fvalue << "\n"
               << "function time = " << function_time << "\n";
}




OptppNLP0::OptppNLP0()
   : nlf(NULL)
{ 
   reset_signal.connect( boost::bind( &OptppNLP0::reset_OptppNLP0, this ) ); 
}

OptppNLP0::~OptppNLP0()
{ if (nlf) delete nlf; }


void OptppNLP0::reset_OptppNLP0()
{
if (problem.empty()) return;
if (nlf) delete nlf;
nlf = new NLF0;
nlf->problem=problem;
}


OptppNLP1::OptppNLP1()
   : nlf(NULL)
{
   reset_signal.connect( boost::bind( &OptppNLP1::reset_OptppNLP1, this ) ); 
}

OptppNLP1::~OptppNLP1()
{ if (nlf) delete nlf; }

void OptppNLP1::reset_OptppNLP1()
{
if (problem.empty()) return;
if (nlf) delete nlf;
nlf = new NLF1;
nlf->problem=problem;
}


OptppNLP2::OptppNLP2()
   : nlf(NULL)
{
   reset_signal.connect( boost::bind( &OptppNLP2::reset_OptppNLP2, this ) ); 
}

OptppNLP2::~OptppNLP2()
{ if (nlf) delete nlf; }

void OptppNLP2::reset_OptppNLP2()
{
if (problem.empty()) return;
if (nlf) delete nlf;
nlf = new NLF2;
nlf->problem=problem;
}


//-----------------------------------------------------------------------------
//  Newton1
//-----------------------------------------------------------------------------

OptppNewton1::OptppNewton1()
   : NLFOptions(properties), 
     GenericOptions(properties), 
     NewtonOptions(properties)
{
   //reset_signal.connect( boost::bind( &OptppNewton1::reset_OptppNewton1, this ) );

   using_fd_hessian=false;
   properties.declare
      ( "fd-hessian", 
        "If true, then use a Quasi-Newton method.",
        utilib::Privileged_Property(using_fd_hessian) );
   //option.add("fd-hessian",using_fd_hessian,
   //           "If true, then use a Quasi-Newton method.");
}


void OptppNewton1::optimize()
{
int n=problem->num_real_vars;
if (n == 0) {
   solver_status.termination_info = "No-Real-Params";
   solver_status.model_status = colin::model_no_solution_returned;
   solver_status.termination_condition = colin::termination_invalidProblem;
   return;
   }
//
// Get the initial point and do error checking
//
if (initial_points->size( problem.application() ) == 0) {
   solver_status.termination_info = "No-Initial-Point";
   solver_status.model_status = colin::model_no_solution_returned;
   solver_status.termination_condition = colin::termination_invalidProblem;
   return;
   }
if ( initial_points->size( problem.application() ) > 1 )
   solver_status.termination_info = "Too many initial points";
initial_points.get_point(problem, nlf->initial_point);
if ( problem->num_real_vars != nlf->initial_point.length() )
   EXCEPTION_MNGR(std::runtime_error, "User-provided initial point has length " << nlf->initial_point.length() << " but the problem size is " << problem->num_real_vars << std::endl);
//
// Run OPT++
//
OPTPP::OptimizeClass* opt;
if ( problem->enforcing_domain_bounds ) {
    if (using_fd_hessian) {
        OPTPP::OptBCFDNewton* tmp = new OPTPP::OptBCFDNewton(nlf,0);
        NLFOptions::set_options(tmp, nlf, problem);
        GenericOptions::set_options(tmp, nlf, problem);
        NewtonOptions::set_options(tmp, nlf, problem);
        opt=tmp;
        }
    else {
        OPTPP::OptBCQNewton* tmp = new OPTPP::OptBCQNewton(nlf,0);
        NLFOptions::set_options(tmp, nlf, problem);
        GenericOptions::set_options(tmp, nlf, problem);
        NewtonOptions::set_options(tmp, nlf, problem);
        opt=tmp;
        }
    }
else {
    if (using_fd_hessian) {
        OPTPP::OptFDNewton* tmp = new OPTPP::OptFDNewton(nlf,0);
        NLFOptions::set_options(tmp, nlf, problem);
        GenericOptions::set_options(tmp, nlf, problem);
        NewtonOptions::set_options(tmp, nlf, problem);
        opt=tmp;
        }
    else {
        OPTPP::OptQNewton* tmp = new OPTPP::OptQNewton(nlf,0);
        NLFOptions::set_options(tmp, nlf, problem);
        GenericOptions::set_options(tmp, nlf, problem);
        NewtonOptions::set_options(tmp, nlf, problem);
        opt=tmp;
        }
    }
opt->optimize();
solver_status.termination_info = opt->getMesg();
solver_status.model_status = colin::model_locally_optimal;
solver_status.termination_condition = colin::termination_locallyOptimal;
// careful; this might be a view; delete below suspicious
TeuchosVector final_point = nlf->getXc();
delete opt;

final_points.add_point(problem, final_point);
}


//-----------------------------------------------------------------------------
//  Newton2
//-----------------------------------------------------------------------------

OptppNewton2::OptppNewton2()
   : NLFOptions(properties), 
     GenericOptions(properties), 
     NewtonOptions(properties)
{
   //reset_signal.connect( boost::bind( &OptppNewton2::reset_OptppNewton2, this ) );
}


void OptppNewton2::optimize()
{
int n=problem->num_real_vars;
if (n == 0) {
   solver_status.termination_info = "No-Real-Params";
   solver_status.model_status = colin::model_no_solution_returned;
   solver_status.termination_condition = colin::termination_invalidProblem;
   return;
   }
//
// Get the initial point and do error checking
//
if (initial_points->size( problem.application() ) == 0) {
   solver_status.termination_info = "No-Initial-Point";
   solver_status.model_status = colin::model_no_solution_returned;
   solver_status.termination_condition = colin::termination_invalidProblem;
   return;
   }
if ( initial_points->size( problem.application() ) > 1 )
   solver_status.termination_info = "Too many initial points";
initial_points.get_point(problem, nlf->initial_point);
if ( problem->num_real_vars != nlf->initial_point.length() )
   EXCEPTION_MNGR(std::runtime_error, "User-provided initial point has length " << nlf->initial_point.length() << " but the problem size is " << problem->num_real_vars << std::endl);
//
// Run OPT++
//
OPTPP::OptimizeClass* opt;
if ( problem->enforcing_domain_bounds ) {
        OPTPP::OptBCNewton* tmp = new OPTPP::OptBCNewton(nlf,0);
        NLFOptions::set_options(tmp, nlf, problem);
        GenericOptions::set_options(tmp, nlf, problem);
        NewtonOptions::set_options(tmp, nlf, problem);
        opt=tmp;
    }
else {
        OPTPP::OptNewton* tmp = new OPTPP::OptNewton(nlf,0);
        NLFOptions::set_options(tmp, nlf, problem);
        GenericOptions::set_options(tmp, nlf, problem);
        NewtonOptions::set_options(tmp, nlf, problem);
        opt=tmp;
    }
opt->optimize();
solver_status.termination_info = opt->getMesg();
solver_status.model_status = colin::model_locally_optimal;
solver_status.termination_condition = colin::termination_locallyOptimal;
// careful; this might be a view; delete below suspicious
TeuchosVector final_point = nlf->getXc();
delete opt;

final_points.add_point(problem, final_point);
}


//-----------------------------------------------------------------------------
//  NIPS1
//-----------------------------------------------------------------------------


OptppNIPS1::OptppNIPS1()
   : NLFOptions(properties), GenericOptions(properties)
{
   //reset_signal.connect( boost::bind( &OptppNIPS1::reset_OptppNIPS1, this ) );

   using_fd_hessian=false;
   properties.declare
      ( "fd-hessian", 
        "If true, then use a Quasi-Newton method.",
        utilib::Privileged_Property(using_fd_hessian) );
   //option.add("fd-hessian",using_fd_hessian,
   //           "If true, then use a Quasi-Newton method.");

   MeritFcn="ArgaezTapia";
   properties.declare
      ( "merit-fn", 
        "Set the merit function: {NormFmu, ArgaezTapia, VanShanno}",
        utilib::Privileged_Property(MeritFcn) );
   //option.add("merit-fn",MeritFcn,
   //           "Set the merit function: NormFmu, ArgaezTapia, VanShanno");

   sigmin=-1.0;
   properties.declare
      ( "centering", 
        "Set centering parameter.  The default, -1, means 'unset'",
        utilib::Privileged_Property(sigmin) );
   //option.add("centering",sigmin,
   //           "Set centering parameter.  The default, -1, means 'unset'");

   taumin=-1.0;
   properties.declare
      ( "boundary-step", 
        "Set percentage step toward boundary.  The default, -1, means 'unset'",
        utilib::Privileged_Property(taumin) );
   //option.add("boundary-step",taumin,
   //           "Set percentage step toward boundary.  The default, -1, means 'unset'");
}


void OptppNIPS1::optimize()
{
int n=problem->num_real_vars;
if (n == 0) {
   solver_status.termination_info = "No-Real-Params";
   solver_status.model_status = colin::model_no_solution_returned;
   solver_status.termination_condition = colin::termination_invalidProblem;
   return;
   }
//
// Get the initial point and do error checking
//
if (initial_points->size( problem.application() ) == 0) {
   solver_status.termination_info = "No-Initial-Point";
   solver_status.model_status = colin::model_no_solution_returned;
   solver_status.termination_condition = colin::termination_invalidProblem;
   return;
   }
if ( initial_points->size( problem.application() ) > 1 )
   solver_status.termination_info = "Too many initial points";
initial_points.get_point(problem, nlf->initial_point);
if ( problem->num_real_vars != nlf->initial_point.length() )
   EXCEPTION_MNGR(std::runtime_error, "User-provided initial point has length " << nlf->initial_point.length() << " but the problem size is " << problem->num_real_vars << std::endl);
//
// Run OPT++
//
OPTPP::OptimizeClass* opt;
if (using_fd_hessian) {
    OPTPP::OptFDNIPS* tmp = new OPTPP::OptFDNIPS(nlf,0);
    NLFOptions::set_options(tmp, nlf, problem);
    GenericOptions::set_options(tmp, nlf, problem);
    set_options(tmp);
    opt=tmp;
    }
else {
    OPTPP::OptQNIPS* tmp = new OPTPP::OptQNIPS(nlf,0);
    NLFOptions::set_options(tmp, nlf, problem);
    GenericOptions::set_options(tmp, nlf, problem);
    set_options(tmp);
    opt=tmp;
    }
opt->optimize();
solver_status.termination_info = opt->getMesg();
//
// todo: how check that this is feasible?
//
solver_status.model_status = colin::model_locally_optimal;
solver_status.termination_condition = colin::termination_locallyOptimal;
// careful; this might be a view; delete below suspicious
TeuchosVector final_point = nlf->getXc();
delete opt;

final_points.add_point(problem, final_point);
}


//-----------------------------------------------------------------------------
//  NIPS2
//-----------------------------------------------------------------------------


OptppNIPS2::OptppNIPS2()
   : NLFOptions(properties), GenericOptions(properties)
{
   //reset_signal.connect( boost::bind( &OptppNIPS2::reset_OptppNIPS2, this ) );

   MeritFcn="ArgaezTapia";
   properties.declare
      ( "merit-fn", 
        "Set the merit function: {NormFmu, ArgaezTapia, VanShanno}",
        utilib::Privileged_Property(MeritFcn) );
   //option.add("merit-fn",MeritFcn,
   //           "Set the merit function: NormFmu, ArgaezTapia, VanShanno");

   sigmin=-1.0;
   properties.declare
      ( "centering", 
        "Set centering parameter.  The default, -1, means 'unset'",
        utilib::Privileged_Property(sigmin) );
   //option.add("centering",sigmin,
   //           "Set centering parameter.  The default, -1, means 'unset'");

   taumin=-1.0;
   properties.declare
      ( "boundary-step", 
        "Set percentage step toward boundary.  The default, -1, means 'unset'",
        utilib::Privileged_Property(taumin) );
   //option.add("boundary-step",taumin,
   //           "Set percentage step toward boundary.  The default, -1, means 'unset'");
}


void OptppNIPS2::optimize()
{
int n=problem->num_real_vars;
if (n == 0) {
   solver_status.termination_info = "No-Real-Params";
   solver_status.model_status = colin::model_no_solution_returned;
   solver_status.termination_condition = colin::termination_invalidProblem;
   return;
   }
//
// Get the initial point and do error checking
//
if (initial_points->size( problem.application() ) == 0) {
   solver_status.termination_info = "No-Initial-Point";
   solver_status.model_status = colin::model_no_solution_returned;
   solver_status.termination_condition = colin::termination_invalidProblem;
   return;
   }
if ( initial_points->size( problem.application() ) > 1 )
   solver_status.termination_info = "Too many initial points";
initial_points.get_point(problem, nlf->initial_point);
if ( problem->num_real_vars != nlf->initial_point.length() )
   EXCEPTION_MNGR(std::runtime_error, "User-provided initial point has length " << nlf->initial_point.length() << " but the problem size is " << problem->num_real_vars << std::endl);
//
// Run OPT++
//
OPTPP::OptNIPS* opt = new OPTPP::OptNIPS(nlf,0);
    NLFOptions::set_options(opt, nlf, problem);
    GenericOptions::set_options(opt, nlf, problem);
    set_options(opt);
opt->optimize();
solver_status.termination_info = opt->getMesg();
solver_status.model_status = colin::model_locally_optimal;
solver_status.termination_condition = colin::termination_locallyOptimal;
// careful; this might be a view; delete below suspicious
TeuchosVector final_point = nlf->getXc();
delete opt;

final_points.add_point(problem, final_point);
}


//-----------------------------------------------------------------------------
//  LBFGS
//-----------------------------------------------------------------------------


OptppLBFGS::OptppLBFGS()
   : NLFOptions(properties), GenericOptions(properties)
{
   //reset_signal.connect( boost::bind( &OptppLBFGS::reset_OptppLBFGS, this ) );
}


void OptppLBFGS::optimize()
{
int n=problem->num_real_vars;
if (n == 0) {
   solver_status.termination_info = "No-Real-Params";
   solver_status.model_status = colin::model_no_solution_returned;
   solver_status.termination_condition = colin::termination_invalidProblem;
   return;
   }
//
// Get the initial point and do error checking
//
if (initial_points->size( problem.application() ) == 0) {
   solver_status.termination_info = "No-Initial-Point";
   solver_status.model_status = colin::model_no_solution_returned;
   solver_status.termination_condition = colin::termination_invalidProblem;
   return;
   }
if ( initial_points->size( problem.application() ) > 1 )
   solver_status.termination_info = "Too many initial points";
initial_points.get_point(problem, nlf->initial_point);
if ( problem->num_real_vars != nlf->initial_point.length() )
   EXCEPTION_MNGR(std::runtime_error, "User-provided initial point has length " << nlf->initial_point.length() << " but the problem size is " << problem->num_real_vars << std::endl);
//
// Run OPT++
//
OPTPP::OptLBFGS* opt = new OPTPP::OptLBFGS(nlf,0);
NLFOptions::set_options(opt, nlf, problem);
GenericOptions::set_options(opt, nlf, problem);
opt->optimize();
solver_status.termination_info = opt->getMesg();
solver_status.model_status = colin::model_locally_optimal;
solver_status.termination_condition = colin::termination_locallyOptimal;
// careful; this might be a view; delete below suspicious
TeuchosVector final_point = nlf->getXc();
delete opt;

final_points.add_point(problem, final_point);
}


//-----------------------------------------------------------------------------
//  CG
//-----------------------------------------------------------------------------


OptppCG::OptppCG()
   : NLFOptions(properties), GenericOptions(properties)
{
   //reset_signal.connect( boost::bind( &OptppCG::reset_OptppCG, this ) );
}


void OptppCG::optimize()
{
int n=problem->num_real_vars;
if (n == 0) {
   solver_status.termination_info = "No-Real-Params";
   solver_status.model_status = colin::model_no_solution_returned;
   solver_status.termination_condition = colin::termination_invalidProblem;
   return;
   }
//
// Get the initial point and do error checking
//
if (initial_points->size( problem.application() ) == 0) {
   solver_status.termination_info = "No-Initial-Point";
   solver_status.model_status = colin::model_no_solution_returned;
   solver_status.termination_condition = colin::termination_invalidProblem;
   return;
   }
if ( initial_points->size( problem.application() ) > 1 )
   solver_status.termination_info = "Too many initial points";
initial_points.get_point(problem, nlf->initial_point);
if ( problem->num_real_vars != nlf->initial_point.length() )
   EXCEPTION_MNGR(std::runtime_error, "User-provided initial point has length " << nlf->initial_point.length() << " but the problem size is " << problem->num_real_vars << std::endl);
//
// Run OPT++
//
OPTPP::OptCG* opt = new OPTPP::OptCG(nlf);
NLFOptions::set_options(opt, nlf, problem);
GenericOptions::set_options(opt, nlf, problem);
opt->optimize();
solver_status.termination_info = opt->getMesg();
solver_status.model_status = colin::model_locally_optimal;
solver_status.termination_condition = colin::termination_locallyOptimal;
// careful; this might be a view; delete below suspicious
TeuchosVector final_point = nlf->getXc();
delete opt;

final_points.add_point(problem, final_point);
}


//-----------------------------------------------------------------------------
//  PDS
//-----------------------------------------------------------------------------


OptppPDS::OptppPDS()
   : GenericOptions(properties)
{
   //reset_signal.connect( boost::bind( &OptppPDS::reset_OptppPDS, this ) );

   search_scheme_size=64;
   properties.declare
      ( "scheme-size", 
        "Number of points in pattern to be evaluated at each iteration",
        utilib::Privileged_Property(search_scheme_size) );
   //option.add("scheme-size",search_scheme_size,
   //           "Number of points in pattern to be evaluated at each iteration");

   simplex_type=2;
   properties.declare
      ( "simplex", 
        "Type of simplex (1=right angle, 2=equal sides, 3=scaled right angle, "
        "4=user defined)",
        utilib::Privileged_Property(simplex_type) );
   //option.add("simplex",simplex_type,
   //           "Type of simplex (1=right angle, 2=equal sides, 3=scaled right angle 4=user defined)");

   properties.declare
      ( "scaling", 
        "Set variable scales",
        utilib::Privileged_Property(vscales) );
   //option.add("scaling",vscales,
   //           "Set variable scales");

   // This is disabled until we can figure out how to support stream operators for
   // NEWMAT.
   //option.add("simplex-matrix",simplex_matrix,
   //"Provide user-defined simplex");

   scheme_filename="OptppPDS_pattern.txt";
   properties.declare
      ( "scheme-file", 
        "Name of file to hold pattern points",
        utilib::Privileged_Property(scheme_filename) );
   //option.add("scheme-file",scheme_filename,
   //           "Name of file to hold pattern points");
}


void OptppPDS::optimize()
{
int n=problem->num_real_vars;
if (n == 0) {
   solver_status.termination_info = "No-Real-Params";
   solver_status.model_status = colin::model_no_solution_returned;
   solver_status.termination_condition = colin::termination_invalidProblem;
   return;
   }
//
// Get the initial point and do error checking
//
if (initial_points->size( problem.application() ) == 0) {
   solver_status.termination_info = "No-Initial-Point";
   solver_status.model_status = colin::model_no_solution_returned;
   solver_status.termination_condition = colin::termination_invalidProblem;
   return;
   }
if ( initial_points->size( problem.application() ) > 1 )
   solver_status.termination_info = "Too many initial points";
initial_points.get_point(problem, nlf->initial_point);
if ( problem->num_real_vars != nlf->initial_point.length() )
   EXCEPTION_MNGR(std::runtime_error, "User-provided initial point has length " << nlf->initial_point.length() << " but the problem size is " << problem->num_real_vars << std::endl);
//
// Create and configure optimizer
//
OPTPP::OptPDS* opt = new OPTPP::OptPDS(nlf);
GenericOptions::set_options(opt, nlf, problem);
opt->setSSS(search_scheme_size);
opt->setSimplexType(simplex_type);
TeuchosVector tmp;
utilib::TypeManager()->lexical_cast(vscales,tmp);
opt->setScale(tmp);
// Disabled.  See above.
//opt->setSimplex(simplex_matrix);
opt->setSchemeFileName(const_cast<char*>(scheme_filename.c_str()));
//
// Run OPT++
//
opt->optimize();
solver_status.termination_info = opt->getMesg();
solver_status.model_status = colin::model_intermediate_nonoptimal;
solver_status.termination_condition = colin::termination_other;
// careful; this might be a view; delete below suspicious
TeuchosVector final_point = nlf->getXc();
delete opt;

final_points.add_point(problem, final_point);
}


REGISTER_COLIN_SOLVER(OptppNewton1, "optpp:newton1", "The OPT++ Newton method for UNLP1 problems.")
REGISTER_COLIN_SOLVER(OptppNewton2, "optpp:newton2", "The OPT++ Newton method for UNLP2 problems.")
REGISTER_COLIN_SOLVER(OptppNIPS1, "optpp:nips1", "The OPT++ NIPS method for NLP1 problems.")
REGISTER_COLIN_SOLVER(OptppNIPS2, "optpp:nips2", "The OPT++ NIPS method for NLP2 problems.")
REGISTER_COLIN_SOLVER(OptppLBFGS, "optpp:lbfgs", "The OPT++ LBFGS method.")
REGISTER_COLIN_SOLVER(OptppCG, "optpp:cg", "The OPT++ CG method.")
REGISTER_COLIN_SOLVER(OptppPDS, "optpp:pds", "The OPT++ PDS method.")

}

#else
namespace interfaces {
namespace StaticInitializers {
extern const volatile bool OptppNewton1_bool = false;
}
}
#endif // disable OPT++ interface

#endif
