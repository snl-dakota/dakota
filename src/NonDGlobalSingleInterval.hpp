/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDSingleInterval
//- Description: Class for interval bound estimation for epistemic UQ
//- Owner:	 Laura Swiler
//- Checked by:
//- Version:

#ifndef NOND_GLOBAL_SINGLE_INTERVAL_H
#define NOND_GLOBAL_SINGLE_INTERVAL_H

#include "NonDGlobalInterval.hpp"
#include "dakota_data_types.hpp"

namespace Dakota {


/// Class for using global nongradient-based optimization approaches
/// to calculate interval bounds for epistemic uncertainty quantification

/** The NonDGlobalSingleInterval class supports global nongradient-based
    optimization apporaches to determining interval bounds for
    epistemic UQ.  The interval bounds may be on the entire function
    in the case of pure interval analysis (e.g. intervals on input =
    intervals on output), or the intervals may be on statistics of an
    "inner loop" aleatory analysis such as intervals on means,
    variances, or percentile levels.  The preliminary implementation
    will use a Gaussian process surrogate to determine interval bounds. */

class NonDGlobalSingleInterval: public NonDGlobalInterval
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  NonDGlobalSingleInterval(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDGlobalSingleInterval();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void initialize();
  void post_process_cell_results(bool maximize);
  void get_best_sample(bool maximize, bool eval_approx);

private:

  //
  //- Heading: Data
  //

  /// counter for finalStatistics
  size_t statCntr;
};

} // namespace Dakota

#endif
