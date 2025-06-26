/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        EvaluationStore
//- Description:  Class implementation
//- Owner:        J. Adam Stephens
#include "EvaluationStore.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <tuple>
#ifdef DAKOTA_HAVE_HDF5
#include "HDF5_IO.hpp"
#endif
#include "DakotaActiveSet.hpp"
#include "DakotaResponse.hpp"
#include "DakotaVariables.hpp"
#include "MarginalsCorrDistribution.hpp"
#include "dakota_data_types.hpp"
#include "dakota_results_types.hpp"

namespace Dakota {

const Real REAL_DSET_FILL_VAL =
    NAN;  // not a number, constant defined in <cmath>
const int INT_DSET_FILL_VAL = INT_MAX;
const String STR_DSET_FILL_VAL = "";

DefaultSet::DefaultSet(const ActiveSet &in_set, const size_t num_metadata)
    : set(in_set), numMetadata(num_metadata) {
  const ShortArray &asv = set.request_vector();
  numFunctions = asv.size();
  numGradients = std::count_if(asv.begin(), asv.end(),
                               [](const short &a) { return a & 2; });
  numHessians = std::count_if(asv.begin(), asv.end(),
                              [](const short &a) { return a & 4; });
}

const int HDF5_CHUNK_SIZE = 40000;
#ifdef DAKOTA_HAVE_HDF5
void EvaluationStore::set_database(std::shared_ptr<HDF5IOHelper> db_ptr) {
  hdf5Stream = db_ptr;
}
#endif

bool EvaluationStore::active() {
#ifdef DAKOTA_HAVE_HDF5
  return bool(hdf5Stream);
#else
  return false;
#endif
}

std::map<unsigned short, String> EvaluationStore::create_variable_type_map() {
  std::map<unsigned short, String> variable_types;
  variable_types[EMPTY_TYPE] = "EMPTY_TYPE";
  variable_types[CONTINUOUS_DESIGN] = "CONTINUOUS_DESIGN";
  variable_types[DISCRETE_DESIGN_RANGE] = "DISCRETE_DESIGN_RANGE";
  variable_types[DISCRETE_DESIGN_SET_INT] = "DISCRETE_DESIGN_SET_INT";
  variable_types[DISCRETE_DESIGN_SET_STRING] = "DISCRETE_DESIGN_SET_STRING";
  variable_types[DISCRETE_DESIGN_SET_REAL] = "DISCRETE_DESIGN_SET_REAL";
  variable_types[NORMAL_UNCERTAIN] = "NORMAL_UNCERTAIN";
  variable_types[LOGNORMAL_UNCERTAIN] = "LOGNORMAL_UNCERTAIN";
  variable_types[UNIFORM_UNCERTAIN] = "UNIFORM_UNCERTAIN";
  variable_types[LOGUNIFORM_UNCERTAIN] = "LOGUNIFORM_UNCERTAIN";
  variable_types[TRIANGULAR_UNCERTAIN] = "TRIANGULAR_UNCERTAIN";
  variable_types[EXPONENTIAL_UNCERTAIN] = "EXPONENTIAL_UNCERTAIN";
  variable_types[BETA_UNCERTAIN] = "BETA_UNCERTAIN";
  variable_types[GAMMA_UNCERTAIN] = "GAMMA_UNCERTAIN";
  variable_types[GUMBEL_UNCERTAIN] = "GUMBEL_UNCERTAIN";
  variable_types[FRECHET_UNCERTAIN] = "FRECHET_UNCERTAIN";
  variable_types[WEIBULL_UNCERTAIN] = "WEIBULL_UNCERTAIN";
  variable_types[HISTOGRAM_BIN_UNCERTAIN] = "HISTOGRAM_BIN_UNCERTAIN";
  variable_types[POISSON_UNCERTAIN] = "POISSON_UNCERTAIN";
  variable_types[BINOMIAL_UNCERTAIN] = "BINOMIAL_UNCERTAIN";
  variable_types[NEGATIVE_BINOMIAL_UNCERTAIN] = "NEGATIVE_BINOMIAL_UNCERTAIN";
  variable_types[GEOMETRIC_UNCERTAIN] = "GEOMETRIC_UNCERTAIN";
  variable_types[HYPERGEOMETRIC_UNCERTAIN] = "HYPERGEOMETRIC_UNCERTAIN";
  variable_types[HISTOGRAM_POINT_UNCERTAIN_INT] =
      "HISTOGRAM_POINT_UNCERTAIN_INT";
  variable_types[HISTOGRAM_POINT_UNCERTAIN_STRING] =
      "HISTOGRAM_POINT_UNCERTAIN_STRING";
  variable_types[HISTOGRAM_POINT_UNCERTAIN_REAL] =
      "HISTOGRAM_POINT_UNCERTAIN_REAL";
  variable_types[CONTINUOUS_INTERVAL_UNCERTAIN] =
      "CONTINUOUS_INTERVAL_UNCERTAIN";
  variable_types[DISCRETE_INTERVAL_UNCERTAIN] = "DISCRETE_INTERVAL_UNCERTAIN";
  variable_types[DISCRETE_UNCERTAIN_SET_INT] = "DISCRETE_UNCERTAIN_SET_INT";
  variable_types[DISCRETE_UNCERTAIN_SET_STRING] =
      "DISCRETE_UNCERTAIN_SET_STRING";
  variable_types[DISCRETE_UNCERTAIN_SET_REAL] = "DISCRETE_UNCERTAIN_SET_REAL";
  variable_types[CONTINUOUS_STATE] = "CONTINUOUS_STATE";
  variable_types[DISCRETE_STATE_RANGE] = "DISCRETE_STATE_RANGE";
  variable_types[DISCRETE_STATE_SET_INT] = "DISCRETE_STATE_SET_INT";
  variable_types[DISCRETE_STATE_SET_STRING] = "DISCRETE_STATE_SET_STRING";
  variable_types[DISCRETE_STATE_SET_REAL] = "DISCRETE_STATE_SET_REAL";
  return variable_types;
}

const std::map<unsigned short, String> EvaluationStore::variableTypes =
    EvaluationStore::create_variable_type_map();

// Declare a source for the mdoel or iterator.
// Permissible values of owner_type are "iterator",
//  "nested", "surrogate", "recast", and "simulation".
// Permissible values of source_type are "iterator", "nested", "surrogate",
// "recast", "simulation", "interface", and "approximation".
void EvaluationStore::declare_source(const String &owner_id,
                                     const String &owner_type,
                                     const String &source_id,
                                     const String &source_type) {
#ifdef DAKOTA_HAVE_HDF5
  if (!active()) return;

  if (owner_type == "iterator") {
    declare_iterator_source(owner_id, source_id, source_type);
  } else {  // owner is one of the model types.
    declare_model_source(owner_id, owner_type, source_id, source_type);
  }
#else
  return;
#endif
}

#ifdef DAKOTA_HAVE_HDF5
void EvaluationStore::declare_iterator_source(const String owner_id,
                                              const String source_id,
                                              const String source_type) {
  String link_location =
      String("/methods/") + owner_id + "/sources/" + source_id;
  if (source_type == "iterator") {
    String source_location = String("/methods/") + source_id;
    hdf5Stream->create_softlink(link_location, source_location);
  } else {  // source is a model
    update_source_models(owner_id, source_id);
    if (model_active(source_id)) {
      String source_location =
          String("/models/") + source_type + "/" + source_id;
      hdf5Stream->create_softlink(link_location, source_location);
    }
  }
}

void EvaluationStore::declare_model_source(const String owner_id,
                                           const String owner_type,
                                           const String source_id,
                                           const String source_type) {
  String link_location = String("/models/") + owner_type + "/" + owner_id +
                         "/sources/" + source_id;
  if (source_type == "iterator") {
    String source_location = String("/methods/") + source_id;
    hdf5Stream->create_softlink(link_location, source_location);
  } else if (source_type == "interface" && interface_active(source_type)) {
    String source_location =
        String("/interfaces/") + source_id + "/" + owner_id;
    hdf5Stream->create_softlink(link_location, source_location);
  } else if (model_active(source_id)) {  // source is a model
    String source_location = String("/models/") + source_type + "/" + source_id;
    hdf5Stream->create_softlink(link_location, source_location);
  }
}
#endif

void EvaluationStore::update_source_models(const String owner_id,
                                           const String source_id) {
  if ((modelSelection == MODEL_EVAL_STORE_TOP_METHOD &&
       owner_id == topLevelMethodId) ||
      modelSelection == MODEL_EVAL_STORE_ALL_METHODS)
    sourceModels.emplace(source_id);
}

EvaluationsDBState EvaluationStore::iterator_allocate(const String &iterator_id,
                                                      const bool &top_level) {
#ifdef DAKOTA_HAVE_HDF5
  if (!active()) return EvaluationsDBState::INACTIVE;
  if (top_level) {
    topLevelMethodId = iterator_id;
    hdf5Stream->add_attribute("/", "top_method", iterator_id);
  }
  return EvaluationsDBState::ACTIVE;
#else
  return EvaluationsDBState::INACTIVE;
#endif
}

/// Allocate storage for model evaluations
EvaluationsDBState EvaluationStore::model_allocate(
    const String &model_id, const String &model_type,
    const Variables &variables, const Pecos::MultivariateDistribution &mv_dist,
    const Response &response, const ActiveSet &set) {
#ifdef DAKOTA_HAVE_HDF5
  if (!(active() && model_active(model_id)))
    return EvaluationsDBState::INACTIVE;
  allocatedModels.emplace(model_id);
  const size_t num_metadata = response.shared_data().metadata_labels().size();
  const auto &ds_pair =
      modelDefaultSets.emplace(model_id, DefaultSet(set, num_metadata));
  const DefaultSet &default_set = (*ds_pair.first).second;
  String root_group = create_model_root(model_id, model_type);
  String scale_root = create_scale_root(root_group);
  // Create evaluation ID dataset, which is attached as a scale to many datasets
  String eval_ids_scale = scale_root + "evaluation_ids";
  hdf5Stream->create_empty_dataset(eval_ids_scale, {0},
                                   ResultsOutputType::INTEGER, HDF5_CHUNK_SIZE);

  std::shared_ptr<Pecos::MarginalsCorrDistribution> mvd_rep =
      std::static_pointer_cast<Pecos::MarginalsCorrDistribution>(
          mv_dist.multivar_dist_rep());
  // BMA: Left this a raw get() due to default of NULL
  allocate_variables(root_group, variables, mvd_rep.get());
  allocate_response(root_group, response, default_set);
  allocate_properties(root_group, variables, response, default_set);
  allocate_metadata(root_group, response);
  return EvaluationsDBState::ACTIVE;
#else
  return EvaluationsDBState::INACTIVE;
#endif
}

/// Allocate storage for evalulations of interface+model pairs
EvaluationsDBState EvaluationStore::interface_allocate(
    const String &model_id, const String &interface_id,
    const String &interface_type, const Variables &variables,
    const Response &response, const ActiveSet &set,
    const String2DArray &an_comp) {
#ifdef DAKOTA_HAVE_HDF5
  if (!(active() && interface_active(interface_type)))
    return EvaluationsDBState::INACTIVE;
  allocatedInterfaces.emplace(make_pair(model_id, interface_id));
  const size_t num_metadata = response.shared_data().metadata_labels().size();
  const auto &ds_pair = interfaceDefaultSets.emplace(
      std::make_pair(model_id, interface_id), DefaultSet(set, num_metadata));
  const DefaultSet &default_set = (*ds_pair.first).second;
  String root_group = create_interface_root(model_id, interface_id);
  String scale_root = create_scale_root(root_group);
  // Create evaluation ID dataset, which is attached as a scale to many datasets
  String eval_ids_scale = scale_root + "evaluation_ids";
  hdf5Stream->create_empty_dataset(eval_ids_scale, {0},
                                   ResultsOutputType::INTEGER, HDF5_CHUNK_SIZE);

  allocate_variables(root_group, variables);
  allocate_response(root_group, response, default_set);
  allocate_properties(root_group, variables, response, default_set, an_comp);
  allocate_metadata(root_group, response);
  return EvaluationsDBState::ACTIVE;
#else
  return EvaluationsDBState::INACTIVE;
#endif
}

/// Store variables and properties for a model evaluation
void EvaluationStore::store_model_variables(const String &model_id,
                                            const String &model_type,
                                            const int &eval_id,
                                            const ActiveSet &set,
                                            const Variables &variables) {
#ifdef DAKOTA_HAVE_HDF5
  if (!active()) return;
  const DefaultSet &default_set_s = modelDefaultSets[model_id];
  if (set.request_vector().size() != default_set_s.numFunctions) {
    if (resizedModels.find(model_id) == resizedModels.end()) {
      resizedModels.insert(model_id);
      Cerr
          << "Warning: Number of functions provided to HDF5 database by model\n"
          << "\n  '" << model_id
          << "'\n\nhas changed since the study began. This behavior currently "
             "is\n"
          << "not supported. Storage will be skipped.\n";
    }
    modelResponseIndexCache.emplace(std::make_tuple(model_id, eval_id), -1);
    return;
  }
  resizedModels.erase(model_id);
  String root_group = create_model_root(model_id, model_type);
  String scale_root = create_scale_root(root_group);
  // Create evaluation ID dataset, which is attached as a scale to many datasets
  String eval_ids_scale = scale_root + "evaluation_ids";
  hdf5Stream->append_scalar(eval_ids_scale, eval_id);
  store_variables(root_group, variables);
  store_properties(root_group, set, default_set_s);

  int resp_idx = hdf5Stream->append_empty(root_group + "responses/functions");
  const ShortArray &default_asv = default_set_s.set.request_vector();
  if (default_set_s.numGradients)
    hdf5Stream->append_empty(root_group + "responses/gradients");
  if (default_set_s.numHessians)
    hdf5Stream->append_empty(root_group + "responses/hessians");
  if (default_set_s.numMetadata)
    hdf5Stream->append_empty(root_group + "metadata");
  modelResponseIndexCache.emplace(std::make_tuple(model_id, eval_id), resp_idx);
#else
  return;
#endif
}

/// Store a response (including metadata) for model evaluation
void EvaluationStore::store_model_response(const String &model_id,
                                           const String &model_type,
                                           const int &eval_id,
                                           const Response &response) {
#ifdef DAKOTA_HAVE_HDF5
  if (!active()) return;
  const DefaultSet &default_set_s = modelDefaultSets[model_id];
  std::tuple<String, int> key(model_id, eval_id);
  int response_index = modelResponseIndexCache[key];
  if (response_index == -1) return;
  String root_group = create_model_root(model_id, model_type);
  store_response(root_group, response_index, response, default_set_s);
  store_metadata(root_group, response_index, response);
  auto cache_entry = modelResponseIndexCache.find(key);
  modelResponseIndexCache.erase(cache_entry);
#else
  return;
#endif
}

/// Store variables for an interface+model evaluation
void EvaluationStore::store_interface_variables(const String &model_id,
                                                const String &interface_id,
                                                const int &eval_id,
                                                const ActiveSet &set,
                                                const Variables &variables) {
#ifdef DAKOTA_HAVE_HDF5
  if (!active()) return;
  String root_group = create_interface_root(model_id, interface_id);
  String scale_root = create_scale_root(root_group);
  const auto set_key = std::make_pair(model_id, interface_id);
  const DefaultSet &default_set_s = interfaceDefaultSets[set_key];
  // Create evaluation ID dataset, which is attached as a scale to many datasets
  String eval_ids_scale = scale_root + "evaluation_ids";
  hdf5Stream->append_scalar(eval_ids_scale, eval_id);
  store_variables(root_group, variables);
  store_properties(root_group, set, default_set_s);

  int resp_idx = hdf5Stream->append_empty(root_group + "responses/functions");
  if (default_set_s.numGradients)
    hdf5Stream->append_empty(root_group + "responses/gradients");
  if (default_set_s.numHessians)
    hdf5Stream->append_empty(root_group + "responses/hessians");
  if (default_set_s.numMetadata)
    hdf5Stream->append_empty(root_group + "metadata");
  interfaceResponseIndexCache.emplace(
      std::make_tuple(model_id, interface_id, eval_id), resp_idx);
#else
  return;
#endif
}

/// Store a response for an interface+model evaluation
void EvaluationStore::store_interface_response(const String &model_id,
                                               const String &interface_id,
                                               const int &eval_id,
                                               const Response &response) {
#ifdef DAKOTA_HAVE_HDF5
  if (!active()) return;
  std::tuple<String, String, int> key(model_id, interface_id, eval_id);
  int response_index = interfaceResponseIndexCache[key];
  String root_group = create_interface_root(model_id, interface_id);
  store_response(root_group, response_index, response,
                 interfaceDefaultSets[std::make_pair(model_id, interface_id)]);
  store_metadata(root_group, response_index, response);
  auto cache_entry = interfaceResponseIndexCache.find(key);
  interfaceResponseIndexCache.erase(cache_entry);
#else
  return;
#endif
}

String EvaluationStore::create_interface_root(const String &model_id,
                                              const String &interface_id) {
  return String("/interfaces/") + interface_id + '/' + model_id + '/';
}

String EvaluationStore::create_model_root(const String &model_id,
                                          const String &model_type) {
  return String("/models/") + model_type + '/' + model_id + '/';
}

String EvaluationStore::create_scale_root(const String &root_group) {
  return String("/_scales") + root_group;
}

/// Allocate storage for variables
void EvaluationStore::allocate_variables(
    const String &root_group, const Variables &variables,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
  // TODO: variable names and order
#ifdef DAKOTA_HAVE_HDF5
  String variables_root_group = root_group + "variables/";
  String scale_root = create_scale_root(root_group);
  String variables_scale_root = scale_root + "variables/";
  String eval_ids = scale_root + "evaluation_ids";

  if (mvd_rep)  // will be NULL for interfaces
    allocate_variable_parameters(root_group, variables, mvd_rep);

  if (variables.acv()) {
    String data_name = variables_root_group + "continuous";
    String labels_name = variables_scale_root + "continuous_descriptors";
    String ids_name = variables_scale_root + "continuous_ids";
    String types_name = variables_scale_root + "continuous_types";

    hdf5Stream->create_empty_dataset(data_name, {0, int(variables.acv())},
                                     ResultsOutputType::REAL, HDF5_CHUNK_SIZE);
    hdf5Stream->store_vector(labels_name,
                             variables.all_continuous_variable_labels());
    hdf5Stream->attach_scale(data_name, eval_ids, "evaluation_ids", 0);
    hdf5Stream->attach_scale(data_name, labels_name, "variables", 1);
    hdf5Stream->store_vector(ids_name, variables.all_continuous_variable_ids());
    hdf5Stream->attach_scale(data_name, ids_name, "ids", 1);

    UShortMultiArrayConstView types = variables.all_continuous_variable_types();
    StringArray type_labels(variables.acv());
    std::transform(types.begin(), types.end(), type_labels.begin(),
                   [](const unsigned short t) { return variableTypes.at(t); });
    hdf5Stream->store_vector(types_name, type_labels);
    hdf5Stream->attach_scale(data_name, types_name, "types", 1);
  }

  if (variables.adiv()) {
    String data_name = variables_root_group + "discrete_integer";
    String labels_name = variables_scale_root + "discrete_integer_descriptors";
    String ids_name = variables_scale_root + "discrete_integer_ids";
    String types_name = variables_scale_root + "discrete_integer_types";

    hdf5Stream->create_empty_dataset(data_name, {0, int(variables.adiv())},
                                     ResultsOutputType::INTEGER,
                                     HDF5_CHUNK_SIZE);
    hdf5Stream->store_vector(labels_name,
                             variables.all_discrete_int_variable_labels());
    hdf5Stream->attach_scale(data_name, eval_ids, "evaluation_ids", 0);
    hdf5Stream->attach_scale(data_name, labels_name, "variables", 1);
    hdf5Stream->store_vector(ids_name,
                             variables.all_discrete_int_variable_ids());
    hdf5Stream->attach_scale(data_name, ids_name, "ids", 1);

    UShortMultiArrayConstView types =
        variables.all_discrete_int_variable_types();
    StringArray type_labels(variables.adiv());
    std::transform(types.begin(), types.end(), type_labels.begin(),
                   [](const unsigned short t) { return variableTypes.at(t); });
    hdf5Stream->store_vector(types_name, type_labels);
    hdf5Stream->attach_scale(data_name, types_name, "types", 1);
  }

  if (variables.adsv()) {
    String data_name = variables_root_group + "discrete_string";
    String labels_name = variables_scale_root + "discrete_string_descriptors";
    String ids_name = variables_scale_root + "discrete_string_ids";
    String types_name = variables_scale_root + "discrete_string_types";

    hdf5Stream->create_empty_dataset(data_name, {0, int(variables.adsv())},
                                     ResultsOutputType::STRING,
                                     HDF5_CHUNK_SIZE);
    hdf5Stream->store_vector(labels_name,
                             variables.all_discrete_string_variable_labels());
    hdf5Stream->attach_scale(data_name, eval_ids, "evaluation_ids", 0);
    hdf5Stream->attach_scale(data_name, labels_name, "variables", 1);
    hdf5Stream->store_vector(ids_name,
                             variables.all_discrete_string_variable_ids());
    hdf5Stream->attach_scale(data_name, ids_name, "ids", 1);

    UShortMultiArrayConstView types =
        variables.all_discrete_string_variable_types();
    StringArray type_labels(variables.adsv());
    std::transform(types.begin(), types.end(), type_labels.begin(),
                   [](const unsigned short t) { return variableTypes.at(t); });
    hdf5Stream->store_vector(types_name, type_labels);
    hdf5Stream->attach_scale(data_name, types_name, "types", 1);
  }

  if (variables.adrv()) {
    String data_name = variables_root_group + "discrete_real";
    String labels_name = variables_scale_root + "discrete_real_descriptors";
    String ids_name = variables_scale_root + "discrete_real_ids";
    String types_name = variables_scale_root + "discrete_real_types";

    hdf5Stream->create_empty_dataset(data_name, {0, int(variables.adrv())},
                                     ResultsOutputType::REAL, HDF5_CHUNK_SIZE);
    hdf5Stream->store_vector(labels_name,
                             variables.all_discrete_real_variable_labels());
    hdf5Stream->attach_scale(data_name, eval_ids, "evaluation_ids", 0);
    hdf5Stream->attach_scale(data_name, labels_name, "variables", 1);
    hdf5Stream->store_vector(ids_name,
                             variables.all_discrete_real_variable_ids());
    hdf5Stream->attach_scale(data_name, ids_name, "ids", 1);

    UShortMultiArrayConstView types =
        variables.all_discrete_real_variable_types();
    StringArray type_labels(variables.adrv());
    std::transform(types.begin(), types.end(), type_labels.begin(),
                   [](const unsigned short t) { return variableTypes.at(t); });
    hdf5Stream->store_vector(types_name, type_labels);
    hdf5Stream->attach_scale(data_name, types_name, "types", 1);
  }
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_continuous_design(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv type: Pecos::CONTINUOUS_RANGE
  // parameters: Pecos::CR_LWR_BND, Pecos::CR_UPR_BND
  RealArray lbs, ubs;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::CR_LWR_BND, lbs);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::CR_UPR_BND, ubs);
  std::vector<VariableParametersField> fields = {
      VariableParametersField("lower_bound", ResultsOutputType::REAL),
      VariableParametersField("upper_bound", ResultsOutputType::REAL)};
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, lbs, "lower_bound");
  hdf5Stream->set_vector_scalar_field(location, ubs, "upper_bound");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_discrete_design_range(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv type: Pecos::DISCRETE_RANGE
  // parameters: Pecos::DR_LWR_BND, Pecos::DR_UPR_BND
  IntArray lbs, ubs;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::DR_LWR_BND, lbs);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::DR_UPR_BND, ubs);
  std::vector<VariableParametersField> fields = {
      VariableParametersField("lower_bound", ResultsOutputType::INTEGER),
      VariableParametersField("upper_bound", ResultsOutputType::INTEGER)};
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, lbs, "lower_bound");
  hdf5Stream->set_vector_scalar_field(location, ubs, "upper_bound");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_discrete_design_set_int(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv type: Pecos::DISCRETE_SET_INT
  // parameters: Pecos::DSI_VALUES
  IntSetArray isa;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::DSI_VALUES, isa);
  // Because h5py barfs on vlen datasets of vlen strings, we have to
  // use regular, fixed-sized datasets that are big enough to hold the
  // maximum number of elements.
  size_t max_num_elements = 0;
  IntArray num_elements;
  for (const auto &e : isa) {
    num_elements.push_back(e.size());
    max_num_elements =
        (max_num_elements > e.size()) ? max_num_elements : e.size();
  }
  // Populate a 1D array with ALL the elements, including padding
  IntArray all_elements(num_rv * max_num_elements, INT_DSET_FILL_VAL);
  for (int i = 0; i < num_rv; ++i)
    std::copy(isa[i].begin(), isa[i].end(),
              &all_elements[i * max_num_elements]);

  std::vector<VariableParametersField> fields = {
      VariableParametersField("num_elements", ResultsOutputType::INTEGER),
      VariableParametersField("elements", ResultsOutputType::INTEGER,
                              {max_num_elements}),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, num_elements, "num_elements");
  hdf5Stream->set_vector_vector_field(location, all_elements, max_num_elements,
                                      "elements");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_discrete_design_set_string(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv type: Pecos::DISCRETE_SET_INT
  // parameters: Pecos::DSI_VALUES
  StringSetArray ssa;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::DSS_VALUES, ssa);
  // Because h5py barfs on vlen datasets of vlen strings, we have to
  // use regular, fixed-sized datasets that are big enough to hold the
  // maximum number of elements.
  size_t max_num_elements = 0;
  IntArray num_elements;
  for (const auto &e : ssa) {
    num_elements.push_back(e.size());
    max_num_elements =
        (max_num_elements > e.size()) ? max_num_elements : e.size();
  }
  // Populate a 1D array with ALL the elements, including padding
  StringArray all_elements(num_rv * max_num_elements, STR_DSET_FILL_VAL);
  for (int i = 0; i < num_rv; ++i)
    std::copy(ssa[i].begin(), ssa[i].end(),
              &all_elements[i * max_num_elements]);

  std::vector<VariableParametersField> fields = {
      VariableParametersField("num_elements", ResultsOutputType::INTEGER),
      VariableParametersField("elements", ResultsOutputType::STRING,
                              {max_num_elements}),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, num_elements, "num_elements");
  hdf5Stream->set_vector_vector_field(location, all_elements, max_num_elements,
                                      "elements");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_discrete_design_set_real(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv type: Pecos::DISCRETE_SET_INT
  // parameters: Pecos::DSI_VALUES
  RealSetArray rsa;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::DSR_VALUES, rsa);
  // Because h5py barfs on vlen datasets of vlen strings, we have to
  // use regular, fixed-sized datasets that are big enough to hold the
  // maximum number of elements.
  size_t max_num_elements = 0;
  IntArray num_elements;
  for (const auto &e : rsa) {
    num_elements.push_back(e.size());
    max_num_elements =
        (max_num_elements > e.size()) ? max_num_elements : e.size();
  }
  // Populate a 1D array with ALL the elements, including padding
  RealArray all_elements(num_rv * max_num_elements, REAL_DSET_FILL_VAL);
  for (int i = 0; i < num_rv; ++i)
    std::copy(rsa[i].begin(), rsa[i].end(),
              &all_elements[i * max_num_elements]);

  std::vector<VariableParametersField> fields = {
      VariableParametersField("num_elements", ResultsOutputType::INTEGER),
      VariableParametersField("elements", ResultsOutputType::REAL,
                              {max_num_elements}),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, num_elements, "num_elements");
  hdf5Stream->set_vector_vector_field(location, all_elements, max_num_elements,
                                      "elements");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_normal_uncertain(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv types: Pecos::NORMAL, Pecos::BOUNDED_NORMAL
  // parameters: Pecos::N_MEAN, Pecos::N_STD_DEV, Pecos::N_LWR_BND,
  // Pecos::N_UPR_BND Use count-based API for lookup since there are two
  // possible Pecos var types
  RealArray means, std_devs, lbs, ubs;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::N_MEAN, means);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::N_STD_DEV, std_devs);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::N_LWR_BND, lbs);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::N_UPR_BND, ubs);
  std::vector<VariableParametersField> fields = {
      VariableParametersField("mean", ResultsOutputType::REAL),
      VariableParametersField("std_deviation", ResultsOutputType::REAL),
      VariableParametersField("lower_bound", ResultsOutputType::REAL),
      VariableParametersField("upper_bound", ResultsOutputType::REAL)};
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, means, "mean");
  hdf5Stream->set_vector_scalar_field(location, std_devs, "std_deviation");
  hdf5Stream->set_vector_scalar_field(location, lbs, "lower_bound");
  hdf5Stream->set_vector_scalar_field(location, ubs, "upper_bound");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_uniform_uncertain(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv types: Pecos::UNIFORM
  // parameters: Pecos::U_LWR_BND, Pecos::U_UPR_BND
  // Use count-based API for lookup since there are two possible Pecos var types
  RealArray lbs, ubs;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::U_LWR_BND, lbs);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::U_UPR_BND, ubs);
  std::vector<VariableParametersField> fields = {
      VariableParametersField("lower_bound", ResultsOutputType::REAL),
      VariableParametersField("upper_bound", ResultsOutputType::REAL)};
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, lbs, "lower_bound");
  hdf5Stream->set_vector_scalar_field(location, ubs, "upper_bound");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_lognormal_uncertain(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv types: Pecos::LOGNORMAL, BOUNDED_LOGNORMAL
  // parameters: (LN_MEAN with LN_STD_DEV or LN_ERR_FACT) OR
  //             (LN_LAMBDA with LN_ZETA)
  //             LN_LWR_BND, LN_UPR_BND
  RealArray lbs, ubs, means, std_devs, err_facts, lambdas, zetas;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::LN_LWR_BND, lbs);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::LN_UPR_BND, ubs);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::LN_MEAN, means);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::LN_STD_DEV, std_devs);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::LN_ERR_FACT, err_facts);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::LN_LAMBDA, lambdas);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::LN_ZETA, zetas);
  std::vector<VariableParametersField> fields = {
      VariableParametersField("lower_bound", ResultsOutputType::REAL),
      VariableParametersField("upper_bound", ResultsOutputType::REAL),
      VariableParametersField("mean", ResultsOutputType::REAL),
      VariableParametersField("std_deviation", ResultsOutputType::REAL),
      VariableParametersField("error_factor", ResultsOutputType::REAL),
      VariableParametersField("lambda", ResultsOutputType::REAL),
      VariableParametersField("zeta", ResultsOutputType::REAL)};
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, lbs, "lower_bound");
  hdf5Stream->set_vector_scalar_field(location, ubs, "upper_bound");
  hdf5Stream->set_vector_scalar_field(location, means, "mean");
  hdf5Stream->set_vector_scalar_field(location, std_devs, "std_deviation");
  hdf5Stream->set_vector_scalar_field(location, err_facts, "error_factor");
  hdf5Stream->set_vector_scalar_field(location, lambdas, "lambda");
  hdf5Stream->set_vector_scalar_field(location, zetas, "zeta");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_loguniform_uncertain(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv types: Pecos::LOGUNIFORM
  // parameters:  LU_LWR_BND, LU_UPR_BND
  RealArray lbs, ubs;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::LU_LWR_BND, lbs);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::LU_UPR_BND, ubs);
  std::vector<VariableParametersField> fields = {
      VariableParametersField("lower_bound", ResultsOutputType::REAL),
      VariableParametersField("upper_bound", ResultsOutputType::REAL),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, lbs, "lower_bound");
  hdf5Stream->set_vector_scalar_field(location, ubs, "upper_bound");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_triangular_uncertain(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv types: Pecos::TRIANGULAR
  // parameters:  T_LWR_BND, T_UPR_BND, T_MODE
  RealArray lbs, ubs, modes;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::T_MODE, modes);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::T_LWR_BND, lbs);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::T_UPR_BND, ubs);
  std::vector<VariableParametersField> fields = {
      VariableParametersField("mode", ResultsOutputType::REAL),
      VariableParametersField("lower_bound", ResultsOutputType::REAL),
      VariableParametersField("upper_bound", ResultsOutputType::REAL),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, modes, "mode");
  hdf5Stream->set_vector_scalar_field(location, lbs, "lower_bound");
  hdf5Stream->set_vector_scalar_field(location, ubs, "upper_bound");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_exponential_uncertain(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv types: Pecos::EXPONENTIAL
  // parameters:  E_BETA
  RealArray betas;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::E_BETA, betas);
  std::vector<VariableParametersField> fields = {
      VariableParametersField("beta", ResultsOutputType::REAL),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, betas, "beta");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_beta_uncertain(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv types: Pecos::BETA
  // parameters:  BE_ALPHA, BE_BETA, BE_LWR_BND, BE_UPR_BND
  RealArray alphas, betas, lbs, ubs;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::BE_ALPHA, alphas);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::BE_BETA, betas);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::BE_LWR_BND, lbs);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::BE_UPR_BND, ubs);
  std::vector<VariableParametersField> fields = {
      VariableParametersField("alpha", ResultsOutputType::REAL),
      VariableParametersField("beta", ResultsOutputType::REAL),
      VariableParametersField("lower_bound", ResultsOutputType::REAL),
      VariableParametersField("upper_bound", ResultsOutputType::REAL),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, alphas, "alpha");
  hdf5Stream->set_vector_scalar_field(location, betas, "beta");
  hdf5Stream->set_vector_scalar_field(location, lbs, "lower_bound");
  hdf5Stream->set_vector_scalar_field(location, ubs, "upper_bound");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_gamma_uncertain(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv types: Pecos::GAMMA
  // parameters:  GA_ALPHA, GA_BETA
  RealArray alphas, betas;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::GA_ALPHA, alphas);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::GA_BETA, betas);
  std::vector<VariableParametersField> fields = {
      VariableParametersField("alpha", ResultsOutputType::REAL),
      VariableParametersField("beta", ResultsOutputType::REAL),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, alphas, "alpha");
  hdf5Stream->set_vector_scalar_field(location, betas, "beta");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_gumbel_uncertain(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv types: Pecos::GUMBEL
  // parameters:  GU_ALPHA, GU_BETA
  RealArray alphas, betas;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::GU_ALPHA, alphas);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::GU_BETA, betas);
  std::vector<VariableParametersField> fields = {
      VariableParametersField("alpha", ResultsOutputType::REAL),
      VariableParametersField("beta", ResultsOutputType::REAL),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, alphas, "alpha");
  hdf5Stream->set_vector_scalar_field(location, betas, "beta");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_frechet_uncertain(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv types: Pecos::FRECHET
  // parameters:  F_ALPHA, F_BETA
  RealArray alphas, betas;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::F_ALPHA, alphas);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::F_BETA, betas);
  std::vector<VariableParametersField> fields = {
      VariableParametersField("alpha", ResultsOutputType::REAL),
      VariableParametersField("beta", ResultsOutputType::REAL),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, alphas, "alpha");
  hdf5Stream->set_vector_scalar_field(location, betas, "beta");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_weibull_uncertain(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv types: Pecos::WEIBULL
  // parameters:  W_ALPHA, W_BETA
  RealArray alphas, betas;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::W_ALPHA, alphas);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::W_BETA, betas);
  std::vector<VariableParametersField> fields = {
      VariableParametersField("alpha", ResultsOutputType::REAL),
      VariableParametersField("beta", ResultsOutputType::REAL),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, alphas, "alpha");
  hdf5Stream->set_vector_scalar_field(location, betas, "beta");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_histogram_bin_uncertain(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv type: Pecos::HISTOGRAM_BIN
  // parameters: Pecos::H_BIN_PAIRS
  RealRealMapArray bin_pairs;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::H_BIN_PAIRS, bin_pairs);
  // Because h5py barfs on vlen datasets of vlen strings, we have to
  // use regular, fixed-sized datasets that are big enough to hold the
  // maximum number of elements.
  size_t max_num_elements = 0;
  SizetArray num_elements;
  for (const auto &p : bin_pairs) {
    num_elements.push_back(p.size());
    max_num_elements =
        (max_num_elements > p.size()) ? max_num_elements : p.size();
  }
  // Populate a 1D array with ALL the elements, including padding
  RealArray counts(num_rv * max_num_elements, REAL_DSET_FILL_VAL),
      abscissas(num_rv * max_num_elements, REAL_DSET_FILL_VAL);
  for (int i = 0; i < num_rv; ++i) {
    std::transform(bin_pairs[i].begin(), bin_pairs[i].end(),
                   &abscissas[i * max_num_elements],
                   [](const std::pair<Real, Real> &p) { return p.first; });
    std::transform(bin_pairs[i].begin(), bin_pairs[i].end(),
                   &counts[i * max_num_elements],
                   [](const std::pair<Real, Real> &p) { return p.second; });
  }

  std::vector<VariableParametersField> fields = {
      VariableParametersField("num_elements", ResultsOutputType::INTEGER),
      VariableParametersField("abscissas", ResultsOutputType::REAL,
                              {max_num_elements}),
      VariableParametersField("counts", ResultsOutputType::REAL,
                              {max_num_elements}),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, num_elements, "num_elements");
  hdf5Stream->set_vector_vector_field(location, abscissas, max_num_elements,
                                      "abscissas");
  hdf5Stream->set_vector_vector_field(location, counts, max_num_elements,
                                      "counts");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_poisson_uncertain(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv types: Pecos::POISSON
  // parameters: P_LAMBDA
  RealArray lambdas;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::P_LAMBDA, lambdas);
  std::vector<VariableParametersField> fields = {
      VariableParametersField("lambda", ResultsOutputType::REAL),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, lambdas, "lambda");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_binomial_uncertain(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv types: Pecos::BINOMIAL
  // parameters: BI_P_PER_TRIAL, BI_TRIALS
  RealArray p_per_trial;
  UIntArray trials;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::BI_P_PER_TRIAL,
                           p_per_trial);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::BI_TRIALS, trials);
  std::vector<VariableParametersField> fields = {
      VariableParametersField("probability_per_trial", ResultsOutputType::REAL),
      VariableParametersField("num_trials", ResultsOutputType::UINTEGER),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, p_per_trial,
                                      "probability_per_trial");
  hdf5Stream->set_vector_scalar_field(location, trials, "num_trials");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_negative_binomial_uncertain(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv types: Pecos::NEGATIVE_BINOMIAL
  // parameters: NBI_P_PER_TRIAL, NBI_TRIALS
  RealArray p_per_trial;
  UIntArray trials;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::NBI_P_PER_TRIAL,
                           p_per_trial);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::NBI_TRIALS, trials);
  std::vector<VariableParametersField> fields = {
      VariableParametersField("probability_per_trial", ResultsOutputType::REAL),
      VariableParametersField("num_trials", ResultsOutputType::UINTEGER),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, p_per_trial,
                                      "probability_per_trial");
  hdf5Stream->set_vector_scalar_field(location, trials, "num_trials");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_geometric_uncertain(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv types: Pecos::GEOMETRIC
  // parameters: GE_P_PER_TRIAL
  RealArray p_per_trial;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::GE_P_PER_TRIAL,
                           p_per_trial);
  std::vector<VariableParametersField> fields = {
      VariableParametersField("probability_per_trial", ResultsOutputType::REAL),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, p_per_trial,
                                      "probability_per_trial");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_hypergeometric_uncertain(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv types: Pecos::HYPERGEOMETRIC
  // parameters: HGE_TOT_POP, HGE_SEL_POP, HGE_DRAWN
  UIntArray tot_pop, sel_pop, num_drawn;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::HGE_TOT_POP, tot_pop);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::HGE_SEL_POP, sel_pop);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::HGE_DRAWN, num_drawn);
  std::vector<VariableParametersField> fields = {
      VariableParametersField("total_population", ResultsOutputType::UINTEGER),
      VariableParametersField("selected_population",
                              ResultsOutputType::UINTEGER),
      VariableParametersField("num_drawn", ResultsOutputType::UINTEGER),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, tot_pop, "total_population");
  hdf5Stream->set_vector_scalar_field(location, sel_pop, "selected_population");
  hdf5Stream->set_vector_scalar_field(location, num_drawn, "num_drawn");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_histogram_point_uncertain_int(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv type: Pecos::HISTOGRAM_PT_INT
  // parameters: Pecos::H_PT_INT_PAIRS
  IntRealMapArray bin_pairs;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::H_PT_INT_PAIRS, bin_pairs);
  // Because h5py barfs on vlen datasets of vlen strings, we have to
  // use regular, fixed-sized datasets that are big enough to hold the
  // maximum number of elements.
  size_t max_num_elements = 0;
  SizetArray num_elements;
  for (const auto &p : bin_pairs) {
    num_elements.push_back(p.size());
    max_num_elements =
        (max_num_elements > p.size()) ? max_num_elements : p.size();
  }
  // Populate a 1D array with ALL the elements, including padding
  IntArray abscissas(num_rv * max_num_elements, INT_DSET_FILL_VAL);
  RealArray counts(num_rv * max_num_elements, REAL_DSET_FILL_VAL);
  for (int i = 0; i < num_rv; ++i) {
    std::transform(bin_pairs[i].begin(), bin_pairs[i].end(),
                   &abscissas[i * max_num_elements],
                   [](const std::pair<int, Real> &p) { return p.first; });
    std::transform(bin_pairs[i].begin(), bin_pairs[i].end(),
                   &counts[i * max_num_elements],
                   [](const std::pair<int, Real> &p) { return p.second; });
  }

  std::vector<VariableParametersField> fields = {
      VariableParametersField("num_elements", ResultsOutputType::INTEGER),
      VariableParametersField("abscissas", ResultsOutputType::INTEGER,
                              {max_num_elements}),
      VariableParametersField("counts", ResultsOutputType::REAL,
                              {max_num_elements}),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, num_elements, "num_elements");
  hdf5Stream->set_vector_vector_field(location, abscissas, max_num_elements,
                                      "abscissas");
  hdf5Stream->set_vector_vector_field(location, counts, max_num_elements,
                                      "counts");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_histogram_point_uncertain_string(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv type: Pecos::HISTOGRAM_PT_STRING
  // parameters: Pecos::H_PT_STR_PAIRS
  StringRealMapArray bin_pairs;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::H_PT_STR_PAIRS, bin_pairs);
  // Because h5py barfs on vlen datasets of vlen strings, we have to
  // use regular, fixed-sized datasets that are big enough to hold the
  // maximum number of elements.
  size_t max_num_elements = 0;
  SizetArray num_elements;
  for (const auto &p : bin_pairs) {
    num_elements.push_back(p.size());
    max_num_elements =
        (max_num_elements > p.size()) ? max_num_elements : p.size();
  }
  // Populate a 1D array with ALL the elements, including padding
  StringArray abscissas(num_rv * max_num_elements, STR_DSET_FILL_VAL);
  RealArray counts(num_rv * max_num_elements, REAL_DSET_FILL_VAL);
  for (int i = 0; i < num_rv; ++i) {
    std::transform(bin_pairs[i].begin(), bin_pairs[i].end(),
                   &abscissas[i * max_num_elements],
                   [](const std::pair<String, Real> &p) { return p.first; });
    std::transform(bin_pairs[i].begin(), bin_pairs[i].end(),
                   &counts[i * max_num_elements],
                   [](const std::pair<String, Real> &p) { return p.second; });
  }

  std::vector<VariableParametersField> fields = {
      VariableParametersField("num_elements", ResultsOutputType::INTEGER),
      VariableParametersField("abscissas", ResultsOutputType::STRING,
                              {max_num_elements}),
      VariableParametersField("counts", ResultsOutputType::REAL,
                              {max_num_elements}),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, num_elements, "num_elements");
  hdf5Stream->set_vector_vector_field(location, abscissas, max_num_elements,
                                      "abscissas");
  hdf5Stream->set_vector_vector_field(location, counts, max_num_elements,
                                      "counts");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_histogram_point_uncertain_real(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv type: Pecos::HISTOGRAM_PT_REAL
  // parameters: Pecos::H_PT_REAL_PAIRS
  RealRealMapArray bin_pairs;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::H_PT_REAL_PAIRS, bin_pairs);
  // Because h5py barfs on vlen datasets of vlen strings, we have to
  // use regular, fixed-sized datasets that are big enough to hold the
  // maximum number of elements.
  size_t max_num_elements = 0;
  SizetArray num_elements;
  for (const auto &p : bin_pairs) {
    num_elements.push_back(p.size());
    max_num_elements =
        (max_num_elements > p.size()) ? max_num_elements : p.size();
  }
  // Populate a 1D array with ALL the elements, including padding
  RealArray abscissas(num_rv * max_num_elements, REAL_DSET_FILL_VAL);
  RealArray counts(num_rv * max_num_elements, REAL_DSET_FILL_VAL);
  for (int i = 0; i < num_rv; ++i) {
    std::transform(bin_pairs[i].begin(), bin_pairs[i].end(),
                   &abscissas[i * max_num_elements],
                   [](const std::pair<Real, Real> &p) { return p.first; });
    std::transform(bin_pairs[i].begin(), bin_pairs[i].end(),
                   &counts[i * max_num_elements],
                   [](const std::pair<Real, Real> &p) { return p.second; });
  }

  std::vector<VariableParametersField> fields = {
      VariableParametersField("num_elements", ResultsOutputType::INTEGER),
      VariableParametersField("abscissas", ResultsOutputType::REAL,
                              {max_num_elements}),
      VariableParametersField("counts", ResultsOutputType::REAL,
                              {max_num_elements}),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, num_elements, "num_elements");
  hdf5Stream->set_vector_vector_field(location, abscissas, max_num_elements,
                                      "abscissas");
  hdf5Stream->set_vector_vector_field(location, counts, max_num_elements,
                                      "counts");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_continuous_interval_uncertain(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv type: Pecos::CONTINUOUS_INTERVAL_UNCERTAIN
  // parameters: Pecos::CIU_BPA
  RealRealPairRealMapArray intervals;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::CIU_BPA, intervals);
  // Because h5py barfs on vlen datasets of vlen strings, we have to
  // use regular, fixed-sized datasets that are big enough to hold the
  // maximum number of elements.
  size_t max_num_elements = 0;
  SizetArray num_elements;
  for (const auto &p : intervals) {
    num_elements.push_back(p.size());
    max_num_elements =
        (max_num_elements > p.size()) ? max_num_elements : p.size();
  }
  // Populate a 1D array with ALL the elements, including padding
  RealArray probs(num_rv * max_num_elements, REAL_DSET_FILL_VAL);
  RealArray lbs(num_rv * max_num_elements, REAL_DSET_FILL_VAL);
  RealArray ubs(num_rv * max_num_elements, REAL_DSET_FILL_VAL);
  for (int i = 0; i < num_rv; ++i) {
    std::transform(intervals[i].begin(), intervals[i].end(),
                   &lbs[i * max_num_elements],
                   [](const std::pair<std::pair<Real, Real>, Real> &p) {
                     return p.first.first;
                   });
    std::transform(intervals[i].begin(), intervals[i].end(),
                   &ubs[i * max_num_elements],
                   [](const std::pair<std::pair<Real, Real>, Real> &p) {
                     return p.first.second;
                   });
    std::transform(intervals[i].begin(), intervals[i].end(),
                   &probs[i * max_num_elements],
                   [](const std::pair<std::pair<Real, Real>, Real> &p) {
                     return p.second;
                   });
  }

  std::vector<VariableParametersField> fields = {
      VariableParametersField("num_elements", ResultsOutputType::INTEGER),
      VariableParametersField("interval_probabilities", ResultsOutputType::REAL,
                              {max_num_elements}),
      VariableParametersField("lower_bounds", ResultsOutputType::REAL,
                              {max_num_elements}),
      VariableParametersField("upper_bounds", ResultsOutputType::REAL,
                              {max_num_elements}),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, num_elements, "num_elements");
  hdf5Stream->set_vector_vector_field(location, probs, max_num_elements,
                                      "interval_probabilities");
  hdf5Stream->set_vector_vector_field(location, lbs, max_num_elements,
                                      "lower_bounds");
  hdf5Stream->set_vector_vector_field(location, ubs, max_num_elements,
                                      "upper_bounds");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_discrete_interval_uncertain(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv type: Pecos::DISCRETE_INTERVAL_UNCERTAIN
  // parameters: Pecos::DIU_BPA
  IntIntPairRealMapArray intervals;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::DIU_BPA, intervals);
  // Because h5py barfs on vlen datasets of vlen strings, we have to
  // use regular, fixed-sized datasets that are big enough to hold the
  // maximum number of elements.
  size_t max_num_elements = 0;
  SizetArray num_elements;
  for (const auto &p : intervals) {
    num_elements.push_back(p.size());
    max_num_elements =
        (max_num_elements > p.size()) ? max_num_elements : p.size();
  }
  // Populate a 1D array with ALL the elements, including padding
  RealArray probs(num_rv * max_num_elements, REAL_DSET_FILL_VAL);
  IntArray lbs(num_rv * max_num_elements, INT_DSET_FILL_VAL);
  IntArray ubs(num_rv * max_num_elements, INT_DSET_FILL_VAL);
  for (int i = 0; i < num_rv; ++i) {
    std::transform(intervals[i].begin(), intervals[i].end(),
                   &lbs[i * max_num_elements],
                   [](const std::pair<std::pair<int, int>, Real> &p) {
                     return p.first.first;
                   });
    std::transform(intervals[i].begin(), intervals[i].end(),
                   &ubs[i * max_num_elements],
                   [](const std::pair<std::pair<int, int>, Real> &p) {
                     return p.first.second;
                   });
    std::transform(
        intervals[i].begin(), intervals[i].end(), &probs[i * max_num_elements],
        [](const std::pair<std::pair<int, int>, Real> &p) { return p.second; });
  }

  std::vector<VariableParametersField> fields = {
      VariableParametersField("num_elements", ResultsOutputType::INTEGER),
      VariableParametersField("interval_probabilities", ResultsOutputType::REAL,
                              {max_num_elements}),
      VariableParametersField("lower_bounds", ResultsOutputType::INTEGER,
                              {max_num_elements}),
      VariableParametersField("upper_bounds", ResultsOutputType::INTEGER,
                              {max_num_elements}),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, num_elements, "num_elements");
  hdf5Stream->set_vector_vector_field(location, probs, max_num_elements,
                                      "interval_probabilities");
  hdf5Stream->set_vector_vector_field(location, lbs, max_num_elements,
                                      "lower_bounds");
  hdf5Stream->set_vector_vector_field(location, ubs, max_num_elements,
                                      "upper_bounds");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_discrete_uncertain_set_int(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv type: Pecos::DISCRETE_UNCERTAIN_SET_INT
  // parameters: Pecos::DUSI_VALUES_PROBS
  IntRealMapArray sets;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::DUSI_VALUES_PROBS, sets);
  // Because h5py barfs on vlen datasets of vlen strings, we have to
  // use regular, fixed-sized datasets that are big enough to hold the
  // maximum number of elements.
  size_t max_num_elements = 0;
  IntArray num_elements;
  for (const auto &e : sets) {
    num_elements.push_back(e.size());
    max_num_elements =
        (max_num_elements > e.size()) ? max_num_elements : e.size();
  }
  // Populate a 1D array with ALL the elements, including padding
  IntArray elements(num_rv * max_num_elements, INT_DSET_FILL_VAL);
  RealArray probs(num_rv * max_num_elements, REAL_DSET_FILL_VAL);
  for (int i = 0; i < num_rv; ++i) {
    std::transform(sets[i].begin(), sets[i].end(),
                   &elements[i * max_num_elements],
                   [](const std::pair<int, Real> &p) { return p.first; });
    std::transform(sets[i].begin(), sets[i].end(), &probs[i * max_num_elements],
                   [](const std::pair<int, Real> &p) { return p.second; });
  }

  std::vector<VariableParametersField> fields = {
      VariableParametersField("num_elements", ResultsOutputType::INTEGER),
      VariableParametersField("elements", ResultsOutputType::INTEGER,
                              {max_num_elements}),
      VariableParametersField("set_probabilities", ResultsOutputType::REAL,
                              {max_num_elements}),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, num_elements, "num_elements");
  hdf5Stream->set_vector_vector_field(location, elements, max_num_elements,
                                      "elements");
  hdf5Stream->set_vector_vector_field(location, probs, max_num_elements,
                                      "set_probabilities");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_discrete_uncertain_set_string(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv type: Pecos::DISCRETE_UNCERTAIN_SET_STRING
  // parameters: Pecos::DUSS_VALUES_PROBS
  StringRealMapArray sets;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::DUSS_VALUES_PROBS, sets);
  // Because h5py barfs on vlen datasets of vlen strings, we have to
  // use regular, fixed-sized datasets that are big enough to hold the
  // maximum number of elements.
  size_t max_num_elements = 0;
  IntArray num_elements;
  for (const auto &e : sets) {
    num_elements.push_back(e.size());
    max_num_elements =
        (max_num_elements > e.size()) ? max_num_elements : e.size();
  }
  // Populate a 1D array with ALL the elements, including padding
  StringArray elements(num_rv * max_num_elements, STR_DSET_FILL_VAL);
  RealArray probs(num_rv * max_num_elements, REAL_DSET_FILL_VAL);
  for (int i = 0; i < num_rv; ++i) {
    std::transform(sets[i].begin(), sets[i].end(),
                   &elements[i * max_num_elements],
                   [](const std::pair<String, Real> &p) { return p.first; });
    std::transform(sets[i].begin(), sets[i].end(), &probs[i * max_num_elements],
                   [](const std::pair<String, Real> &p) { return p.second; });
  }

  std::vector<VariableParametersField> fields = {
      VariableParametersField("num_elements", ResultsOutputType::INTEGER),
      VariableParametersField("elements", ResultsOutputType::STRING,
                              {max_num_elements}),
      VariableParametersField("set_probabilities", ResultsOutputType::REAL,
                              {max_num_elements}),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, num_elements, "num_elements");
  hdf5Stream->set_vector_vector_field(location, elements, max_num_elements,
                                      "elements");
  hdf5Stream->set_vector_vector_field(location, probs, max_num_elements,
                                      "set_probabilities");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_discrete_uncertain_set_real(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv type: Pecos::DISCRETE_UNCERTAIN_SET_REAL
  // parameters: Pecos::DUSR_VALUES_PROBS
  RealRealMapArray sets;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::DUSR_VALUES_PROBS, sets);
  // Because h5py barfs on vlen datasets of vlen strings, we have to
  // use regular, fixed-sized datasets that are big enough to hold the
  // maximum number of elements.
  size_t max_num_elements = 0;
  IntArray num_elements;
  for (const auto &e : sets) {
    num_elements.push_back(e.size());
    max_num_elements =
        (max_num_elements > e.size()) ? max_num_elements : e.size();
  }
  // Populate a 1D array with ALL the elements, including padding
  RealArray elements(num_rv * max_num_elements, REAL_DSET_FILL_VAL);
  RealArray probs(num_rv * max_num_elements, REAL_DSET_FILL_VAL);
  for (int i = 0; i < num_rv; ++i) {
    std::transform(sets[i].begin(), sets[i].end(),
                   &elements[i * max_num_elements],
                   [](const std::pair<Real, Real> &p) { return p.first; });
    std::transform(sets[i].begin(), sets[i].end(), &probs[i * max_num_elements],
                   [](const std::pair<Real, Real> &p) { return p.second; });
  }

  std::vector<VariableParametersField> fields = {
      VariableParametersField("num_elements", ResultsOutputType::INTEGER),
      VariableParametersField("elements", ResultsOutputType::REAL,
                              {max_num_elements}),
      VariableParametersField("set_probabilities", ResultsOutputType::REAL,
                              {max_num_elements}),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, num_elements, "num_elements");
  hdf5Stream->set_vector_vector_field(location, elements, max_num_elements,
                                      "elements");
  hdf5Stream->set_vector_vector_field(location, probs, max_num_elements,
                                      "set_probabilities");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_continuous_state(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv type: Pecos::CONTINUOUS_RANGE
  // parameters: Pecos::CR_LWR_BND, Pecos::CR_UPR_BND
  RealArray lbs, ubs;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::CR_LWR_BND, lbs);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::CR_UPR_BND, ubs);
  std::vector<VariableParametersField> fields = {
      VariableParametersField("lower_bound", ResultsOutputType::REAL),
      VariableParametersField("upper_bound", ResultsOutputType::REAL)};
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, lbs, "lower_bound");
  hdf5Stream->set_vector_scalar_field(location, ubs, "upper_bound");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_discrete_state_range(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv type: Pecos::DISCRETE_RANGE
  // parameters: Pecos::DR_LWR_BND, Pecos::DR_UPR_BND
  IntArray lbs, ubs;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::DR_LWR_BND, lbs);
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::DR_UPR_BND, ubs);
  std::vector<VariableParametersField> fields = {
      VariableParametersField("lower_bound", ResultsOutputType::INTEGER),
      VariableParametersField("upper_bound", ResultsOutputType::INTEGER)};
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, lbs, "lower_bound");
  hdf5Stream->set_vector_scalar_field(location, ubs, "upper_bound");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_discrete_state_set_int(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv type: Pecos::DISCRETE_SET_INT
  // parameters: Pecos::DSI_VALUES
  IntSetArray isa;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::DSI_VALUES, isa);
  // Because h5py barfs on vlen datasets of vlen strings, we have to
  // use regular, fixed-sized datasets that are big enough to hold the
  // maximum number of elements.
  size_t max_num_elements = 0;
  IntArray num_elements;
  for (const auto &e : isa) {
    num_elements.push_back(e.size());
    max_num_elements =
        (max_num_elements > e.size()) ? max_num_elements : e.size();
  }
  // Populate a 1D array with ALL the elements, including padding
  IntArray all_elements(num_rv * max_num_elements, INT_DSET_FILL_VAL);
  for (int i = 0; i < num_rv; ++i)
    std::copy(isa[i].begin(), isa[i].end(),
              &all_elements[i * max_num_elements]);

  std::vector<VariableParametersField> fields = {
      VariableParametersField("num_elements", ResultsOutputType::INTEGER),
      VariableParametersField("elements", ResultsOutputType::INTEGER,
                              {max_num_elements}),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, num_elements, "num_elements");
  hdf5Stream->set_vector_vector_field(location, all_elements, max_num_elements,
                                      "elements");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_discrete_state_set_string(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv type: Pecos::DISCRETE_SET_INT
  // parameters: Pecos::DSI_VALUES
  StringSetArray ssa;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::DSS_VALUES, ssa);
  // Because h5py barfs on vlen datasets of vlen strings, we have to
  // use regular, fixed-sized datasets that are big enough to hold the
  // maximum number of elements.
  size_t max_num_elements = 0;
  IntArray num_elements;
  for (const auto &e : ssa) {
    num_elements.push_back(e.size());
    max_num_elements =
        (max_num_elements > e.size()) ? max_num_elements : e.size();
  }
  // Populate a 1D array with ALL the elements, including padding
  StringArray all_elements(num_rv * max_num_elements, STR_DSET_FILL_VAL);
  for (int i = 0; i < num_rv; ++i)
    std::copy(ssa[i].begin(), ssa[i].end(),
              &all_elements[i * max_num_elements]);

  std::vector<VariableParametersField> fields = {
      VariableParametersField("num_elements", ResultsOutputType::INTEGER),
      VariableParametersField("elements", ResultsOutputType::STRING,
                              {max_num_elements}),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, num_elements, "num_elements");
  hdf5Stream->set_vector_vector_field(location, all_elements, max_num_elements,
                                      "elements");
