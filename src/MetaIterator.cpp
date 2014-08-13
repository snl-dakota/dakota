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


int MetaIterator::get_min_procs_per_iterator(ProblemDescDB& problem_db)
{
  // Define min_procs_per_iterator to accomodate any lower level overrides.
  // With default_config = PUSH_DOWN, this is less critical.
  int ppa = problem_db.get_int("interface.direct.processors_per_analysis"),
      num_a_serv = problem_db.get_int("interface.analysis_servers"),
      ppe = problem_db.get_int("interface.processors_per_evaluation"),
      num_e_serv = problem_db.get_int("interface.evaluation_servers");
  int min_procs_per_eval = std::max(1, ppa);
  if (num_a_serv)
    min_procs_per_eval *= num_a_serv;
  //if (analysisScheduling == MASTER_SCHEDULING) ++min_procs_per_eval;
  int min_procs_per_iterator = std::max(min_procs_per_eval, ppe);
  if (num_e_serv)
    min_procs_per_iterator *= num_e_serv;
  //if (evalScheduling == MASTER_SCHEDULING) ++min_procs_per_iterator;
  return min_procs_per_iterator;
}


int MetaIterator::
get_max_procs_per_iterator(ProblemDescDB& problem_db, int max_eval_concurrency)
{
  // Define max_procs_per_iterator to estimate maximum processor usage
  // from all lower levels.  With default_config = PUSH_DOWN, this is
  // important to avoid pushing down more resources than can be utilized.
  // The primary input is algorithmic concurrency, but we also incorporate
  // explicit user overrides for _lower_ levels (user overrides for the
  // current level can be managed by resolve_inputs()).

  if (problem_db.get_string("interface.type") == "direct")
    return problem_db.parallel_library().world_size();
  else { // processors_per_analysis = 1 for system/fork
    int max_procs_per_eval, max_procs_per_iterator,
      num_a_serv = problem_db.get_int("interface.analysis_servers"),
      ppe = problem_db.get_int("interface.processors_per_evaluation");

    // compute max_procs_per_eval, incorporating all user overrides
    if (ppe)
      max_procs_per_eval = ppe;
    else {
      int num_drivers
	= problem_db.get_sa("interface.application.analysis_drivers").size();
      short a_sched = problem_db.get_short("interface.analysis_scheduling");
      if (num_a_serv) {
	max_procs_per_eval = num_a_serv;
	int alac = std::max(1, 
	  problem_db.get_int("interface.asynch_local_analysis_concurrency"));
	if (num_a_serv * alac < num_drivers && num_a_serv > 1 &&
	    a_sched != PEER_SCHEDULING) //&& !peer_dynamic_analysis
	  ++max_procs_per_eval;
      }
      else { // assume peer partition unless explicit override to master
	max_procs_per_eval = std::max(1, num_drivers);
	if (a_sched == MASTER_SCHEDULING)
	  ++max_procs_per_eval;
      }
    }

    // compute max_procs_per_iterator (iterator server overrides can be
    // managed by resolve_inputs())
    int num_e_serv = problem_db.get_int("interface.evaluation_servers");
    short e_sched = problem_db.get_short("interface.evaluation_scheduling");
    if (num_e_serv) {
      max_procs_per_iterator = max_procs_per_eval * num_e_serv;
      int alec = std::max(1, 
        problem_db.get_int("interface.asynch_local_evaluation_concurrency"));
      // for peer dynamic, max_procs_per_eval == 1 is imperfect in that it does
      // not capture all possibilities, but this is conservative and hopefully
      // close enough for use in this context (an upper bound estimate).
      bool peer_dynamic_avail
	= (problem_db.get_short("interface.local_evaluation_scheduling") !=
	   STATIC_SCHEDULING && max_procs_per_eval == 1);
      if (num_e_serv * alec < max_eval_concurrency && num_e_serv > 1 &&
	  e_sched != PEER_DYNAMIC_SCHEDULING &&
	  e_sched != PEER_STATIC_SCHEDULING  && !peer_dynamic_avail)
	++max_procs_per_iterator;
    }
    else { // assume peer partition unless explicit override to master
      max_procs_per_iterator = max_procs_per_eval * max_eval_concurrency;
      if (e_sched == MASTER_SCHEDULING)
	++max_procs_per_iterator;
    }

    return max_procs_per_iterator;
  }
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
  int min_ppi = get_min_procs_per_iterator(probDescDB),
      max_ppi = get_max_procs_per_iterator(probDescDB, max_eval_concurrency);

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
  int min_ppi = get_min_procs_per_iterator(probDescDB),
      max_ppi = get_max_procs_per_iterator(probDescDB, max_eval_concurrency);

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
