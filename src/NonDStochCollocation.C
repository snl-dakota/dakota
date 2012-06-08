/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDStochCollocation
//- Description: Implementation code for NonDStochCollocation class
//- Owner:       Mike Eldred

#include "system_defs.h"
#include "NonDStochCollocation.H"
#include "NonDSparseGrid.H"
#include "DakotaModel.H"
#include "DakotaResponse.H"
#include "ProblemDescDB.H"
#include "DataFitSurrModel.H"
#include "PecosApproximation.H"
#include "InterpPolyApproximation.hpp"

//#define ALLOW_GLOBAL_HERMITE_INTERPOLATION


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator
    instantiation using the ProblemDescDB. */
NonDStochCollocation::NonDStochCollocation(Model& model):
  NonDExpansion(model), sgBasisType(DEFAULT_INTERPOLANT)
{
  // ----------------------------------------------
  // Resolve settings and initialize natafTransform
  // ----------------------------------------------
  short data_order,
    u_space_type = probDescDB.get_short("method.nond.expansion_type");
  resolve_inputs(u_space_type, data_order);
  initialize(u_space_type);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  bool global_bnds
    = (numContDesVars || numContEpistUncVars || numContStateVars);
  construct_u_space_model(iteratedModel, g_u_model, global_bnds);

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  // LHS/Incremental LHS/Quadrature/SparseGrid samples in u-space
  // generated using active sampling view:
  Iterator u_space_sampler;
  const UShortArray& quad_order_spec
    = probDescDB.get_usa("method.nond.quadrature_order");
  const UShortArray& ssg_level_spec
    = probDescDB.get_usa("method.nond.sparse_grid_level");
  const RealVector& dim_pref
    = probDescDB.get_rv("method.nond.dimension_preference");
  if (!quad_order_spec.empty()) {
    expansionCoeffsApproach = Pecos::QUADRATURE;
    construct_quadrature(u_space_sampler, g_u_model, quad_order_spec, dim_pref);
  }
  else if (!ssg_level_spec.empty()) {
    sgBasisType = probDescDB.get_short("method.nond.sparse_grid_basis_type");
    switch (sgBasisType) {
    case HIERARCHICAL_INTERPOLANT:
      expansionCoeffsApproach = Pecos::HIERARCHICAL_SPARSE_GRID;          break;
    case NODAL_INTERPOLANT:
      expansionCoeffsApproach = Pecos::COMBINED_SPARSE_GRID;              break;
    case DEFAULT_INTERPOLANT:
      if ( u_space_type == STD_UNIFORM_U && nestedRules &&// TO DO:retire nested
	   ( refineControl == Pecos::DIMENSION_ADAPTIVE_CONTROL_GENERALIZED ||
	     refineControl == Pecos::LOCAL_ADAPTIVE_CONTROL ) ) {
	expansionCoeffsApproach = Pecos::HIERARCHICAL_SPARSE_GRID;
	sgBasisType = HIERARCHICAL_INTERPOLANT;
      }
      else {
	expansionCoeffsApproach = Pecos::COMBINED_SPARSE_GRID;
	sgBasisType = NODAL_INTERPOLANT;
      }
      break;
    }
    /*
    if (refineControl == Pecos::LOCAL_ADAPTIVE_CONTROL) {
      if (!piecewiseBasis || sgBasisType != HIERARCHICAL_INTERPOLANT) {
	// TO DO: promote this error check to resolve_inputs()
	PCerr << "Warning: overriding basis type to local hierarchical\n.";
	piecewiseBasis = true; sgBasisType = HIERARCHICAL_INTERPOLANT;
      }
      expansionCoeffsApproach = Pecos::HIERARCHICAL_SPARSE_GRID;
    }
    */
    construct_sparse_grid(u_space_sampler, g_u_model, ssg_level_spec, dim_pref);
  }

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for SCBDO with polynomials over {u}+{d}, change view to All.
  short  corr_order = -1, corr_type = NO_CORRECTION;
  String pt_reuse, approx_type;
  if (piecewiseBasis)
    approx_type = (sgBasisType == HIERARCHICAL_INTERPOLANT) ? 
      "piecewise_hierarchical_interpolation_polynomial" :
      "piecewise_nodal_interpolation_polynomial";
  else
    approx_type = (sgBasisType == HIERARCHICAL_INTERPOLANT) ?
      "global_hierarchical_interpolation_polynomial" :
      "global_nodal_interpolation_polynomial";
  UShortArray approx_order; // empty
  //const Variables& g_u_vars = g_u_model.current_variables();
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
    //g_u_vars.view(), g_u_vars.variables_components(),
    //g_u_model.current_response().active_set(),
    approx_type, approx_order, corr_type, corr_order, data_order, pt_reuse,
    outputLevel), false);
  initialize_u_space_model();

  // -------------------------------
  // Construct expSampler, if needed
  // -------------------------------
  construct_expansion_sampler();

  // uSpaceModel concurrency is defined by the number of samples used
  // in evaluating the stochastic expansion
  uSpaceModel.init_communicators(
    numSamplesOnExpansion*uSpaceModel.derivative_concurrency());
}


