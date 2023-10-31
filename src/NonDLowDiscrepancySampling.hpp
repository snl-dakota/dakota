/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_LOW_DISCREPANCY_SAMPLING_H
#define NOND_LOW_DISCREPANCY_SAMPLING_H

#include "dakota_data_types.hpp"
#include "NonDLHSSampling.hpp"
#include "LowDiscrepancySequence.hpp"

namespace Dakota {

/// Class for low-discrepancy sampling in Dakota

/**
This is a templated adapter class that holds a reference to an object of type
`LowDiscrepancySequence`. It's main function is to provide the implementation
of the function `get_parameter_sets`, and to provide mappings from [0, 1) to
the distribution of interest (TODO: future work).

New low-discrepancy sequences only need to inherit from a
`LowDiscrepancySequence` and provide implementation of a function
`get_points` that generates the low-discrepancy points in [0, 1).

NOTE: `NonDRank1LatticeSampling` inherits from` NonDLHSSampling` so we can 
reuse the cool features such as global sensitivity analysis, 
reliability analysis etc.
*/
class NonDLowDiscrepancySampling: public NonDLHSSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// Default constructor
  NonDLowDiscrepancySampling(
    ProblemDescDB& problem_db,
    Model& model
  );

  /// Destructor
  ~NonDLowDiscrepancySampling();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  /// Use the distributions and/or bounds in the given model to generate
  /// the rank-1 lattice points
  void get_parameter_sets(
    Model& model
  );

  /// Same as above, but store the lattice points in the given matrix
  void get_parameter_sets(
    Model& model,
    const size_t num_samples, 
    RealMatrix& sample_matrix
  );
                          
  /// Same as above, but allow verbose outputs
  void get_parameter_sets(
    Model& model,
    const size_t num_samples,
    RealMatrix& sample_matrix,
    bool write_message
  );

  /// Generate a set of rank-1 lattice points using the given lower and upper
  /// bounds and store the results in `allSamples`
  void get_parameter_sets(
    const RealVector& lower,
    const RealVector& upper
  );

  /// Generate a set of normally-distributed points by mapping the rank-1
  /// lattice points using the inverse normal cdf
  void get_parameter_sets(
    const RealVector& means,
    const RealVector& std_devs,
    const RealVector& lower,
    const RealVector& upper,
    RealSymMatrix& correl
  );

  // 
  // - Heading: Member functions
  // 

private:

  //
  // - Heading: Data
  //

  /// The low-discrepancy sequence to sample from
  std::unique_ptr<LowDiscrepancySequence> sequence;

  /// Keep track of how many samples are already generated when using
  /// refinement samples
  int colPtr;

  /// Function to transform a given sample matrix from [0, 1) to the 
  /// probability density functions given in the model
  /// Assumes that the sample matrix has shape `numParams` x `numSamples`
  void transform(
    Model& model,
    RealMatrix& sample_matrix
  );

  /// Function to scale a given sample matrix from [0, 1) to the given lower
  /// and upper bounds
  /// Assumes that the sample matrix has shape `numParams` x `numSamples`
  void scale(
    const RealVector& lower,
    const RealVector& upper,
    RealMatrix& sample_matrix
  );

  /// Check if this multivariate distribution is supported by low-discrepancy
  /// sampling methods
  void check_support(
    Pecos::MultivariateDistribution& mv_dist
  );
  
};

} // namespace Dakota

#endif