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
  nestedRules(false)
{
  // initialize the numerical integration driver
  numIntDriver = Pecos::IntegrationDriver(Pecos::QUADRATURE);
  tpqDriver = (Pecos::TensorProductDriver*)numIntDriver.driver_rep();

  // natafTransform available: initialize_random_variables() called in
  // NonDIntegration ctor
  check_variables(natafTransform.x_types());

  short u_space_type = probDescDB.get_short("method.nond.expansion_type");
  short refine_type
    = probDescDB.get_short("method.nond.expansion_refinement_type");
  bool piecewise_basis
    = (u_space_type == PIECEWISE_U || refine_type == Pecos::H_REFINEMENT);
  bool use_derivs = probDescDB.get_bool("method.derivative_usage");
  bool equidistant_rules = true; // NEWTON_COTES pts for piecewise interpolants
  short nested_uniform_rule = Pecos::GAUSS_PATTERSON;//CLENSHAW_CURTIS,FEJER2
  tpqDriver->initialize_grid(natafTransform.u_types(), nestedRules,
                             piecewise_basis, equidistant_rules, use_derivs,
			     nested_uniform_rule);

  // initialize_quadrature_order() needs integrationRules from initialize_grid()
  initialize_quadrature_order(
    probDescDB.get_dusa("method.nond.quadrature_order")[0]);
  maxConcurrency *= tpqDriver->grid_size();
}


/** This alternate constructor is used for on-the-fly generation and
    evaluation of numerical quadrature points. */
NonDQuadrature::
NonDQuadrature(Model& model, const Pecos::ShortArray& u_types,
	       unsigned short order, const RealVector& dim_pref,
	       bool nested_rules, bool piecewise_basis, bool use_derivs): 
  NonDIntegration(NoDBBaseConstructor(), model, dim_pref),
  nestedRules(nested_rules), numFilteredSamples(0)
{
  // initialize the numerical integration driver
  numIntDriver = Pecos::IntegrationDriver(Pecos::QUADRATURE);
  tpqDriver = (Pecos::TensorProductDriver*)numIntDriver.driver_rep();

  // local natafTransform not yet updated: x_types would have to be passed in
  // from NonDExpansion if check_variables() needed to be called here.  Instead,
  // it is deferred until run time in NonDIntegration::quantify_uncertainty().
  //check_variables(x_types);

  bool equidistant_rules = true; // NEWTON_COTES pts for piecewise interpolants
  short nested_uniform_rule = Pecos::GAUSS_PATTERSON;//CLENSHAW_CURTIS,FEJER2
  tpqDriver->initialize_grid(u_types, nestedRules, piecewise_basis,
			     equidistant_rules, use_derivs,
			     nested_uniform_rule);

  initialize_quadrature_order(order); // use initialize_grid()->integrationRules
  maxConcurrency *= tpqDriver->grid_size();
}


/** This alternate constructor is used for on-the-fly generation and
    evaluation of numerical quadrature points. */
NonDQuadrature::
NonDQuadrature(Model& model, const Pecos::ShortArray& u_types,
	       int num_filt_samples, const RealVector& dim_pref,
	       bool nested_rules, bool piecewise_basis, bool use_derivs): 
  NonDIntegration(NoDBBaseConstructor(), model, dim_pref),
  numFilteredSamples(num_filt_samples), nestedRules(nested_rules)
{
  // initialize the numerical integration driver
  numIntDriver = Pecos::IntegrationDriver(Pecos::QUADRATURE);
  tpqDriver = (Pecos::TensorProductDriver*)numIntDriver.driver_rep();

  // local natafTransform not yet updated: x_types would have to be passed in
  // from NonDExpansion if check_variables() needed to be called here.  Instead,
  // it is deferred until run time in NonDIntegration::quantify_uncertainty().
  //check_variables(x_types);

  bool equidistant_rules = true; // NEWTON_COTES pts for piecewise interpolants
  short nested_uniform_rule = Pecos::GAUSS_PATTERSON;//CLENSHAW_CURTIS,FEJER2
  tpqDriver->initialize_grid(u_types, nestedRules, piecewise_basis,
			     equidistant_rules, use_derivs,
			     nested_uniform_rule);
  compute_min_quadrature_order(numFilteredSamples);
  maxConcurrency *= numFilteredSamples;
}


NonDQuadrature::~NonDQuadrature()
{ }


void NonDQuadrature::initialize_quadrature_order(unsigned short quad_order_spec)
{
  // Update quadOrder{Spec,Ref} from quad_order_spec and dimPrefSpec
  if (dimPrefSpec.empty()) { // update quadOrderSpec for aniso tensor grid
    quadOrderSpec.reserve(numContinuousVars);
    quadOrderSpec.assign(numContinuousVars, quad_order_spec);
  }
  else
    anisotropic_preference(quad_order_spec, dimPrefSpec, quadOrderSpec);
  quadOrderRef = quadOrderSpec;
  //dimPrefRef = dimPrefSpec; // not currently necessary

  // Update Pecos::TensorProductDriver::quadOrder from quadOrderRef
  if (nestedRules) nested_quadrature_order(quadOrderRef);
  else         tpqDriver->quadrature_order(quadOrderRef);
}


