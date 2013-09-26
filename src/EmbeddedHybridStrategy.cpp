/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       EmbeddedHybridStrategy
//- Description: Implementation code for the EmbeddedHybridStrategy class
//- Owner:       Mike Eldred
//- Checked by:

#include "EmbeddedHybridStrategy.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"

static const char rcsId[]="@(#) $Id: EmbeddedHybridStrategy.cpp 6715 2010-04-02 21:58:15Z wjbohnh $";


using namespace std;

namespace Dakota {

EmbeddedHybridStrategy::EmbeddedHybridStrategy(ProblemDescDB& problem_db):
  HybridStrategy(problem_db),
  localSearchProb(
    problem_db.get_real("strategy.hybrid.local_search_probability"))
{
  if (worldRank == 0)
    cout << "Constructing Embedded Hybrid Optimizer Strategy...\n";

  numIterators = 2;
  methodList.resize(2);
  methodList[0]
    = problem_db.get_string("strategy.hybrid.local_method_pointer");
  methodList[1]
    = problem_db.get_string("strategy.hybrid.global_method_pointer");

  init_iterator_parallelism(); // maxConcurrency is 1 (the default)
  allocate_methods();
}


EmbeddedHybridStrategy::~EmbeddedHybridStrategy()
{
  // Virtual destructor handles referenceCount at Strategy level.

  deallocate_methods();
}


void EmbeddedHybridStrategy::run_strategy()
{
  Iterator& iterator1 = selectedIterators[1];
  Model&    model1    = userDefinedModels[1];

  if (worldRank == 0) {
    cout << "\n>>>>> Running Embedded Hybrid Optimizer Strategy with global "
         << "method = " << methodList[1] << " and local method = " 
	 << methodList[0] << endl;
    // set up plots and tabular data file
    iterator1.initialize_graphics(graph2DFlag, tabularDataFlag,tabularDataFile);
    //iterator1.set_local_search(selectedIterators[0]);
    //iterator1.set_local_search_probability(localSearchProb);
  }

  schedule_iterators(iterator1, model1);

  // Output interesting iterator progress metrics...
  if (worldRank == 0)
    cout << "\n<<<<< Embedded Hybrid Optimizer Strategy completed." << endl;
}

} // namespace Dakota
