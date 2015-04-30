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
 * \file MultiStart.h
 *
 * Defines the \c MultiStartBase and \c MultiStart classes
 */

#ifndef scolib_Multistart_h
#define scolib_Multistart_h

#include <acro_config.h>
#include <utilib/SampleGenerators.h>
#include <utilib/DUniform.h>
#include <utilib/CharString.h>
#include <colin/solver/ColinSolver.h>
//#include <colin/AppResponseAnalysis.h>

namespace scolib
{

using utilib::CharString;
using utilib::MixedIntVars;



/** \class MultiStartBase
  *
  * A generic engine that manages a set of independent local search
  * runs.
  */
template <class DomainT, class ProblemT, class GeneratorT>
class MultiStartBase : public colin::ColinSolver<DomainT, ProblemT>
{
   typedef colin::ColinSolver<DomainT, ProblemT> base_t;
#if !defined(SOLARIS)
   using base_t::max_time;
   using base_t::problem;
   using base_t::max_iters;
   using base_t::curr_iter;
   using base_t::constraint_tolerance;
   using base_t::max_neval;
   using base_t::accuracy;
#endif

   ///
   void reset_MultiStartBase();

public:

   /// Constructor
   MultiStartBase();

   /// Destructor
   virtual ~MultiStartBase() {}

   ///
   void optimize();

   ///
   void 
   write_parameter_values(std::ostream& os, const char* opt_label = "") const
   {
      EXCEPTION_MNGR(std::logic_error, "MultiStartBase::"
                     "write_parameter_values(): NOT YET IMPLEMENTED.");
      /*
      if (strcmp(opt_label, "") == 0)
         utilib::ParameterSet::write_parameter_values(os, "Multistart-Master");
      else
         utilib::ParameterSet::write_parameter_values(os, opt_label);
      for (unsigned int i = 0; i < this->sub_solver.size(); i++)
      {
         CharString str = "Solver-";
         str += i;
         this->sub_solver[i].first->write_parameter_values(os, str.data());
      }
      */
   }

   ///
   void write_parameter_values(utilib::PackBuffer& os) const
      {base_t::write_parameter_values(os);}

protected:

   ///
   unsigned int batch_size;

   ///
   double ls_frequency;

   ///
   utilib::DUniform<size_type> drand;

   ///
   utilib::Uniform urand;

   ///
   utilib::BasicArray< colin::real > val;

   ///
   utilib::BasicArray< colin::AppResponse > ans;

   /// The sum of squared constraint violations
   //utilib::BasicArray<colin::real > sq_violation;

   ///
   utilib::BasicArray<DomainT> point;

   ///
   GeneratorT generator;

   ///
   void generate_point(DomainT& pt)
   {
      initialize_point(pt);
      generator.new_point(pt);
   }

   ///
   virtual void initialize_point(DomainT&) {}

   ///
   virtual void setup()
   {
      if (this->initial_point_flag)
      {
         evaluate_point(this->initial_point,
                        this->best().value(),
                        this->best().response);
      }
      else
      {
         generate_point(point[0]);
         this->best().point = point[0];
         evaluate_point(point[0],
                        this->best().value(),
                        this->best().response);
      }
      synchronize();
   }

   ///
   void do_ls(DomainT& pt, colin::AppResponse& response)
   {
      try
      {
         size_type ndx = drand();
         this->sub_solver[ndx].first->set_initial_point(pt);
         this->sub_solver[ndx].first->reset();
         this->sub_solver[ndx].first->minimize();
         response = this->sub_solver[ndx].first->opt_response.response;
         pt << this->sub_solver[ndx].first->opt_response.point
            .template expose<DomainT>();
         //ucout << "BUG: " << response.function_value() << " " << this->sub_solver[ndx].first->opt_response().response.function_value() << std::endl;
      }
      catch (std::exception& err)
      {
         EXCEPTION_MNGR(std::runtime_error, "MultiStartBase::do_ls - "
                        "Problem applying local search\n\tError: "
                        << err.what());
      }
   }

   ///
   void evaluate_point( const DomainT& pt, colin::real& val, 
                        colin::AppResponse& response )
   {
      int priority = 1;
      colin::AppRequest request = problem->set_domain(pt);
      problem->Request_response(request,response);
      problem->Request_F(request, val);
      this->eval_mngr().queue_evaluation(request);
   }

