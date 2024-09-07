/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef LOW_DISCREPANCY_DRIVER_H
#define LOW_DISCREPANCY_DRIVER_H

#include "dakota_data_types.hpp"
#include "LowDiscrepancySequence.hpp"
#include "MultivariateDistribution.hpp"
#include "ProblemDescDB.hpp"

namespace Dakota {

// Wrapper around a LowDiscrepancySequence
class LDDriver
{
public:

    // Default constructor
    LDDriver(ProblemDescDB& problem_db);

    // Get the seed of the wrapped low-discrepancy sequence
    int get_seed();

    // Set the seed of the wrapped low-discrepancy sequence
    void set_seed(int seed);

    // Generate samples of the wrapped low-discrepancy sequence
    void generate_samples(
        const Model& model,      // The model to get the multivariate distribution from
        const size_t numSamples, // Number of samples to generate
        RealMatrix& sampleMatrix // Matrix to store the generated samples (needs to be resized to numVariables x numSamples)
    );

private:

    // A pointer to a low-discrepancy sequence
    std::unique_ptr<LowDiscrepancySequence> sequence_;

    // The number of low-discrepancy samples already taken
    std::size_t numSamples_;

    /// Check if the given multivariate distribution is supported by 
    /// low-discrepancy sampling methods
    void check_support(
        Pecos::MultivariateDistribution& mv_dist
    );

    // Ensure the given sample matrix has the given shape
    void reshape_sample_matrix(
        RealMatrix& sampleMatrix,
        int numRows,
        int numCols
    );

    // Function to scale a given sample matrix from a uniform distribution over [0, 1)
    // to a uniform distribution between given lower and upper bounds
    void scale(
        const RealVector& lowerBounds,
        const RealVector& upperBounds, 
        RealMatrix& sampleMatrix       // The matrix of samples to transform (shape numVariables x numSamples)
    );

    // Function to transform a given sample matrix from a uniform distribution over [0, 1)
    // to the multivariate distribution defined by the given model
    void transform(
        const Model& model,      // The model to get the target multivariate distribution from
        RealMatrix& sampleMatrix // The matrix of samples to transform (shape numVariables x numSamples)
    );
};

} // Namespace Dakota

#endif