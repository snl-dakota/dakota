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

/**
 * \file OptSolverUtil.h
 *
 *  This file defines a variety of utility functions that are useful for
 *  running Solvers in a straight-forward manner.  The first set
 *  of functions are templated on the optimizer type.  The ColinySolver
 *  class is used by the remaining functions to allow a user to specify
 *  a solver using predefined names.
 */

#ifndef colin_OptSolverUtil_h
#define colin_OptSolverUtil_h

#include <acro_config.h>
#include <colin/Solver.h>
#include <colin/Utilities.h>
#include <utilib/pvector.h>
#ifdef USING_MPI
#include <colin/MasterSlaveOptExec.h>
#endif

namespace colin
{

//
// Core solver routine that applies a COLIN solver in a generic fashion
//
template <class CommT>
void solve(SolverHandle opt,
           utilib::seed_t seed,
           bool debug_flag,
           bool verbose_flag,
#if defined(ACRO_HAVE_MPI)
           CommT comm)
#else
           CommT)
#endif
{
   try
   {
#if defined(ACRO_HAVE_MPI)
      /// TODO: how use the communicator here?
      int processor_rank = utilib::uMPI::rank;
      int num_processors = utilib::uMPI::size;
#else
      int processor_rank = 0, num_processors = 1;
#endif

      ApplicationHandle app = opt.solver()->get_problem_handle();
      //Problem<ProblemT>& prob = opt.get_problem();
      if ( app.empty() )
         EXCEPTION_MNGR(std::runtime_error, "solve - Optimizer not initialized with a problem.");
      app.application()->reset();

      if (!verbose_flag)
         ColinGlobals::output_level = "quiet";
#if 0
      // WEH - this IO is redundant in contexts like AMPL and XML input files, where we have
      //		already printed out a summary while loading the problem info.
      if (verbose_flag && (processor_rank == 0))
      {
         app.application()->print_summary(ucout);
         ucout << std::endl;
      }
#endif
      if (processor_rank == 0)
      {
         //PM_LCG rng(seed);
         //opt.set_rng(&rng);
         if (debug_flag)
         {
            opt.solver()->debug = 10000000;
            opt.solver()->set_parameter("output_level", "verbose");
         }
         if (seed > 0)
            opt.solver()->set_parameter("seed", seed);
         ucout << utilib::Flush;
         std::cout << std::flush;
         opt.solver()->reset();
         if (verbose_flag)
         {
            ucout << std::endl;
            ucout << "*****************************************************************************" << std::endl;
            ucout << "Options:" << std::endl;
            ucout << "*****************************************************************************" << std::endl;
            opt.solver()->write_parameters(ucout);
            ucout << std::endl;
            ucout << std::endl;
            ucout << "*****************************************************************************" << std::endl;
            ucout << "Option Values:" << std::endl;
            ucout << "*****************************************************************************" << std::endl;
            opt.solver()->write_parameter_values(ucout);
         }
      }
      ucout << utilib::Flush;
      std::cout << std::flush;
      double run_start = CPUSeconds();
      if (num_processors == 1)
         opt.solver()->optimize();
      else
      {
         if (debug_flag)
            ucout << "Starting Parallel Execution" << std::endl;
#ifdef USING_MPI
         colin::MasterSlaveOptExec(opt, prob, 1, false, debug_flag);
#endif
      }
      double run_end = CPUSeconds();
      if (processor_rank == 0)
      {
         ucout << "-----------------------------------------------------------------------" << std::endl;
         ucout << "Solver Statistics" << std::endl;
         ucout << "-----------------------------------------------------------------------" << std::endl;
         opt.solver()->print(ucout);
         if (app.application()->num_objectives == 1)
         {
#ifdef ACRO_VALIDATING
            if (fabs(opt.solver()->opt_response().value() - opt.solver()->opt_response().response.function_value()) > 1e-8)
               EXCEPTION_MNGR(std::runtime_error, "Solver Error: Discrepancy between the OptResponse value and the final value of the OptResponse object");
#endif
            }
         ucout << "Solver-Time: " << (run_end - run_start) << std::endl;
      }
   }
   catch (std::exception& err)
   {
      if (verbose_flag)
      {
         ucout << err.what() << std::endl;
      }
      throw;
   }
}


//
// Core solver routine that applies a COLIN solver in a generic fashion
//
template <class DomainT, class ProblemT, class CommT>
void solve(Solver<DomainT, ProblemT>& opt,
           utilib::seed_t seed,
           bool debug_flag,
           bool verbose_flag,
           CommT comm)
{
   solve(SolverHandle(opt.solver()), seed, debug_flag, verbose_flag, comm);
}

