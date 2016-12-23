/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDQuadrature
//- Description: Implementation code for NonDQuadrature class
//- Owner:       Mike Eldred
//- Revised by:  
//- Version:

#include "dakota_data_types.hpp"
#include "dakota_system_defs.hpp"
#include "NonDQuadrature.hpp"
#include "DakotaModel.hpp"
#include "ProblemDescDB.hpp"
#include "PolynomialApproximation.hpp"
#include "LHSDriver.hpp"

static const char rcsId[]="@(#) $Id: NonDQuadrature.cpp,v 1.57 2004/06/21 19:57:32 mseldre Exp $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator
    instantiation.  In this case, set_db_list_nodes has been called
    and probDescDB can be queried for settings from the method
    specification.  It is not currently used, as there is not yet a
    separate nond_quadrature method specification. */
NonDQuadrature::NonDQuadrature(ProblemDescDB& problem_db, Model& model):
  NonDIntegration(problem_db, model),
  nestedRules(
    probDescDB.get_short("method.nond.nesting_override") == Pecos::NESTED),
  quadOrderSeqSpec(probDescDB.get_usa("method.nond.quadrature_order")),
  numSamples(0), quadMode(FULL_TENSOR)
{
  // initialize the numerical integration driver
  numIntDriver = Pecos::IntegrationDriver(Pecos::QUADRATURE);
  tpqDriver = (Pecos::TensorProductDriver*)numIntDriver.driver_rep();

  // natafTransform available: initialize_random_variables() called in
  // NonDIntegration ctor
  check_variables(natafTransform.x_random_variables());

  Pecos::ExpansionConfigOptions
    ec_options(Pecos::QUADRATURE,
	       probDescDB.get_short("method.nond.expansion_basis_type"),
	       outputLevel, probDescDB.get_bool("method.variance_based_decomp"),
	       probDescDB.get_ushort("method.nond.vbd_interaction_order"),
	       probDescDB.get_short("method.nond.expansion_refinement_control"),
	       //maxIterations,
	       probDescDB.get_int("method.nond.max_refinement_iterations"),
	       probDescDB.get_int("method.nond.max_solver_iterations"),
	       convergenceTol,
	       probDescDB.get_ushort("method.soft_convergence_limit"));

  bool piecewise_basis = (probDescDB.get_bool("method.nond.piecewise_basis") ||
    probDescDB.get_short("method.nond.expansion_refinement_type") ==
    Pecos::H_REFINEMENT);
  bool use_derivs = probDescDB.get_bool("method.derivative_usage");
  bool equidist_rules = true; // NEWTON_COTES pts for piecewise interpolants
  Pecos::BasisConfigOptions bc_options(nestedRules, piecewise_basis,
				       equidist_rules, use_derivs);

  tpqDriver->initialize_grid(natafTransform.u_types(), ec_options, bc_options);

  reset(); // init_dim_quad_order() uses integrationRules from initialize_grid()

  tpqDriver->precompute_rules(); // efficiency optimization

  maxEvalConcurrency *= tpqDriver->grid_size();
}


/** This alternate constructor is used for on-the-fly generation and
    evaluation of numerical quadrature points. */
NonDQuadrature::
NonDQuadrature(Model& model, const UShortArray& quad_order_seq,
	       const RealVector& dim_pref, short driver_mode):
  NonDIntegration(QUADRATURE_INTEGRATION, model, dim_pref), nestedRules(false),
  quadOrderSeqSpec(quad_order_seq), numSamples(0), quadMode(FULL_TENSOR)
{
  // initialize the numerical integration driver
  numIntDriver = Pecos::IntegrationDriver(Pecos::QUADRATURE);
  tpqDriver = (Pecos::TensorProductDriver*)numIntDriver.driver_rep();

  tpqDriver->mode(driver_mode);

  // local natafTransform not yet updated: x_ran_vars would have to be passed in
  // from NonDExpansion if check_variables() needed to be called here.  Instead,
  // it is deferred until run time in NonDIntegration::core_run().
  //check_variables(x_ran_vars);
}


