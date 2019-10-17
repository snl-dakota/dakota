/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDReliability
//- Description: Implementation code for NonDReliability class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "dakota_system_defs.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "NonDReliability.hpp"
#include "DakotaGraphics.hpp"

static const char rcsId[] = "@(#) $Id: NonDReliability.cpp 6793 2010-05-21 00:07:25Z mseldre $";

namespace Dakota {


NonDReliability::NonDReliability(ProblemDescDB& problem_db, Model& model):
  NonD(problem_db, model),
  mppSearchType(probDescDB.get_ushort("method.nond.reliability_search_type")),
  integrationRefinement(
    probDescDB.get_ushort("method.nond.integration_refinement")),
  numRelAnalyses(0)
  //refinementSamples(probDescDB.get_int("method.samples")),
  //refinementSeed(probDescDB.get_int("method.random_seed"))
{
  // Check for suitable distribution types.
  if (numDiscreteIntVars || numDiscreteStringVars || numDiscreteRealVars) {
    Cerr << "Error: discrete random variables are not supported in reliability "
	 << "methods." << std::endl;
    abort_handler(-1);
  }

  initialize_final_statistics(); // default statistics set

  // RealVectors are sized within derived classes
  computedRespLevels.resize(numFunctions);
  computedProbLevels.resize(numFunctions);
  computedGenRelLevels.resize(numFunctions);
}


NonDReliability::~NonDReliability()
{ }


bool NonDReliability::resize()
{
  bool parent_reinit_comms = NonD::resize();

  initialize_final_statistics(); // default statistics set

  // RealVectors are sized within derived classes
  computedRespLevels.resize(numFunctions);
  computedProbLevels.resize(numFunctions);
  computedGenRelLevels.resize(numFunctions);

  return parent_reinit_comms;
}


void NonDReliability::initialize_graphics(int iterator_server_id)
{
  // Set up special graphics for CDF/CCDF visualization
  if (totalLevelRequests) {

    OutputManager& mgr = parallelLib.output_manager();
    Graphics& dakota_graphics = mgr.graphics();

    // For graphics, limit (currently) to server id 1, for both ded master
    // (parent partition rank 1) and peer partitions (parent partition rank 0)
    if (mgr.graph2DFlag && iterator_server_id == 1) { // initialize the 2D plots
      dakota_graphics.create_plots_2d(iteratedModel.current_variables(),
				      iteratedModel.current_response());
      // Visualize mostProbPointX in the vars windows and CDF/CCDF
      // probability/reliability-response level pairs in the response windows.
      dakota_graphics.set_x_labels2d("Response Level");
      size_t i;
      for (i=0; i<numFunctions; i++)
	dakota_graphics.set_y_label2d(i, "Probability");
      for (i=0; i<numContinuousVars; i++)
	dakota_graphics.set_y_label2d(i+numFunctions, "Most Prob Point");
    }

    /*
    // For output/restart/tabular data, all Iterator masters stream output
    if (mgr.tabularDataFlag) { // initialize the tabular data file
      dakota_graphics.tabular_counter_label("z");
      dakota_graphics.create_tabular_datastream(
        iteratedModel.current_variables(), iteratedModel.current_response(),
        );
    }
    */
  }
}


void NonDReliability::pre_run()
{
  Analyzer::pre_run();
  
  // IteratorScheduler::run_iterator() + Analyzer::initialize_run() ensure
  // initialization of Model mappings for iteratedModel, but local recursions
  // are not visible -> recur DataFitSurr +  ProbabilityTransform if needed.
  // > Note: part of this occurs at DataFit build time. Therefore, take
  //         care to avoid redundancy using mappingInitialized flag.
  if (!mppModel.is_null()) { // all except Mean Value
    if (!mppModel.mapping_initialized()) {
      ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
      /*bool var_size_changed =*/ mppModel.initialize_mapping(pl_iter);
      //if (var_size_changed) resize();
    }

    // now that vars/labels/bounds/targets have flowed down at run-time from
    // any higher level recursions, propagate them up local Model recursions
    // so that they are correct when they propagate back down.
    mppModel.update_from_subordinate_model(); // depth = max
  }
}


void NonDReliability::post_run(std::ostream& s)
{
  ++numRelAnalyses;

  if (!mppModel.is_null() && mppModel.mapping_initialized()) {
    /*bool var_size_changed =*/ mppModel.finalize_mapping();
    //if (var_size_changed) resize();
  }

  Analyzer::post_run(s);
}

} // namespace Dakota
