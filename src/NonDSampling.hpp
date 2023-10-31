/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_SAMPLING_H
#define NOND_SAMPLING_H

#include "dakota_data_types.hpp"
#include "DakotaNonD.hpp"
#include "LHSDriver.hpp"
#include "SensAnalysisGlobal.hpp"

namespace Dakota {


/// Base class for common code between NonDLHSSampling,
/// NonDAdaptImpSampling, and other specializations

/** This base class provides common code for sampling methods which
    employ the Latin Hypercube Sampling (LHS) package from Sandia
    Albuquerque's Risk and Reliability organization. NonDSampling now
    exclusively utilizes the 1998 Fortran 90 LHS version as documented
    in SAND98-0210, which was converted to a UNIX link library in
    2001.  The 1970's vintage LHS (that had been f2c'd and converted
    to incomplete classes) has been removed. */
class NonDSampling: public NonD
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// alternate constructor for evaluating and computing statistics
  /// for the provided set of samples
  NonDSampling(Model& model, const RealMatrix& sample_matrix);

  /// destructor
  ~NonDSampling();

  //
  //- Heading: Public member functions
  //

  /// For the input sample set, computes mean, standard deviation, and
  /// probability/reliability/response levels (aleatory uncertainties)
  /// or intervals (epsitemic or mixed uncertainties)
  void compute_statistics(const RealMatrix&     vars_samples,
			  const IntResponseMap& resp_samples);

  /// called by compute_statistics() to calculate min/max intervals
  /// using allResponses
  void compute_intervals(RealRealPairArray& extreme_fns);
  /// called by compute_statistics() to calculate extremeValues from samples
  void compute_intervals(const IntResponseMap& samples);
  /// called by compute_statistics() to calculate min/max intervals
  /// using samples
  void compute_intervals(RealRealPairArray& extreme_fns,
			 const IntResponseMap& samples);

  /// calculates sample moments from a matrix of observations for a set of QoI
  void compute_moments(const RealVectorArray& fn_samples);
  /// calculate sample moments and confidence intervals from a map of
  /// response observations
  void compute_moments(const IntResponseMap& samples);
  /// convert IntResponseMap to RealVectorArray and invoke helpers
  void compute_moments(const IntResponseMap& samples, RealMatrix& moment_stats,
		       RealMatrix& moment_grads, RealMatrix& moment_conf_ints,
		       short moments_type, const StringArray& labels);
  /// core compute_moments() implementation with all data as inputs
  static void compute_moments(const RealVectorArray& fn_samples,
			      SizetArray& sample_counts,
			      RealMatrix& moment_stats, short moments_type,
			      const StringArray& labels);
  /// core compute_moments() implementation with all data as inputs
  static void compute_moments(const RealVectorArray& fn_samples,
			      RealMatrix& moment_stats, short moments_type);
  /// alternate RealMatrix samples API for use by external clients
  static void compute_moments(const RealMatrix& fn_samples,
			      RealMatrix& moment_stats, short moments_type);

  /// compute moment_grads from function and gradient samples
  void compute_moment_gradients(const RealVectorArray& fn_samples,
				const RealMatrixArray& grad_samples,
				const RealMatrix& moment_stats,
				RealMatrix& moment_grads, short moments_type);

  /// compute moment confidence intervals from moment values
  void compute_moment_confidence_intervals(const RealMatrix& moment_stats,
					   RealMatrix& moment_conf_ints,
					   const SizetArray& sample_counts,
					   short moments_type);

  /// archive moment statistics in results DB
  void archive_moments(size_t inc_id = 0);
  /// archive moment confidence intervals in results DB
  void archive_moment_confidence_intervals(size_t inc_id = 0);
  /// archive standardized regression coefficients in results DB
  void archive_std_regress_coeffs();

  /// archive extreme values (epistemic result) in results DB
  void archive_extreme_responses(size_t inc_id = 0);

