/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDLHSSampling
//- Description: Implementation code for NonDLHSSampling class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDLHSSampling.hpp"
#include "ProblemDescDB.hpp"
#include "ReducedBasis.hpp"
#include "dakota_linear_algebra.hpp"

#include "Teuchos_LAPACK.hpp"
#include "Teuchos_SerialDenseSolver.hpp"
#include "Teuchos_SerialDenseHelpers.hpp"
#include "DakotaApproximation.hpp"
//#include <Teuchos_MatrixMarket_Raw_Writer.hpp>
#include "nested_sampling.hpp"
#include "BasisPolynomial.hpp"
#include "OrthogPolyApproximation.hpp"

static const char rcsId[]="@(#) $Id: NonDLHSSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

// instance of static member variables
RealArray NonDLHSSampling::rawData;

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDLHSSampling::NonDLHSSampling(ProblemDescDB& problem_db, Model& model):
  NonDSampling(problem_db, model), numResponseFunctions(0),
  refineSamples(probDescDB.get_iv("method.nond.refinement_samples")),
  dOptimal(probDescDB.get_bool("method.nond.d_optimal")),
  numCandidateDesigns(probDescDB.get_sizet("method.num_candidate_designs")),
  oversampleRatio(probDescDB.get_real("method.nond.collocation_ratio")),
  pcaFlag(probDescDB.get_bool("method.principal_components")),
  varBasedDecompFlag(probDescDB.get_bool("method.variance_based_decomp")),
  percentVarianceExplained(
    probDescDB.get_real("method.percent_variance_explained"))
{
  // sampleType default in DataMethod.cpp is SUBMETHOD_DEFAULT (0).
  // Enforce an LHS default for this method.
  if (sampleType == SUBMETHOD_DEFAULT)
    sampleType = SUBMETHOD_LHS;

  if (model.primary_fn_type() == GENERIC_FNS)
    numResponseFunctions = model.num_primary_fns();

  if (dOptimal) {
    if (numDesignVars || numEpistemicUncVars || numStateVars) {
      Cerr << "\nError: 'd_optimal' sampling not supported for design, "
	   << "epistemic, or state\n       variables. Consider aleatory "
	   << "uncertain variables instead.\n";
      abort_handler(-1);
    }
    bool leja = (oversampleRatio > 0.0);
    if (leja) {
      if (oversampleRatio < 1.0) {
        Cerr << "\nError: 'leja_oversample_ratio' must be at least 1.0\n";
        abort_handler(-1);
      }
      if (numPoissonVars || numBinomialVars || numNegBinomialVars ||
	  numGeometricVars || numHyperGeomVars || numHistogramPtIntVars ||
	  numHistogramPtStringVars || numHistogramPtRealVars) {
        Cerr << "\nError: 'd_optimal', 'leja_oversample_ratio' does not support "
             << "discrete variables.\n";
        abort_handler(-1);
      }
    }
    else {
      // classical D-optimal
      if (numCandidateDesigns == 0)
        numCandidateDesigns = 100;
    }
    // NOTE: Classical D-optimal works with regular LHS by generating
    // candidate designs that are Latin and picking the best.
    if (sampleType == SUBMETHOD_LHS && outputLevel > SILENT_OUTPUT)
      if (refineSamples.length() > 0)
        Cout << "Warning: 'd_optimal' currently has no effect for incrementally"
             << " refined LHS \n         sampling" << std::endl;
      else if (leja)
        Cout << "Warning: 'd_optimal', 'leja_oversample_ratio' specified with "
	     << "LHS sampling;\n         candidate design will be Latin, but "
	     << "final design will not." << std::endl;
  }
}


/** This alternate constructor is used for generation and evaluation
    of Model-based sample sets.  A set_db_list_nodes has not been
    performed so required data must be passed through the constructor.
    It's purpose is to avoid the need for a separate LHS specification
    within methods that use LHS sampling. */
NonDLHSSampling::
NonDLHSSampling(Model& model, unsigned short sample_type, int samples,
		int seed, const String& rng, bool vary_pattern,
		short sampling_vars_mode): 
  NonDSampling(RANDOM_SAMPLING, model, sample_type, samples, seed, rng,
	       vary_pattern, sampling_vars_mode),
  numResponseFunctions(numFunctions), dOptimal(false), oversampleRatio(0.0),
  pcaFlag(false), varBasedDecompFlag(false)
{ }


