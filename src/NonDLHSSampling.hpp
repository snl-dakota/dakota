/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDLHSSampling
//- Description: Wrapper class for Fortran 90 LHS library
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#ifndef NOND_LHS_SAMPLING_H
#define NOND_LHS_SAMPLING_H

#include "NonDSampling.hpp"
#include "DataMethod.hpp"


namespace Dakota {

/// Performs LHS and Monte Carlo sampling for uncertainty quantification.

/** The Latin Hypercube Sampling (LHS) package from Sandia
    Albuquerque's Risk and Reliability organization provides
    comprehensive capabilities for Monte Carlo and Latin Hypercube
    sampling within a broad array of user-specified probabilistic
    parameter distributions.  It enforces user-specified rank
    correlations through use of a mixing routine.  The NonDLHSSampling
    class provides a C++ wrapper for the LHS library and is used for
    performing forward propagations of parameter uncertainties into
    response statistics. 

    Batch generation options, including D-Optimal and incremental LHS
    are provided.

    The incremental LHS sampling capability allows one to supplement
    an initial sample of size n to size 2n while maintaining the
    correct stratification of the 2n samples and also maintaining the
    specified correlation structure.  The incremental version of LHS
    will return a sample of size n, which when combined with the
    original sample of size n, allows one to double the size of the
    sample.
*/
class NonDLHSSampling: public NonDSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDLHSSampling(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor for sample generation and evaluation "on the fly"
  NonDLHSSampling(Model& model, unsigned short sample_type,
		  int samples, int seed, const String& rng,
		  bool vary_pattern = true, short sampling_vars_mode = ACTIVE);
  /// alternate constructor for uniform sample generation "on the fly"
  NonDLHSSampling(unsigned short sample_type, int samples, int seed,
		  const String& rng, const RealVector& lower_bnds,
		  const RealVector& upper_bnds);
  /// alternate constructor for sample generation of correlated normals
  /// "on the fly"
  NonDLHSSampling(unsigned short sample_type, int samples, int seed,
		  const String& rng, const RealVector& means, 
                  const RealVector& std_devs, const RealVector& lower_bnds,
		  const RealVector& upper_bnds, RealSymMatrix& correl);
  /// destructor
  ~NonDLHSSampling();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  /// increment to next in sequence of refinement samples
  void sampling_increment();

  // together the three run components perform a forward uncertainty
  // propagation by using LHS to generate a set of parameter samples,
  // performing function evaluations on these parameter samples, and
  // computing statistics on the ensemble of results.

  /// generate LHS samples in non-VBD cases
  void pre_run();
  /// perform the evaluate parameter sets portion of run
  void core_run();
  /// generate statistics for LHS runs in non-VBD cases
  void post_run(std::ostream& s);

  void post_input();

  /// update finalStatistics and (if MC sampling) finalStatErrors
  void update_final_statistics();

  /// compute a principal components analysis on the sample set
  void compute_pca(std::ostream& s);

  /// print the final statistics
  void print_results(std::ostream& s);

  //
  //- Heading: Member functions
  //

  /// generate a d-optimal parameter set, leaving the first
  /// previous_samples columns intact and adding new_samples new
  /// columns following them
  void d_optimal_parameter_set(int previous_samples, int new_samples, 
                               RealMatrix& full_samples);

  /// Populate the first new_samples columns of allSamples with an LHS
  /// design and update the stored ranks
  void initial_increm_lhs_set(int new_samples, 
                              RealMatrix& full_samples, IntMatrix& full_ranks);

  /// generate a new batch that is Latin w.r.t. the previous samples
  void increm_lhs_parameter_set(int previous_samples, int new_samples,
                                RealMatrix& full_samples, IntMatrix& all_ranks);

  /// store the ranks of the last generated sample for continuous
  /// (based on sampleRanks) and calculate/store discrete ranks
  void store_ranks(const RealMatrix& sample_values, IntMatrix &sample_ranks);

  /// store the combined ranks from sampleRanks to leading submatrix
  /// local cached ranks matrix
  void store_ranks(IntMatrix& full_ranks);

  /// merge the discrete ranks into a submatrix of sampleRanks
  void combine_discrete_ranks(const RealMatrix& initial_values, 
                              const RealMatrix& increm_values);

  /// sort algorithm to compute ranks for rank correlations
  static bool rank_sort(const int& x, const int& y);

private:

  //
  //- Heading: Data
  //

  /// number of response functions; used to distinguish NonD from opt/NLS usage
  size_t numResponseFunctions;

  /// list of refinement sample batch sizes
  IntVector refineSamples;

  /// whether to generate d-optimal point sets
  bool dOptimal;

  /// number of candidate designs to generate for classical D-optimal designs
  size_t numCandidateDesigns;

  /// oversampling ratio for Leja D-optimal candidate set generation
  Real oversampleRatio;

  /// static data used by static rank_sort() fn
  static RealArray rawData;

  /// flags computation of variance-based decomposition indices
  bool varBasedDecompFlag;

  /// flag to specify the calculation of principal components
  bool pcaFlag;
  /// Threshold to keep number of principal components that explain 
  /// this much variance
  Real percentVarianceExplained;
};

} // namespace Dakota

#endif