   ///
   void synchronize()
   { this->eval_mngr().synchronize(); }

};


template <class DomainT, class ProblemT, class GeneratorT>
MultiStartBase<DomainT, ProblemT, GeneratorT>::MultiStartBase()
{
   utilib::OptionParser& opt = this->option;

   batch_size = 1;
   opt.add("batch_size", batch_size,
           "The number of points generated in each iteration");

   ls_frequency = 0.0;
   opt.add("ls_frequency", ls_frequency,
           "The probability of applying local search to each point generated");

   this->reset_signal.connect(boost::bind(&MultiStartBase<DomainT, ProblemT, GeneratorT>::reset_MultiStartBase, this));
}


template <class DomainT, class ProblemT, class GeneratorT>
void MultiStartBase<DomainT, ProblemT, GeneratorT>::reset_MultiStartBase()
{
   if ( problem.empty() ) return;

   if ((ls_frequency < 0.0) || (ls_frequency > 1.0))
   {
      EXCEPTION_MNGR(std::runtime_error, "Bad local search frequency: " << ls_frequency);
   }

   if (!(problem->finite_bound_constraints()))
   {
      EXCEPTION_MNGR(std::runtime_error, "MultiStartBase::reset - not enforcing all bounds.");
   }

   urand.generator(&(this->rng));
   drand.generator(&(this->rng));
   generator.set_rng((this->rng));

   if ((this->sub_solver.size() > 0) && (ls_frequency == 0.0))
   {
      ls_frequency = 1.0;
   }
   for (unsigned int i = 0; i < this->sub_solver.size(); i++)
   {
      this->sub_solver[i].first->option().set_parameter("accuracy", accuracy);
      this->sub_solver[i].first->option().set_parameter("max_neval", max_neval);
      this->sub_solver[i].first->option().set_parameter("max_time", max_time);
   }

   ans.resize(batch_size);
   val.resize(batch_size);
   point.resize(batch_size);
   drand.low(0);
   drand.high(this->sub_solver.size() - 1);
}


template <class DomainT, class ProblemT, class GeneratorT>
void MultiStartBase<DomainT, ProblemT, GeneratorT>::optimize()
{
//
// Misc initialization of the optimizer
//
   this->opt_init();
//
// Generate initial point and setup data structures
//
   setup();
//
// Setup iteration counters and debugging IO
//
   unsigned int num_iters;
   if (max_iters <= 0)
      num_iters = MAXINT;
   else
      num_iters = curr_iter + max_iters;
   this->debug_io(ucout);
//
// Main Loop
//
   for (curr_iter++; curr_iter <= num_iters;  curr_iter++)
   {
      //
      // Check termination rules
      //
      if (this->check_convergence())
         break;
      //
      // Generate all of the points.
      //
      for (unsigned int b = 0; b < batch_size; b++)
      {
         generate_point(point[b]);
         if ((ls_frequency >= 1.0) ||
               ((ls_frequency > 0.0) && (urand() < ls_frequency)))
            do_ls(point[b], ans[b]);
         else
            evaluate_point(point[b], val[b], ans[b]);
      }
      //
      // Synchronize and compute the penalized objective
      //
      synchronize();
      DEBUGPR(100,
              for (unsigned int i = 0; i < batch_size; i++)
              {
                 ucout << "Trial point " << i << std::endl;
                 ucout << "Final Point: " << point[i] << std::endl;
                 ucout << "Value: " << val[i] << std::endl << utilib::Flush;
              }
              );

      //
      // Find the best point
      //
      int bp = 0;
      for (unsigned int i = 1; i < val.size(); i++)
         if (val[i] < val[bp]) bp = i;
      if (val[bp] < this->best().value())
      {
         this->best().value() = val[bp];
         this->best().response = ans[bp];
         //this->best().constraint_violation = sq_violation[bp];
         this->best().point = point[bp];
      }

      this->debug_io(ucout);
   }

   this->debug_io(ucout, true);
}


template <class DomainT, 
          class ProblemT, 
          class GeneratorT = utilib::UniformSampleGenerator<DomainT> >
class MultiStart : public MultiStartBase<DomainT, ProblemT, GeneratorT>
{
public:

   /// Constructor
   MultiStart() {}
};


template <class ProblemT>
class MultiStart< MixedIntVars, ProblemT, 
                  utilib::UniformSampleGenerator<MixedIntVars> >
   : public MultiStartBase< MixedIntVars, ProblemT, 
                            utilib::UniformSampleGenerator<MixedIntVars> >
{
   typedef MultiStartBase
   < MixedIntVars, ProblemT, 
     utilib::UniformSampleGenerator<MixedIntVars> > base_t;

   ///
   void reset_MultiStart()
   {
      if ( this->get_problem().empty() ) 
         return;
      this->get_problem()->get_real_bounds
         (this->generator.d_l_bound, this->generator.d_u_bound);
      this->get_problem()->get_int_bounds
         (this->generator.i_l_bound, this->generator.i_u_bound);
   }

public:


   /// Constructor
   MultiStart() 
   { 
      this->reset_signal.connect
         (boost::bind(&MultiStart<MixedIntVars, ProblemT, 
                      utilib::UniformSampleGenerator<MixedIntVars> >::reset_MultiStart, this)); 
   }
         

   ///
   void initialize_point(MixedIntVars& point)
   {
      point.resize(this->get_problem()->num_binary_vars(), 
                   this->get_problem()->num_int_vars(), 
                   this->get_problem()->num_real_vars());
   }

protected:
   std::string define_solver_type() const
   { return "MultiStart"; }
};

} // namespace scolib

#endif
