/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDHierarchSampling
//- Description: Base class for hierarchical ML/MF sampling methods
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#ifndef NOND_HIERARCHICAL_SAMPLING_H
#define NOND_HIERARCHICAL_SAMPLING_H

#include "NonDEnsembleSampling.hpp"
#include "DataMethod.hpp"


namespace Dakota {

/// Performs Hierarch Monte Carlo sampling for uncertainty quantification.

/** Hierarch Monte Carlo (MLMC) is a variance-reduction technique
    that utilitizes lower fidelity simulations that have response QoI
    that are correlated with the high-fidelity response QoI. */

class NonDHierarchSampling: public NonDEnsembleSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDHierarchSampling(ProblemDescDB& problem_db, Model& model);
  /// destructor (virtual declaration should be redundant with ~Iterator,
  /// but this is top of MLMF diamond so doesn't hurt to be explicit)
  virtual ~NonDHierarchSampling();

  //
  //- Heading: Virtual function redefinitions
  //

  //bool resize();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  //void pre_run();
  //void core_run();
  //void post_run(std::ostream& s);
  //void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

  //
  //- Heading: Member functions
  //

  // synchronize iteratedModel and activeSet on AGGREGATED_MODELS mode
  //void aggregated_models_mode();
  // synchronize iteratedModel and activeSet on BYPASS_SURROGATE mode
  //void bypass_surrogate_mode();
  /// synchronize iteratedModel and activeSet on UNCORRECTED_SURROGATE mode
  void uncorrected_surrogate_mode();

  //
  //- Heading: Data
  //

private:

  //
  //- Heading: Helper functions
  //

};


inline void NonDHierarchSampling::uncorrected_surrogate_mode()
{
  if (iteratedModel.surrogate_response_mode() != UNCORRECTED_SURROGATE) {
    iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE); // LF
    activeSet.reshape(numFunctions);// synch with model.response_size()
  }
}

} // namespace Dakota

#endif
