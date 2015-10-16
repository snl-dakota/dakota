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

  if ( numDiscreteStringVars ) {
    Cout << "Error: Discrete distribution string types are not "
 	 << "supported in incremental LHS sampling.  " << std::endl;
    abort_handler(-1);
  }
  
  numSamples = previousSamples;
  sampleRanksMode = GET_RANKS;

  //NOW: instead of getting the original parameter sets by calling 
  //LHS with the seed, we read it from the restart file.

  // Now read in variable and response data
  size_t num_evals = data_pairs.size();
  if (num_evals > numSamples)
    Cout << "Warning: The number of function evaluations in the restart file ("
	 << num_evals << ") does not equal the number of previous samples ("
	 << numSamples << ").  Only the trailing " << numSamples
	 << " samples will be used. \n";
  else if (num_evals < numSamples) {
    Cerr << "Error: Number of function evaluations in the restart file ("
	 << num_evals << ") is less than the number of previous samples ("
	 << numSamples << ").  Incremental sampling cannot proceed."<<std::endl;
    abort_handler(-1);
  }

  size_t v, s, cntr;
  RealVectorArray sample1_values(numSamples); // views OK
  IntVectorArray sample1int_values(numSamples);
  RealVectorArray sample1real_values(numSamples);
  //std::vector<StringMultiArrayConstView> sample1string_values(numSamples);

  // Restart records have negative eval IDs in the cache, so walk them
  // in reverse eval ID order (-1 to -numSamples).  May not work in
  // context of partial or concatenated restart.
  PRPCacheCRevIter prp_rev_iter = data_pairs.rbegin();
  for (s=0; s<numSamples; ++s, ++prp_rev_iter) {
    sample1_values[s] = prp_rev_iter->variables().continuous_variables();
    sample1int_values[s] = prp_rev_iter->variables().discrete_int_variables();
    sample1real_values[s] = prp_rev_iter->variables().discrete_real_variables();
    //sample1string_values[s] = prp_rev_iter->variables().discrete_string_variables();
  }
#ifdef DEBUG
  Cout << "\nsample1\n" << sample1_values << '\n';
  Cout << "\nsample1int\n" << sample1int_values << '\n';
  Cout << "\nsample1real\n" << sample1real_values << '\n';
  //Cout << "\nsample1string\n" << sample1string_values << '\n';
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
    IntMatrix sample1_ranks(numContinuousVars + numDiscreteIntVars + numDiscreteRealVars, numSamples, false);

    // store sample values and ranks returned by 1st get_parameter_sets() call
    for (v=0; v<numContinuousVars; ++v) {
      for (size_t rank_count = 0; rank_count < numSamples; rank_count++){
        rank_col[rank_count] = rank_count;
        rawData[rank_count] = sample1_values[rank_count][v];
      }
      std::sort(rank_col.begin(), rank_col.end(), rank_sort);
      for (s=0; s<numSamples; ++s)
        final_rank[rank_col[s]] = s;
      for (s=0; s<numSamples; ++s) // can't be combined with loop above
        sample1_ranks(v, s) = final_rank[s] + 1;
    }
    for (v=0; v<numDiscreteIntVars; ++v) {
      for (size_t rank_count = 0; rank_count < numSamples; rank_count++){
        rank_col[rank_count] = rank_count;
        rawData[rank_count] = sample1int_values[rank_count][v];
      }
      std::sort(rank_col.begin(), rank_col.end(), rank_sort);
      for (s=0; s<numSamples; ++s)
        final_rank[rank_col[s]] = s;
      for (s=0; s<numSamples; ++s) // can't be combined with loop above
        sample1_ranks(v+numContinuousVars, s) = final_rank[s] + 1;
    }
    for (v=0; v<numDiscreteRealVars; ++v) {
      for (size_t rank_count = 0; rank_count < numSamples; rank_count++){
        rank_col[rank_count] = rank_count;
        rawData[rank_count] = sample1real_values[rank_count][v];
      }
      std::sort(rank_col.begin(), rank_col.end(), rank_sort);
      for (s=0; s<numSamples; ++s)
        final_rank[rank_col[s]] = s;
      for (s=0; s<numSamples; ++s) // can't be combined with loop above
        sample1_ranks(v+numContinuousVars+numDiscreteIntVars, s) = final_rank[s] + 1;
    }