#else
  return;
#endif
}

void EvaluationStore::store_parameters_for_discrete_state_set_real(
    const size_t start_rv, const size_t num_rv, const String &location,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  // pecos rv type: Pecos::DISCRETE_SET_REAL
  // parameters: Pecos::DSR_VALUES
  RealSetArray rsa;
  mvd_rep->pull_parameters(start_rv, num_rv, Pecos::DSR_VALUES, rsa);
  // Because h5py barfs on vlen datasets of vlen strings, we have to
  // use regular, fixed-sized datasets that are big enough to hold the
  // maximum number of elements.
  size_t max_num_elements = 0;
  IntArray num_elements;
  for (const auto &e : rsa) {
    num_elements.push_back(e.size());
    max_num_elements =
        (max_num_elements > e.size()) ? max_num_elements : e.size();
  }
  // Populate a 1D array with ALL the elements, including padding
  RealArray all_elements(num_rv * max_num_elements, REAL_DSET_FILL_VAL);
  for (int i = 0; i < num_rv; ++i)
    std::copy(rsa[i].begin(), rsa[i].end(),
              &all_elements[i * max_num_elements]);

  std::vector<VariableParametersField> fields = {
      VariableParametersField("num_elements", ResultsOutputType::INTEGER),
      VariableParametersField("elements", ResultsOutputType::REAL,
                              {max_num_elements}),
  };
  IntArray dims = {int(num_rv)};
  hdf5Stream->create_empty_dataset(location, dims, fields);
  hdf5Stream->set_vector_scalar_field(location, num_elements, "num_elements");
  hdf5Stream->set_vector_vector_field(location, all_elements, max_num_elements,
                                      "elements");
#else
  return;
#endif
}

