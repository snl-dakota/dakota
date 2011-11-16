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
#include "NonDIntegration.H"
#include "DakotaModel.H"
#include "DakotaResponse.H"
#include "ProblemDescDB.H"
#include "DataFitSurrModel.H"
#include "PecosApproximation.H"
#include "InterpPolyApproximation.hpp"

//#define ALLOW_HERMITE_INTERPOLATION


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator
    instantiation using the ProblemDescDB. */
NonDStochCollocation::NonDStochCollocation(Model& model): NonDExpansion(model)
{
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
  short u_space_type = probDescDB.get_short("method.nond.expansion_type");
  bool piecewise_basis
    = (u_space_type == PIECEWISE_U || refineType == Pecos::H_REFINEMENT);
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
  short data_order = 1;
  if (probDescDB.get_bool("method.derivative_usage")) {// || iteratedModel.icv()
    if (gradientType  != "none") data_order |= 2;
    //if (hessianType != "none") data_order |= 4; // not yet supported
#ifdef ALLOW_HERMITE_INTERPOLATION
    if (data_order == 1)
      Cerr << "\nWarning: use_derivatives option in stoch_collocation "
	   << "requires a response\n         gradient specification.  "
	   << "Option will be ignored.\n" << std::endl;
#else
    if (piecewise_basis) {
      if (data_order == 1)
	Cerr << "\nWarning: use_derivatives option in stoch_collocation "
	     << "requires a response\n         gradient specification.  "
	     << "Option will be ignored.\n" << std::endl;
    }
    else {
      Cerr << "\nWarning: use of global gradient-enhanced interpolants is "
	   << "disallowed in production\n         executables.  To activate "
	   << "this research capability, define\n         ALLOW_HERMITE_"
	   << "INTERPOLATION in Dakota::NonDStochCollocation and recompile.\n"
	   << std::endl;
      data_order = 1;
    }
#endif
  }
  if (data_order > 1) useDerivs = true;
  // LHS/Incremental LHS/Quadrature/SparseGrid samples in u-space
  // generated using active sampling view:
  Iterator u_space_sampler;
  const UShortArray& quad_order_spec
    = probDescDB.get_dusa("method.nond.quadrature_order");
  const UShortArray& ssg_level_spec
    = probDescDB.get_dusa("method.nond.sparse_grid_level");
  const RealVector& dim_pref
    = probDescDB.get_rdv("method.nond.dimension_preference");
  if (!quad_order_spec.empty()) {
    expansionCoeffsApproach = Pecos::QUADRATURE;
    construct_quadrature(u_space_sampler, g_u_model, quad_order_spec, dim_pref);
  }
  else if (!ssg_level_spec.empty()) {
    expansionCoeffsApproach = Pecos::SPARSE_GRID;
    construct_sparse_grid(u_space_sampler, g_u_model, ssg_level_spec, dim_pref,
			  piecewise_basis);
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
  if (piecewise_basis)
    approx_type = (probDescDB.get_short("method.nond.piecewise_basis_type") ==
		   HIERARCHICAL_INTERPOLANT) ? 
      "piecewise_hierarchical_interpolation_polynomial" :
      "piecewise_nodal_interpolation_polynomial";
  else
    approx_type = "global_interpolation_polynomial";
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
		     bool use_derivs):
  NonDExpansion(model, exp_coeffs_approach, u_space_type)
{
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
  bool piecewise_basis
    = (u_space_type == PIECEWISE_U || refineType == Pecos::H_REFINEMENT);
  short data_order = 1;
  if (use_derivs) {// || iteratedModel.icv()
    if (gradientType  != "none") data_order |= 2;
    //if (hessianType != "none") data_order |= 4; // not yet supported
#ifdef ALLOW_HERMITE_INTERPOLATION
    if (data_order == 1)
      Cerr << "\nWarning: use_derivatives option in stoch_collocation "
	   << "requires a response\n         gradient specification.  "
	   << "Option will be ignored.\n" << std::endl;
#else
    if (piecewise_basis) {
      if (data_order == 1)
	Cerr << "\nWarning: use_derivatives option in stoch_collocation "
	     << "requires a response\n         gradient specification.  "
	     << "Option will be ignored.\n" << std::endl;
    }
    else {
      Cerr << "\nWarning: use of global gradient-enhanced interpolants is "
	   << "disallowed in production\n         executables.  To activate "
	   << "this research capability, define\n         ALLOW_HERMITE_"
	   << "INTERPOLATION in Dakota::NonDStochCollocation and recompile.\n"
	   << std::endl;
      data_order = 1;
    }
#endif
  }
  if (data_order > 1) useDerivs = true;
  RealVector  dim_pref;                      // empty -> isotropic
  UShortArray num_int_seq(1, num_int_level); // single sequence
  // LHS/Incremental LHS/Quadrature/SparseGrid samples in u-space
  // generated using active sampling view:
  Iterator u_space_sampler;
  if (expansionCoeffsApproach == Pecos::QUADRATURE)
    construct_quadrature(u_space_sampler, g_u_model, num_int_seq, dim_pref);
  else if (expansionCoeffsApproach == Pecos::SPARSE_GRID)
    construct_sparse_grid(u_space_sampler, g_u_model, num_int_seq, dim_pref,
			  piecewise_basis);

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for SCBDO with polynomials over {u}+{d}, change view to All.
  short  corr_order = -1, corr_type = NO_CORRECTION;
  String pt_reuse, approx_type;
  if (piecewise_basis)
    // for now, rather than mapping piecewise basis type through, hardwire
    // a reasonable selection (nodal OK outside of local refinement)
    approx_type = (refineType == Pecos::H_REFINEMENT) ? 
      "piecewise_hierarchical_interpolation_polynomial" :
      "piecewise_nodal_interpolation_polynomial";
  else
    approx_type = "global_interpolation_polynomial";
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


void NonDStochCollocation::initialize_u_space_model()
{
  if (expansionCoeffsApproach == Pecos::QUADRATURE ||
      expansionCoeffsApproach == Pecos::CUBATURE ||
      expansionCoeffsApproach == Pecos::SPARSE_GRID) {

    // build a polynomial basis for purposes of defining collocation pts/wts
    std::vector<Pecos::BasisPolynomial> num_int_poly_basis;
    bool piecewise_basis = uSpaceModel.surrogate_type().begins("piecewise_");
    Pecos::BasisConfigOptions bc_options(nestedRules, piecewise_basis,
					 true, useDerivs);
    Pecos::InterpPolyApproximation::construct_basis(natafTransform.u_types(),
      iteratedModel.distribution_parameters(), bc_options, num_int_poly_basis);

    // set the polynomial basis within the NonDIntegration instance
    NonDIntegration* u_space_sampler_rep
      = (NonDIntegration*)uSpaceModel.subordinate_iterator().iterator_rep();
    u_space_sampler_rep->initialize_grid(num_int_poly_basis);
  }

  // perform last due to numSamplesOnModel update
  NonDExpansion::initialize_u_space_model();
}

} // namespace Dakota
