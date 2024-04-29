/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_data_types.hpp"
#include "dakota_system_defs.hpp"
#include "NonDCubature.hpp"
#include "DakotaModel.hpp"
#include "ProblemDescDB.hpp"
#include "MarginalsCorrDistribution.hpp"

static const char rcsId[]="@(#) $Id: NonDCubature.cpp,v 1.57 2004/06/21 19:57:32 mseldre Exp $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator
    instantiation.  In this case, set_db_list_nodes has been called
    and probDescDB can be queried for settings from the method
    specification.  It is not currently used, as there is not yet a
    separate nond_cubature method specification. */
NonDCubature::NonDCubature(ProblemDescDB& problem_db, Model& model):
  NonDIntegration(problem_db, model),
  cubIntOrderRef(probDescDB.get_ushort("method.nond.cubature_integrand"))
{
  // initialize the numerical integration driver
  numIntDriver =  Pecos::IntegrationDriver(Pecos::CUBATURE);
  cubDriver    = std::static_pointer_cast<Pecos::CubatureDriver>
    (numIntDriver.driver_rep());

  // additional initializations in NonDIntegration ctor
  Pecos::MultivariateDistribution& mv_dist = model.multivariate_distribution();
  assign_rule(mv_dist); // assign cubIntRule

  // update CubatureDriver::{numVars,cubIntOrder,integrationRule}
  cubDriver->initialize_grid(mv_dist, cubIntOrderRef, cubIntRule);
  maxEvalConcurrency *= cubDriver->grid_size();
}


/** This alternate constructor is used for on-the-fly generation and
    evaluation of numerical cubature points. */
NonDCubature::
NonDCubature(Model& model, unsigned short cub_int_order): 
  NonDIntegration(CUBATURE_INTEGRATION, model), cubIntOrderRef(cub_int_order)
{
  // initialize the numerical integration driver
  numIntDriver =  Pecos::IntegrationDriver(Pecos::CUBATURE);
  cubDriver    = std::static_pointer_cast<Pecos::CubatureDriver>
    (numIntDriver.driver_rep());
  cubDriver->integrand_order(cubIntOrderRef);

  assign_rule(model.multivariate_distribution());
}


void NonDCubature::
initialize_grid(const std::vector<Pecos::BasisPolynomial>& poly_basis)
{
  cubDriver->initialize_grid(poly_basis);
  maxEvalConcurrency *= cubDriver->grid_size();
}


NonDCubature::~NonDCubature()
{ }

  
void NonDCubature::
assign_rule(const Pecos::MultivariateDistribution& mvd)
{
  const ShortArray& rv_types = mvd.random_variable_types();
  short rv_type0 = rv_types[0];  size_t i, num_rv = rv_types.size();
  for (size_t i=1; i<num_rv; ++i)
    if (rv_types[i] != rv_type0) {
      Cerr << "Error: homogeneity required in random variable types for "
	   << "NonDCubature integration." << std::endl;
      abort_handler(METHOD_ERROR);
    }

  // Note: homogeneity of distribution parameters is verified at run time
  // in Pecos::CubatureDriver::initialize_grid_parameters()

  switch (rv_type0) {
  case Pecos::STD_NORMAL:
    cubIntRule = Pecos::GAUSS_HERMITE;      break;
  case Pecos::STD_UNIFORM:
    cubIntRule = Pecos::GAUSS_LEGENDRE;     break;
  case Pecos::STD_EXPONENTIAL:
    cubIntRule = Pecos::GAUSS_LAGUERRE;     break;
  case Pecos::STD_BETA:
    cubIntRule = Pecos::GAUSS_JACOBI;       break;
  case Pecos::STD_GAMMA:
    cubIntRule = Pecos::GEN_GAUSS_LAGUERRE; break;
  default:
    cubIntRule = Pecos::GOLUB_WELSCH;       break;
  }
}


void NonDCubature::get_parameter_sets(Model& model)
{
  // capture any distribution parameter insertions
  Pecos::MultivariateDistribution& mv_dist = model.multivariate_distribution();
  if (!numIntegrations || subIteratorFlag)
    cubDriver->initialize_grid_parameters(mv_dist);

  //cubDriver->precompute_rules(); // not implemented

  size_t i, j, num_cub_points = cubDriver->grid_size();
  Cout << "\nCubature integrand order = " << cubDriver->integrand_order()
       << "\nTotal number of integration points: " << num_cub_points << '\n';

  // Compute the cubature grid and store in allSamples
  cubDriver->compute_grid(allSamples);
  if (outputLevel > NORMAL_OUTPUT)
    print_points_weights("dakota_cubature_tabular.dat");
}


/** used by DataFitSurrModel::build_global() to publish the minimum
    number of points needed from the cubature routine in order to
    build a particular global approximation. */
void NonDCubature::
sampling_reset(size_t min_samples, bool all_data_flag, bool stats_flag)
{
  // cubature order may be increased ***or decreased*** to provide at least
  // min_samples, but the original user specification (cubIntOrderSpec) is a
  // hard lower bound.  With the introduction of uniform/adaptive refinements,
  // cubIntOrderRef (which is incremented from cubIntOrderSpec) replaces
  // cubIntOrderSpec as the lower bound.

  // Note: Pecos::CubatureDriver is not currently managed by active keys, so
  // do not need to worry about tracking multiple reference points.

  // should be cubIntOrderRef already, unless min_samples previously enforced
  //cubDriver->integrand_order(cubIntOrderRef);

  // determine minimum integrand order that provides at least min_samples
  unsigned short min_order = cubDriver->integrand_order();//cubIntOrderRef;
  while (cubDriver->grid_size() < min_samples)
    cubDriver->integrand_order(++min_order);
  // leave cubDriver at min_order; do not update cubIntOrderRef

  // maxEvalConcurrency must not be updated since parallel config management
  // depends on having the same value at ctor/run/dtor times.

  // not currently used by this class:
  //allDataFlag = all_data_flag;
  //statsFlag   = stats_flag;
}


void NonDCubature::increment_grid()
{ ++cubIntOrderRef; cubDriver->integrand_order(cubIntOrderRef); }


void NonDCubature::decrement_grid()
{ --cubIntOrderRef; cubDriver->integrand_order(cubIntOrderRef); }

} // namespace Dakota
