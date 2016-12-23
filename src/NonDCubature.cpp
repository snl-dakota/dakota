/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDCubature
//- Description: Implementation code for NonDCubature class
//- Owner:       Mike Eldred
//- Revised by:  
//- Version:

#include "dakota_data_types.hpp"
#include "dakota_system_defs.hpp"
#include "NonDCubature.hpp"
#include "DakotaModel.hpp"
#include "ProblemDescDB.hpp"

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
  numIntDriver = Pecos::IntegrationDriver(Pecos::CUBATURE);
  cubDriver = (Pecos::CubatureDriver*)numIntDriver.driver_rep();

  // natafTransform available: initialize_random_variables() called in
  // NonDIntegration ctor
  const Pecos::ShortArray& u_types = natafTransform.u_types();
  check_variables(natafTransform.x_random_variables());
  check_integration(u_types, iteratedModel.aleatory_distribution_parameters());

  // update CubatureDriver::{numVars,cubIntOrder,integrationRule}
  cubDriver->initialize_grid(u_types, cubIntOrderRef, cubIntRule);
  //cubDriver->precompute_rules(); // not implemented
  maxEvalConcurrency *= cubDriver->grid_size();
}


/** This alternate constructor is used for on-the-fly generation and
    evaluation of numerical cubature points. */
NonDCubature::
NonDCubature(Model& model, const Pecos::ShortArray& u_types,
	     unsigned short cub_int_order): 
  NonDIntegration(CUBATURE_INTEGRATION, model), cubIntOrderRef(cub_int_order)
{
  // initialize the numerical integration driver
  numIntDriver = Pecos::IntegrationDriver(Pecos::CUBATURE);
  cubDriver = (Pecos::CubatureDriver*)numIntDriver.driver_rep();
  cubDriver->integrand_order(cubIntOrderRef);

  // local natafTransform not yet updated: x_types would have to be passed in
  // from NonDExpansion if check_variables() needed to be called here.  Instead,
  // it is deferred until run time in NonDIntegration::core_run().
  //check_variables(x_ran_vars);
  check_integration(u_types, iteratedModel.aleatory_distribution_parameters());
}


void NonDCubature::
initialize_grid(const std::vector<Pecos::BasisPolynomial>& poly_basis)
{
  cubDriver->initialize_grid(poly_basis);
  //cubDriver->precompute_rules(); // not implemented
  maxEvalConcurrency *= cubDriver->grid_size();
}


NonDCubature::~NonDCubature()
{ }


void NonDCubature::
check_integration(const Pecos::ShortArray& u_types,
		  const Pecos::AleatoryDistParams& adp)
{
  bool err_flag = false;

  // For parameterized polynomials (including numerically-generated), check
  // u_types and dp for isotropy; for other polynomials, check u_types only.
  short type0 = u_types[0];
  switch (type0) {
  case Pecos::STD_BETA: { // verify isotropy in u_type and dp
    const RealVector& beuv_alphas = adp.beta_alphas();
    const RealVector& beuv_betas  = adp.beta_betas();
    const Real& alpha0 = beuv_alphas[0]; const Real& beta0 = beuv_betas[0];
    for (size_t i=1; i<numContinuousVars; ++i)
      if (u_types[i]    != type0 || beuv_alphas[i] != alpha0 ||
	  beuv_betas[i] != beta0)
	err_flag = true;
    break;
  }
  case Pecos::STD_GAMMA: { // verify isotropy in u_type and dp
    const RealVector& gauv_alphas = adp.gamma_alphas();
    const Real& alpha0 = gauv_alphas[0];
    for (size_t i=1; i<numContinuousVars; ++i)
      if (u_types[i] != type0 || gauv_alphas[i] != alpha0)
	err_flag = true;
    break;
  }

  // TO DO: Golub-Welsch parameter checks!

  default: // no dp; verify isotropy in u_type only
    for (size_t i=1; i<numContinuousVars; ++i)
      if (u_types[i] != type0)
	err_flag = true;
    break;
  }

  if (err_flag) {
    Cerr << "Error: homogeneous u-space types required for NonDCubature "
	 << "integration." << std::endl;
    abort_handler(-1);
  }

  switch (type0) {
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
  if (!numIntegrations || subIteratorFlag)
    cubDriver->initialize_grid_parameters(natafTransform.u_types(),
      iteratedModel.aleatory_distribution_parameters());

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
sampling_reset(int min_samples, bool all_data_flag, bool stats_flag)
{
  // cubature order may be increased ***or decreased*** to provide at least
  // min_samples, but the original user specification (cubIntOrderSpec) is a
  // hard lower bound.  With the introduction of uniform/adaptive refinements,
  // cubIntOrderRef (which is incremented from cubIntOrderSpec) replaces
  // cubIntOrderSpec as the lower bound.

  // should be cubIntOrderRef already, unless min_order previous enforced
  cubDriver->integrand_order(cubIntOrderRef);
  if (min_samples > cubDriver->grid_size()) { // reference grid size
    // determine minimum integrand order that provides at least min_samples
    unsigned short min_order = cubIntOrderRef + 1;
    cubDriver->integrand_order(min_order);
    while (cubDriver->grid_size() < min_samples)
      cubDriver->integrand_order(++min_order);
    // leave cubDriver at min_order; do not update cubIntOrderRef

    // maxEvalConcurrency must not be updated since parallel config management
    // depends on having the same value at ctor/run/dtor times.
  }

  // not currently used by this class:
  //allDataFlag = all_data_flag;
  //statsFlag   = stats_flag;
}


void NonDCubature::increment_grid()
{ increment_reference(); cubDriver->integrand_order(cubIntOrderRef); }

} // namespace Dakota
