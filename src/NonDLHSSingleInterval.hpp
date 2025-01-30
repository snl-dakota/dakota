/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_LHS_SINGLE_INTERVAL_H
#define NOND_LHS_SINGLE_INTERVAL_H

#include "dakota_data_types.hpp"
#include "NonDLHSInterval.hpp"
#include "DakotaModel.hpp"
#include "DakotaIterator.hpp"

namespace Dakota {


/// Class for pure interval propagation using LHS

/** The NonDSingleInterval class implements the propagation of epistemic
    uncertainty using ... */

class NonDLHSSingleInterval: public NonDLHSInterval
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  NonDLHSSingleInterval(ProblemDescDB& problem_db, std::shared_ptr<Model> model);
  /// destructor
  ~NonDLHSSingleInterval() override;

protected:

  //
  //- Heading: virtual function redefinitions
  //

  void initialize() override;
  void post_process_samples() override;

private:

  //
  // - Heading: Data
  //

  /// counter for finalStatistics
  size_t statCntr;
};

} // namespace Dakota

#endif