#ifdef DEBUG
    Cout << "rank1\n" << sample1_ranks << '\n';
#endif //DEBUG

    // initialize the matrix which identifies whether the sample 2 rank should
    // be switched or not.  The switch matrix is only for continuous variables
    BoolDequeArray switch_ranks(numContinuousVars);
    for (v=0; v<numContinuousVars; ++v)
      switch_ranks[v] = BoolDeque(numSamples, false);

    const std::vector<Pecos::RandomVariable>& x_ran_vars
      = natafTransform.x_random_variables();
    for (v=0; v<numContinuousVars; ++v) {
      const Pecos::RandomVariable& rv = x_ran_vars[v];
      for (s=0; s<numSamples; ++s) {
	Real val  = sample1_values[s][v];
	int rank1 = sample1_ranks(v,s), index = rank1 - 1;
	if ( rv.cdf(val) < (Real)(2*rank1 - 1) / (Real)(2*numSamples) )
	  switch_ranks[v][index] = true;
      }
    }
//#ifdef DEBUG
//    Cout << "switch rank1\n" << switch_ranks << '\n';
//#endif //DEBUG

    varyPattern = true;
    //numSamples = previousSamples;
    //sampleRanksMode = GET_RANKS;
    get_parameter_sets(iteratedModel);
    get_parameter_sets(iteratedModel);
    // store sample ranks returned by 2nd get_parameter_sets() call
    int num_samp2 = samplesRef - numSamples;

    IntMatrix sample2_ranks(numContinuousVars+numDiscreteIntVars+numDiscreteRealVars, num_samp2, false);
    for (s=0; s<num_samp2; ++s) {
      int* s2r_s = sample2_ranks[s]; Real* sR_s = sampleRanks[s];
      for (v=0; v<numContinuousVars; ++v)
	s2r_s[v] = (int)std::floor(sR_s[v]+.5); // round to nearest integer
    }
    for (v=0; v<numDiscreteIntVars; ++v) {
      for (size_t rank_count = 0; rank_count < num_samp2; rank_count++){
        rank_col[rank_count] = rank_count;
        rawData[rank_count] = allSamples[rank_count][v+numContinuousVars];
      }
      std::sort(rank_col.begin(), rank_col.end(), rank_sort);
      for (s=0; s<num_samp2; ++s)
        final_rank[rank_col[s]] = s;
      for (s=0; s<num_samp2; ++s) // can't be combined with loop above
        sample2_ranks(v+numContinuousVars, s) = final_rank[s] + 1;
    }
    for (v=0; v<numDiscreteRealVars; ++v) {
      for (size_t rank_count = 0; rank_count < num_samp2; rank_count++){
        rank_col[rank_count] = rank_count;
        rawData[rank_count] = allSamples[rank_count][v+numContinuousVars+numDiscreteIntVars];
      }
      std::sort(rank_col.begin(), rank_col.end(), rank_sort);
      for (s=0; s<num_samp2; ++s)
        final_rank[rank_col[s]] = s;
      for (s=0; s<num_samp2; ++s) // can't be combined with loop above
        sample2_ranks(v+numContinuousVars+numDiscreteIntVars, s) = final_rank[s] + 1;
    }

#ifdef DEBUG
    Cout << "lhs2 test\nsample2\n"; write_data(Cout, allSamples, false);
    Cout << "rank2\n" << sample2_ranks;
