/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef LD_DRIVER_ADAPTER_H
#define LD_DRIVER_ADAPTER_H

#include "dakota_data_types.hpp"
#include "DakotaModel.hpp"
#include "LDDriver.hpp"
#include "LowDiscrepancySequence.hpp"
#include "ProblemDescDB.hpp"
#include "SamplerDriver.hpp"

namespace Dakota {

// Adapter for LDDriver
class LDDriverAdapter : public SamplerDriver {
private:
    // Composition, contains an instance of LDDriver
    LDDriver ldDriver;

public:
    // Constructor
    LDDriverAdapter(ProblemDescDB& problem_db);

    // Override methods from SamplerAdapter
    void generate_uniform_samples(
        const RealVector& lowerBounds,
        const RealVector& upperBounds,
        RealSymMatrix& correlationMatrix,
        size_t numSamples,
        RealMatrix& samplesArray
    ) override {
        // Model model;
        // model.continuous_lower_bounds(lowerBounds);
        // model.continuous_upper_bounds(upperBounds);
        // for (int j = 0; j < lowerBounds.length(); ++j) {
        //     model.continuous_variable_type(UNIFORM_UNCERTAIN, j);
        // }
        // ldDriver.generate_samples(model, numSamples, samplesArray);
        Cerr << "\nError: 'generate_uniform_samples' currently not supported for low-discrepancy sampling" << std::endl;
        abort_handler(METHOD_ERROR);
    }

    void generate_normal_samples(
        const RealVector& means,
        const RealVector& standardDeviations,
        const RealVector& lowerBounds,
        const RealVector& upperBounds,
        RealSymMatrix& correlationMatrix,
        size_t numSamples,
        RealMatrix& samplesArray
    ) override {
        // Model model;
        // model.continuous_lower_bounds(lowerBounds);
        // model.continuous_upper_bounds(upperBounds);
        // for (int j = 0; j < lowerBounds.length(); ++j) {
        //     model.continuous_variable_type(NORMAL_UNCERTAIN, j);
        // }
        // ldDriver.generate_samples(model, numSamples, samplesArray);

        // // Map from standard normal to given distribution
        // for (int row = 0; row < samplesArray.numRows(); ++row) {
        //     for (int col = 0; col < samplesArray.numCols(); ++col) {
        //         samplesArray(row, col) = means[row] + standardDeviations[row] * samplesArray(row, col);
        //     }
        // }
        Cerr << "\nError: 'generate_normal_samples' currently not supported for low-discrepancy sampling" << std::endl;
        abort_handler(METHOD_ERROR);
    }

    void generate_unique_samples(
        const Model& model,
        size_t numSamples,
        RealMatrix& samplesArray,
        RealMatrix& sampleRanks,
        const BitArray& activeVariables,
        const BitArray& activeCorrelations
    ) override {
        Cerr << "\nError: low-discrepancy sampling does not support discrete random variables." << std::endl;
        abort_handler(METHOD_ERROR);
    }

    void generate_samples(
        const Model& model,
        size_t numSamples,
        RealMatrix& samplesArray,
        RealMatrix& sampleRanks,
        const BitArray& activeVariables,
        const BitArray& activeCorrelations
    ) override;

    void rng(String rngName) override {
        if (rngName != "mt19937") {
            Cerr << "\nError: low-discrepancy sampling only supports 'mt19937' as RNG algorithm" << std::endl;
            abort_handler(METHOD_ERROR);
        }
    }

    void seed(int randomSeed) override;

    void advance_seed_sequence() override {
        Cerr << "\nError: low-discrepancy sampling does not support 'rnum2' as RNG algorithm" << std::endl;
        abort_handler(METHOD_ERROR);
    }

    int seed() override;

    void initialize(
        const String& sampleType,
		short sampleRanksMode,
        bool reports
    ) override {
        // Nothing to be done here
    }
};

} // Namespace Dakota

#endif