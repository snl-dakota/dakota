/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef SAMPLER_DRIVER_H
#define SAMPLER_DRIVER_H

#include "dakota_data_types.hpp"
#include "pecos_data_types.hpp"

namespace Dakota {

// This class provides a common interface for the sampler drivers
class SamplerDriver {
public:
    // Destructor
    virtual ~SamplerDriver() = default;

    // Common methods for sampler drivers
    virtual void generate_uniform_samples(
        const RealVector& lowerBounds,
        const RealVector& upperBounds,
        RealSymMatrix& correlationMatrix,
        size_t numSamples,
        RealMatrix& samplesArray
    ) = 0;

    virtual void generate_normal_samples(
        const RealVector& means,
        const RealVector& standardDeviations,
        const RealVector& lowerBounds,
        const RealVector& upperBounds,
        RealSymMatrix& correlationMatrix,
        size_t numSamples,
        RealMatrix& samplesArray
    ) = 0;

    virtual void generate_unique_samples(
        const Model& model,
        size_t numSamples,
        RealMatrix& samplesArray,
        RealMatrix& sampleRanks,
        const BitArray& activeVariables,
        const BitArray& activeCorrelations
    ) = 0;

    virtual void generate_samples(
        const Model& model,
        size_t numSamples,
        RealMatrix& samplesArray,
        RealMatrix& sampleRanks,
        const BitArray& activeVariables,
        const BitArray& activeCorrelations
    ) = 0;

    virtual void rng(String rngName) = 0;

    virtual void seed(int randomSeed) = 0;

    virtual void advance_seed_sequence() = 0;

    virtual int seed() = 0;

    virtual void initialize(
        const String& sampleType,
		short sampleRanksMode,
        bool reports
    ) = 0;
};

} // Namespace Dakota

#endif