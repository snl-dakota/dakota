/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       EmbedHybridMetaIterator
//- Description: Implementation code for the EmbedHybridMetaIterator class
//- Owner:       Mike Eldred
//- Checked by:

#include "EmbedHybridMetaIterator.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"

static const char rcsId[]="@(#) $Id: EmbedHybridMetaIterator.cpp 6715 2010-04-02 21:58:15Z wjbohnh $";


namespace Dakota {

EmbedHybridMetaIterator::EmbedHybridMetaIterator(ProblemDescDB& problem_db):
  MetaIterator(problem_db),
  localSearchProb(
    problem_db.get_real("method.hybrid.local_search_probability"))
{
  maxIteratorConcurrency = 1;
  iterSched.init_iterator_parallelism(maxIteratorConcurrency);

  const String& global_ptr
    = problem_db.get_string("method.hybrid.global_method_pointer");
  const String& global_name
    = problem_db.get_string("method.hybrid.global_method_name");
  const String& local_ptr
    = problem_db.get_string("method.hybrid.local_method_pointer");
  const String& local_name
    = problem_db.get_string("method.hybrid.local_method_name");

  if (!global_ptr.empty())
    allocate_by_pointer(global_ptr, globalIterator, globalModel);
  else if (!global_name.empty())
    allocate_by_name(global_name,
		     problem_db.get_string("method.sub_model_pointer"),
		     globalIterator, iteratedModel);

  if (!local_ptr.empty())
    allocate_by_pointer(local_ptr, localIterator, localModel);
  else if (!local_name.empty())
    allocate_by_name(local_name,
		     problem_db.get_string("method.sub_model_pointer"),
		     localIterator, iteratedModel);
}


EmbedHybridMetaIterator::
EmbedHybridMetaIterator(ProblemDescDB& problem_db, Model& model):
  MetaIterator(problem_db, model),
  localSearchProb(
    problem_db.get_real("method.hybrid.local_search_probability"))
{
  // Hard-wired to lightweight methodList instantiation for now.  To support
  // a more general case indicated by the sequential hybrid spec, will need
  // to validate iteratedModel against any model pointers (--> warnings, see
  // SurrBasedLocalMinimizer for example).

  maxIteratorConcurrency = 1;
  iterSched.init_iterator_parallelism(maxIteratorConcurrency);

  String empty_model_ptr; // for now
  allocate_by_name(problem_db.get_string("method.hybrid.global_method_name"),
		   empty_model_ptr, globalIterator, iteratedModel);
  allocate_by_name(problem_db.get_string("method.hybrid.local_method_name"),
		   empty_model_ptr, localIterator,  iteratedModel);
}


EmbedHybridMetaIterator::~EmbedHybridMetaIterator()
{
  // Virtual destructor handles referenceCount at Iterator level.

  if (globalModel.is_null()) deallocate(globalIterator, iteratedModel);
  else                       deallocate(globalIterator, globalModel);
  if (localModel.is_null())  deallocate(localIterator,  iteratedModel);
  else                       deallocate(localIterator,  localModel);
}


void EmbedHybridMetaIterator::core_run()
{
  if (iterSched.lead_rank()) {
    Cout << "\n>>>>> Running Embedded Hybrid Minimizer with global method = "
         << globalIterator.method_string() << " and local method = "
	 <<  localIterator.method_string() << std::endl;
    //globalIterator.set_local_search(localIterator);
    //globalIterator.set_local_search_probability(localSearchProb);
  }

  iterSched.schedule_iterators(*this, globalIterator);
}

} // namespace Dakota
