/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DATA_FIT_SURR_BASED_LOCAL_MINIMIZER_H
#define DATA_FIT_SURR_BASED_LOCAL_MINIMIZER_H

#include "SurrBasedLocalMinimizer.hpp"
#include "DakotaModel.hpp"
#include "SurrBasedLevelData.hpp"

namespace Dakota {


/// Class for provably-convergent local surrogate-based optimization
/// and nonlinear least squares.

/** This minimizer uses a SurrogateModel to perform minimization based
    on local, global, or hierarchical surrogates. It achieves provable
    convergence through the use of a sequence of trust regions and the
    application of surrogate corrections at the trust region centers. */


/**
 * \brief A version of TraitsBase specialized for local surrogate-based minimizer
 *
 */

class DataFitSurrBasedLocalTraits: public TraitsBase
{
  public:

  /// default constructor
  DataFitSurrBasedLocalTraits() { }

  /// destructor
  ~DataFitSurrBasedLocalTraits() override { }

  /// A temporary query used in the refactor
  bool is_derived() override { return true; }

  /// Return the flag indicating whether method supports continuous variables
  bool supports_continuous_variables() override { return true; }

  /// Return the flag indicating whether method supports linear equalities
  bool supports_linear_equality() override { return true; }

  /// Return the flag indicating whether method supports linear inequalities
  bool supports_linear_inequality() override { return true; }

  /// Return the flag indicating whether method supports nonlinear equalities
  bool supports_nonlinear_equality() override { return true; }

  /// Return the flag indicating whether method supports nonlinear inequalities
  bool supports_nonlinear_inequality() override { return true; }
};


class DataFitSurrBasedLocalMinimizer: public SurrBasedLocalMinimizer
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// constructor
  DataFitSurrBasedLocalMinimizer(ProblemDescDB& problem_db, ParallelLibrary& parallel_lib,  std::shared_ptr<Model> model);
  /// alternate constructor for instantiations "on the fly"
  DataFitSurrBasedLocalMinimizer(std::shared_ptr<Model> model, short merit_fn,
				 short accept_logic, short constr_relax,
				 Real tr_factor, size_t max_iter,
				 size_t max_eval, Real conv_tol,
				 unsigned short soft_conv_limit,
				 bool use_derivs);
  /// destructor
  ~DataFitSurrBasedLocalMinimizer() override;

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void pre_run() override;

  void post_run(std::ostream& s) override;

  //
  //- Heading: Virtual functions
  //

  SurrBasedLevelData& trust_region() override;

  void update_trust_region() override;

  void build() override;
  void minimize() override;
  void verify() override;

  bool build_global();
  bool build_centered();
  void compute_center_correction(bool embed_correction);

  unsigned short converged() override;

  //
  //- Heading: Convenience member functions
  //

  /// helper for shared ctor code
  void initialize_trust_region_data(const String& approx_type,
				    short corr_order);

  /// retrieve responseCenterTruth if possible, evaluate it if not
  void find_center_truth();
  /// retrieve responseCenter_approx if possible, evaluate it if not
  void find_center_approx();

  //
  //- Heading: Data members
  //

  /// container for trust region variable/response data
  SurrBasedLevelData trustRegionData;

  /// flags the use of a global data fit surrogate (NN, MARS, GP, RBF, et al.)
  bool globalApproxFlag;
  /// flags the use of a multipoint data fit surrogate (MPEA et al.)
  bool multiPtApproxFlag;
  /// flags the use of a two-point data fit surrogate (TANA)
  bool twoPtApproxFlag;
  /// flags the use of a local/single-point data fit surrogate (Taylor series)
  bool localApproxFlag;

  // flag indicating inclusion of the center point in the DACE
  // evaluations for global approximations (CCD, Box-Behnken)
  //bool daceCenterPtFlag;
  // secondary flag indicating daceCenterPtFlag and no bounds truncation
  //bool daceCenterEvalFlag;

  /// flags the simultaneous presence of two conditions: (1) additional
  /// layerings w/i actual_model (e.g., surrogateModel = layered/nested/layered
  /// -> actual_model = nested/layered), and (2) a user-specification to bypass
  /// all layerings within actual_model for the evaluation of truth data
  /// (responseCenterTruth and responseStarTruth).
  bool multiLayerBypassFlag;
  /// flag for the "use_derivatives" specification for which derivatives
  /// are to be evaluated at each DACE point in global surrogate builds.
  bool useDerivsFlag;
};


inline DataFitSurrBasedLocalMinimizer::~DataFitSurrBasedLocalMinimizer()
{ }


inline SurrBasedLevelData& DataFitSurrBasedLocalMinimizer::trust_region()
{ return trustRegionData; }


inline void DataFitSurrBasedLocalMinimizer::update_trust_region()
{ update_trust_region_data(trustRegionData, globalLowerBnds, globalUpperBnds); }


inline unsigned short DataFitSurrBasedLocalMinimizer::converged()
{ return trustRegionData.converged(); }

} // namespace Dakota

#endif
