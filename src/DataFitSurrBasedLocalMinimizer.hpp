/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       DataFitSurrBasedLocalMinimizer
//- Description: A local surrogate-based algorithm which successively invokes
//-              a minimizer on an approximate model within a trust region.
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: DataFitSurrBasedLocalMinimizer.hpp 6879 2010-07-30 01:05:11Z mseldre $

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

class DataFitSurrBasedLocalMinimizer: public SurrBasedLocalMinimizer
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// constructor
  DataFitSurrBasedLocalMinimizer(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~DataFitSurrBasedLocalMinimizer();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void pre_run();

  void post_run(std::ostream& s);

  //
  //- Heading: Virtual functions
  //

  SurrBasedLevelData& trust_region();

  void update_trust_region();

  void build();
  void minimize();
  void verify();

  bool build_global();
  bool build_local();
  void compute_center_correction(bool embed_correction);

  unsigned short converged();

  //
  //- Heading: Convenience member functions
  //

  /// retrieve responseCenterTruth if possible, evaluate it if not
  void find_center_truth();
  /// retrieve responseCenter_approx if possible, evaluate it if not
  void find_center_approx();

  //
  //- Heading: Data members
  //

  /// container for trust region variable/response data
  SurrBasedLevelData trustRegionData;

  /// flags the use of a global data fit surrogate (rsm, ann, mars, kriging)
  bool globalApproxFlag;
  /// flags the use of a multipoint data fit surrogate (TANA)
  bool multiptApproxFlag;
  /// flags the use of a local data fit surrogate (Taylor series)
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
