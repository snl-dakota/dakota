/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDPolynomialChaos
//- Description: Implementation code for NonDPolynomialChaos class
//- Owner:       Mike Eldred, Sandia National Laboratories

#include "system_defs.h"
#include "NonDPolynomialChaos.H"
#include "DakotaModel.H"
#include "DakotaResponse.H"
#include "ProblemDescDB.H"
#include "DataFitSurrModel.H"
#include "NonDQuadrature.H"
#include "NonDSampling.H"
#include "PecosApproximation.H"
#include "SparseGridDriver.hpp"
#include "TensorProductDriver.hpp"
#include "CubatureDriver.hpp"
#include "data_io.h"


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator
    instantiation using the ProblemDescDB. */
NonDPolynomialChaos::NonDPolynomialChaos(Model& model): NonDExpansion(model),
  expansionImportFile(
    probDescDB.get_string("method.nond.expansion_import_file")),
  expansionTerms(probDescDB.get_int("method.nond.expansion_terms")),
  collocRatio(probDescDB.get_real("method.nond.collocation_ratio")),
  tensorRegression(probDescDB.get_bool("method.nond.tensor_grid"))
{
  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  bool global_bnds
    = (numContDesVars || numContEpistUncVars || numContStateVars);
  if (iteratedModel.surrogate_type() == "hierarchical")
    iteratedModel.surrogate_response_mode(MODEL_DISCREPANCY);
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
  // expansion_order defined for expansion_samples/collocation_pts
  UShortArray exp_order = probDescDB.get_dusa("method.nond.expansion_order");
  if (exp_order.size() == 1) {
    unsigned short order = exp_order[0];
    exp_order.resize(numContinuousVars);
    exp_order.assign(numContinuousVars, order);
  }
  if (expansionImportFile.empty()) {
    const UShortArray& quad_order_spec
      = probDescDB.get_dusa("method.nond.quadrature_order");
    const UShortArray& ssg_level_spec
      = probDescDB.get_dusa("method.nond.sparse_grid_level");
    unsigned short cub_int_spec
      = probDescDB.get_ushort("method.nond.cubature_integrand");
    if (!quad_order_spec.empty()) {
      expansionCoeffsApproach = Pecos::QUADRATURE;
      construct_quadrature(u_space_sampler, g_u_model, quad_order_spec,
	probDescDB.get_rdv("method.nond.dimension_preference"),
	piecewise_basis, use_derivs);
    }
    else if (!ssg_level_spec.empty()) {
      expansionCoeffsApproach = Pecos::SPARSE_GRID;
      construct_sparse_grid(u_space_sampler, g_u_model, ssg_level_spec,
	probDescDB.get_rdv("method.nond.dimension_preference"),
	piecewise_basis, use_derivs);
    }
    else if (cub_int_spec != USHRT_MAX) {
      expansionCoeffsApproach = Pecos::CUBATURE;
      construct_cubature(u_space_sampler, g_u_model, cub_int_spec);
    }
    else { // expansion_samples or collocation_points
      int exp_samples = probDescDB.get_int("method.nond.expansion_samples");
      if (exp_samples > 0) { // expectation
	if (refineType) { // no obvious logic for sample refinement
	  Cerr << "Error: uniform/adaptive refinement of expansion_samples not "
	       << "supported." << std::endl;
	  abort_handler(-1);
	}
	numSamplesOnModel       = exp_samples;
	expansionCoeffsApproach = Pecos::SAMPLING;
	// reuse seed/rng settings intended for the expansion_sampler
	construct_lhs(u_space_sampler, g_u_model, numSamplesOnModel,
		      probDescDB.get_int("method.random_seed"),
		      probDescDB.get_string("method.random_number_generator"));
      }
      else { // regression
	if (refineType && refineControl > Pecos::UNIFORM_CONTROL) {
	  // adaptive precluded since grid anisotropy not readily supported
	  // for synchronization with expansion anisotropy.
	  Cerr << "Error: only uniform refinement is supported for LHS "
	       << "regression." << std::endl;
	  abort_handler(-1);
	}
	expansionCoeffsApproach = Pecos::REGRESSION;
	size_t exp_terms = (exp_order.empty()) ? expansionTerms : 
	  Pecos::PolynomialApproximation::total_order_terms(exp_order);
	int colloc_pts = probDescDB.get_int("method.nond.collocation_points");
	if (colloc_pts > 0) {
	  numSamplesOnModel = colloc_pts;
	  // define collocRatio for use in uniform refinement
	  collocRatio = terms_samples_to_ratio(exp_terms, colloc_pts);
	}
	else if (collocRatio > 0.)
	  numSamplesOnModel = terms_ratio_to_samples(exp_terms, collocRatio);
	if (tensorRegression) {// "probabilistic collocation": subset of TPQ pts
	  // since NonDExpansion invokes uSpaceModel.build_approximation() which
	  // in turn invokes daceIterator.run_iterator(), we need to avoid
	  // execution of the full tensor grid in real fn evals by overloading
	  // the NonDQuad ctor to allow internal point set filtering.
	  RealVector dim_pref; // empty (not part of regression spec)
	  construct_quadrature(u_space_sampler, g_u_model, numSamplesOnModel,
			       dim_pref, piecewise_basis, use_derivs);
	}
	else                  // "point collocation": LHS sampling
	  // reuse seed/rng settings intended for the expansion_sampler
	  construct_lhs(u_space_sampler, g_u_model, numSamplesOnModel,
	    probDescDB.get_int("method.random_seed"),
	    probDescDB.get_string("method.random_number_generator"));
	// TO DO:
	//if (probDescDB.get_string("method.nond.expansion_sample_type")
	//    == "incremental_lhs"))
	//  construct_incremental_lhs();
      }
    }

    // iteratedModel concurrency is defined by the number of samples
    // used in constructing the PC expansion
    if (numSamplesOnModel) // optional with default = 0
      maxConcurrency *= numSamplesOnModel;
  }

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for PCBDO with polynomials over {u}+{d}, change view to All.
  short  corr_order = -1, corr_type = NO_CORRECTION;
  String pt_reuse, approx_type = (piecewise_basis) ?
    "piecewise_orthogonal_polynomial" : "global_orthogonal_polynomial";
  if (expansionCoeffsApproach == Pecos::REGRESSION && !tensorRegression) {
    pt_reuse = probDescDB.get_string("method.nond.collocation_point_reuse");
    // if reusing samples within a refinement strategy ensure different random
    // numbers are generated for points w/i the grid (even if #samples differs)
    if (refineType && !pt_reuse.empty())
      ((Analyzer*)u_space_sampler.iterator_rep())->vary_pattern(true);
  }
  //const Variables& g_u_vars = g_u_model.current_variables();
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
    //g_u_vars.view(), g_u_vars.variables_components(),
    //g_u_model.current_response().active_set(),
    approx_type, exp_order, corr_type, corr_order, data_order, pt_reuse),
    false);
  initialize_u_space_model();

  // -------------------------------------
  // Construct expansionSampler, if needed
  // -------------------------------------
  construct_expansion_sampler();

  // uSpaceModel concurrency is defined by the number of samples used
  // in evaluating the PC expansion
  uSpaceModel.init_communicators(
    numSamplesOnExpansion*uSpaceModel.derivative_concurrency());
}


