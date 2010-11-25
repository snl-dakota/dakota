/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
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
#include "NonDIntegration.H"
#include "PecosApproximation.H"
#include "OrthogPolyApproximation.hpp"
#include "SparseGridDriver.hpp"
#include "TensorProductDriver.hpp"
#include "CubatureDriver.hpp"
#include "data_io.h"


namespace Dakota {

NonDPolynomialChaos::NonDPolynomialChaos(Model& model): NonDExpansion(model),
  expansionImportFile(
    probDescDB.get_string("method.nond.expansion_import_file")),
  expansionTerms(probDescDB.get_int("method.nond.expansion_terms"))
{
  // This constructor is called for a standard letter-envelope iterator 
  // instantiation.

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  construct_g_u_model(g_u_model);

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
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
    unsigned short ssg_level_spec
      = probDescDB.get_ushort("method.nond.sparse_grid_level");
    unsigned short cub_int_spec
      = probDescDB.get_ushort("method.nond.cubature_integrand");
    if (!quad_order_spec.empty())
      construct_quadrature(u_space_sampler, g_u_model, quad_order_spec);
    else if (ssg_level_spec != USHRT_MAX)
      construct_sparse_grid(u_space_sampler, g_u_model, ssg_level_spec,
	probDescDB.get_rdv("method.nond.sparse_grid_dimension_preference"));
    else if (cub_int_spec != USHRT_MAX)
      construct_cubature(u_space_sampler, g_u_model, cub_int_spec);
    else { // expansion_samples or collocation_points
      int exp_samples = probDescDB.get_int("method.nond.expansion_samples"),
	  colloc_pts  = probDescDB.get_int("method.nond.collocation_points");
      const Real& colloc_ratio
	= probDescDB.get_real("method.nond.collocation_ratio");
      if (exp_samples > 0) {
	numSamplesOnModel       = exp_samples;
	expansionCoeffsApproach = Pecos::SAMPLING;
      }
      else if (colloc_pts > 0) {
	numSamplesOnModel       = colloc_pts;
	expansionCoeffsApproach = Pecos::REGRESSION;
      }
      else if (colloc_ratio > 0.) {
	int num_exp_terms = (exp_order.empty()) ? expansionTerms : 
	  Pecos::PolynomialApproximation::total_order_terms(exp_order);
	int data_per_pt
	  = (useDerivsFlag && g_u_model.gradient_type() != "none")
	  ? numContinuousVars + 1 : 1;
	Real min_pts = (Real)num_exp_terms/(Real)data_per_pt;
	int min_samples = (int)std::ceil(min_pts),
	  tgt_samples = (int)std::floor(colloc_ratio*min_pts + .5);
	numSamplesOnModel = std::max(min_samples, tgt_samples);
	expansionCoeffsApproach = Pecos::REGRESSION;
      }
      construct_lhs(u_space_sampler, g_u_model);
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
  String approx_type = "global_orthogonal_polynomial", sample_reuse, corr_type;
  if (expansionCoeffsApproach == Pecos::REGRESSION)
    sample_reuse = probDescDB.get_string("method.nond.collocation_point_reuse");
  short corr_order = -1;
  //const Variables& g_u_vars = g_u_model.current_variables();
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
    //g_u_vars.view(), g_u_vars.variables_components(),
    //g_u_model.current_response().active_set(),
    approx_type, exp_order, corr_type, corr_order, sample_reuse), false);
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


NonDPolynomialChaos::~NonDPolynomialChaos()
{
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
  std::vector<Approximation>& orthog_poly_approxs
    = uSpaceModel.approximations();
  const Pecos::ShortArray& u_types = natafTransform.u_types();

  PecosApproximation* pa_rep;
  Pecos::OrthogPolyApproximation *popa_rep, *popa_rep0;
  NonDIntegration* u_space_sampler_rep = NULL;
  Pecos::IntegrationDriver* driver_rep = NULL;
  if (num_int) {
    u_space_sampler_rep
      = (NonDIntegration*)uSpaceModel.subordinate_iterator().iterator_rep();
    driver_rep = u_space_sampler_rep->driver().driver_rep();
  }
  Pecos::IntArray empty_array;

  for (size_t i=0; i<numFunctions; i++) {
    // TO DO: consider delegating to pa_rep/u_space_sampler_rep levels
    pa_rep   = (PecosApproximation*)orthog_poly_approxs[i].approx_rep();
    popa_rep = (Pecos::OrthogPolyApproximation*)pa_rep->
      pecos_basis_approximation().approx_rep();
    if (popa_rep) { // may be NULL based on approxFnIndices
      if (num_int) { // reuse driver basis
	switch (expansionCoeffsApproach) {
	case Pecos::QUADRATURE:
	  popa_rep->distribution_types(u_types,
	    ((Pecos::TensorProductDriver*)driver_rep)->integration_rules());
	  break;
	case Pecos::SPARSE_GRID:
	  popa_rep->distribution_types(u_types,
	    ((Pecos::SparseGridDriver*)driver_rep)->integration_rules());
	  break;
	case Pecos::CUBATURE: { // TO DO: clean up this temporary hack
	  int rule = ((Pecos::CubatureDriver*)driver_rep)->integration_rule();
	  IntArray rules(numContinuousVars, rule);
	  popa_rep->distribution_types(u_types, rules);
	  break;
	}
	}
	// reuse polynomial bases since they are the same for all response fns
	// (efficiency becomes more important for numerically-generated polys).
        popa_rep->polynomial_basis(driver_rep->polynomial_basis());
      }
      else if (i == 0) { // construct a new basis for rep0
	popa_rep->distributions(u_types, empty_array,
				iteratedModel.distribution_parameters());
	popa_rep0 = popa_rep;
      }
      else {             // reuse rep0 basis
        popa_rep->distribution_types(u_types, empty_array);
	popa_rep->polynomial_basis(popa_rep0->polynomial_basis());
      }
      // NumerGenOrthogPolynomial instances need to compute polyCoeffs and
      // orthogPolyNormsSq in addition to gaussPoints and gaussWeights
      popa_rep->coefficients_norms_flag(true);
      // transfer an expansionTerms spec to the OrthogPolyApproximation
      if (expansionTerms)
	popa_rep->expansion_terms(expansionTerms);
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
    RealVectorArray chaos_coeffs_array(numFunctions);
    PecosApproximation* pa_rep; Pecos::OrthogPolyApproximation* popa_rep;
    for (size_t i=0; i<numFunctions; i++) {
      pa_rep   = (PecosApproximation*)poly_approxs[i].approx_rep();
      popa_rep = (Pecos::OrthogPolyApproximation*)pa_rep->
	pecos_basis_approximation().approx_rep();
      popa_rep->allocate_arrays();
      chaos_coeffs_array[i].sizeUninitialized(popa_rep->expansion_terms());
    }
    read_data(import_stream, chaos_coeffs_array);
    uSpaceModel.approximation_coefficients(chaos_coeffs_array);
  }
}


void NonDPolynomialChaos::print_results(std::ostream& s)
{
  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision);

  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  const StringArray& fn_labels = iteratedModel.response_labels();

  size_t i, j,  width = write_precision+7,
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

  s << "-------------------------------------------------------------------"
    << "\nStatistics derived analytically from polynomial expansion:\n";

  s << "\nCentral moments for each response function:\n"
    << std::setw(width+16)    << "Mean" << std::setw(width+2) << "Variance"
    << std::setw(width+2) << "Skewness" << std::setw(width+3) << "Kurtosis\n";
  PecosApproximation* pa_rep; size_t exp_mom, num_mom;
  for (i=0; i<numFunctions; ++i) {
    pa_rep = (PecosApproximation*)poly_approxs[i].approx_rep();
    const RealVector& exp_moments = pa_rep->central_expansion_moments();
    const RealVector& num_moments = pa_rep->central_numerical_moments();
    exp_mom = exp_moments.length(); num_mom = num_moments.length();

    if (num_mom) { // TPQ, SSG, or Cubature: report both moment sets
      s << fn_labels[i] << '\n' << std::setw(14) << "expansion:  ";
      for (j=0; j<exp_mom; ++j)
	s << "  " << std::setw(width) << exp_moments[j];
      s << '\n' << std::setw(14) << "numerical:  ";
      for (j=0; j<num_mom; ++j)
	s << "  " << std::setw(width) << num_moments[j];
      s << '\n';
    }
    else { // regression, sampling: only expansion moments available
      s << std::setw(14) << fn_labels[i];
      for (j=0; j<exp_mom; ++j)
	s << "  " << std::setw(width) << exp_moments[j];
      s << '\n';
    }

    //Real mean = exp_moments[0], std_dev = std::sqrt(exp_moments[1]);
    //s << "  Std. Dev. = " << std::setw(width-1) << std_dev
    //  << "  Coeff. of Variation = ";
    //if (std::abs(mean) > 1.e-25)
    //  s << std::setw(width) << std_dev/mean << '\n';
    //else
    //  s << "Undefined\n";
  }

  NonDExpansion::print_results(s);
}

} // namespace Dakota