/** This constructor is used for helper iterator instantiation on the fly. */
NonDStochCollocation::
NonDStochCollocation(Model& model, short exp_coeffs_approach,
		     unsigned short num_int_level, short u_space_type,
		     bool piecewise_basis, bool use_derivs):
  NonDExpansion(model, exp_coeffs_approach, u_space_type,
		piecewise_basis, use_derivs), sgBasisType(DEFAULT_INTERPOLANT)
{
  // ----------------------------------------------
  // Resolve settings and initialize natafTransform
  // ----------------------------------------------
  short data_order;
  resolve_inputs(u_space_type, data_order);
  initialize(u_space_type);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  bool global_bnds
    = (numContDesVars || numContEpistUncVars || numContStateVars);
  construct_u_space_model(iteratedModel, g_u_model, global_bnds);

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  RealVector  dim_pref;                      // empty -> isotropic
  UShortArray num_int_seq(1, num_int_level); // single sequence
  // LHS/Incremental LHS/Quadrature/SparseGrid samples in u-space
  // generated using active sampling view:
  Iterator u_space_sampler;
  switch (expansionCoeffsApproach) {
  case Pecos::QUADRATURE:
    // sgBasisType left as DEFAULT_INTERPOLANT
    construct_quadrature(u_space_sampler, g_u_model, num_int_seq, dim_pref);
    break;
  case Pecos::COMBINED_SPARSE_GRID:
    sgBasisType = NODAL_INTERPOLANT;
    construct_sparse_grid(u_space_sampler, g_u_model, num_int_seq, dim_pref);
    break;
  case Pecos::HIERARCHICAL_SPARSE_GRID:
    sgBasisType = HIERARCHICAL_INTERPOLANT;
    construct_sparse_grid(u_space_sampler, g_u_model, num_int_seq, dim_pref);
    break;
  }

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for SCBDO with polynomials over {u}+{d}, change view to All.
  short  corr_order = -1, corr_type = NO_CORRECTION;
  String pt_reuse, approx_type;
  if (piecewiseBasis)
    approx_type = (sgBasisType == HIERARCHICAL_INTERPOLANT) ?
      "piecewise_hierarchical_interpolation_polynomial" :
      "piecewise_nodal_interpolation_polynomial";
  else
    approx_type = (sgBasisType == HIERARCHICAL_INTERPOLANT) ?
      "global_hierarchical_interpolation_polynomial" :
      "global_nodal_interpolation_polynomial";
  UShortArray approx_order; // empty
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
    approx_type, approx_order, corr_type, corr_order, data_order, pt_reuse,
    outputLevel), false);
  initialize_u_space_model();

  // no expansionSampler, no numSamplesOnExpansion
}


NonDStochCollocation::~NonDStochCollocation()
{
  if (numSamplesOnExpansion)
    uSpaceModel.free_communicators(
      numSamplesOnExpansion*uSpaceModel.derivative_concurrency());
}


