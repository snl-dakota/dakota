/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDLHSInterval
//- Description: Class for the Interval theory methods within DAKOTA/UQ
//- Owner:	 Laura Swiler
//- Checked by:
//- Version:

#ifndef NOND_LHS_INTERVAL_H
#define NOND_LHS_INTERVAL_H

#include "dakota_data_types.hpp"
#include "NonDInterval.hpp"
#include "DakotaModel.hpp"

namespace Dakota {


/// Class for the LHS-based interval methods within DAKOTA/UQ

/** The NonDLHSInterval class implements the propagation of epistemic
    uncertainty using LHS-based methods. */

class NonDLHSInterval: public NonDInterval
{
public:

  //
  //- Heading: Constructors and destructor
  //

  NonDLHSInterval(ProblemDescDB& problem_db, Model& model); ///< constructor
  ~NonDLHSInterval();                                       ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  void derived_init_communicators(ParLevLIter pl_iter);
  void derived_set_communicators(ParLevLIter pl_iter);
  void derived_free_communicators(ParLevLIter pl_iter);

  /// performs an epistemic uncertainty propagation using LHS samples
  void core_run();

protected:

  //
  //- Heading: New virtual functions
  //

  /// perform any required initialization
  virtual void initialize();
  /// post-process the output from executing lhsSampler
  virtual void post_process_samples() = 0;

  //
  // - Heading: Data
  //

  Iterator  lhsSampler; ///< the LHS sampler instance
  const int seedSpec;   ///< the user seed specification (default is 0)     
  int       numSamples; ///< the number of samples used
  String    rngName;	///< name of the random number generator

private:

  //
  // - Heading: Data
  //

};

} // namespace Dakota

#endif
