/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        EvaluationStore
//- Description:  Class implementation
//- Owner:        J. Adam Stephens
#include <memory>
#include <algorithm>
#include <tuple>
#include "EvaluationStore.hpp"
#ifdef DAKOTA_HAVE_HDF5
#include "HDF5_IO.hpp"
#endif
#include "DakotaVariables.hpp"
#include "DakotaResponse.hpp"
#include "DakotaActiveSet.hpp"
#include "dakota_data_types.hpp"

namespace Dakota {

DefaultSet::DefaultSet(const ActiveSet &in_set) : set(in_set) {
  const ShortArray &asv = set.request_vector();
  numFunctions = asv.size();
  numGradients = std::count_if(asv.begin(),
                                asv.end(),
              [](const short &a){return a & 2;});
  numHessians = std::count_if(asv.begin(),
                                asv.end(),
              [](const short &a){return a & 4;});
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

// Declare a source for the mdoel or iterator. 
// Permissible values of owner_type are "iterator",
//  "nested", "surrogate", "recast", and "simulation".
// Permissible values of source_type are "iterator", "nested", "surrogate",
// "recast", "simulation", "interface", and "approximation".
void EvaluationStore::
declare_source(const String &owner_id, const String &owner_type,
               const String &source_id, const String &source_type) {
  if(!active())
    return;
#ifdef DAKOTA_HAVE_HDF5
  // Location of source model or interface evals or method results
  String source_location;
  // Location of the link to the source
  String link_location;
  // TODO: Report/raise some kind of error for invalid owner or source strings

  if(owner_type == "iterator") {
    link_location = String("/methods/") + owner_id + "/sources/" + source_id;
    if(source_type == "iterator") { // always link iterator sources
      source_location = String("/methods/") + source_id;
      hdf5Stream->create_softlink(link_location, source_location);
    } else { // source is a model
      if( (modelSelection == MODEL_EVAL_STORE_TOP_METHOD && owner_id == topLevelMethodId) || 
           modelSelection == MODEL_EVAL_STORE_ALL_METHODS )
        sourceModels.emplace(source_id);
      if(model_active(source_id)) { // Only link if evals for this model will be stored
        source_location = String("/models/") + source_type + "/" + source_id; 
        hdf5Stream->create_softlink(link_location, source_location);
      }
    } 
  } else { // owner is a model. Assume it should be stored.
    link_location = String("/models/") + owner_type + "/" + owner_id + "/sources/" + source_id;
    if(source_type == "iterator") {
      source_location = String("/methods/") + source_id;
      hdf5Stream->create_softlink(link_location, source_location);
    } else if(source_type == "interface" && interface_active(source_type)) {
      source_location = String("/interfaces/") + source_id + "/" + owner_id;
      hdf5Stream->create_softlink(link_location, source_location);
    }
    else if(model_active(source_id)) { // source is a model
      source_location = String("/models/") + source_type + "/" + source_id;
      hdf5Stream->create_softlink(link_location, source_location);
    }
  }
  // group creation step not needed for soft links
#endif
}

EvaluationsDBState EvaluationStore::iterator_allocate(const String &iterator_id,
    const bool &top_level) {
  if(!active())
    return EvaluationsDBState::INACTIVE;
  if(top_level)
    topLevelMethodId = iterator_id;
  return EvaluationsDBState::ACTIVE;
}

/// Allocate storage for model evaluations
EvaluationsDBState EvaluationStore::model_allocate(const String &model_id, const String &model_type, 
                    const Variables &variables, const Response &response,
                    const ActiveSet &set) {
  if(! (active() && model_active(model_id)))
    return EvaluationsDBState::INACTIVE;
#ifdef DAKOTA_HAVE_HDF5
  //Cout << "EvaluationStore::model_allocate()\nmodel_id: " << model_id << "\nmodel_type: " << model_type << std::endl;
  allocatedModels.emplace(model_id);
  const auto & ds_pair = modelDefaultSets.emplace(model_id, DefaultSet(set));
  const DefaultSet &default_set = (*ds_pair.first).second;
  String root_group = create_model_root(model_id, model_type);
  String scale_root = create_scale_root(root_group);
  // Create evaluation ID dataset, which is attached as a scale to many datasets
  String eval_ids_scale = scale_root + "evaluation_ids";
  hdf5Stream->create_empty_dataset(eval_ids_scale, {0}, 
      ResultsOutputType::REAL, HDF5_CHUNK_SIZE);
  
  allocate_variables(root_group, variables);
  allocate_response(root_group, response, default_set);
  allocate_metadata(root_group, variables, response, default_set);
  return EvaluationsDBState::ACTIVE;
#endif
}

/// Allocate storage for evalulations of interface+model pairs
EvaluationsDBState EvaluationStore::interface_allocate(const String &model_id, const String &interface_id,
                    const String &interface_type, const Variables &variables, const Response &response,
                    const ActiveSet &set, const String2DArray &an_comp) {
  if(!(active() && interface_active(interface_type)))
    return EvaluationsDBState::INACTIVE;
#ifdef DAKOTA_HAVE_HDF5
  //Cout << "EvaluationStore::interface_allocate()\ninterface_id: " << interface_id << "\nmodel_id: " << model_id << std::endl;
  allocatedInterfaces.emplace(make_pair(model_id, interface_id));
  const auto & ds_pair = interfaceDefaultSets.emplace(std::make_pair(model_id, interface_id), DefaultSet(set));
  const DefaultSet &default_set = (*ds_pair.first).second;
  String root_group = create_interface_root(model_id, interface_id);
  String scale_root = create_scale_root(root_group);
  // Create evaluation ID dataset, which is attached as a scale to many datasets
  String eval_ids_scale = scale_root + "evaluation_ids";
  hdf5Stream->create_empty_dataset(eval_ids_scale, {0}, 
      ResultsOutputType::REAL, HDF5_CHUNK_SIZE);
  
  allocate_variables(root_group, variables);
  allocate_response(root_group, response, default_set);
  allocate_metadata(root_group, variables, response, default_set, an_comp);
  return EvaluationsDBState::ACTIVE;
#endif
}

/// Store a model evaluation
void EvaluationStore::store_model_variables(const String &model_id, const String &model_type, 
                            const int &eval_id, const ActiveSet &set, const Variables &variables) {
  if(!active())
    return;
#ifdef DAKOTA_HAVE_HDF5
  //Cout << "EvaluationStore::store_model_variables()\nmodel_id: " << 
  //  model_id << "\nmodel_type: " << model_type << 
  //  "\neval_id: " << eval_id << std::endl;
  String root_group = create_model_root(model_id, model_type);
  String scale_root = create_scale_root(root_group);
  const DefaultSet &default_set_s = modelDefaultSets[model_id]; 
  // Create evaluation ID dataset, which is attached as a scale to many datasets
  String eval_ids_scale = scale_root + "evaluation_ids";
  hdf5Stream->append_scalar(eval_ids_scale, eval_id);
  store_variables(root_group, variables);
  store_metadata(root_group, set, default_set_s);
 

  int resp_idx = hdf5Stream->append_empty(root_group + "responses/functions");
  const ShortArray &default_asv = default_set_s.set.request_vector();
  if( default_set_s.numGradients )
    hdf5Stream->append_empty(root_group + "responses/gradients");
  if( default_set_s.numHessians ) 
    hdf5Stream->append_empty(root_group + "responses/hessians");
  modelResponseIndexCache.emplace(std::make_tuple(model_id, eval_id), resp_idx);
#endif
}

/// Store a response for model evaluation
void EvaluationStore::store_model_response(const String &model_id, const String &model_type, 
                            const int &eval_id, const Response &response) {
  if(!active())
    return;
#ifdef DAKOTA_HAVE_HDF5
  std::tuple<String, int> key(model_id, eval_id);
  int response_index = modelResponseIndexCache[key];
  String root_group = create_model_root(model_id, model_type);
  store_response(root_group, response_index, response, modelDefaultSets[model_id]);
  auto cache_entry = modelResponseIndexCache.find(key);
  modelResponseIndexCache.erase(cache_entry);
#endif
}

/// Store variables for an interface+model evaluation
void EvaluationStore::store_interface_variables(const String &model_id, const String &interface_id, 
                            const int &eval_id, const ActiveSet &set, const Variables &variables) {
  if(!active())
    return;
#ifdef DAKOTA_HAVE_HDF5
  //Cout << "EvaluationStore::store_interface_variables()\nmodel_id: " << 
  //  model_id << "\ninterface_id: " << interface_id << 
  //  "\neval_id: " << eval_id << std::endl;
  String root_group = create_interface_root(model_id, interface_id);
  String scale_root = create_scale_root(root_group);
  const auto set_key = std::make_pair(model_id, interface_id);
  const DefaultSet &default_set_s = interfaceDefaultSets[set_key];
  // Create evaluation ID dataset, which is attached as a scale to many datasets
  String eval_ids_scale = scale_root + "evaluation_ids";
  hdf5Stream->append_scalar(eval_ids_scale, eval_id);
  store_variables(root_group, variables);
  store_metadata(root_group, set, default_set_s);
  
  int resp_idx = hdf5Stream->append_empty(root_group + "responses/functions");
  if( default_set_s.numGradients) 
    hdf5Stream->append_empty(root_group + "responses/gradients");
  if( default_set_s.numHessians) 
    hdf5Stream->append_empty(root_group + "responses/hessians");
  interfaceResponseIndexCache.emplace(std::make_tuple(model_id, interface_id, eval_id), resp_idx);
#endif
}

/// Store a response for an interface+model evaluation
void EvaluationStore::store_interface_response(const String &model_id, const String &interface_id, 
                            const int &eval_id, const Response &response) {
  if(!active())
    return;
#ifdef DAKOTA_HAVE_HDF5
  std::tuple<String, String, int> key(model_id, interface_id, eval_id);
  int response_index = interfaceResponseIndexCache[key];
  String root_group = create_interface_root(model_id, interface_id);
  store_response(root_group, response_index, response, interfaceDefaultSets[std::make_pair(model_id, interface_id)]);
  auto cache_entry = interfaceResponseIndexCache.find(key);
  interfaceResponseIndexCache.erase(cache_entry);
#endif
}

String EvaluationStore::create_interface_root(const String &model_id, const String &interface_id) {
  return String("/interfaces/") + interface_id + '/' + model_id + '/';
}

String EvaluationStore::create_model_root(const String &model_id, const String &model_type) {
  return String("/models/") + model_type + '/' + model_id + '/';
}

String EvaluationStore::create_scale_root(const String &root_group) {
  return String("/_scales") + root_group;
}

/// Allocate storage for variables
void EvaluationStore::allocate_variables(const String &root_group, const Variables &variables) {
  // TODO: variable names and order
#ifdef DAKOTA_HAVE_HDF5
  String variables_root_group = root_group + "variables/";
  String scale_root = create_scale_root(root_group);
  String variables_scale_root = scale_root + "variables/";
  String eval_ids = scale_root + "evaluation_ids";

  if(variables.acv()) {
    String data_name = variables_root_group + "continuous";
    String labels_name = variables_scale_root + "continuous_descriptors";
    hdf5Stream->create_empty_dataset(data_name, {0, int(variables.acv())}, 
        ResultsOutputType::REAL, HDF5_CHUNK_SIZE);
    hdf5Stream->store_vector(labels_name,
                             variables.all_continuous_variable_labels());
    hdf5Stream->attach_scale(data_name, eval_ids, "evaluation_ids", 0);
    hdf5Stream->attach_scale(data_name, labels_name, "variables", 1);
  }

  if(variables.adiv()) {
    String data_name = variables_root_group + "discrete_integer";
    String labels_name = variables_scale_root + "discrete_integer_descriptors";
    hdf5Stream->create_empty_dataset(data_name, {0, int(variables.adiv())}, 
        ResultsOutputType::INTEGER, HDF5_CHUNK_SIZE);
    hdf5Stream->store_vector(labels_name,
                             variables.all_discrete_int_variable_labels());
    hdf5Stream->attach_scale(data_name, eval_ids, "evaluation_ids", 0);
    hdf5Stream->attach_scale(data_name, labels_name, "variables", 1);
  }

  if(variables.adsv()) {
    String data_name = variables_root_group + "discrete_string";
    String labels_name = variables_scale_root + "discrete_string_descriptors";
    hdf5Stream->create_empty_dataset(data_name, {0, int(variables.adsv())}, 
        ResultsOutputType::STRING, HDF5_CHUNK_SIZE);
    hdf5Stream->store_vector(labels_name,
                             variables.all_discrete_string_variable_labels());
    hdf5Stream->attach_scale(data_name, eval_ids, "evaluation_ids", 0);
    hdf5Stream->attach_scale(data_name, labels_name, "variables", 1);
  }

  if(variables.adrv()) {
    String data_name = variables_root_group + "discrete_real";
    String labels_name = variables_scale_root + "discrete_real_descriptors";
    hdf5Stream->create_empty_dataset(data_name, {0, int(variables.adrv())}, 
        ResultsOutputType::REAL, HDF5_CHUNK_SIZE);
    hdf5Stream->store_vector(labels_name,
                             variables.all_discrete_real_variable_labels());
    hdf5Stream->attach_scale(data_name, eval_ids, "evaluation_ids", 0);
    hdf5Stream->attach_scale(data_name, labels_name, "variables", 1);
  }
#endif
}

/// Allocate storage for responses
void EvaluationStore::allocate_response(const String &root_group, const Response &response,
    const DefaultSet &set_s) {
#ifdef DAKOTA_HAVE_HDF5
  String response_root_group = root_group + "responses/";
  String scale_root = create_scale_root(root_group);
  String response_scale_root = scale_root + "responses/";
  String eval_ids = scale_root + "evaluation_ids";
  int num_functions = int(response.num_functions());
  // Store function labels
  String function_labels_name = response_scale_root + "function_descriptors";
  hdf5Stream->store_vector(function_labels_name, response.function_labels());
  // Create functions dataset
  String functions_name = response_root_group + "functions";
  hdf5Stream->create_empty_dataset(functions_name, {0, num_functions}, 
      ResultsOutputType::REAL, HDF5_CHUNK_SIZE);
  hdf5Stream->attach_scale(functions_name, eval_ids, "evaluation_ids", 0);
  hdf5Stream->attach_scale(functions_name, function_labels_name, "responses", 1);
  // Create gradients dataset, if needed
  const ShortArray &asv = set_s.set.request_vector();
  int num_gradients = set_s.numGradients;
  int num_hessians =  set_s.numHessians; 
  if(num_gradients) {
    int dvv_length = set_s.set.derivative_vector().size();
    String gradients_name = response_root_group + "gradients";
    hdf5Stream->create_empty_dataset(gradients_name, {0, num_gradients, dvv_length},
      ResultsOutputType::REAL, HDF5_CHUNK_SIZE);
    hdf5Stream->attach_scale(gradients_name, eval_ids, "evaluation_ids", 0);
    if(num_gradients == num_functions)
      hdf5Stream->attach_scale(gradients_name, function_labels_name, "resposnes", 1);
    else { // mixed gradients
      StringArray gradient_labels;
      const StringArray & function_labels = response.function_labels();
      for(int i = 0; i < num_functions; ++i)
        if(asv[i] & 2)
          gradient_labels.push_back(function_labels[i]);
      String gradient_labels_name = response_scale_root + "gradient_descriptors";
      hdf5Stream->store_vector(gradient_labels_name, gradient_labels);
      hdf5Stream->attach_scale(gradients_name, gradient_labels_name, "responses", 1);
    }
  }
  if(num_hessians) {
    int dvv_length = set_s.set.derivative_vector().size();
    String hessians_name = response_root_group + "hessians";
    hdf5Stream->create_empty_dataset(hessians_name, {0, num_hessians, dvv_length, dvv_length},
      ResultsOutputType::REAL, HDF5_CHUNK_SIZE);
    hdf5Stream->attach_scale(hessians_name, eval_ids, "evaluation_ids", 0);
    if(num_hessians == num_functions)
      hdf5Stream->attach_scale(hessians_name, function_labels_name, "resposnes", 1);
    else { // mixed hessians
      StringArray hessian_labels;
      const StringArray & function_labels = response.function_labels();
      for(int i = 0; i < num_functions; ++i)
        if(asv[i] & 4)
          hessian_labels.push_back(function_labels[i]);
      String hessian_labels_name = response_scale_root + "hessian_descriptors";
      hdf5Stream->store_vector(hessian_labels_name, hessian_labels);
      hdf5Stream->attach_scale(hessians_name, hessian_labels_name, "responses", 1);
    }
  }
#endif
}
/// Allocate storage for metadata
void EvaluationStore::allocate_metadata(const String &root_group, const Variables &variables, 
    const Response &response, const DefaultSet &set_s, const String2DArray &an_comps) {
#ifdef DAKOTA_HAVE_HDF5
  String scale_root = create_scale_root(root_group);
  String metadata_root = root_group + "metadata/";
  String metadata_scale_root = scale_root + "metadata/";
  String eval_ids = scale_root + "evaluation_ids";
  const ShortArray &asv = set_s.set.request_vector();
  const SizetArray &dvv = set_s.set.derivative_vector();
  const int &num_functions = set_s.numFunctions;
  int num_deriv_vars = dvv.size();
  // ASV
  String asv_name = metadata_root + "active_set_vector";
  hdf5Stream->create_empty_dataset(asv_name, {0, num_functions}, ResultsOutputType::INTEGER, HDF5_CHUNK_SIZE);
  hdf5Stream->attach_scale(asv_name, eval_ids, "evaluation_ids", 0);
  hdf5Stream->attach_scale(asv_name, scale_root+"responses/function_descriptors", "responses", 1);
  hdf5Stream->store_vector(metadata_scale_root + "default_asv", asv);
  hdf5Stream->attach_scale(asv_name, metadata_scale_root + "default_asv", "default_active_set_vector", 1);
  // DVV
  // only create a DVV dataset when gradients or hessians are available.

  if(set_s.numGradients || set_s.numHessians) {
    String dvv_name = metadata_root + "derivative_variables_vector";
    hdf5Stream->create_empty_dataset(dvv_name, {0, num_deriv_vars}, ResultsOutputType::INTEGER,  HDF5_CHUNK_SIZE);
    hdf5Stream->attach_scale(dvv_name, eval_ids, "evaluation_ids", 0);
    // The ids are 1-based, not 0-based
    SizetMultiArrayConstView cont_ids = variables.all_continuous_variable_ids();
    StringMultiArrayConstView cont_labels = variables.all_continuous_variable_labels();
    StringArray dv_labels(num_deriv_vars);
    
    for(int i = 0; i < dvv.size(); ++i) {
      const size_t idx = find_index(cont_ids, dvv[i]);
      dv_labels[i] = cont_labels[idx];
    }
    hdf5Stream->store_vector(metadata_scale_root + "dv_descriptors", dv_labels);
    hdf5Stream->attach_scale(dvv_name, metadata_scale_root + "dv_descriptors", "variables", 1);
    hdf5Stream->store_vector(metadata_scale_root + "dvv", dvv);
    hdf5Stream->attach_scale(dvv_name, metadata_scale_root + "dvv", "variable_ids", 1);
  }
  // Analysis Components
  // TODO: these perhaps should be stored as a 2D dataset
  if(an_comps.size()) {
    StringArray all_comps;
    for(const auto &v : an_comps)
      all_comps.insert(all_comps.end(), v.begin(), v.end());
    hdf5Stream->store_vector(metadata_root, all_comps);
  }
#endif
}


void EvaluationStore::store_variables(const String &root_group, const Variables &variables) {
#ifdef DAKOTA_HAVE_HDF5
  String variables_root = root_group + "variables/";
  if(variables.acv())
    hdf5Stream->append_vector(variables_root+"continuous",
        variables.all_continuous_variables());
  if(variables.adiv())
    hdf5Stream->append_vector(variables_root+"discrete_integer",
        variables.all_discrete_int_variables());
  if(variables.adsv())
    hdf5Stream->append_vector(variables_root+"discrete_string",
        variables.all_discrete_string_variables());
  if(variables.adrv())
    hdf5Stream->append_vector(variables_root+"discrete_real",
        variables.all_discrete_real_variables());
#endif
}

void EvaluationStore::store_response(const String &root_group, const int &resp_idx, 
    const Response &response, const DefaultSet &default_set_s) {
#ifdef DAKOTA_HAVE_HDF5
  String response_root = root_group + "responses/";
  const ActiveSet &set = response.active_set();
  const ShortArray &asv = set.request_vector();
  const SizetArray &dvv = set.derivative_vector();
  const size_t num_functions = asv.size();
  const ShortArray &default_asv = default_set_s.set.request_vector();
  const size_t num_default_deriv_vars = default_set_s.set.derivative_vector().size();
  // function values
  bool has_functions = bool(default_set_s.numFunctions); 
  String functions_name = response_root + "functions";
  if(has_functions)
    hdf5Stream->set_vector(functions_name, response.function_values(), resp_idx);
  // Gradients. Gradients and hessians are more complicated than function values for two reasons.
  // 1) The dataset was allocated to accomodate the maximum dvv length, and the current
  //    dvv length may be shorter
  // 2) The dataset was sized to hold gradients only for responses for which they are
  //    available (i.e. mixed gradients), while Dakota (seems to) allocate space for every 
  //    response.
  const int &num_gradients = default_set_s.numGradients;
  String gradients_name = response_root + "gradients";
  if(num_gradients) {
    // First do the simple case where the dvv is the same length as default dvv and gradients are 
    // not mixed
    if(dvv.size() == num_default_deriv_vars && num_gradients == num_functions) {
        hdf5Stream->set_matrix(gradients_name, response.function_gradients(), resp_idx, true /*transpose*/);
    } else {
      // Need to grab the gradients only for the subset of responses that can have them, and then
      // for those gradients, grab the components that are in the dvv
      IntArray gradient_idxs; // Indexes of responses that can have gradients
      for(int i = 0; i < num_functions; ++i)
        if(default_asv[i] & 2)
          gradient_idxs.push_back(i);    
      const int num_default_gradients = gradient_idxs.size();
      RealMatrix full_gradients(num_default_deriv_vars, num_default_gradients);
      for(int i = 0; i < num_default_gradients; ++i) {
        const RealVector col = response.function_gradient_view(gradient_idxs[i]);
        for(int j = 0; j < dvv.size(); ++j) {
          full_gradients(dvv[j]-1, i) = col(j);
        }
      }
      hdf5Stream->set_matrix(gradients_name, full_gradients, resp_idx, true /*transpose */);
    }
  } 
  // Hessians. Same bookkeeping needs to be done here as for gradients. Addditionally, the
  // hessians have to be converted from symmetric matrices to regular ones. (Probably should
  // consider just storing them as row or column major 1D arrays)
  const int &num_hessians = default_set_s.numHessians;
  String hessians_name = response_root + "hessians";
  if(num_hessians) {
    // First do the simple case where the dvv is the same length as default dvv, and
    // hessians are not mixed.
    if(dvv.size() == num_default_deriv_vars && num_hessians == num_functions) {
      RealMatrixArray full_hessians;
      for(const auto &m : response.function_hessians()) {
        RealMatrix full_hessian(num_default_deriv_vars, num_default_deriv_vars);
        for(int i = 0; i < num_default_deriv_vars; ++i) {
          full_hessian(i, i) = m(i, i);
          for(int j = i+1; j < num_default_deriv_vars; ++j) {
            full_hessian(j,i) = full_hessian(i,j) = m(i,j);
          }
        }
        full_hessians.push_back(full_hessian);
      }
      hdf5Stream->set_vector_matrix(hessians_name, full_hessians, resp_idx, true /*transpose (for efficiency)*/);
    } else {
      IntArray hessian_idxs; // Indexes of responses that can have hessians
      for(int i = 0; i < num_functions; ++i)
        if(default_asv[i] & 4)
          hessian_idxs.push_back(i);    
      int num_default_hessians = hessian_idxs.size();
      RealMatrixArray full_hessians;  //(num_default_deriv_vars, num_default_gradients);
      for(int mi = 0; mi < num_default_hessians; ++mi) {
        RealMatrix full_hessian(num_default_deriv_vars, num_default_deriv_vars);
        const RealSymMatrix &resp_hessian = response.function_hessian_view(hessian_idxs[mi]);
        for(int i = 0; i < dvv.size(); ++i) {
          int dvv_i = dvv[i] - 1;
          full_hessian(dvv_i, dvv_i) = resp_hessian(i,i);
          for(int j = i+1; j < dvv.size(); ++j) {
            int dvv_j = dvv[j] - 1;
            full_hessian(dvv_j, dvv_i) = full_hessian(dvv_i, dvv_j) = resp_hessian(i, j);
          }
        }
        full_hessians.push_back(full_hessian);
      }
      hdf5Stream->set_vector_matrix(hessians_name, full_hessians, resp_idx, true /*transpose */);
    }
  } 
#endif
}

void EvaluationStore::store_metadata(const String &root_group, const ActiveSet &set, 
        const DefaultSet &default_set_s) {
#ifdef DAKOTA_HAVE_HDF5
  String metadata_root = root_group + "metadata/";
  // ASV
  hdf5Stream->append_vector(metadata_root + "active_set_vector", set.request_vector());
  // DVV. The dvv in set may be shorter than the default one, and so it has to be aligned
  // by ID.
  const SizetArray &default_dvv = default_set_s.set.derivative_vector();
  const ShortArray &default_asv = default_set_s.set.request_vector();
  // The DVV dataset doesn't exist unless gradients or hessians can be provided
  if(default_set_s.numGradients || default_set_s.numHessians) {
    const SizetArray &dvv = set.derivative_vector();
    // vector that will be apppended to the dataset. "bits" defaulted to 0 ("off")
    IntArray dvv_row(default_dvv.size(), 0);
    // Most of the time, all possible derivative variables will be "active" (the lengths of the 
    // current and default DVV will match), so we don't need to examine the DVV entry by entry.
    if(dvv.size() == default_dvv.size())
      std::fill(dvv_row.begin(), dvv_row.end(), 1);
    else {
      // This logic assumes that the entries in dvv and default_dvv are sorted in ascending order.
      // It iterates over the entries of the current dvv, and for each, advances through the default
      // dvv until the entry is found. It then sets the bit for that entry and goes to the next one
      // in the current dvv.
      int di = 0;
      for(int si = 0; si < dvv.size(); ++si) {
        for(; di < default_dvv.size(); ++di) {
          if(dvv[si] == default_dvv[di]) {
            dvv_row[di] = 1;
            ++di;
            break;
          }
        }
      }
    }
    hdf5Stream->append_vector(metadata_root + "derivative_variables_vector", dvv_row);
  }
#endif
}

void EvaluationStore::model_selection(const unsigned short &selection) {
  modelSelection = selection;
}

void EvaluationStore::interface_selection(const unsigned short &selection) {
  interfaceSelection = selection;
}

bool EvaluationStore::model_active(const String &model_id) {
  if(modelSelection == MODEL_EVAL_STORE_ALL)
    return true;
  else if(modelSelection == MODEL_EVAL_STORE_NONE)
    return false;
  else // MODEL_EVAL_STORE_TOP_METHOD and ALL_METHODS
    return sourceModels.find(model_id) != sourceModels.end();
}

bool EvaluationStore::interface_active(const String &iface_type) {
  if(interfaceSelection == INTERF_EVAL_STORE_ALL)
    return true;
  else if(interfaceSelection == INTERF_EVAL_STORE_NONE)
    return false;
  else if(iface_type != "approximation") // simulation only 
    return true;
}

} // Dakota namespace


