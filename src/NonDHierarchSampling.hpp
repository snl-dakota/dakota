/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDHierarchSampling
//- Description: Base class for hierarchical ML/MF sampling methods
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#ifndef NOND_HIERARCHICAL_SAMPLING_H
#define NOND_HIERARCHICAL_SAMPLING_H

#include "NonDSampling.hpp"
#include "DataMethod.hpp"


namespace Dakota {

/// Performs Hierarch Monte Carlo sampling for uncertainty quantification.

/** Hierarch Monte Carlo (MLMC) is a variance-reduction technique
    that utilitizes lower fidelity simulations that have response QoI
    that are correlated with the high-fidelity response QoI. */

class NonDHierarchSampling: public NonDSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDHierarchSampling(ProblemDescDB& problem_db, Model& model);
  /// destructor (virtual declaration should be redundant with ~Iterator,
  /// but this is top of MLMF diamond so doesn't hurt to be explicit)
  virtual ~NonDHierarchSampling();

  //
  //- Heading: Virtual function redefinitions
  //

  bool resize();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  //void pre_run();
  //void core_run();
  void post_run(std::ostream& s);
  void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

  //
  //- Heading: Member functions
  //

  void evaluate_sample_increment(const unsigned short& step);

  /// synchronize iteratedModel and activeSet on AGGREGATED_MODELS mode
  void aggregated_models_mode();
  /// synchronize iteratedModel and activeSet on BYPASS_SURROGATE mode
  void bypass_surrogate_mode();
  /// synchronize iteratedModel and activeSet on UNCORRECTED_SURROGATE mode
  void uncorrected_surrogate_mode();

  /// return (aggregate) level cost
  Real level_cost(const RealVector& cost, size_t step);

  /// advance any sequence specifications
  void assign_specification_sequence(size_t index);
  /// extract current random seed from randomSeedSeqSpec
  int random_seed(size_t index) const;

  /// manage response mode and active model key from {group,form,lev} triplet.
  /// seq_type defines the active dimension for a 1D model sequence.
  void configure_indices(unsigned short group, unsigned short form,
			 size_t lev, short seq_type);
  /// convert group and form and call overload
  void configure_indices(size_t group, size_t form, size_t lev, short seq_type);

  /// export allSamples to tagged tabular file
  void export_all_samples(String root_prepend, const Model& model,
			  size_t iter, size_t step);

  /// convert uncentered raw moments (multilevel expectations) to
  /// standardized moments
  void convert_moments(const RealMatrix& raw_mom, RealMatrix& final_mom);

  /// convert uncentered (raw) moments to centered moments; biased estimators
  static void uncentered_to_centered(Real  rm1, Real  rm2, Real  rm3, Real  rm4,
			      Real& cm1, Real& cm2, Real& cm3, Real& cm4,
			      size_t Nlq);
  /// convert uncentered (raw) moments to centered moments; unbiased estimators
  static void uncentered_to_centered(Real  rm1, Real  rm2, Real  rm3, Real  rm4,
			      Real& cm1, Real& cm2, Real& cm3, Real& cm4);
  /// convert centered moments to standardized moments
  static void centered_to_standard(Real  cm1, Real  cm2, Real  cm3, Real  cm4,
			    Real& sm1, Real& sm2, Real& sm3, Real& sm4);
  /// detect, warn, and repair a negative central moment (for even orders)
  static void check_negative(Real& cm);

  /// compute sum of a set of observations
  Real sum(const Real* vec, size_t vec_len) const;
  /// compute average of a set of observations
  Real average(const Real* vec, size_t vec_len) const;
  /// compute average of a set of observations
  Real average(const RealVector& vec) const;
  /// compute average of a set of observations
  Real average(const SizetArray& sa) const;

  //
  //- Heading: Data
  //

  /// total number of successful sample evaluations (excluding faults)
  /// for each model form, discretization level, and QoI
  Sizet3DArray NLev;

  /// store the pilot_samples input specification, prior to run-time
  /// invocation of load_pilot_sample()
  SizetArray pilotSamples;

  /// user specification for seed_sequence
  SizetArray randomSeedSeqSpec;

  /// major iteration counter
  size_t mlmfIter;

