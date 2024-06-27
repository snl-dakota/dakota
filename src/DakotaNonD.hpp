/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_NOND_H
#define DAKOTA_NOND_H

#include "DakotaAnalyzer.hpp"
#include "dakota_stat_util.hpp"

//#define DERIV_DEBUG


namespace Dakota {


/// Base class for all nondetermistic iterators (the DAKOTA/UQ branch).

/** The base class for nondeterministic iterators consolidates
    uncertain variable data and probabilistic utilities for inherited
    classes. */

class NonD: public Analyzer
{
public:

  //
  //- Heading: Member functions
  //

  /// set requestedRespLevels, requestedProbLevels, requestedRelLevels,
  /// requestedGenRelLevels, respLevelTarget, cdfFlag, and pdfOutput
  /// (used in combination with alternate ctors)
  void requested_levels(const RealVectorArray& req_resp_levels,
			const RealVectorArray& req_prob_levels,
			const RealVectorArray& req_rel_levels,
			const RealVectorArray& req_gen_rel_levels,
			short resp_lev_tgt, short resp_lev_tgt_reduce,
			bool cdf_flag, bool pdf_output);

  /// prints the z/p/beta/beta* mappings reflected in
  /// {requested,computed}{Resp,Prob,Rel,GenRel}Levels for default
  /// qoi_type and qoi_labels
  void print_level_mappings(std::ostream& s) const;
  /// prints the z/p/beta/beta* mappings reflected in
  /// {requested,computed}{Resp,Prob,Rel,GenRel}Levels
  void print_level_mappings(std::ostream& s, String qoi_type,
			    const StringArray& qoi_labels) const;
  /// print level mapping statistics using optional pre-pend
  void print_level_mappings(std::ostream& s, const RealVector& level_maps,
			    bool moment_offset, const String& prepend = "");

  //
  //- Heading: Virtual member function redefinitions
  //

  bool resize();

  //
  //- Heading: Set/get routines
  //

  /// get pdfOutput
  bool pdf_output() const;
  /// set pdfOutput
  void pdf_output(bool output);

