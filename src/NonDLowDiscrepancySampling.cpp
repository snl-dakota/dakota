/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_data_types.hpp"
#include "NonDLowDiscrepancySampling.hpp"
#include "ProbabilityTransformModel.hpp"
#include "Rank1Lattice.hpp"
#include "DigitalNet.hpp"

#include <boost/math/special_functions/erf.hpp>

#include <ciso646> // macros for logical operators (e.g. not for !)
                   // Needed by MSVS.

namespace Dakota {

/// Default constructor
NonDLowDiscrepancySampling::NonDLowDiscrepancySampling(
  ProblemDescDB& problem_db,
  Model& model
) : 
NonDLHSSampling(problem_db, model),
sequence(
  /// NOTE: static cast because the ternary conditional operator requires its 
  /// second and third operands to have the same type.
  problem_db.get_bool("method.rank_1_lattice") ? 
    static_cast<LowDiscrepancySequence*>(new Rank1Lattice(problem_db)) :
    static_cast<LowDiscrepancySequence*>(new DigitalNet(problem_db))
), 
colPtr(0)
{

}

// Default destructor
NonDLowDiscrepancySampling::~NonDLowDiscrepancySampling( )
{

}

/// Use the distributions and/or bounds in the given model to generate
/// the rank-1 lattice points
void NonDLowDiscrepancySampling::get_parameter_sets(
  Model& model
)
{
  get_parameter_sets(model, numSamples, allSamples);
}

/// Same as above, but store the lattice points in the given matrix
void NonDLowDiscrepancySampling::get_parameter_sets(
  Model& model,
  const size_t num_samples, 
  RealMatrix& sample_matrix
)
{
  get_parameter_sets(model, num_samples, sample_matrix, true);
}
                        
/// Same as above, but allow verbose outputs
/// NOTE: Check if this method correctly handles active variables
void NonDLowDiscrepancySampling::get_parameter_sets(
  Model& model,
  const size_t num_samples,
  RealMatrix& sample_matrix,
  bool write_message
)
{
  /// Check if low-discrepancy sampling supports the random variables 
  /// associated with this model
  check_support(model.multivariate_distribution());

  /// NOTE: this 'switch case' is copied from 'NonDSampling', this should be
  /// refactored as soon as coupling between 'NonDSampling' and 'lhsDriver'
  /// has been removed
  switch (samplingVarsMode) {

    /// NOTE: This mode gets called when 'method' is 'sampling' in the Dakota
    /// input file
    case ACTIVE:
    /// NOTE: Same remark as above regarding the 'backfillDuplicates' flag
    /// The only difference with the 'ACTIVE' case is 'active_corr'
    /// I think 'active_corr' is to switch on/off correlations between random
    /// variables? Since we don't support correlated random variables,
    /// I'm going to refactor this into one branch
    case DESIGN:
    case ALEATORY_UNCERTAIN:
    case EPISTEMIC_UNCERTAIN:
    case UNCERTAIN:
    case STATE:
    case ALL:
    {
      /// Randomize sequence if 'unique' samples are required
      /// NOTE: I assume this then deals correctly with the 'get_samples' vs
      /// 'get_unique_samples' in 'LHSDriver'?
      if ( backfillDuplicates )
      {
        sequence->randomize();
      }

      /// Generate points from the low-discrepancy sequence
      sequence->get_points(colPtr, colPtr + num_samples, sample_matrix);

      /// Transform points from [0, 1) to the marginal distributions given in
      /// the model
      transform(model, sample_matrix);

      break;
    }

    case ACTIVE_UNIFORM:
    case ALL_UNIFORM:
    case UNCERTAIN_UNIFORM:
    case ALEATORY_UNCERTAIN_UNIFORM:
    case EPISTEMIC_UNCERTAIN_UNIFORM:
    {
      /// Create a model view
      short model_view = model.current_variables().view().first;
      RealSymMatrix corr; // assume uncorrelated samples

      /// Sample uniformly from ACTIVE lower/upper bounds (regardless of model
      /// view), from UNCERTAIN lower/upper bounds (with model in DISTINCT 
      /// view), or from ALL lower/upper bounds (with model in ALL view)
      if ( samplingVarsMode == ACTIVE_UNIFORM ||
        ( samplingVarsMode == ALL_UNIFORM && 
          ( model_view == RELAXED_ALL || model_view == MIXED_ALL ) ) ||
        ( samplingVarsMode == UNCERTAIN_UNIFORM &&
          ( model_view == RELAXED_UNCERTAIN ||
            model_view == MIXED_UNCERTAIN ) ) ||
        ( samplingVarsMode == ALEATORY_UNCERTAIN_UNIFORM &&
          ( model_view == RELAXED_ALEATORY_UNCERTAIN ||
            model_view == MIXED_ALEATORY_UNCERTAIN ) ) ||
        ( samplingVarsMode == EPISTEMIC_UNCERTAIN_UNIFORM &&
          ( model_view == RELAXED_EPISTEMIC_UNCERTAIN ||
            model_view == MIXED_EPISTEMIC_UNCERTAIN ) ) )
      {
        /// Generate points from the low-discrepancy sequence
        sequence->get_points(colPtr, colPtr + num_samples, sample_matrix);
        
        /// Scale points from [0, 1) to the given lower and upper bounds
        scale(
          ModelUtils::continuous_lower_bounds(model), 
          model.continuous_upper_bounds(), 
          allSamples
        );
      }
      /// Sample uniformly from ALL lower/upper bounds with model in distinct 
      /// view
      else if (samplingVarsMode == ALL_UNIFORM)
      {
        /// Generate points from the low-discrepancy sequence
        sequence->get_points(colPtr, colPtr + num_samples, sample_matrix);

        /// Scale points from [0, 1) to the given lower and upper bounds
        scale(
          model.all_continuous_lower_bounds(), 
          model.all_continuous_upper_bounds(), 
          allSamples
        );
      }
      // Sample uniformly from {A,E,A+E} UNCERTAIN lower/upper bounds
      // with model using a non-corresponding view (?)
      else
      {
        size_t start_acv, num_acv, dummy;
        mode_counts(model.current_variables(), start_acv, num_acv, dummy, 
          dummy, dummy, dummy, dummy, dummy);
        if (!num_acv)
        {
          Cerr << "Error: no active continuous variables for sampling in "
            << "uniform mode" << std::endl;
          abort_handler(METHOD_ERROR);
        }
        /// This is copied from 'NonDSampling'
        const RealVector& all_c_l_bnds = model.all_continuous_lower_bounds();
        const RealVector& all_c_u_bnds = model.all_continuous_upper_bounds();
        RealVector uncertain_c_l_bnds(Teuchos::View,
          const_cast<Real*>(&all_c_l_bnds[start_acv]), num_acv);
        RealVector uncertain_c_u_bnds(Teuchos::View,
          const_cast<Real*>(&all_c_u_bnds[start_acv]), num_acv);

        /// Generate points from the low-discrepancy sequence
        sequence->get_points(colPtr, colPtr + num_samples, sample_matrix);

        /// Scale points from [0, 1) to the given lower and upper bounds
        scale(uncertain_c_l_bnds, uncertain_c_u_bnds, allSamples);
      }

      break;
    }
    default:
    {
      Cerr << "\nError: this sampling mode has not been implemented yet." 
        << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }

  /// Update 'colPtr' when using refinement samples
  if ( allSamples.numCols() != sample_matrix.numCols() )
    colPtr += num_samples;
}

/// Generate a set of rank-1 lattice points using the given lower and upper
/// bounds and store the results in `allSamples`
void NonDLowDiscrepancySampling::get_parameter_sets(
  const RealVector& lower,
  const RealVector& upper
)
{
  /// Generate points from the low-discrepancy sequence
  sequence->get_points(allSamples.numCols(), allSamples);

  /// Scale points from [0, 1) to the model's lower and upper bounds
  scale(lower, upper, allSamples);
}

/// Generate a set of normally-distributed points by mapping the rank-1
/// lattice points using the inverse normal cdf
/// NOTE: can't handle correlated samples for now, so this method just
/// throws an error
void NonDLowDiscrepancySampling::get_parameter_sets(
  const RealVector& means,
  const RealVector& std_devs,
  const RealVector& lower,
  const RealVector& upper,
  RealSymMatrix& correl
)
{
  Cout << "\n\n\n ERROR: This function is not implemented yet."
    << std::endl;
  abort_handler(METHOD_ERROR);
}

/// Function to transform a given sample matrix from [0, 1) to the probability
/// density functions given in the model
/// Assumes that the sample matrix has shape `numParams` x `numSamples`
void NonDLowDiscrepancySampling::transform(
  Model& model,
  RealMatrix& sample_matrix
)
{
  /// Transform the samples to [-1, 1]
  auto numParams = sample_matrix.numRows();
  RealVector lower(numParams);
  RealVector upper(numParams);
  lower = -1.0;
  upper = 1.0;
  scale(lower, upper, sample_matrix); // transform from [0, 1) to [-1, 1)

  /// If correlated, tranform samples to standard normal first
  if ( model.multivariate_distribution().correlation() )
  {
    /// vSpaceModel has uncorrelated standard normal random variables
    Model vSpaceModel;
    vSpaceModel.assign_rep(
      std::make_shared<ProbabilityTransformModel>(model, STD_NORMAL_U)
    );

    /// uSpaceModel has uncorrelated standard uniform random variables
    Model uSpaceModel;
    uSpaceModel.assign_rep(
      std::make_shared<ProbabilityTransformModel>(vSpaceModel, STD_UNIFORM_U)
    );

    /// First transform from standard uniform to standard normal
    Pecos::ProbabilityTransformation& uNataf = 
      uSpaceModel.probability_transformation();
    transform_samples(uNataf, sample_matrix, model.current_variables().continuous_variable_ids(), 
      model.current_variables().continuous_variable_ids(), false);

    // Then transform from standard normal to actual model
    Pecos::ProbabilityTransformation& vNataf = 
      vSpaceModel.probability_transformation();
    transform_samples(vNataf, sample_matrix, model.current_variables().continuous_variable_ids(), 
      model.current_variables().continuous_variable_ids(), false);
  }
  else // If uncorrelated, directly apply the transform
  {
    /// uSpaceModel has uncorrelated standard uniform random variables
    Model uSpaceModel;
    uSpaceModel.assign_rep(
      std::make_shared<ProbabilityTransformModel>(model, STD_UNIFORM_U)
    );

    /// Transform samples using Nataf transformation (component-wise inverse CDF)
    Pecos::ProbabilityTransformation& uNataf = 
      uSpaceModel.probability_transformation();
    transform_samples(uNataf, sample_matrix, model.current_variables().continuous_variable_ids(), 
      model.current_variables().continuous_variable_ids(), false);
  } 
}

/// Function to scale a given sample matrix from [0, 1) to the given lower and
/// upper bounds
/// Assumes that the sample matrix has shape `numParams` x `numSamples`
void NonDLowDiscrepancySampling::scale(
  const RealVector& lower,
  const RealVector& upper,
  RealMatrix& sample_matrix
)
{
  auto numParams = sample_matrix.numRows(); // # params = # rows
  auto numSamples = sample_matrix.numCols(); // # samples = # columns
  /// Loop over each sample / column
  for (size_t col=0; col < numSamples; col++) 
  {
    /// Loop over each row / parameter
    for (size_t row=0; row < numParams; row++)
    {
      /// Scale from [0, 1) to [lower, upper)
      Real u = upper[row];
      Real l = lower[row];
      sample_matrix[col][row] = sample_matrix[col][row]*(u - l) + l;
    }
  }
}

/// Check if the given multivariate distribution is supported by 
/// low-discrepancy sampling methods
/// NOTE: A multivariate distribution is supported when it contains
/// uncorrelated continuous random variables
void NonDLowDiscrepancySampling::check_support(
  Pecos::MultivariateDistribution& mv_dist
)
{
  /// Loop over each variable and check if its type is allowed
  /// TODO: is there a better way to do this, e.g., variable.is_discrete()?
  std::vector<Pecos::RandomVariable>& variables = mv_dist.random_variables();
  for ( Pecos::RandomVariable variable : variables )
  {
    auto variable_type = variable.type();
    if ( 
      not (
        ( variable_type == Pecos::STD_NORMAL ) || 
        ( variable_type == Pecos::NORMAL ) || 
        ( variable_type == Pecos::BOUNDED_NORMAL ) || 
        ( variable_type == Pecos::LOGNORMAL ) || 
        ( variable_type == Pecos::BOUNDED_LOGNORMAL ) || 
        ( variable_type == Pecos::STD_UNIFORM ) || 
        ( variable_type == Pecos::UNIFORM ) || 
        ( variable_type == Pecos::LOGUNIFORM ) || 
        ( variable_type == Pecos::TRIANGULAR ) || 
        ( variable_type == Pecos::STD_EXPONENTIAL ) || 
        ( variable_type == Pecos::EXPONENTIAL ) || 
        ( variable_type == Pecos::STD_BETA ) || 
        ( variable_type == Pecos::BETA ) || 
        ( variable_type == Pecos::STD_GAMMA ) || 
        ( variable_type == Pecos::GAMMA ) || 
        ( variable_type == Pecos::GUMBEL ) || 
        ( variable_type == Pecos::FRECHET ) || 
        ( variable_type == Pecos::WEIBULL ) || 
        ( variable_type == Pecos::HISTOGRAM_BIN ) || 
        ( variable_type == Pecos::INV_GAMMA ) ||
        ( variable_type == Pecos::CONTINUOUS_RANGE )
      )
    )
    {
      Cerr << "\nError: low-discrepancy sampling does not support discrete "
        << "random variables." << std::endl;
        abort_handler(METHOD_ERROR);
    }
  }
}

} // namespace Dakota
