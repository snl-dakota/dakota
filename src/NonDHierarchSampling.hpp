/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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

  Real estimator_accuracy_metric();

  //
  //- Heading: Member functions
  //

  /// queue a batch of samples to be performed for the active model subset
  void ensemble_sample_batch(const String& prepend, int batch_id,
			     bool new_samples = true);
  /// set request = 1 for QoI in (lev-1,lev)
  void ml_active_set(size_t lev, size_t offset = 0, bool clear_req = true);

  /// export allSamples to tagged tabular file
  void export_all_samples(String root_prepend, const Model& model,
			  size_t iter, int batch_id);

  //
  //- Heading: Data
  //

  /// number of model forms/resolution in the (outer) sequence
  size_t numSteps;
  /// setting for inactive model dimension not traversed by loop over numSteps
  size_t secondaryIndex;

  /// final estimator variance for targeted moment (usually mean), averaged
  /// across QoI
  Real avgEstVar;

private:

  //
  //- Heading: Helper functions
  //

};


inline Real NonDHierarchSampling::estimator_accuracy_metric()
{ return avgEstVar; }


inline void NonDHierarchSampling::
ml_active_set(size_t lev, size_t offset, bool clear_req)
{
  if (clear_req) activeSet.request_values(0);

  if (lev) {
    size_t lm1 = lev - 1;
    if (lm1) offset += lm1 * numFunctions;
    size_t end = offset + numFunctions;
    activeSet.request_values(1, offset, end);
    offset = end;
  }
  activeSet.request_values(1, offset, offset + numFunctions);
}

} // namespace Dakota

#endif
