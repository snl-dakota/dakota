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
		      const UShortArray& num_int_seq,
		      const RealVector& dim_pref, short u_space_type,
		      bool piecewise_basis, bool use_derivs);
  /// alternate constructor for regression (least squares, CS, OLI)
  NonDPolynomialChaos(Model& model, short exp_coeffs_approach,
		      const UShortArray& exp_order_seq,
		      const RealVector& dim_pref,
		      const SizetArray& colloc_pts_seq, Real colloc_ratio,
		      int seed, short u_space_type, bool piecewise_basis,
		      bool use_derivs, bool cv_flag,
		      const String& import_build_points_file,
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
  //- Heading: Virtual function redefinitions
  //

  void derived_init_communicators(ParLevLIter pl_iter);
  void derived_set_communicators(ParLevLIter pl_iter);
  void derived_free_communicators(ParLevLIter pl_iter);

  void resolve_inputs(short& u_space_type, short& data_order);

  void initialize_u_space_model();

  void increment_specification_sequence();

  /// form or import an orthogonal polynomial expansion using PCE methods
  void compute_expansion();

  void multifidelity_expansion();

  void select_refinement_points(const RealVectorArray& candidate_samples,
				unsigned short batch_size,
				RealMatrix& best_samples);

  void select_refinement_points_deprecated(
    const RealVectorArray& candidate_samples, unsigned short batch_size,
    RealMatrix& best_samples);

  void append_expansion(const RealMatrix& samples,
			const IntResponseMap& resp_map);

  void increment_order_and_grid();

  /// print the final coefficients and final statistics
  void print_results(std::ostream& s);
  /// print the PCE coefficient array for the orthogonal basis
  void print_coefficients(std::ostream& s);

  /// archive the PCE coefficient array for the orthogonal basis
  void archive_coefficients();

  //
  //- Heading: Member functions
  //

  /// special case of multifidelity_expansion() for multilevel allocation of
  /// samples, mirroring NonDMultilevelSampling for least sq/compressed sensing
  void multilevel_regression(size_t model_form);
  /// increment the sequence in numSamplesOnModel for multilevel_regression()
  void increment_sample_sequence(size_t new_samp, size_t total_samp);
  /// generate new samples from numSamplesOnModel and update expansion
  void append_expansion();

private:

  /// define a grid increment that is consistent with an advancement in
  /// expansion order
  void increment_grid_from_order();
  /// define an expansion order that is consistent with an advancement in
  /// structured/unstructured grid level/density
  void increment_order_from_grid();

  /// convert number of expansion terms and collocation ratio to a
  /// number of collocation samples
  int  terms_ratio_to_samples(size_t num_exp_terms, Real colloc_ratio);
  /// convert number of expansion terms and number of collocation samples
  /// to a collocation ratio
  Real terms_samples_to_ratio(size_t num_exp_terms, int samples);
  /// convert collocation ratio and number of samples to expansion order
  void ratio_samples_to_order(Real colloc_ratio, int num_samples,
			      UShortArray& exp_order, bool less_than_or_equal);
  /// convert an isotropic/anisotropic expansion_order vector into a scalar
  /// plus a dimension preference vector
  void order_to_dim_preference(const UShortArray& order, unsigned short& p,
			       RealVector& dim_pref);

  //
  //- Heading: Data
  //

  /// filename for export of chaos coefficients
  String expansionExportFile;
  /// filename for import of chaos coefficients
  String expansionImportFile;

  /// factor applied to terms^termsOrder in computing number of regression
  /// points, either user specified or inferred
  Real collocRatio;
  /// exponent applied to number of expansion terms for computing
  /// number of regression points
  Real termsOrder;

  /// seed for random number generator used for regression with LHS
  /// and sub-sampled tensor grids
  int randomSeed;

  /// option for regression PCE using a filtered set tensor-product points
  bool tensorRegression;

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
  UShortArray expOrderSeqSpec;
  /// user specification for dimension_preference
  RealVector dimPrefSpec;
  /// user specification for collocation_points (array for multifidelity)
  SizetArray collocPtsSeqSpec;
  /// user specification for expansion_samples (array for multifidelity)
  SizetArray expSamplesSeqSpec;
  /// sequence index for {expOrder,collocPts,expSamples}SeqSpec
  size_t sequenceIndex;

  /// derivative of the PCE with respect to the x-space variables
  /// evaluated at the means (used as uncertainty importance metrics)
  RealMatrix pceGradsMeanX;

  /// user request for use of normalization when outputting PCE coefficients
  bool normalizedCoeffOutput;

  /// user requested expansion type
  short uSpaceType;

  /// user request of quadrature order
  UShortArray quadOrderSeqSpec;
  /// user request of sparse grid level
  UShortArray ssgLevelSeqSpec;
  /// cubature integrand
  unsigned short cubIntSpec;

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

  // local flag to signal a resizing occurred
  //bool resizedFlag;
  // local flag to signal an explicit call to resize() is necessary
  //bool callResize;

  /// number of samples allocated to each level of a discretization
  /// hierarchy within multilevel regression
  SizetArray NLev;
  /// equivalent number of high fidelity evaluations accumulated using samples
  /// across multiple model forms and/or discretization levels
  Real equivHFEvals;
};


inline void NonDPolynomialChaos::
append_expansion(const RealMatrix& samples, const IntResponseMap& resp_map)
{
  // adapt the expansion in sync with the dataset
  numSamplesOnModel += resp_map.size();
  if (expansionCoeffsApproach != Pecos::ORTHOG_LEAST_INTERPOLATION)
    increment_order_from_grid();

  // utilize rebuild following expansion updates
  uSpaceModel.append_approximation(samples, resp_map, true);
}


inline void NonDPolynomialChaos::append_expansion()
{
  // Reqmts: numSamplesOnModel updated and propagated to uSpaceModel
  //         increment_order_from_grid() called

  // Run uSpaceModel::daceIterator, append data sets, and rebuild expansion
  uSpaceModel.subordinate_iterator().sampling_reset(numSamplesOnModel,
						    true, false);
  uSpaceModel.run_dace_iterator(true); // appends and rebuilds
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
