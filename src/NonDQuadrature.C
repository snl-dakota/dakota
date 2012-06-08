/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDQuadrature
//- Description: Implementation code for NonDQuadrature class
//- Owner:       Mike Eldred
//- Revised by:  
//- Version:

#include "data_types.h"
#include "system_defs.h"
#include "NonDQuadrature.H"
#include "DakotaModel.H"
#include "ProblemDescDB.H"
#include "PolynomialApproximation.hpp"

static const char rcsId[]="@(#) $Id: NonDQuadrature.C,v 1.57 2004/06/21 19:57:32 mseldre Exp $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator
    instantiation.  In this case, set_db_list_nodes has been called
    and probDescDB can be queried for settings from the method
    specification.  It is not currently used, as there is not yet a
    separate nond_quadrature method specification. */
NonDQuadrature::NonDQuadrature(Model& model): NonDIntegration(model),
  nestedRules(
    probDescDB.get_short("method.nond.nesting_override") == Pecos::NESTED),
  quadOrderSpec(probDescDB.get_usa("method.nond.quadrature_order"))
{
  // initialize the numerical integration driver
  numIntDriver = Pecos::IntegrationDriver(Pecos::QUADRATURE);
  tpqDriver = (Pecos::TensorProductDriver*)numIntDriver.driver_rep();

  // natafTransform available: initialize_random_variables() called in
  // NonDIntegration ctor
  check_variables(natafTransform.x_types());

  bool piecewise_basis = (probDescDB.get_bool("method.nond.piecewise_basis") ||
    probDescDB.get_short("method.nond.expansion_refinement_type") ==
    Pecos::H_REFINEMENT);
  bool use_derivs = probDescDB.get_bool("method.derivative_usage");
  bool equidist_rules = true; // NEWTON_COTES pts for piecewise interpolants

  Pecos::BasisConfigOptions bc_options(nestedRules, piecewise_basis,
				       equidist_rules, use_derivs);
  tpqDriver->initialize_grid(natafTransform.u_types(), bc_options);

  reset(); // init_dim_quad_order() uses integrationRules from initialize_grid()
  maxConcurrency *= tpqDriver->grid_size();
}


/** This alternate constructor is used for on-the-fly generation and
    evaluation of numerical quadrature points. */
NonDQuadrature::
NonDQuadrature(Model& model, const UShortArray& quad_order,
	       const RealVector& dim_pref):
  NonDIntegration(NoDBBaseConstructor(), model, dim_pref), nestedRules(false),
  quadOrderSpec(quad_order), numFilteredSamples(0)
{
  // initialize the numerical integration driver
  numIntDriver = Pecos::IntegrationDriver(Pecos::QUADRATURE);
  tpqDriver = (Pecos::TensorProductDriver*)numIntDriver.driver_rep();

  // local natafTransform not yet updated: x_types would have to be passed in
  // from NonDExpansion if check_variables() needed to be called here.  Instead,
  // it is deferred until run time in NonDIntegration::quantify_uncertainty().
  //check_variables(x_types);
}


/** This alternate constructor is used for on-the-fly generation and
    evaluation of numerical quadrature points. */
NonDQuadrature::
NonDQuadrature(Model& model, int num_filt_samples, const RealVector& dim_pref): 
  NonDIntegration(NoDBBaseConstructor(), model, dim_pref), nestedRules(false),
  numFilteredSamples(num_filt_samples)
{
  // initialize the numerical integration driver
  numIntDriver = Pecos::IntegrationDriver(Pecos::QUADRATURE);
  tpqDriver = (Pecos::TensorProductDriver*)numIntDriver.driver_rep();

  // local natafTransform not yet updated: x_types would have to be passed in
  // from NonDExpansion if check_variables() needed to be called here.  Instead,
  // it is deferred until run time in NonDIntegration::quantify_uncertainty().
  //check_variables(x_types);
}


NonDQuadrature::~NonDQuadrature()
{ }


