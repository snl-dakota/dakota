/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef ADAPTER_MODEL_H
#define ADAPTER_MODEL_H

#include "DakotaModel.hpp"

namespace Dakota {


/// Derived model class which wraps call-back functions for solving
/// minimization sub-problems.
  
/** The AdapterModel class uses C-style function pointers to:
    (a) allow use of existing Iterator constructor APIs that utilize
    an incoming Model to extract sub-problem data, and (b) enable Model
    recursions on top of these call-backs. */

class AdapterModel: public Model
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// standard full constructor with minimizer-specific bounds/targets;
  /// doubles as a partial constructor given default value for response
  /// mapping function pointer
  AdapterModel(const Variables& initial_pt, const Constraints& cons,
	       const Response& resp,
	       void (*resp_map) (const Variables& vars, const ActiveSet& set,
				 Response& response) = NULL);

  /// alternate partial constructor; constructs response map but
  /// requires subsequent init_minimizer_data() call
  AdapterModel(void (*resp_map) (const Variables& vars, const ActiveSet& set,
				 Response& response));

  /// destructor
  ~AdapterModel();

  //
  //- Heading: Member functions
  //

  /// initialize map callbacks after alternate construction
  void initialize_response_map(void (*resp_map) (const Variables& vars,
						 const ActiveSet& set,
						 Response& response));

  /*
  /// perform transformation of Response (sub-model --> recast)
  void transform_response(const Variables& recast_vars,
			  const Variables& sub_model_vars,
			  const Response& sub_model_resp,
			  Response& recast_resp);
  /// invoke transform_response() on each response within old_resp_map
  /// to create new_resp_map
  void transform_response_map(const IntResponseMap& old_resp_map,
			      IntResponseMap& new_resp_map);

  ActiveSet default_active_set();
  void declare_sources();
  */

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  /// portion of evaluate() specific to AdapterModel
  void derived_evaluate(const ActiveSet& set);
  /// portion of evaluate_nowait() specific to AdapterModel
  void derived_evaluate_nowait(const ActiveSet& set);
  /// portion of synchronize() specific to AdapterModel
  const IntResponseMap& derived_synchronize();
  /// portion of synchronize_nowait() specific to AdapterModel
  const IntResponseMap& derived_synchronize_nowait();

  /// return the current evaluation id for the AdapterModel
  int derived_evaluation_id() const;

  /*
  bool initialize_mapping(ParLevLIter pl_iter);
  bool finalize_mapping();

  /// AdapterModel only supports parallelism in subModel, so this
  /// virtual function redefinition is simply a sanity check.
  void component_parallel_mode(short mode);

  /// return subModel's MI parallel level index
  size_t mi_parallel_level_index() const;

  /// return subModel local synchronization setting
  short local_eval_synchronization();
  /// return subModel local evaluation concurrency
  int local_eval_concurrency();
  /// flag which prevents overloading the master with a multiprocessor
  /// evaluation (request forwarded to subModel)
  bool derived_master_overload() const;

  /// set up AdapterModel for parallel operations
  void derived_init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				  bool recurse_flag = true);
  /// set up AdapterModel for serial operations
  void derived_init_serial();
  /// set active parallel configuration within subModel
  void derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				 bool recurse_flag = true);
  /// deallocate communicator partitions for the AdapterModel
  void derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				  bool recurse_flag = true);

  /// Service subModel job requests received from the master.
  /// Completes when a termination message is received from stop_servers().
  void serve_run(ParLevLIter pl_iter, int max_eval_concurrency);
  /// executed by the master to terminate subModel server operations
  /// when AdapterModel iteration is complete.
  void stop_servers();

  /// print the evaluation summary for the AdapterModel (request
  /// forwarded to subModel)
  void print_evaluation_summary(std::ostream& s, bool minimal_header = false,
				bool relative_count = true) const;

  /// set the hierarchical eval ID tag prefix
  void eval_tag_prefix(const String& eval_id_str);

  //
  //- Heading: Member functions
  //

  /// assign static pointer instance to this for use in static
  /// transformation functions
  void assign_instance();

  /// initialize currentVariables and related info from the passed
  /// size/type info
  bool init_variables(const ShortShortPair& recast_vars_view,
		      const SizetArray& vars_comps_totals,
		      const BitArray& all_relax_di, 
		      const BitArray& all_relax_dr);
  /// initialize currentResponse from the passed size info
  void init_response(size_t num_recast_primary_fns, 
		     size_t num_recast_secondary_fns, 
		     short recast_resp_order, bool reshape_vars);
  /// initialize userDefinedConstraints from the passed size info
  void init_constraints(size_t num_recast_secondary_fns,
			size_t recast_secondary_offset, bool reshape_vars);
  */

  //
  //- Heading: Data members
  //

  /// local evaluation id counter used for id mapping
  int adapterModelEvalCntr;