  /// get finalMomentsType
  short final_moments_type() const;
  /// set finalMomentsType
  void final_moments_type(short type);

protected:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  NonD(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor for sample generation and evaluation "on the fly"
  NonD(unsigned short method_name, Model& model);
  /// alternate constructor for sample generation and evaluation "on the fly"
  NonD(unsigned short method_name, Model& model,
       const ShortShortPair& approx_view);
  /// alternate constructor for sample generation "on the fly"
  NonD(unsigned short method_name, const RealVector& lower_bnds,
       const RealVector& upper_bnds);

  ~NonD(); ///< destructor

  //
  //- Heading: Virtual member function redefinitions
  //

  void derived_set_communicators(ParLevLIter pl_iter);

  void initialize_run();
  void finalize_run();

  // return the final uncertain variables from the nondeterministic iteration
  //const Variables& variables_results() const;
  /// return the final statistics from the nondeterministic iteration
  const Response& response_results() const;
  /// set the active set within finalStatistics
  void response_results_active_set(const ActiveSet& set);

  //
  //- Heading: New virtual member functions
  //

  /// initializes respCovariance
  virtual void initialize_response_covariance();
  /// initializes finalStatistics for storing NonD final results
  virtual void initialize_final_statistics();
  /// update finalStatistics::functionValues
  virtual void update_final_statistics();

  /// flag identifying whether sample counts correspond to level discrepancies
  virtual bool discrepancy_sample_counts() const;

  //
  //- Heading: Utility routines
  //

  /// concatenate computed{Resp,Prob,Rel,GenRel}Levels into level_maps
  void pull_level_mappings(RealVector& level_maps, size_t offset);
  /// update computed{Resp,Prob,Rel,GenRel}Levels from level_maps
  void push_level_mappings(const RealVector& level_maps, size_t offset);

  /// configure a one-dimensional hierarchical sequence (ML or MF)
  void configure_1d_sequence(size_t& num_steps, size_t& secondary_index,
			     short& seq_type);
  /// configure a two-dimensional hierarchical sequence (MLMF)
  void configure_2d_sequence(size_t& num_steps, size_t& secondary_index,
			     short& seq_type);
  /// configure the total number of model form/resolution level options
  void configure_enumeration(size_t& num_combinations, short& seq_type);

  /// extract cost estimates from model ensemble, enforcing requirements
  /// (case without metadata support)
  short configure_cost(size_t num_steps, short seq_type, RealVector& cost);
  /// extract cost estimates from model ensemble, enforcing requirements
  /// (case with metadata support)
  short configure_cost(size_t num_steps, short seq_type, RealVector& cost,
		       SizetSizetPairArray& cost_md_indices);
  // optionally extract cost estimates from model, if available
  //short query_cost(Model& model, size_t num_costs, short seq_type);
  /// optionally extract cost estimates from model ensemble, if available
  /// (case without metadata support)
  short query_cost(size_t num_steps, short seq_type, RealVector& cost);
  /// optionally extract cost estimates from model ensemble, if available
  /// (case with metadata support)
  short query_cost(size_t num_steps, short seq_type, RealVector& cost,
		   BitArray& model_cost_spec,
		   const SizetSizetPairArray& cost_md_indices);
  /// check cost specification and metadata indices for each active model
  void test_cost(short seq_type, const BitArray& model_cost_spec,
		 SizetSizetPairArray& cost_md_indices);
  /// check cost specification and metadata indices for a given model
  bool test_cost(bool cost_spec, SizetSizetPair& cost_md_indices,
		 const String& model_id);
  /// test cost for value > 0
  bool valid_cost(Real cost) const;
  /// test costs for valid values > 0
  bool valid_costs(const RealVector& costs) const;

  /// distribute pilot sample specification across model forms or levels
  void load_pilot_sample(const SizetArray& pilot_spec, size_t num_steps,
			 SizetArray& delta_N_l);
  /// distribute pilot sample specification across model forms and levels
  void load_pilot_sample(const SizetArray& pilot_spec, short seq_type,
			 const Sizet3DArray& N_l, Sizet2DArray& delta_N_l);

  /// update the relevant slice of N_l_3D from the final 2D multilevel
  /// or 2D multifidelity sample profile
  template <typename ArrayType>
  void inflate_approx_samples(const ArrayType& N_l, bool multilev,
			      size_t secondary_index,
			      std::vector<ArrayType>& N_l_vec);
  /// update the relevant slice of N_l_3D from the final 2D multilevel
  /// or 2D multifidelity sample profile
  template <typename ArrayType>
  void inflate_sequence_samples(const ArrayType& N_l, bool multilev,
				size_t secondary_index,
				std::vector<ArrayType>& N_l_vec);

  /// resizes finalStatistics::functionGradients based on finalStatistics ASV
  void resize_final_statistics_gradients();
  /// update finalStatistics::functionValues from momentStats and
  /// computed{Prob,Rel,GenRel,Resp}Levels
  void update_aleatory_final_statistics();
  /// update system metrics from component metrics within finalStatistics
  void update_system_final_statistics();
  /// update finalStatistics::functionGradients
  void update_system_final_statistics_gradients();

  /// size computed{Resp,Prob,Rel,GenRel}Levels
  void initialize_level_mappings();
  /// compute the PDF bins from the CDF/CCDF values and store in
  /// computedPDF{Abscissas,Ordinates}
  void compute_densities(const RealRealPairArray& min_max_fns,
			 bool prob_refinement = false,
			 bool all_levels_computed = false);
  /// output the PDFs reflected in computedPDF{Abscissas,Ordinates} using
  /// default qoi_type and pdf_labels
  void print_densities(std::ostream& s) const;
  /// output the PDFs reflected in computedPDF{Abscissas,Ordinates}
  void print_densities(std::ostream& s, String qoi_type,
		       const StringArray& pdf_labels) const;
  /// print system series/parallel mappings for response levels
  void print_system_mappings(std::ostream& s) const;

  /// print evaluation summary for multilevel sampling across 1D level profile
  void print_multilevel_evaluation_summary(std::ostream& s,
					   const SizetArray& N_m);
  /// print evaluation summary for multilevel sampling across 2D
  /// level+QoI profile
  void print_multilevel_evaluation_summary(std::ostream& s,
					   const Sizet2DArray& N_m);

  /// print evaluation summary for multilevel sampling across 1D level
  /// profile for discrepancy across levels
  void print_multilevel_discrepancy_summary(std::ostream& s,
					    const SizetArray& N_m);
  /// print evaluation summary for multilevel sampling across 1D level
  /// profile for discrepancy across model forms
  void print_multilevel_discrepancy_summary(std::ostream& s,
					    const SizetArray& N_m,
					    const SizetArray& N_mp1);
  /// print evaluation summary for multilevel sampling across 2D
  /// level+QoI profile for discrepancy across levels
  void print_multilevel_discrepancy_summary(std::ostream& s,
					    const Sizet2DArray& N_m);
  /// print evaluation summary for multilevel sampling across 2D
  /// level+QoI profile for discrepancy across model forms
  void print_multilevel_discrepancy_summary(std::ostream& s,
					    const Sizet2DArray& N_m,
					    const Sizet2DArray& N_mp1);

  /// print evaluation summary for multilevel sampling across 2D model+level
  /// profile (allocations) or 3D model+level+QoI profile (actual)
  template <typename ArrayType>
  void print_multilevel_model_summary(std::ostream& s,
				      const std::vector<ArrayType>& N_samp,
				      String type,// = "Final");
				      short seq_type, bool discrep_flag);

  /// assign a NonDLHSSampling instance within u_space_sampler
  void construct_lhs(Iterator& u_space_sampler, Model& u_model,
		     unsigned short sample_type, int num_samples, int seed,
		     const String& rng, bool vary_pattern,
		     short sampling_vars_mode = ACTIVE);

  /// utility for vetting sub-method request against optimizers within
  /// the package configuration
  unsigned short sub_optimizer_select(unsigned short requested_sub_method,
    unsigned short default_sub_method = SUBMETHOD_NPSOL);

  /// compute a one-sided sample increment for multilevel methods to
  /// move current sampling level to a new target
  size_t one_sided_relax_round(Real diff, Real relax_factor = 1.);
  /// compute a one-sided sample increment for multilevel methods to
  /// move current sampling level to a new target
  size_t one_sided_delta(Real current, Real target, Real relax_factor = 1.);
  /// compute a one-sided sample increment for multilevel methods to
  /// move current sampling level to a new target
  size_t one_sided_delta(const SizetArray& current, const RealVector& targets,
			 Real relax_factor = 1., size_t power = 1);
  /// compute a one-sided sample increment for multilevel methods to
  /// move current sampling level to a new target
  size_t one_sided_delta(const SizetArray& current, Real target,
			 Real relax_factor = 1., size_t power = 1);
  //size_t one_sided_delta(const Sizet2DArray& current,
  //                       const RealMatrix& targets, Real relax_factor = 1.,
  //                       size_t power = 1);
  /// compute a one-sided sample increment vector to move current sampling
  /// levels to new targets
  void one_sided_delta(const SizetArray& current, const RealVector& targets,
		       SizetArray& delta_N, Real relax_factor = 1.);
  /// compute a one-sided sample increment vector to move current sampling
  /// levels to new targets
  void one_sided_delta(const Sizet2DArray& current, const RealVector& targets,
		       SizetArray& delta_N, Real relax_factor = 1.);

  /// return true if fine-grained reporting differs from coarse-grained
  bool differ(size_t N_alloc_ij, const SizetArray& N_actual_ij) const;
  /// return true if fine-grained reporting differs from coarse-grained
  bool differ(const SizetArray& N_alloc_i,
	      const Sizet2DArray& N_actual_i) const;
  /// return true if fine-grained reporting differs from coarse-grained
  bool differ(const Sizet2DArray& N_alloc, const Sizet3DArray& N_actual) const;

  /// allocate results array storage for distribution mappings
  void archive_allocate_mappings();
  /// archive the mappings from specified response levels for specified fn
  void archive_from_resp(size_t fn_index, size_t inc_id = 0);
  /// archive the mappings to computed response levels for specified fn and
  /// (optional) increment id.
  void archive_to_resp(size_t fn_index, size_t inc_id = 0);
  /// allocate results array storage for pdf histograms
  void archive_allocate_pdf();
  /// archive a single pdf histogram for specified function
  void archive_pdf(size_t fn_index, size_t inc_id = 0);
  /// archive the equivalent number of HF evals (used by ML/MF methods)
  void archive_equiv_hf_evals(const Real equiv_hf_evals);

  /// return true if N_m is empty or only populated with zeros
  bool zeros(const SizetArray& N_m) const;
  /// return true if N_m is empty or only populated with zeros
  bool zeros(const Sizet2DArray& N_m) const;
  /// return true if N_m is empty or only populated with zeros
  bool zeros(const SizetVector& N_m) const;
  /// return true if N_l has consistent values
  bool homogeneous(const SizetArray& N_l) const;

  //
  //- Heading: Data members
  //

  /// pointer to the active object instance used within static evaluator
  /// functions in order to avoid the need for static data
  static NonD* nondInstance;
  /// pointer containing previous value of nondInstance
  NonD* prevNondInstance;

  /// starting index of continuous aleatory uncertain variables within
  /// active continuous variables (convenience for managing offsets)
  size_t startCAUV;
  /// number of active continuous aleatory uncertain variables
  size_t numCAUV;

  /// flag for computing interval-type metrics instead of integrated metrics
  /// If any epistemic vars are active in a metric evaluation, then flag is set.
  bool epistemicStats;

  /// standardized or central resp moments, as determined by finalMomentsType.
  /// Calculated in compute_moments()) and indexed as (moment,fn).
  RealMatrix momentStats;

  // map response level z -> probability level p, reliability level beta,
  // or generalized reliability level beta*

  /// requested response levels for all response functions
  RealVectorArray requestedRespLevels;
  /// output probability levels for all response functions resulting
  /// from requestedRespLevels
  RealVectorArray computedProbLevels;
  /// output reliability levels for all response functions resulting
  /// from requestedRespLevels
  RealVectorArray computedRelLevels;
  /// output generalized reliability levels for all response functions
  /// resulting from requestedRespLevels
  RealVectorArray computedGenRelLevels;
  /// indicates mapping of z->p (PROBABILITIES), z->beta (RELIABILITIES),
  /// or z->beta* (GEN_RELIABILITIES)
  short respLevelTarget;
  /// indicates component or system series/parallel failure metrics
  short respLevelTargetReduce;

  // map probability level p, reliability level beta, or generalized
  // reliability level beta* -> response level z

  /// requested probability levels for all response functions
  RealVectorArray requestedProbLevels;
  /// requested reliability levels for all response functions
  RealVectorArray requestedRelLevels;
  /// requested generalized reliability levels for all response functions
  RealVectorArray requestedGenRelLevels;
  /// output response levels for all response functions resulting from
  /// requestedProbLevels, requestedRelLevels, or requestedGenRelLevels
  RealVectorArray computedRespLevels;

  /// total number of levels specified within requestedRespLevels,
  /// requestedProbLevels, and requestedRelLevels
  size_t totalLevelRequests;

  /// flag for type of probabilities/reliabilities used in mappings:
  /// cumulative/CDF (true) or complementary/CCDF (false)
  bool cdfFlag;

  /// flag for managing output of response probability density functions (PDFs)
  bool pdfOutput;
  /// sorted response PDF intervals bounds extracted from min/max sample
  /// and requested/computedRespLevels (vector lengths = num bins + 1)
  RealVectorArray computedPDFAbscissas;
  /// response PDF densities computed from bin counts divided by
  /// (unequal) bin widths (vector lengths = num bins)
  RealVectorArray computedPDFOrdinates;

  /// final statistics from the uncertainty propagation used in strategies:
  /// response means, standard deviations, and probabilities of failure
  Response finalStatistics;
  /// type of moments logged within finalStatistics: none, central, standard
  short finalMomentsType;

  /// index for the active ParallelLevel within ParallelConfiguration::miPLIters
  size_t miPLIndex;

  /// Whether PDF was computed for function i; used to determine whether
  /// a pdf should be archived
  BitArray pdfComputed;

private:

  /// initialize data based on variable counts
  void initialize_counts();

  /// convenience function for distributing a vector of levels among multiple
  /// response functions if a short-hand specification is employed.
  void distribute_levels(RealVectorArray& levels, bool ascending = true);

  /// Write level mappings to a file for a single response
  void level_mappings_file(size_t fn_index, const String& qoi_label) const;

  /// Print level mapping for a single response function to ostream
  void print_level_map(std::ostream& s, size_t fn_index,
		       const String& qoi_label) const;

  /// print an set of aggregated QoI sample counts for a level
  void print_multilevel_row(std::ostream& s, const SizetArray& N_j);
  /// print an unrolled set of aggregated QoI sample counts for a level
  void print_multilevel_row(std::ostream& s, const SizetArray& N_j,
			    const SizetArray& N_jp1);

  //
  //- Heading: Data members
  //

};


inline NonD::~NonD()
{ }


inline short NonD::
configure_cost(size_t num_steps, short seq_type, RealVector& cost)
{
  // NonDExpansion uses this fn for enforcing cost from spec (no metadata)
  size_t m, num_mf = iteratedModel.subordinate_models(false).size();
  BitArray model_cost_spec;  SizetSizetPairArray cost_md_indices(num_mf);
  for (m=0; m<num_mf; ++m)
    cost_md_indices[m] = SizetSizetPair(SZ_MAX, 0); // no metadata for any model
  short cost_source
    = query_cost(num_steps, seq_type, cost, model_cost_spec, cost_md_indices);
  test_cost(seq_type, model_cost_spec, cost_md_indices);
  return cost_source;
}


inline short NonD::
configure_cost(size_t num_steps, short seq_type, RealVector& cost,
	       SizetSizetPairArray& cost_md_indices)
{
  // enforce required costs available from either spec or recovery:
  BitArray model_cost_spec;
  short cost_source
    = query_cost(num_steps, seq_type, cost, model_cost_spec, cost_md_indices);
  test_cost(seq_type, model_cost_spec, cost_md_indices);
  return cost_source;
}


inline short NonD::
query_cost(size_t num_steps, short seq_type, RealVector& cost)
{
  // NonDExpansion uses this function for optional cost
  size_t m, num_mf = iteratedModel.subordinate_models(false).size();
  BitArray model_cost_spec;  SizetSizetPairArray cost_md_indices(num_mf);
  for (m=0; m<num_mf; ++m)
    cost_md_indices[m] = SizetSizetPair(SZ_MAX, 0); // no metadata for any model
  return query_cost(num_steps, seq_type, cost, model_cost_spec,cost_md_indices);
}


inline bool NonD::
test_cost(bool cost_spec, SizetSizetPair& cost_md_indices,
	  const String& model_id)
{
  // lower-level version for scalar inputs: return true if neither user spec
  // nor metadata recovery are available
  bool err = false;
  if (cost_spec == true) // precedence: user spec, then online recovery
    cost_md_indices.first = SZ_MAX; // deactivate cost recovery for this model
  else if (cost_md_indices.first == SZ_MAX)
    err = true; // neither spec nor recovery
  if (err)
    Cerr << "Error: insufficient cost data for model " << model_id << ".\n";
  return err;
}


inline bool NonD::valid_cost(Real cost) const
{ return (cost > 0.) ? true : false; }


inline bool NonD::valid_costs(const RealVector& cost) const
{
  size_t i, len = cost.length();
  for (i=0; i<len; ++i)
    if (cost[i] <= 0.)
      return false;
  return true;
}


inline bool NonD::pdf_output() const
{ return pdfOutput; }


inline void NonD::pdf_output(bool output)
{ pdfOutput = output; }


inline short NonD::final_moments_type() const
{ return finalMomentsType; }


inline void NonD::final_moments_type(short type)
{ finalMomentsType = type; }


inline void NonD::initialize_run()
{ 
  Analyzer::initialize_run();
  prevNondInstance = nondInstance; 
  nondInstance = this; 
}


inline void NonD::finalize_run()
{ 
  nondInstance = prevNondInstance;
  Analyzer::finalize_run();
}


inline const Response& NonD::response_results() const
{ return finalStatistics; }


inline void NonD::response_results_active_set(const ActiveSet& set)
{ finalStatistics.active_set(set); }


inline void NonD::print_level_mappings(std::ostream& s) const
{
  print_level_mappings(s, "response function", iteratedModel.response_labels());
}


inline void NonD::print_densities(std::ostream& s) const
{ print_densities(s, "response function", iteratedModel.response_labels()); }


inline bool NonD::discrepancy_sample_counts() const
{ return false; }


inline bool NonD::homogeneous(const SizetArray& N_l) const
{
  size_t N0 = N_l[0], i, len = N_l.size();
  for (i=1; i<len; ++i)
    if (N_l[i] != N0)
      return false;
  return true;
}


inline bool NonD::
differ(size_t N_alloc_ij, const SizetArray& N_actual_ij) const
{
  if (N_actual_ij.empty()) // allow empty NLevActual[i][j]
    return (N_alloc_ij > 0);
  size_t q, num_q = N_actual_ij.size();
  for (q=0; q<num_q; ++q)
    if (N_alloc_ij != N_actual_ij[q])
      return true;
  return false;
}


inline bool NonD::
differ(const SizetArray& N_alloc_i, const Sizet2DArray& N_actual_i) const
{
  size_t j, num_lev = N_alloc_i.size();
  if (N_actual_i.size() != num_lev) return true; // require both non-empty lev
  for (j=0; j<num_lev; ++j)
    if (differ(N_alloc_i[j], N_actual_i[j]))
      return true;
  return false;
}


inline bool NonD::
differ(const Sizet2DArray& N_alloc, const Sizet3DArray& N_actual) const
{
  size_t i, num_mf = N_alloc.size();
  if (N_actual.size() != num_mf) return true; // require both non-empty mf
  for (i=0; i<num_mf; ++i)
    if (differ(N_alloc[i], N_actual[i]))
      return true;
  return false;
}


inline size_t NonD::
one_sided_relax_round(Real diff, Real relax_factor)
{
  if (relax_factor == 1.)
    return (diff > 0.) ? (size_t)std::floor(diff + .5) : 0;
  else if (diff > 0.) {
    size_t delta = (size_t)std::floor(relax_factor * diff + .5);
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Relaxation: diff " << diff << " relaxed with factor "
	   << relax_factor << " and rounded to " << delta << std::endl;
    return delta;
  }
  else return 0;
}


inline size_t NonD::
one_sided_delta(Real current, Real target, Real relax_factor)
{ return one_sided_relax_round(target - current, relax_factor); }


inline size_t NonD::
one_sided_delta(const SizetArray& current, const RealVector& targets,
		Real relax_factor, size_t power)
{
  size_t i, len = current.size();
  Real diff, pow_mean = 0.;
  switch (power) {
  case 1: // average difference same as difference of averages
    for (i=0; i<len; ++i)
      pow_mean += targets[i] - (Real)current[i]; // Note: not one-sided 
    pow_mean /= len;
    break;
  case SZ_MAX: // find max difference
    for (i=0; i<len; ++i) {
      diff = targets[i] - (Real)current[i];
      if (diff > pow_mean) pow_mean = diff;
    }
    break;
  default:
    Cerr << "Error: power " << power << " not supported in NonD::"
	 << "one_sided_delta()." << std::endl;
    abort_handler(METHOD_ERROR);
    break;
  }
  /*
  case 2: // RMS: norm of diff != difference of norms (use latter)
    for (i=0; i<len; ++i) {
      norm_t += targets[i] * targets[i];
      norm_c += current[i] * current[i];
    }
    pow_mean = std::sqrt(norm_t / len) - std::sqrt(norm_c / len);
    break;
  default: {
    for (i=0; i<len; ++i) {
      norm_t += std::pow(targets[i], (Real)power);
      norm_c += std::pow(current[i], (Real)power);
    }
    Real inv_p = 1./(Real)power;
    pow_mean = std::pow(norm_t / len, inv_p) - std::pow(norm_c / len, inv_p);
    break;
  }
  }
  */

  return one_sided_relax_round(pow_mean, relax_factor);
}


inline size_t NonD::
one_sided_delta(const SizetArray& current, Real target, Real relax_factor,
		size_t power)
{
  size_t i, len = current.size();
  Real diff, pow_mean = 0.;
  switch (power) {
  case 1: // average difference same as difference of averages
    for (i=0; i<len; ++i)
      pow_mean += target - (Real)current[i]; // Note: not one-sided 
    pow_mean /= len;
    break;
  case SZ_MAX: // find max difference
    for (i=0; i<len; ++i) {
      diff = target - (Real)current[i];
      if (diff > pow_mean) pow_mean = diff;
    }
    break;
  default:
    Cerr << "Error: power " << power << " not supported in NonD::"
	 << "one_sided_delta()." << std::endl;
    abort_handler(METHOD_ERROR);
    break;
  }

  return one_sided_relax_round(pow_mean, relax_factor);
}


/*
inline size_t NonD::
one_sided_delta(const Sizet2DArray& current, const RealMatrix& targets,
                Real relax_factor, size_t power)
{
  size_t r, c, rows = targets.numRows(), cols = targets.numCols();
  Real diff, pow_mean = 0.;
  switch (power) {
  case 1: // average difference same as difference of averages
    for (r=0; r<rows; ++r) {
      const SizetArray& curr_r = current[r];
      for (c=0; c<cols; ++c)
	pow_mean += targets(r,c) - curr_r[c];
    }
    pow_mean /= rows * cols;
    break;
  case SZ_MAX: // find one-sided max difference
    for (r=0; r<rows; ++r) {
      const SizetArray& curr_r = current[r];
      for (c=0; c<cols; ++c) {
	diff = targets(r,c) - curr_r[c];
	if (diff > pow_mean) pow_mean = diff;
      }
    }
    break;
  default:
    Cerr << "Error: power " << power << " not supported in NonD::"
	 << "one_sided_delta()." << std::endl;
    abort_handler(METHOD_ERROR);
    break;
  }
  // see notes on other finite norms above

  return one_sided_delta(pow_mean, relax_factor);
}
*/


inline void NonD::
one_sided_delta(const SizetArray& current, const RealVector& targets,
		SizetArray& delta_N, Real relax_factor)
{
  size_t i, c_len = current.size(), t_len = targets.length(), diff_i;
  if (c_len != t_len) {
    Cerr << "Error: inconsistent array sizes in NonD::one_sided_delta()."
	 << std::endl;
    abort_handler(METHOD_ERROR);
  }
  if (delta_N.size() != c_len) delta_N.resize(c_len);
  for (i=0; i<c_len; ++i)
    delta_N[i]
      = one_sided_relax_round(targets[i] - (Real)current[i], relax_factor);
}


inline void NonD::
one_sided_delta(const Sizet2DArray& current, const RealVector& targets,
		SizetArray& delta_N, Real relax_factor)
{
  size_t i, c_len = current.size(), t_len = targets.length();  Real diff_i;
  if (c_len != t_len) {
    Cerr << "Error: inconsistent array sizes in NonD::one_sided_delta()."
	 << std::endl;
    abort_handler(METHOD_ERROR);
  }
  if (delta_N.size() != c_len) delta_N.resize(c_len);
  for (i=0; i<c_len; ++i) // avg over all qoi
    delta_N[i]
      = one_sided_relax_round(targets[i] - average(current[i]), relax_factor);
}


inline bool NonD::zeros(const SizetArray& N_m) const
{
  size_t j, len = N_m.size();
  for (j=0; j<len; ++j)
    if (N_m[j])
      return false;
  return true;
}


inline bool NonD::zeros(const Sizet2DArray& N_m) const
{
  size_t j, len = N_m.size();
  for (j=0; j<len; ++j)
    if (!zeros(N_m[j]))
      return false;
  return true;
}


inline bool NonD::zeros(const SizetVector& N_m) const
{
  int j, len = N_m.length();
  for (j=0; j<len; ++j)
    if (N_m[j])
      return false;
  return true;
}


template <typename ArrayType> void NonD::
inflate_approx_samples(const ArrayType& N_l, bool multilev,
		       size_t secondary_index, std::vector<ArrayType>& N_l_vec)
{
  // 2D array is num_steps x num_qoi
  // 3D array is num_mf x num_lev x num_qoi which we slice as either:
  // > MF case: 1:num_mf x active_lev x 1:num_qoi
  // > ML case: active_mf x 1:num_lev x 1:num_qoi

  size_t i, num_mf = N_l_vec.size(), num_approx;
  if (multilev) { // ML case
    // see NonD::configure_sequence(): secondary_index should be num_mf - 1
    if (secondary_index == SZ_MAX || secondary_index >= num_mf) {
      Cerr << "Error: invalid secondary index in NonD::"
	   << "inflate_approx_samples()." << std::endl;
      abort_handler(METHOD_ERROR);
    }
    ArrayType& N_l_s = N_l_vec[secondary_index];
    num_approx = N_l_s.size() - 1;
    for (i=0; i<num_approx; ++i)
      N_l_s[i] = N_l[i];
  }
  else { // MF case
    num_approx = num_mf - 1;
    if (secondary_index == SZ_MAX) {
      ModelList& sub_models = iteratedModel.subordinate_models(false);
      ModelLIter m_iter = sub_models.begin();
      size_t m_soln_lev, active_lev;
      for (i=0; i<num_approx && m_iter != sub_models.end(); ++i, ++m_iter) {
	m_soln_lev = m_iter->solution_level_cost_index();
	active_lev = (m_soln_lev == _NPOS) ? 0 : m_soln_lev;
	N_l_vec[i][active_lev] = N_l[i];  // assign vector of qoi samples
      }
    }
    else // valid secondary_index
      for (i=0; i<num_approx; ++i)
	N_l_vec[i][secondary_index] = N_l[i]; // assign vector of qoi samples
  }
}


template <typename ArrayType> void NonD::
inflate_sequence_samples(const ArrayType& N_l, bool multilev,
			 size_t secondary_index,
			 std::vector<ArrayType>& N_l_vec)
{
  // 2D array is num_steps x num_qoi
  // 3D array is num_mf x num_lev x num_qoi which we slice as either:
  // > MF case: 1:num_mf x active_lev x 1:num_qoi
  // > ML case: active_mf x 1:num_lev x 1:num_qoi

  size_t i, num_mf = N_l_vec.size();  
  if (multilev) { // ML case
    // see NonD::configure_sequence(): secondary_index should be num_mf - 1
    if (secondary_index == SZ_MAX || secondary_index >= num_mf) {
      Cerr << "Error: invalid secondary index in NonD::"
	   << "inflate_sequence_samples()." << std::endl;
      abort_handler(METHOD_ERROR);
    }
    N_l_vec[secondary_index] = N_l;
  }
  else { // MF case
    if (secondary_index == SZ_MAX) {
      ModelList& sub_models = iteratedModel.subordinate_models(false);
      ModelLIter m_iter = sub_models.begin();
      size_t m_soln_lev, active_lev;
      for (i=0; i<num_mf && m_iter != sub_models.end(); ++i, ++m_iter) {
	m_soln_lev = m_iter->solution_level_cost_index();
	active_lev = (m_soln_lev == _NPOS) ? 0 : m_soln_lev;
	N_l_vec[i][active_lev] = N_l[i];  // assign vector of qoi samples
      }
    }
    else // valid secondary_index
      for (i=0; i<num_mf; ++i)
	N_l_vec[i][secondary_index] = N_l[i]; // assign vector of qoi samples
  }
}


template <typename ArrayType> void NonD::
print_multilevel_model_summary(std::ostream& s,
			       const std::vector<ArrayType>& N_samp,
			       String type, short seq_type, bool discrep_flag)
{
  // Sizet3DArray used for successful sample counts --> Nsamp[i] binds with 2D

  // Sizet2DArray used for sample allocations --> Nsamp[i] binds with 1D,
  // which is identical to 3D case with homogenous QoI samples

  size_t i, j, num_mf = N_samp.size(), width = write_precision+7;
  if (num_mf == 1) {
    s << "<<<<< " << type << " samples per level:\n";
    if (discrep_flag) print_multilevel_discrepancy_summary(s, N_samp[0]);
    else              print_multilevel_evaluation_summary(s,  N_samp[0]);
  }
  else {
    bool mf_seq = (seq_type == Pecos::MODEL_FORM_1D_SEQUENCE);
    ModelList& sub_models = iteratedModel.subordinate_models(false);
    ModelLIter     m_iter = sub_models.begin();
    s << "<<<<< " << type << " samples per model form:\n";
    for (i=0; i<num_mf; ++i, ++m_iter) {
      const ArrayType& N_i = N_samp[i];
      if (N_i.empty() || zeros(N_i)) continue;

      s << "      Model Form " << m_iter->model_id() << ":\n";
      if (!discrep_flag) // no discrepancies
	print_multilevel_evaluation_summary(s,  N_i);
      else if (mf_seq && i+1 < num_mf) // discrepancy across model forms
	print_multilevel_discrepancy_summary(s, N_i, N_samp[i+1]);
      else // discrepancy across levels or for last model form
	print_multilevel_discrepancy_summary(s, N_i);

      /*
      if (discrep_flag} {
	// To restrict inactive level outputs, could employ secondary index or
        // lookups, but output of inactive levels seems acceptable and provides
        // additional context relative to a user's broader specification.
	if (mf_seq) {
	  size_t c_index = sm_iter->solution_level_cost_index(),
	    lev_index_i = (c_index == SZ_MAX) ? 0 : c_index;
	  if (i+1 < num_mf) {
	    c_index = (sm_iter+1)->solution_level_cost_index();
	    size_t lev_index_ip1 = (c_index == SZ_MAX) ? 0 : c_index;
	    print_multilevel_discrepancy_summary(s, N_i[lev_index_i],
						 N_samp[i+1][lev_index_ip1]);
	  }
	  else // now separated from case immediately below
	    print_multilevel_discrepancy_summary(s, N_i[lev_index_i]);
	}
	else
	  print_multilevel_discrepancy_summary(s, N_i);
      }
      */
    }
  }
}

} // namespace Dakota

#endif
