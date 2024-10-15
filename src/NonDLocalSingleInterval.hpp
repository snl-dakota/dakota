/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_LOCAL_SINGLE_INTERVAL_H
#define NOND_LOCAL_SINGLE_INTERVAL_H

#include "NonDLocalInterval.hpp"

namespace Dakota {


/// Class for using local gradient-based optimization approaches to
/// calculate interval bounds for epistemic uncertainty quantification

/** The NonDLocalSingleInterval class supports local gradient-based
    optimization apporaches to determining interval bounds for
    epistemic UQ.  The interval bounds may be on the entire function
    in the case of pure interval analysis (e.g. intervals on input =
    intervals on output), or the intervals may be on statistics of an
    "inner loop" aleatory analysis such as intervals on means,
    variances, or percentile levels. */

class NonDLocalSingleInterval: public NonDLocalInterval
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  NonDLocalSingleInterval(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDLocalSingleInterval() override;

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void initialize() override;
  void post_process_cell_results(bool maximize) override;

private:

  //
  //- Heading: Data
  //

  /// counter for finalStatistics
  size_t statCntr;
};

} // namespace Dakota

#endif
