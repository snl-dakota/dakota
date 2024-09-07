/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "LHSDriverAdapter.hpp"
#include "DakotaModel.hpp"
#include "MultivariateDistribution.hpp"

namespace Dakota {

// Constructor
LHSDriverAdapter::LHSDriverAdapter() : lhsDriver() { }

// Override methods from SamplerAdapter
void LHSDriverAdapter::generate_uniform_samples(
    const RealVector& lowerBounds,
    const RealVector& upperBounds,
    RealSymMatrix& correlationMatrix,
    size_t numSamples,
    RealMatrix& samplesArray
) { 
    lhsDriver.generate_uniform_samples(lowerBounds, upperBounds, correlationMatrix, numSamples, samplesArray);
}

void LHSDriverAdapter::generate_normal_samples(
    const RealVector& means,
    const RealVector& standardDeviations,
    const RealVector& lowerBounds,
    const RealVector& upperBounds,
    RealSymMatrix& correlationMatrix,
    size_t numSamples,
    RealMatrix& samplesArray
) {
    lhsDriver.generate_normal_samples(means, standardDeviations, lowerBounds, upperBounds, correlationMatrix, numSamples, samplesArray);
}

void LHSDriverAdapter::generate_unique_samples(
    const Model& model,
    size_t numSamples,
    RealMatrix& samplesArray,
    RealMatrix& sampleRanks,
    const BitArray& activeVariables,
    const BitArray& activeCorrelations
) {
    const Pecos::MultivariateDistribution& multivariateDistribution = model.multivariate_distribution();
    lhsDriver.generate_unique_samples(multivariateDistribution.random_variables(), multivariateDistribution.correlation_matrix(), numSamples, samplesArray, sampleRanks, activeVariables, activeCorrelations);
}

void LHSDriverAdapter::generate_samples(
    const Model& model,
    size_t numSamples,
    RealMatrix& samplesArray,
    RealMatrix& sampleRanks,
    const BitArray& activeVariables,
    const BitArray& activeCorrelations
) {
    const Pecos::MultivariateDistribution& multivariateDistribution = model.multivariate_distribution();
    lhsDriver.generate_samples(multivariateDistribution.random_variables(), multivariateDistribution.correlation_matrix(), numSamples, samplesArray, sampleRanks, activeVariables, activeCorrelations);
}

void LHSDriverAdapter::rng(String rngName) {
    lhsDriver.rng(rngName);
}

void LHSDriverAdapter::seed(int randomSeed) {
    lhsDriver.seed(randomSeed);
}

void LHSDriverAdapter::advance_seed_sequence() {
    lhsDriver.advance_seed_sequence();
}

int LHSDriverAdapter::seed() const {
    return lhsDriver.seed();
}

void LHSDriverAdapter::initialize(
    const String& sampleType,
    short sampleRanksMode,
    bool reports
) {
    lhsDriver.initialize(sampleType, sampleRanksMode, reports);
}

} // Namespace Dakota