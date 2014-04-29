/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonLHSInterval
//- Description: Implementation code for NonDLHSInterval class
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDLHSInterval.hpp"
#include "dakota_data_types.hpp"
#include "dakota_system_defs.hpp"
#include "NonDLHSSampling.hpp"
#include "ProblemDescDB.hpp" 
#include "pecos_stat_util.hpp"

//#define DEBUG

namespace Dakota {


NonDLHSInterval::NonDLHSInterval(ProblemDescDB& problem_db, Model& model):
  NonDInterval(problem_db, model),
  seedSpec(probDescDB.get_int("method.random_seed")),
  numSamples(probDescDB.get_int("method.samples")),
  rngName(probDescDB.get_string("method.random_number_generator"))
{
  // if the user does not specify the number of samples, 
  // perform at least 10000 to get accurate assessments of belief and plaus.
  if (!numSamples)
    numSamples = 10000;

  maxEvalConcurrency *= numSamples;

  unsigned short sample_type = SUBMETHOD_DEFAULT;
  bool vary_pattern = false; // for consistency across outer loop invocations
  lhsSampler.assign_rep(new NonDLHSSampling(iteratedModel, sample_type,
    numSamples, seedSpec, rngName, vary_pattern, ACTIVE), false);
}


NonDLHSInterval::~NonDLHSInterval()
{ }


void NonDLHSInterval::init_communicators()
{
  //iteratedModel.init_communicators(maxEvalConcurrency);
  lhsSampler.init_communicators();
}


void NonDLHSInterval::free_communicators()
{
  lhsSampler.free_communicators();
  //iteratedModel.free_communicators(maxEvalConcurrency);
}


void NonDLHSInterval::quantify_uncertainty()
{
  // set up data structures
  initialize();

  // Evaluate a set of random samples
  // no summary output since on-the-fly constructed:
  lhsSampler.run(Cout);

  // Use the sample set generated above to determine the maximum and minimum 
  // of each function within each input interval combination
  post_process_samples();
}


void NonDLHSInterval::initialize()
{ } // default is no-op

} // namespace Dakota
