/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDInterval
//- Description: Implementation code for NonDInterval class
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDInterval.hpp"
#include "dakota_data_types.hpp"
#include "dakota_system_defs.hpp"
//#include "DakotaResponse.hpp"
#include "ProblemDescDB.hpp"
#include "NonDLHSSampling.hpp"
#include "pecos_stat_util.hpp"

//#define DEBUG

static const char rcsId[] = "@(#) $Id: NonDInterval.cpp 6080 2009-09-08 19:03:20Z gtang $";

namespace Dakota {

NonDInterval::NonDInterval(ProblemDescDB& problem_db, Model& model):
  NonD(problem_db, model),
  singleIntervalFlag(methodName ==  LOCAL_INTERVAL_EST ||
		     methodName == GLOBAL_INTERVAL_EST)
{
  bool err_flag = false;

  // initialize finalStatistics using non-default definition (there is no mean
  // or standard deviation and each level mapping involves lower/upper bounds).
  initialize_final_statistics();
	
  if (singleIntervalFlag) {
    if (totalLevelRequests) {
      Cerr << "Error: level mappings not supported in NonDInterval single "
	   << "interval mode." << std::endl;
      err_flag = true;
    }
  }
  else {
    // reliability_levels not currently supported, but could be
    if (!probDescDB.get_rva("method.nond.reliability_levels").empty()) {
      Cerr << "Error: reliability_levels not supported in NonDInterval "
	   << "evidence mode." << std::endl;
      err_flag = true;
    }

    // size output arrays.  Note that for each response or probability level
    // request, we get two values from the belief and plausibility functions.
    computedRespLevels.resize(numFunctions);
    computedProbLevels.resize(numFunctions);
    computedGenRelLevels.resize(numFunctions);
    size_t i, j;
    for (i=0; i<numFunctions; ++i) {
      size_t rl_len = requestedRespLevels[i].length(), pl_gl_len
	= requestedProbLevels[i].length() + requestedGenRelLevels[i].length();
      computedRespLevels[i].resize(2*pl_gl_len);
      if (respLevelTarget == PROBABILITIES)
	computedProbLevels[i].resize(2*rl_len);
      else
	computedGenRelLevels[i].resize(2*rl_len);
    }
  }

  if (err_flag)
    abort_handler(-1);
}


NonDInterval::~NonDInterval()
{}

bool NonDInterval::resize()
{
  bool parent_reinit_comms = NonD::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}


void NonDInterval::initialize_final_statistics()
{
  size_t num_final_stats = (singleIntervalFlag) ?
    2*numFunctions : 2*totalLevelRequests;
  // default response ASV/DVV may be overridden by NestedModel update
  // in subIterator.response_results_active_set(sub_iterator_set)
  ActiveSet stats_set(num_final_stats);
  stats_set.derivative_vector(iteratedModel.inactive_continuous_variable_ids());
  finalStatistics = Response(SIMULATION_RESPONSE, stats_set);

  // Assign meaningful fn labels to final stats (appear in NestedModel output)
  size_t i, j, num_levels, cntr = 0;
  StringArray stats_labels(num_final_stats);
  if (singleIntervalFlag) {
    const StringArray& fn_labels = iteratedModel.response_labels();
    for (i=0; i<numFunctions; ++i) {
      stats_labels[cntr++] = fn_labels[i] + String("_min");
      stats_labels[cntr++] = fn_labels[i] + String("_max");
    }
  }
  else {
    char tag_string[10], lev_string[15];
    for (i=0; i<numFunctions; ++i) {
      std::sprintf(tag_string, "_r%i", i+1);
      num_levels = requestedRespLevels[i].length();
      for (j=0; j<num_levels; ++j) {
	stats_labels[cntr] = (cdfFlag) ? String("cdf") : String("ccdf");
	if (respLevelTarget == PROBABILITIES)
	  std::sprintf(lev_string, "_plev%i", j+1);
	else
	  std::sprintf(lev_string, "_b*lev%i", j+1);
	stats_labels[cntr] +=
	  String("_bel") + String(lev_string) + String(tag_string);
	cntr++;
	stats_labels[cntr] +=
	  String("_pls") + String(lev_string) + String(tag_string);
	cntr++;
      }
      num_levels = requestedProbLevels[i].length() +
	requestedGenRelLevels[i].length();
      for (j=0; j<num_levels; ++j) {
	stats_labels[cntr] = (cdfFlag) ? String("cdf") : String("ccdf");
	std::sprintf(lev_string, "_zlev%i", j+1);
	stats_labels[cntr] +=
	  String("_bel") + String(lev_string) + String(tag_string);
	cntr++;
	stats_labels[cntr] +=
	  String("_pls") + String(lev_string) + String(tag_string);
	cntr++;
      }
    }
  }
  finalStatistics.function_labels(stats_labels);
}


void NonDInterval::calculate_cells_and_bpas()
{
  Pecos::EpistemicDistParams& edp
    = iteratedModel.epistemic_distribution_parameters();
  // Information we want: for each hyper cube i, give the bpa, and bounds on i.
  // ci_bpa[i][j] gives jth bpa of jth interval of ith variable
  // ci_{l,u}_bnds[i][j] gives jth {lower,upper} bound for the ith variable
  const RealRealPairRealMapArray& ci_bpa
    = edp.continuous_interval_basic_probabilities();
  const IntIntPairRealMapArray& di_bpa
    = edp.discrete_interval_basic_probabilities();
  const IntRealMapArray& dsi_vals_probs
    = edp.discrete_set_int_values_probabilities();
  const RealRealMapArray& dsr_vals_probs
    = edp.discrete_set_real_values_probabilities();

  size_t i, j, k, var_cntr, cell_cntr, prev_bpa_len;
  int num_total_vars = numContIntervalVars  + numDiscIntervalVars
                     + numDiscSetIntUncVars + numDiscSetRealUncVars;

  UShortArray scale_factor(num_total_vars, 1);
  numCells = 1;

  // continuous interval variables
  for (i=0, var_cntr=0; i<numContIntervalVars; ++i, ++var_cntr) {
    if (var_cntr)
      scale_factor[i] = scale_factor[i-1] * prev_bpa_len;
    numCells *= prev_bpa_len = ci_bpa[i].size();
  }

  // discrete interval variables
  for (i=0; i<numDiscIntervalVars; ++i, ++var_cntr) {
    if (var_cntr)
      scale_factor[var_cntr] = scale_factor[var_cntr-1] * prev_bpa_len;
    numCells *= prev_bpa_len = di_bpa[i].size();
  }

  // discrete interval sets
  for (i=0; i<numDiscSetIntUncVars; ++i, ++var_cntr) {
    if (var_cntr)
      scale_factor[var_cntr] = scale_factor[var_cntr-1] * prev_bpa_len;
    numCells *= prev_bpa_len = dsi_vals_probs[i].size();
  }
  
  // discrete real sets
  for (i=0; i<numDiscSetRealUncVars; ++i, ++var_cntr){
    if (var_cntr)
      scale_factor[var_cntr] = scale_factor[var_cntr-1] * prev_bpa_len;
    numCells *= prev_bpa_len = dsr_vals_probs[i].size();
  }
 
  if (outputLevel > NORMAL_OUTPUT)
    Cout << "scale factor:\n" << scale_factor
	 << "prev_bpa_len = " << prev_bpa_len
	 << ", numCells = "   << numCells << '\n';

  // shape cell length
  if (numContIntervalVars) {
    cellContLowerBounds.resize(numCells);
    cellContUpperBounds.resize(numCells);
  }
  if (numDiscIntervalVars) {
    cellIntRangeLowerBounds.resize(numCells);
    cellIntRangeUpperBounds.resize(numCells);
  }
  if (numDiscSetIntUncVars)
    cellIntSetBounds.resize(numCells);
  if (numDiscSetRealUncVars)
    cellRealSetBounds.resize(numCells);
  
  cellBPA.sizeUninitialized(numCells); cellBPA = 1.;
  for (i=0; i<numCells; ++i) {
    if (numContIntervalVars) {
      cellContLowerBounds[i].resize(numContIntervalVars);
      cellContUpperBounds[i].resize(numContIntervalVars);
    }
    if (numDiscIntervalVars) {
      cellIntRangeLowerBounds[i].resize(numDiscIntervalVars);
      cellIntRangeUpperBounds[i].resize(numDiscIntervalVars);
    }
    if (numDiscSetIntUncVars)
      cellIntSetBounds[i].resize(numDiscSetIntUncVars);
    if (numDiscSetRealUncVars)
      cellRealSetBounds[i].resize(numDiscSetRealUncVars);
  }

  // This loops num_variables*num_cells
  Real lower_bound_i_of_j, upper_bound_i_of_j;
  int intervals_in_var_j;

  for (j=0, var_cntr=0; j<numContIntervalVars; ++j, ++var_cntr) {
    const RealRealPairRealMap& ci_bpa_j = ci_bpa[j];
    RRPRMCIter cit = ci_bpa_j.begin();
    intervals_in_var_j = ci_bpa_j.size();
    for (i=0; i<intervals_in_var_j; ++i, ++cit) {
      const RealRealPair& bnds = cit->first;
      Real l_bnd = bnds.first, u_bnd = bnds.second, p = cit->second;
      cell_cntr = i * scale_factor[var_cntr];
      while (cell_cntr < numCells) {
	for (k=0; k<scale_factor[var_cntr]; k++) {
	  cellContLowerBounds[cell_cntr+k][j] = l_bnd;
	  cellContUpperBounds[cell_cntr+k][j] = u_bnd;
	  cellBPA[cell_cntr+k] *= p;
	}
	cell_cntr += intervals_in_var_j * scale_factor[var_cntr]; 
      }
    }
  }

  for (j=0; j<numDiscIntervalVars; ++j, ++var_cntr) {
    const IntIntPairRealMap& di_bpa_j = di_bpa[j];
    IIPRMCIter cit = di_bpa_j.begin();
    intervals_in_var_j = di_bpa_j.size();
    for (i=0; i<intervals_in_var_j; ++i, ++cit) {
      const IntIntPair& bnds = cit->first;
      int l_bnd = bnds.first, u_bnd = bnds.second; Real p = cit->second;
      cell_cntr = i * scale_factor[var_cntr];
      while (cell_cntr < numCells) {
	for (k=0; k<scale_factor[var_cntr]; k++) {
	  cellIntRangeLowerBounds[cell_cntr+k][j] = l_bnd;
	  cellIntRangeUpperBounds[cell_cntr+k][j] = u_bnd;
	  cellBPA[cell_cntr+k] *= p;
	}
	cell_cntr += intervals_in_var_j * scale_factor[var_cntr]; 
      }
    }
  }

  for (j=0; j<numDiscSetIntUncVars; ++j, ++var_cntr) {
    intervals_in_var_j = dsi_vals_probs[j].size();
    IRMCIter cit = dsi_vals_probs[j].begin();
    for (i=0; i<intervals_in_var_j; ++i, ++cit) {
      int val = cit->first; Real p = cit->second;
      cell_cntr = i*scale_factor[var_cntr];
      while (cell_cntr < numCells) {
	for (k=0; k<scale_factor[var_cntr]; k++) {
	  cellIntSetBounds[cell_cntr+k][j] = val;
	  cellBPA[cell_cntr+k] *= p;
	}
	cell_cntr += intervals_in_var_j * scale_factor[var_cntr]; 
      }
    }
  }

  for (j=0; j<numDiscSetRealUncVars; ++j, ++var_cntr) {
    intervals_in_var_j = dsr_vals_probs[j].size();
    RRMCIter cit = dsr_vals_probs[j].begin();
    for (i=0; i<intervals_in_var_j; ++i, ++cit) {
      Real val = cit->first, p = cit->second;
      cell_cntr = i*scale_factor[var_cntr];
      while (cell_cntr < numCells) {
	for (k=0; k<scale_factor[var_cntr]; k++) {
	  cellRealSetBounds[cell_cntr+k][j] = val;
	  cellBPA[cell_cntr+k] *= p;
	}
	cell_cntr += intervals_in_var_j * scale_factor[var_cntr]; 
      }
    }
  }

  StringMultiArrayConstView cv_labels
    = iteratedModel.continuous_variable_labels();
  StringMultiArrayConstView div_labels
    = iteratedModel.discrete_int_variable_labels();
  //StringMultiArrayConstView dsv_labels
  //  = iteratedModel.discrete_string_variable_labels();
  StringMultiArrayConstView drv_labels
    = iteratedModel.discrete_real_variable_labels();
  for (i=0; i<numCells; ++i) {
    Cout << "Cell " << i+1 << ":\n";
    for (j=0; j<numContIntervalVars; ++j)
      Cout << cv_labels[j] << ": [ " << cellContLowerBounds[i][j] << ", "
	   << cellContUpperBounds[i][j] << " ]\n";
    for (j=0; j<numDiscIntervalVars; ++j)
      Cout << div_labels[j] << ": [ " << cellIntRangeLowerBounds[i][j] << ", "
	   << cellIntRangeUpperBounds[i][j] << " ]\n";
    for (j=0; j<numDiscSetIntUncVars; ++j)
      Cout  << div_labels[j+numDiscIntervalVars] << ": [ "
	    << cellIntSetBounds[i][j] << " ]\n";
    for (j=0; j<numDiscSetRealUncVars; ++j)
      Cout  << drv_labels[j] << ": [ " << cellRealSetBounds[i][j] << " ]\n";
  }

  // shape belief/plausibility structure arrays
  ccBelFn.resize(numFunctions);
  ccPlausFn.resize(numFunctions);
  ccBelVal.resize(numFunctions);
  ccPlausVal.resize(numFunctions);
  // shape cell min/max arrays for each response
  cellFnUpperBounds.resize(numFunctions);
  cellFnLowerBounds.resize(numFunctions);
  // shape individual vectors
  for (i=0; i<numFunctions; ++i) {
    ccBelFn[i].resize(numCells);
    ccPlausFn[i].resize(numCells);
    ccBelVal[i].resize(numCells);
    ccPlausVal[i].resize(numCells);
    cellFnUpperBounds[i].resize(numCells);
    cellFnLowerBounds[i].resize(numCells);
  }
}


// GT: Attempts to replace CCBFPF_F77
void NonDInterval::calculate_cbf_cpf(bool complementary)
{
  // In order to obtain the CBF, sort the maximum values in ascending order
  // sum up the BPAs, in that order; corresponding max value is response level
  // To obtain the CPF, sort all the min values in ascending order
  // sum up the BPAs, in that order; corresponding min value is response level
  // Similar logic for CCBF and CCPF

  // Because cellFn values are of RealVector type, use sorted
  // containers instead of STL algorithmic sort
  std::multimap<Real, size_t> cell_min;
  std::multimap<Real, size_t> cell_max;
  std::multimap<Real, size_t>::iterator it_max, it_min;
  size_t i;
  // Insert into a map and sort by key (i.e. function value)
  Real bpa_sum = 0.;
  RealVector& cell_fn_lb = cellFnLowerBounds[respFnCntr];
  RealVector& cell_fn_ub = cellFnUpperBounds[respFnCntr];
  for (i=0; i<numCells; ++i) {
    bpa_sum += cellBPA[i];
    cell_min.insert(std::pair<Real, size_t>(cell_fn_lb[i], i));
    cell_max.insert(std::pair<Real, size_t>(cell_fn_ub[i], i));
  }

  Real bel_total, plaus_total;
  RealVector bel_fn(numCells, false),  plaus_fn(numCells, false),
            bel_val(numCells, false), plaus_val(numCells, false);
  // if CCBF/CCPF desired
  if (complementary) {
    bel_total = bpa_sum; plaus_total = bpa_sum;
    for (it_max  = cell_max.begin(), it_min  = cell_min.begin(), i=0; 
	 it_max != cell_max.end() && it_min != cell_min.end() && i<numCells;
	 ++it_max, ++it_min, ++i) {
      bel_fn[i]    = bel_total;
      plaus_fn[i]  = plaus_total;
      bel_val[i]   = it_min->first;
      plaus_val[i] = it_max->first;

#ifdef DEBUG
      Cout << "(response_level,belief)\t( " << bel_val[i] << ", " << bel_fn[i]
	   << ")\n(response_level,plausibility)\t( "
	   << plaus_val[i] << ", " << plaus_fn[i] << ")\n";
#endif

      bel_total   -= cellBPA[it_min->second];
      plaus_total -= cellBPA[it_max->second];
    }
  }
  // if CBF/CPF desired
  else {
    bel_total = plaus_total = 0.;
    for (it_max  = cell_max.begin(), it_min  = cell_min.begin(), i=0; 
	 it_max != cell_max.end() && it_min != cell_min.end() && i<numCells;
	 ++it_max, ++it_min, ++i) {
      bel_total   += cellBPA[it_max->second];
      plaus_total += cellBPA[it_min->second];
      bel_fn[i]    = bel_total;
      plaus_fn[i]  = plaus_total;
      bel_val[i]   = it_max->first;
      plaus_val[i] = it_min->first;

#ifdef DEBUG
      Cout << "(response_level,belief)\t( " << bel_val[i] << ", " << bel_fn[i]
	   << ")\n(response_level,plausibility)\t( "
	   << plaus_val[i]  << ", " << plaus_fn[i] << ")\n";
#endif
    }
  }

  // now copy back into member variables
  ccBelFn[respFnCntr]    = bel_fn;
  ccPlausFn[respFnCntr]  = plaus_fn;
  ccBelVal[respFnCntr]   = bel_val;
  ccPlausVal[respFnCntr] = plaus_val;
}

 
void NonDInterval::compute_evidence_statistics()
{
  // > CDF/CCDF mappings of probability/reliability levels to response levels

  int i, j, bel_index, plaus_index, cntr = 0;
#ifdef DEBUG 
  for (i=0; i<numFunctions; ++i) {
    Cout << "Function number " << i << '\n';
  for (j=0; j<numCells; ++j)
    Cout << "CCBF " << ccBelFn[i][j] << "value "
	 << ccBelVal[i][j] << "CCPF "
	 << ccPlausFn[i][j] << "value " << ccPlausVal[i][j] << '\n';
  }
#endif //DEBUG

  // CDF/CCDF mappings of response levels to belief and plaus: z -> belief/plaus
  for (i=0; i<numFunctions; ++i) {
    size_t rl_len = requestedRespLevels[i].length(),
           pl_len = requestedProbLevels[i].length(),
           gl_len = requestedGenRelLevels[i].length(),
        pl_gl_len = pl_len + gl_len;

    for (j=0; j<rl_len; ++j) { // z -> belief/plausibility
      Real z = requestedRespLevels[i][j];
      // z -> belief (based on CCBF)
      bel_index = 0;
      Real prob;
      if (cdfFlag) {
        while (z > ccPlausVal[i][bel_index] && bel_index < numCells-1)
	  bel_index++;
        prob = (z > ccPlausVal[i][numCells-1]) ? 1. : 
	  1.-ccPlausFn[i][bel_index];
      }
      else {
        while (z > ccBelVal[i][bel_index] && bel_index < numCells-1)
	  bel_index++;
	prob = (z > ccBelVal[i][numCells-1]) ? 0. : ccBelFn[i][bel_index];
      }
      if (respLevelTarget == PROBABILITIES) {
	computedProbLevels[i][j] = prob;
	finalStatistics.function_value(prob, cntr++);
      }
      else {
	Real beta;
	if (prob < Pecos::SMALL_NUMBER) // see p=0,p=1 assignments above
	  beta =  Pecos::LARGE_NUMBER; // Phi(-Inf) = 0
	else if (1.- prob < Pecos::SMALL_NUMBER)
	  beta = -Pecos::LARGE_NUMBER; // Phi(+Inf) = 1
	else
	  beta = -Pecos::NormalRandomVariable::inverse_std_cdf(prob);
	computedGenRelLevels[i][j] = beta;
	finalStatistics.function_value(beta, cntr++);
      }
      // z -> plausibility (based on CCPF)
      plaus_index = 0;
      if (cdfFlag) {
        while (z > ccBelVal[i][plaus_index] && plaus_index < numCells-1)
	  plaus_index++;
        prob = (z > ccBelVal[i][numCells-1]) ?
	  1. : 1.-ccBelFn[i][plaus_index];
      }
      else { 
        while (z > ccPlausVal[i][plaus_index] && plaus_index < numCells-1)
	  plaus_index++;
	prob = (z > ccPlausVal[i][numCells-1]) ?
	  0. : ccPlausFn[i][plaus_index];
      }
      if (respLevelTarget == PROBABILITIES) {
	computedProbLevels[i][j+rl_len] = prob;
	finalStatistics.function_value(prob, cntr++);
      }
      else {
	Real beta;
	if (prob < Pecos::SMALL_NUMBER) // see p=0,p=1 assignments above
	  beta =  Pecos::LARGE_NUMBER; // Phi(-Inf) = 0
	else if (1.- prob < Pecos::SMALL_NUMBER)
	  beta = -Pecos::LARGE_NUMBER; // Phi(+Inf) = 1
	else
	  beta = -Pecos::NormalRandomVariable::inverse_std_cdf(prob);
	computedGenRelLevels[i][j+rl_len] = beta;
	finalStatistics.function_value(beta, cntr++);
      }
    }

    for (j=0; j<pl_gl_len; ++j) { // belief/plausibility -> z
      Real cc_prob;
      if (j<pl_len)
	cc_prob = (cdfFlag) ? 1. - requestedProbLevels[i][j] :
	  requestedProbLevels[i][j];
      else
	cc_prob = (cdfFlag) ?
	  Pecos::NormalRandomVariable::std_cdf(requestedGenRelLevels[i][j]) :
	  Pecos::NormalRandomVariable::std_ccdf(requestedGenRelLevels[i][j]);
      // belief -> z (based on CCBF)
      bel_index = 0;
      if (cdfFlag) {
        while (cc_prob < ccPlausFn[i][bel_index] && bel_index < numCells-1)
	  bel_index++;
        const Real& z = computedRespLevels[i][j] = ccPlausVal[i][bel_index]; 
	finalStatistics.function_value(z, cntr++);
      } 
      else {
        while (cc_prob < ccBelFn[i][bel_index] && bel_index < numCells-1)
	  bel_index++;
        const Real& z = computedRespLevels[i][j] = ccBelVal[i][bel_index]; 
	finalStatistics.function_value(z, cntr++);
      }
      // plausibility -> z (based on CCPF)
      plaus_index = 0;
      if (cdfFlag) {
        while (cc_prob <ccBelFn[i][plaus_index] && plaus_index < numCells-1)
	  plaus_index++;
        const Real& z = computedRespLevels[i][j+pl_gl_len]
	  = ccBelVal[i][plaus_index];
	finalStatistics.function_value(z, cntr++);
      }
      else {
        while (cc_prob < ccPlausFn[i][plaus_index] &&
	       plaus_index < numCells-1)
	  plaus_index++;
        const Real& z = computedRespLevels[i][j+pl_gl_len]
	  = ccPlausVal[i][plaus_index];
	finalStatistics.function_value(z, cntr++);
      }
    }
  }
}


void NonDInterval::print_results(std::ostream& s)
{
  const StringArray& fn_labels = iteratedModel.response_labels();
  s << "------------------------------------------------------------------\n"
    << std::scientific << std::setprecision(write_precision);

  if (singleIntervalFlag) {
    // output response intervals
    s << "Min and Max estimated values for each response function:\n";
    size_t i, stat_cntr = 0;
    const RealVector& final_stats = finalStatistics.function_values();
    for (i=0; i<numFunctions; ++i) {
      s << fn_labels[i] << ":  Min = " << final_stats[stat_cntr]; ++stat_cntr;
      s <<  "  Max = " << final_stats[stat_cntr] << '\n';         ++stat_cntr;
    }
  }
  else {
    // output CBF/CCBF and CPF/CCPF response/probabilities pairs
    s << "\nBelief and Plausibility for each response function:\n";
    size_t i, j;
    for (i=0; i<numFunctions; ++i) {
      size_t rl_len = requestedRespLevels[i].length(),
	     pl_len = requestedProbLevels[i].length(),
	     gl_len = requestedGenRelLevels[i].length(),
          pl_gl_len = pl_len + gl_len;
      if (cdfFlag)
	s << "Cumulative Belief/Plausibility Functions (CBF/CPF) for ";
      else
	s << "Complementary Cumulative Belief/Plausibility Functions "
	  << "(CCBF/CCPF) for ";
      s << fn_labels[i] << ":\n";

//#ifdef DEBUG
      // Print out cell info:
      s << "\n  Basic Prob Assign       Response Min       Response Max    Cell"
	<< "\n  -----------------       ------------       ------------    ----"
	<< '\n';
      for (j=0; j<numCells; ++j)
	s << "  " << std::setw(17) << cellBPA[j]
	  << "  " << std::setw(17) << cellFnLowerBounds[i][j]  
	  << "  " << std::setw(17) << cellFnUpperBounds[i][j] 
	  << "  " << std::setw(6) << j+1 << '\n';
//#endif

      // Print out CBF/CPF
      s << "\n     Response Level             Belief\n"
	<< "     --------------             ------\n";
      for (j=0; j<numCells; ++j)
	s << "  " << std::setw(17) << ccBelVal[i][j] << "  " 
	  << std::setw(17) << ccBelFn[i][j] << '\n';
      s << "\n     Response Level       Plausibility\n"
	<< "     --------------       ------------\n";
      for (j=0; j<numCells; ++j)
	s << "  " << std::setw(17) << ccPlausVal[i][j] << "  " 
	  << std::setw(17) << ccPlausFn[i][j] << '\n';
      s << '\n';

      if (rl_len) {
	if (respLevelTarget == PROBABILITIES) {
	  s << "     Response Level  Belief Prob Level   Plaus Prob Level\n"
	    << "     --------------  -----------------   ----------------\n";
	  for (j=0; j<rl_len; ++j)
	    s << "  " << std::setw(17) << requestedRespLevels[i][j] << "  "
	      << std::setw(17) << computedProbLevels[i][j] << "  " 
	      << std::setw(17) << computedProbLevels[i][j+rl_len] << '\n';
	}
	else {
	  s << "     Response Level Belief Gen Rel Lev  Plaus Gen Rel Lev\n"
	    << "     -------------- ------------------  -----------------\n";
	  for (j=0; j<rl_len; ++j)
	    s << "  " << std::setw(17) << requestedRespLevels[i][j] << "  "
	      << std::setw(17) << computedGenRelLevels[i][j] << "  " 
	      << std::setw(17) << computedGenRelLevels[i][j+rl_len] << '\n';
	}
      }
      if (pl_len) {
	s << "  Probability Level  Belief Resp Level   Plaus Resp Level\n"
	  << "  -----------------  -----------------   ----------------\n";
	for (j=0; j<pl_len; ++j)
	  s << "  " << std::setw(17) << requestedProbLevels[i][j] << "  "
	    << std::setw(17) << computedRespLevels[i][j] << "  " 
	    << std::setw(17) << computedRespLevels[i][j+pl_gl_len] << '\n';
      }
      if (gl_len) {
	s << "  General Rel Level  Belief Resp Level   Plaus Resp Level\n"
	  << "  -----------------  -----------------   ----------------\n";
	for (j=0; j<gl_len; ++j)
	  s << "  " << std::setw(17) << requestedGenRelLevels[i][j] << "  "
	    << std::setw(17) << computedRespLevels[i][j+pl_len] << "  " 
	    << std::setw(17) << computedRespLevels[i][j+pl_len+pl_gl_len]
	    << '\n';
      }
      s << '\n';
    }
  }

  s << "-----------------------------------------------------------------"
    << std::endl;
#ifdef DEBUG
  s << "Final statistics:\n" << finalStatistics;
#endif //DEBUG	
}

} // namespace Dakota
