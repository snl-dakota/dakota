/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDLocalEvidence
//- Description: Class for the Evidence theory methods within DAKOTA/UQ
//- Owner:	 Laura Swiler
//- Checked by:
//- Version:

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

  NonDLocalEvidence(ProblemDescDB& problem_db, Model& model); ///< constructor
  ~NonDLocalEvidence();                                       ///< destructor

  //
  //- Heading: Member functions
  //

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void initialize();
  void set_cell_bounds();
  void truncate_to_cell_bounds(RealVector& initial_pt);
  void post_process_cell_results(bool maximize);
  void post_process_response_fn_results();
  void post_process_final_results();

private:

  //
  //- Heading: Data members
  //

};

} // namespace Dakota

#endif