/** This constructor is used for helper iterator instantiation on the fly. */
NonDPolynomialChaos::
NonDPolynomialChaos(Model& model, short exp_coeffs_approach,
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
    RealVector  dim_pref;                     // empty -> isotropic
    UShortArray quad_order(1, num_int_level); // single sequence
    construct_quadrature(u_space_sampler, g_u_model, quad_order, dim_pref,
			 piecewise_basis, use_derivs);
  }
  else if (expansionCoeffsApproach == Pecos::SPARSE_GRID) {
    RealVector  dim_pref;                    // empty -> isotropic
    UShortArray ssg_level(1, num_int_level); // single sequence
    construct_sparse_grid(u_space_sampler, g_u_model, ssg_level, dim_pref,
			  piecewise_basis, use_derivs);
  }
  else if (expansionCoeffsApproach == Pecos::CUBATURE)
    construct_cubature(u_space_sampler, g_u_model, num_int_level);

  // iteratedModel concurrency is defined by the number of samples
  // used in constructing the PC expansion
  if (numSamplesOnModel) // optional with default = 0
    maxConcurrency *= numSamplesOnModel;

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for PCBDO with polynomials over {u}+{d}, change view to All.
  short  corr_order = -1, corr_type = NO_CORRECTION;
  String pt_reuse, approx_type = (piecewise_basis) ?
    "piecewise_orthogonal_polynomial" : "global_orthogonal_polynomial";
  UShortArray exp_order; // empty for numerical integration approaches
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
    approx_type, exp_order, corr_type, corr_order, data_order, pt_reuse),
    false);
  initialize_u_space_model();

  // no expansionSampler, no numSamplesOnExpansion
}


