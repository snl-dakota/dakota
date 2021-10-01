/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonD
//- Description: Base class for NonDeterministic branch
//- Owner:	 Mike Eldred
//- Checked by:
//- Version:

#ifndef DAKOTA_NOND_H
#define DAKOTA_NOND_H

#include "DakotaAnalyzer.hpp"

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

  //
  //- Heading: Utility routines
  //

  /// concatenate computed{Resp,Prob,Rel,GenRel}Levels into level_maps
  void pull_level_mappings(RealVector& level_maps, size_t offset);
  /// update computed{Resp,Prob,Rel,GenRel}Levels from level_maps
  void push_level_mappings(const RealVector& level_maps, size_t offset);

  /// configure fidelity/level counts from model hierarchy
  void configure_sequence(size_t& num_steps, size_t& fixed_index,
			  short& seq_type);
  /// extract cost estimates from model hierarchy (forms or resolutions)
  void configure_cost(unsigned short num_steps, bool multilevel,
		      RealVector& cost);
  /// extract cost estimates from model hierarchy, if available
  bool query_cost(unsigned short num_steps, bool multilevel, RealVector& cost);

  /// distribute pilot sample specification across model forms or levels
  void load_pilot_sample(const SizetArray& pilot_spec, size_t num_steps,
			 SizetArray& delta_N_l);
  /// distribute pilot sample specification across model forms and levels
  void load_pilot_sample(const SizetArray& pilot_spec, const Sizet3DArray& N_l,
			 Sizet2DArray& delta_N_l);
  /// update the relevant slice of N_l_3D from the final 2D multilevel
  /// or 2D multifidelity sample profile
  void inflate_final_samples(const Sizet2DArray& N_l_2D, bool multilev,
			     size_t fixed_index, Sizet3DArray& N_l_3D);

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

  /// print evaluation summary for multilevel sampling across 1D profile
  void print_multilevel_evaluation_summary(std::ostream& s,
					   const SizetArray& N_samp);
  /// print evaluation summary for multilevel sampling across 2D profile
  void print_multilevel_evaluation_summary(std::ostream& s,
					   const Sizet2DArray& N_samp);
  /// print evaluation summary for multilevel sampling across 3D profile
  void print_multilevel_evaluation_summary(std::ostream& s,
					   const Sizet3DArray& N_samp,
					   String type = "Final");

  /// assign a NonDLHSSampling instance within u_space_sampler
  void construct_lhs(Iterator& u_space_sampler, Model& u_model,
		     unsigned short sample_type, int num_samples, int seed,
		     const String& rng, bool vary_pattern,
		     short sampling_vars_mode = ACTIVE);

  /// utility for vetting sub-method request against optimizers within
  /// the package configuration
  unsigned short sub_optimizer_select(unsigned short requested_sub_method,
    unsigned short default_sub_method = SUBMETHOD_SQP);

  /// compute a one-sided sample increment for multilevel methods to
  /// move current sampling level to a new target
  size_t one_sided_delta(Real current, Real target);
  /// compute a one-sided sample increment for multilevel methods to
  /// move current sampling level to a new target
  size_t one_sided_delta(const SizetArray& current, const RealVector& targets,
			 size_t power);
  //size_t one_sided_delta(const Sizet2DArray& current,
  //			   const RealMatrix& targets, size_t power);

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

  /// return true if N_l has consistent values
  bool homogeneous(const SizetArray& N_l) const;

  //
  //- Heading: Data members
  //

};


inline NonD::~NonD()
{ }


inline void NonD::
configure_cost(unsigned short num_steps, bool multilevel, RealVector& cost)
{
  bool cost_defined = query_cost(num_steps, multilevel, cost);
  if (!cost_defined) {
    Cerr << "Error: missing required simulation cost data in NonD::"
	 << "configure_cost()." << std::endl;
    abort_handler(METHOD_ERROR);
  }
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


inline bool NonD::homogeneous(const SizetArray& N_l) const
{
  size_t N0 = N_l[0], i, len = N_l.size();
  for (i=1; i<len; ++i)
    if (N_l[i] != N0)
      return false;
  return true;
}


inline size_t NonD::one_sided_delta(Real current, Real target)
{ return (target > current) ? (size_t)std::floor(target - current + .5) : 0; }


inline size_t NonD::
one_sided_delta(const SizetArray& current, const RealVector& targets,
		size_t power)
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

  return (pow_mean > 0.) ? (size_t)std::floor(pow_mean + .5) : 0; // round
}


/*
inline size_t NonD::
one_sided_delta(const Sizet2DArray& current, const RealMatrix& targets,
		size_t power)
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

  return (pow_mean > 0.) ? (size_t)std::floor(pow_mean + .5) : 0; // round
}
*/

} // namespace Dakota

#endif
