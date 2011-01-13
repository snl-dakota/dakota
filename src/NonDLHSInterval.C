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

#include "NonDLHSInterval.H"
#include "data_types.h"
#include "system_defs.h"
#include "NonDLHSSampling.H"
#include "ProblemDescDB.H" 
#include "pecos_stat_util.hpp"

//#define DEBUG

namespace Dakota {


NonDLHSInterval::NonDLHSInterval(Model& model): NonDInterval(model),
  seedSpec(probDescDB.get_int("method.random_seed")),
  numSamples(probDescDB.get_int("method.samples")),
  rngName(probDescDB.get_string("method.random_number_generator"))
{
  // if the user does not specify the number of samples, 
  // perform at least 10000 to get accurate assessments of belief and plaus.
  if (!numSamples)
    numSamples = 10000;

  String sample_type; // empty string: use default sample type
  lhsSampler.assign_rep(new NonDLHSSampling(iteratedModel, sample_type,
    numSamples, seedSpec, rngName), false);  
  iteratedModel.init_communicators(lhsSampler.maximum_concurrency());      
}


NonDLHSInterval::~NonDLHSInterval()
{ iteratedModel.free_communicators(lhsSampler.maximum_concurrency()); }


void NonDLHSInterval::quantify_uncertainty()
{
  // set up data structures
  initialize();

  // Evaluate a set of random samples
  // no summary output since on-the-fly constructed:
  lhsSampler.run_iterator(Cout);

  // Use the sample set generated above to determine the maximum and minimum 
  // of each function within each input interval combination
  post_process_samples();
}


void NonDLHSInterval::initialize()
{ } // default is no-op

} // namespace Dakota
