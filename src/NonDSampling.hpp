/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDSampling
//- Description: Wrapper class for Fortran 90 LHS library
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

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
  void compute_moments(const RealMatrix& samples);
  /// calculate sample moments and confidence intervals from a map of
  /// response observations
  void compute_moments(const IntResponseMap& samples);
  /// convert IntResponseMap to RealMatrix and invoke helpers
  void compute_moments(const IntResponseMap& samples, RealMatrix& moment_stats,
		       RealMatrix& moment_conf_ints, short moments_type,
		       const StringArray& labels);
  /// core compute_moments() implementation with all data as inputs
  static void compute_moments(const RealMatrix& samples,
			      SizetArray& sample_counts,
			      RealMatrix& moment_stats, short moments_type,
			      const StringArray& labels);
  /// core compute_moments() implementation with all data as inputs
  static void compute_moments(const RealMatrix& samples,
			      RealMatrix& moment_stats, short moments_type);

  /// compute moment confidence intervals from moment values
  static void compute_moment_confidence_intervals(
    const RealMatrix& moment_stats,  RealMatrix& moment_conf_ints,
    const SizetArray& sample_counts, short moments_type);

  /// archive moment statistics in results DB
  void archive_moments(const RealMatrix& moment_stats, short moments_type,
		       const StringArray& labels);
  /// archive moment confidence intervals in results DB
  void archive_moment_confidence_intervals(const RealMatrix& moment_conf_ints,
					   short moments_type,
					   const StringArray& labels);

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

  /// update finalStatistics from minValues/maxValues, finalMomentStats,
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

  /// calculates the beta paramter given number of samples using the Wilks formula
  /// Static so I can test without instantiating a NonDSampling object - RWH
  static Real compute_wilks_beta(unsigned short order, int nsamples, Real alpha, bool twosided = false);

  /// Get the lower and upper bounds supported by Wilks bisection solves
  static Real get_wilks_alpha_min() { return 1.e-6; }
  static Real get_wilks_alpha_max() { return 0.999999; }
  static Real get_wilks_beta_min() { return 1.e-6; }
  static Real get_wilks_beta_max() { return 0.999999; }

  /// transform allSamples imported by alternate constructor.  This is needed
  /// since random variable distribution parameters are not updated until
  /// run time and an imported sample_matrix is typically in x-space.
  void transform_samples(bool x_to_u = true);

  /// transform the specified samples matrix from x to u or u to x
  void transform_samples(RealMatrix& sample_matrix, bool x_to_u, 
			 int num_samples = 0);