NonDPolynomialChaos::~NonDPolynomialChaos()
{
  if (numSamplesOnExpansion)
    uSpaceModel.free_communicators(
      numSamplesOnExpansion*uSpaceModel.derivative_concurrency());
}


void NonDPolynomialChaos::initialize_expansion()
{
  NonDExpansion::initialize_expansion();

  // The expansion_order specification is passed through the DataFitSurrModel
  // ctor.  Any additional specification data needed by OrthogPolyApproximation
  // must be passed through by diving through the hierarchy.
  bool num_int = (expansionCoeffsApproach == Pecos::QUADRATURE ||
		  expansionCoeffsApproach == Pecos::CUBATURE ||
		  expansionCoeffsApproach == Pecos::SPARSE_GRID);
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  const Pecos::ShortArray& u_types = natafTransform.u_types();

  PecosApproximation *poly_approx_rep, *poly_approx_rep0;
  NonDIntegration *u_space_sampler_rep = NULL;
  Pecos::IntegrationDriver *driver_rep = NULL;
  if (num_int) {
    u_space_sampler_rep
      = (NonDIntegration*)uSpaceModel.subordinate_iterator().iterator_rep();
    driver_rep = u_space_sampler_rep->driver().driver_rep();
  }

  // reuse polynomial bases since they are the same for all response fns
  // (efficiency becomes more important for numerically-generated polynomials).
  bool first = true;
  for (size_t i=0; i<numFunctions; i++) {
    poly_approx_rep = (PecosApproximation*)poly_approxs[i].approx_rep();
    if (poly_approx_rep) { // may be NULL based on approxFnIndices
      if (first) {
	if (num_int) { // reuse driver basis for rep0
	  poly_approx_rep->distribution_types(u_types);
	  poly_approx_rep->polynomial_basis(driver_rep->polynomial_basis());
	}
	else           // construct a new basis for rep0
	  poly_approx_rep->
	    distributions(u_types, iteratedModel.distribution_parameters());
	poly_approx_rep0 = poly_approx_rep;
	first = false;
      }
      else {           // reuse rep0 basis for all other reps
        poly_approx_rep->basis_types(poly_approx_rep0->basis_types());
	poly_approx_rep->polynomial_basis(poly_approx_rep0->polynomial_basis());
      }
      // NumerGenOrthogPolynomial instances need to compute polyCoeffs and
      // orthogPolyNormsSq in addition to gaussPoints and gaussWeights
      poly_approx_rep->coefficients_norms_flag(true);
      // transfer an expansionTerms spec to the OrthogPolyApproximation
      if (expansionTerms)
	poly_approx_rep->expansion_terms(expansionTerms);
    }
  }
}


void NonDPolynomialChaos::compute_expansion()
{
  if (expansionImportFile.empty())
    // ------------------------------
    // Calculate the PCE coefficients
    // ------------------------------
    NonDExpansion::compute_expansion(); // default implementation
  else {
    // ---------------------------
    // Import the PCE coefficients
    // ---------------------------
    std::ifstream import_stream(expansionImportFile);
    if (!import_stream) {
      Cerr << "\nError: cannot open polynomial chaos expansion import file "
	   << expansionImportFile << std::endl;
      abort_handler(-1);
    }
    if (subIteratorFlag || !finalStatistics.function_gradients().empty()) {
      Cerr << "\nError: PCE coefficient import not supported in advanced modes"
	   << std::endl;
      abort_handler(-1);
    }
    std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
    RealVectorArray chaos_coeffs(numFunctions);
    PecosApproximation* poly_approx_rep;
    for (size_t i=0; i<numFunctions; i++) {
      poly_approx_rep = (PecosApproximation*)poly_approxs[i].approx_rep();
      if (poly_approx_rep) { // may be NULL based on approxFnIndices
	poly_approx_rep->allocate_arrays();
	chaos_coeffs[i].sizeUninitialized(poly_approx_rep->expansion_terms());
      }
    }
    read_data(import_stream, chaos_coeffs);
    uSpaceModel.approximation_coefficients(chaos_coeffs);
  }
}


