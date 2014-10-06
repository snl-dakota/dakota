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
{
  if (!numFinalSolutions)  // default is zero
    numFinalSolutions = 1; // for now...  (TO DO: hybrids, concurrent)
}


MetaIterator::MetaIterator(ProblemDescDB& problem_db, Model& model):
  Iterator(BaseConstructor(), problem_db),
  iterSched(problem_db.parallel_library(),
	    problem_db.get_int("method.iterator_servers"),
	    problem_db.get_int("method.processors_per_iterator"),
	    problem_db.get_short("method.iterator_scheduling"))
{
  iteratedModel = model;
  //update_from_model(iteratedModel);

  if (!numFinalSolutions)  // default is zero
    numFinalSolutions = 1; // for now...  (TO DO: hybrids, concurrent)
}


MetaIterator::~MetaIterator()
{ }


bool MetaIterator::
new_model(const String& method_ptr, const String& model_ptr)
{
  // if an existing model was passed in through ctor, return false
  if (!iteratedModel.is_null())
    return false;

  bool new_flag = false;
  if (!method_ptr.empty()) {
    size_t restore_index = probDescDB.get_db_method_node(); // for restoration
    probDescDB.set_db_method_node(method_ptr);
    if (!probDescDB.get_string("method.model_pointer").empty())
      new_flag = true;
    probDescDB.set_db_method_node(restore_index);           // restore
  }
  else if (!model_ptr.empty())
    new_flag = true;

  return new_flag;
}


void MetaIterator::
check_model(const String& method_ptr, const String& model_ptr)
{
  bool warn_flag = false;
  if (!method_ptr.empty()) {
    size_t restore_index = probDescDB.get_db_method_node(); // for restoration
    probDescDB.set_db_method_node(method_ptr);
    if (probDescDB.get_string("method.model_pointer") !=
	iteratedModel.model_id())
      warn_flag = true;
    probDescDB.set_db_method_node(restore_index);           // restore
  }
  else if (!model_ptr.empty() && model_ptr != iteratedModel.model_id())
    warn_flag = true;

  if (warn_flag)
    Cerr << "Warning: concurrent meta-iterator specification includes an "
	 << "inconsistent\n          model_pointer.  Sub-iterator database "
	 << "initialization could be inconsistent\n          with passed Model."
	 << std::endl;
}


void MetaIterator::
allocate_by_pointer(const String& method_ptr, Iterator& the_iterator,
		    Model& the_model)
{
  // due to the possibility of Model recursion, store/restore the method/model
  // indices separately
  size_t method_index = probDescDB.get_db_method_node(),
         model_index  = probDescDB.get_db_model_node(); // for restoration
  probDescDB.set_db_list_nodes(method_ptr);
  if (the_model.is_null())
    the_model = probDescDB.get_model();
  iterSched.init_iterator(probDescDB, the_iterator, the_model);
  probDescDB.set_db_method_node(method_index);          // restore
  probDescDB.set_db_model_nodes(model_index);           // restore
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
  iterSched.init_iterator(method_string, the_iterator, the_model);
  if (set)
    probDescDB.set_db_model_nodes(model_index);   // restore
}


std::pair<int, int> MetaIterator::
estimate_by_pointer(const String& method_ptr, Iterator& the_iterator,
		    Model& the_model)
{
  // due to the possibility of Model recursion, store/restore the method/model
  // indices separately
  size_t method_index = probDescDB.get_db_method_node(),
         model_index  = probDescDB.get_db_model_node(); // for restoration
  probDescDB.set_db_list_nodes(method_ptr);

  if (the_model.is_null())
    the_model = probDescDB.get_model();

  int max_eval_concurrency =
    iterSched.init_evaluation_concurrency(probDescDB, the_iterator, the_model);

  // needs to follow set_db_list_nodes
  int min_ppi = probDescDB.get_min_procs_per_iterator(),
      max_ppi = probDescDB.get_max_procs_per_iterator(max_eval_concurrency);

  probDescDB.set_db_method_node(method_index);          // restore
  probDescDB.set_db_model_nodes(model_index);           // restore
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

  int max_eval_concurrency
    = iterSched.init_evaluation_concurrency(method_string, the_iterator,
					    the_model);

  // needs to follow set_db_list_nodes
  int min_ppi = probDescDB.get_min_procs_per_iterator(),
      max_ppi = probDescDB.get_max_procs_per_iterator(max_eval_concurrency);

  if (set)
    probDescDB.set_db_model_nodes(model_index);   // restore
  return std::pair<int, int>(min_ppi, max_ppi);
}


void MetaIterator::post_run(std::ostream& s)
{
  if (iterSched.lead_rank()) 
    print_results(s);
}

} // namespace Dakota