/// Store parameters for a single "domain" (e.g. all continuous variables)
void EvaluationStore::store_parameters_for_domain(
    const String &root_group, const UShortMultiArrayConstView &types,
    const SizetMultiArrayConstView &ids, const StringMultiArrayView &labels,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
#ifdef DAKOTA_HAVE_HDF5
  String scale_root = create_scale_root(root_group);  // root_group already has
                                                      // variable_parameters
  // The loop below chunks up the set of variables by Dakota type (e.g.
  // normal_uncertain)
  auto first_it = types.begin();  // iterator to first variable of this type
  size_t first_idx,
      last_idx;  // Indexes to first and last variable of this type.
  while (first_it !=
         types.end()) {  // iterate until all variables have been processed
    // Find iterator to last variable of this type
    UShortArray to_find = {*first_it};
    auto last_it =
        std::find_end(first_it, types.end(), to_find.begin(), to_find.end());
    first_idx = std::distance(types.begin(), first_it);
    last_idx = std::distance(first_it, last_it) + first_idx;
    const unsigned short &this_type = *first_it;
    // parameters are obtained from the mvd_rep object by starting index and
    // number of steps
    size_t start_rv = ids[first_idx] - 1;
    size_t num_rv = last_idx - first_idx + 1;
    bool store_scales = true;  // it's safe to store scales; will be set to
                               // false if no datasets are created, which can
                               // happen if there's a unhandled type of variable
    String location = root_group, scale_location = scale_root;
#define CALL_STORE_PARAMETERS_FOR(vtype)                             \
  location += #vtype;                                                \
  scale_location += #vtype;                                          \
  store_parameters_for_##vtype(start_rv, num_rv, location, mvd_rep); \
  break;

    switch (this_type) {
      case CONTINUOUS_DESIGN:
        CALL_STORE_PARAMETERS_FOR(continuous_design)
      case DISCRETE_DESIGN_RANGE:
        CALL_STORE_PARAMETERS_FOR(discrete_design_range)
      case DISCRETE_DESIGN_SET_INT:
        CALL_STORE_PARAMETERS_FOR(discrete_design_set_int)
      case DISCRETE_DESIGN_SET_STRING:
        CALL_STORE_PARAMETERS_FOR(discrete_design_set_string)
      case DISCRETE_DESIGN_SET_REAL:
        CALL_STORE_PARAMETERS_FOR(discrete_design_set_real)
      case NORMAL_UNCERTAIN:
        CALL_STORE_PARAMETERS_FOR(normal_uncertain);
      case UNIFORM_UNCERTAIN:
        CALL_STORE_PARAMETERS_FOR(uniform_uncertain);
      case LOGNORMAL_UNCERTAIN:
        CALL_STORE_PARAMETERS_FOR(lognormal_uncertain);
      case LOGUNIFORM_UNCERTAIN:
        CALL_STORE_PARAMETERS_FOR(loguniform_uncertain);
      case TRIANGULAR_UNCERTAIN:
        CALL_STORE_PARAMETERS_FOR(triangular_uncertain);
      case EXPONENTIAL_UNCERTAIN:
        CALL_STORE_PARAMETERS_FOR(exponential_uncertain);
      case BETA_UNCERTAIN:
        CALL_STORE_PARAMETERS_FOR(beta_uncertain);
      case GAMMA_UNCERTAIN:
        CALL_STORE_PARAMETERS_FOR(gamma_uncertain);
      case GUMBEL_UNCERTAIN:
        CALL_STORE_PARAMETERS_FOR(gumbel_uncertain);
      case FRECHET_UNCERTAIN:
        CALL_STORE_PARAMETERS_FOR(frechet_uncertain);
      case WEIBULL_UNCERTAIN:
        CALL_STORE_PARAMETERS_FOR(weibull_uncertain);
      case HISTOGRAM_BIN_UNCERTAIN:
        CALL_STORE_PARAMETERS_FOR(histogram_bin_uncertain);
      case POISSON_UNCERTAIN:
        CALL_STORE_PARAMETERS_FOR(poisson_uncertain);
      case BINOMIAL_UNCERTAIN:
        CALL_STORE_PARAMETERS_FOR(binomial_uncertain);
      case NEGATIVE_BINOMIAL_UNCERTAIN:
        CALL_STORE_PARAMETERS_FOR(negative_binomial_uncertain);
      case GEOMETRIC_UNCERTAIN:
        CALL_STORE_PARAMETERS_FOR(geometric_uncertain);
      case HYPERGEOMETRIC_UNCERTAIN:
        CALL_STORE_PARAMETERS_FOR(hypergeometric_uncertain);
      case HISTOGRAM_POINT_UNCERTAIN_INT:
        CALL_STORE_PARAMETERS_FOR(histogram_point_uncertain_int);
      case HISTOGRAM_POINT_UNCERTAIN_STRING:
        CALL_STORE_PARAMETERS_FOR(histogram_point_uncertain_string);
      case HISTOGRAM_POINT_UNCERTAIN_REAL:
        CALL_STORE_PARAMETERS_FOR(histogram_point_uncertain_real);
      case CONTINUOUS_INTERVAL_UNCERTAIN:
        CALL_STORE_PARAMETERS_FOR(continuous_interval_uncertain);
      case DISCRETE_INTERVAL_UNCERTAIN:
        CALL_STORE_PARAMETERS_FOR(discrete_interval_uncertain);
      case DISCRETE_UNCERTAIN_SET_INT:
        CALL_STORE_PARAMETERS_FOR(discrete_uncertain_set_int);
      case DISCRETE_UNCERTAIN_SET_STRING:
        CALL_STORE_PARAMETERS_FOR(discrete_uncertain_set_string);
      case DISCRETE_UNCERTAIN_SET_REAL:
        CALL_STORE_PARAMETERS_FOR(discrete_uncertain_set_real);
      case CONTINUOUS_STATE:
        CALL_STORE_PARAMETERS_FOR(continuous_state);
      case DISCRETE_STATE_RANGE:
        CALL_STORE_PARAMETERS_FOR(discrete_state_range);
      case DISCRETE_STATE_SET_INT:
        CALL_STORE_PARAMETERS_FOR(discrete_state_set_int);
      case DISCRETE_STATE_SET_STRING:
        CALL_STORE_PARAMETERS_FOR(discrete_state_set_string);
      case DISCRETE_STATE_SET_REAL:
        CALL_STORE_PARAMETERS_FOR(discrete_state_set_real);
      default:
        store_scales = false;  // if no cases were executed, then there's no
                               // dataset to add scales to.
    }
    if (store_scales) {
      StringMultiArrayConstView these_labels(
          labels[boost::indices[idx_range(first_idx, last_idx + 1)]]);
      SizetMultiArrayConstView these_ids(
          ids[boost::indices[idx_range(first_idx, last_idx + 1)]]);
      // Create descriptors dimension scale
      String labels_location = scale_location + "/labels";
      hdf5Stream->store_vector(labels_location, these_labels);
      hdf5Stream->attach_scale(location, labels_location, "labels", 0);
      // Create ids dimension scale
      String ids_location = scale_location + "/ids";
      hdf5Stream->store_vector(ids_location, these_ids);
      hdf5Stream->attach_scale(location, ids_location, "ids", 0);
    }
    // Increment to the next type
    first_it = ++last_it;
  }
#else
  return;
#endif
}

