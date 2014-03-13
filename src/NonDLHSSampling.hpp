/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDSampling
//- Description: Wrapper class for Fortran 90 LHS library
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#ifndef NOND_LHS_SAMPLING_H
#define NOND_LHS_SAMPLING_H

#include "NonDSampling.hpp"
#include "DataMethod.hpp"


namespace Dakota {

/// Performs LHS and Monte Carlo sampling for uncertainty quantification.

/** The Latin Hypercube Sampling (LHS) package from Sandia
    Albuquerque's Risk and Reliability organization provides
    comprehensive capabilities for Monte Carlo and Latin Hypercube
    sampling within a broad array of user-specified probabilistic
    parameter distributions.  It enforces user-specified rank
    correlations through use of a mixing routine.  The NonDLHSSampling
    class provides a C++ wrapper for the LHS library and is used for
    performing forward propagations of parameter uncertainties into
    response statistics. */

class NonDLHSSampling: public NonDSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDLHSSampling(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor for sample generation and evaluation "on the fly"
  NonDLHSSampling(Model& model, unsigned short sample_type,
		  int samples, int seed, const String& rng,
		  bool vary_pattern = true, short sampling_vars_mode = ACTIVE);
  /// alternate constructor for sample generation "on the fly"
  NonDLHSSampling(unsigned short sample_type, int samples, int seed,
		  const String& rng, const RealVector& lower_bnds,
		  const RealVector& upper_bnds);
  /// destructor
  ~NonDLHSSampling();

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

  void post_input();

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

  /// number of response functions; used to distinguish NonD from opt/NLS usage
  size_t numResponseFunctions;

  /// flags computation of variance-based decomposition indices
  bool varBasedDecompFlag;
};



} // namespace Dakota

#endif