/** This alternate constructor is used for on-the-fly generation and
    evaluation of filtered tensor quadrature points. */
NonDQuadrature::
NonDQuadrature(Model& model, int num_filt_samples, const RealVector& dim_pref,
	       short driver_mode): 
  NonDIntegration(QUADRATURE_INTEGRATION, model, dim_pref),
  nestedRules(false), // minimize zeros introduced into Vandermonde matrix
  quadMode(FILTERED_TENSOR), numSamples(num_filt_samples)
{
  // initialize the numerical integration driver
  numIntDriver = Pecos::IntegrationDriver(Pecos::QUADRATURE);
  tpqDriver = (Pecos::TensorProductDriver*)numIntDriver.driver_rep();

  tpqDriver->mode(driver_mode);

  // local natafTransform not yet updated: x_ran_vars would have to be passed in
  // from NonDExpansion if check_variables() needed to be called here.  Instead,
  // it is deferred until run time in NonDIntegration::core_run().
  //check_variables(x_ran_vars);
}


/** This alternate constructor is used for on-the-fly generation and
    evaluation of random sampling from a tensor quadrature multi-index. */
NonDQuadrature::
NonDQuadrature(Model& model, int num_rand_samples, int seed,
	       const UShortArray& quad_order_seq, const RealVector& dim_pref,
	       short driver_mode): 
  NonDIntegration(QUADRATURE_INTEGRATION, model, dim_pref),
  nestedRules(false), // minimize zeros introduced into Vandermonde matrix
  quadOrderSeqSpec(quad_order_seq), quadMode(RANDOM_TENSOR),
  numSamples(num_rand_samples), randomSeed(seed)
{
  // initialize the numerical integration driver
  numIntDriver = Pecos::IntegrationDriver(Pecos::QUADRATURE);
  tpqDriver = (Pecos::TensorProductDriver*)numIntDriver.driver_rep();

  tpqDriver->mode(driver_mode);

  // local natafTransform not yet updated: x_ran_vars would have to be passed in
  // from NonDExpansion if check_variables() needed to be called here.  Instead,
  // it is deferred until run time in NonDIntegration::core_run().
  //check_variables(x_ran_vars);
}


NonDQuadrature::~NonDQuadrature()
{ }


/** Used in combination with alternate NonDQuadrature constructor. */
void NonDQuadrature::
initialize_grid(const std::vector<Pecos::BasisPolynomial>& poly_basis)
{
  tpqDriver->initialize_grid(poly_basis);

  switch (quadMode) {
  case FULL_TENSOR:
    // infer nestedRules
    for (size_t i=0; i<numContinuousVars; ++i) {
      short rule = poly_basis[i].collocation_rule();
      // Distinguish between rules that *support* vs. *require* nesting, since
      // TPQ should allow unrestricted order specifications where supported.
      // > GENZ_KEISTER and GAUSS_PATTERSON are only defined as nested rules
      // > NEWTON_COTES, CLENSHAW_CURTIS, FEJER2 support nesting but also allow
      //   arbitrary order specification
      if (rule == Pecos::GENZ_KEISTER || rule == Pecos::GAUSS_PATTERSON)// ||
	//rule == Pecos::NEWTON_COTES || rule == Pecos::CLENSHAW_CURTIS ||
	//rule == Pecos::FEJER2)
	{ nestedRules = true; break; }
    }
    reset();
    maxEvalConcurrency *= tpqDriver->grid_size();
    break;
  case FILTERED_TENSOR:
    // nested overrides not currently part of tensor regression spec
    //for () if () { nestedRules = true; break; }
    update(); // compute min quad order reqd for numSamples
    maxEvalConcurrency *= numSamples;
    break;
   case RANDOM_TENSOR:
    // nested overrides not currently part of tensor regression spec
    //for () if () { nestedRules = true; break; }
    reset();  // propagate updated settings to tpqDriver
    update(); // enforce min quad order constraints
    maxEvalConcurrency *= numSamples;
    break;
  }

  // Precompute quadrature rules (e.g., by defining maximal order for
  // NumGenOrthogPolynomial::solve_eigenproblem()):
  tpqDriver->precompute_rules(); // efficiency optimization
}