/// Allocate storage for variable paramters
void EvaluationStore::allocate_variable_parameters(
    const String &root_group, const Variables &variables,
    Pecos::MarginalsCorrDistribution *mvd_rep) {
  String parameters_group = root_group + "properties/variable_parameters/";
  if (variables.acv()) {
    store_parameters_for_domain(
        parameters_group, variables.all_continuous_variable_types(),
        variables.all_continuous_variable_ids(),
        variables.all_continuous_variable_labels(), mvd_rep);
  }
  if (variables.adiv()) {
    store_parameters_for_domain(
        parameters_group, variables.all_discrete_int_variable_types(),
        variables.all_discrete_int_variable_ids(),
        variables.all_discrete_int_variable_labels(), mvd_rep);
  }
  if (variables.adsv()) {
    store_parameters_for_domain(
        parameters_group, variables.all_discrete_string_variable_types(),
        variables.all_discrete_string_variable_ids(),
        variables.all_discrete_string_variable_labels(), mvd_rep);
  }
  if (variables.adrv()) {
    store_parameters_for_domain(
        parameters_group, variables.all_discrete_real_variable_types(),
        variables.all_discrete_real_variable_ids(),
        variables.all_discrete_real_variable_labels(), mvd_rep);
  }
}

/// Allocate storage for responses
void EvaluationStore::allocate_response(const String &root_group,
                                        const Response &response,
                                        const DefaultSet &set_s) {
#ifdef DAKOTA_HAVE_HDF5
  String response_root_group = root_group + "responses/";
  String scale_root = create_scale_root(root_group);
  String response_scale_root = scale_root + "responses/";
  String eval_ids = scale_root + "evaluation_ids";
  int num_functions = int(set_s.numFunctions);
  // Store function labels
  String function_labels_name = response_scale_root + "function_descriptors";
  hdf5Stream->store_vector(function_labels_name, response.function_labels());
  // Create functions dataset
  String functions_name = response_root_group + "functions";
  hdf5Stream->create_empty_dataset(functions_name, {0, num_functions},
                                   ResultsOutputType::REAL, HDF5_CHUNK_SIZE,
                                   &REAL_DSET_FILL_VAL);
  hdf5Stream->attach_scale(functions_name, eval_ids, "evaluation_ids", 0);
  hdf5Stream->attach_scale(functions_name, function_labels_name, "responses",
                           1);
  // Create gradients dataset, if needed
  const ShortArray &asv = set_s.set.request_vector();
  int num_gradients = set_s.numGradients;
  int num_hessians = set_s.numHessians;
  if (num_gradients) {
    int dvv_length = set_s.set.derivative_vector().size();
    String gradients_name = response_root_group + "gradients";
    hdf5Stream->create_empty_dataset(
        gradients_name, {0, num_gradients, dvv_length}, ResultsOutputType::REAL,
        HDF5_CHUNK_SIZE, &REAL_DSET_FILL_VAL);
    hdf5Stream->attach_scale(gradients_name, eval_ids, "evaluation_ids", 0);
    if (num_gradients == num_functions)
      hdf5Stream->attach_scale(gradients_name, function_labels_name,
                               "resposnes", 1);
    else {  // mixed gradients
      StringArray gradient_labels;
      const StringArray &function_labels = response.function_labels();
      for (int i = 0; i < num_functions; ++i)
        if (asv[i] & 2) gradient_labels.push_back(function_labels[i]);
      String gradient_labels_name =
          response_scale_root + "gradient_descriptors";
      hdf5Stream->store_vector(gradient_labels_name, gradient_labels);
      hdf5Stream->attach_scale(gradients_name, gradient_labels_name,
                               "responses", 1);
    }
  }
  if (num_hessians) {
    int dvv_length = set_s.set.derivative_vector().size();
    String hessians_name = response_root_group + "hessians";
    hdf5Stream->create_empty_dataset(
        hessians_name, {0, num_hessians, dvv_length, dvv_length},
        ResultsOutputType::REAL, HDF5_CHUNK_SIZE, &REAL_DSET_FILL_VAL);
    hdf5Stream->attach_scale(hessians_name, eval_ids, "evaluation_ids", 0);
    if (num_hessians == num_functions)
      hdf5Stream->attach_scale(hessians_name, function_labels_name, "resposnes",
                               1);
    else {  // mixed hessians
      StringArray hessian_labels;
      const StringArray &function_labels = response.function_labels();
      for (int i = 0; i < num_functions; ++i)
        if (asv[i] & 4) hessian_labels.push_back(function_labels[i]);
      String hessian_labels_name = response_scale_root + "hessian_descriptors";
      hdf5Stream->store_vector(hessian_labels_name, hessian_labels);
      hdf5Stream->attach_scale(hessians_name, hessian_labels_name, "responses",
                               1);
    }
  }
#else
  return;
#endif
}
/// Allocate storage for properties (ASV, DVV, analysis components)
void EvaluationStore::allocate_properties(const String &root_group,
                                          const Variables &variables,
                                          const Response &response,
                                          const DefaultSet &set_s,
                                          const String2DArray &an_comps) {
#ifdef DAKOTA_HAVE_HDF5
  String scale_root = create_scale_root(root_group);
  String properties_root = root_group + "properties/";
  String properties_scale_root = scale_root + "properties/";
  String eval_ids = scale_root + "evaluation_ids";
  const ShortArray &asv = set_s.set.request_vector();
  const SizetArray &dvv = set_s.set.derivative_vector();
  const int &num_functions = set_s.numFunctions;
  int num_deriv_vars = dvv.size();
  // ASV
  String asv_name = properties_root + "active_set_vector";
  hdf5Stream->create_empty_dataset(asv_name, {0, num_functions},
                                   ResultsOutputType::INTEGER, HDF5_CHUNK_SIZE);
  hdf5Stream->attach_scale(asv_name, eval_ids, "evaluation_ids", 0);
  hdf5Stream->attach_scale(
      asv_name, scale_root + "responses/function_descriptors", "responses", 1);
  hdf5Stream->store_vector(properties_scale_root + "default_asv", asv);
  hdf5Stream->attach_scale(asv_name, properties_scale_root + "default_asv",
                           "default_active_set_vector", 1);
  // DVV
  // only create a DVV dataset when gradients or hessians are available.

  if (set_s.numGradients || set_s.numHessians) {
    String dvv_name = properties_root + "derivative_variables_vector";
    hdf5Stream->create_empty_dataset(dvv_name, {0, num_deriv_vars},
                                     ResultsOutputType::INTEGER,
                                     HDF5_CHUNK_SIZE);
    hdf5Stream->attach_scale(dvv_name, eval_ids, "evaluation_ids", 0);
    // The ids are 1-based, not 0-based
    StringMultiArrayConstView cont_labels =
        variables.all_continuous_variable_labels();
    hdf5Stream->store_vector(properties_scale_root + "dv_descriptors",
                             cont_labels);
    hdf5Stream->attach_scale(dvv_name, properties_scale_root + "dv_descriptors",
                             "variables", 1);
    hdf5Stream->store_vector(properties_scale_root + "dvv", dvv);
    hdf5Stream->attach_scale(dvv_name, properties_scale_root + "dvv",
                             "variable_ids", 1);
  }
  // Analysis Components
  // TODO: these perhaps should be stored as a 2D dataset, with shape
  // (analysis drivers x components per driver)
  if (an_comps.size()) {
    StringArray all_comps;
    for (const auto &v : an_comps)
      all_comps.insert(all_comps.end(), v.begin(), v.end());
    hdf5Stream->store_vector(properties_root + "analysis_components",
                             all_comps);
  }
  return;
#endif
}

