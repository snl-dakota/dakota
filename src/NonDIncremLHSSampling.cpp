/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDIncremLHSSampling
//- Description: Implementation code for the incremental version of LHS class
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDIncremLHSSampling.hpp"
#include "dakota_system_defs.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "ProblemDescDB.hpp"
#include "ParamResponsePair.hpp"
#include "PRPMultiIndex.hpp"
#include "DakotaVariables.hpp"
//#ifdef HAVE_BOOST
#include "pecos_data_types.hpp"
/*
#elif HAVE_GSL
#include "gsl/gsl_cdf.h"
#endif
*/
#include "pecos_stat_util.hpp"
#include <algorithm>

//#define DEBUG

static const char rcsId[]="@(#) $Id: NonDIncremLHSSampling.cpp 6715 2010-04-02 21:58:15Z wjbohnh $";

namespace Dakota {
  extern PRPCache data_pairs;

RealArray NonDIncremLHSSampling::rawData;


/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDIncremLHSSampling::
NonDIncremLHSSampling(ProblemDescDB& problem_db, Model& model):
  NonDSampling(problem_db, model),
  varBasedDecompFlag(probDescDB.get_bool("method.variance_based_decomp")),
  previousSamples(probDescDB.get_int("method.previous_samples"))
{
  initialize_random_variable_transformation();
  initialize_random_variable_types(); // x_types only
}


NonDIncremLHSSampling::~NonDIncremLHSSampling()
{ }


bool NonDIncremLHSSampling::rank_sort(const int& x, const int& y)
{ return rawData[x]<rawData[y]; }


/** Generate incremental samples.  Loop over the set of samples and 
    compute responses.  Compute statistics on the set of responses 
    if statsFlag is set. */
void NonDIncremLHSSampling::quantify_uncertainty()
{
  // Capture any run-time updates for x-space distributions
  initialize_random_variable_parameters();

  // Note: incremental LHS does not support variance based decomposition

#ifdef DEBUG
  Cout << "lhs1 test sample type " << submethod_enum_to_string(sampleType)
       << '\n';
#endif //DEBUG

  if (sampleType == SUBMETHOD_INCREMENTAL_LHS &&
      previousSamples*2 != numSamples) {
    Cout << "Error: For incremental LHS sampling, the number of samples "
 	 << "must double each time.  The number of samples must be 2 times " 
         << "the number of previous samples." << std::endl;
    abort_handler(-1);
  }

  if (numDiscreteIntVars || numDiscreteStringVars || numDiscreteRealVars) {
    Cout << "Error: Discrete distribution types are not "
 	 << "supported in incremental LHS sampling.  " << std::endl;
    abort_handler(-1);
  }
  
  numSamples = previousSamples;
  sampleRanksMode = GET_RANKS;

  //NOW:  instead of getting the original parameter sets by calling 
  //LHS with the seed, we read it from the restart file.

  // Now read in variable and response data
  size_t num_evals = data_pairs.size();
  if (num_evals > numSamples){
    Cout << "Warning: The number of function evaluations in the restart " 
         << "file does not equal the number of previous samples.  "  
         << "Only the previous number of samples will be taken. \n";
  }          
  else if (num_evals < numSamples) {
    Cerr << "Error: Number of function evaluations in the restart file "
	 << "is less than the number of previous samples.  Incremental " 
	 << "sampling cannot proceed."  << std::endl;
    abort_handler(-1);
  }

  size_t v, s;
  RealVectorArray sample_values_first(numSamples); // views OK
  // Restart records have negative eval IDs in the cache, so walk them
  // in reverse eval ID order (-1 to -numSamples).  May not work in
  // context of partial or concatenated restart.
  PRPCacheCRevIter prp_rev_iter = data_pairs.rbegin();
  for (s=0; s<numSamples; ++s, ++prp_rev_iter)
    sample_values_first[s] = prp_rev_iter->variables().continuous_variables();
#ifdef DEBUG
  Cout << "\nsample1\n" << sample_values_first << '\n';
#endif

  if (sampleType == SUBMETHOD_INCREMENTAL_RANDOM) {
    // for random sampling, obtain a new set of samples of the full size
    numSamples = samplesRef;
    varyPattern = true; 
    get_parameter_sets(iteratedModel);
  }
  else if (sampleType == SUBMETHOD_INCREMENTAL_LHS) {
    // for LHS, determine rank of current sample, rank of second "fill in" 
    // sample, and ranks of combined sample
    IntArray rank_col(numSamples), final_rank(numSamples);
    rawData.resize(numSamples);
    RealMatrix sample_ranks_first(numContinuousVars, numSamples, false);
    
    // store sample values and ranks returned by 1st get_parameter_sets() call
    for (v=0; v<numContinuousVars; ++v) {
      for (size_t rank_count = 0; rank_count < numSamples; rank_count++){
        rank_col[rank_count] = rank_count;
        rawData[rank_count] = sample_values_first[rank_count][v];
      }
      std::sort(rank_col.begin(), rank_col.end(), rank_sort);
      for (s=0; s<numSamples; ++s) {
        final_rank[rank_col[s]] = s;
        sample_ranks_first(v, s) = (Real)final_rank[s] + 1.;
      }
    }

#ifdef DEBUG
    Cout << "rank1\n" << sample_ranks_first << '\n';
#endif //DEBUG

    // initialize the matrix which identifies whether the sample 2 rank should
    // be switched or not
    BoolDequeArray switch_ranks(numContinuousVars);
    for (v=0; v<numContinuousVars; ++v)
      switch_ranks[v] = BoolDeque(numSamples, false);

    const std::vector<Pecos::RandomVariable>& x_ran_vars
      = natafTransform.x_random_variables();
    for (v=0; v<numContinuousVars; ++v) {
      const Pecos::RandomVariable& rv = x_ran_vars[v];
      for (s=0; s<numSamples; ++s) {
	Real val = sample_values_first[s][v], rank = sample_ranks_first(v,s);
	if ( rv.cdf(val) < (rank - .5)/numSamples ) {
	  int index = static_cast<int>(rank) - 1;
	  switch_ranks[v][index] = true;
	}
      }
    }
#ifdef DEBUG
    Cout << "switch rank1\n" << switch_ranks << '\n';
#endif //DEBUG

    varyPattern = true;
    //numSamples = previousSamples;
    //sampleRanksMode = GET_RANKS;
    get_parameter_sets(iteratedModel);
    get_parameter_sets(iteratedModel);
    // store sample ranks returned by 2nd get_parameter_sets() call
    RealMatrix sample_ranks_second = sampleRanks;

#ifdef DEBUG
    Cout << "lhs2 test\nsample2\n"; write_data(Cout, allSamples, false);
    Cout << "rank2\n" << sample_ranks_second;
#endif // DEBUG

    // calculate the combined ranks  
    sampleRanks.shapeUninitialized(numContinuousVars, samplesRef);
    for (s=0; s<numSamples; ++s)
      for (v=0; v<numContinuousVars; ++v) {
	Real rank = sample_ranks_first(v,s);
	int index = static_cast<int>(rank) - 1;
	sampleRanks(v,s) = (switch_ranks[v][index]) ? 2.*rank-1. : 2.*rank;
      }
    for (s=numSamples; s<samplesRef; ++s)
      for (v=0; v<numContinuousVars; ++v) {
	Real rank = sample_ranks_second(v,s-numSamples);
	int index = static_cast<int>(rank)-1;
	sampleRanks(v,s) = (switch_ranks[v][index]) ? 2.*rank : 2.*rank-1.;
      }

#ifdef DEBUG
    Cout << "\ncombined ranks\n" << sampleRanks;
#endif //DEBUG

    // send LHS the full sampleRanks matrix for the combined set
    // and get back a sample that maintains the structure   
    numSamples = samplesRef;
#ifdef DEBUG
    sampleRanksMode = SET_GET_RANKS;
#else
    sampleRanksMode = SET_RANKS;
#endif //DEBUG
    get_parameter_sets(iteratedModel); // using sampleRanks as input

#ifdef DEBUG
    Cout << "rank_combined\n" << sampleRanks << '\n';// updated by SET_GET_RANKS
#endif //DEBUG
  }

  // need to put the first set back  
  for (s=0; s<previousSamples; ++s)
    copy_data(sample_values_first[s], allSamples[s], (int)numContinuousVars);
  
#ifdef DEBUG
  numSamples = samplesRef;
  Cout <<"\nallSamples\n";
  write_data(Cout, allSamples, false, true, true);
#endif //DEBUG

  // evaluate full parameter set of size samplesRef, where the first half
  // should be intercepted via restart file duplication detection
  evaluate_parameter_sets(iteratedModel, true, false);
  compute_statistics(allSamples, allResponses);
}


void NonDIncremLHSSampling::print_results(std::ostream& s)
{
  if (statsFlag && !varBasedDecompFlag) {
    s << "\nStatistics based on " << numSamples << " samples:\n";
    print_statistics(s);
  }
}

} // namespace Dakota
