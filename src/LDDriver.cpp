/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_data_types.hpp"
#include "DakotaModel.hpp"
#include"DigitalNet.hpp"
#include "LDDriver.hpp"
#include "ProbabilityTransformModel.hpp"
#include "ProblemDescDB.hpp"
#include "RandomVariable.hpp"
#include "Rank1Lattice.hpp"

namespace Dakota {

// Default constructor
LDDriver::LDDriver(ProblemDescDB& problem_db) :
    sequence_(
        problem_db.get_bool("method.rank_1_lattice") ? 
            static_cast<LowDiscrepancySequence*>(new Rank1Lattice(problem_db)) :
            static_cast<LowDiscrepancySequence*>(new DigitalNet(problem_db))
    ),
    numSamples_(0)
{

}

// Get the seed of the wrapped low-discrepancy sequence
int LDDriver::get_seed() {
    return sequence_->get_seed();
}

// Set the seed of the wrapped low-discrepancy sequence
void LDDriver::set_seed(int seed) {
    sequence_->set_seed(seed);
}

// Generate samples of the wrapped low-discrepancy sequence
void LDDriver::generate_samples(
    std::shared_ptr<Model> model,      // The model to get the multivariate distribution from
    const size_t numSamples, // Number of samples to generate
    RealMatrix& sampleMatrix // Matrix to store the generated samples (needs to be resized to numVariables x numSamples)
)
{
    // Reshape the sample matrix if needed
    reshape_sample_matrix(sampleMatrix, ModelUtils::cv(*model), numSamples);

    // Generate points from the low-discrepancy sequence
    sequence_->get_points(numSamples_, numSamples_ + numSamples, sampleMatrix);
        
    // Transform points from [0, 1) to the multivariate distribution in the given model
    transform(model, sampleMatrix);

    // Update sample counter
    numSamples_ += numSamples;
}

// Check if the given multivariate distribution is supported by 
// low-discrepancy sampling methods
void LDDriver::check_support(
  Pecos::MultivariateDistribution& mv_dist
)
{
    // Loop over each variable and check if its type is allowed
    // TODO: is there a better way to do this, e.g., variable.is_discrete()?
    std::vector<Pecos::RandomVariable>& variables = mv_dist.random_variables();
    for ( Pecos::RandomVariable variable : variables )
    {
        auto variable_type = variable.type();
        if (    
            ( variable_type != Pecos::STD_NORMAL )        && 
            ( variable_type != Pecos::NORMAL )            && 
            ( variable_type != Pecos::BOUNDED_NORMAL )    && 
            ( variable_type != Pecos::LOGNORMAL )         && 
            ( variable_type != Pecos::BOUNDED_LOGNORMAL ) && 
            ( variable_type != Pecos::STD_UNIFORM )       && 
            ( variable_type != Pecos::UNIFORM )           && 
            ( variable_type != Pecos::LOGUNIFORM )        && 
            ( variable_type != Pecos::TRIANGULAR )        && 
            ( variable_type != Pecos::STD_EXPONENTIAL )   && 
            ( variable_type != Pecos::EXPONENTIAL )       && 
            ( variable_type != Pecos::STD_BETA )          && 
            ( variable_type != Pecos::BETA )              && 
            ( variable_type != Pecos::STD_GAMMA )         && 
            ( variable_type != Pecos::GAMMA )             && 
            ( variable_type != Pecos::GUMBEL )            && 
            ( variable_type != Pecos::FRECHET )           && 
            ( variable_type != Pecos::WEIBULL )           && 
            ( variable_type != Pecos::HISTOGRAM_BIN )     && 
            ( variable_type != Pecos::INV_GAMMA )         &&
            ( variable_type != Pecos::CONTINUOUS_RANGE )
        )
        {
            Cerr << "\nError: low-discrepancy sampling does not support discrete "
                << "random variables." << std::endl;
            abort_handler(METHOD_ERROR);
        }
    }
}

// Ensure the given sample matrix has the given shape
void LDDriver::reshape_sample_matrix(
    RealMatrix& sampleMatrix,
    int numRows,
    int numCols
)
{
    if (sampleMatrix.numRows() != numRows || sampleMatrix.numCols() != numCols) {
        sampleMatrix.shape(numRows, numCols);
    }
}

// Function to scale a given sample matrix from a uniform distribution over [0, 1)
// to a uniform distribution between given lower and upper bounds
void LDDriver::scale(
  const RealVector& lowerBounds,
  const RealVector& upperBounds,
  RealMatrix& sampleMatrix       // The matrix of samples to transform (shape numVariables x numSamples)
)
{
  auto numVariables = sampleMatrix.numRows(); // # variables = # rows
  auto numSamples = sampleMatrix.numCols();   // # samples = # column

  // Loop over each sample / column
  for (size_t col=0; col < numSamples; col++) 
  {
    // Loop over each row / parameter
    for (size_t row=0; row < numVariables; row++)
    {
      // Scale from [0, 1) to [lower, upper)
      Real lowerBound = lowerBounds[row];
      Real upperBound = upperBounds[row];
      sampleMatrix[col][row] = sampleMatrix[col][row] * (upperBound - lowerBound) + lowerBound;
    }
  }
}

// Function to transform a given sample matrix from a uniform distribution over [0, 1)
// to the multivariate distribution defined by the given model
void LDDriver::transform(
    std::shared_ptr<Model> model,      // The model to get the target multivariate distribution from
    RealMatrix& sampleMatrix // The matrix of samples to transform (shape numVariables x numSamples)
)
{
    // Transform the samples to [-1, 1]
    auto numVariables = sampleMatrix.numRows();
    auto numSamples = sampleMatrix.numCols();
    RealVector lowerBounds(numVariables);
    RealVector upperBounds(numVariables);
    lowerBounds = -1.0;
    upperBounds = 1.0;
    scale(lowerBounds, upperBounds, sampleMatrix); // transform from [0, 1) to [-1, 1)

    // If correlated, tranform samples to standard normal first
    if ( model->multivariate_distribution().correlation() )
    {
        // vSpaceModel has uncorrelated standard normal random variables
        auto vSpaceModel = std::make_shared<ProbabilityTransformModel>(model, STD_NORMAL_U);
        auto uSpaceModel = std::make_shared<ProbabilityTransformModel>(vSpaceModel, STD_UNIFORM_U);
        // First transform from standard uniform to standard normal
        Pecos::ProbabilityTransformation& uNataf = 
          uSpaceModel->probability_transformation();
        for (size_t sample = 0; sample < numSamples; ++sample) {
            RealVector uSample(Teuchos::Copy, sampleMatrix[sample], numVariables);
            RealVector xSample(Teuchos::View, sampleMatrix[sample], numVariables);
            uNataf.trans_U_to_X(uSample, 
			    ModelUtils::continuous_variable_ids(*model), 
			    xSample, ModelUtils::continuous_variable_ids(*model));
        }

        // Then transform from standard normal to actual model
        Pecos::ProbabilityTransformation& vNataf = 
        vSpaceModel->probability_transformation();
        for (size_t sample = 0; sample < numSamples; ++sample) {
            RealVector uSample(Teuchos::Copy, sampleMatrix[sample], numVariables);
            RealVector xSample(Teuchos::View, sampleMatrix[sample], numVariables);
            vNataf.trans_U_to_X(uSample,
			    ModelUtils::continuous_variable_ids(*model), 
			    xSample, ModelUtils::continuous_variable_ids(*model));
        }
    }
    else // If uncorrelated, directly apply the transform
    {
        // uSpaceModel has uncorrelated standard uniform random variables
        auto uSpaceModel = std::make_shared<ProbabilityTransformModel>(model, STD_UNIFORM_U);
        // Transform samples using Nataf transformation (component-wise inverse CDF)
        Pecos::ProbabilityTransformation& uNataf = 
          uSpaceModel->probability_transformation();
        for (size_t sample = 0; sample < numSamples; ++sample) {
            RealVector uSample(Teuchos::Copy, sampleMatrix[sample], numVariables);
            RealVector xSample(Teuchos::View, sampleMatrix[sample], numVariables);
            uNataf.trans_U_to_X(uSample, 
			    ModelUtils::continuous_variable_ids(*model),
			    xSample, ModelUtils::continuous_variable_ids(*model));
        }
    } 
}

} // Namespace Dakota
