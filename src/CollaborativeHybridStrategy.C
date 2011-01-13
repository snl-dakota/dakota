/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       CollaborativeHybridStrategy
//- Description: Implementation code for the CollaborativeHybridStrategy class
//- Owner:       Patty Hough/John Siirola
//- Checked by:

#include "CollaborativeHybridStrategy.H"
#include "ProblemDescDB.H"
#include "ParallelLibrary.H"

static const char rcsId[]="@(#) $Id: CollaborativeHybridStrategy.C 6715 2010-04-02 21:58:15Z wjbohnh $";


namespace Dakota {

CollaborativeHybridStrategy::
CollaborativeHybridStrategy(ProblemDescDB& problem_db):
  HybridStrategy(problem_db)
  //hybridCollabType(
  //  problem_db.get_string("strategy.hybrid.collaborative_type"))
{
  if (worldRank == 0)
    Cout << "Constructing Collaborative Hybrid Optimizer Strategy...\n";

  methodList = problem_db.get_dsa("strategy.hybrid.method_list");
  numIterators = methodList.size();
  if (!numIterators) { // verify at least one method in list
    Cerr << "Error: method_list must have a least one entry." << std::endl;
    abort_handler(-1);
  }

  //maxConcurrency = ;
  init_iterator_parallelism();
  allocate_methods();
  // Note: rather than the standard allocate_methods() approach, this is where
  // logic would be placed to process the model list and create
  // CollaborativeModel recursions, one for each unique user-defined model.
  // These CollaborativeModels would then be passed into the selectedIterators
  // instantiations and would support a shared processing queue.
}


CollaborativeHybridStrategy::~CollaborativeHybridStrategy()
{
  // Virtual destructor handles referenceCount at Strategy level.

  deallocate_methods();
}


void CollaborativeHybridStrategy::run_strategy()
{
  // THIS CODE IS JUST A PLACEHOLDER

  for (size_t i=0; i<numIterators; i++) {

    Iterator& curr_iterator = selectedIterators[i];
    Model&    curr_model    = userDefinedModels[i];

    if (worldRank == 0) {
      Cout << "\n>>>>> Running Collaborative Hybrid Optimizer Strategy with "
	   << "iterator " << methodList[i] << ".\n";
      // set up plots and tabular data file
      curr_iterator.initialize_graphics(graph2DFlag, tabularDataFlag,
					tabularDataFile);
    }

    schedule_iterators(curr_iterator, curr_model);
  }

  // Output interesting iterator statistics...
  if (worldRank == 0)
    Cout << "\n<<<<< CollaborativeHybrid Optimizer Strategy completed.\n";
}

} // namespace Dakota
