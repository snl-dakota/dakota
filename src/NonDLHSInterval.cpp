/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
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
#include "ParallelLibrary.hpp" 
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


void NonDLHSInterval::derived_init_communicators(ParLevLIter pl_iter)
{
  //iteratedModel.init_communicators(pl_iter, maxEvalConcurrency);

  // lhsSampler uses NoDBBaseConstructor, so no need to manage DB list
  // nodes at this level
  lhsSampler.init_communicators(pl_iter);
}


void NonDLHSInterval::derived_set_communicators(ParLevLIter pl_iter)
{
  miPLIndex = methodPCIter->mi_parallel_level_index(pl_iter);
  //iteratedModel.set_communicators(pl_iter, maxEvalConcurrency);

  // lhsSampler uses NoDBBaseConstructor, so no need to manage DB list
  // nodes at this level
  lhsSampler.set_communicators(pl_iter);
}


void NonDLHSInterval::derived_free_communicators(ParLevLIter pl_iter)
{
  lhsSampler.free_communicators(pl_iter);
  //iteratedModel.free_communicators(pl_iter, maxEvalConcurrency);
}


void NonDLHSInterval::core_run()
{
  // set up data structures
  initialize();

  // Evaluate a set of random samples
  ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
  lhsSampler.run(pl_iter);

  // Use the sample set generated above to determine the maximum and minimum 
  // of each function within each input interval combination
  post_process_samples();
}


void NonDLHSInterval::initialize()
{ } // default is no-op

} // namespace Dakota
