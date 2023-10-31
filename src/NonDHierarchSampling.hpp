/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
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

  /// average costs once accumulations are complete
  void average_online_cost(const RealVector& accum_cost,
			   const SizetArray& num_cost, RealVector& seq_cost);
  /// recover partial estimates of simulation cost using aggregated (paired)
  /// response metadata
  void accumulate_paired_online_cost(RealVector& accum_cost,
				     SizetArray& num_cost, size_t step);
  /// accumulate cost and counts and then perform averaging
  void recover_paired_online_cost(RealVector& seq_cost, size_t step);

  //
  //- Heading: Data
  //

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
average_online_cost(const RealVector& accum_cost, const SizetArray& num_cost,
		    RealVector& seq_cost)
{
  // Finalize the average cost for the ensemble
  size_t step, num_steps = accum_cost.length();
  if (seq_cost.length() != num_steps) seq_cost.sizeUninitialized(num_steps);
  for (step=0; step<num_steps; ++step)
    seq_cost[step] = accum_cost[step] / num_cost[step];
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Online cost: accum_cost:\n" << accum_cost << "num_cost:\n"
	 << num_cost << "seq_cost:\n" << seq_cost << std::endl;
}


inline void NonDHierarchSampling::
recover_paired_online_cost(RealVector& seq_cost, size_t step)
{
  int len = seq_cost.length();
  RealVector accum_cost(len);                    // init to 0
  SizetArray num_cost;  num_cost.assign(len, 0); // init to 0
  accumulate_paired_online_cost(accum_cost, num_cost, step);
  average_online_cost(accum_cost, num_cost, seq_cost);
}

} // namespace Dakota

#endif
