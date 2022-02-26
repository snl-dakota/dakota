/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       EnsembleSurrModel
//- Description: A model which manages a surrogate relationship between a low
//-              fidelity approximate model and a high fidelity truth model.
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: EnsembleSurrModel.hpp 7024 2010-10-16 01:24:42Z mseldre $

#ifndef ENSEMBLE_SURR_MODEL_H
#define ENSEMBLE_SURR_MODEL_H

#include "SurrogateModel.hpp"
#include "ParallelLibrary.hpp"
#include "DataModel.hpp"

namespace Dakota {


/// Derived model class within the surrogate model branch for managing
/// subordinate models of varying fidelity.

/** The EnsembleSurrModel class manages subordinate models of varying
    fidelity. */

class EnsembleSurrModel: public SurrogateModel
{
public:

  //
  //- Heading: Constructors and destructor
  //

  EnsembleSurrModel(ProblemDescDB& problem_db); ///< constructor
  ~EnsembleSurrModel();                         ///< destructor

protected:

  //
  //- Heading: New virtual functions
  //

  virtual void derived_synchronize_sequential(
    IntResponseMapArray& model_resp_maps_rekey, bool block) = 0;
  virtual void derived_synchronize_combine(
    IntResponseMapArray& model_resp_maps,
    IntResponseMap&      combined_resp_map) = 0;
  virtual void derived_synchronize_combine_nowait(
    IntResponseMapArray& model_resp_maps,
    IntResponseMap&      combined_resp_map) = 0;

  /// return the number of models that approximate the truth model
  virtual size_t num_approximation_models() const = 0;
  /// initialize truth and surrogate model keys to default values
  virtual void assign_default_keys() = 0;
  /// size id_maps and cached_resp_maps arrays according to responseMode
  virtual void resize_maps() = 0;
  /// resize currentResponse based on responseMode
  virtual void resize_response(bool use_virtual_counts = true) = 0;

  //
  //- Heading: Virtual function redefinitions
  //

  size_t qoi() const;

  const SizetArray& nested_acv1_indices() const;
  const ShortArray& nested_acv2_targets() const;
  short query_distribution_parameter_derivatives() const;

  void check_submodel_compatibility(const Model& sub_model);

  const IntResponseMap& derived_synchronize();
  const IntResponseMap& derived_synchronize_nowait();

  void stop_servers();

  bool multifidelity() const;
  bool multilevel() const;
  bool multilevel_multifidelity() const;

  bool multifidelity_precedence() const;
  void multifidelity_precedence(bool mf_prec, bool update_default = true);

  /// set responseMode and pass any bypass request on to the high
  /// fidelity model for any lower-level surrogate recursions
  void surrogate_response_mode(short mode);

  /// (re)set the surrogate index set in SurrogateModel::surrogateFnIndices
  void surrogate_function_indices(const SizetSet& surr_fn_indices);

  // return truthModel interface identifier?
  //const String& interface_id() const;

  /// set the evaluation counter reference points for the EnsembleSurrModel
  /// (request forwarded to truth and surrogate models)
  void set_evaluation_reference();

  //
  //- Heading: member functions
  //

  /// called from derived_synchronize() for case of distinct models/interfaces
  /// with competing LF/HF job queues
  void derived_synchronize_competing();

  /// helper to select among Variables::all_discrete_{int,string,real}_
  /// variable_labels() for exporting a solution control variable label
  const String& solution_control_label();

  /// helper to select among Model::solution_level_{int,string,real}_value()
  /// for exporting a scalar solution level value
  void add_tabular_solution_level_value(Model& model);

  //
  //- Heading: Data members
  //

  /// key defining active model form / resolution level for the truth model
  Pecos::ActiveKey truthModelKey;

  /// flag indicating that the {low,high}FidelityKey correspond to the
  /// same model instance, requiring modifications to updating and evaluation
  /// scheduling processes
  bool sameModelInstance;
  /// flag indicating that the models identified by {low,high}FidelityKey
  /// employ the same interface instance, requiring modifications to evaluation
  /// scheduling processes
  bool sameInterfaceInstance;
  /// index of solution control variable within all variables
  size_t solnCntlAVIndex;
  /// tie breaker for type of model hierarchy when forms and levels are present
  bool mfPrecedence;

  // store aggregate model key that is active in component_parallel_mode()
  //Pecos::ActiveKey componentParallelKey;
  /// size of MPI buffer containing responseMode and an aggregated activeKey
  int modeKeyBufferSize;

