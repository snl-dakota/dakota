/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       HierarchSurrBasedLocalMinimizer
//- Description: A local optimization algorithm that uses multiple model forms
//-              and discretization levels to accelerate convergence of a high-
//-              fidelity, finely-resolved model.
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: HierarchSurrBasedLocalMinimizer.hpp 6879 2010-07-30 01:05:11Z mseldre $

#ifndef HIERARCH_SURR_BASED_LOCAL_MINIMIZER_H
#define HIERARCH_SURR_BASED_LOCAL_MINIMIZER_H

#include "SurrBasedLocalMinimizer.hpp"
#include "HierarchSurrModel.hpp"
#include "DakotaModel.hpp"
#include "SurrBasedLevelData.hpp"

namespace Dakota {


/// Class for multilevel-multifidelity optimization algorithm

/** This minimizer uses SurrogateModel(s) to perform minimization leveraging
    multiple model forms and discretization levels. */

class HierarchSurrBasedLocalMinimizer: public SurrBasedLocalMinimizer
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// constructor
  HierarchSurrBasedLocalMinimizer(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~HierarchSurrBasedLocalMinimizer();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void pre_run();
  void post_run(std::ostream& s);

  //void reset();

  SurrBasedLevelData& trust_region();

  void update_trust_region();

  void build();
  void minimize();
  void verify();

private:

  //
  //- Heading: Convenience member functions
  //

  /// Retrieve or evaluate SurrBasedLevelData::responseCenterTruthUncorrected
  void find_center_truth(size_t tr_index);
  /// Retrieve or evaluate SurrBasedLevelData::responseCenterApproxUncorrected
  void find_center_approx(size_t tr_index);
  /// Retrieve or evaluate SurrBasedLevelData::responseStarApproxUncorrected
  void find_star_approx(size_t tr_index);

  /// activate model forms and, optionally, discretization levels within
  /// the HierarchSurrModel associated with trustRegions[tr_index]
  void set_model_states(size_t tr_index);

  // MG/Opt functions:

  RealVector MG_Opt(const RealVector &xk, int k);

  void MG_Opt_driver(const Variables &x0);

  RealVector optimize(const RealVector &x, int max_iter, int index);

  RealVector linesearch(const RealVector &xk, const RealVector &pk,
                        double alpha0);

  //
  //- Heading: Data members
  //

  /// number of ordered model fidelities within iteratedModel
  /// (a HierarchSurrModel)
  size_t numFid;
  //bool multiFid;

  /// number of discretization levels per ordered model fidelity
  SizetArray numLev;
  /// flag indicating presence of more than one level per fidelity
  bool multiLev;

  /// index for trustRegions at which the minimization is performed
  size_t minimizeIndex;

  std::vector<SurrBasedLevelData> trustRegions;
  bool nestedTrustRegions;

  // pointer to MLMF instance used in static member functions
  //static HierarchSurrBasedLocalMinimizer* mlmfInstance;
};


inline SurrBasedLevelData& HierarchSurrBasedLocalMinimizer::trust_region()
{ return trustRegions[minimizeIndex]; }


inline void HierarchSurrBasedLocalMinimizer::set_model_states(size_t tr_index)
{
  if (multiLev) {
    iteratedModel.surrogate_model_indices(
      trustRegions[tr_index].approx_model_form(),
      trustRegions[tr_index].approx_model_level());
    iteratedModel.truth_model_indices(
      trustRegions[tr_index].truth_model_form(),
      trustRegions[tr_index].truth_model_level());
  }
  else {
    iteratedModel.surrogate_model_indices(
      trustRegions[tr_index].approx_model_form());
    iteratedModel.truth_model_indices(
      trustRegions[tr_index].truth_model_form());
  }
}

} // namespace Dakota

#endif