void NonDQuadrature::compute_min_quadrature_order(size_t min_samples)
{
  quadOrderRef.reserve(numContinuousVars);
  quadOrderRef.assign(numContinuousVars, 1);
  // compute minimal order tensor grid with at least numFilteredSamples points
  if (dimPrefSpec.empty()) // isotropic tensor grid
    while (tpqDriver->grid_size() < min_samples)
      increment_grid();
  else                   // anisotropic tensor grid
    while (tpqDriver->grid_size() < min_samples)
      increment_grid_preference(dimPrefSpec);
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
  // With the introduction of uniform/adaptive refinements, quadOrderRef
  // (which is incremented from quadOrderSpec) replaces quadOrderSpec as
  // the current lower bound.  Pecos::TensorProductDriver::quadOrder may be
  // increased ***or decreased*** to provide at least min_samples subject
  // to this lower bound.  quadOrderRef is ***not*** updated by min_samples.
  if (min_samples > tpqDriver->grid_size()) {
    // store quadOrderRef prior to temporary update by compute_min_quad_order()
    UShortArray quad_order_low_bnd = quadOrderRef;
    compute_min_quadrature_order(min_samples); // iso or aniso TPQ
    // enforce lower bound
    UShortArray new_quad_order(numContinuousVars);
    for (size_t i=0; i<numContinuousVars; ++i)
      new_quad_order[i] = std::max(quad_order_low_bnd[i], quadOrderRef[i]);
    if (nestedRules) nested_quadrature_order(new_quad_order);
    else         tpqDriver->quadrature_order(new_quad_order);
    // restore quadOrderRef
    quadOrderRef = quad_order_low_bnd;
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
	webbur::level_growth_to_order(1, &level, &i_rule, &g_rule, order);
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


void NonDQuadrature::increment_grid()
{
  // Used for uniform refinement: all quad orders are incremented by 1.
  if (nestedRules) {
    // define reference point
    size_t orig_size = tpqDriver->grid_size();
    // initial increment and nestedness enforcement
    increment_reference();      nested_quadrature_order(quadOrderRef);
    // ensure change in presence of restricted growth
    while (tpqDriver->grid_size() == orig_size)
      { increment_reference();  nested_quadrature_order(quadOrderRef); }
  }
  else
    { increment_reference(); tpqDriver->quadrature_order(quadOrderRef); }
}


void NonDQuadrature::increment_reference()
{
  // quadOrderRef is a reference point for quadOrder, e.g., a lower bound
  for (size_t i=0; i<numContinuousVars; ++i)
    quadOrderRef[i] += 1;
}


void NonDQuadrature::increment_grid_preference(const RealVector& dim_pref)
{
  // Used for dimension-adaptive refinement: order lower bounds are enforced
  // using quadOrderRef such that anisotropy may not reduce dimension resolution
  // once performed (as w/ SparseGridDriver::axisLowerBounds in NonDSparseGrid).
  if (nestedRules) {
    // define reference point
    size_t orig_size = tpqDriver->grid_size();
    // initial increment, anisotropy update, and nestedness enforcement
    increment_reference(dim_pref);     nested_quadrature_order(quadOrderRef);
    // ensure change in presence of restricted growth
    while (tpqDriver->grid_size() == orig_size)
      { increment_reference(dim_pref); nested_quadrature_order(quadOrderRef); }
  }
  else {
    increment_reference(dim_pref);
    tpqDriver->quadrature_order(quadOrderRef);
  }
}


void NonDQuadrature::increment_reference(const RealVector& dim_pref)
{
  // determine the dimension with max preference
  Real max_dim_pref = dim_pref[0]; size_t max_dim_pref_index = 0;
  for (size_t i=1; i<numContinuousVars; ++i)
    if (dim_pref[i] > max_dim_pref)
      { max_dim_pref = dim_pref[i]; max_dim_pref_index = i; }
  // increment only the dimension with max preference by 1
  quadOrderRef[max_dim_pref_index] += 1;
  // now balance the other dims relative to this new increment, preserving
  // previous resolution
  anisotropic_preference(dim_pref, quadOrderRef);
}


/** This version of anisotropic_preference() converts a scalar
    quad_order_spec and a dim_pref vector into a quad_order vector.  It is
    used for initialization and does not enforce a reference lower bound. */
void NonDQuadrature::
anisotropic_preference(unsigned short quad_order_spec,
		       const RealVector& dim_pref_spec, UShortArray& quad_order)
{
  Real max_dim_pref = dim_pref_spec[0]; size_t i, max_dim_pref_index = 0;
  for (i=1; i<numContinuousVars; ++i)
    if (dim_pref_spec[i] > max_dim_pref)
      { max_dim_pref = dim_pref_spec[i]; max_dim_pref_index = i; }

  quad_order.resize(numContinuousVars);
  for (i=0; i<numContinuousVars; ++i)
    quad_order[i] = (i == max_dim_pref_index) ? quad_order_spec :
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