/** This alternate constructor is used by ConcurrentStrategy for
    generation of uniform, uncorrelated sample sets.  It is _not_ a
    letter-envelope instantiation and a set_db_list_nodes has not been
    performed.  It is called with all needed data passed through the
    constructor and is designed to allow more flexibility in variables
    set definition (i.e., relax connection to a variables
    specification and allow sampling over parameter sets such as
    multiobjective weights).  In this case, a Model is not used and 
    the object must only be used for sample generation (no evaluation). */
NonDLHSSampling::
NonDLHSSampling(unsigned short sample_type, int samples, int seed,
		const String& rng, const RealVector& lower_bnds,
		const RealVector& upper_bnds): 
  NonDSampling(sample_type, samples, seed, rng, lower_bnds, upper_bnds),
  numResponseFunctions(0), dOptimal(false), oversampleRatio(0.0), 
  pcaFlag(false), varBasedDecompFlag(false)
{
  // since there will be no late data updates to capture in this case
  // (no sampling_reset()), go ahead and get the parameter sets.
  get_parameter_sets(lower_bnds, upper_bnds);
}


/** This alternate constructor is used to generate correlated normal
    sample sets.  It is _not_ a letter-envelope instantiation and a
    set_db_list_nodes has not been performed.  It is called with all
    needed data passed through the constructor.  In this case, a Model
    is not used and the object must only be used for sample generation
    (no evaluation). */
NonDLHSSampling::
NonDLHSSampling(unsigned short sample_type, int samples, int seed,
		const String& rng, const RealVector& means, 
                const RealVector& std_devs, const RealVector& lower_bnds,
		const RealVector& upper_bnds, RealSymMatrix& correl): 
  NonDSampling(sample_type, samples, seed, rng, means, std_devs,
	       lower_bnds, upper_bnds, correl),
  numResponseFunctions(0), dOptimal(false), oversampleRatio(0.0),
  pcaFlag(false), varBasedDecompFlag(false)
{
  // since there will be no late data updates to capture in this case
  // (no sampling_reset()), go ahead and get the parameter sets.
  get_parameter_sets(means, std_devs, lower_bnds, upper_bnds, correl);
}


NonDLHSSampling::~NonDLHSSampling()
{ }


void NonDLHSSampling::sampling_increment()
{
  // if no refinment samples, leave numSamples at baseline
  varyPattern = true;
  if (refineSamples.length() > 0) {
    numSamples = refineSamples[samplesIncrement];
    samplesIncrement = std::min(samplesIncrement + 1, refineSamples.length()-1);
  }
}


void NonDLHSSampling::pre_run()
{
  Analyzer::pre_run();

  bool increm_lhs_active = (sampleType == SUBMETHOD_LHS && 
			    refineSamples.length() > 0);
  if (dOptimal)
    // initialize nataf transform for generating basis
    initialize_random_variables(EXTENDED_U);
  else if (increm_lhs_active) {
    // incremental LHS needs CDF to compute ranks
    initialize_random_variable_transformation();
    initialize_random_variable_types(); // x_types only
    // Capture any run-time updates for x-space distributions
    initialize_random_variable_parameters();
  }

  // BMA TODO: D-optimal incremental LHS (challenging due to set/get ranks)

  // BMA TODO: resolve interaction between VBD and batch sampling
  // (need to generate the VBD replicates for each batch instead of
  // doing VBD on each sequences of samples in order to properly
  // detect duplicates); probably this means this pre_run code
  // migrates to another get_parameter_sets variant that VBD can call

  if (varBasedDecompFlag) {
    get_vbd_parameter_sets(iteratedModel, numSamples);
  }
  else {
    // DataFitSurrModel sets subIteratorFlag; if true it will manage
    // batch increments 
    // BMA TODO: refactor to handle increments more gracefully
    bool sample_all_batches = !subIteratorFlag;

    // Initial numSamples may be augmented by 1 or more sets of refineSamples
    int seq_len = 1;
    if (sample_all_batches)
      seq_len += refineSamples.length();
    // the user may have fixed the seed; we have to advance it
    if (refineSamples.length() > 0)
      varyPattern = true;

    IntVector samples_vec(seq_len);
    samples_vec[0] = numSamples;
    if (sample_all_batches)
      copy_data_partial(refineSamples, samples_vec, 1);

    // BMA TODO: VBD and other functions aren't accounting for string variables
    // Sampling supports modes beyond just active... do member
    // variable counts suffice?
    size_t cv_start, num_cv, div_start, num_div, dsv_start, num_dsv,
      drv_start, num_drv;
    mode_counts(iteratedModel, cv_start, num_cv, div_start, num_div,
                dsv_start, num_dsv, drv_start, num_drv);
    size_t num_vars = num_cv + num_div + num_dsv + num_drv;
    int previous_samples = 0, total_samples = samples_vec.normOne();
    
    // Initialize allSamples and all_ranks for total sample size
    if (allSamples.numRows() != num_vars || 
        allSamples.numCols() != total_samples)
      allSamples.shape(num_vars, total_samples);
    IntMatrix all_ranks;
    if (increm_lhs_active)
      all_ranks.shape(num_vars, total_samples);

    for (int batch_ind = 0; batch_ind < seq_len; ++batch_ind) {

      // generate samples of each batch size to reproduce the series
      // of increments, including the point selection
      int new_samples = samples_vec[batch_ind];

      if (increm_lhs_active) {
        // CASE: incremental LHS
        // BMA TODO: allow each batch to be D-optimal w.r.t. previous batch
        if (batch_ind == 0)
          initial_increm_lhs_set(new_samples, allSamples, all_ranks);
        else 
          increm_lhs_parameter_set(previous_samples, new_samples, 
                                   allSamples, all_ranks);
      }
      else if (dOptimal) {
        // CASES: random, incremental random, LHS w/ D-optimal
        // populate the correct subset of allSamples, preserving previous
        d_optimal_parameter_set(previous_samples, new_samples, allSamples);
      }
      else {
	// CASES: random, incremental random, LHS
        // sub-matrix of allSamples to populate
        RealMatrix batch_samples(Teuchos::View, allSamples, 
                                 num_vars, new_samples,  // num row/col
                                 0, previous_samples);   // start row/col
        get_parameter_sets(iteratedModel, new_samples, batch_samples);
      }
      previous_samples += new_samples;
    }
  }
}


