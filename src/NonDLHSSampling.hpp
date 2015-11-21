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
    response statistics. */

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

  // together the three run components perform a forward uncertainty
  // propagation by using LHS to generate a set of parameter samples,
  // performing function evaluations on these parameter samples, and
  // computing statistics on the ensemble of results.

  /// generate LHS samples in non-VBD cases
  void pre_run();
  /// perform the evaluate parameter sets portion of run
  void quantify_uncertainty();
  /// generate statistics for LHS runs in non-VBD cases
  void post_run(std::ostream& s);

  void post_input();

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

private:

  //
  //- Heading: Data
  //

  /// number of response functions; used to distinguish NonD from opt/NLS usage
  size_t numResponseFunctions;

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