/// Allocate storage for metadata results
void EvaluationStore::allocate_metadata(const String &root_group,
                                        const Response &response) {
#ifdef DAKOTA_HAVE_HDF5
  const StringArray &metadata_labels = response.shared_data().metadata_labels();
  if (metadata_labels.empty()) return;

  String scale_root = create_scale_root(root_group);
  String metadata_root = root_group;
  String metadata_scale_root = scale_root;
  String eval_ids = scale_root + "evaluation_ids";
  String metadata_labels_name = scale_root + "metadata_descriptors";
  const int num_metadata = metadata_labels.size();

  hdf5Stream->store_vector(metadata_labels_name, metadata_labels);

  String metadata_name = metadata_root + "metadata";
  hdf5Stream->create_empty_dataset(metadata_name, {0, num_metadata},
                                   ResultsOutputType::REAL, HDF5_CHUNK_SIZE);
  hdf5Stream->attach_scale(metadata_name, eval_ids, "evaluation_ids", 0);
  hdf5Stream->attach_scale(metadata_name, metadata_labels_name, "metadata", 1);
#else
  return;
#endif
}

void EvaluationStore::store_variables(const String &root_group,
                                      const Variables &variables) {
#ifdef DAKOTA_HAVE_HDF5
  String variables_root = root_group + "variables/";
  if (variables.acv())
    hdf5Stream->append_vector(variables_root + "continuous",
                              variables.all_continuous_variables());
  if (variables.adiv())
    hdf5Stream->append_vector(variables_root + "discrete_integer",
                              variables.all_discrete_int_variables());
  if (variables.adsv())
    hdf5Stream->append_vector(variables_root + "discrete_string",
                              variables.all_discrete_string_variables());
  if (variables.adrv())
    hdf5Stream->append_vector(variables_root + "discrete_real",
                              variables.all_discrete_real_variables());
#else
  return;
#endif
}