/** Used in combination with alternate NonDQuadrature constructor. */
void NonDQuadrature::
initialize_grid(const std::vector<Pecos::BasisPolynomial>& poly_basis)
{
  tpqDriver->initialize_grid(poly_basis);
  if (numFilteredSamples) {
    // nested overrides not currently part of tensor regression spec
    //for () if () { nestedRules = true; break; }
    compute_minimum_quadrature_order();
    maxConcurrency *= numFilteredSamples;
  }
  else {
    // infer nestedRules
    for (size_t i=0; i<numContinuousVars; ++i) {
      short colloc_rule = poly_basis[i].collocation_rule();
      if (colloc_rule == Pecos::GENZ_KEISTER    ||
	  colloc_rule == Pecos::GAUSS_PATTERSON) //||
	// don't need to enforce a growth pattern for these;
	// also SLOW_RESTRICTED_GROWTH is not currently supported
	// in nested_quadrature_order()
	//colloc_rule == Pecos::NEWTON_COTES    ||
	//colloc_rule == Pecos::CLENSHAW_CURTIS ||
	//colloc_rule == Pecos::FEJER2)
	{ nestedRules = true; break; }
    }
    reset();
    maxConcurrency *= tpqDriver->grid_size();
  }
}


void NonDQuadrature::
initialize_dimension_quadrature_order(unsigned short quad_order_spec,
				      const RealVector& dim_pref_spec,
				      UShortArray& dim_quad_order)
{
  // Update dimQuadOrderRef from quad_order_spec and dimPrefSpec
  if (dim_pref_spec.empty()) { // iso tensor grid
    dim_quad_order.reserve(numContinuousVars);
    dim_quad_order.assign(numContinuousVars, quad_order_spec);
  }
  else                       // aniso tensor grid
    anisotropic_preference(quad_order_spec, dim_pref_spec, dim_quad_order);
  //dimPrefRef = dimPrefSpec; // not currently necessary

  // Update Pecos::TensorProductDriver::quadOrder from dim_quad_order
  if (nestedRules) nested_quadrature_order(dim_quad_order);
  else         tpqDriver->quadrature_order(dim_quad_order);
}


void NonDQuadrature::
compute_minimum_quadrature_order(size_t min_samples, const RealVector& dim_pref,
				 UShortArray& dim_quad_order)
{
  dim_quad_order.reserve(numContinuousVars);
  dim_quad_order.assign(numContinuousVars, 1);
  // compute minimal order tensor grid with at least numFilteredSamples points
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
      iteratedModel.distribution_parameters());

  size_t i, j, num_quad_points = tpqDriver->grid_size();
  const Pecos::UShortArray& quad_order = tpqDriver->quadrature_order();
  Cout << "\nNumber of Gauss points per variable: { ";
  for (i=0; i<numContinuousVars; ++i)
    Cout << quad_order[i] << ' ';
  Cout << "}\nTotal number of integration points: " << num_quad_points << '\n';

  // Compute the tensor-product grid and store in allSamples
  tpqDriver->compute_grid(allSamples);

  // retain a subset of the minimal order tensor grid
  if (numFilteredSamples)
    filter_parameter_sets();
  else if (outputLevel > NORMAL_OUTPUT)
    print_points_weights("dakota_quadrature_tabular.dat");
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
  // truncate allSamples to the first numFilteredSamples with largest weight
  allSamples.reshape(numContinuousVars, numFilteredSamples);
  std::multimap<Real, RealVector>::iterator it;
  for (i=0, it=ordered_pts.begin(); i<numFilteredSamples; ++i, ++it)
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
  // dimQuadOrderRef (potentially incremented from quadOrderSpec[numIntSeqIndex]
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
    if (nestedRules) nested_quadrature_order(new_dqo);
    else         tpqDriver->quadrature_order(new_dqo);
  }

  // not currently used by this class:
  //allDataFlag = all_data_flag;
  //statsFlag   = stats_flag;
}


/* This function selects the smallest nested rule order that meets the
   integrand precision of a corresponding Gauss rule.  It is similar to
   the moderate exponential growth option in sparse grids. */