void NonDLHSSampling::
initial_increm_lhs_set(int new_samples, 
                       RealMatrix& full_samples, IntMatrix& full_ranks)
{
  int num_vars = numContinuousVars + numDiscreteIntVars 
    + numDiscreteRealVars + numDiscreteStringVars;

  // sub-matrix to populate
  RealMatrix batch_samples(Teuchos::View, full_samples, 
                           num_vars, new_samples, 0, 0);
  sampleRanksMode = GET_RANKS;
  get_parameter_sets(iteratedModel, new_samples, batch_samples);

  // sub-matrix of all_ranks to populate
  IntMatrix batch_ranks(Teuchos::View, full_ranks,
                        num_vars, new_samples, 0, 0);
  store_ranks(batch_samples, batch_ranks);

}


void NonDLHSSampling::
increm_lhs_parameter_set(int previous_samples, int new_samples,
                         RealMatrix& full_samples, IntMatrix& full_ranks)
{
  // BMA NOTE: this could likely be extended to grow by any power of 2
  if (previous_samples != new_samples) {
    Cout << "Error: For incremental LHS, the total number of samples must double"
         << " each time.\n       For example, samples = 20 refinement_samples = "
         << "20 40 80." << std::endl;
    abort_handler(-1);
  }
   
  // BMA TODO: are these sizes correct for cases where we are sampling inactive?
  // Should this be based on mode_counts instead of these? See DakotaNonD.cpp
  const int num_vars = numContinuousVars + numDiscreteIntVars 
    + numDiscreteRealVars + numDiscreteStringVars;

  const int total_samples = previous_samples + new_samples;

  // initial samples is a view of the first previous_samples columns
  const RealMatrix initial_samples(Teuchos::View, full_samples, 
				   num_vars, previous_samples, 0, 0);
  // new samples to populate, Latin w.r.t. initial samples
  RealMatrix increm_samples(Teuchos::View, full_samples, 
                            num_vars, new_samples, 0, previous_samples);

  // ranks of the previous_samples are already cached here
  const IntMatrix initial_ranks(Teuchos::View, full_ranks, 
				num_vars, previous_samples, 0 ,0);
  // ranks of the new (increment) samples will be cached here
  IntMatrix increm_ranks(Teuchos::View, full_ranks,
                         num_vars, new_samples, 0 , previous_samples);
#ifdef DEBUG
  Cout << "initial_ranks\n" << initial_ranks << '\n';
#endif //DEBUG

  // initialize the matrix which identifies whether the sample 2 rank should
  // be switched or not.  The switch matrix is only for continuous variables
  BoolDequeArray switch_ranks(numContinuousVars, 
                              BoolDeque(previous_samples, false));
  const std::vector<Pecos::RandomVariable>& x_ran_vars
    = natafTransform.x_random_variables();
  for (int v=0; v<numContinuousVars; ++v) {
    const Pecos::RandomVariable& rv = x_ran_vars[v];
    for (int s=0; s<previous_samples; ++s) {
      Real val  = initial_samples[s][v];
      int rank1 = initial_ranks(v,s), index = rank1 - 1;
      if ( rv.cdf(val) < (Real)(2*rank1 - 1) / (Real)(2*previous_samples) )
        switch_ranks[v][index] = true;
    }
  }
//#ifdef DEBUG
//    Cout << "switch_ranks\n" << switch_ranks << '\n';
//#endif //DEBUG

  // generate the candidate set for the increment batch
  sampleRanksMode = GET_RANKS;
  get_parameter_sets(iteratedModel, new_samples, increm_samples);
  // Temporarily store the ranks of the new sample in increm_ranks, as
  // they are needed in the combined rank calculation
  store_ranks(increm_samples, increm_ranks);

#ifdef DEBUG
  Cout << "increm_sample\n"; write_data(Cout, increm_samples, false);
  Cout << "increm_ranks\n" << increm_ranks;
#endif // DEBUG

  // Calculate the combined ranks of continuous vars, populating sampleRanks
  sampleRanks.shapeUninitialized(num_vars, total_samples);
  for (int s=0; s<previous_samples; ++s)
    for (int v=0; v<numContinuousVars; ++v) {
      int rank1 = initial_ranks(v,s), index = rank1 - 1,
        comb_rank = (switch_ranks[v][index]) ? 2*rank1-1 : 2*rank1;
      sampleRanks(v,s) = (Real)comb_rank;
    }
  for (int s=previous_samples, cntr=0; s<total_samples; ++s,++cntr)
    for (int v=0; v<numContinuousVars; ++v) {
      int rank2 = increm_ranks(v,cntr), index = rank2 - 1,
        comb_rank = (switch_ranks[v][index]) ? 2*rank2 : 2*rank2-1;
      sampleRanks(v,s) = (Real)comb_rank;
    }
  // Ranks for discrete variables.  In the discrete case, we don't
  // switch, we just calculate the overall ranking of the combined set
  // (populates sampleRanks)
  combine_discrete_ranks(initial_samples, increm_samples);

  // Store the combined ranks so they are available to next iteration.
  // The total range of the combined ranks should now be 1 to total_samples.
  // BMA TODO: may not need to store this as well as sampleRanks
  store_ranks(full_ranks);
  
  // send LHS the full sampleRanks matrix for the combined set and get
  // back a sample that maintains the structure
#ifdef DEBUG
  sampleRanksMode = SET_GET_RANKS;
  Cout << "\ncombined ranks\n" << sampleRanks;
#else
  sampleRanksMode = SET_RANKS;
#endif //DEBUG
  // BMA TODO: Can we send LHS only increm_ranks and get the right
  // sample back?  Would be good to not make this extra matrix
  // allocation, so Laura's approach may be better (caches a matrix of
  // half the size)
  RealMatrix all_samples(num_vars, total_samples);
  get_parameter_sets(iteratedModel, total_samples, all_samples);
  RealMatrix concat_samples(Teuchos::View, all_samples,
                            num_vars, new_samples, 0, previous_samples);
  increm_samples.assign(concat_samples);

#ifdef DEBUG
  Cout << "rank_combined\n" << sampleRanks << '\n';// updated by SET_GET_RANKS
  Cout << "Full sample set allSamples\n" ; 
  write_data(Cout,allSamples,false,true,true); 
#endif //DEBUG

}


