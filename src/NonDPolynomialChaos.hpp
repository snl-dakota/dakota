/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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
		      short u_space_type, short refine_type,
		      short refine_control, short covar_control,
		      short rule_nest, short rule_growth,
		      bool piecewise_basis, bool use_derivs,
                      String exp_expansion_file = "");
  /// alternate constructor for regression (least squares, CS, OLI)
  NonDPolynomialChaos(Model& model, short exp_coeffs_approach,
		      unsigned short exp_order, const RealVector& dim_pref,
		      size_t colloc_pts, Real colloc_ratio, int seed,
		      short u_space_type, short refine_type,
		      short refine_control, short covar_control,
		      //short rule_nest, short rule_growth,
		      bool piecewise_basis, bool use_derivs, bool cv_flag,
		      const String& import_build_pts_file,
		      unsigned short import_build_format,
		      bool import_build_active_only,
                      String exp_expansion_file = "");
  /// alternate constructor for coefficient import
  NonDPolynomialChaos(Model& model, const String& exp_import_file,
		      short u_space_type, const ShortShortPair& approx_view);

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
  /// (method_name is not necessary, rather it is just a convenient overload
  /// allowing the derived ML PCE class to bypass the standard PCE ctor)
  NonDPolynomialChaos(unsigned short method_name, ProblemDescDB& problem_db,
		      Model& model);
  /// base constructor for lightweight construction of multifidelity PCE
  /// using numerical integration
  NonDPolynomialChaos(unsigned short method_name, Model& model,
		      short exp_coeffs_approach, const RealVector& dim_pref,
		      short u_space_type, short refine_type,
		      short refine_control, short covar_control,
		      short ml_alloc_control, short ml_discrep, short rule_nest,
		      short rule_growth, bool piecewise_basis, bool use_derivs);
  /// base constructor for lightweight construction of multilevel PCE
  /// using regression
  NonDPolynomialChaos(unsigned short method_name, Model& model,
		      short exp_coeffs_approach, const RealVector& dim_pref,
		      short u_space_type, short refine_type,
		      short refine_control, short covar_control,
		      const SizetArray& colloc_pts_seq, Real colloc_ratio,
		      short ml_alloc_control, short ml_discrep,
		      //short rule_nest, short rule_growth,
		      bool piecewise_basis, bool use_derivs, bool cv_flag);

  //
  //- Heading: Virtual function redefinitions
  //

  void derived_init_communicators(ParLevLIter pl_iter);
  void derived_set_communicators(ParLevLIter pl_iter);
  void derived_free_communicators(ParLevLIter pl_iter);

  void resolve_inputs(short& u_space_type, short& data_order);

  void initialize_u_space_model();

  size_t collocation_points() const;

  //void initialize_expansion();
  void compute_expansion();

  void select_refinement_points(const RealVectorArray& candidate_samples,
				unsigned short batch_size,
				RealMatrix& best_samples);

  void select_refinement_points_deprecated(
    const RealVectorArray& candidate_samples, unsigned short batch_size,
    RealMatrix& best_samples);

  void append_expansion(const RealMatrix& samples,
			const IntResponseMap& resp_map);

  void update_samples_from_order_increment();
  void sample_allocation_metric(Real& sparsity_metric, Real power);

  /// Inherit to allow on-the-fly instances to customize behavior
  virtual void post_run(std::ostream& s) override;
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

  /// configure u_space_sampler and approx_type based on numerical
  /// integration specification
  bool config_integration(unsigned short quad_order, unsigned short ssg_level,
			  unsigned short cub_int, Iterator& u_space_sampler,
			  Model& g_u_model, String& approx_type);
  /// configure u_space_sampler and approx_type based on expansion_samples
  /// specification
  bool config_expectation(size_t exp_samples, unsigned short sample_type,
			  int seed, const String& rng,
			  Iterator& u_space_sampler, Model& g_u_model,
			  String& approx_type);
  /// configure u_space_sampler and approx_type based on regression
  /// specification
  bool config_regression(const UShortArray& exp_orders, size_t colloc_pts,
			 Real colloc_ratio_order, short regress_type,
			 short ls_regress_type,
			 const UShortArray& tensor_grid_order,
			 unsigned short sample_type, int seed,
			 const String& rng, const String& pt_reuse,
			 Iterator& u_space_sampler, Model& g_u_model,
			 String& approx_type);

  /// define an expansion order that is consistent with an advancement in
  /// structured/unstructured grid level/density
  void increment_order_from_grid();

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

  /// flag for use of cross-validation for selection of parameter settings
  /// in regression approaches
  bool crossValidation;
  /// flag to restrict cross-validation to only estimate the noise
  /// tolerance in order to manage computational cost
  bool crossValidNoiseOnly;
  /// maximum number of expansion order candidates for cross-validation
  /// in regression-based PCE
  unsigned short maxCVOrderCandidates;
  /// flag for scaling response data to [0,1] for alignment with regression tols
  bool respScaling;

  /// user-specified file for importing build points
  String importBuildPointsFile;

  /// filename for import of chaos coefficients
  String expansionImportFile;
  /// filename for export of chaos coefficients
  String expansionExportFile;

private:

  /// convert an isotropic/anisotropic expansion_order vector into a scalar
  /// plus a dimension preference vector
  void order_to_dim_preference(const UShortArray& order, unsigned short& p,
			       RealVector& dim_pref);

  //
  //- Heading: Data
  //

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
append_expansion(const RealMatrix& samples, const IntResponseMap& resp_map)
{
  switch (expansionCoeffsApproach) {
  case Pecos::ORTHOG_LEAST_INTERPOLATION: // no exp order update
    NonDExpansion::append_expansion(samples, resp_map); break;
  default:
    // adapt the expansion in sync with the dataset
    numSamplesOnModel += resp_map.size();
    increment_order_from_grid();
    // utilize rebuild following expansion updates
    uSpaceModel.append_approximation(samples, resp_map, true);
    break;
  }
}


inline size_t NonDPolynomialChaos::collocation_points() const
{ return collocPtsSpec; }

} // namespace Dakota

#endif