void NonDStochCollocation::
resolve_inputs(short& u_space_type, short& data_order)
{
  // perform first due to piecewiseBasis overrides
  NonDExpansion::resolve_inputs(u_space_type, data_order);

  // There are two derivative cases of interest: (1) derivatives used as
  // additional data for forming the approximation (derivatives w.r.t. the
  // expansion variables), and (2) derivatives that will be approximated 
  // separately (derivatives w.r.t. auxilliary variables).  The data_order
  // passed through the DataFitSurrModel defines Approximation::buildDataOrder,
  // which is restricted to managing the former case.  If we need to manage the
  // latter case in the future, we do not have a good way to detect this state
  // at construct time, as neither the finalStats ASV/DVV nor subIteratorFlag
  // have yet been defined.  One indicator that is defined is the presence of
  // inactive continuous vars, since the subModel inactive view is updated
  // within the NestedModel ctor prior to subIterator instantiation.
  data_order = 1;
  if (useDerivs) { // input specification
    if (gradientType  != "none") data_order |= 2;
    //if (hessianType != "none") data_order |= 4; // not yet supported
#ifdef ALLOW_GLOBAL_HERMITE_INTERPOLATION
    if (data_order == 1)
      Cerr << "\nWarning: use_derivatives option in stoch_collocation "
	   << "requires a response\n         gradient specification.  "
	   << "Option will be ignored.\n" << std::endl;
#else
    if (piecewiseBasis) {
      if (data_order == 1)
	Cerr << "\nWarning: use_derivatives option in stoch_collocation "
	     << "requires a response\n         gradient specification.  "
	     << "Option will be ignored.\n" << std::endl;
    }
    else {
      Cerr << "\nWarning: use of global gradient-enhanced interpolants is "
	   << "disallowed in production\n         executables.  To activate "
	   << "this research capability, define\n         ALLOW_GLOBAL_HERMITE_"
	   << "INTERPOLATION in Dakota::NonDStochCollocation and recompile.\n"
	   << std::endl;
      data_order = 1;
    }
#endif
  }
  useDerivs = (data_order > 1); // override input specification

  // override u_space_type to STD_UNIFORM_U for global Hermite interpolation
  if (useDerivs && !piecewiseBasis) {
    if (u_space_type == ASKEY_U) // non-default
      Cerr << "\nWarning: overriding ASKEY to STD_UNIFORM for Hermite "
	   << "interpolation.\n" << std::endl;
    else if (u_space_type == STD_NORMAL_U) // non-default
      Cerr << "\nWarning: overriding WIENER to STD_UNIFORM for Hermite "
	   << "interpolation.\n" << std::endl;
    u_space_type = STD_UNIFORM_U;
  }
}


void NonDStochCollocation::initialize_u_space_model()
{
  // build a polynomial basis for purposes of defining collocation pts/wts
  std::vector<Pecos::BasisPolynomial> num_int_poly_basis;
  Pecos::BasisConfigOptions bc_options(nestedRules, piecewiseBasis,
				       true, useDerivs);
  Pecos::InterpPolyApproximation::construct_basis(natafTransform.u_types(),
    iteratedModel.distribution_parameters(), bc_options, num_int_poly_basis);

  // set the polynomial basis within the NonDIntegration instance
  NonDIntegration* u_space_sampler_rep
    = (NonDIntegration*)uSpaceModel.subordinate_iterator().iterator_rep();
  u_space_sampler_rep->initialize_grid(num_int_poly_basis);

  // perform last due to numSamplesOnModel update
  NonDExpansion::initialize_u_space_model();
}


void NonDStochCollocation::update_expansion()
{
  if (sgBasisType == HIERARCHICAL_INTERPOLANT) {
    // grid levels have been updated, now evaluate the new points
    NonDSparseGrid* nond_sparse = (NonDSparseGrid*)
      uSpaceModel.subordinate_iterator().iterator_rep();
    nond_sparse->evaluate_grid_increment(); // like NonDSG::evaluate_set()
    // append the new data to the existing approximation and rebuild
    uSpaceModel.append_approximation(true); // rebuild
  }
  else
    NonDExpansion::update_expansion(); // default: build from scratch
}


Real NonDStochCollocation::compute_covariance_metric()
{
  if (sgBasisType == HIERARCHICAL_INTERPOLANT) {
    size_t i, j;
    RealSymMatrix delta_resp_covar(numFunctions, false);
    bool warn_flag = false,
      all_vars = (numContDesVars || numContEpistUncVars || numContStateVars);
    std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
    for (i=0; i<numFunctions; ++i) {
      PecosApproximation* pa_rep_i
	= (PecosApproximation*)poly_approxs[i].approx_rep();
      if (pa_rep_i->expansion_coefficient_flag())
	for (j=0; j<=i; ++j) {
	  PecosApproximation* pa_rep_j
	    = (PecosApproximation*)poly_approxs[j].approx_rep();
	  if (pa_rep_j->expansion_coefficient_flag())
	    delta_resp_covar(i,j) = (all_vars) ?
	      pa_rep_i->delta_covariance(initialPtU, pa_rep_j) :
	      pa_rep_i->delta_covariance(pa_rep_j);
	  else
	    { warn_flag = true; delta_resp_covar(i,j) = 0.; }
	}
      else {
	warn_flag = true;
	for (j=0; j<=i; ++j)
	  delta_resp_covar(i,j) = 0.;
      }
    }
    if (warn_flag)
      Cerr << "Warning: expansion coefficients unavailable in "
	   << "NonDStochCollocation::compute_covariance_metric().\n         "
	   << "Zeroing affected delta_covariance terms." << std::endl;
    // reference covariance gets restored in NonDExpansion::increment_sets()
    respCovariance += delta_resp_covar;
    return delta_resp_covar.normFrobenius();
  }
  else // use default implementation
    return NonDExpansion::compute_covariance_metric();
}

} // namespace Dakota