void NonDQuadrature::
initialize_dimension_quadrature_order(unsigned short quad_order_spec,
				      const RealVector& dim_pref_spec,
				      UShortArray& dim_quad_order)
{
  // Update dimQuadOrderRef from quad_order_spec and dimPrefSpec
  dimension_preference_to_anisotropic_order(quad_order_spec,   dim_pref_spec,
					    numContinuousVars, dim_quad_order);
  //dimPrefRef = dimPrefSpec; // not currently necessary

  // Update Pecos::TensorProductDriver::quadOrder from dim_quad_order
  if (nestedRules) tpqDriver->nested_quadrature_order(dim_quad_order);
  else             tpqDriver->quadrature_order(dim_quad_order);
}


void NonDQuadrature::
compute_minimum_quadrature_order(size_t min_samples, const RealVector& dim_pref,
				 UShortArray& dim_quad_order)
{
  dim_quad_order.assign(numContinuousVars, 1);
  // compute minimal order tensor grid with at least min_samples points
  if (dim_pref.empty()) // isotropic tensor grid
    while (tpqDriver->grid_size() < min_samples)
      increment_grid(dim_quad_order);
  else                   // anisotropic tensor grid
    while (tpqDriver->grid_size() < min_samples)
      increment_grid_preference(dim_pref, dim_quad_order);
}


void NonDQuadrature::get_parameter_sets(Model& model)
{
  // capture any distribution parameter insertions
  if (!numIntegrations || subIteratorFlag)
    tpqDriver->initialize_grid_parameters(natafTransform.u_types(),
      iteratedModel.aleatory_distribution_parameters());

  size_t i, j, num_quad_points = tpqDriver->grid_size();
  const Pecos::UShortArray& quad_order = tpqDriver->quadrature_order();
  Cout << "\nNumber of Gauss points per variable: { ";
  for (i=0; i<numContinuousVars; ++i)
    Cout << quad_order[i] << ' ';
  Cout << "}\n";

  switch (quadMode) {
  // --------------------------------
  // Tensor quadrature (default mode)
  // --------------------------------
  case FULL_TENSOR:
    Cout << "Total number of integration points: " << num_quad_points << '\n';
    // Compute the tensor-product grid and store in allSamples
    tpqDriver->compute_grid(allSamples);
    if (outputLevel > NORMAL_OUTPUT)
      print_points_weights("dakota_quadrature_tabular.dat");
    break;
  // ------------------------------------------------------------------------
  // Probabilistic collocation from a tensor grid filtered by product weights
  // ------------------------------------------------------------------------
  case FILTERED_TENSOR:
    Cout << "Filtered to " << numSamples
	 << " samples with max product weight.\n";
    // Compute the tensor-product grid and store in allSamples
    tpqDriver->compute_grid(allSamples);
    // retain a subset of the minimal order tensor grid
    filter_parameter_sets();
    break;
  // ----------------------------------------------------------------------
  // Probabilistic collocation from random sampling of a tensor multi-index
  // ----------------------------------------------------------------------
  case RANDOM_TENSOR:
    Cout << numSamples << " samples drawn randomly from tensor grid.\n";
    const Pecos::UShortArray& lev_index = tpqDriver->level_index();
    tpqDriver->update_1d_collocation_points_weights(quad_order, lev_index);
    const Pecos::Real3DArray& colloc_pts_1d
      = tpqDriver->collocation_points_1d();
    allSamples.shapeUninitialized(numContinuousVars, numSamples);

    // prevent case of all lhs_const variables, which is an lhs_prep error
    bool lhs_error_trap = true;
    for (i=0; i<numContinuousVars; ++i)
      if (quad_order[i] > 1)
	{ lhs_error_trap = false; break; }
    if (lhs_error_trap) { // only 1 point from which to draw samples
      for (i=0; i<numContinuousVars; ++i) {
	Real samp_i = colloc_pts_1d[0][i][0]; // all levels,indices = 0
	for (j=0; j<numSamples; ++j)
	  allSamples(i,j) = samp_i;
      }
    }
    else { // sample randomly from the tensor multi-index
      IntVector index_l_bnds(numContinuousVars), // init to 0
                index_u_bnds(numContinuousVars, false);
      for (j=0; j<numContinuousVars; ++j)
	index_u_bnds[j] = quad_order[j] - 1;
      IntMatrix sorted_samples;
      // generate unique samples since redundancy degrades the conditioning
      Pecos::LHSDriver lhs("lhs", IGNORE_RANKS, false);
      if (!randomSeed) randomSeed = generate_system_seed();
      lhs.seed(randomSeed);
      lhs.generate_unique_index_samples(index_l_bnds, index_u_bnds, numSamples,
					sorted_samples);

      // convert multi-index samples into allSamples
      for (i=0; i<numSamples; ++i){
	Real* all_samp_i = allSamples[i];
	int*  sorted_samples_i = sorted_samples[i];
	for (j=0; j<numContinuousVars; ++j)
	  all_samp_i[j] = colloc_pts_1d[lev_index[j]][j][sorted_samples_i[j]];
      }
    }
    break;
  }
}


