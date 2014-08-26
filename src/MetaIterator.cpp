/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       MetaIterator
//- Description: Implementation code for the MetaIterator class
//- Owner:       Mike Eldred
//- Checked by:

#include "MetaIterator.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"

static const char rcsId[]="@(#) $Id: MetaIterator.cpp 6715 2010-04-02 21:58:15Z wjbohnh $";


namespace Dakota {

MetaIterator::MetaIterator(ProblemDescDB& problem_db):
  Iterator(BaseConstructor(), problem_db),
  iterSched(problem_db.parallel_library(),
	    problem_db.get_int("method.iterator_servers"),
	    problem_db.get_int("method.processors_per_iterator"),
	    problem_db.get_short("method.iterator_scheduling"))
{ }


MetaIterator::MetaIterator(ProblemDescDB& problem_db, Model& model):
  Iterator(BaseConstructor(), problem_db),
  iterSched(problem_db.parallel_library(),
	    problem_db.get_int("method.iterator_servers"),
	    problem_db.get_int("method.processors_per_iterator"),
	    problem_db.get_short("method.iterator_scheduling"))
{
  iteratedModel = model;
  //update_from_model(iteratedModel);
}


MetaIterator::~MetaIterator()
{
  // deallocate the mi_pl parallelism level
  iterSched.free_iterator_parallelism();
}


void MetaIterator::
allocate_by_pointer(const String& method_ptr, Iterator& the_iterator,
		    Model& the_model)
{
  // set_db_list_nodes() sets all the nodes w/i the linked lists to the
  // appropriate Variables, Interface, and Response specs (as governed
  // by the pointer strings in the method specification).
  size_t method_index = probDescDB.get_db_method_node(); // for restoration
  probDescDB.set_db_list_nodes(method_ptr);
  if (the_model.is_null())
    the_model = probDescDB.get_model();
  iterSched.init_iterator(probDescDB, the_iterator, the_model,
    probDescDB.parallel_library().parallel_configuration().mi_parallel_level());
  probDescDB.set_db_list_nodes(method_index);            // restore
}


void MetaIterator::
allocate_by_name(const String& method_string, const String& model_ptr,
		 Iterator& the_iterator, Model& the_model)
{
  // model instantiation is DB-based, iterator instantiation is not
  bool set = !model_ptr.empty(); size_t model_index;
  if (set) {// && the_model.is_null())
    model_index = probDescDB.get_db_model_node(); // for restoration
    probDescDB.set_db_model_nodes(model_ptr);
  }
  if (the_model.is_null())
    the_model = probDescDB.get_model();
  iterSched.init_iterator(method_string, the_iterator, the_model,
    probDescDB.parallel_library().parallel_configuration().mi_parallel_level());
  if (set)
    probDescDB.set_db_model_nodes(model_index);   // restore
}


std::pair<int, int> MetaIterator::
estimate_by_pointer(const String& method_ptr, Iterator& the_iterator,
		    Model& the_model)
{
  // set_db_list_nodes() sets all the nodes w/i the linked lists to the
  // appropriate Variables, Interface, and Response specs (as governed
  // by the pointer strings in the method specification).
  size_t method_index = probDescDB.get_db_method_node(); // for restoration
  probDescDB.set_db_list_nodes(method_ptr);

  if (the_model.is_null())
    the_model = probDescDB.get_model();

  const ParallelConfiguration& pc
    = probDescDB.parallel_library().parallel_configuration();
  int max_eval_concurrency
    = iterSched.init_evaluation_concurrency(probDescDB, the_iterator,
					    the_model, pc.w_parallel_level());

  // needs to follow set_db_list_nodes
  int min_ppi = probDescDB.get_min_procs_per_iterator(),
      max_ppi = probDescDB.get_max_procs_per_iterator(max_eval_concurrency);

  probDescDB.set_db_list_nodes(method_index);            // restore
  return std::pair<int, int>(min_ppi, max_ppi);
}


std::pair<int, int> MetaIterator::
estimate_by_name(const String& method_string, const String& model_ptr,
		 Iterator& the_iterator, Model& the_model)
{
  // model instantiation is DB-based, iterator instantiation is not
  bool set = !model_ptr.empty(); size_t model_index;
  if (set) {// && the_model.is_null())
    model_index = probDescDB.get_db_model_node(); // for restoration
    probDescDB.set_db_model_nodes(model_ptr);
  }

  if (the_model.is_null())
    the_model = probDescDB.get_model();

  const ParallelConfiguration& pc
    = probDescDB.parallel_library().parallel_configuration();
  int max_eval_concurrency
    = iterSched.init_evaluation_concurrency(method_string, the_iterator,
					    the_model, pc.w_parallel_level());

  // needs to follow set_db_list_nodes
  int min_ppi = probDescDB.get_min_procs_per_iterator(),
      max_ppi = probDescDB.get_max_procs_per_iterator(max_eval_concurrency);

  if (set)
    probDescDB.set_db_model_nodes(model_index);   // restore
  return std::pair<int, int>(min_ppi, max_ppi);
}


void MetaIterator::deallocate(Iterator& the_iterator, Model& the_model)
{
  iterSched.free_iterator(the_iterator,
    probDescDB.parallel_library().parallel_configuration().mi_parallel_level());
}


void MetaIterator::post_run(std::ostream& s)
{
  if (iterSched.lead_rank()) 
    print_results(s);
}

} // namespace Dakota