/** Used for uniform refinement of regression-based PCE. */
void NonDPolynomialChaos::increment_expansion()
{
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  PecosApproximation* poly_approx_rep; size_t exp_terms = 0;
  for (size_t i=0; i<numFunctions; i++) {
    poly_approx_rep = (PecosApproximation*)poly_approxs[i].approx_rep();
    if (poly_approx_rep) { // may be NULL based on approxFnIndices
      poly_approx_rep->increment_order();
      exp_terms = std::max(poly_approx_rep->expansion_terms(), exp_terms);
    }
  }

  // update numSamplesOnModel based on existing collocatio ratio and
  // updated number of expansion terms
  numSamplesOnModel = terms_ratio_to_samples(exp_terms, collocRatio);

  // update u-space sampler to use new sample count
  if (tensorRegression) {
    NonDQuadrature* nond_quad
      = (NonDQuadrature*)uSpaceModel.subordinate_iterator().iterator_rep();
    nond_quad->filtered_samples(numSamplesOnModel);
    nond_quad->compute_minimum_quadrature_order();
  }
  else {
    NonDSampling* nond_sampling
      = (NonDSampling*)uSpaceModel.subordinate_iterator().iterator_rep();
    nond_sampling->sampling_reference(0); // no lower bound
    DataFitSurrModel* dfs_model = (DataFitSurrModel*)uSpaceModel.model_rep();
    dfs_model->total_points(numSamplesOnModel);
  }
}


void NonDPolynomialChaos::print_coefficients(std::ostream& s)
{
  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision);

  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  const StringArray& fn_labels = iteratedModel.response_labels();

  size_t i, j, width = write_precision+7,
    num_cuv = numContAleatUncVars+numContEpistUncVars;
  char tag[10];

  s << "-------------------------------------------------------------------\n";
  for (i=0; i<numFunctions; i++) {
    s << "Polynomial Chaos coefficients for " << fn_labels[i] << ":\n";
    // header
    s << "  " << std::setw(width) << "coefficient";
    for (j=0; j<numContDesVars; j++)
      { std::sprintf(tag, "d%i", j+1); s << std::setw(5) << tag; }
    for (j=0; j<num_cuv; j++)
      { std::sprintf(tag, "u%i", j+1); s << std::setw(5) << tag; }
    for (j=0; j<numContStateVars; j++)
      { std::sprintf(tag, "s%i", j+1); s << std::setw(5) << tag; }
    s << "\n  " << std::setw(width) << "-----------";
    for (j=0; j<numContinuousVars; j++)
      s << " ----";
    poly_approxs[i].print_coefficients(s);
  }
}


void NonDPolynomialChaos::print_moments(std::ostream& s)
{
  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision);

  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  const StringArray& fn_labels = iteratedModel.response_labels();

  size_t i, j, width = write_precision+7;

  s << "\nMoment-based statistics for each response function:\n"
    << std::setw(width+15) << "Mean"     << std::setw(width+1) << "Std Dev"
    << std::setw(width+1)  << "Skewness" << std::setw(width+2) << "Kurtosis\n";
  //<< std::setw(width+2)  << "Coeff of Var\n";
  PecosApproximation* poly_approx_rep; size_t exp_mom, num_mom;
  for (i=0; i<numFunctions; ++i) {
    poly_approx_rep = (PecosApproximation*)poly_approxs[i].approx_rep();
    if (poly_approx_rep && poly_approx_rep->expansion_coefficient_flag()) {
      const RealVector& exp_moments = poly_approx_rep->expansion_moments();
      const RealVector& num_moments = poly_approx_rep->numerical_moments();
      exp_mom = exp_moments.length(); num_mom = num_moments.length();

      if (num_mom) // TPQ, SSG, or Cubature: report both moment sets
	s << fn_labels[i] << '\n' << std::setw(14) << "expansion:  ";
      else // regression, sampling: only expansion moments available
	s << std::setw(14) << fn_labels[i];
      for (j=0; j<exp_mom; ++j)
	if (j==1) s << ' ' << std::setw(width) << std::sqrt(exp_moments[j]);
	else      s << ' ' << std::setw(width) << exp_moments[j];
      if (num_mom) {
	s << '\n' << std::setw(14) << "numerical:  ";
	for (j=0; j<num_mom; ++j)
	  if (j==1) s << ' ' << std::setw(width) << std::sqrt(num_moments[j]);
	  else      s << ' ' << std::setw(width) << num_moments[j];
      }
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