void NonDLHSSampling::
store_ranks(const RealMatrix& sample_values, IntMatrix& sample_ranks)
{
  // BMA: This is slightly different from before; always using
  // sampleRanks for continuous (previously used values for the
  // initial sample set and computed their ranks) and sorted values
  // for the discrete (as before)

  // BMA TODO: check if sample_ranks and values are compatible

  const int num_vars = sample_values.numRows();
  const int num_samples = sample_values.numCols();

  // store continuous variable ranks from the LHS-returned ranks
  for (int v=0; v<numContinuousVars; ++v)
    for (int s=0; s<num_samples; ++s)
      sample_ranks(v,s) = boost::math::round(sampleRanks(v,s));

  // compute and store the discrete ranks
  IntArray rank_col(num_samples), final_rank(num_samples);
  rawData.resize(num_samples);
  for (size_t v=numContinuousVars; v<num_vars; ++v) {
    for (size_t rank_count = 0; rank_count < num_samples; rank_count++){
      rank_col[rank_count] = rank_count;
      rawData[rank_count] = sample_values[rank_count][v];
    }
    std::sort(rank_col.begin(), rank_col.end(), rank_sort);
    for (size_t s=0; s<num_samples; ++s)
      final_rank[rank_col[s]] = s;
    for (size_t s=0; s<num_samples; ++s) // can't be combined with loop above
      sample_ranks(v, s) = final_rank[s] + 1;
    }
}


