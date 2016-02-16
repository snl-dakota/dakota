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

  initialize_random_variable_transformation();
  initialize_random_variable_types(STD_NORMAL_U); // need ranVarTypesX below
  // Note: initialize_random_variable_parameters() is performed at run time
  initialize_random_variable_correlations();
  verify_correlation_support(STD_NORMAL_U);
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

  initialize_random_variable_transformation();
  initialize_random_variable_types(STD_NORMAL_U); // need ranVarTypesX below
  // Note: initialize_random_variable_parameters() is performed at run time
  initialize_random_variable_correlations();
  verify_correlation_support(STD_NORMAL_U);
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
      for (i=0; i<numUncertainVars; i++)
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

} // namespace Dakota