void NonDQuadrature::nested_quadrature_order(const UShortArray& quad_order_ref)
{
  const Pecos::ShortArray&  rules    = tpqDriver->collocation_rules();
  const Pecos::UShortArray& gk_order = tpqDriver->genz_keister_order();
  const Pecos::UShortArray& gk_prec  = tpqDriver->genz_keister_precision();
  for (size_t i=0; i<numContinuousVars; ++i)
    switch (rules[i]) {
    case Pecos::CLENSHAW_CURTIS: case Pecos::NEWTON_COTES: { // closed rules
      unsigned short int_goal = 2*quad_order_ref[i] - 1, level = 0, order,
	int_actual = 1;
      while (int_actual < int_goal) {
	++level;
	order = (level) ? (unsigned short)std::pow(2., (int)level) + 1 : 1;
	int_actual = (order % 2) ? order : order - 1;// for CC; also used for NC
      }
      tpqDriver->quadrature_order(order, i);             break;
    }
    case Pecos::FEJER2: { // open rule
      unsigned short int_goal = 2*quad_order_ref[i] - 1, level = 0, order,
	int_actual = 1;
      while (int_actual < int_goal) {
	++level;
	order = (unsigned short)std::pow(2., (int)level+1) - 1;
	int_actual = (order % 2) ? order : order - 1;// for CC; also used for F2
      }
      tpqDriver->quadrature_order(order, i);             break;
    }
    case Pecos::GAUSS_PATTERSON: { // open rule
      unsigned short int_goal = 2*quad_order_ref[i] - 1, level = 0, order = 1,
	int_actual = 1, previous = order;
      while (int_actual < int_goal) {
	++level;
	order = (unsigned short)std::pow(2., (int)level+1) - 1; // exp growth
	int_actual = 2*order - previous; // 2m-1 - constraints + 1
	previous = order;
      }
      tpqDriver->quadrature_order(order, i);             break;
    }
    case Pecos::GENZ_KEISTER: { // open rule with lookup
      unsigned short int_goal = 2*quad_order_ref[i] - 1,
	level = 0, max_level = 5;
      while (level <= max_level && gk_prec[level] < int_goal)
	++level;
      tpqDriver->quadrature_order(gk_order[level], i); break;
      /*
      unsigned short int_goal = 2*quad_order_ref[i] - 1, level = 0, order = 1,
	int_actual = 1, previous = order, i_rule = Pecos::GENZ_KEISTER,
	g_rule = Pecos::FULL_EXPONENTIAL; // map l->o without restriction
      while (int_actual < int_goal) {
	++level;
	webbur::level_growth_to_order_new(1, &level, &i_rule, &g_rule, order);
	int_actual = 2*order - previous; // 2m-1 - constraints + 1
	previous = order;
      }
      tpqDriver->quadrature_order(order, i);             break;
      */
    }
    default:
      tpqDriver->quadrature_order(quad_order_ref[i], i); break;
    }
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

  if (nestedRules) nested_quadrature_order(dim_quad_order);
  else         tpqDriver->quadrature_order(dim_quad_order);
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
  anisotropic_preference(dim_pref, dim_quad_order);

  if (nestedRules) nested_quadrature_order(dim_quad_order);
  else         tpqDriver->quadrature_order(dim_quad_order);
}


/** This version of anisotropic_preference() converts a scalar
    quad_order_spec and a dim_pref vector into a quad_order vector.  It is
    used for initialization and does not enforce a reference lower bound. */
void NonDQuadrature::
anisotropic_preference(unsigned short quad_order_spec,
		       const RealVector& dim_pref_spec,
		       UShortArray& dim_quad_order)
{
  Real max_dim_pref = dim_pref_spec[0]; size_t i, max_dim_pref_index = 0;
  for (i=1; i<numContinuousVars; ++i)
    if (dim_pref_spec[i] > max_dim_pref)
      { max_dim_pref = dim_pref_spec[i]; max_dim_pref_index = i; }

  dim_quad_order.resize(numContinuousVars);
  for (i=0; i<numContinuousVars; ++i)
    dim_quad_order[i] = (i == max_dim_pref_index) ? quad_order_spec :
      (unsigned short)(quad_order_spec*dim_pref_spec[i]/max_dim_pref);//truncate
}


void NonDQuadrature::
anisotropic_preference(const RealVector& dim_pref, UShortArray& quad_order_ref)
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
  if (nestedRules)
    nested_quadrature_order(quad_order_ref);
  else
    tpqDriver->quadrature_order(quad_order_ref);
  */
}

} // namespace Dakota