void NonDLHSSampling::store_ranks(IntMatrix& full_ranks)
{
  const int num_vars = sampleRanks.numRows();
  const int total_samples = sampleRanks.numCols();
  // Leading sub-matrix to populate
  IntMatrix combined_ranks(Teuchos::View, full_ranks,
                           num_vars, total_samples, 0, 0);
  for (int s=0; s<total_samples; ++s)
    for (int v=0; v<num_vars; ++v)
      combined_ranks(v,s) = boost::math::round(sampleRanks(v,s));
}


void NonDLHSSampling::
combine_discrete_ranks(const RealMatrix& initial_values, 
                       const RealMatrix& increm_values)
{

  const int num_vars = initial_values.numRows();
  const int previous_samples = initial_values.numCols();
  const int new_samples = increm_values.numCols();
  const int total_samples = previous_samples + new_samples;
  IntArray rank_col(total_samples), final_rank(total_samples);
  rawData.resize(total_samples);

  // vars_start lets us skip the continuous variables
  for (size_t v=numContinuousVars; v<num_vars; ++v) {
    for (size_t rank_count = 0; rank_count < previous_samples; rank_count++){
      rank_col[rank_count] = rank_count;
      rawData[rank_count] = initial_values[rank_count][v];
    }
    for (size_t rank_count = previous_samples; rank_count < total_samples; 
         rank_count++) {
      rank_col[rank_count] = rank_count;
      rawData[rank_count] = increm_values[rank_count-previous_samples][v];
    }
    std::sort(rank_col.begin(), rank_col.end(), rank_sort);
    for (size_t s=0; s<total_samples; ++s)
      final_rank[rank_col[s]] = s+1;
#ifdef DEBUG
    Cout << "final ranks " << final_rank << '\n';
    Cout << "rawData " << rawData << '\n'; 
#endif
    for (size_t s=0; s<total_samples; ++s) // can't be combined with loop above
      sampleRanks(v, s) = final_rank[s];
  }
}


bool NonDLHSSampling::rank_sort(const int& x, const int& y)
{ return rawData[x]<rawData[y]; }


/** For now, when this function is called, numSamples is the number of
    new samples to generate. */
