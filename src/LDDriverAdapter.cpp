/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "LDDriverAdapter.hpp"
#include "ProblemDescDB.hpp"

namespace Dakota {

// Constructor
LDDriverAdapter::LDDriverAdapter(ProblemDescDB& problem_db) : ldDriver(problem_db) { }

// Override methods from SamplerAdapter
void LDDriverAdapter::generate_samples(
    const Model& model,
    size_t numSamples,
    RealMatrix& samplesArray,
    RealMatrix& sampleRanks,
    const BitArray& activeVariables,
    const BitArray& activeCorrelations
) {
    ldDriver.generate_samples(model, numSamples, samplesArray);
}

void LDDriverAdapter::seed(int randomSeed) {
    ldDriver.set_seed(randomSeed);
}

int LDDriverAdapter::seed() {
    return ldDriver.get_seed();
}

} // Namespace Dakota