void EvaluationStore::store_response(const String &root_group,
                                     const int &resp_idx,
                                     const Response &response,
                                     const DefaultSet &default_set_s) {
#ifdef DAKOTA_HAVE_HDF5
  String response_root = root_group + "responses/";
  const ActiveSet &set = response.active_set();
  const ShortArray &asv = set.request_vector();
  const SizetArray &dvv = set.derivative_vector();
  const size_t num_functions = asv.size();
  const ShortArray &default_asv = default_set_s.set.request_vector();
  const size_t num_default_deriv_vars =
      default_set_s.set.derivative_vector().size();
  const SizetArray &default_dvv = default_set_s.set.derivative_vector();
  // function values
  bool has_functions = bool(default_set_s.numFunctions);
  String functions_name = response_root + "functions";
  if (has_functions) {
    // because of NaN fill value, we have to do some legwork. If all of the
    // function values are set, we can write them all without making a copy. If
    // some of them are set, we have to make a copy (initizlied to NaN and write
    // just the values that are present. If none are set, we do nothing, because
    // tThe dataset by default has NaN fill values.
    const RealVector &f = response.function_values();
    int num1 = std::count_if(asv.begin(), asv.end(),
                             [](const short &a) { return a & 1; });
    if (num1 == num_functions) {
      hdf5Stream->set_vector(functions_name, f, resp_idx);
    } else if (num1 > 0) {
      RealVector f_copy(num_functions, false /*don't zero out */);
      f_copy = REAL_DSET_FILL_VAL;
      for (int i = 0; i < num_functions; ++i) {
        if (asv[i] & 1) f_copy[i] = f[i];
      }
      hdf5Stream->set_vector(functions_name, f_copy, resp_idx);
    }  // else, none are set, do nothing.
  }
  // Gradients. Gradients and hessians are more complicated than function values
  // for two reasons. 1) The dataset was allocated to accomodate the maximum dvv
  // length, and the current
  //    dvv length may be shorter
  // 2) The dataset was sized to hold gradients only for responses for which
  // they are
  //    available (i.e. mixed gradients), while Dakota (seems to) allocate space
  //    for every response.
  const int &num_gradients = default_set_s.numGradients;
  String gradients_name = response_root + "gradients";
  IntVector dvv_idx;  // indexes into the full gradient matrix of the deriv
                      // vars. Declare at this scope so it can be reused for
                      // Hessian storage, if needed
  if (num_gradients && std::any_of(asv.begin(), asv.end(),
                                   [](const short &a) { return a & 2; })) {
    // First do the simple case where the dvv is the same length as default dvv
    // and gradients are not mixed.
    if (dvv.size() == num_default_deriv_vars &&
        num_gradients == num_functions) {
      hdf5Stream->set_matrix(gradients_name, response.function_gradients(),
                             resp_idx, true /*transpose*/);
    } else {
      // Need to grab the gradients only for the subset of responses that can
      // have them, and then for those gradients, grab the components that are
      // in the dvv
      IntArray gradient_idxs;  // Indexes of responses that can have gradients
      for (int i = 0; i < num_functions; ++i)
        if (default_asv[i] & 2) gradient_idxs.push_back(i);
      const int num_default_gradients = gradient_idxs.size();
      RealMatrix full_gradients(num_default_deriv_vars, num_default_gradients,
                                false /*don't zero out*/);
      full_gradients = REAL_DSET_FILL_VAL;
      dvv_idx.resize(dvv.size());
      for (int i = 0; i < dvv.size(); ++i)
        dvv_idx[i] = find_index(default_dvv, dvv[i]);
      for (int i = 0; i < num_default_gradients; ++i) {
        const RealVector col =
            response.function_gradient_view(gradient_idxs[i]);
        for (int j = 0; j < dvv.size(); ++j) {
          full_gradients(dvv_idx[j], i) = col(j);
        }
      }
      hdf5Stream->set_matrix(gradients_name, full_gradients, resp_idx,
                             true /*transpose */);
    }
  }
  // Hessians. Same bookkeeping needs to be done here as for gradients.
  // Addditionally, the hessians have to be converted from symmetric matrices to
  // regular ones. (Probably should consider just storing them as row or column
  // major 1D arrays)
  const int &num_hessians = default_set_s.numHessians;
  String hessians_name = response_root + "hessians";
  if (num_hessians && std::any_of(asv.begin(), asv.end(),
                                  [](const short &a) { return a & 4; })) {
    // First do the simple case where the dvv is the same length as default dvv,
    // and hessians are not mixed.
    if (dvv.size() == num_default_deriv_vars && num_hessians == num_functions) {
      RealMatrixArray full_hessians;
      for (const auto &m : response.function_hessians()) {
        RealMatrix full_hessian(num_default_deriv_vars, num_default_deriv_vars,
                                false /*don't zero out*/);
        for (int i = 0; i < num_default_deriv_vars; ++i) {
          full_hessian(i, i) = m(i, i);
          for (int j = i + 1; j < num_default_deriv_vars; ++j) {
            full_hessian(j, i) = full_hessian(i, j) = m(i, j);
          }
        }
        full_hessians.push_back(full_hessian);
      }
      hdf5Stream->set_vector_matrix(hessians_name, full_hessians, resp_idx,
                                    true /*transpose (for efficiency)*/);
    } else {
      IntArray hessian_idxs;  // Indexes of responses that can have hessians
      for (int i = 0; i < num_functions; ++i)
        if (default_asv[i] & 4) hessian_idxs.push_back(i);
      int num_default_hessians = hessian_idxs.size();
      RealMatrixArray
          full_hessians;      //(num_default_deriv_vars, num_default_gradients);
      if (dvv_idx.empty()) {  // not yet populated by gradient storage block
        dvv_idx.resize(dvv.size());
        for (int i = 0; i < dvv.size(); ++i)
          dvv_idx[i] = find_index(default_dvv, dvv[i]);
      }
      for (int mi = 0; mi < num_default_hessians; ++mi) {
        RealMatrix full_hessian(num_default_deriv_vars, num_default_deriv_vars,
                                false /*don't zero out*/);
        full_hessian = REAL_DSET_FILL_VAL;
        const RealSymMatrix &resp_hessian =
            response.function_hessian_view(hessian_idxs[mi]);
        for (int i = 0; i < dvv.size(); ++i) {
          const int &dvv_i = dvv_idx[i];
          full_hessian(dvv_i, dvv_i) = resp_hessian(i, i);
          for (int j = i + 1; j < dvv.size(); ++j) {
            const int &dvv_j = dvv_idx[j];
            full_hessian(dvv_j, dvv_i) = full_hessian(dvv_i, dvv_j) =
                resp_hessian(i, j);
          }
        }
        full_hessians.push_back(full_hessian);
      }
      hdf5Stream->set_vector_matrix(hessians_name, full_hessians, resp_idx,
                                    true /*transpose */);
    }
  }
#else
  return;
#endif
}

