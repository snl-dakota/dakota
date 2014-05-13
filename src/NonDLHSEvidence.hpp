/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDLHSEvidence
//- Description: Class for the Evidence theory methods within DAKOTA/UQ
//- Owner:	 Laura Swiler
//- Checked by:
//- Version:

#ifndef NOND_LHS_EVIDENCE_H
#define NOND_LHS_EVIDENCE_H

#include "dakota_data_types.hpp"
#include "NonDLHSInterval.hpp"

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

class NonDLHSEvidence: public NonDLHSInterval
{
public:

  //
  //- Heading: Constructors and destructor
  //

  NonDLHSEvidence(ProblemDescDB& problem_db, Model& model); ///< constructor
  ~NonDLHSEvidence();                                       ///< destructor

  //
  //- Heading: virtual function redefinitions
  //

  void initialize();
  void post_process_samples();

private:

  //
  // - Heading: Data
  //

};

} // namespace Dakota

#endif
