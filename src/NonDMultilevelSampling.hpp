/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDSampling
//- Description: Wrapper class for Fortran 90 LHS library
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#ifndef NOND_MULTILEVEL_SAMPLING_H
#define NOND_MULTILEVEL_SAMPLING_H

#include "NonDSampling.hpp"
#include "DataMethod.hpp"


namespace Dakota {

/// Performs Multilevel Monte Carlo sampling for uncertainty quantification.

/** Multilevel Monte Carlo (MLMC) is a variance-reduction technique
    that utilitizes lower fidelity simulations that have response QoI
    that are correlated with the high-fidelity response QoI. */

class NonDMultilevelSampling: public NonDSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDMultilevelSampling(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDMultilevelSampling();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  // together the three run components perform a forward uncertainty
  // propagation by using LHS to generate a set of parameter samples,
  // performing function evaluations on these parameter samples, and
  // computing statistics on the ensemble of results.

  /// generate LHS samples in non-VBD cases
  void pre_run();
  /// perform the evaluate parameter sets portion of run
  void quantify_uncertainty();
  /// generate statistics for LHS runs in non-VBD cases
  void post_run(std::ostream& s);

  /// print the final statistics
  void print_results(std::ostream& s);

private:

  //
  //- Heading: Data
  //

};



} // namespace Dakota

#endif
