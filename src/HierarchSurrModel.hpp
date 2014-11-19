/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       HierarchSurrModel
//- Description: A model which manages a surrogate relationship between a low
//-              fidelity approximate model and a high fidelity truth model.
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: HierarchSurrModel.hpp 7024 2010-10-16 01:24:42Z mseldre $

#ifndef HIERARCH_SURR_MODEL_H
#define HIERARCH_SURR_MODEL_H

#include "SurrogateModel.hpp"
#include "ParallelLibrary.hpp"
#include "DataModel.hpp"


namespace Dakota {

// define special values for componentParallelMode
#define LF_MODEL 1
#define HF_MODEL 2


/// Derived model class within the surrogate model branch for managing
/// hierarchical surrogates (models of varying fidelity).

/** The HierarchSurrModel class manages hierarchical models of varying
    fidelity.  In particular, it uses a low fidelity model as a
    surrogate for a high fidelity model.  The class contains a
    lowFidelityModel which performs the approximate low fidelity
    function evaluations and a highFidelityModel which provides truth
    evaluations for computing corrections to the low fidelity results. */

class HierarchSurrModel: public SurrogateModel
{
public:

  //
  //- Heading: Constructors and destructor
  //

  HierarchSurrModel(ProblemDescDB& problem_db); ///< constructor
  ~HierarchSurrModel();                         ///< destructor

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  // Perform the response computation portions specific to this derived 
  // class.  In this case, it simply applies a correction to  
  // lowFidelityModel.compute_response()/synchronize()/synchronize_nowait()
  //
  /// portion of compute_response() specific to HierarchSurrModel
  void derived_compute_response(const ActiveSet& set);
  /// portion of asynch_compute_response() specific to HierarchSurrModel
  void derived_asynch_compute_response(const ActiveSet& set);
  /// portion of synchronize() specific to HierarchSurrModel
  const IntResponseMap& derived_synchronize();
  /// portion of synchronize_nowait() specific to HierarchSurrModel
  const IntResponseMap& derived_synchronize_nowait();

  /// return lowFidelityModel
  Model& surrogate_model();
  /// return highFidelityModel
  Model& truth_model();
  /// return lowFidelityModel and highFidelityModel
  void derived_subordinate_models(ModelList& ml, bool recurse_flag);

  /// set the relative weightings for multiple objective functions or least
  /// squares terms and optionally recurses into LF/HF models
  void primary_response_fn_weights(const RealVector& wts,
				   bool recurse_flag = true);

  /// set responseMode and pass any bypass request on to highFidelityModel
  /// for any lower-level surrogate recursions.
  void surrogate_response_mode(short mode);

  /// (re)set the surrogate index set in SurrogateModel::surrogateFnIndices
  void surrogate_function_indices(const IntSet& surr_fn_indices);

  /// use highFidelityModel to compute the truth values needed for
  /// correction of lowFidelityModel results
  void build_approximation();
  // Uses the c_vars/response anchor point to define highFidResponse
  //bool build_approximation(const RealVector& c_vars,const Response& response);

  /// update component parallel mode for supporting parallelism in
  /// lowFidelityModel and highFidelityModel
  void component_parallel_mode(short mode);

  IntIntPair estimate_partition_bounds(int max_eval_concurrency);

  /// set up lowFidelityModel and highFidelityModel for parallel operations
  void derived_init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				  bool recurse_flag = true);
  /// set up lowFidelityModel and highFidelityModel for serial operations.
  void derived_init_serial();
  /// set active parallel configuration within lowFidelityModel and
  /// highFidelityModel
  void derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				 bool recurse_flag = true);
  /// deallocate communicator partitions for the HierarchSurrModel
  /// (request forwarded to lowFidelityModel and highFidelityModel)
  void derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
				  bool recurse_flag = true);

  /// Service lowFidelityModel and highFidelityModel job requests received
  /// from the master.  Completes when a termination message is received from
  /// stop_servers().
  void serve_run(ParLevLIter pl_iter, int max_eval_concurrency);
  /// Executed by the master to terminate lowFidelityModel and
  /// highFidelityModel server operations when iteration on the
  /// HierarchSurrModel is complete.
  void stop_servers();

  /// update the Model's inactive view based on higher level (nested)
  /// context and optionally recurse into 
  void inactive_view(short view, bool recurse_flag = true);

  /// Return the current evaluation id for the HierarchSurrModel
  int evaluation_id() const;

  /// set the evaluation counter reference points for the HierarchSurrModel
  /// (request forwarded to lowFidelityModel and highFidelityModel)
  void set_evaluation_reference();
  /// request fine-grained evaluation reporting within lowFidelityModel
  /// and highFidelityModel
  void fine_grained_evaluation_counters();
  /// print the evaluation summary for the HierarchSurrModel
  /// (request forwarded to lowFidelityModel and highFidelityModel)
  void print_evaluation_summary(std::ostream& s, bool minimal_header = false,
				bool relative_count = true) const;

  /// set the hierarchical eval ID tag prefix
  void eval_tag_prefix(const String& eval_id_str);

