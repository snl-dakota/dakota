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
  //short growth_rate = Pecos::MODERATE_RESTRICTED_GROWTH;//SLOW_,UNRESTRICTED_
  //short nested_uniform_rule = Pecos::GAUSS_PATTERSON;//CLENSHAW_CURTIS,FEJER2
  tpqDriver->initialize_grid(natafTransform.u_types(), nestedRules);
                             //, growth_rate, nested_uniform_rule);
  // check_integration() needs integrationRules from initialize_grid()
  check_integration(probDescDB.get_dusa("method.nond.quadrature_order"));
  maxConcurrency *= tpqDriver->grid_size();
}


/** This alternate constructor is used for on-the-fly generation and
    evaluation of numerical quadrature points. */
NonDQuadrature::
NonDQuadrature(Model& model, const Pecos::ShortArray& u_types,
	       const UShortArray& order, bool nested_rules): 
  NonDIntegration(NoDBBaseConstructor(), model), nestedRules(nested_rules),
  numFilteredSamples(0)
{
  // initialize the numerical integration driver
  numIntDriver = Pecos::IntegrationDriver(Pecos::QUADRATURE);
  tpqDriver = (Pecos::TensorProductDriver*)numIntDriver.driver_rep();

  // local natafTransform not yet updated: x_types would have to be passed in
  // from NonDExpansion if check_variables() needed to be called here.  Instead,
  // it is deferred until run time in NonDIntegration::quantify_uncertainty().
  //check_variables(x_types);
  //short growth_rate = Pecos::MODERATE_RESTRICTED_GROWTH;//SLOW_,UNRESTRICTED_
  //short nested_uniform_rule = Pecos::GAUSS_PATTERSON;//CLENSHAW_CURTIS,FEJER2
  tpqDriver->initialize_grid(u_types, nestedRules);
                             //, growth_rate, nested_uniform_rule);
  check_integration(order); // needs integrationRules from initialize_grid()
  maxConcurrency *= tpqDriver->grid_size();
}


/** This alternate constructor is used for on-the-fly generation and
    evaluation of numerical quadrature points. */
NonDQuadrature::
NonDQuadrature(Model& model, const Pecos::ShortArray& u_types,
	       int num_filt_samples, bool nested_rules): 
  NonDIntegration(NoDBBaseConstructor(), model),
  numFilteredSamples(num_filt_samples), nestedRules(nested_rules)
{
  // initialize the numerical integration driver
  numIntDriver = Pecos::IntegrationDriver(Pecos::QUADRATURE);
  tpqDriver = (Pecos::TensorProductDriver*)numIntDriver.driver_rep();

  // local natafTransform not yet updated: x_types would have to be passed in
  // from NonDExpansion if check_variables() needed to be called here.  Instead,
  // it is deferred until run time in NonDIntegration::quantify_uncertainty().
  //check_variables(x_types);
  //short growth_rate = Pecos::MODERATE_RESTRICTED_GROWTH;//SLOW_,UNRESTRICTED_
  //short nested_uniform_rule = Pecos::GAUSS_PATTERSON;//CLENSHAW_CURTIS,FEJER2
  tpqDriver->initialize_grid(u_types, nestedRules);
                             //, growth_rate, nested_uniform_rule);
  compute_min_order();
  maxConcurrency *= numFilteredSamples;
}


NonDQuadrature::~NonDQuadrature()
{ }


void NonDQuadrature::check_integration(const UShortArray& quad_order_spec)
{
  bool err_flag = false;

  // Update quadOrderRef from quad_order_spec (the original user specification)
  size_t quad_spec_len = quad_order_spec.size();
  if (quad_spec_len == numContinuousVars)
    quadOrderRef = quadOrderSpec = quad_order_spec;
  else if (quad_spec_len == 1) {
    unsigned short quad_order = quad_order_spec[0];
    quadOrderRef.reserve(numContinuousVars);
    quadOrderRef.assign(numContinuousVars, quad_order);
    quadOrderSpec.reserve(numContinuousVars);
    quadOrderSpec.assign(numContinuousVars, quad_order);
  }
  else {
    err_flag = true;
    if (quad_spec_len == 0)
      Cerr << "Error: quadrature order specification required in "
	   << "NonDQuadrature::check_quadrature()." << std::endl;
    else
      Cerr << "Error: length of quadrature order specification is "
	   << "inconsistent with active\n       continuous variables in "
	   << "NonDQuadrature::check_quadrature()." << std::endl;
  }

  if (err_flag)
    abort_handler(-1);

  // Update TensorProductDriver::quadOrder from quadOrderRef
  if (nestedRules) nested_quadrature_order(quadOrderRef);
  else         tpqDriver->quadrature_order(quadOrderRef);
}


