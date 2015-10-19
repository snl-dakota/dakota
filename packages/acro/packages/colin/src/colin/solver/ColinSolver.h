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
 * \file ColinSolver.h
 *
 * Defines the colin::ColinSolver class.
 */

#ifndef colin_ColinSolver_h
#define colin_ColinSolver_h

#include <acro_config.h>
#include <colin/Solver.h>
#include <colin/ParetoSet.h>
#include <colin/SolverMngr.h>
#include <colin/ProblemTraits.h>

#include <colin/OptResponse.h>

//#include <utilib/_math.h>
#include <utilib/PM_LCG.h>
#include <utilib/CommonIO.h>

#include <utilib/seconds.h>


namespace colin {

template <bool MultiObjFlag>
class ObjectiveInfo
{
public:

   ///
   ObjectiveInfo( OptResponse* opt_response_, 
                  utilib::Privileged_PropertyDict &option )
   {
      opt_response_ptr = opt_response_;
      Debug_best = false;
      option.declare
         ( "debug_best", 
           "Print info about the value of the best point found so far",
           utilib::Privileged_Property(Debug_best) );
      //option.add
      //   ("debug_best", Debug_best, 
      //    "Print info about the value of the best point found so far");
      
      Debug_best_point = false;
      option.declare
         ( "debug_best_point", 
           "Print info about the best point found so far",
           utilib::Privileged_Property(Debug_best_point) );
      //option.add
      //   ("debug_best_point", Debug_best_point,
      //    "Print info about the best point found so far");
   }

   ///
   void reset_ObjectiveInfo()
   {
      prev_best = -999.999;
      prev_cbest = -999.999;
      Debug_best = false;
      Debug_best_point = false;
   }

   ///
   bool update_iteration_stats(bool finishing, bool iter_flag)
   {
      if ((!finishing && (prev_best == opt_response_ptr->value())) ||
            (finishing && iter_flag)
         )
      {
         prev_best  = opt_response_ptr->value();
         prev_cbest = opt_response_ptr->constraint_violation;
         return true;
      }
      return false;
   }

   ///
   void print_summary(std::ostream& os, bool prev_flag)
   {
      os << "  Value: ";
      if (prev_flag)
         os << prev_best;
      else
         os << opt_response_ptr->value();
      os << "  CValue: ";
      if (prev_flag)
         os << prev_cbest;
      else
         os << opt_response_ptr->constraint_violation;
   }

   ///
   void write_points(std::ostream& os) const 
   {
      os << "[Single objective] ColinSolver::write_points() "
         "not implemented." << std::endl;
   }

   ///
   void print_objectives(std::ostream& os)
   {
      os << "\tBest Point - Objective Fn:\t\t";
      //os << this->best().response.function_value() << std::endl;
      os << this->opt_response_ptr->value();
      os << std::endl;
#if 0
      if (this->problem->numNonlinearConstraints() > 0)
         os << "\tBest Point - COLIN Merit Fn:\t\t" 
            << this->best().response.augmented_function_value() << std::endl;
#endif
#if 0
      if (problem->numConstraints() > 0)
      {
         os << "\tBest Point - Constraint L2 Norm:\t" 
            << opt_response_ptr->constraint_violation << std::endl;
      }
#endif
   }

   ///
   void print_search_stats(std::ostream& os, bool debug_flag)
   {
      if (Debug_best || debug_flag)
      {
         os << "[Min:\t";
         os << opt_response_ptr->value();
         os << "]\n";
      }

      if (Debug_best_point || debug_flag)
      {
         os << "[Min-point:\n\tPrinting 'Anys' Not Supported]\n";
      }
   }

   ///
   void write(std::ostream& os) const
   {
      os << "Debug_best       " << Debug_best << std::endl;
      os << "Debug_best_point " << Debug_best_point << std::endl;
   }

   /// The value of the best point at iteration \c prev_iter
   double prev_best;

   /// The value of the constraint violation at iteration \c prev_iter
   double prev_cbest;

   /// If true, then print the value of the best point
   bool Debug_best;

   /// If true, then print info about the best point
   bool Debug_best_point;

   ///
   OptResponse* opt_response_ptr;
};


template <>
class ObjectiveInfo<true>
{
public:

   ///
   ObjectiveInfo(OptResponse* opt_response_, 
                 utilib::Privileged_PropertyDict& option)
   {
      opt_response_ptr = opt_response_;
      static_cast<void>(option);
   }