  /// called by compute_statistics() to calculate CDF/CCDF mappings of
  /// z to p/beta and of p/beta to z as well as PDFs
  void compute_level_mappings(const IntResponseMap& samples);

  /// prints the statistics computed in compute_statistics()
  void print_statistics(std::ostream& s) const;

  /// prints the intervals computed in compute_intervals() with default
  /// qoi_type and moment_labels
  void print_intervals(std::ostream& s) const;
  /// prints the intervals computed in compute_intervals()
  void print_intervals(std::ostream& s, String qoi_type,
		       const StringArray& interval_labels) const;

  /// prints the moments computed in compute_moments() with default
  /// qoi_type and moment_labels
  void print_moments(std::ostream& s) const;
  /// prints the moments computed in compute_moments()
  void print_moments(std::ostream& s, String qoi_type,
		     const StringArray& moment_labels) const;
  /// core print moments that can be called without object
  static void print_moments(std::ostream& s, const RealMatrix& moment_stats,
			    const RealMatrix moment_cis, String qoi_type,
			    short moments_type,
			    const StringArray& moment_labels, bool print_cis);

  /// prints the Wilks stastics
  void print_wilks_stastics(std::ostream& s) const;

  /// prints the tolerance intervals stastics
  void print_tolerance_intervals_statistics(std::ostream& s) const;

  /// archive the tolerance intervals statistics in results DB
  void archive_tolerance_intervals(size_t inc_id = 0);

  /// update finalStatistics from minValues/maxValues, momentStats,
  /// and computedProbLevels/computedRelLevels/computedRespLevels
  void update_final_statistics();

  /// calculates the number of samples using the Wilks formula
  /// Static so I can test without instantiating a NonDSampling object - RWH
  static int compute_wilks_sample_size(unsigned short order, Real alpha, Real beta, bool twosided = false);

  /// Helper function - calculates the Wilks residual 
  static Real compute_wilks_residual(unsigned short order, int nsamples, Real alpha, Real beta, bool twosided);

  /// calculates the alpha paramter given number of samples using the Wilks formula
  /// Static so I can test without instantiating a NonDSampling object - RWH
  static Real compute_wilks_alpha(unsigned short order, int nsamples, Real beta, bool twosided = false);

  /// calculates the beta parameter given number of samples using the Wilks formula
  /// Static so I can test without instantiating a NonDSampling object - RWH
  static Real compute_wilks_beta(unsigned short order, int nsamples, Real alpha, bool twosided = false);

  /// Get the lower and upper bounds supported by Wilks bisection solves
  static Real get_wilks_alpha_min() { return 1.e-6; }
  static Real get_wilks_alpha_max() { return 0.999999; }
  static Real get_wilks_beta_min() { return 1.e-6; }
  static Real get_wilks_beta_max() { return 0.999999; }

  /// transform allSamples using configuration data from the source
  /// and target models
  void transform_samples(Model& src_model, Model& tgt_model,
			 bool x_to_u = true);
  /// alternate version to transform allSamples.  This is needed since
  /// random variable distribution parameters are not updated until run
  /// time and an imported sample_matrix is typically in x-space.
  void transform_samples(Pecos::ProbabilityTransformation& nataf,
  			 bool x_to_u = true);
  /// transform the specified samples matrix from x to u or u to x,
  /// assuming identical view and ids
  void transform_samples(Pecos::ProbabilityTransformation& nataf,
			 RealMatrix& sample_matrix, bool x_to_u = true);
  /// transform the specified samples matrix from x to u or u to x
  void transform_samples(Pecos::ProbabilityTransformation& nataf,
			 RealMatrix& sample_matrix,
			 SizetMultiArrayConstView src_cv_ids,
			 SizetMultiArrayConstView tgt_cv_ids,
			 bool x_to_u = true);

