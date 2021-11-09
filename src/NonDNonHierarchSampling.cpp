/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDNonHierarchSampling
//- Description: Implementation code for NonDNonHierarchSampling class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
//#include "dakota_tabular_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDNonHierarchSampling.hpp"
#include "ProblemDescDB.hpp"
#include "ActiveKey.hpp"
#include "DakotaIterator.hpp"

static const char rcsId[]="@(#) $Id: NonDNonHierarchSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";

namespace Dakota {


/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDNonHierarchSampling::
NonDNonHierarchSampling(ProblemDescDB& problem_db, Model& model):
  NonDEnsembleSampling(problem_db, model),
  solutionMode(
    problem_db.get_short("method.nond.ensemble_sampling_solution_mode")),
  optSubProblemForm(0)
{
  // check iteratedModel for model form hi1erarchy and/or discretization levels;
  // set initial response mode for set_communicators() (precedes core_run()).
  bool err_flag = false;
  if (iteratedModel.surrogate_type() == "non_hierarchical")
    aggregated_models_mode(); // truth model + all approx models
  else {
    Cerr << "Error: Non-hierarchical sampling requires a non-hierarchical "
         << "surrogate model specification." << std::endl;
    err_flag = true;
  }

  ModelList& model_ensemble = iteratedModel.subordinate_models(false);
  size_t i, num_mf = model_ensemble.size(), num_lev;
  ModelLIter ml_it;
  NLev.resize(num_mf);
  for (i=0, ml_it=model_ensemble.begin(); ml_it!=model_ensemble.end();
       ++i, ++ml_it) {
    // for now, only SimulationModel supports solution_{levels,costs}()
    num_lev = ml_it->solution_levels(); // lower bound is 1 soln level

    // Ensure there is consistent cost data available as SimulationModel must
    // be allowed to have empty solnCntlCostMap (when optional solution control
    // is not specified).  Passing false bypasses lower bound of 1.
    // > For ACV, only require 1 solution cost, neglecting resolutions for now
    //if (num_lev > ml_it->solution_levels(false)) { // 
    if (ml_it->solution_levels(false) == 0) { // default is 0 soln costs
      Cerr << "Error: insufficient cost data provided for ACV sampling."
	   << "\n       Please provide solution_level_cost estimates for model "
	   << ml_it->model_id() << '.' << std::endl;
      err_flag = true;
    }

    //Sizet2DArray& Nl_i = NLev[i];
    NLev[i].resize(num_lev); //Nl_i.resize(num_lev);
    //for (j=0; j<num_lev; ++j)
    //  Nl_i[j].resize(numFunctions); // defer
  }

  if (err_flag)
    abort_handler(METHOD_ERROR);

  size_t num_steps;
  configure_sequence(num_steps, secondaryIndex, sequenceType);
  numApprox = num_steps - 1;
  bool multilev = (sequenceType == Pecos::RESOLUTION_LEVEL_SEQUENCE);
  configure_cost(num_steps, multilev, sequenceCost);
  load_pilot_sample(problem_db.get_sza("method.nond.pilot_samples"),
		    num_steps, pilotSamples);

  size_t max_ps = find_max(pilotSamples);
  if (max_ps) maxEvalConcurrency *= max_ps;
}


NonDNonHierarchSampling::~NonDNonHierarchSampling()
{ }


/*
bool NonDNonHierarchSampling::resize()
{
  bool parent_reinit_comms = NonDSampling::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}
*/


void NonDNonHierarchSampling::pre_run()
{
  NonDEnsembleSampling::pre_run();

  // prefer MF over ML if both available
  iteratedModel.multifidelity_precedence(true);
  // assign an aggregate model key that persists for core_run()
  bool multilev = (sequenceType == Pecos::RESOLUTION_LEVEL_SEQUENCE);
  assign_active_key(numApprox+1, secondaryIndex, multilev);
}


void NonDNonHierarchSampling::
assign_active_key(size_t num_steps, size_t secondary_index, bool multilev)
{
  // For M-model control variate, select fidelities/resolutions
  Pecos::ActiveKey active_key, truth_key;
  std::vector<Pecos::ActiveKey> approx_keys(numApprox);
  //unsigned short truth_form;  size_t truth_lev;
  if (multilev) {
    unsigned short fixed_form = (secondary_index == SZ_MAX) ?
      USHRT_MAX : secondary_index;
    truth_key.form_key(0, fixed_form, numApprox);
    for (size_t approx=0; approx<numApprox; ++approx)
      approx_keys[approx].form_key(0, fixed_form, approx);
    //truth_form = fixed_form;  truth_lev = numApprox;
  }
  else {
    truth_key.form_key(0, numApprox, secondary_index);
    for (unsigned short approx=0; approx<numApprox; ++approx)
      approx_keys[approx].form_key(0, approx, secondary_index);
    //truth_form = numApprox;  truth_lev = secondary_index;
  }
  active_key.aggregate_keys(truth_key, approx_keys, Pecos::RAW_DATA);
  aggregated_models_mode();
  iteratedModel.active_model_key(active_key); // data group 0
}


void NonDNonHierarchSampling::shared_increment(size_t iter)
{
  if (iter == 0) Cout << "\nNon-hierarchical pilot sample: ";
  else Cout << "\nNon-hierarchical sampling iteration " << iter
	    << ": shared sample increment = ";
  Cout << numSamples << '\n';

  if (numSamples) {
    //aggregated_models_mode();
    //iteratedModel.active_model_key(agg_key);

    activeSet.request_values(1);
    ensemble_sample_increment(iter, numApprox+1); // BLOCK if not shared_approx_increment()  *** TO DO: step value
  }
}


void NonDNonHierarchSampling::shared_approx_increment(size_t iter)
{
  if (iter == 0) Cout << "\nNon-hierarchical approx pilot sample: ";
  else Cout << "\nNon-hierarchical sampling iteration " << iter
	    << ": shared approx sample increment = ";
  Cout << numSamples << '\n';

  if (numSamples) {
    //aggregated_models_mode();
    //iteratedModel.active_model_key(agg_key);

    size_t approx_qoi = numApprox  * numFunctions,
                  end = approx_qoi + numFunctions;
    activeSet.request_values(1, 0,   approx_qoi); // all approx QoI
    activeSet.request_values(0, approx_qoi, end); //   no truth QoI

    ensemble_sample_increment(iter, numApprox); // BLOCK  *** TO DO: step value
  }
}


void NonDNonHierarchSampling::
ensemble_sample_increment(size_t iter, size_t step)
{
  // generate new MC parameter sets
  get_parameter_sets(iteratedModel);// pull dist params from any model

  // export separate output files for each data set:
  if (exportSampleSets) { // for HF+LF models, use the HF tags
    export_all_samples("cv_", iteratedModel.truth_model(), iter, step);
    for (size_t i=0; i<numApprox; ++i)
      export_all_samples("cv_", iteratedModel.surrogate_model(i), iter, step);
  }

  // compute allResponses from allVariables using hierarchical model
  evaluate_parameter_sets(iteratedModel, true, false);
}


void NonDNonHierarchSampling::
mfmc_eval_ratios(const RealMatrix& rho2_LH, const RealVector& cost,
		 SizetArray& model_sequence, RealMatrix& eval_ratios,
		 short& subprob_form, bool for_warm_start)
{
  if (eval_ratios.empty())
    eval_ratios.shapeUninitialized(numFunctions, numApprox);

  // -------------------------------------------------------------
  // Based on rho2_LH sequencing, determine best solution approach
  // -------------------------------------------------------------
  // compute a model sequence sorted by Low-High correlation
  // > rho2, N_L, N_H, {eval,mse}_ratios, etc. are all ordered based on the
  //   user-provided model list ordering
  // > we employ model_sequence to pair approximations using a different order
  //   for computing rho2_diff, cost --> eval_ratios --> mse_ratios, but the
  //   results are indexed by the original approx ordering
  // > control variate compute/apply are per approx, so sequencing not required
  // > approx_increment requires model sequence to define the sample pyramid

  bool ordered = ordered_model_sequence(rho2_LH); // for all QoI and all Approx
  if (ordered) { // not necessary to retain sequencing array
    Cout << "MFMC: model sequence provided is ordered in Low-High correlation "
	 << "for all QoI.\n      Computing standard analytic solution.\n"
	 << std::endl;
    subprob_form = ANALYTIC_SOLUTION;  model_sequence.clear();
  }
  else {
    Cout << "MFMC: model sequence provided is out of order with respect to "
	 << "Low-High\n      correlation for at least one QoI.  Switching to "
	 << "alternate solution.\n";
    subprob_form = //(for_warm_start) ?
      REORDERED_ANALYTIC_SOLUTION; // : N_VECTOR_LINEAR_CONSTRAINT;
  }

  size_t qoi, approx, num_am1 = numApprox - 1;
  Real cost_L, cost_H = cost[numApprox]; // HF cost
  switch (subprob_form) {
  case ANALYTIC_SOLUTION: { // standard approach for well-ordered models
    RealVector factor(numFunctions, false);
    for (qoi=0; qoi<numFunctions; ++qoi)
      factor[qoi] = cost_H / (1. - rho2_LH(qoi, num_am1));
    for (approx=0; approx<numApprox; ++approx) {
      Real*   eval_ratios_a = eval_ratios[approx];
      const Real* rho2_LH_a =     rho2_LH[approx];
      cost_L                =        cost[approx];
      // NOTE: indexing is reversed from Peherstorfer (HF = 1, MF = 2, LF = 3)
      // > becomes Approx LF = 0 and MF = 1, Truth HF = 2
      // > i+1 becomes i-1 and most correlated ref is rho2_LH(qoi, num_am1)
      if (approx)
	for (qoi=0; qoi<numFunctions; ++qoi)
	  eval_ratios_a[qoi] = std::sqrt(factor[qoi] / cost_L *
	    (rho2_LH_a[qoi] - rho2_LH(qoi, approx-1)));
      else // rho2_LH for approx-1 (non-existent model) is zero
	for (qoi=0; qoi<numFunctions; ++qoi)
	  eval_ratios_a[qoi] = std::sqrt(factor[qoi] / cost_L * rho2_LH_a[qoi]);
    }
    break;
  }
  case REORDERED_ANALYTIC_SOLUTION: {
    // employ a single model reordering that is shared across the QoI
    RealVector avg_rho2_LH;  average(rho2_LH, 0, avg_rho2_LH); // avg over QoI
    ordered = ordered_model_sequence(avg_rho2_LH, model_sequence);
    // Note: even if avg_rho2_LH is now ordered, rho2_LH is not for all QoI,
    //       so we stick with this alternate formulation.
    if (ordered)
      Cout << "MFMC: averaged correlations are now ordered.\n" << std::endl;
    else
      Cout << "MFMC: reordered approximation model sequence (low to high):\n"
	   << model_sequence << std::endl;

    // precompute a factor based on most-correlated model
    size_t most_corr = (ordered) ? num_am1 : model_sequence[num_am1];  int i;
    Real rho2, prev_rho2, rho2_diff, r_i, prev_ri,
      factor = cost_H / (1. - avg_rho2_LH[most_corr]);// most correlated
    // Compute averaged eval_ratios using averaged rho2 for model_sequence
    RealVector r_unconstrained(numApprox, false);
    for (i=0; i<numApprox; ++i) {
      approx = (ordered) ? i : model_sequence[i];
      cost_L = cost[approx];
      // NOTE: indexing is inverted from Peherstorfer: HF = 1, MF = 2, LF = 3
      // > i+1 becomes i-1 and most correlated is rho2_LH(qoi, most_corr)
      rho2_diff = rho2  = avg_rho2_LH[approx];
      if (i) rho2_diff -= prev_rho2;
      r_unconstrained[i] = std::sqrt(factor / cost_L * rho2_diff);
      prev_rho2 = rho2;
    }
    // Reverse loop order and enforce monotonicity in reordered r_i
    prev_ri = 1.;
    for (i=numApprox-1; i>=0; --i) {
      r_i = std::max(r_unconstrained[i], prev_ri);
      approx = (ordered) ? i : model_sequence[i];
      Real* eval_ratios_a = eval_ratios[approx];
      for (qoi=0; qoi<numFunctions; ++qoi)
	eval_ratios_a[qoi] = r_i; // eval_ratios shared across QoI
      prev_ri = r_i;
    }
    break;
  }
  case N_VECTOR_LINEAR_CONSTRAINT: {
    Cerr << "MFMC: N_VECTOR_LINEAR_CONSTRAINT not yet supported. Coming soon!"
	 << std::endl;
    abort_handler(METHOD_ERROR);
    break;
  }
  }
}


/*
void NonDNonHierarchSampling::print_results(std::ostream& s, short results_state)
{
  if (statsFlag) {
    // Alternate print_results() to eliminate inflate_final_samples()
    print_multilevel_evaluation_summary(s, N_L);
    print_multilevel_evaluation_summary(s, N_H);
    //print_multilevel_evaluation_summary(s, NLev);
    s << "<<<<< Equivalent number of high fidelity evaluations: "
      << equivHFEvals << "\n\nStatistics based on multilevel sample set:\n";

  //print_statistics(s);
    print_moments(s, "response function",
		  iteratedModel.truth_model().response_labels());
    archive_moments();
    archive_equiv_hf_evals(equivHFEvals);
  }
}
*/

} // namespace Dakota