//
// Core solver routine that applies a COLIN solver in a generic fashion
//
template <class DomainT, class ProblemT, class CommT>
void solve(Solver<DomainT, ProblemT>& opt,
           utilib::seed_t seed,
           bool debug_flag,
           bool verbose_flag,
           DomainT& best_point, real& best_value,
           CommT comm)
{
   solve(opt, seed, debug_flag, verbose_flag, comm);
   TypeManager()->lexical_cast(opt.best().point, best_point);
   best_value = opt.best().value();
}


//
// Core solver routine that applies a COLIN solver with an initial point
// in a generic fashion.
//
template <class DomainT, class ProblemT>
void solve(Solver<DomainT, ProblemT>& opt, Problem<ProblemT>& prob, DomainT& point,
           utilib::seed_t seed,
           bool debug_flag,
           bool verbose_flag,
           DomainT& best_point, real& best_value)
{
   prob.init_real_params(point.size());
   opt.set_problem(prob);
   opt.set_initial_point(point);
#if defined(ACRO_HAVE_MPI)
   solve(opt, seed, debug_flag, verbose_flag, best_point, best_value, utilib::uMPI::comm);
#else
   int dummy = 0;
   solve(opt, seed, debug_flag, verbose_flag, best_point, best_value, dummy);
#endif
}


//
// Core solver routine that applies a COLIN solver to optimize globally
// in a generic fashion.
//
template <class DomainT, class ProblemT>
void solve(Solver<DomainT, ProblemT>& opt, Problem<ProblemT>& prob,
           utilib::seed_t seed,
           bool debug_flag,
           bool verbose_flag,
           DomainT& best_point, real& best_value)
{
   opt.set_problem(prob);
#if defined(ACRO_HAVE_MPI)
   solve(opt, prob, seed, debug_flag, verbose_flag, best_point, best_value, utilib::uMPI::comm);
#else
   int dummy = 0;
   solve(opt, prob, seed, debug_flag, verbose_flag, best_point, best_value, dummy);
#endif
}



/*
//
// Core solver routine that applies a COLIN solver with an initial point
// in a generic fashion.
//
template <class ProblemT, class DomainT>
void solve(const char* opt, ProblemT& prob, DomainT& point,
			int seed,
			std::istream& is,
			bool debug_flag,
			bool verbose_flag,
			DomainT& best_point, real& best_value)
{
ColinySolver<ProblemT,DomainT> solver(opt);
solver.read_parameter_values(is);
bool status = solver.minimize(prob,point,seed,debug_flag,verbose_flag,best_point,best_value);
return status;
}


//
// Core solver routine that applies a COLIN solver to optimize globally
// in a generic fashion.
//
template <class ProblemT, class DomainT>
void solve(const char* opt, ProblemT& prob,
			int seed,
			std::istream& is,
			bool debug_flag,
			bool verbose_flag,
			DomainT& best_point, real& best_value)
{
ColinySolver<ProblemT,DomainT> solver(opt);
solver.read_parameter_values(is);
bool status = solver.minimize(prob,seed,debug_flag,verbose_flag,best_point,best_value);
return status;
}


//
// Core solver routine that applies a COLIN solver with an initial point
// in a generic fashion.
//
template <class ProblemT, class DomainT>
void solve(const char* opt, ProblemT& prob, DomainT& point,
			int seed,
			std::istream& is,
			bool debug_flag,
			bool verbose_flag,
			DomainT& opt_point,
			DomainT& best_point, real& best_value)
{
ColinySolver<ProblemT,DomainT> solver;
solver.initialize(opt);
solver.read_parameter_values(is);
bool status = solver.minimize(prob,point,seed,debug_flag,verbose_flag,opt_point,best_point,best_value);
return status;
}


//
// Core solver routine that applies a COLIN solver to optimize globally
// in a generic fashion.
//
template <class ProblemT, class DomainT>
void solve(const char* opt, ProblemT& prob,
			int seed,
			std::istream& is,
			bool debug_flag,
			bool verbose_flag,
			DomainT& opt_point,
			DomainT& best_point, real& best_value)
{
ColinySolver<ProblemT,DomainT> solver;
solver.initialize(opt);
solver.read_parameter_values(is);
bool status = solver.minimize(prob,seed,debug_flag,verbose_flag,opt_point,best_point,best_value);
return status;
}
*/

}

#endif