  /// return sampleType
  unsigned short sampling_scheme() const;
  /// return rngName
  const String& random_number_generator() const;

protected:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  NonDSampling(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor for sample generation and evaluation "on the fly"
  NonDSampling(unsigned short method_name, Model& model,
	       unsigned short sample_type, size_t samples, int seed,
	       const String& rng, bool vary_pattern, short sampling_vars_mode);
  /// alternate constructor for sample generation "on the fly"
  NonDSampling(unsigned short sample_type, size_t samples, int seed,
	       const String& rng, const RealVector& lower_bnds,
	       const RealVector& upper_bnds);
  /// alternate constructor for sample generation of correlated normals
  /// "on the fly"
  NonDSampling(unsigned short sample_type, size_t samples, int seed,
               const String& rng, const RealVector& means,
               const RealVector& std_devs, const RealVector& lower_bnds,
               const RealVector& upper_bnds, RealSymMatrix& correl);
  
  //
  //- Heading: Virtual function redefinitions
  //

  void pre_run();
  void core_run();

  size_t num_samples() const;

  /// resets number of samples and sampling flags
  void sampling_reset(size_t min_samples, bool all_data_flag, bool stats_flag);

  /// set reference number of samples, which is a lower bound during reset 
  void sampling_reference(size_t samples_ref);

  /// assign randomSeed
  void random_seed(int seed);

  /// set varyPattern
  void vary_pattern(bool pattern_flag);

  /// Uses lhsDriver to generate a set of samples from the
  /// distributions/bounds defined in the incoming model.
  void get_parameter_sets(Model& model);
  /// Uses lhsDriver to generate a set of samples from the
  /// distributions/bounds defined in the incoming model and populates
  /// the specified design matrix.
  void get_parameter_sets(Model& model, const size_t num_samples, 
                          RealMatrix& design_matrix);
  /// core of get_parameter_sets that accepts message print control
  void get_parameter_sets(Model& model, const size_t num_samples,
                          RealMatrix& design_matrix, bool write_msg);
  /// Uses lhsDriver to generate a set of uniform samples over
  /// lower_bnds/upper_bnds.
  void get_parameter_sets(const RealVector& lower_bnds,
                          const RealVector& upper_bnds);
  /// Uses lhsDriver to generate a set of normal samples 
  void get_parameter_sets(const RealVector& means,
                          const RealVector& std_devs,
                          const RealVector& lower_bnds,
                          const RealVector& upper_bnds,
                          RealSymMatrix& correl);

  /// Override default update of continuous vars only
  void update_model_from_sample(Model& model, const Real* sample_vars);
  /// override default mapping of continuous variables only
  void sample_to_variables(const Real* sample_vars, Variables& vars);
  /// override default mapping of continuous variables only
  void variables_to_sample(const Variables& vars, Real* sample_vars);

  /// return error estimates associated with each of the finalStatistics
  const RealSymMatrix& response_error_estimates() const;

  //
  //- Heading: New virtual functions
  //

  /// detect whether the seed has been updated since the most recent
  /// sample set generation
  virtual bool seed_updated();

  /// in the case of sub-iteration, map from finalStatistics.active_set()
  /// requests to activeSet used in evaluate_parameter_sets()
  virtual void active_set_mapping();

  //
  //- Heading: Convenience member functions for derived classes
  //

  /// increments numLHSRuns, sets random seed, and initializes lhsDriver
  void initialize_sample_driver(bool write_message, size_t num_samples);

  /// compute sampled subsets (all, active, uncertain) within all
  /// variables (acv/adiv/adrv) from samplingVarsMode and model
  void mode_counts(const Variables& vars, size_t& cv_start, size_t& num_cv,
		   size_t& div_start, size_t& num_div, size_t& dsv_start,
		   size_t& num_dsv, size_t& drv_start, size_t& num_drv) const;
  /// define subset views for sampling modes
  void mode_bits(const Variables& vars, BitArray& active_vars,
		 BitArray& active_corr) const;

  //
  //- Heading: Data members
  //

  int       seedSpec;    ///< the user seed specification (default is 0)
  int       randomSeed;  ///< the current seed
  const int samplesSpec; ///< initial specification of number of samples
  size_t    samplesRef;  ///< reference number of samples updated for refinement
  size_t    numSamples;  ///< the current number of samples to evaluate
  String    rngName;	 ///< name of the random number generator
  unsigned short sampleType; ///< the sample type: default, random, lhs,
                             ///< incremental random, or incremental lhs
  bool      wilksFlag; ///< flags use of Wilks formula to calculate num samples
  unsigned short wilksOrder;
  Real      wilksAlpha;    
  Real      wilksBeta;    
  short     wilksSidedness;

  /// gradients of standardized or central moments of response functions, as
  /// determined by finalMomentsType.  Calculated in compute_moments() and
  /// indexed as (var,moment) when moment id runs from 1:2*numFunctions.
  RealMatrix momentGrads;

  /// standard errors (estimator std deviation) for each of the finalStatistics
  RealSymMatrix finalStatErrors;

  /// current increment in a sequence of samples
  int samplesIncrement;

  Pecos::LHSDriver lhsDriver; ///< the C++ wrapper for the F90 LHS library
  size_t numLHSRuns; ///< counter for number of sample set generations

  bool stdRegressionCoeffs; ///< flags computation/output of standardized
                            ///< regression coefficients

  bool toleranceIntervalsFlag; ///< flags of double sided tolerance interval
                               ///<  equivalent normal
  Real tiCoverage; ///< coverage to be used in the calculation of the double
                   ///< sided tolerance interval equivaluent normal
  Real tiConfidenceLevel; ///< confidence interval to be used in the
                          ///< calculation of the double sided tolerance
                          ///< interval equivalent normal
  size_t     tiNumValidSamples;
  RealVector tiDstienMus;
  Real       tiDeltaMultiplicativeFactor;
  RealVector tiSampleSigmas;
  RealVector tiDstienSigmas;

  bool statsFlag;   ///< flags computation/output of statistics
  bool allDataFlag; ///< flags update of allResponses
                    ///< (allVariables or allSamples already defined)

  /// the sampling mode: ALEATORY_UNCERTAIN{,_UNIFORM},
  /// EPISTEMIC_UNCERTAIN{,_UNIFORM}, UNCERTAIN{,_UNIFORM},
  /// ACTIVE{,_UNIFORM}, or ALL{,_UNIFORM}.  This is a secondary control
  /// on top of the variables view that allows sampling over subsets of
  /// variables that may differ from the view.
  short samplingVarsMode;
  /// mode for input/output of LHS sample ranks: IGNORE_RANKS, GET_RANKS,
  /// SET_RANKS, or SET_GET_RANKS
  short sampleRanksMode;

  /// flag for generating a sequence of seed values within multiple
  /// get_parameter_sets() calls so that these executions (e.g., for
  /// SBO/SBNLS) are not repeated, but are still repeatable
  bool varyPattern;
  /// data structure to hold the sample ranks
  RealMatrix sampleRanks;

   /// initialize statistical post processing
  SensAnalysisGlobal nonDSampCorr;

  /// flags whether to use backfill to enforce uniqueness of discrete
  /// LHS samples
  bool backfillDuplicates;

  /// Minimum and maximum values of response functions for epistemic
  /// calculations (calculated in compute_intervals()),
  RealRealPairArray extremeValues;
  
  /// Function moments have been computed; used to determine whether
  /// to archive the moments
  bool functionMomentsComputed;

private:

  //
  //- Heading: Convenience functions
  //
  
  /// helper function to consolidate update code
  void sample_to_variables(const Real* sample_vars, Variables& vars,
			   Model& model);
  /// helper function to copy a range from sample_vars to a variables type
  void sample_to_type(const Real* sample_vars, Variables& vars,
		      size_t& cv_index, size_t num_cv, size_t& div_index,
		      size_t num_div, size_t& dsv_index, size_t num_dsv,
		      size_t& drv_index, size_t num_drv, size_t& samp_index,
		      Model& model);
  /// helper function to copy a range from sample_vars to a variables type
  void sample_to_cv_type(const Real* sample_vars, Variables& vars,
			 size_t& cv_index, size_t num_cv, size_t& div_index,
			 size_t num_div, size_t& dsv_index, size_t num_dsv,
			 size_t& drv_index, size_t num_drv, size_t& samp_index);
                         //, Model& model);
  /// helper function to copy a range from sample_vars to continuous variables
  void sample_to_cv(const Real* sample_vars, Variables& vars, size_t& acv_index,
		    size_t num_acv, size_t& samp_index);
  /// helper function to copy a range from sample_vars to discrete int variables
  void sample_to_div(const Real* sample_vars, Variables& vars,
		     size_t& adiv_index, size_t num_adiv, size_t& samp_index);
  /// helper function to copy a range from sample_vars to discrete string vars
  void sample_to_dsv(const Real* sample_vars, Variables& vars,
		     size_t& adsv_index, size_t num_adsv, size_t& samp_index,
		     const StringSetArray& dss_values);
  /// helper function to copy a range from sample_vars to discrete real vars
  void sample_to_drv(const Real* sample_vars, Variables& vars,
		     size_t& adrv_index, size_t num_adrv, size_t& samp_index);

  //
  //- Heading: Data
  //
  
  /// Matrix of confidence internals on moments, with rows for mean_lower,
  /// mean_upper, sd_lower, sd_upper (calculated in compute_moments())
  RealMatrix momentCIs;
};


inline const String& NonDSampling::random_number_generator() const
{ return rngName; }


inline void NonDSampling::pre_run()
{ 
  Analyzer::pre_run();

  // synchronize the derivative components flowing down from a NestedModel's
  // call to subIterator.response_results_active_set(), so that the correct 
  // derivs are computed in Analyzer::evaluate_parameter_sets()
  if (subIteratorFlag)
    active_set_mapping();
}


inline void NonDSampling::compute_moments(const RealVectorArray& fn_samples)
{
  SizetArray sample_counts;
  compute_moments(fn_samples, sample_counts, momentStats,
		  finalMomentsType, iteratedModel.response_labels());
}


inline void NonDSampling::compute_moments(const IntResponseMap& samples)
{
  compute_moments(samples, momentStats, momentGrads, momentCIs,
		  finalMomentsType, iteratedModel.response_labels());
}


inline void NonDSampling::compute_intervals(RealRealPairArray& extreme_fns)
{ compute_intervals(extreme_fns, allResponses); }


inline void NonDSampling::compute_intervals(const IntResponseMap& samples)
{ compute_intervals(extremeValues, samples); }


inline void NonDSampling::print_intervals(std::ostream& s) const
{ print_intervals(s, "response function", iteratedModel.response_labels()); }


inline void NonDSampling::
print_moments(std::ostream& s, String qoi_type,
	      const StringArray& moment_labels) const
{
  bool print_cis = (numSamples > 1);
  print_moments(s, momentStats, momentCIs, qoi_type, finalMomentsType,
		moment_labels, print_cis);
}


inline void NonDSampling::print_moments(std::ostream& s) const
{ print_moments(s, "response function", iteratedModel.response_labels()); }


inline void NonDSampling::sampling_reference(size_t samples_ref)
{ samplesRef = samples_ref; }


inline size_t NonDSampling::num_samples() const
{ return numSamples; }


/** used by DataFitSurrModel::build_global() to publish the minimum
    number of samples needed from the sampling routine (to build a
    particular global approximation) and to set allDataFlag and
    statsFlag.  In this case, allDataFlag is set to true (vectors of
    variable and response sets must be returned to build the global
    approximation) and statsFlag is set to false (statistics
    computations are not needed). */
inline void NonDSampling::
sampling_reset(size_t min_samples, bool all_data_flag, bool stats_flag)
{
  // allow sample reduction relative to previous sampling_reset() calls
  // (that is, numSamples may be increased or decreased to match min_samples),
  // but not relative to the original user specification (samplesSpec is a hard
  // lower bound).  With the introduction of uniform/adaptive refinements,
  // samplesRef (which is incremented from samplesSpec) replaces samplesSpec as
  // the lower bound.  maxEvalConcurrency must not be updated since parallel
  // config management depends on having the same value at ctor/run/dtor times.
  numSamples = (min_samples > samplesRef) ? min_samples : samplesRef;
  // note that previous value of numSamples is irrelevant: may increase or
  // decrease relative to previous value
  samplesIncrement = 0;

  allDataFlag = all_data_flag;
  statsFlag   = stats_flag;
}


inline void NonDSampling::random_seed(int seed)
{ /*seedSpec = */randomSeed = seed; }
// lhsDriver initialized in initialize_sample_driver()


inline bool NonDSampling::seed_updated()
{
  // default / base implementation does not involve seed sequencing
  // > min change from above (more logic could be specialized/isolated)
  return (seedSpec && seedSpec != randomSeed);
}


inline unsigned short NonDSampling::sampling_scheme() const
{ return sampleType; }


inline void NonDSampling::vary_pattern(bool pattern_flag)
{ varyPattern = pattern_flag; }


inline void NonDSampling::
transform_samples(Model& src_model, Model& tgt_model, bool x_to_u)
{
  Pecos::ProbabilityTransformation& nataf = (x_to_u) ?
    tgt_model.probability_transformation() :
    src_model.probability_transformation();

  transform_samples(nataf, allSamples, src_model.continuous_variable_ids(),
		    tgt_model.continuous_variable_ids(), x_to_u);
}


/** transform x_samples to u_samples for use by expansionSampler */
inline void NonDSampling::
transform_samples(Pecos::ProbabilityTransformation& nataf, bool x_to_u)
{
  // No model recursion available, assume same x/u ids for mapping:
  SizetMultiArrayConstView cv_ids = iteratedModel.continuous_variable_ids();
  transform_samples(nataf, allSamples, cv_ids, cv_ids, x_to_u);
}


inline void NonDSampling::
transform_samples(Pecos::ProbabilityTransformation& nataf,
		  RealMatrix& sample_matrix, bool x_to_u)
{
  // No model recursion available, assume same x/u ids for mapping:
  SizetMultiArrayConstView cv_ids = iteratedModel.continuous_variable_ids();
  transform_samples(nataf, sample_matrix, cv_ids, cv_ids, x_to_u);
}


/** This version of get_parameter_sets() extracts data from the
    user-defined model in any of the four sampling modes and populates
    the specified design matrix. */
inline void NonDSampling::
get_parameter_sets(Model& model, const size_t num_samples,
		   RealMatrix& design_matrix)
{ get_parameter_sets(model, num_samples, design_matrix, true); }


/** This version of get_parameter_sets() extracts data from the
    user-defined model in any of the four sampling modes and populates
    class member allSamples. */
inline void NonDSampling::get_parameter_sets(Model& model)
{ get_parameter_sets(model, numSamples, allSamples); }


inline void NonDSampling::
sample_to_cv(const Real* sample_vars, Variables& vars, size_t& acv_index,
	     size_t num_acv, size_t& samp_index)
{
  // sampled continuous vars (by value)
  for (size_t i=0; i<num_acv; ++i, ++samp_index, ++acv_index)
    vars.all_continuous_variable(sample_vars[samp_index], acv_index);
}


inline void NonDSampling::
sample_to_div(const Real* sample_vars, Variables& vars, size_t& adiv_index,
	      size_t num_adiv, size_t& samp_index)
{
  // sampled discrete int vars (by value cast from Real)
  for (size_t i=0; i<num_adiv; ++i, ++samp_index, ++adiv_index)
    vars.all_discrete_int_variable((int)sample_vars[samp_index], adiv_index);
}


inline void NonDSampling::
sample_to_dsv(const Real* sample_vars, Variables& vars, size_t& adsv_index,
	      size_t num_adsv, size_t& samp_index,
	      const StringSetArray& dss_values)
{
  // sampled discrete string vars (by index cast from Real)
  size_t i, set_index;
  for (i=0; i<num_adsv; ++i, ++samp_index, ++adsv_index) {
    set_index = (size_t)sample_vars[samp_index];
    const String& dss = set_index_to_value(set_index, dss_values[adsv_index]);
    vars.all_discrete_string_variable(dss, adsv_index);
  }
}


inline void NonDSampling::
sample_to_drv(const Real* sample_vars, Variables& vars, size_t& adrv_index,
	      size_t num_adrv, size_t& samp_index)
{
  // sampled discrete real vars (by value)
  for (size_t i=0; i<num_adrv; ++i, ++samp_index, ++adrv_index)
    vars.all_discrete_real_variable(sample_vars[samp_index], adrv_index);
}


inline void NonDSampling::
sample_to_type(const Real* sample_vars, Variables& vars, size_t& cv_index,
	       size_t num_cv, size_t& div_index, size_t num_div,
	       size_t& dsv_index, size_t num_dsv, size_t& drv_index,
	       size_t num_drv, size_t& samp_index, Model& model)
{
  sample_to_cv(sample_vars,  vars,  cv_index, num_cv,  samp_index);
  sample_to_div(sample_vars, vars, div_index, num_div, samp_index);
  if (num_dsv) {
    short active_view = vars.view().first, all_view =
      ( active_view == RELAXED_ALL || ( active_view >= RELAXED_DESIGN &&
        active_view <= RELAXED_STATE )) ? RELAXED_ALL : MIXED_ALL;
    // Note: Model::activeDiscSetStringValues is cached, so no penalty for
    //       repeated query with same view
    sample_to_dsv(sample_vars, vars, dsv_index, num_dsv, samp_index,
		  model.discrete_set_string_values(all_view));
  }
  sample_to_drv(sample_vars, vars, drv_index, num_drv, samp_index);
}


inline void NonDSampling::
sample_to_cv_type(const Real* sample_vars, Variables& vars, size_t& cv_index,
		  size_t num_cv, size_t& div_index, size_t num_div,
		  size_t& dsv_index, size_t num_dsv, size_t& drv_index,
		  size_t num_drv, size_t& samp_index)//, Model& model)
{
  // UNIFORM views do not currently support non-relaxed discrete

  sample_to_cv(sample_vars, vars, cv_index, num_cv, samp_index);
  //sample_to_div(sample_vars, vars, div_index, num_div, samp_index);
  //if (num_dsv) {
    //short active_view = vars.view().first, all_view = () ? : ;
    //sample_to_dsv(sample_vars, vars, dsv_index,num_dsv,samp_index,
    //              model.discrete_set_string_values(all_view));
  //}
  //sample_to_drv(sample_vars, vars, drv_index, num_drv, samp_index);
}


inline void NonDSampling::
update_model_from_sample(Model& model, const Real* sample_vars)
{ sample_to_variables(sample_vars, model.current_variables(), model); }


inline void NonDSampling::
sample_to_variables(const Real* sample_vars, Variables& vars)
{ sample_to_variables(sample_vars, vars, iteratedModel); }
// default to iteratedModel for dss values


inline const RealSymMatrix& NonDSampling::response_error_estimates() const
{ return finalStatErrors; }

} // namespace Dakota

#endif