   ///
   void reset_ObjectiveInfo()
   {
      prev_psize = 0;
   }

   ///
   bool update_iteration_stats(bool finishing, bool iter_flag)
   {
      if (finishing && iter_flag)
      {
         prev_psize = pareto_set.size();
         return true;
      }
      return false;
   }

   ///
   void write_points(std::ostream& os) const
   {
   os << std::endl;
   os << "Multi-Objective Cache (Point/Values)" << std::endl;
   os << "------------------------------------" << std::endl;
   os << pareto_set;
   os << std::endl;
   os << utilib::Flush;
   }

   ///
   void print_summary(std::ostream& os, bool prev_flag)
   {
      os << "  ParetoSize: ";
      if (prev_flag)
         os << prev_psize;
      else
         os << pareto_set.size();
   }

   ///
   void print_objectives(std::ostream& os)
   {
      os << "\tPareto Size:\t\t\t\t";
      os << pareto_set.size();
      os << std::endl;
   }

   ///
   void print_search_stats(std::ostream& os, bool debug_flag)
   {
      static_cast<void>(debug_flag);
      os << "[Multi-objective search stats not implemented." << std::endl;
   }

   ///
   void write(std::ostream& os) const
   {
      os << "[Multi-objective ColinSolver::write() not implemented." 
         << std::endl;
   }

   ///
   size_t prev_psize;

   ///
   ParetoSet<utilib::BasicArray<double>, utilib::BasicArray<colin::real>, 
             ArrayDomination<utilib::BasicArray<colin::real> > > pareto_set;