void NonDLHSSampling::
d_optimal_parameter_set(int previous_samples, int new_samples,
                        RealMatrix& full_samples)
{
  // BMA TODO: verify we can use numerically generated for discrete types

  // BMA TODO: can allow MC or LHS with new strategy, including
  // incremental if we want; pick the new Latin design that maximizes
  // det.

  // BMA TODO: Sampling supports modes beyond just active; this gets
  // counts for more cases, but may not cover all use cases
  size_t cv_start, num_cv, div_start, num_div, dsv_start, num_dsv,
    drv_start, num_drv;
  mode_counts(iteratedModel, cv_start, num_cv, div_start, num_div,
	      dsv_start, num_dsv, drv_start, num_drv);
  size_t num_vars = num_cv + num_div + num_dsv + num_drv;

  int total_samples = previous_samples + new_samples;

  // initial samples is a view of the first previous samples columns
  RealMatrix initial_samples(Teuchos::View, full_samples, 
			     num_vars, previous_samples, 0, 0);

  // downselect points, populating full_samples with the initial
  // points plus any new selected points; selected_samples is the
  // aggregate of initial_samples and new samples
  RealMatrix selected_samples(Teuchos::View, full_samples, 
			      num_vars, total_samples, 0, 0);

  // Build polynomial basis using default basis configuration options
  Pecos::BasisConfigOptions bc_options;
  std::vector<Pecos::BasisPolynomial> poly_basis;
  ShortArray basis_types, colloc_rules;
  Pecos::SharedOrthogPolyApproxData::
    construct_basis(natafTransform.u_types(),
		    iteratedModel.aleatory_distribution_parameters(), 
		    bc_options, poly_basis, basis_types, colloc_rules);
  Pecos::SharedOrthogPolyApproxData::coefficients_norms_flag(true,
							     basis_types,
							     poly_basis);

  // transform from x to u space; should we make a copy?
  bool x_to_u = true;
  transform_samples(initial_samples, x_to_u);

  bool leja = (oversampleRatio > 0.0);
  if (leja) {
    // Use LejaSequence to select points using higher order basis

    // BMA TODO: detect bad alloc in the candidate set
    int num_candidates = (int) std::ceil(oversampleRatio * (Real) new_samples);

    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "\nGenerating " << new_samples << " sample D-optimal design with "
           << "Leja sampling from " << num_candidates << "\ncandidate points."
           << std::endl;

    // generate a parameter set of size candidate
    RealMatrix candidate_samples(num_vars, num_candidates);
    get_parameter_sets(iteratedModel, num_candidates, candidate_samples);
    // transform from x to u space; should we make a copy?
    transform_samples(candidate_samples, x_to_u);

    // BMA TODO: construct and preserve the LejaSampler if possible
    // BMA TODO: discuss with John what's needed...
    LejaSampler down_sampler;
    down_sampler.set_precondition(true);
    down_sampler.set_polynomial_basis(poly_basis);
    down_sampler.set_total_degree_basis_from_num_samples(num_vars,
							 total_samples);

    // this interface takes an initial set of samples, number of samples
    // to add, a candidate set, and returns selected = [initial, new]
    down_sampler.Sampler::enrich_samples(num_vars, initial_samples, new_samples,
					 candidate_samples, selected_samples);

  }
  else {
    // Classical D-optimal, maximize det(B'*B) for linear u-space basis_matrix B

    // linear (order 1) terms only
    UShort2DArray multi_index(num_vars, UShortArray(num_vars, 0));
    for (size_t vi=0; vi<num_vars; ++vi)
      multi_index[vi][vi] = 1;

    // repeatedly generate a parameter set of size new_samples
    // the newly populated points go in this block of full_samples
    RealMatrix curr_new_samples(Teuchos::View, full_samples, 
                                num_vars, new_samples, 0, previous_samples);
    // the best design is tracked here:
    RealMatrix best_new_samples(num_vars, new_samples);

    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "\nGenerating " << new_samples << " sample D-optimal design from "
           << numCandidateDesigns << " candidate designs." << std::endl;

    double best_det = 0.0;
    for (int cand_i = 0; cand_i < numCandidateDesigns; ++cand_i) {

      get_parameter_sets(iteratedModel, new_samples, curr_new_samples, false);
      transform_samples(curr_new_samples, x_to_u);

      // build basis matrix from total sample set (selected_samples
      // includes intiial and new samples)
      RealMatrix basis_matrix;
      Pecos::OrthogPolyApproximation::
        basis_matrix(selected_samples, poly_basis, multi_index, basis_matrix);
      // preconditioning shouldn't matter for linear regression:
      //LejaSampler::apply_preconditioning(basis_matrix);

      double det = det_AtransA(basis_matrix);
      if (det > best_det) {
        best_det = det;
        best_new_samples.assign(curr_new_samples);
      }
    }
    curr_new_samples.assign(best_new_samples);
  }
  // transform whole samples matrix from u back to x space
  bool u_to_x = false;
  transform_samples(selected_samples, u_to_x);
}

void NonDLHSSampling::post_input()
{
  size_t cv_start, num_cv, div_start, num_div, dsv_start, num_dsv,
    drv_start, num_drv;
  mode_counts(iteratedModel, cv_start, num_cv, div_start, num_div,
	      dsv_start, num_dsv, drv_start, num_drv);
  size_t num_vars = num_cv + num_div + num_dsv + num_drv;
  // call convenience function from Analyzer
  read_variables_responses(numSamples, num_vars);
}


/** Loop over the set of samples and compute responses.  Compute
    statistics on the set of responses if statsFlag is set. */
void NonDLHSSampling::core_run()
{
  bool log_resp_flag = (allDataFlag || statsFlag);
  bool log_best_flag = !numResponseFunctions; // DACE mode w/ opt or NLS
    evaluate_parameter_sets(iteratedModel, log_resp_flag, log_best_flag);
}


void NonDLHSSampling::post_run(std::ostream& s)
{
  // Statistics are generated here and output in NonDLHSSampling's
  // redefinition of print_results().
  if (statsFlag)
    // BMA TODO: always compute all stats, even in VBD mode (stats on
    // first two replicates)
    if (varBasedDecompFlag)
      compute_vbd_stats(numSamples, allResponses);
    else 
      compute_statistics(allSamples, allResponses);

  Analyzer::post_run(s);
 
  if (pcaFlag)
    compute_pca(s);
}


