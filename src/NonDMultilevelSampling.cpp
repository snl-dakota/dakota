/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDMultilevelSampling
//- Description: Implementation code for NonDMultilevelSampling class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDMultilevelSampling.hpp"
#include "ProblemDescDB.hpp"

static const char rcsId[]="@(#) $Id: NonDMultilevelSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDMultilevelSampling::
NonDMultilevelSampling(ProblemDescDB& problem_db, Model& model):
  NonDSampling(problem_db, model),
  pilotSamples(probDescDB.get_sza("method.nond.pilot_samples"))
{
  sampleType = SUBMETHOD_RANDOM;

  // check the iteratedModel for the appropriate hierarchical structure
  if (iteratedModel.surrogate_type() != "hierarchical") {
    Cerr << "Error: Multilevel Monte Carlo requires a hierarchical surrogate "
	 << "model specification." << std::endl;
    abort_handler(-1);
  }
  // set SurrogateModel::responseMode
  iteratedModel.surrogate_response_mode(MODEL_DISCREPANCY);
}


NonDMultilevelSampling::~NonDMultilevelSampling()
{ }


void NonDMultilevelSampling::pre_run()
{
}


/** Loop over the set of samples and compute responses.  Compute
    statistics on the set of responses if statsFlag is set. */
void NonDMultilevelSampling::quantify_uncertainty()
{
  // to start, assume one model form with multiple discretization levels...
  // later, return hierarchy of model forms from the hierarchical surrogate,
  // each of which may contain multiple discretization levels...

  //model,
  //  surrogate hierarchical
  //    ordered_model_fidelities = 'LF' 'MF' 'HF'
  //
  // Note: how to specify peer dimension of equivalent alternatives
  // May want to include these in MLMC process with adaptive selection of
  // most correlated alternative (or a convex combination of alternatives)

  //model
  //  single id_model = 'LF'
  //    # point to state vars; ordered based on set values for h, delta-t
  //    discretization_levels = 'dssiv_space' 'dssiv_time'
  //    # relative cost estimates in same order as state set values
  //    # --> re-sort into map keyed by cost
  //    discretization_level_costs = 10 2 .02 # space
  //                                 1 10 100 # time
  //
  // Need to formulate as a coordinated progression towards convergence, where
  // e.g., time step is NOT a state parameter and is inferred from the spatial
  // discretization based on some criterion, e.g. CFL condition.
  // Can we reliably capture runtime estimates as part of pilot run w/i Dakota?
  // Ultimately seems desirable to support either online or offline cost
  // estimates, to allow more accurate resource allocation when possible
  // or necessary (e.g., combustion processes with expense that is highly
  // parameter dependent).
  
  size_t lev, num_lev = 1,//iteratedModel.discretization_levels().size(),
    qoi, num_qoi = iteratedModel.num_functions(), samp, new_N_l;
  SizetArray N_l, delta_N_l;
  RealVector agg_var(num_lev, false), cost(num_lev);
  RealMatrix sum_Y(num_qoi, num_lev),        sum_Y2(num_qoi, num_lev),
             exp_Y(num_qoi, num_lev, false),  var_Y(num_qoi, num_lev, false);
  bool log_resp_flag = (allDataFlag || statsFlag), log_best_flag = false;
  Real agg_var_l, eps = 1.e-6, sum_var_cost, mean;
  
  // Initialize for pilot sample
  N_l.assign(num_lev, 0);
  if (pilotSamples.empty()) delta_N_l.assign(num_lev, 100); // default
  else                      delta_N_l = pilotSamples;

  // How to manage a set of statistics:
  // 1. Simplest: proposal is to use the mean estimator to drive the algorithm,
  //    but carry along other estimates.
  // 2. Later: could consider a refinement for converging the estimator of the
  //    variance after convergence of the mean estimator.

  // How to manage a vector of QoI:
  // 1. Worst case: select N_l based only on QoI w/ highest total variance
  //      from pilot run --> fix for all levels and don't allow switching
  //      across major iterations (possible oscillation?  Or simple overlay
  //      of resolution reqmts?)
  // 2. Better: select N_l based on convergence in aggregated variance
  
  // now converge on sample counts per level (N_l)
  while (Pecos::l1_norm(delta_N_l)) {
      
    sum_var_cost = 0.;
    for (lev=0; lev<num_lev; ++lev) {

      // set the discretization level within the model form
      //iteratedModel.discretization_level_index(lev);
      // set the number of current samples from the defined increment
      numSamples = delta_N_l[lev];
      // update total samples performed for this level
      N_l[lev]  += numSamples;

      // generate new MC parameter sets
      get_parameter_sets(iteratedModel);// pull dist params from any model
      // evaluate the parameter sets on the hierarchical model
      // TO DO: would be desirable to return delta-QoI for all levels...
      evaluate_parameter_sets(iteratedModel, log_resp_flag, log_best_flag);
      
      // process allResponses... loop over new samples and qoi...
      Real *sum_Y_l = sum_Y[lev], *sum_Y2_l = sum_Y2[lev],
	   *exp_Y_l = exp_Y[lev],  *var_Y_l =  var_Y[lev];
      for (qoi=0; qoi<num_qoi; ++qoi) {
	for (samp=0; samp<numSamples; ++samp) {
	  Real delta_fn  = allResponses[samp].function_value(qoi);
	  sum_Y_l[qoi]  += delta_fn; // running sum across all increments
	  sum_Y2_l[qoi] += delta_fn * delta_fn;
	}
	mean = exp_Y_l[qoi] = sum_Y_l[qoi] / N_l[lev];
	var_Y_l[qoi] = sum_Y2_l[qoi] / N_l[lev] - mean * mean;
	// TO DO: other stats?
      }

      // aggregate independent variances across QoI for estimating N_l
      Real& agg_var_l = agg_var[lev]; agg_var_l = 0.;
      for (qoi=0; qoi<num_qoi; ++qoi)
	agg_var_l += var_Y_l[qoi];
      sum_var_cost += agg_var_l * cost[lev];
    }

    // update targets based on variance estimates
    for (lev=0; lev<num_lev; ++lev) {
      new_N_l = std::sqrt(agg_var[lev] / cost[lev] * sum_var_cost)
	      * 2. / (eps * eps);
      delta_N_l[lev] = (new_N_l > N_l[lev]) ? new_N_l - N_l[lev] : 0;
    }
  }

  // aggregate mean and variance of estimator(s)
  // Final estimator result is sum of final mu_l from telescopic sum
  // Final variance of estimator is a similar sum of variances
  RealVector exp_f(num_qoi), var_f(num_qoi); exp_f = 0.;
  for (lev=0; lev<num_lev; ++lev) {
    Real *exp_Y_l = exp_Y[lev], *var_Y_l = var_Y[lev];
    for (qoi=0; qoi<num_qoi; ++qoi) {
      exp_f[qoi] += exp_Y_l[qoi];
      var_f[qoi] += var_Y_l[qoi];
    }
  }
}


void NonDMultilevelSampling::post_run(std::ostream& s)
{
  // Statistics are generated here and output in print_results() below
  if (statsFlag) // calculate statistics on allResponses
    compute_statistics(allSamples, allResponses);

  Analyzer::post_run(s);
}


void NonDMultilevelSampling::print_results(std::ostream& s)
{
  if (statsFlag) {
    s << "\nStatistics based on " << numSamples << " samples:\n";
    print_statistics(s);
  }
}

} // namespace Dakota