  /// equivalent number of high fidelity evaluations accumulated using samples
  /// across multiple model forms and/or discretization levels
  Real equivHFEvals;

  /// if defined, export each of the sample increments in ML, CV, MLCV
  /// using tagged tabular files
  bool exportSampleSets;
  /// format for exporting sample increments using tagged tabular files
  unsigned short exportSamplesFormat;

private:

  //
  //- Heading: Helper functions
  //

};


inline void NonDHierarchSampling::aggregated_models_mode()
{
  if (iteratedModel.surrogate_response_mode() != AGGREGATED_MODELS) {
    iteratedModel.surrogate_response_mode(AGGREGATED_MODELS); // set HF,LF
    // synch activeSet with iteratedModel.response_size()
    activeSet.reshape(2*numFunctions);
    activeSet.request_values(1);
  }
}


inline void NonDHierarchSampling::bypass_surrogate_mode()
{
  if (iteratedModel.surrogate_response_mode() != BYPASS_SURROGATE) {
    iteratedModel.surrogate_response_mode(BYPASS_SURROGATE); // HF
    activeSet.reshape(numFunctions);// synch with model.response_size()
  }
}


inline void NonDHierarchSampling::uncorrected_surrogate_mode()
{
  if (iteratedModel.surrogate_response_mode() != UNCORRECTED_SURROGATE) {
    iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE); // LF
    activeSet.reshape(numFunctions);// synch with model.response_size()
  }
}


inline void NonDHierarchSampling::
configure_indices(size_t group, size_t form, size_t lev, short seq_type)
{
  // preserve special values across type conversions
  unsigned short grp = (group == SZ_MAX) ? USHRT_MAX : (unsigned short)group,
                 frm = (form  == SZ_MAX) ? USHRT_MAX : (unsigned short)form;
  configure_indices(grp, frm, lev, seq_type);
}


inline Real NonDHierarchSampling::
level_cost(const RealVector& cost, size_t step)
{
  // discrepancies incur two level costs
  return (step) ?
    cost[step] + cost[step-1] : // aggregated {HF,LF} mode
    cost[step];                 //     uncorrected LF mode
}


/** extract an active seed from a seed sequence */
inline int NonDHierarchSampling::random_seed(size_t index) const
{
  // return 0 for cases where seed is undefined or will not be updated

  if (randomSeedSeqSpec.empty()) return 0; // no spec -> non-repeatable samples
  else if (!varyPattern) // continually reset seed to specified value
    return (index < randomSeedSeqSpec.size()) ?
      randomSeedSeqSpec[index] : randomSeedSeqSpec.back();
  // only set sequence of seeds for first pass, then let RNG state continue
  else if (mlmfIter == 0 && index < randomSeedSeqSpec.size()) // pilot iter only
    return randomSeedSeqSpec[index];
  else return 0; // seed sequence exhausted, do not update
}


/** For single-level moment calculations with a scalar Nlq. */
inline void NonDHierarchSampling::
uncentered_to_centered(Real  rm1, Real  rm2, Real  rm3, Real  rm4,
		       Real& cm1, Real& cm2, Real& cm3, Real& cm4)
{
  // convert from uncentered ("raw") to centered moments for a single level

  // For moments from sampling:
  // > Raw moments are unbiased. Central moments are unbiased for an exact mean
  //   (i.e., the samples represent the full population).
  // > For sampling a portion of the population, central moments {2,3,4} are 
  //   biased estimators since the mean is approximated.  The conversion to
  //   unbiased requires a correction based on the number of samples, as
  //   implemented in the subsequent function.

  cm1 = rm1;             // mean
  cm2 = rm2 - cm1 * cm1; // variance 

  cm3 = rm3 - cm1 * (3. * cm2 + cm1 * cm1);                         // using cm
  //  = rm3 - cm1 * (3. * rm2 - 2. * cm1 * cm1);                    // using rm

  // the 4th moment is the central moment (non-excess, not cumulant)
  cm4 = rm4 - cm1 * (4. * cm3 + cm1 * (6. * cm2 + cm1 * cm1));      // using cm
  //  = rm4 - cm1 * (4. * rm3 - cm1 * (6. * rm2 - 3. * cm1 * cm1)); // using rm

  if(cm4 < 0){
    Cerr << "NonDHierarchSampling::uncentered_to_centered: cm4 < 0" << std::endl; 
    //check_negative(cm4);
  }
}