void NonDLHSSampling::update_final_statistics()
{
  NonDSampling::update_final_statistics();
  
  // if MC sampling, assign standard errors for moments within finalStatErrors
  if (sampleType == SUBMETHOD_RANDOM && finalMomentsType && !epistemicStats) {

    if (finalStatErrors.empty())
      finalStatErrors.size(finalStatistics.num_functions()); // init to 0.

    size_t i, cntr = 0;
    Real sqrt2 = std::sqrt(2.), ns = (Real)numSamples, sqrtn = std::sqrt(ns),
       sqrtnm1 = std::sqrt(ns - 1.);
    switch (finalMomentsType) {
    case STANDARD_MOMENTS:
      for (i=0; i<numFunctions; ++i) {
	Real qoi_stdev = finalMomentStats(1,i);
	// standard error (estimator std-dev) for Monte Carlo mean
	finalStatErrors[cntr++] = qoi_stdev / sqrtn;
	// standard error (estimator std-dev) for Monte Carlo std-deviation
	// (Harding et al., 2014: assumes normally distributed population): 
	finalStatErrors[cntr++] = qoi_stdev / (sqrt2*sqrtnm1);
	// level mapping errors not implemented at this time
	cntr +=
	  requestedRespLevels[i].length() +   requestedProbLevels[i].length() +
	  requestedRelLevels[i].length()  + requestedGenRelLevels[i].length();
      }
      break;
    case CENTRAL_MOMENTS:
      for (i=0; i<numFunctions; ++i) {
	Real qoi_var = finalMomentStats(1,i), qoi_stdev = std::sqrt(qoi_var);
	// standard error (estimator std-dev) for Monte Carlo mean
	finalStatErrors[cntr++] = qoi_stdev / sqrtn;
	// standard error (estimator std-dev) for Monte Carlo variance
	// (Harding et al., 2014: assumes normally distributed population): 
	finalStatErrors[cntr++] = qoi_var * sqrt2 / sqrtnm1;
	// level mapping errors not implemented at this time
	cntr +=
	  requestedRespLevels[i].length() +   requestedProbLevels[i].length() +
	  requestedRelLevels[i].length()  + requestedGenRelLevels[i].length();
      }
      break;
    }
  }
}


