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
 * \file TGPO.h
 *
 * Defines the scolib::TGPO class
 */

#ifndef scolib_TGPO_h
#define scolib_TGPO_h

#include <acro_config.h>
#include <colin/Solver.h>

namespace scolib {

class TGPO : public colin::Solver<colin::UNLP0_problem>
{
public:
   /// Constructor
   TGPO();

   ///
   void optimize();

   /// The number of points to use for the initial LHS sample
   utilib::Property LHS_sample_size;

   /// The number of points to use to evaluate the TGP model
   utilib::Property TGP_sample_size;

   /// The number of D-Optimal designs to generate using the TGP model
   utilib::Property DOptimal_designs;

   /// The MCMC parameters for sensitivity sampling
   utilib::Property MCMC_BTE;

   /// Evaluate the EGO point identified by the TGP model
   utilib::Property eval_ego_point;

   /// Evaluate point from the TGP model with the highest standard deviation
   utilib::Property eval_max_stdev_point;

   /// Evaluate point from the TGP model with the best bound on estimate
   utilib::Property eval_confidence_bounded_point;

   /// Local optimizer to create & use to refine EGO point
   utilib::Property local_optimizer;

   /// How often to run the local search algorithm (every NN TGP iterations)
   utilib::Property local_search_frequency;

   /// Relative tolerance between best identified point and the 95%
   /// confidence bound for the TGP model
   utilib::Property optimality_tolerance;

   /// Maximum number of major iterations (number of TGP models generated)
   utilib::Property maximum_iterations;

   /// Scaled proxmity threshold for excluding data from the evaluation cache
   /** Scaled proxmity threshold for excluding data from the evaluation cache
    *    0 == no filter, 
    *    1 == force points to be roughly as far apart as the TGP sample grid
    *    > TGP samples / Num variables: filter all but the best point
    */
   utilib::Property data_filter_threshold;

private:
   struct Data;
   Data *data;

   struct tgp_results_t {
      // Solution with best expected improvement
      utilib::Any ego_domain;
      double      ego_mean;
      double      ego_q1;
      utilib::Any ego_eval;

      // Best solution based on lower confidence bound
      utilib::Any conf_domain;
      double      conf_mean;
      double      conf_q1;
      utilib::Any conf_eval;

      // Best solution based on lower confidence bound
      utilib::Any sd_domain;
      double      sd_mean;
      double      sd_q1;
      utilib::Any sd_eval;
   };

   void cb_new_best_point(colin::Cache::cache_t::iterator);

   int get_lhs_num_samples();

   int get_tgp_num_samples();

   void sample_problem_LHS();

   void TGP(tgp_results_t&);

   void processEvaluationCache(colin::CacheHandle cache);

   void printResponse(std::string prompt, colin::AppResponse r);

protected:
   std::string define_solver_type() const
   { return "TGPO"; }
};


} // namespace scolib

#endif // defined scolib_TGPO_h
