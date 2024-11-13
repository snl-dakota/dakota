/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef LHS_DRIVER_ADAPTER_H
#define LHS_DRIVER_ADAPTER_H

#include "dakota_data_types.hpp"
#include "LHSDriver.hpp"
#include "SamplerDriver.hpp"

namespace Dakota {

// Adapter for LHSDriver
class LHSDriverAdapter : public SamplerDriver {
private:
    // Composition, contains an instance of LHSDriver
    Pecos::LHSDriver lhsDriver;

public:
    // Constructor
    LHSDriverAdapter();

    // Override methods from SamplerAdapter
    void generate_uniform_samples(
        const RealVector& lowerBounds,
        const RealVector& upperBounds,
        RealSymMatrix& correlationMatrix,
        size_t numSamples,
        RealMatrix& samplesArray
    ) override;

    void generate_normal_samples(
        const RealVector& means,
        const RealVector& standardDeviations,
        const RealVector& lowerBounds,
        const RealVector& upperBounds,
        RealSymMatrix& correlationMatrix,
        size_t numSamples,
        RealMatrix& samplesArray
    ) override;

    void generate_unique_samples(
        const Model& model,
        size_t numSamples,
        RealMatrix& samplesArray,
        RealMatrix& sampleRanks,
        const BitArray& activeVariables,
        const BitArray& activeCorrelations
    ) override;

    void generate_samples(
        const Model& model,
        size_t numSamples,
        RealMatrix& samplesArray,
        RealMatrix& sampleRanks,
        const BitArray& activeVariables,
        const BitArray& activeCorrelations
    ) override;

    void rng(String rngName) override;

    void seed(int randomSeed) override;

    void advance_seed_sequence() override;

    int seed() override;

    void initialize(
        const String& sampleType,
		short sampleRanksMode,
        bool reports
    ) override;
};

} // Namespace Dakota

#endif