void NonDLHSSampling::compute_pca(std::ostream& s)
{
  IntRespMCIter r_it; size_t fn, samp;
  RealMatrix responseMatrix;
  responseMatrix.reshape(numSamples,numFunctions);
  for (samp=0, r_it=allResponses.begin(); samp<numSamples; ++samp, ++r_it)
    for (fn=0; fn<numFunctions; ++fn) 
      responseMatrix(samp,fn) = r_it->second.function_value(fn);
  s << "numFunctions " << numFunctions << "\nnumSamples " << numSamples << '\n';
  if (outputLevel == DEBUG_OUTPUT) {
    s << "Original Data\n";
    for (samp=0; samp<numSamples; ++samp) {
      for (fn=0; fn<numFunctions; ++fn)
	s << responseMatrix(samp,fn) << "  " ;
      s << '\n';
    }
    s << std::endl;
  }
  // Compute the SVD (includes centering the matrix)
  ReducedBasis pcaReducedBasis; 
  pcaReducedBasis.set_matrix(responseMatrix);
  pcaReducedBasis.update_svd();

  /*ReducedBasis::VarianceExplained trunc(0.99);
    RealVector sing_values = pcaReducedBasis.get_singular_values(trunc);
    int num_values = sing_values.length();
    s << "num_values " << num_values << '\n'; 
  */

  // Get the centered version of the original response matrix
  RealMatrix centered_matrix = pcaReducedBasis.get_matrix();
  if (outputLevel == DEBUG_OUTPUT) {
    s << "Centered matrix\n";
    centered_matrix.print(std::cout);
    s << std::endl;
  }

  // for now get the first factor score
  RealMatrix principal_comp
    = pcaReducedBasis.get_right_singular_vector_transpose();
    
  std::ofstream myfile;
  myfile.open("princ_comp.txt");
  if (outputLevel == DEBUG_OUTPUT) {
    s << "principal components\n";
    for (samp=0; samp<numSamples; ++samp) {
      for (fn=0; fn<numFunctions; ++fn) {
	s << principal_comp(samp,fn) << "  " ;
	myfile <<  std::fixed << std::setprecision(16) << principal_comp(samp,fn) << "  " ;
      }
      s << '\n';
      myfile << '\n';
    }
    s << std::endl;
  }
    
  RealMatrix factor_scores(numSamples, numFunctions);
  int myerr = factor_scores.multiply(Teuchos::NO_TRANS, Teuchos::TRANS, 1., 
				     centered_matrix, principal_comp, 0.);
 
  RealMatrix
    f_scores(Teuchos::Copy, factor_scores, numSamples, numSamples, 0, 0);
  if (outputLevel == DEBUG_OUTPUT) {
    s << "myerr" << myerr <<'\n';
    s << "FactorScores\n";
    for (samp=0; samp<numSamples; ++samp) {
      for (fn=0; fn<numSamples; ++fn)
	s << f_scores(samp,fn) << " " ;
      s << '\n';
    }
    s << std::endl;
  }
  //Dakota::compute_svd( responseMatrix, numSamples, numFunctions,
  //                     singular_values );
  RealVector singular_values = pcaReducedBasis.get_singular_values();
  if (outputLevel == DEBUG_OUTPUT)
    s << "singular values " << singular_values <<'\n';
  s << std::endl;

  //Build GPs
  // Get number of principal components to retain
  s << "percent_variance_explained " << percentVarianceExplained << '\n';
  ReducedBasis::VarianceExplained truncation(percentVarianceExplained);
  int num_signif_Pcomps = truncation.get_num_components(pcaReducedBasis);
  s << "number of significant principal components " <<  num_signif_Pcomps
    << '\n' << std::endl;

  String approx_type; 
  approx_type = "global_kriging";  // Surfpack GP
  UShortArray approx_order;
  short data_order = 1;  // assume only function values
  short output_level = NORMAL_OUTPUT;
  SharedApproxData sharedData;

  sharedData = SharedApproxData(approx_type, approx_order, numContinuousVars,
				data_order, output_level);
  std::vector<Approximation> gpApproximations;

  // build one GP for each Principal Component
  for (int i = 0; i < num_signif_Pcomps; ++i) {
    gpApproximations.push_back(Approximation(sharedData));
  }           
  for (int i = 0; i < num_signif_Pcomps; ++i) {
    RealVector factor_i = Teuchos::getCol(Teuchos::View,f_scores,i);
    gpApproximations[i].add(allSamples, factor_i );
    gpApproximations[i].build();
    std::stringstream ss;
    ss << i;
    std::string GPstring = ss.str();
    const String GPPrefix = "PCA_GP";
    gpApproximations[i].export_model(GPstring, GPPrefix, ALGEBRAIC_FILE);
  }

  // Now form predictions based on new input points
   
  Real pca_coeff;
  RealVector this_pred(numFunctions);
  get_parameter_sets(iteratedModel); // NOTE:  allSamples is now different

  if (outputLevel == DEBUG_OUTPUT) {
    s << "All Sample new values " << std::endl;
    for (samp=0; samp<numSamples; ++samp) {
      for (fn=0; fn<numContinuousVars; ++fn)
        s << allSamples(samp,fn) << "  " ;
        s << '\n';
      }
    s << std::endl;
  } 

  RealMatrix predMatrix;
  predMatrix.reshape(numSamples, numFunctions);

  RealVector column_means = pcaReducedBasis.get_column_means();
  for( int k=0; k<numFunctions; ++k )
    myfile << std::fixed << std::setprecision(16) << column_means(k) << "  " ;
  myfile << '\n';
  myfile.close();

  for (int j = 0; j < numSamples; j++) {
    //this_pred = mean_vec;
    this_pred = 0.0;
    for (int i = 0; i < num_signif_Pcomps; ++i) {
      RealVector new_sample = Teuchos::getCol(Teuchos::View,allSamples,j);
      pca_coeff=gpApproximations[i].value(new_sample);
      if (outputLevel == DEBUG_OUTPUT)
	s << "pca_coeff " << pca_coeff << '\n';
      // need Row, not column
      RealVector local_pred(numFunctions);
      for( int k=0; k<numFunctions; ++k )
	local_pred(k) = pca_coeff*principal_comp(i,k);
      this_pred += local_pred;
    }
    for( int k=0; k<numFunctions; ++k )
      predMatrix(j,k) = this_pred(k)+column_means(k);
  }
  if (outputLevel == DEBUG_OUTPUT) {
    s << "\nPrediction Matrix\n";
    for (samp=0; samp<numSamples; ++samp) {
      for (fn=0; fn<numFunctions; ++fn)
        s << predMatrix(samp,fn) << "  " ;
      s << '\n';
    }
  }
}


void NonDLHSSampling::print_results(std::ostream& s)
{
  if (!numResponseFunctions) // DACE mode w/ opt or NLS
    Analyzer::print_results(s);

  if (varBasedDecompFlag)
    print_sobol_indices(s);
  else if (statsFlag) {
    int actual_samples = allSamples.numCols();
    s << "---------------------------------------------------------------------"
      << "--------\nStatistics based on " << actual_samples << " samples:\n";
    print_statistics(s);
    s << "---------------------------------------------------------------------"
      << "--------" << std::endl;
  }
}

} // namespace Dakota