   ///
   OptResponse* opt_response_ptr;
};


//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------


/** The \c ColinSolver class extends Solver to define a
  * core Solver infrastructure that has proven useful in the development
  * of Coliny.
  */
template <class DomainT, class ProblemT>
class ColinSolver
   : public Solver<ProblemT>,
     public ObjectiveInfo<HasProblemTrait(ProblemT, multiple_objectives)>
     //public utilib::CommonIO
{
public:
   
   ///
   ColinSolver();

   ///
   ~ColinSolver() {}

   ///
   virtual void optimize();
   
   ///
   void write(std::ostream& os) const;

   ///
   void write_points(std::ostream& os) const
   {
      if (HasProblemTrait(ProblemT, multiple_objectives))
         ObjectiveInfo < HasProblemTrait(ProblemT, multiple_objectives) >
            ::write_points(os);
      else {
         DomainT tmp;
         utilib::TypeManager()->lexical_cast(this->best().point,tmp);
         os << "Final-Point: " << tmp << std::endl;
         os << "Final-Value: " << this->best().value() << std::endl;
      }
   }

   ///
   const OptResponse& best() const
   { return opt_response; }

   ///
   OptResponse& best() 
   { return opt_response; }

   ///
   void read_initial_point(TiXmlElement* point_elt)
   {
      try {
      read_xml_object(initial_point, point_elt);
      }
      catch (std::exception& err) {
 	 std::cerr << "Error parsing Solver initial point" << std::endl;
         throw;
	}
      opt_response.point = initial_point;
      initial_point_flag = true;
   }

   ///
   void set_initial_point(const utilib::AnyRef& point)
   {
      initial_point_flag = true;
      utilib::TypeManager()->lexical_cast(point, initial_point);
   }


   bool check_convergence()
   {
      time_curr = WallClockSeconds();
      if ((max_time > 0.0) && ((time_curr - time_start) >= max_time))
      {
         this->solver_status.termination_info = "Time-Limit";
         return true;
      }

      if ((max_iters > 0) && (curr_iter > max_iters))
      {
         std::stringstream tmp;
         tmp << "Max-Num-Iterations (" << curr_iter << ">" << max_iters << ")";
         this->solver_status.termination_info = tmp.str();
         return true;
      }

      if ((max_neval > 0) && (this->neval() >= max_neval))
      {
         std::stringstream tmp;
         tmp << "Max-Num-Evals (" << max_neval << "<=" << this->neval() << ")";
         this->solver_status.termination_info = tmp.str();
         return true;
      }

      if ((max_neval_curr > 0) && ((this->neval() - neval_start) >= max_neval_curr))
      {
         std::stringstream tmp;
         tmp << "Max-Num-Evals-Curr (" << max_neval_curr << "<=" 
             << (this->neval() - neval_start) << ")";
         this->solver_status.termination_info = tmp.str();
         return true;
      }

      if ((this->get_problem()->num_objectives == 1) &&
            (opt_response.value() <= accuracy))
      {
         std::stringstream tmp;
         tmp << "Accuracy (" << best().value() << "<=" << accuracy << ")";
         this->solver_status.termination_info = tmp.str();
         return true;
      }

      return false;
   }

   
protected:

   ///
   void print_summary(std::ostream& os, bool prev_flag)
   {
      os << "[\nSummary:\tOpt: " << this->type()
         << "  Iter: " << prev_iter;
      os << "  Neval: " << prev_neval;
      ObjectiveInfo < HasProblemTrait(ProblemT, multiple_objectives) >
         ::print_summary(os, prev_flag);
      os << "\n]\n";
   }

   ///
   bool update_iteration_stats(bool finishing, double total_time)
   {
      if ( ObjectiveInfo < HasProblemTrait(ProblemT, multiple_objectives) >
           ::update_iteration_stats(finishing, prev_iter == prev_print) )
      {
         prev_time  = total_time;
         prev_neval = this->neval();
         prev_iter  = this->curr_iter;
         return true;
      }
      return false;
   }

   /// A single iteration of optimize
   virtual void optimize_iteration()
   {
      EXCEPTION_MNGR(std::logic_error, "ColinSolver::optimize_iteration(): "
                     "Derived solver does not support single iteration "
                     "stepping.");
   }


   ///
   OptResponse opt_response;

   /**@name Termination Controls */

   /** Return the number of CPU seconds since a given point in time.
     * By default, this relies on the UTILIB timing routines.
     */
   virtual double CPUSeconds()
   {return ::CPUSeconds();}
 
   /** Return the number of Wall clock seconds since a given point in time.
     * By default, this relies on the UTILIB timing routines.
     */
   virtual double WallClockSeconds()
   {return ::WallClockSeconds();}
 
   /// The current time
   double time_curr;
 
   /// The time optimization was begin
   double time_start;

   /// The average time for each evaluation
   double time_eval;

   /// The current iteration
   unsigned int curr_iter;

   /// The number of function evaluations at the start of \c optimize
   int neval_start;

   /// An array of sub_solvers
   utilib::BasicArray<std::pair<Solver_Base*, bool> > sub_solver;
   //@}


   /**@name Debugging Controls */
   //@{
   /// The output level (for user output)
   std::string output_levelstr;

   /// The output level for header
   std::string output_headerstr;

   /// The output level (for user output)
   int output_level;

   /// If true, then flush output
   bool Output_flush;

   /// If true, then this is the final output
   bool output_final;

   /// If true, then dynamically control output.
   /// Only iterations with new solution values are
   /// printed.
   bool output_dynamic;

   /// The frequency of output.
   int Output_freq;

   ///
   virtual void virt_debug_io(std::ostream& /*os*/,
                              const bool /*finishing*/,
                              const int /*output_level*/) {}


   /** Print debugging information and call \c virt_debug_io for
     * subclass debugging information.
     */
   void debug_io(std::ostream& os, bool finishing = false);
   //@}

   /**@name Control Options */
   //@{
   /** The maximum number of iterations allowed before termination.
     * If the optimizer is run multiple times, this provides a limit on the
     * total amount of work that is allowed.
     */
   unsigned int max_iters;

   /// The maximum number of function evaluations allowed.
   int max_neval;

   /** The maximum number of function evaluations allowed in the current
     * execution of the optimizer.
     */
   int max_neval_curr;

   /// The maximum number of Wall CLock seconds allowed before termination.
   double max_time;

   /// The minimum function value allowed before termination.
   double accuracy;

   /// The function tolerance required by the algorithm.
   double ftol;

   /// Tolerance used to determine if a constraint is violated
   double constraint_tolerance;

   ///
   int print_precision;
   //@}


   ///
   int debug;

   bool verbosity(int threshhold) const
   { 
      return threshhold <= this->debug; 
   }

   /// If true, then print all debugging info
   bool Debug_all;

   /// If true, then print time info
   bool Debug_time;

   /// If true, then print the number of evaluations
   bool Debug_neval;

   /// If true, then print the iteration info
   bool Debug_iter;

   /// If true, then print info about the optimizer
   bool Debug_opt_stats;

   /// The previous iteration that had a different value
   int prev_iter;

   /// The number of evaluations at iteration \c prev_iter
   int prev_neval;

   /// The time at iteration \c prev_iter
   double prev_time;

   /// TODO
   bool end_iter_flag;

   /// TODO
   int prev_print;

   /// Random number generator
   utilib::PM_LCG pm_rng;

   /// Seed for the random number generator
   utilib::seed_t seed;

   /// Set to true by \c set_initial_point
   bool initial_point_flag;

   ///
   DomainT initial_point;

private:

   ///
   void reset_ColinSolver()
   {
      if (Output_freq < 0)
         Output_freq = 0;

      ucout.precision(print_precision);
      std::cout.precision(print_precision);

      curr_iter = 0;

      if (output_levelstr == "none")
         output_level = 0;
      else if (output_levelstr == "summary")
         output_level = 1;
      else if (output_levelstr == "normal")
         output_level = 2;
      else if (output_levelstr == "verbose")
         output_level = 3;
      else
      {
         EXCEPTION_MNGR(std::runtime_error, "ColinSolver::reset - "
                        "unknown output level \"" << output_levelstr << 
                        "\"\n\t\tOptions: none, summary, normal, verbose");
      }

      if ((output_dynamic) && (output_level > 1))
      {
         std::cerr << "Can only use dynamic output with output "
            "level=\"summary\"\n\tReseting output level to \"summary\"";
         output_level = 1;
      }

      best().reset();
      //for (size_t i = 0; i < sub_solver.size(); i++)
      //   sub_solver[i].first->set_rng(&rng);
      if ( ! this->get_problem_handle().empty() )
         this->get_problem_handle()->set_rng(utilib::AnyRNG(&this->rng));

      prev_iter = 0;
      prev_neval = 0;
      prev_time = 0.0;
      prev_print = -1;
      end_iter_flag = false;
      curr_iter = 0;
      if (seed > 0)
         pm_rng.set_seed(seed);

      
      // bring in the initial point from the initial point cache
      size_t pt_ct = this->initial_points->size(this->problem.application());
      if ( pt_ct == 1 )
      {
         this->initial_points.get_point(this->problem, initial_point);
         initial_point_flag = true;
      }
      else if ( pt_ct > 1 )
         std::cerr << std::endl 
                   << "WARNING: ColinSolver::reset_ColinSolver(): "
            "ColinSolver derivatives can only handle a single "
            "initial point (provided " << pt_ct << ")." 
                   << std::endl << std::endl;


      // ...from former opt_init()

      time_start = WallClockSeconds();
      neval_start = this->neval();

      if (output_headerstr == "verbose")
      {
         ucout << std::endl;
         ucout << std::string(77, '*') << std::endl;
         ucout << std::string(77, '*') << std::endl;
         ucout << "****** Coliny Solver: " << this->type() << std::endl;
         ucout << std::string(77, '*') << std::endl;
         ucout << std::string(77, '*') << std::endl;
      }

      if (output_headerstr == "verbose")
      {
         ucout << std::endl << "Solver Parameters:" << std::endl;
         this->properties.write(utilib::PropertyDict::DescriptionWriter(ucout));
         //this->option.write(ucout);
         ucout << std::endl << "Solver Parameter Values:" << std::endl;
         this->properties.write(utilib::PropertyDict::ValueWriter(ucout));
         //this->write_parameter_values(ucout);
      }
   }

   ///
   void 
   print_ColinSolver(std::ostream& output) const
   {
      bool multiobj = HasProblemTrait(ProblemT, multiple_objectives);
      output << "Final-Stats:";
      output.precision(12);
      output << " neval " << this->neval();
      if (!multiobj)
      {
         output << " constraint_violation ";
         output.precision(12);
         output << best().constraint_violation ;
      }
      output << std::endl;
      output << "Termination: " << this->solver_status.termination_info << std::endl;
      output.flush();
      //output << "ModelStatus: " << best().model_status << std::endl;
      //output << "SolverStatus: " << best().solver_status << std::endl;
   }

};


//============================================================================
//
//
template <class DomainT, class ProblemT>
ColinSolver<DomainT, ProblemT>::ColinSolver()
   : Solver<ProblemT>(),
     ObjectiveInfo < HasProblemTrait(ProblemT, multiple_objectives) >
        (&this->opt_response, this->properties)
{
   output_level = 2;
   prev_iter = 0;
   prev_neval = 0;
   prev_time = 0.0;
   end_iter_flag = false;
   prev_print = -1;
   this->rng = &pm_rng;

   //utilib::OptionParser &opt = this->option;
   /// Termination parameters

   max_iters = 0;
   this->properties.declare
      ( "max_iterations", 
        "Maximum # of iterations before termination",
        utilib::Privileged_Property(max_iters) );
   //opt.add("max_iterations", max_iters,
   //        "Maximum # of iterations before termination");
   //opt.alias("max_iterations", "max_iters");

   
   max_neval = 0;
   this->properties.declare
      ( "max_function_evaluations", 
        "Maximum # of function evaluations before termination",
        utilib::Privileged_Property(max_neval) );
   //opt.add("max_func_evaluations", max_neval,
   //        "Max # of fevals allowed before termination (total ever allowed)");
   //opt.alias("max_func_evaluations", "max_neval");
   //opt.alias("max_func_evaluations", "max_function_evaluations");

   max_neval_curr = 0;
   this->properties.declare
      ( "max_function_evaluations_this_trial", 
        "Maximum # of function evaluations before terminating this trial",
        utilib::Privileged_Property(max_neval_curr) );
   //opt.add("max_func_evaluations_this_trial", max_neval_curr,
   //        "Max # of fevals allowed in this minimization trial");
   //opt.alias("max_func_evaluations_this_trial", "max_neval_curr");
   //opt.alias("max_func_evaluations_this_trial", 
   //          "max_function_evaluations_this_trial");

   max_time = 0.0;
   this->properties.declare
      ( "max_time", 
        "Maximum time (in seconds) before termination",
        utilib::Privileged_Property(max_time) );
   //opt.add("max_time", max_time,
   //        "Max time allowed before termination");

   accuracy = -MAXDOUBLE;
   this->properties.declare
      ( "sufficient_objective_value", 
        "Minimum solution value allowed before termination",
        utilib::Privileged_Property(accuracy) );
   //opt.add("min_function_value", accuracy,
   //        "Minimum solution value allowed before termination");
   //opt.alias("min_function_value", "accuracy");

   
   ftol = 0.0;
   this->properties.declare
      ( "function_value_tolerance", 
        "A function tolerance used for termination",
        utilib::Privileged_Property(ftol) );
   //opt.add("function_value_tolerance", ftol,
   //        "A function tolerance used for termination");
   //opt.alias("function_value_tolerance", "ftol");


   /// General parameters

   constraint_tolerance = 1e-8;
   this->properties.declare
      ( "constraint_tolerance", 
        "Tolerance used to determine if a constraint is violated",
        utilib::Privileged_Property(constraint_tolerance) );
   //opt.add("constraint_tolerance", constraint_tolerance,
   //        "Tolerance used to determine whether a constraint is violated");

   print_precision = 10;
   this->properties.declare
      ( "precision", 
        "Controls the reported significant digits for floating point I/O",
        utilib::Privileged_Property(print_precision) );
   //opt.add("precision", print_precision,
   //        "Used to control the precision of I/O");
   
   /// Debugging parameters

   debug = 0;
   this->properties.declare
      ( "debug", 
        "General debugging level (positive integer)",
        utilib::Privileged_Property(debug) );
   //opt.add("debug", debug,
   //        "General debugging level (>0)");

   output_headerstr = "normal";
   this->properties.declare
      ( "output_header", 
        "Output level for solver header:\n"
        "   none    - No header output\n"
        "   normal  - Simple header output\n"
        "   verbose - Simple header output plus parameter information",
        utilib::Privileged_Property(output_headerstr) );
   //opt.add("output_header", output_headerstr,
   //        "Output level for the solver header:\n"
   //        "\t  none    - No header output\n"
   //        "\t  normal  - Simple header output\n"
   //        "\t  verbose - Simple header output plus parameter information");

   output_levelstr = "normal";
   this->properties.declare
      ( "output_level", 
        "Output level for debugging:\n"
        "   none    - No debugging output\n"
        "   normal  - Normal I/O\n"
        "   summary - Summary debugging information\n"
        "   verbose - All debugging information",
        utilib::Privileged_Property(output_levelstr) );
   //opt.add("output_level", output_levelstr,
   //        "Output level for debugging:\n"
   //        "\t  none    - No debugging output\n"
   //        "\t  normal  - Normal I/O\n"
   //        "\t  summary - Summary debugging information\n"
   //        "\t  verbose - All debugging information");
   
   Output_freq = 1;
   this->properties.declare
      ( "output_frequency", 
        "The frequency (in iterations) with which output is printed",
        utilib::Privileged_Property(Output_freq) );
   //opt.add("output_frequency", Output_freq,
   //        "The frequency with which output is printed.");
   //opt.alias("output_frequency", "output_freq");

   output_dynamic = false;
   this->properties.declare
      ( "output_dynamic", 
        "If true, then only print output when an improving value is found",
        utilib::Privileged_Property(output_dynamic) );
   //opt.add("output_dynamic", output_dynamic,
   //        "If true, then only print output when an improving value is found");

   output_final = false;
   this->properties.declare
      ( "output_final", 
        "If true, then only print output when finished with optimization",
        utilib::Privileged_Property(output_final) );
   //opt.add("output_final", output_final,
   //        "If true, then only print output when finished with optimization");

   Output_flush = true;
   this->properties.declare
      ( "output_flush", 
        "If true, then flush after every time debugging IO is generated",
        utilib::Privileged_Property(Output_flush) );
   //opt.add("output_flush", Output_flush,
   //        "If true, then flush after every time debugging IO is generated");


   /// Debugging parameters

   seed = 0;
   this->properties.declare
      ( "seed", 
        "The seed value for the random number generator.  A value "
        "of zero indicates that the current time is used to seed the random "
        "number generator",
        utilib::Privileged_Property(seed) );
   //opt.add
   //   ("seed", seed,
   //    "The seed value for the random number generator.  A value "
   //    "of zero indicates that the current time is used to seed the random "
   //    "number generator");

   Debug_time = false;
   this->properties.declare
      ( "debug_time", 
        "Print info about the amount of elapsed time",
        utilib::Privileged_Property(Debug_time) );
   //opt.add("debug_time", Debug_time,
   //        "Print info about the amount of elapsed time");

   Debug_neval = false;
   this->properties.declare
      ( "debug_num_evaluations", 
        "Print info about the amount of function evaluations",
        utilib::Privileged_Property(Debug_neval) );
   //opt.add("debug_num_evaluations", Debug_neval,
   //        "Print info about the number of fevals");
   //opt.alias("debug_num_evaluations", "debug_neval");

   Debug_iter = false;
   this->properties.declare
      ( "debug_iteration_info", 
        "Print info about the current iteration",
        utilib::Privileged_Property(Debug_iter) );
   //opt.add("debug_iteration_info", Debug_iter,
   //        "Print info about the current iteration");
   //opt.alias("debug_iteration_info", "debug_iter");

   Debug_opt_stats = false;
   this->properties.declare
      ( "debug_opt_stats", 
        "Print general optimization statistics",
        utilib::Privileged_Property(Debug_opt_stats) );
   //opt.add("debug_opt_stats", Debug_opt_stats,
   //        "Print general optimization statistics");

   Debug_all = false;
   this->properties.declare
      ( "debug_all", 
        "Enable all debug_* options",
        utilib::Privileged_Property(Debug_all) );
   //opt.add("debug_all", Debug_all,
   //        "Enable all debug_* options");


   this->reset_signal.connect
      ( boost::bind( &ObjectiveInfo<HasProblemTrait
                     (ProblemT, multiple_objectives)>::reset_ObjectiveInfo, 
                     this ) );
   this->reset_signal.connect
      ( boost::bind( &ColinSolver<DomainT, ProblemT>::reset_ColinSolver, 
                     this ) );
   #if 0
   this->print_signal.connect
      ( boost::bind( &ColinSolver<DomainT, ProblemT>::print_ColinSolver, 
                     this, _1 ) );
   #endif
}


//============================================================================
//
//
template <class DomainT, class ProblemT>
void ColinSolver<DomainT, ProblemT>::optimize()
{
   //this->opt_init();

   unsigned int num_iters;
   if (this->max_iters <= 0)
      num_iters = MAXINT;
   else
      num_iters = this->curr_iter + this->max_iters;

   debug_io(ucout);
   for (this->curr_iter++; this->curr_iter <= num_iters; this->curr_iter++)
   {
      if (this->check_convergence())
         break;

      optimize_iteration();

      debug_io(ucout);
   }

   debug_io(ucout, true);
}


//============================================================================
//
//
template <class DomainT, class ProblemT>
void ColinSolver<DomainT, ProblemT>::write(std::ostream& os) const
{
   //Solver<ProblemT>::write(os);
   ObjectiveInfo<HasProblemTrait(ProblemT, multiple_objectives)>::write(os);

   os << "Debug_time       " << Debug_time << std::endl;
   os << "Debug_neval      " << Debug_neval << std::endl;
   os << "Debug_iter       " << Debug_iter << std::endl;
   os << "Debug_opt_stats  " << Debug_opt_stats << std::endl;
}


//============================================================================
//
//
template <class DomainT, class ProblemT>
void ColinSolver<DomainT, ProblemT>::debug_io( std::ostream& os,
                                                   bool finishing )
{
//
// Perform a preliminary check to see if anything is going to be
// printed.  This doesn't check for the virt_debug_io routines,
// which are restricted to being printed by Output_freq
//
   if ((this->Output_freq == 0) ||
         ((this->debug == 0) && (this->output_level == 0)) ||
         (this->output_final && !finishing)) return;

   if ((this->Output_freq > 0) &&
         !(((this->curr_iter % this->Output_freq) == 0) ^ finishing))
   {
      os << ")\n";
      if (this->solver_status.termination_info != "")
      {
         os << "[ Termination: ";
         os << this->solver_status.termination_info;
         os << " ]\n";
      }
      if (this->Output_flush) { os.flush(); ucout << utilib::Flush; }
      return;
   }

   this->time_curr = WallClockSeconds();
   double total_time = this->time_curr - this->time_start;

   if (this->output_dynamic)
      update_iteration_stats(finishing, total_time);

   if (end_iter_flag && (this->output_level > 0))
      os << ")\n";
   else
      end_iter_flag = 1;

   if (!finishing &&
         this->output_dynamic &&
         (this->output_level == 1) &&
         (this->curr_iter > 0) &&
         (prev_print != prev_iter))
   {
      os << "(---COLIN--- Begin Optimizer Iteration -----------------------------------------" << std::endl;
      print_summary(os, true);
      os << ")\n";
   }

   if (this->output_level > 0)
      os << "(---COLIN--- Begin Optimizer Iteration -----------------------------------------" << std::endl;


//
// Go through list of debuging stats
//
   if (((this->Output_freq > 0) &&
         (((this->curr_iter % this->Output_freq) == 0) ^ finishing)) ||
         this->output_dynamic || this->output_final)
   {

      //
      // User Output
      //
      switch (this->output_level)
      {
      case 0:            // No user output
         break;

      case 1:
         if (finishing && this->output_dynamic)
            print_summary(os, true);
         else
         {
            print_summary(os, false);
            prev_print = this->curr_iter;
         }
         break;

      case 2:
      case 3:
         if (this->output_level == 2)
            os << "[\nNormal:" << std::endl;
         else
            os << "[\nVerbose:" << std::endl;
         os << "\tOptimizer Type:\t\t\t\t" << this->type() << std::endl;
         os << "\tOptimizer Name:\t\t\t\t" 
            << SolverMngr().get_solver_name(this) << std::endl;
         os << "\tIteration Number:\t\t\t" << this->curr_iter << std::endl;
         os << "\tTotal # Func Evals:\t\t\t" << this->neval() << std::endl;
         os << "\tTotal Time (wall clock):\t\t" << total_time << std::endl;
         this->print_objectives(os);
         this->virt_debug_io(os, finishing, this->output_level);
         os << "]\n";
         break;
      };

      if (this->debug)
      {
         //
         // Debugging Output
         //
         if (Debug_iter || Debug_all)
            os << "[Iter:\t" << this->curr_iter << "]\n";

         if (Debug_neval || Debug_all)
            os << "[Neval:\t" << this->neval() << "]\n";

         if (Debug_time || Debug_all)
         {
            os << "[Time:\t" << total_time << "]\n";
            os << "[Time-Stamp:\t" << ElapsedCPUSeconds() << "]\n";
         }

         this->print_search_stats(os, Debug_opt_stats || Debug_all);

         //
         // Call virtual function which may print more,
         // optimizer-specific stats
         //
         this->virt_debug_io(os, finishing, -1);
      }
   }

   if (this->Output_flush) { os.flush(); ucout << utilib::Flush; }
}

}

#endif // colin_ColinSolver_h
