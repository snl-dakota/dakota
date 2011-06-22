/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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

#include "data_types.h"
#include "DakotaNonD.H"
#include "LHSDriver.hpp"
#include "SensAnalysisGlobal.H"

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
  //- Heading: Public member functions
  //

  /// set meanStats and stdDevStats
  void moments(const RealVector& means, const RealVector& std_devs);

  /// called by compute_statistics() to calculate CDF/CCDF mappings of
  /// z to p/beta and of p/beta to z
  void compute_distribution_mappings(const IntResponseMap& samples);

  /// update finalStatistics from minValues/maxValues, meanStats/stdDevStats,
  /// and computedProbLevels/computedRelLevels/computedRespLevels
  void update_final_statistics();

  /// prints the PDFs computed in compute_statistics()
  void print_pdf_mappings(std::ostream& s) const;

  /// set samplesRef
  void sampling_reference(int samples_ref);

protected:

  //
  //- Heading: Constructors and destructor
  //

  NonDSampling(Model& model); ///< constructor

  /// alternate constructor for sample generation and evaluation "on the fly"
  NonDSampling(NoDBBaseConstructor, Model& model, const String& sample_type,
	       int samples, int seed, const String& rng);

  /// alternate constructor for sample generation "on the fly"
  NonDSampling(NoDBBaseConstructor, const String& sample_type, int samples,
	       int seed, const String& rng, const RealVector& lower_bnds,
	       const RealVector& upper_bnds);

  ~NonDSampling(); ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  int num_samples() const;

  /// resets number of samples and sampling flags
  void sampling_reset(int min_samples, bool all_data_flag, bool stats_flag);

  /// return sampleType: "lhs" or "random"
  const String& sampling_scheme() const; // returns sampling method

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

  /// compute sampled subsets of all cv/div/drv from samplingVarsMode and model
  void view_counts(const Model& model, size_t& cv_start, size_t& num_cv,
		   size_t& div_start, size_t& num_div,
		   size_t& drv_start, size_t& num_drv) const;


  //
  //- Heading: Data members
  //

  const int seedSpec;    ///< the user seed specification (default is 0)
  int       randomSeed;  ///< the current seed
  const int samplesSpec; ///< initial specification of number of samples
  int       samplesRef;  ///< reference number of samples updated for refinement
  int       numSamples;  ///< the current number of samples to evaluate
  String    rngName;	 ///< name of the random number generator
  String    sampleType;  ///< the sample type: random, lhs, or incremental_lhs

  Pecos::LHSDriver lhsDriver; ///< the C++ wrapper for the F90 LHS library

  bool statsFlag;   ///< flags computation/output of statistics
  bool allDataFlag; ///< flags update of allResponses
                    ///< (allVariables or allSamples already defined)

  /// the sampling mode: ACTIVE, ACTIVE_UNIFORM, ALL, or ALL_UNIFORM
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

  /// Plus/minus deltas on response function means for 95% confidence
  /// intervals (calculated in compute_moments())
  RealVector mean95CIDeltas;
  /// Lower bound for 95% confidence interval on std deviation
  /// (calculated in compute_moments())
  RealVector stdDev95CILowerBnds;
  /// Upper bound for 95% confidence interval on std deviation
  /// (calculated in compute_moments())
  RealVector stdDev95CIUpperBnds;  
  /// initialize statistical post processing
  SensAnalysisGlobal nonDSampCorr;

private:

  //
  //- Heading: Data
  //

  /// counter for number of executions of get_parameter_sets() for this object
  size_t numLHSRuns;

  /// means of response functions (calculated in compute_moments())
  RealVector meanStats;
  /// std deviations of response functions (calculated in compute_moments())
  RealVector stdDevStats;
  /// skewness of response functions (calculated in compute_moments())
  RealVector skewnessStats;
  /// kurtosis of response functions (calculated in compute_moments())
  RealVector kurtosisStats;

  /// Min values of response functions for epistemic calculations
  /// (calculated in compute_intervals())
  RealVector minValues;
  /// Max values of response functions for epistemic calculations
  /// (calculated in compute_intervals())
  RealVector maxValues;

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
  // the lower bound.  maxConcurrency must not be updated since parallel config
  // management depends on having the same value at ctor/run/dtor times.
  numSamples = (min_samples > samplesRef) ? min_samples : samplesRef;
  // note that previous value of numSamples is irrelevant: may increase or
  // decrease relative to previous value

  allDataFlag = all_data_flag;
  statsFlag   = stats_flag;
}


inline const String& NonDSampling::sampling_scheme() const
{ return sampleType; }


inline void NonDSampling::vary_pattern(bool pattern_flag)
{ varyPattern = pattern_flag; }


inline void NonDSampling::
moments(const RealVector& means, const RealVector& std_devs)
{ meanStats = means; stdDevStats = std_devs; }

} // namespace Dakota

#endif
