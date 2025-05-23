/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_LOCAL_EVIDENCE_H
#define NOND_LOCAL_EVIDENCE_H

#include "NonDLocalInterval.hpp"

namespace Dakota {


/// Class for the Dempster-Shafer Evidence Theory methods within DAKOTA/UQ

/** The NonDEvidence class implements the propagation of epistemic uncertainty
    using Dempster-Shafer theory of evidence. In this approach, one assigns 
    a set of basic probability assignments (BPA) to intervals defined 
    for the uncertain variables.  Input interval combinations are calculated,
    along with their BPA.  Currently, the response function is evaluated at 
    a set of sample points, then a response surface is constructed which is 
    sampled extensively to find the minimum and maximum within each input 
    interval cell, corresponding to the belief and plausibility within that 
    cell, respectively.  This data is then aggregated to calculate cumulative 
    distribution functions for belief and plausibility. */

class NonDLocalEvidence: public NonDLocalInterval
{
public:

  //
  //- Heading: Constructors and destructor
  //

  NonDLocalEvidence(ProblemDescDB& problem_db, std::shared_ptr<Model> model); ///< constructor
  ~NonDLocalEvidence() override;                                       ///< destructor

  //
  //- Heading: Member functions
  //

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void initialize() override;
  void set_cell_bounds() override;
  void truncate_to_cell_bounds(RealVector& initial_pt) override;
  void post_process_cell_results(bool maximize) override;
  void post_process_response_fn_results() override;
  void post_process_final_results() override;

private:

  //
  //- Heading: Data members
  //

};

} // namespace Dakota

#endif
