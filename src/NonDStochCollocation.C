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
#include "DakotaModel.H"
#include "DakotaResponse.H"
#include "ProblemDescDB.H"
#include "DataFitSurrModel.H"
#include "PecosApproximation.H"


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
    if (gradientType != "none") data_order |= 2;
    if (hessianType  != "none") data_order |= 4;
  }
  short u_space_type = probDescDB.get_short("method.nond.expansion_type");
  bool  use_derivs = (data_order > 1), piecewise_basis
    = (u_space_type == PIECEWISE_U || refineType == Pecos::H_REFINEMENT);
  // LHS/Incremental LHS/Quadrature/SparseGrid samples in u-space
  // generated using active sampling view:
  Iterator u_space_sampler;
  const UShortArray& quad_order_spec
    = probDescDB.get_dusa("method.nond.quadrature_order");
  unsigned short ssg_level_spec
    = probDescDB.get_ushort("method.nond.sparse_grid_level");
  if (!quad_order_spec.empty()) {
    expansionCoeffsApproach = Pecos::QUADRATURE;
    construct_quadrature(u_space_sampler, g_u_model, quad_order_spec,
			 piecewise_basis, use_derivs);
  }
  else if (ssg_level_spec != USHRT_MAX) {
    expansionCoeffsApproach = Pecos::SPARSE_GRID;
    construct_sparse_grid(u_space_sampler, g_u_model, ssg_level_spec,
      probDescDB.get_rdv("method.nond.sparse_grid_dimension_preference"),
      piecewise_basis, use_derivs);
  }
  // iteratedModel concurrency is defined by the number of samples
  // used in constructing the expansion
  if (numSamplesOnModel) // optional with default = 0
    maxConcurrency *= numSamplesOnModel;

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for SCBDO with polynomials over {u}+{d}, change view to All.
  short  corr_order = -1;
  String corr_type, pt_reuse, approx_type = (piecewise_basis) ?
    "piecewise_interpolation_polynomial" : "global_interpolation_polynomial";
  UShortArray approx_order; // empty
  //const Variables& g_u_vars = g_u_model.current_variables();
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
    //g_u_vars.view(), g_u_vars.variables_components(),
    //g_u_model.current_response().active_set(),
    approx_type, approx_order, corr_type, corr_order, data_order, pt_reuse),
    false);
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
  NonDExpansion(model, exp_coeffs_approach, u_space_type, use_derivs)
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
  short data_order = 1;
  if (use_derivs) {// || iteratedModel.icv()
    if (gradientType != "none") data_order |= 2;
    if (hessianType  != "none") data_order |= 4;
  }
  bool piecewise_basis
    = (u_space_type == PIECEWISE_U || refineType == Pecos::H_REFINEMENT);
  // LHS/Incremental LHS/Quadrature/SparseGrid samples in u-space
  // generated using active sampling view:
  Iterator u_space_sampler;
  if (expansionCoeffsApproach == Pecos::QUADRATURE) {
    UShortArray quad_order(numContinuousVars, num_int_level);
    construct_quadrature(u_space_sampler, g_u_model, quad_order,
			 piecewise_basis, use_derivs);
  }
  else if (expansionCoeffsApproach == Pecos::SPARSE_GRID) {
    RealVector dim_pref; // empty
    construct_sparse_grid(u_space_sampler, g_u_model, num_int_level, dim_pref,
			  piecewise_basis, use_derivs);
  }
  // iteratedModel concurrency is defined by the number of samples
  // used in constructing the expansion
  if (numSamplesOnModel) // optional with default = 0
    maxConcurrency *= numSamplesOnModel;

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for SCBDO with polynomials over {u}+{d}, change view to All.
  short  corr_order = -1;
  String corr_type, pt_reuse, approx_type = (piecewise_basis) ?
    "piecewise_interpolation_polynomial" : "global_interpolation_polynomial";
  UShortArray approx_order; // empty
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
    approx_type, approx_order, corr_type, corr_order, data_order, pt_reuse),
    false);
  initialize_u_space_model();

  // no expansionSampler, no numSamplesOnExpansion
}


NonDStochCollocation::~NonDStochCollocation()
{
  if (numSamplesOnExpansion)
    uSpaceModel.free_communicators(
      numSamplesOnExpansion*uSpaceModel.derivative_concurrency());
}


void NonDStochCollocation::print_moments(std::ostream& s)
{
  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision);

  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  const StringArray& fn_labels = iteratedModel.response_labels();
  size_t i, j, width = write_precision+7;

  s << "\nMoment-based statistics for each response function:\n"
    << std::setw(width+15) << "Mean"     << std::setw(width+1) << "Std Dev"
    << std::setw(width+1)  << "Skewness" << std::setw(width+2) << "Kurtosis\n";
  PecosApproximation* poly_approx_rep; size_t num_mom;
  for (i=0; i<numFunctions; ++i) {
    poly_approx_rep = (PecosApproximation*)poly_approxs[i].approx_rep();
    if (poly_approx_rep->expansion_coefficient_flag()) {
      const RealVector& num_moments = poly_approx_rep->numerical_moments();
      num_mom = num_moments.length();

      s << std::setw(14) << fn_labels[i];
      for (j=0; j<num_mom; ++j)
	if (j==1) s << ' ' << std::setw(width) << std::sqrt(num_moments[j]);
	else      s << ' ' << std::setw(width) << num_moments[j];
      s << '\n';

      /* COV has been removed:
      if (std::abs(mean) > 1.e-25)
        s << "  " << std::setw(width)   << std_dev/mean << '\n';
      else
        s << "  " << std::setw(width+1) << "Undefined\n";
      */
    }
  }
}

} // namespace Dakota