#endif // DEBUG

    // calculate the combined ranks  
    sampleRanks.shapeUninitialized(numContinuousVars+numDiscreteIntVars+numDiscreteRealVars, samplesRef);
    for (s=0; s<numSamples; ++s)
      for (v=0; v<numContinuousVars; ++v) {
	int rank1 = sample1_ranks(v,s), index = rank1 - 1,
	  comb_rank = (switch_ranks[v][index]) ? 2*rank1-1 : 2*rank1;
	sampleRanks(v,s) = (Real)comb_rank;
      }
    for (s=numSamples,cntr=0; s<samplesRef; ++s,++cntr)
      for (v=0; v<numContinuousVars; ++v) {
	int rank2 = sample2_ranks(v,cntr), index = rank2 - 1,
	  comb_rank = (switch_ranks[v][index]) ? 2*rank2 : 2*rank2-1;
	sampleRanks(v,s) = (Real)comb_rank;
      }
    // Ranks for discrete variables.  In the discrete case, we don't switch, we just 
    // calculate the overall ranking of the combined set 
    rank_col.resize(samplesRef), final_rank.resize(samplesRef);
    rawData.resize(samplesRef); 
    if (numDiscreteIntVars > 0) {
      for (v=0; v<numDiscreteIntVars; ++v) {
        for (size_t rank_count = 0; rank_count < numSamples; rank_count++){
          rank_col[rank_count] = rank_count;
          rawData[rank_count] = sample1int_values[rank_count][v];
        }
        for (size_t rank_count = numSamples ; rank_count < samplesRef; rank_count++){
          rank_col[rank_count] = rank_count;
          rawData[rank_count] = allSamples[rank_count-numSamples][v+numContinuousVars];
        }
        std::sort(rank_col.begin(), rank_col.end(), rank_sort);
        for (s=0; s<samplesRef; ++s)
          final_rank[rank_col[s]] = s+1;
        Cout << "final ranks " << final_rank << '\n';
        Cout << "rawData " << rawData << '\n'; 
        //combine_ranks_discrete(ave_ranks, sample1int_values);
        for (s=0; s<samplesRef; ++s) // can't be combined with loop above
          sampleRanks(v+numContinuousVars, s) = final_rank[s];
      }
    }
    if (numDiscreteRealVars > 0) {
      for (v=0; v<numDiscreteRealVars; ++v) {
        for (size_t rank_count = 0; rank_count < numSamples; rank_count++){
          rank_col[rank_count] = rank_count;
          rawData[rank_count] = sample1real_values[rank_count][v];
        }
        for (size_t rank_count = numSamples ; rank_count < samplesRef; rank_count++){
          rank_col[rank_count] = rank_count;
          rawData[rank_count] = allSamples[rank_count-numSamples][v+numContinuousVars+numDiscreteIntVars];
        }
        std::sort(rank_col.begin(), rank_col.end(), rank_sort);
        for (s=0; s<samplesRef; ++s)
          final_rank[rank_col[s]] = s+1;
        Cout << "final ranks " << final_rank << '\n';
        Cout << "rawData " << rawData << '\n'; 
        //combine_ranks_discrete(ave_ranks, sample1int_values);
        for (s=0; s<samplesRef; ++s) // can't be combined with loop above
          sampleRanks(v+numContinuousVars+numDiscreteIntVars, s) = final_rank[s];
      }
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
    Cout << "First cut full sample set\n" ; 
    write_data(Cout,allSamples,false,true,true); 
#endif //DEBUG

  }

  // need to put the first set back  
  for (s=0; s<previousSamples; ++s)
    copy_data(sample1_values[s], allSamples[s], (int)numContinuousVars);
  for (s=0; s<previousSamples; ++s) {
    for (size_t k=0; k<numDiscreteIntVars; ++k)
      allSamples[s][numContinuousVars+k]=sample1int_values[s][k];
    for (size_t k=0; k<numDiscreteRealVars; ++k)
      allSamples[s][numContinuousVars+numDiscreteIntVars+k]=sample1real_values[s][k];
  }

#ifdef DEBUG
  numSamples = samplesRef;
  Cout <<"\nallSamples\n"; write_data(Cout, allSamples, false, true, true);
#endif //DEBUG

  // evaluate full parameter set of size samplesRef, where the first half
  // should be intercepted via restart file duplication detection
  evaluate_parameter_sets(iteratedModel, true, false);
  compute_statistics(allSamples, allResponses);
}

void NonDIncremLHSSampling::print_results(std::ostream& s)
{
  if (statsFlag && !varBasedDecompFlag) {
    s << "---------------------------------------------------------------------"
      << "--------\nStatistics based on " << numSamples << " samples:\n";
    print_statistics(s);
    s << "---------------------------------------------------------------------"
      << "--------" << std::endl;
  }
}

} // namespace Dakota
