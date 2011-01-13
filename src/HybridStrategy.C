/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       HybridStrategy
//- Description: Implementation code for the HybridStrategy class
//- Owner:       Mike Eldred
//- Checked by:

#include "HybridStrategy.H"
#include "ProblemDescDB.H"

static const char rcsId[]="@(#) $Id: HybridStrategy.C 6715 2010-04-02 21:58:15Z wjbohnh $";


namespace Dakota {

HybridStrategy::HybridStrategy(ProblemDescDB& problem_db):
  Strategy(BaseConstructor(), problem_db)
{ }


HybridStrategy::~HybridStrategy()
{ }


void HybridStrategy::allocate_methods()
{
  // Instantiate iterator and model arrays:

  // default constructors initialize repPointers to NULL (object is not built)
  userDefinedModels.resize(numIterators);
  selectedIterators.resize(numIterators); // slaves also need for run_iterator

  // Instantiate all Models and Iterators.
  // set_db_list_nodes() sets all the nodes w/i the linked lists to the
  // appropriate Variables, Interface, and Response specs (as governed
  // by the pointer strings in the method specification).
  for (size_t i=0; i<numIterators; ++i) {
    if (worldRank == 0)
      Cout << "calling set_db_list_nodes with " << methodList[i] << std::endl;
    probDescDB.set_db_list_nodes(methodList[i]);

    // Instantiate the i-th userDefinedModel and selectedIterator
    userDefinedModels[i] = probDescDB.get_model();
    if (!stratIterDedMaster || worldRank)
      init_iterator(selectedIterators[i], userDefinedModels[i]);
  }
}


void HybridStrategy::deallocate_methods()
{
  if (!stratIterDedMaster || worldRank)
    for (size_t i=0; i<numIterators; ++i)
      free_iterator(selectedIterators[i], userDefinedModels[i]);
}

} // namespace Dakota