private:

  //
  //- Heading: Convenience functions
  //

  /// update the incoming model (lowFidelityModel or highFidelityModel)
  /// with current variable values/bounds/labels
  void update_model(Model& model);

  //
  //- Heading: Data members
  //

  /// number of calls to derived_compute_response()/
  /// derived_asynch_compute_response()
  int hierModelEvalCntr;

  /// map of high-fidelity responses retrieved in derived_synchronize_nowait()
  /// that could not be returned since corresponding low-fidelity response
  /// portions were still pending.
  IntResponseMap cachedTruthRespMap;

  /// provides approximate low fidelity function evaluations.  Model is of
  /// arbitrary type and supports recursions (e.g., lowFidelityModel can be
  /// a data fit surrogate on a low fidelity model).
  Model lowFidelityModel;

  /// provides truth evaluations for computing corrections to the low
  /// fidelity results.  Model is of arbitrary type and supports recursions.
  Model highFidelityModel;

  /// the reference high fidelity response computed in build_approximation()
  /// and used for calculating corrections.
  Response highFidRefResponse;

  /// cached evalTag Prefix from parents to use at compute_response time
  String evalTagPrefix;
};


inline HierarchSurrModel::~HierarchSurrModel()
{ } // Virtual destructor handles referenceCount at Strategy level.


inline Model& HierarchSurrModel::surrogate_model()
{ return lowFidelityModel; }


inline Model& HierarchSurrModel::truth_model()
{ return highFidelityModel; }


inline void HierarchSurrModel::
derived_subordinate_models(ModelList& ml, bool recurse_flag)
{
  ml.push_back(lowFidelityModel);
  if (recurse_flag)
    lowFidelityModel.derived_subordinate_models(ml, true);
  ml.push_back(highFidelityModel);
  if (recurse_flag)
    highFidelityModel.derived_subordinate_models(ml, true);
}


inline void HierarchSurrModel::
primary_response_fn_weights(const RealVector& wts, bool recurse_flag)
{
  primaryRespFnWts = wts;
  if (recurse_flag) {
    lowFidelityModel.primary_response_fn_weights(wts,  recurse_flag);
    highFidelityModel.primary_response_fn_weights(wts, recurse_flag);
  }
}


inline void HierarchSurrModel::surrogate_response_mode(short mode)
{
  responseMode = mode;
  // don't pass to lowFidelityModel (in case it includes surrogates) since the
  // point of a surrogate bypass is to get a surrogate-free truth evaluation
  if (mode == BYPASS_SURROGATE) // recurse in this case
    highFidelityModel.surrogate_response_mode(mode);
}


inline void HierarchSurrModel::
surrogate_function_indices(const IntSet& surr_fn_indices)
{ surrogateFnIndices = surr_fn_indices; }


inline IntIntPair HierarchSurrModel::
estimate_partition_bounds(int max_eval_concurrency)
{
  // responseMode is a run-time setting, so we are conservative on usage of
  // max_eval_concurrency as in derived_init_communicators()

  probDescDB.set_db_model_nodes(
    probDescDB.get_string("model.surrogate.low_fidelity_model_pointer"));
  IntIntPair lf_min_max
    = lowFidelityModel.estimate_partition_bounds(max_eval_concurrency);

  probDescDB.set_db_model_nodes(
    probDescDB.get_string("model.surrogate.high_fidelity_model_pointer"));
  IntIntPair hf_min_max
    = highFidelityModel.estimate_partition_bounds(max_eval_concurrency);

  return IntIntPair(std::min(lf_min_max.first,  hf_min_max.first),
		    std::max(lf_min_max.second, hf_min_max.second));

  // list nodes are reset at the calling level after completion of recursion
}


inline void HierarchSurrModel::derived_init_serial()
{
  lowFidelityModel.init_serial();
  highFidelityModel.init_serial();
}


inline void HierarchSurrModel::stop_servers()
{ component_parallel_mode(0); }


inline void HierarchSurrModel::inactive_view(short view, bool recurse_flag)
{
  currentVariables.inactive_view(view);
  userDefinedConstraints.inactive_view(view);
  if (recurse_flag) {
    lowFidelityModel.inactive_view(view,  recurse_flag);
    highFidelityModel.inactive_view(view, recurse_flag);
  }
}


/** return the hierarchical model evaluation count.  Due to possibly
    intermittent use of surrogate bypass, this is not the same as
    either the loFi or hiFi model evaluation counts.  It also does not
    distinguish duplicate evals. */
inline int HierarchSurrModel::evaluation_id() const
{ return hierModelEvalCntr; }


inline void HierarchSurrModel::set_evaluation_reference()
{
  //lowFidelityModel.set_evaluation_reference();

  // don't recurse this, since the eval reference is for the top level iteration
  //if (responseMode == BYPASS_SURROGATE)
  //highFidelityModel.set_evaluation_reference();

  // may want to add this in time
  //hierModelEvalRef = hierModelEvalCntr;
}


inline void HierarchSurrModel::fine_grained_evaluation_counters()
{
  lowFidelityModel.fine_grained_evaluation_counters();
  highFidelityModel.fine_grained_evaluation_counters();
}


inline void HierarchSurrModel::
print_evaluation_summary(std::ostream& s, bool minimal_header,
			 bool relative_count) const
{
  lowFidelityModel.print_evaluation_summary(s, minimal_header, relative_count);
  highFidelityModel.print_evaluation_summary(s, minimal_header, relative_count);
}

} // namespace Dakota

#endif