void NonDQuadrature::filter_parameter_sets()
{
  size_t i, num_tensor_pts = allSamples.numCols();
  const Pecos::RealVector& wts = tpqDriver->type1_weight_sets();
#ifdef DEBUG
  Cout << "allSamples pre-filter:" << allSamples
       << "weights pre-filter:\n"  << wts;
#endif // DEBUG
  // sort TPQ points in order of descending weight
  std::multimap<Real, RealVector> ordered_pts;
  for (i=0; i<num_tensor_pts; ++i) {
    RealVector col_i(Teuchos::Copy, allSamples[i], numContinuousVars);
    ordered_pts.insert(std::pair<Real, RealVector>(-std::abs(wts[i]), col_i));
  }
  // truncate allSamples to the first numSamples with largest weight
  allSamples.reshape(numContinuousVars, numSamples);
  std::multimap<Real, RealVector>::iterator it;
  for (i=0, it=ordered_pts.begin(); i<numSamples; ++i, ++it)
    Teuchos::setCol(it->second, (int)i, allSamples);
#ifdef DEBUG
  Cout << "allSamples post-filter:" << allSamples;
#endif // DEBUG
}


/** used by DataFitSurrModel::build_global() to publish the minimum
    number of points needed from the quadrature routine in order to
    build a particular global approximation. */
void NonDQuadrature::
sampling_reset(int min_samples, bool all_data_flag, bool stats_flag)
{
  // dimQuadOrderRef (possibly incremented from quadOrderSeqSpec[numIntSeqIndex]
  // due to uniform/adaptive refinements) provides the current lower bound
  // reference point.  Pecos::TensorProductDriver::quadOrder may be increased
  // ***or decreased*** to provide at least min_samples subject to this lower
  // bound.  dimQuadOrderRef is ***not*** updated by min_samples.
  if (min_samples > tpqDriver->grid_size()) {
    UShortArray dqo_l_bnd; // isotropic or anisotropic based on dimPrefSpec
    compute_minimum_quadrature_order(min_samples, dimPrefSpec, dqo_l_bnd);
    // enforce lower bound
    UShortArray new_dqo(numContinuousVars);
    for (size_t i=0; i<numContinuousVars; ++i)
      new_dqo[i] = std::max(dqo_l_bnd[i], dimQuadOrderRef[i]);
    // update tpqDriver
    if (nestedRules) tpqDriver->nested_quadrature_order(new_dqo);
    else             tpqDriver->quadrature_order(new_dqo);
  }

  // not currently used by this class:
  //allDataFlag = all_data_flag;
  //statsFlag   = stats_flag;
}


void NonDQuadrature::increment_grid(UShortArray& dim_quad_order)
{
  // Used for uniform refinement: all quad orders are incremented by 1.
  if (nestedRules) {
    // define reference point
    size_t orig_size = tpqDriver->grid_size();
    // initial increment and nestedness enforcement
    increment_dimension_quadrature_order(dim_quad_order);
    // ensure change in presence of restricted growth
    while (tpqDriver->grid_size() == orig_size)
      increment_dimension_quadrature_order(dim_quad_order);
  }
  else
    increment_dimension_quadrature_order(dim_quad_order);
}


