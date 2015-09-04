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
/// NonDIncremLHSSampling, and NonDAdaptImpSampling

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

  /// called by compute_statistics() to calculate CDF/CCDF mappings of
  /// z to p/beta and of p/beta to z
  void compute_distribution_mappings(const IntResponseMap& samples);

  /// update finalStatistics from minValues/maxValues, momentStats,
  /// and computedProbLevels/computedRelLevels/computedRespLevels
  void update_final_statistics();

  /// prints the PDFs computed in compute_statistics()
  void print_pdf_mappings(std::ostream& s) const;

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
  /// destructor
  ~NonDSampling();

  //
  //- Heading: Virtual function redefinitions
  //

  void quantify_uncertainty();

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

  /// Uses lhsDriver to generate a set of uniform samples over
  /// lower_bnds/upper_bnds.
  void get_parameter_sets(const RealVector& lower_bnds,
			  const RealVector& upper_bnds);

  /// Override default update of continuous vars only
  void update_model_from_sample(Model& model, const Real* sample_vars);

  /// override default mapping of continuous variables only
  void sample_to_variables(const Real* sample_vars, Variables& vars);


  void variables_to_sample(const Variables& vars, Real* sample_vars);


  //
  //- Heading: Convenience member functions for derived classes
  //

  /// increments numLHSRuns, sets random seed, and initializes lhsDriver
  void initialize_lhs(bool write_message);

  /// For the input sample set, computes mean, standard deviation, and
  /// probability/reliability/response levels (aleatory uncertainties)
  /// or intervals (epsitemic or mixed uncertainties)
  void compute_statistics(const RealMatrix&     vars_samples,
			  const IntResponseMap& resp_samples);
  /// called by compute_statistics() to calculate min/max intervals
  void compute_intervals(const IntResponseMap& samples);
  /// called by compute_statistics() to calculate means, std
  /// deviations, and confidence intervals
  void compute_moments(const IntResponseMap& samples);

  /// prints the statistics computed in compute_statistics()
  void print_statistics(std::ostream& s) const;
  /// prints the intervals computed in compute_intervals()
  void print_intervals(std::ostream& s) const;
  /// prints the moments computed in compute_moments()
  void print_moments(std::ostream& s) const;

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

  //
  //- Heading: Data members
  //

  const int seedSpec;    ///< the user seed specification (default is 0)
  int       randomSeed;  ///< the current seed
  const int samplesSpec; ///< initial specification of number of samples
  int       samplesRef;  ///< reference number of samples updated for refinement
  int       numSamples;  ///< the current number of samples to evaluate
  String    rngName;	 ///< name of the random number generator
  unsigned short sampleType;  ///< the sample type: default, random, lhs,
                              ///< incremental random, or incremental lhs

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

  /// flags whether to use backfill to enforce uniqueness of discrete LHS samples
  bool backfillFlag;

private:

  /// allocate results array storage for pdf histograms
  void archive_allocate_pdf();
  /// archive a single pdf histogram for specified function
  void archive_pdf(size_t fn_index);

  //
  //- Heading: Data
  //

  /// counter for number of executions of get_parameter_sets() for this object
  size_t numLHSRuns;

  /// Matrix of confidence internals on moments, with rows for
  /// mean_lower, mean_upper, sd_lower, sd_upper (calculated in
  /// compute_moments())
  RealMatrix momentCIs;

  /// Minimum (row 0) and maximum (row 1) values of response functions
  /// for epistemic calculations (calculated in compute_intervals()),
  RealMatrix extremeValues;

  /// sorted response PDF intervals bounds extracted from min/max sample
  /// and requested/computedRespLevels (vector lengths = num bins + 1)
  RealVectorArray computedPDFAbscissas;
  /// response PDF densities computed from bin counts divided by
  /// (unequal) bin widths (vector lengths = num bins)
  RealVectorArray computedPDFOrdinates;
};


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

  allDataFlag = all_data_flag;
  statsFlag   = stats_flag;
}


inline unsigned short NonDSampling::sampling_scheme() const
{ return sampleType; }


inline void NonDSampling::vary_pattern(bool pattern_flag)
{ varyPattern = pattern_flag; }

} // namespace Dakota

#endif
