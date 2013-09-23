/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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

  /// set up lowFidelityModel and highFidelityModel for parallel operations
  void derived_init_communicators(int max_iterator_concurrency,
				  bool recurse_flag = true);
  /// set up lowFidelityModel and highFidelityModel for serial operations.
  void derived_init_serial();
  /// set active parallel configuration within lowFidelityModel and
  /// highFidelityModel
  void derived_set_communicators(int max_iterator_concurrency,
				 bool recurse_flag = true);
  /// deallocate communicator partitions for the HierarchSurrModel
  /// (request forwarded to lowFidelityModel and highFidelityModel)
  void derived_free_communicators(int max_iterator_concurrency,
				  bool recurse_flag = true);

  /// Service lowFidelityModel and highFidelityModel job requests received
  /// from the master.  Completes when a termination message is received from
  /// stop_servers().
  void serve();
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
  virtual void eval_tag_prefix(const String& eval_id_str);

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


inline void HierarchSurrModel::
derived_init_communicators(int max_iterator_concurrency, bool recurse_flag)
{
  // responseMode is a run-time setting (in SBLMinimizer, it is switched among
  // AUTO_CORRECTED_SURROGATE, BYPASS_SURROGATE, and UNCORRECTED_SURROGATE;
  // in NonDExpansion, it is switching between MODEL_DISCREPANCY and
  // UNCORRECTED_SURROGATE).  Since it is neither static nor generally
  // available at construct/init time, take a conservative approach with init
  // and free and a more aggressive approach with set.

  if (recurse_flag) {
    // superset of possible init calls (two configurations for HF)
    lowFidelityModel.init_communicators(max_iterator_concurrency);
    highFidelityModel.init_communicators(
      highFidelityModel.derivative_concurrency());
    highFidelityModel.init_communicators(max_iterator_concurrency);

    /*
    switch (responseMode) {
    case UNCORRECTED_SURROGATE:
      // LF are used in iterator evals
      lowFidelityModel.init_communicators(max_iterator_concurrency);
      break;
    case AUTO_CORRECTED_SURROGATE:
      // LF are used in iterator evals
      lowFidelityModel.init_communicators(max_iterator_concurrency);
      // HF evals are for correction and validation:
      // concurrency = one eval at a time * derivative concurrency per eval
      highFidelityModel.init_communicators(
	highFidelityModel.derivative_concurrency());
      break;
    case BYPASS_SURROGATE:
      // HF are used in iterator evals
      highFidelityModel.init_communicators(max_iterator_concurrency);
      break;
    case MODEL_DISCREPANCY:
      // LF and HF are used in iterator evals
      lowFidelityModel.init_communicators(max_iterator_concurrency);
      highFidelityModel.init_communicators(max_iterator_concurrency);
      break;
    }
    */
  }
}


inline void HierarchSurrModel::
derived_set_communicators(int max_iterator_concurrency, bool recurse_flag)
{
  // asynchEvalFlag & asynchEvalFlag assignments are not overridden in
  // Model::set_communicators() since HierarchSurrModels do not define
  // the ie_parallel_level

  // This aggressive logic is appropriate for invocations of the Model via
  // Iterator::run(), but is fragile w.r.t. invocations of the Model outside
  // this scope (e.g., Model::compute_response() within SBLMinimizer).  The
  // default responseMode value is AUTO_CORRECTED_SURROGATE, which mitigates
  // the specific case of SBLMinimizer, but the general fragility remains.
  if (recurse_flag) {
    switch (responseMode) {
    case UNCORRECTED_SURROGATE:
      lowFidelityModel.set_communicators(max_iterator_concurrency);
      asynchEvalFlag     = lowFidelityModel.asynch_flag();
      evaluationCapacity = lowFidelityModel.evaluation_capacity();
      break;
    case AUTO_CORRECTED_SURROGATE: {
      lowFidelityModel.set_communicators(max_iterator_concurrency);
      int hf_deriv_conc = highFidelityModel.derivative_concurrency();
      highFidelityModel.set_communicators(hf_deriv_conc);
      asynchEvalFlag = ( lowFidelityModel.asynch_flag() ||
	( hf_deriv_conc > 1 && highFidelityModel.asynch_flag() ) );
      evaluationCapacity = std::max( lowFidelityModel.evaluation_capacity(),
				     highFidelityModel.evaluation_capacity() );
      break;
    }
    case BYPASS_SURROGATE:
      highFidelityModel.set_communicators(max_iterator_concurrency);
      asynchEvalFlag     = highFidelityModel.asynch_flag();
      evaluationCapacity = highFidelityModel.evaluation_capacity();
      break;
    case MODEL_DISCREPANCY:
      lowFidelityModel.set_communicators(max_iterator_concurrency);
      highFidelityModel.set_communicators(max_iterator_concurrency);
      asynchEvalFlag = ( lowFidelityModel.asynch_flag() ||
			 highFidelityModel.asynch_flag() );
      evaluationCapacity = std::max( lowFidelityModel.evaluation_capacity(),
				     highFidelityModel.evaluation_capacity() );
      break;
    }
  }
}


inline void HierarchSurrModel::derived_init_serial()
{
  lowFidelityModel.init_serial();
  highFidelityModel.init_serial();
}


inline void HierarchSurrModel::
derived_free_communicators(int max_iterator_concurrency, bool recurse_flag)
{
  if (recurse_flag) {
    // superset of possible free calls (two configurations for HF)
    lowFidelityModel.free_communicators(max_iterator_concurrency);
    highFidelityModel.free_communicators(
      highFidelityModel.derivative_concurrency());
    highFidelityModel.free_communicators(max_iterator_concurrency);

    /*
    switch (responseMode) {
    case UNCORRECTED_SURROGATE:
      lowFidelityModel.free_communicators(max_iterator_concurrency);
      break;
    case AUTO_CORRECTED_SURROGATE:
      lowFidelityModel.free_communicators(max_iterator_concurrency);
      highFidelityModel.free_communicators(
	highFidelityModel.derivative_concurrency());
      break;
    case BYPASS_SURROGATE:
      highFidelityModel.free_communicators(max_iterator_concurrency);
      break;
    case MODEL_DISCREPANCY:
      lowFidelityModel.free_communicators(max_iterator_concurrency);
      highFidelityModel.free_communicators(max_iterator_concurrency);
      break;
    }
    */
  }
}


inline void HierarchSurrModel::serve()
{
  // manage lowFidelityModel and highFidelityModel servers
  componentParallelMode = 1;
  while (componentParallelMode) {
    parallelLib.bcast_i(componentParallelMode);
    if (componentParallelMode == LF_MODEL)
      lowFidelityModel.serve();
    else if (componentParallelMode == HF_MODEL)
      highFidelityModel.serve();
  }
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