void NonDQuadrature::compute_min_order()
{
  unsigned short min_order = 1; size_t i, num_tensor_pts = 1;
  while (num_tensor_pts < numFilteredSamples) {
    ++min_order; num_tensor_pts = min_order;
    for (i=1; i<numContinuousVars; ++i)
      num_tensor_pts *= min_order;
  }
  quadOrderRef.reserve(numContinuousVars);
  quadOrderRef.assign(numContinuousVars, min_order);

  // Update TensorProductDriver::quadOrder from quadOrderRef
  if (nestedRules) nested_quadrature_order(quadOrderRef);
  else         tpqDriver->quadrature_order(quadOrderRef);
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
  const Pecos::RealVector& wts = tpqDriver->weight_sets();
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
  // quadrature order may be increased ***or decreased*** to provide at least
  // min_samples, but the original user specification (quadOrderSpec) is a
  // hard lower bound.  With the introduction of uniform/adaptive refinements,
  // quadOrderRef (which is incremented from quadOrderSpec) replaces
  // quadOrderSpec as the lower bound.
  if (min_samples > reference_grid_size()) {
    // enforcing a minimum isotropic order for an anisotropic expansion isn't
    // exact, but it's close enough since this function should only come into
    // play for regression-based isotropic PCE.
    unsigned short min_iso_order = (unsigned short)std::ceil(
      std::pow((Real)min_samples, 1./(Real)numContinuousVars) - 1.e-10);
    if (nestedRules) {
      UShortArray new_quad_order_ref(numContinuousVars);
      for (size_t i=0; i<numContinuousVars; ++i)
	new_quad_order_ref[i] = std::max(min_iso_order, quadOrderRef[i]);
      nested_quadrature_order(new_quad_order_ref);
    }
    else
      for (size_t i=0; i<numContinuousVars; ++i)
	tpqDriver->
	  quadrature_order(std::max(min_iso_order, quadOrderRef[i]), i);

    // maxConcurrency must not be updated since parallel config management
    // depends on having the same value at ctor/run/dtor times.
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
  const Pecos::IntArray&    rules    = tpqDriver->integration_rules();
  const Pecos::UShortArray& gk_order = tpqDriver->genz_keister_order();
  const Pecos::UShortArray& gk_prec  = tpqDriver->genz_keister_precision();
  for (size_t i=0; i<numContinuousVars; ++i)
    switch (rules[i]) {
    case Pecos::CLENSHAW_CURTIS: case Pecos::FEJER2: {
      unsigned short int_goal = 2*quad_order_ref[i] - 1, level = 0, order,
	int_actual = 1;
      while (int_actual < int_goal) {
	++level;
	order = (level) ? (unsigned short)std::pow(2., (int)level) + 1 : 1;
	int_actual = (order % 2) ? order : order - 1;
      }
      tpqDriver->quadrature_order(order, i);             break;
    }
    case Pecos::GAUSS_PATTERSON: {
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
    case Pecos::GENZ_KEISTER: {
      unsigned short int_goal = 2*quad_order_ref[i] - 1,
	level = 0, max_level = 4;
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
    // ensure change in presence of slow growth
    while (tpqDriver->grid_size() == orig_size)
      { increment_reference();  nested_quadrature_order(quadOrderRef); }
  }
  else
    { increment_reference(); tpqDriver->quadrature_order(quadOrderRef); }
}


void NonDQuadrature::increment_grid(const RealVector& dim_pref)
{
  // Used for dimension-adaptive refinement: order lower bounds are enforced
  // using quadOrderRef such that anisotropy may not reduce dimension resolution
  // once performed (as w/ SparseGridDriver::axisLowerBounds in NonDSparseGrid).
  if (nestedRules) {
    // define reference point
    size_t orig_size = tpqDriver->grid_size();
    // initial increment, anisotropy update, and nestedness enforcement
    increment_reference(dim_pref);     nested_quadrature_order(quadOrderRef);
    // ensure change in presence of slow growth
    while (tpqDriver->grid_size() == orig_size)
      { increment_reference(dim_pref); nested_quadrature_order(quadOrderRef); }
  }
  else {
    increment_reference(dim_pref);
    tpqDriver->quadrature_order(quadOrderRef);
  }
}


void NonDQuadrature::anisotropic_preference(const RealVector& dim_pref)
{
  // Logic is loosely patterned after anisotropic SSG in which the dominant
  // dimension is constrained by the current ssgLevel and all nondominant
  // dimensions are scaled back.
  unsigned short max_quad_ref = quadOrderRef[0];
  Real max_dim_pref = dim_pref[0]; size_t max_dim_pref_index = 0;
  for (size_t i=1; i<numContinuousVars; ++i) {
    if (quadOrderRef[i] > max_quad_ref)
      max_quad_ref = quadOrderRef[i];
    if (dim_pref[i] > max_dim_pref)
      { max_dim_pref = dim_pref[i]; max_dim_pref_index = i; }
  }
  // There are several different options for the following operation.
  // I have initially chosen to preserve the current (recently incremented)
  // quadOrderRef entry in the dimension with max_dim_pref and to compute
  // the others relative to max_quad_ref (whose dimension may differ from
  // max_dim_pref).  Could also, e.g., decide to set max_quad_ref in the
  // max_dim_pref dimension, but this could be a much larger increment.
  // In all cases, the current quadOrderRef is used as a lower bound to
  // prevent reducing previous resolution.
  for (size_t i=0; i<numContinuousVars; ++i)
    if (i != max_dim_pref_index)
      quadOrderRef[i] = std::max(quadOrderRef[i],
	(unsigned short)(max_quad_ref*dim_pref[i]/max_dim_pref));

  /* When used as a stand-alone update:
  if (nestedRules)
    nested_quadrature_order(quadOrderRef);
  else
    quadOrder = quadOrderRef;
  */
}

} // namespace Dakota
