/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDLocalSingleInterval
//- Description: Class for interval bound estimation for epistemic UQ
//- Owner:	 Mike Eldred
//- Checked by:
//- Version:

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
  ~NonDLocalSingleInterval();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void initialize();
  void post_process_cell_results(bool maximize);

private:

  //
  //- Heading: Data
  //

  /// counter for finalStatistics
  size_t statCntr;
};

} // namespace Dakota

#endif