void EvaluationStore::store_properties(const String &root_group,
                                       const ActiveSet &set,
                                       const DefaultSet &default_set_s) {
#ifdef DAKOTA_HAVE_HDF5
  String properties_root = root_group + "properties/";
  hdf5Stream->append_vector(properties_root + "active_set_vector",
                            set.request_vector());
  // DVV. The dvv in set may be shorter than the default one, and so it has to
  // be properties  // by ID.
  const SizetArray &default_dvv = default_set_s.set.derivative_vector();
  const ShortArray &default_asv = default_set_s.set.request_vector();
  // The DVV dataset doesn't exist unless gradients or hessians can be provided
  if (default_set_s.numGradients || default_set_s.numHessians) {
    const SizetArray &dvv = set.derivative_vector();
    // vector that will be apppended to the dataset. "bits" defaulted to 0
    // ("off")
    IntArray dvv_row(default_dvv.size(), 0);
    // Most of the time, all possible derivative variables will be "active" (the
    // lengths of the current and default DVV will match), so we don't need to
    // examine the DVV entry by entry.
    if (dvv.size() == default_dvv.size())
      std::fill(dvv_row.begin(), dvv_row.end(), 1);
    else {
      // This logic assumes that the entries in dvv and default_dvv are sorted
      // in ascending order. It iterates over the entries of the current dvv,
      // and for each, advances through the default dvv until the entry is
      // found. It then sets the bit for that entry and goes to the next one in
      // the current dvv.
      int di = 0;
      for (int si = 0; si < dvv.size(); ++si) {
        for (; di < default_dvv.size(); ++di) {
          if (dvv[si] == default_dvv[di]) {
            dvv_row[di] = 1;
            ++di;
            break;
          }
        }
      }
    }
    hdf5Stream->append_vector(properties_root + "derivative_variables_vector",
                              dvv_row);
  }
  return;
#endif
}

