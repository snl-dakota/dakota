/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDPolynomialChaos
//- Description: Iterator to compute/employ Polynomial Chaos expansions
//- Owner:       Mike Eldred, Sandia National Laboratories

#ifndef NOND_POLYNOMIAL_CHAOS_H
#define NOND_POLYNOMIAL_CHAOS_H

#include "NonDExpansion.hpp"
#include "BasisPolynomial.hpp"

namespace Dakota {

/// Nonintrusive polynomial chaos expansion approaches to uncertainty
/// quantification

/** The NonDPolynomialChaos class uses a polynomial chaos expansion
    (PCE) approach to approximate the effect of parameter
    uncertainties on response functions of interest.  It utilizes the
    OrthogPolyApproximation class to manage multiple types of
    orthogonal polynomials within a Wiener-Askey scheme to PCE.  It
    supports PCE coefficient estimation via sampling, quadrature,
    point-collocation, and file import. */

class NonDPolynomialChaos: public NonDExpansion
{
public:

  //
  //- Heading: Constructors and destructor
  //
 
  /// standard constructor
  NonDPolynomialChaos(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor for numerical integration (tensor, sparse, cubature)
  NonDPolynomialChaos(Model& model, short exp_coeffs_approach,
		      unsigned short num_int, const RealVector& dim_pref,
		      short u_space_type, bool piecewise_basis,
		      bool use_derivs);
  /// alternate constructor for regression (least squares, CS, OLI)
  NonDPolynomialChaos(Model& model, short exp_coeffs_approach,
		      unsigned short exp_order, const RealVector& dim_pref,
		      size_t colloc_pts, Real colloc_ratio, int seed,
		      short u_space_type, bool piecewise_basis, bool use_derivs,
		      bool cv_flag, const String& import_build_pts_file,
		      unsigned short import_build_format,
		      bool import_build_active_only);

  /// destructor
  ~NonDPolynomialChaos();

  //
  //- Heading: Virtual function redefinitions
  //

  bool resize();

protected:

  //
  //- Heading: Constructors
  //
 
  /// base constructor for DB construction of multilevel/multifidelity PCE
  NonDPolynomialChaos(BaseConstructor, ProblemDescDB& problem_db, Model& model);
  /// base constructor for lightweight construction of 
  /// multilevel/multifidelity PCE using numerical integration
  NonDPolynomialChaos(unsigned short method_name, Model& model,
		      short exp_coeffs_approach, const RealVector& dim_pref,
		      short u_space_type, bool piecewise_basis,
		      bool use_derivs);
  /// base constructor for lightweight construction of 
  /// multilevel/multifidelity PCE using regression
  NonDPolynomialChaos(unsigned short method_name, Model& model,
		      short exp_coeffs_approach, const RealVector& dim_pref,
		      short u_space_type, bool piecewise_basis, bool use_derivs,
		      Real colloc_ratio, int seed, bool cv_flag);

  //
  //- Heading: Virtual function redefinitions
  //

  void derived_init_communicators(ParLevLIter pl_iter);
  void derived_set_communicators(ParLevLIter pl_iter);
  void derived_free_communicators(ParLevLIter pl_iter);

  void resolve_inputs(short& u_space_type, short& data_order);

  void initialize_u_space_model();

  /// form or import an orthogonal polynomial expansion using PCE methods
  void compute_expansion(size_t index = _NPOS);

  void select_refinement_points(const RealVectorArray& candidate_samples,
				unsigned short batch_size,
				RealMatrix& best_samples);

  void select_refinement_points_deprecated(
    const RealVectorArray& candidate_samples, unsigned short batch_size,
    RealMatrix& best_samples);

  void append_expansion(const RealMatrix& samples,
			const IntResponseMap& resp_map, size_t index = _NPOS);

  void increment_order_and_grid();

  /// print the final coefficients and final statistics
  void print_results(std::ostream& s, short results_state = FINAL_RESULTS);
  /// print the PCE coefficient array for the orthogonal basis
  void print_coefficients(std::ostream& s);
  /// export the PCE coefficient array to expansionExportFile
  void export_coefficients();

  /// archive the PCE coefficient array for the orthogonal basis
  void archive_coefficients();

  //
  //- Heading: Member functions
  //

  /// generate new samples from numSamplesOnModel and update expansion
  void append_expansion(size_t index = _NPOS);

  /// configure exp_orders from inputs
  void config_expansion_orders(unsigned short exp_order,
			       const RealVector& dim_pref,
			       UShortArray& exp_orders);
  /// configure u_space_sampler and approx_type based on numerical
  /// integration specification
  bool config_integration(unsigned short quad_order, unsigned short ssg_level,
			  unsigned short cub_int, Iterator& u_space_sampler,
			  Model& g_u_model, String& approx_type);
  /// configure u_space_sampler and approx_type based on expansion_samples
  /// specification
  bool config_expectation(size_t exp_samples, unsigned short sample_type,
			  const String& rng, Iterator& u_space_sampler,
			  Model& g_u_model,  String& approx_type);
  /// configure u_space_sampler and approx_type based on regression
  /// specification
  bool config_regression(const UShortArray& exp_orders, size_t colloc_pts,
			 Real colloc_ratio_order, short regress_type,
			 short ls_regress_type,
			 const UShortArray& tensor_grid_order,
			 unsigned short sample_type, const String& rng,
			 const String& pt_reuse, Iterator& u_space_sampler,
			 Model& g_u_model, String& approx_type);

  /// convert number of expansion terms and collocation ratio to a
  /// number of collocation samples
  int  terms_ratio_to_samples(size_t num_exp_terms, Real colloc_ratio);
  /// convert number of expansion terms and number of collocation samples
  /// to a collocation ratio
  Real terms_samples_to_ratio(size_t num_exp_terms, int samples);
  /// convert collocation ratio and number of samples to expansion order
  void ratio_samples_to_order(Real colloc_ratio, int num_samples,
			      UShortArray& exp_order, bool less_than_or_equal);

  //
  //- Heading: Data
  //

  /// user requested expansion type
  short uSpaceType;

  /// cubature integrand
  unsigned short cubIntSpec;

  /// user specification for dimension_preference
  RealVector dimPrefSpec;

  /// factor applied to terms^termsOrder in computing number of regression
  /// points, either user specified or inferred
  Real collocRatio;
  /// option for regression PCE using a filtered set tensor-product points
  bool tensorRegression;

  /// user specified import build points file
  String importBuildPointsFile;
  /// user specified import build file format
  unsigned short importBuildFormat;
  /// user specified import build active only
  bool importBuildActiveOnly;

  /// user specified import approx. points file
  String importApproxPointsFile;
  /// user specified import approx. file format
  unsigned short importApproxFormat;
  /// user specified import approx. active only
  bool importApproxActiveOnly;

  /// filename for import of chaos coefficients
  String expansionImportFile;
  /// filename for export of chaos coefficients
  String expansionExportFile;

private:

  /// define a grid increment that is consistent with an advancement in
  /// expansion order
  void increment_grid_from_order();
  /// define an expansion order that is consistent with an advancement in
  /// structured/unstructured grid level/density
  void increment_order_from_grid();

  /// convert an isotropic/anisotropic expansion_order vector into a scalar
  /// plus a dimension preference vector
  void order_to_dim_preference(const UShortArray& order, unsigned short& p,
			       RealVector& dim_pref);

  //
  //- Heading: Data
  //

  /// exponent applied to number of expansion terms for computing
  /// number of regression points
  Real termsOrder;

  /// seed for random number generator used for regression with LHS
  /// and sub-sampled tensor grids
  int randomSeed;

  /// flag for use of cross-validation for selection of parameter settings
  /// in regression approaches
  bool crossValidation;
  /// flag to restrict cross-validation to only estimate the noise
  /// tolerance in order to manage computational cost
  bool crossValidNoiseOnly;

  /// noise tolerance for compressive sensing algorithms; vector form used
  /// in cross-validation
  RealVector noiseTols;
  /// L2 penalty for LASSO algorithm (elastic net variant)
  Real l2Penalty;

  // initial grid level for adapted expansions with the
  // ADAPTED_BASIS_GENERALIZED approach
  //unsigned short initSGLevel;
  /// number of frontier expansions per iteration with the
  /// ADAPTED_BASIS_EXPANDING_FRONT approach
  unsigned short numAdvance;

  /// user specification for expansion_order (array for multifidelity)
  unsigned short expOrderSpec;
  /// user specification for collocation_points (array for multifidelity)
  size_t collocPtsSpec;
  /// user specification for expansion_samples (array for multifidelity)
  size_t expSamplesSpec;
  /// user request of quadrature order
  unsigned short quadOrderSpec;
  /// user request of sparse grid level
  unsigned short ssgLevelSpec;

  /// derivative of the PCE with respect to the x-space variables
  /// evaluated at the means (used as uncertainty importance metrics)
  RealMatrix pceGradsMeanX;

  /// user request for use of normalization when outputting PCE coefficients
  bool normalizedCoeffOutput;

  // local flag to signal a resizing occurred
  //bool resizedFlag;
  // local flag to signal an explicit call to resize() is necessary
  //bool callResize;
};


inline void NonDPolynomialChaos::
append_expansion(const RealMatrix& samples, const IntResponseMap& resp_map,
		 size_t index)
{
  // adapt the expansion in sync with the dataset
  numSamplesOnModel += resp_map.size();
  if (expansionCoeffsApproach != Pecos::ORTHOG_LEAST_INTERPOLATION)
    increment_order_from_grid();

  // utilize rebuild following expansion updates
  uSpaceModel.append_approximation(samples, resp_map, true, index);
}


inline void NonDPolynomialChaos::append_expansion(size_t index)
{
  // Reqmts: numSamplesOnModel updated and propagated to uSpaceModel
  //         increment_order_from_grid() called

  // Run uSpaceModel::daceIterator, append data sets, and rebuild expansion
  uSpaceModel.subordinate_iterator().sampling_reset(numSamplesOnModel,
						    true, false);
  uSpaceModel.run_dace_iterator(true, index); // appends and rebuilds
}


inline int NonDPolynomialChaos::
terms_ratio_to_samples(size_t num_exp_terms, Real colloc_ratio)
{
  // for under-determined solves (compressed sensing), colloc_ratio can be < 1
  size_t data_per_pt = (useDerivs) ? numContinuousVars + 1 : 1;
  Real min_pts = std::pow((Real)num_exp_terms, termsOrder) / (Real)data_per_pt;
  int tgt_samples = (int)std::floor(colloc_ratio*min_pts + .5); // rounded
  if (colloc_ratio >= 1.) {
    // logic is to round to the nearest integral sample count for the given
    // colloc_ratio, but with a lower bound determined by rounding up with a
    // unit colloc_ratio.  The lower bound prevents creating an under-determined
    // system due to rounding down when the intent is over- or uniquely
    // determined (can only happen with non-integral min_pts due to use of
    // derivative enhancement).
    int min_samples = (int)std::ceil(min_pts); // lower bound
    return std::max(min_samples, tgt_samples);
  }
  else
    // for under-determined systems, data starvation is not a problem and we
    // just need at least one sample.
    return std::max(tgt_samples, 1);
}


inline Real NonDPolynomialChaos::
terms_samples_to_ratio(size_t num_exp_terms, int samples)
{
  size_t data_per_pt = (useDerivs) ? numContinuousVars + 1 : 1;
  return (Real)(samples * data_per_pt) /
    std::pow((Real)num_exp_terms, termsOrder);
}

} // namespace Dakota

#endif
