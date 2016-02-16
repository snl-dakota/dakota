/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
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

#include "dakota_data_types.hpp"
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

  /// standard constructor
  NonDAdaptImpSampling(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor for on-the-fly instantiations
  NonDAdaptImpSampling(Model& model, unsigned short sample_type, int samples,
		       int seed, const String& rng, bool vary_pattern,
		       unsigned short is_type, bool cdf_flag,
		       bool x_space_model, bool use_model_bounds,
		       bool track_extreme);
  /// destructor
  ~NonDAdaptImpSampling();

  //
  //- Heading: Virtual function redefinitions
  //

  bool resize();
  void derived_init_communicators(ParLevLIter pl_iter);
  void derived_set_communicators(ParLevLIter pl_iter);
  void derived_free_communicators(ParLevLIter pl_iter);

  /// performs adaptive importance sampling and computes probability of failure
  void core_run();

  /// print the final statistics
  void print_results(std::ostream& s);

  //
  //- Heading: Member functions
  //

  /// initializes data needed for importance sampling: an initial set
  /// of points around which to sample, a failure threshold, an
  /// initial probability to refine, and flags to control transformations
  void initialize(const RealVectorArray& full_points, bool x_space_data,
		  size_t resp_index, Real initial_prob, Real failure_threshold);
  /// initializes data needed for importance sampling: an initial set
  /// of points around which to sample, a failure threshold, an
  /// initial probability to refine, and flags to control transformations
  void initialize(const RealMatrix& full_points, bool x_space_data,
		  size_t resp_index, Real initial_prob, Real failure_threshold);
  /// initializes data needed for importance sampling: an initial
  /// point around which to sample, a failure threshold, an
  /// initial probability to refine, and flags to control transformations
  void initialize(const RealVector& full_point, bool x_space_data,
		  size_t resp_index, Real initial_prob, Real failure_threshold);

  /// returns the final probability calculated by the importance sampling
  Real final_probability();
  /// return extremeValues
  const RealRealPairArray& extreme_values() const;

protected:

private:

  //
  //- Heading: Convenience functions
  //

  //
  //- Heading: Utility routines
  //

  /// select representative points from a set of samples
  void select_rep_points(const RealVectorArray& var_samples_u,
			 const RealVector& fn_samples);

  /// iteratively generate samples and select representative points
  /// until probability and (optionally) coefficient of variation converge
  void converge_statistics(bool cov_flag);

  /// generate a set of samples based on multimodal sampling density
  void generate_samples(RealVectorArray& var_samples_u);
  /// evaluate the model at the sample points and store the responses
  void evaluate_samples(const RealVectorArray& var_samples_u,
		        RealVector& fn_samples);

  /// calculate the probability of exceeding the failure threshold and
  /// the coefficent of variation (if requested)
  void calculate_statistics(const RealVectorArray& var_samples_u,
			    const RealVector& fn_samples, size_t total_samples,
			    Real& sum_prob, Real& prob, bool compute_cov,
			    Real& sum_var, Real& cov);

  /// compute Euclidean distance between points a and b
  Real distance(const RealVector& a, const RealVector& b);
  /// compute density between a representative point 
  /// and a sample point, assuming standard normal
  Real recentered_density(const RealVector& sample_point);

  //
  //- Heading: Data members
  //

  // Note: requested/computed response/probability level arrays are managed
  // by NonD(Global/Local)Reliability, and the currently active scalars (for
  // a particular response function at a particular level) are passed though
  // initialize().

  /// importance sampling is performed in standardized probability space.
  /// This u-space model is either passed in (alternate constructor for
  /// helper AIS) or constructed using transform_model() (standard
  /// constructor for stand-alone AIS)
  Model uSpaceModel;

  /// integration type (is, ais, mmais) provided by input specification
  unsigned short importanceSamplingType;

  /// flag to identify if initial points are generated from an LHS sample
  bool initLHS;
  /// flag to control if the sampler should respect the model bounds
  bool useModelBounds;
  /// flag for inversion of probability values using 1.-p
  bool invertProb;
  /// flag for tracking min/max values encountered when evaluating samples
  bool trackExtremeValues;

  /// size of sample batch within each refinement iteration
  int refineSamples;

  /// the active response function index in the model to be sampled
  size_t respFnIndex;
  /// design subset for which uncertain subset is being sampled
  RealVector designPoint;
  /// the original set of u-space samples passed in initialize()
  RealVectorArray initPointsU;
  /// the set of representative points in u-space around which to sample
  RealVectorArray repPointsU;
  /// the weight associated with each representative point
  RealVector repWeights;

  /// the probability estimate that is iteratively refined by
  /// importance sampling
  Real probEstimate;
  /// the failure threshold (z-bar) for the problem.
  Real failThresh;
};


inline NonDAdaptImpSampling::~NonDAdaptImpSampling()
{ }


inline Real NonDAdaptImpSampling::final_probability()
{ return probEstimate; }


inline const RealRealPairArray& NonDAdaptImpSampling::extreme_values() const
{ return extremeValues; }


inline Real NonDAdaptImpSampling::
distance(const RealVector& a, const RealVector& b)
{
  size_t len = a.length();
  if (b.length() != len) {
    Cerr << "Error: inconsistent vector length in NonDAdaptImpSampling::"
	 << "distance()" << std::endl;
    abort_handler(-1);
  }

  Real amb, dist_sq = 0.;
  for (size_t j=0; j<len; ++j)
    { amb = a[j] - b[j]; dist_sq += amb * amb; }
  return std::sqrt(dist_sq);
}

} // namespace Dakota

#endif
