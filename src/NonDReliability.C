/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDReliability
//- Description: Implementation code for NonDReliability class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "system_defs.h"
#include "ProblemDescDB.H"
#include "NonDReliability.H"
#include "DakotaGraphics.H"

static const char rcsId[] = "@(#) $Id: NonDReliability.C 6793 2010-05-21 00:07:25Z mseldre $";

namespace Dakota {
extern Graphics dakota_graphics; // defined in ParallelLibrary.C


NonDReliability::NonDReliability(Model& model): NonD(model),
  integrationRefinement(NO_INT_REFINE), numRelAnalyses(0)
  //refinementSamples(probDescDB.get_int("method.samples")),
  //refinementSeed(probDescDB.get_int("method.random_seed"))
{
  // Check for suitable distribution types.
  if (numDiscreteIntVars || numDiscreteRealVars) {
    Cerr << "Error: discrete random variables are not supported in reliability "
	 << "methods." << std::endl;
    abort_handler(-1);
  }

  initialize_random_variable_transformation();
  initialize_random_variable_types(STD_NORMAL_U); // need ranVarTypesX below
  // Note: initialize_random_variable_parameters() is performed at run time
  initialize_random_variable_correlations();
  verify_correlation_support();
  initialize_final_statistics(); // default statistics set

  // RealVectors are sized within derived classes
  computedRespLevels.resize(numFunctions);
  computedProbLevels.resize(numFunctions);
  computedGenRelLevels.resize(numFunctions);
}


NonDReliability::~NonDReliability()
{ }


void NonDReliability::initialize_graphics(bool graph_2d, bool tabular_data,
					  const String& tabular_file)
{
  // Set up special graphics for CDF/CCDF visualization
  if (totalLevelRequests) {
    // Customizations must follow 2D plot initialization (setting axis labels
    // calls SciPlotUpdate) and must precede tabular data file initialization
    // (so that the file header includes any updates to tabularCntrLabel).
    if (graph_2d) {     // initialize the 2D plots
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

    //if (tabular_data) { // initialize the tabular data file
    //  dakota_graphics.tabular_counter_label("z");
    //  dakota_graphics.create_tabular_datastream(
    //    iteratedModel.current_variables(), iteratedModel.current_response(),
    //    tabular_file);
    //}
  }
}

} // namespace Dakota
