/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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

  NonDLHSInterval(ProblemDescDB& problem_db, std::shared_ptr<Model> model); ///< constructor
  ~NonDLHSInterval() override;                                       ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  void derived_init_communicators(ParLevLIter pl_iter) override;
  void derived_set_communicators(ParLevLIter pl_iter) override;
  void derived_free_communicators(ParLevLIter pl_iter) override;

  /// performs an epistemic uncertainty propagation using LHS samples
  void core_run() override;

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

  std::unique_ptr<Iterator>  lhsSampler; ///< the LHS sampler instance
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
