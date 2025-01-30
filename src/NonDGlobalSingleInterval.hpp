/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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
  NonDGlobalSingleInterval(ProblemDescDB& problem_db, std::shared_ptr<Model> model);
  /// destructor
  ~NonDGlobalSingleInterval() override;

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void initialize() override;
  void post_process_cell_results(bool maximize) override;
  void get_best_sample(bool maximize, bool eval_approx) override;

private:

  //
  //- Heading: Data
  //

  /// counter for finalStatistics
  size_t statCntr;
};

} // namespace Dakota

#endif
