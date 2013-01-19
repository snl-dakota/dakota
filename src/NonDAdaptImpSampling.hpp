/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDAdaptImpSampling
//- Description: Class for the Adaptive Importance Sampling methods
//- Owner:	 Barron Bichon and Laura Swiler
//- Checked by:
//- Version:

#ifndef NOND_ADAPT_IMP_SAMPLING_H
#define NOND_ADAPT_IMP_SAMPLING_H

#include "data_types.hpp"
#include "NonDSampling.hpp"
#include "DakotaModel.hpp"
#include "DakotaIterator.hpp"

namespace Dakota {


/// Class for the Adaptive Importance Sampling methods within DAKOTA

/** The NonDAdaptImpSampling implements the multi-modal adaptive importance 
    sampling used for reliability calculations.  (eventually we will want 
    to broaden this).  Need to add more detail to this description. */

class NonDAdaptImpSampling: public NonDSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// constructors
  NonDAdaptImpSampling(Model& model); ///< standard constructor
  
  NonDAdaptImpSampling(Model& model, const String& sample_type, int samples,
		       int seed, const String& rng, bool vary_pattern,
		       short is_type, bool cdf_flag, bool x_space_data,
		       bool x_space_model, bool bounded_model);

  ~NonDAdaptImpSampling(); ///< destructor

  //
  //- Heading: Member functions
  //

  /// performs an adaptive importance sampling and returns probability of 
  /// failure. 
  void quantify_uncertainty();

  /// initializes data needed for importance sampling: an initial set
  /// of points around which to sample, a failure threshold, an
  /// initial probability to refine, and flags to control transformations
  void initialize(const RealVectorArray& initial_points, int resp_fn,
		  const Real& initial_prob, const Real& failure_threshold);

  /// initializes data needed for importance sampling: an initial
  /// point around which to sample, a failure threshold, an
  /// initial probability to refine, and flags to control transformations
  void initialize(const RealVector& initial_point, int resp_fn,
		  const Real& initial_prob, const Real& failure_threshold);

  /// returns the probability calculated by the importance sampling
  const Real& get_probability();

  /// print the final statistics
  void print_results(std::ostream& s);

private:

  //
  //- Heading: Convenience functions
  //

  //
  //- Heading: Utility routines
  //

  /// iteratively generate samples and select representative points
  /// until coefficient of variation converges
  void converge_cov();

  /// iteratively generate samples from final set of representative points
  /// until probability converges
  void converge_probability();

  /// select representative points from initial set of samples
  void select_init_rep_points(const RealVectorArray& samples);

  /// select representative points from a set of samples
  void select_rep_points(const RealVectorArray& samples);

  /// calculate relative weights of representative points
  void calculate_rep_weights();

  /// generate a set of samples based on multimodal sampling density
  void generate_samples(RealVectorArray& samples);

  /// calculate the probability of exceeding the failure threshold and
  /// the coefficent of variation (if requested)
  void calculate_statistics(const RealVectorArray& samples,
			    const size_t& total_sample_number,
			    Real& probability_sum,
			    Real& probability,
			    bool  cov_flag,
			    Real& variance_sum,
			    Real& coeff_of_variation);

  //
  //- Heading: Data members
  //

  /// integration type (is, ais, mmais) provided by input specification
  short importanceSamplingType;
  /// flag for inversion of probability values using 1.-p
  bool invertProb;
  /// the number of representative points around which to sample
  size_t numRepPoints;
  /// the response function in the model to be sampled
  size_t respFn;
  /// the original set of samples passed into the MMAIS routine
  RealVectorArray initPoints;
  /// the set of representative points around which to sample
  RealVectorArray repPoints;
  /// the weight associated with each representative point
  RealVector repWeights;
  /// design point at which uncertain space is being sampled
  RealVector designPoint;
  /// flag to control if x->u transformation should be performed for
  /// initial points
  bool transInitPoints;
  /// flag to control if u->x transformation should be performed
  /// before evaluation
  bool transPoints;
  /// flag to control if the sampler should respect the model bounds
  bool useModelBounds;
  /// flag to identify if initial points are generated from an LHS sample
  bool initLHS;
  /// the initial probability (from FORM or SORM)
  Real initProb;
  /// the final calculated probability (p)
  Real finalProb;
  /// the failure threshold (z-bar) for the problem.
  Real failThresh;
  // Note: requested/computed response/probability level arrays are managed
  // by NonD(Global/Local)Reliability, and the currently active scalars (for
  // a particular response function at a particular level) are passed though
  // initialize().
};


inline NonDAdaptImpSampling::~NonDAdaptImpSampling()
{ }


inline const Real& NonDAdaptImpSampling::get_probability()
{ return finalProb; }

} // namespace Dakota

#endif