  /// map of variables used by derived_evaluate_nowait().
  /// Caches values needed for evaluation in synchronization routines.
  IntVariablesMap adapterVarsMap;
  /// map of active set passed to derived_evaluate_nowait().
  /// Caches values needed for evaluation in synchronization routines.
  IntActiveSetMap adapterSetMap;
  /// map of responses returned by derived_synchronize() and
  /// derived_synchronize_nowait()
  IntResponseMap adapterRespMap;

private:

  //
  //- Heading: Convenience member functions
  //

  //
  //- Heading: Data members
  //

  /// holds pointer for primary response mapping function passed in
  /// ctor/initialize
  void (*respMapping) (const Variables& vars, const ActiveSet& set,
		       Response& response);
};


inline AdapterModel::~AdapterModel()
{ }


inline void AdapterModel::
initialize_response_map(void (*resp_map) (const Variables& vars,
					  const ActiveSet& set,
					  Response& response))
{ respMapping = resp_map; } // includes NULL default


inline int AdapterModel::derived_evaluation_id() const
{ return adapterModelEvalCntr; }


/*
inline bool AdapterModel::initialize_mapping(ParLevLIter pl_iter)
{
  Model::initialize_mapping(pl_iter);

  bool sub_model_resize = subModel.initialize_mapping(pl_iter);

  // update message lengths for send/receive of parallel jobs (normally
  // performed once in Model::init_communicators() just after construct time)
  if (sub_model_resize)
    estimate_message_lengths();

  return sub_model_resize;
}


inline bool AdapterModel::finalize_mapping()
{
  bool sub_model_resize = subModel.finalize_mapping();
  Model::finalize_mapping();
  return sub_model_resize;
}


inline size_t AdapterModel::qoi() const
{ return subModel.qoi(); }


inline void AdapterModel::active_model_key(const Pecos::ActiveKey& key)
{ subModel.active_model_key(key); }


inline const Pecos::ActiveKey& AdapterModel::active_model_key() const
{ return subModel.active_model_key(); }


inline void AdapterModel::clear_model_keys()
{ subModel.clear_model_keys(); }


inline Interface& AdapterModel::derived_interface()
{ return subModel.derived_interface(); }


inline void AdapterModel::component_parallel_mode(short mode)
{
  //if (mode != SUB_MODEL_MODE) {
  //  Cerr << "Error: AdapterModel only supports the SUB_MODEL_MODE "
  //       << "component parallel mode." << std::endl;
  //  abort_handler(-1);
  //}

  // Since we don't want the thin recast wrapper interfering with subModel
  // parallelism, we forward the parallel mode to the subModel.  This differs
  // from all other derived Model implementations (which utilize the mode
  // locally and do not forward it).
  subModel.component_parallel_mode(mode);
}


inline size_t AdapterModel::mi_parallel_level_index() const
{ return subModel.mi_parallel_level_index(); }


inline short AdapterModel::local_eval_synchronization()
{ return subModel.local_eval_synchronization(); }


inline int AdapterModel::local_eval_concurrency()
{ return subModel.local_eval_concurrency(); }


inline bool AdapterModel::derived_master_overload() const
{ return subModel.derived_master_overload(); }


inline void AdapterModel::
derived_init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			   bool recurse_flag)
{
  if (recurse_flag)
    subModel.init_communicators(pl_iter, max_eval_concurrency);
}


inline void AdapterModel::derived_init_serial()
{ subModel.init_serial(); }


inline void AdapterModel::
derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			  bool recurse_flag)
{
  if (recurse_flag) {
    subModel.set_communicators(pl_iter, max_eval_concurrency);

    // AdapterModels do not utilize default set_ie_asynchronous_mode()
    // as they do not define the ie_parallel_level
    asynchEvalFlag     = subModel.asynch_flag();
    evaluationCapacity = subModel.evaluation_capacity();
  }
}


inline void AdapterModel::
derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
			   bool recurse_flag)
{
  if (recurse_flag)
    subModel.free_communicators(pl_iter, max_eval_concurrency);
}


inline void AdapterModel::
serve_run(ParLevLIter pl_iter, int max_eval_concurrency)
{
  // don't recurse, as subModel.serve() will set subModel comms
  set_communicators(pl_iter, max_eval_concurrency, false);

  subModel.serve_run(pl_iter, max_eval_concurrency); // sets subModel comms
}


inline void AdapterModel::stop_servers()
{ subModel.stop_servers(); }


inline const String& AdapterModel::interface_id() const
{ return subModel.interface_id(); }


inline void AdapterModel::
print_evaluation_summary(std::ostream& s, bool minimal_header,
			 bool relative_count) const
{ subModel.print_evaluation_summary(s, minimal_header, relative_count); }


inline void AdapterModel::eval_tag_prefix(const String& eval_id_str)
{ subModel.eval_tag_prefix(eval_id_str); }
*/

} // namespace Dakota

#endif