/** For single-level moment calculations with a scalar Nlq. */
inline void NonDHierarchSampling::
uncentered_to_centered(Real  rm1, Real  rm2, Real  rm3, Real  rm4, Real& cm1,
		       Real& cm2, Real& cm3, Real& cm4, size_t Nlq)
{
  // convert from uncentered ("raw") to centered moments for a single level

  // Biased central moment estimators:
  uncentered_to_centered(rm1, rm2, rm3, rm4, cm1, cm2, cm3, cm4);

  // Bias corrections for population-based estimators w/ estimated means:
  if (Nlq > 3) {
    Real cm1_sq = cm1 * cm1;
    Real nm1 = Nlq - 1., nm2 = Nlq - 2., n_sq = Nlq * Nlq;
    cm2 *= Nlq / nm1; // unbiased population variance from Bessel's correction
    cm3 *= n_sq / (nm1 * nm2);
    // From "Modeling with Data," Klemens 2009 (Appendix M).
    // Notes:
    // (1) this is 4th central moment (non-excess, unnormalized),
    //     which differs from the fourth cumulant (excess, unnormalized)
    // (2) cm2 is now unbiased within following conversion:
    
    //cm4 = ( n_sq * Nlq * cm4 / nm1 - (6. * Nlq - 9.) * cm2 * cm2 )
    //    / (n_sq - 3. * Nlq + 3.);
    //[fm] account for bias correction due to cm2^2 term
    cm4 = ( n_sq * Nlq * cm4 / nm1 - (6. * Nlq - 9.) * (n_sq - Nlq) / (n_sq - 2. * Nlq + 3) * cm2 * cm2 )
        / ( (n_sq - 3. * Nlq + 3.) - (6. * Nlq - 9.) * (n_sq - Nlq) / (Nlq * (n_sq - 2. * Nlq + 3.)) );
  }
  else
    Cerr << "Warning: due to small sample size, resorting to biased estimator "
	 << "conversion in NonDHierarchSampling::uncentered_to_centered().\n";
}


inline void NonDHierarchSampling::
centered_to_standard(Real  cm1, Real  cm2, Real  cm3, Real  cm4,
		     Real& sm1, Real& sm2, Real& sm3, Real& sm4)
{
  // convert from centered to standardized moments
  sm1 = cm1;                    // mean
  if (cm2 > 0.) {
    sm2 = std::sqrt(cm2);         // std deviation
    sm3 = cm3 / (cm2 * sm2);      // skewness
    sm4 = cm4 / (cm2 * cm2) - 3.; // excess kurtosis
  }
  else {
    Cerr << "\nWarning: central to standard conversion failed due to "
	 << "non-positive\n         variance.  Retaining central moments.\n";
    sm2 = 0.; sm3 = cm3; sm4 = cm4; // or assign NaN to sm{3,4}
  }
}


inline void NonDHierarchSampling::check_negative(Real& cm)
{
  if (cm < 0.) {
    Cerr << "\nWarning: central moment less than zero (" << cm << ").  "
	 << "Repairing to zero.\n";
    cm = 0.;
    // TO DO:  consider hard error if COV < -tol (pass in mean and cm order)
  }
}


inline Real NonDHierarchSampling::sum(const Real* vec, size_t vec_len) const
{
  Real sum = 0.;
  for (size_t i=0; i<vec_len; ++i)
    sum += vec[i];
  return sum;
}


inline Real NonDHierarchSampling::
average(const Real* vec, size_t vec_len) const
{ return sum(vec, vec_len) / (Real)vec_len; }


inline Real NonDHierarchSampling::average(const RealVector& vec) const
{ return average(vec.values(), vec.length()); }


inline Real NonDHierarchSampling::average(const SizetArray& sa) const
{
  size_t i, len = sa.size(), sum = 0;
  for (i=0; i<len; ++i)
    sum += sa[i];
  return (Real)sum / (Real)len;
}

} // namespace Dakota

#endif