  /// map from evaluation ids of truthModel/unorderedModels to
  /// EnsembleSurrModel ids
  IntIntMapArray modelIdMaps;
  /// maps of responses retrieved in derived_synchronize_nowait() that
  /// could not be returned since corresponding response portions were
  /// still pending, blocking response aggregation
  IntResponseMapArray cachedRespMaps;

private:

  //
  //- Heading: Convenience functions
  //

  // check whether there are any non-empty maps
  bool test_id_maps(const IntIntMapArray& id_maps);
  // count number of non-empty maps
  size_t count_id_maps(const IntIntMapArray& id_maps);
};


inline EnsembleSurrModel::~EnsembleSurrModel()
{ }


inline size_t EnsembleSurrModel::qoi() const
{
  switch (responseMode) {
  // Note: resize_response() aggregates {truth,surrogate}_model().num_fns(),
  //       such that code below is a bit more general that currResp num_fns/2
  case AGGREGATED_MODELS:  return truth_model().qoi();  break;
  default:                 return response_size();      break;
  }
}


inline const SizetArray& EnsembleSurrModel::nested_acv1_indices() const
{ return truth_model().nested_acv1_indices(); }


inline const ShortArray& EnsembleSurrModel::nested_acv2_targets() const
{ return truth_model().nested_acv2_targets(); }


inline short EnsembleSurrModel::
query_distribution_parameter_derivatives() const
{ return truth_model().query_distribution_parameter_derivatives(); }


inline void EnsembleSurrModel::
check_submodel_compatibility(const Model& sub_model)
{
  bool err1 = check_active_variables(sub_model),
       err2 = check_inactive_variables(sub_model),
       err3 = check_response_qoi(sub_model);

  if (err1 || err2 || err3)
    abort_handler(MODEL_ERROR);
}


inline bool EnsembleSurrModel::multifidelity() const
{
  // This function is used when we don't want to alter logic at run-time based
  // on a deactivated key (as for same{Model,Interface}Instance)
  // > we rely on mfPrecedence passed from NonDExpansion::configure_sequence()
  //   based on the ML/MF algorithm selection; otherwise defaults to true

  return ( num_approximation_models() &&
	   ( mfPrecedence || truth_model().solution_levels() <= 1 ) );
}


inline bool EnsembleSurrModel::multilevel() const
{
  return ( truth_model().solution_levels() > 1 &&
	   ( !mfPrecedence || !num_approximation_models() ) );
}


inline bool EnsembleSurrModel::multilevel_multifidelity() const
{ return (num_approximation_models() && truth_model().solution_levels() > 1); }


inline bool EnsembleSurrModel::multifidelity_precedence() const
{ return mfPrecedence; }


inline void EnsembleSurrModel::
multifidelity_precedence(bool mf_prec, bool update_default)
{
  if (mfPrecedence != mf_prec) {
    mfPrecedence = mf_prec;
    if (update_default) assign_default_keys();
  }
}


inline void EnsembleSurrModel::set_evaluation_reference()
{
  //surrogate_model().set_evaluation_reference();

  // don't recurse this, since the eval reference is for the top level iteration
  //if (responseMode == BYPASS_SURROGATE)
  //  truth_model().set_evaluation_reference();

  // may want to add this in time
  //surrModelEvalRef = surrModelEvalCntr;
}


inline bool EnsembleSurrModel::test_id_maps(const IntIntMapArray& id_maps)
{
  size_t i, num_map = id_maps.size();
  for (i=0; i<num_map; ++i)
    if (!id_maps[i].empty())
      return true;
  return false;
}


inline size_t EnsembleSurrModel::count_id_maps(const IntIntMapArray& id_maps)
{
  size_t i, num_map = id_maps.size(), cntr = 0;
  for (i=0; i<num_map; ++i)
    if (!id_maps[i].empty())
      ++cntr;
  return cntr;
}


inline void EnsembleSurrModel::surrogate_response_mode(short mode)
{
  responseMode = mode;

  // if necessary, resize the response for entering/exiting an aggregated mode.
  // Since parallel job scheduling only involves either the LF or HF model at
  // any given time, this call does not need to be matched on serve_run() procs.
  resize_response();

  /// allocate modelIdMaps and cachedRespMaps arrays based on responseMode
  resize_maps();

  // don't pass to approx models since point of a surrogate bypass is to get
  // a surrogate-free truth evaluation
  if (mode == BYPASS_SURROGATE) // recurse in this case
    truth_model().surrogate_response_mode(mode);
}


inline void EnsembleSurrModel::
surrogate_function_indices(const SizetSet& surr_fn_indices)
{ surrogateFnIndices = surr_fn_indices; }


inline void EnsembleSurrModel::stop_servers()
{ component_parallel_mode(0); }

} // namespace Dakota

#endif
