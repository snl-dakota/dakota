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
#include "HierarchSurrBasedLocalMinimizerHelper.hpp"

namespace Dakota
{

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

  /// reset convergence controls in case of multiple MLMF executions
  void reset();

  /// update the trust region bounds, strictly contained within global bounds
  void update_trust_region();

  void verify();
  void minimize();
  void build();

private:

  //
  //- Heading: Convenience member functions
  //

  void find_center(size_t tr_index);

  void hard_convergence_check(size_t tr_index);

  void tr_ratio_check(size_t tr_index);

  void set_model_states(size_t tr_index);

  //
  //- Heading: Data members
  //

  size_t numFid;
  SizetArray numLev;
  size_t minimizeIndex;

  RealVector initialPoint;

  std::vector<HierarchSurrBasedLocalMinimizerHelper> trustRegions;

  /// pointer to MLMF instance used in static member functions
  static HierarchSurrBasedLocalMinimizer* mlmfInstance;
};

} // namespace Dakota

#endif