void EvaluationStore::store_metadata(const String &root_group,
                                     const int &resp_idx,
                                     const Response &response) {
#ifdef DAKOTA_HAVE_HDF5
  const auto &metadata = response.metadata();
  if (metadata.empty()) return;
  const size_t num_metadata = metadata.size();
  String metadata_name = root_group + "metadata";

  hdf5Stream->set_vector(metadata_name, metadata, resp_idx);
#else
  return;
#endif
}

void EvaluationStore::model_selection(const unsigned short &selection) {
  modelSelection = selection;
}

void EvaluationStore::interface_selection(const unsigned short &selection) {
  interfaceSelection = selection;
}

bool EvaluationStore::model_active(const String &model_id) {
  if (modelSelection == MODEL_EVAL_STORE_ALL)
    return true;
  else if (modelSelection == MODEL_EVAL_STORE_NONE)
    return false;
  else  // MODEL_EVAL_STORE_TOP_METHOD and ALL_METHODS
    return sourceModels.find(model_id) != sourceModels.end();
}

bool EvaluationStore::interface_active(const String &iface_type) {
  if (interfaceSelection == INTERF_EVAL_STORE_ALL)
    return true;
  else if (interfaceSelection == INTERF_EVAL_STORE_NONE)
    return false;
  else if (iface_type == "approximation")  // simulation only
    return false;
  else
    return true;
}

}  // namespace Dakota