protected:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  NonDSampling(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor for sample generation and evaluation "on the fly"
  NonDSampling(unsigned short method_name, Model& model,
	       unsigned short sample_type, int samples, int seed,
	       const String& rng, bool vary_pattern, short sampling_vars_mode);
  /// alternate constructor for sample generation "on the fly"
  NonDSampling(unsigned short sample_type, int samples, int seed,
	       const String& rng, const RealVector& lower_bnds,
	       const RealVector& upper_bnds);
  /// alternate constructor for sample generation of correlated normals
  /// "on the fly"
  NonDSampling(unsigned short sample_type, int samples, int seed,
               const String& rng, const RealVector& means,
               const RealVector& std_devs, const RealVector& lower_bnds,
               const RealVector& upper_bnds, RealSymMatrix& correl);
  
  /// destructor
  ~NonDSampling();

  //
  //- Heading: Virtual function redefinitions
  //

  void core_run();

  int num_samples() const;

  /// resets number of samples and sampling flags
  void sampling_reset(int min_samples, bool all_data_flag, bool stats_flag);

  /// set reference number of samples, which is a lower bound during reset 
  void sampling_reference(int samples_ref);

  /// return sampleType
  unsigned short sampling_scheme() const;

  /// set varyPattern
  void vary_pattern(bool pattern_flag);

  /// Uses lhsDriver to generate a set of samples from the
  /// distributions/bounds defined in the incoming model.
  void get_parameter_sets(Model& model);

  /// Uses lhsDriver to generate a set of samples from the
  /// distributions/bounds defined in the incoming model and populates
  /// the specified design matrix.
  void get_parameter_sets(Model& model, const int num_samples, 
                          RealMatrix& design_matrix);

  /// core of get_parameter_sets that accepts message print control
  void get_parameter_sets(Model& model, const int num_samples,
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

  void variables_to_sample(const Variables& vars, Real* sample_vars);

  /// return error estimates associated with each of the finalStatistics
  const RealVector& response_error_estimates() const;

  //
  //- Heading: Convenience member functions for derived classes
  //

  /// increments numLHSRuns, sets random seed, and initializes lhsDriver
  void initialize_lhs(bool write_message, int num_samples);

  /// compute sampled subsets (all, active, uncertain) within all
  /// variables (acv/adiv/adrv) from samplingVarsMode and model
  void view_design_counts(const Model& model, size_t& num_cdv, size_t& num_ddiv,
			  size_t& num_ddsv, size_t& num_ddrv) const;
  /// compute sampled subsets (all, active, uncertain) within all
  /// variables (acv/adiv/adrv) from samplingVarsMode and model
  void view_aleatory_uncertain_counts(const Model& model, size_t& num_cauv,
				      size_t& num_dauiv, size_t& num_dausv,
				      size_t& num_daurv) const;
  /// compute sampled subsets (all, active, uncertain) within all
  /// variables (acv/adiv/adrv) from samplingVarsMode and model
  void view_epistemic_uncertain_counts(const Model& model, size_t& num_ceuv,
				       size_t& num_deuiv, size_t& num_deusv,
				       size_t& num_deurv) const;
  /// compute sampled subsets (all, active, uncertain) within all
  /// variables (acv/adiv/adrv) from samplingVarsMode and model
  void view_uncertain_counts(const Model& model, size_t& num_cuv,
			     size_t& num_duiv, size_t& num_dusv,
			     size_t& num_durv) const;
  /// compute sampled subsets (all, active, uncertain) within all
  /// variables (acv/adiv/adrv) from samplingVarsMode and model
  void view_state_counts(const Model& model, size_t& num_csv, size_t& num_dsiv,
			 size_t& num_dssv, size_t& num_dsrv) const;
  /// compute sampled subsets (all, active, uncertain) within all
  /// variables (acv/adiv/adrv) from samplingVarsMode and model
  void mode_counts(const Model& model, size_t& cv_start, size_t& num_cv,
		   size_t& div_start,  size_t& num_div,
		   size_t& dsv_start,  size_t& num_dsv,
		   size_t& drv_start,  size_t& num_drv) const;

  /// helper to accumulate sum of finite samples
  static void accumulate_mean(const RealMatrix& samples, size_t q,
			      size_t& num_samp, Real& mean);
  /// helper to accumulate higher order sums of finite samples
  static void accumulate_moments(const RealMatrix& samples, size_t q,
				 Real* moments, short moments_type);

  //
  //- Heading: Data members
  //

  const int seedSpec;    ///< the user seed specification (default is 0)
  int       randomSeed;  ///< the current seed
  const int samplesSpec; ///< initial specification of number of samples
  int       samplesRef;  ///< reference number of samples updated for refinement
  int       numSamples;  ///< the current number of samples to evaluate
  String    rngName;	 ///< name of the random number generator
  unsigned short sampleType; ///< the sample type: default, random, lhs,
                             ///< incremental random, or incremental lhs
  bool      wilksFlag; ///< flags use of Wilks formula to calculate num samples
  unsigned short wilksOrder;
  Real      wilksAlpha;    
  Real      wilksBeta;    
  short     wilksSidedness;

  /// standard errors (estimator std deviation) for each of the finalStatistics
  RealVector finalStatErrors;

  /// current increment in a sequence of samples
  int samplesIncrement;

  Pecos::LHSDriver lhsDriver; ///< the C++ wrapper for the F90 LHS library

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
  bool backfillFlag;

  /// Minimum and maximum values of response functions for epistemic
  /// calculations (calculated in compute_intervals()),
  RealRealPairArray extremeValues;

private:

  //
  //- Heading: Data
  //

  /// counter for number of executions of get_parameter_sets() for this object
  size_t numLHSRuns;

  /// Matrix of confidence internals on moments, with rows for mean_lower,
  /// mean_upper, sd_lower, sd_upper (calculated in compute_moments())
  RealMatrix finalMomentCIs;
};


inline void NonDSampling::compute_moments(const RealMatrix& samples)
{
  SizetArray sample_counts;
  compute_moments(samples, sample_counts, finalMomentStats, finalMomentsType,
		  iteratedModel.response_labels());
}


inline void NonDSampling::compute_moments(const IntResponseMap& samples)
{
  compute_moments(samples, finalMomentStats, finalMomentCIs, finalMomentsType,
		  iteratedModel.response_labels());
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
  print_moments(s, finalMomentStats, finalMomentCIs, qoi_type, finalMomentsType,
		moment_labels, print_cis);
}


inline void NonDSampling::print_moments(std::ostream& s) const
{ print_moments(s, "response function", iteratedModel.response_labels()); }


inline void NonDSampling::sampling_reference(int samples_ref)
{ samplesRef = samples_ref; }


inline int NonDSampling::num_samples() const
{ return numSamples; }


/** used by DataFitSurrModel::build_global() to publish the minimum
    number of samples needed from the sampling routine (to build a
    particular global approximation) and to set allDataFlag and
    statsFlag.  In this case, allDataFlag is set to true (vectors of
    variable and response sets must be returned to build the global
    approximation) and statsFlag is set to false (statistics
    computations are not needed). */
inline void NonDSampling::
sampling_reset(int min_samples, bool all_data_flag, bool stats_flag)
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


inline unsigned short NonDSampling::sampling_scheme() const
{ return sampleType; }


inline void NonDSampling::vary_pattern(bool pattern_flag)
{ varyPattern = pattern_flag; }


inline const RealVector& NonDSampling::response_error_estimates() const
{ return finalStatErrors; }

} // namespace Dakota

#endif