void NonDQuadrature::
increment_dimension_quadrature_order(UShortArray& dim_quad_order)
{
  // increment uniformly by 1
  for (size_t i=0; i<numContinuousVars; ++i)
    dim_quad_order[i] += 1;

  if (nestedRules) tpqDriver->nested_quadrature_order(dim_quad_order);
  else             tpqDriver->quadrature_order(dim_quad_order);
}


void NonDQuadrature::
increment_grid_preference(const RealVector& dim_pref,
			  UShortArray& dim_quad_order)
{
  // Used for dimension-adaptive refinement: order lower bounds are enforced
  // using dimQuadOrderRef such that anisotropy may not reduce dimension
  // resolution once grids have been resolved (as with
  // SparseGridDriver::axisLowerBounds in NonDSparseGrid).
  if (nestedRules) {
    // define reference point
    size_t orig_size = tpqDriver->grid_size();
    // initial increment, anisotropy update, and nestedness enforcement
    increment_dimension_quadrature_order(dim_pref, dim_quad_order);
    // ensure change in presence of restricted growth
    while (tpqDriver->grid_size() == orig_size)
      increment_dimension_quadrature_order(dim_pref, dim_quad_order);
  }
  else
    increment_dimension_quadrature_order(dim_pref, dim_quad_order);
}


void NonDQuadrature::
increment_dimension_quadrature_order(const RealVector& dim_pref,
				     UShortArray& dim_quad_order)
{
  // determine the dimension with max preference
  Real max_dim_pref = dim_pref[0]; size_t max_dim_pref_index = 0;
  for (size_t i=1; i<numContinuousVars; ++i)
    if (dim_pref[i] > max_dim_pref)
      { max_dim_pref = dim_pref[i]; max_dim_pref_index = i; }
  // increment only the dimension with max preference by 1
  dim_quad_order[max_dim_pref_index] += 1;
  // now balance the other dims relative to this new increment, preserving
  // previous resolution
  update_anisotropic_order(dim_pref, dim_quad_order);

  if (nestedRules) tpqDriver->nested_quadrature_order(dim_quad_order);
  else             tpqDriver->quadrature_order(dim_quad_order);
}


void NonDQuadrature::
update_anisotropic_order(const RealVector& dim_pref,
			 UShortArray& quad_order_ref)
{
  // Logic is loosely patterned after anisotropic SSG in which the dominant
  // dimension is constrained by the current ssgLevel and all nondominant
  // dimensions are scaled back.
  unsigned short max_quad_ref = quad_order_ref[0];
  Real max_dim_pref = dim_pref[0]; size_t max_dim_pref_index = 0;
  for (size_t i=1; i<numContinuousVars; ++i) {
    if (quad_order_ref[i] > max_quad_ref)
      max_quad_ref = quad_order_ref[i];
    if (dim_pref[i] > max_dim_pref)
      { max_dim_pref = dim_pref[i]; max_dim_pref_index = i; }
  }
  // There are several different options for the following operation.  Initial
  // choice is to preserve the current (recently incremented) quad_order_ref
  // entry in the dimension with max_dim_pref and to compute the others relative
  // to max_quad_ref (whose dimension may differ from max_dim_pref).  Could also
  // decide to set max_quad_ref in the max_dim_pref dimension, but this could be
  // a much larger increment.  In all cases, the current quad_order_ref is used
  // as a lower bound to prevent reducing previous resolution.
  for (size_t i=0; i<numContinuousVars; ++i)
    if (i != max_dim_pref_index)
      quad_order_ref[i] = std::max(quad_order_ref[i],
	(unsigned short)(max_quad_ref*dim_pref[i]/max_dim_pref)); // truncate

  /* When used as a stand-alone update:
  if (nestedRules) tpqDriver->nested_quadrature_order(quad_order_ref);
  else             tpqDriver->quadrature_order(quad_order_ref);
  */
}

} // namespace Dakota
