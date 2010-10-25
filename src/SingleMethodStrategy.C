/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SingleMethodStrategy
//- Description: Implementation code for the SingleMethodStrategy class
//- Owner:       Mike Eldred
//- Checked by:

#include "SingleMethodStrategy.H"
#include "ProblemDescDB.H"

static const char rcsId[]="@(#) $Id: SingleMethodStrategy.C 6492 2009-12-19 00:04:28Z briadam $";


using namespace std;

namespace Dakota {

SingleMethodStrategy::SingleMethodStrategy(ProblemDescDB& problem_db):
  Strategy(BaseConstructor(), problem_db)
{
  if (worldRank==0)
    cout << "Constructing Single Method Strategy...\n";

  init_iterator_parallelism();
  // Since this strategy does not support any iterator concurrency, verify 
  // sanity of settings so that simple run_iterator() may be used below
  // (schedule_iterators() could be used, but this adds unnecessary overhead).
  if (stratIterDedMaster || numIteratorServers > 1) {
    cerr << "Error: Single Method Strategy does not support concurrent "
	 << "iterator parallelism." << endl;
    abort_handler(-1);
  }

  // With respect to SingleMethodStrategy interaction with the problem_db 
  // linked lists, the current design allows the user to either fully specify
  // the method to be used (which may itself contain pointers to variables, 
  // interface, and responses) or to rely on the default behavior in which 
  // the last data populated in the calls to the keyword handlers is used to
  // build the model and the iterator (should be the only call for lazy 
  // specification of a single_method strategy to be the most sensible).
  const String& method_ptr = problem_db.get_string("strategy.method_pointer");
  // In this strategy, the method pointer is optional and some detective work
  // may be required to resolve which method sits on top of a recursion.
  if (method_ptr.empty())
    problem_db.resolve_top_method();
  else
    problem_db.set_db_list_nodes(method_ptr);

  // Instantiate the userDefinedModel
  userDefinedModel = problem_db.get_model();
  // Instantiate the selectedIterator and manage its parallel configurations
  init_iterator(selectedIterator, userDefinedModel);
}


SingleMethodStrategy::~SingleMethodStrategy()
{
  // Virtual destructor handles referenceCount at Strategy level.

  // deallocate communicator partitions for selectedIterator/userDefinedModel
  free_iterator(selectedIterator, userDefinedModel);
}


void SingleMethodStrategy::
run_strategy()
{
  if (worldRank==0) {
    cout << "\n>>>>> Running Single Method Strategy.\n";
    // set up plots and tabular data file
    selectedIterator.initialize_graphics(graph2DFlag, tabularDataFlag,
					 tabularDataFile);
  }

  run_iterator(selectedIterator, userDefinedModel);

  if (worldRank==0)
    cout << "<<<<< Single Method Strategy completed.\n";
}

} // namespace Dakota
