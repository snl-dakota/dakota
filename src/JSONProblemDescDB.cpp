/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_global_defs.hpp"
#include "JSONProblemDescDB.hpp"
#include <sstream>
#include <unordered_map>
#include <cmath>

namespace {

  json
    load_json_from_file(const std::string& filename)
  {
    std::ifstream file(filename);
    if (!file.is_open()) {
      throw std::runtime_error("Could not open the file: " + filename);
    }

    json j;
    file >> j;
    return j;
  }
}

namespace Dakota {

// ---------------------------------------------------------------------------
// active_data / get_block_list
// ---------------------------------------------------------------------------

JSONProblemDescDB::DataMap&
JSONProblemDescDB::active_data(const std::string& block_name)
{
  if (block_name == "environment") return environmentData;
  if (block_name == "method")     return methodList.at(activeMethodIdx);
  if (block_name == "model")      return modelList.at(activeModelIdx);
  if (block_name == "variables")  return variablesList.at(activeVariablesIdx);
  if (block_name == "interface")  return interfaceList.at(activeInterfaceIdx);
  if (block_name == "responses")  return responsesList.at(activeResponsesIdx);
  throw std::runtime_error(
    "JSONProblemDescDB::active_data: unknown block " + block_name);
}

std::vector<JSONProblemDescDB::DataMap>&
JSONProblemDescDB::get_block_list(const std::string& block_name)
{
  if (block_name == "method")     return methodList;
  if (block_name == "model")      return modelList;
  if (block_name == "variables")  return variablesList;
  if (block_name == "interface")  return interfaceList;
  if (block_name == "responses")  return responsesList;
  throw std::runtime_error(
    "JSONProblemDescDB::get_block_list: unknown block " + block_name);
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

JSONProblemDescDB::JSONProblemDescDB(const std::string& filename)
{
  json key_map_obj;
  try {
    jsonOptions = load_json_from_file(filename);
    key_map_obj = load_json_from_file("key_mapping_v2.json");
    std::cout << "Dakota JSON Input: " << jsonOptions.dump(4) << std::endl;
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  // Build an index from JSON-path -> list of mapping bindings.
// Each binding corresponds to one PDB key (top-level key in "mappings") and
// carries the json_paths[] entry metadata (handler_type, storage_type, etc.).
std::unordered_map<std::string, json> path_index;
if (key_map_obj.contains("mappings")) {
  for (auto& [pdb_key, mapping] : key_map_obj["mappings"].items()) {
    if (!mapping.contains("json_paths"))
      continue;
    for (auto const& jp : mapping["json_paths"]) {
      if (!jp.contains("path"))
        continue;
      std::string p = jp["path"].get<std::string>();
      json binding = jp;
      // Attach the PDB key associated with this mapping entry.
      binding["pdb_key"] = pdb_key;
      path_index[p].push_back(binding);
    }
  }
}

// Parse the Dakota json input and populate per-block data maps
std::function<void(const json&, const std::string&)>
  parse_json = [&](const json& j, const std::string& path)
{
  for (auto& [key, value] : j.items()) {
    std::string currentPath = path.empty() ? key : path + "/" + key;

    // Top-level object (environment): set up storage context
    if (path.empty() && value.is_object() && key == "environment") {
      currentData_ = &environmentData;
      currentBlockJson_ = &value;
    }

    // Top-level arrays (method/model/variables/interface/responses): one DataMap per element
    if (path.empty() && value.is_array()) {
      std::cout << currentPath << ": [Array of size " << value.size() << "]" << std::endl;
      auto& block_list = get_block_list(key);
      for (size_t i = 0; i < value.size(); ++i) {
        block_list.emplace_back();
        currentData_ = &block_list.back();
        currentBlockJson_ = &value[i];
        parse_json(value[i], currentPath);
      }
      continue;
    }

    // If this path is mapped, handle it (supports both leaf and non-leaf paths)
    auto it = path_index.find(currentPath);
    if (it != path_index.end()) {
      handle_keyword(it->second, currentPath, value);
    }

    // Recurse into objects to reach mapped leaves
    if (value.is_object()) {
      parse_json(value, currentPath);
    }
    // For arrays not handled as top-level blocks, recurse into elements
    else if (value.is_array()) {
      for (size_t i = 0; i < value.size(); ++i) {
        parse_json(value[i], currentPath);
      }
    }
  }
};

parse_json(jsonOptions, "");
  std::cout << "\n\n" << std::endl;
}

// ---------------------------------------------------------------------------
// handle_keyword
// ---------------------------------------------------------------------------

void
JSONProblemDescDB::handle_keyword(const json& bindings,
                                  const std::string& currentPath,
                                  const json& value)
{
  std::cout << currentPath << ": " << value << std::endl;

  if (!bindings.is_array() || bindings.empty()) {
    std::cout << currentPath << " --> " << "No mapping bindings for path" << std::endl;
    return;
  }

  for (auto const& binding : bindings)
  {
    // Support multiple cache keys (rare): prefer pdb_key, else pdb_keys
    std::vector<Dakota::String> cache_keys;
    if (binding.contains("pdb_key"))
      cache_keys.push_back(binding["pdb_key"]);
    else if (binding.contains("pdb_keys"))
      for (auto const& k : binding["pdb_keys"])
        cache_keys.push_back(k);

    if (cache_keys.empty()) {
      std::cout << currentPath << " --> " << "No pdb_key(s) in binding" << std::endl;
      continue;
    }

    const std::string storage_type = binding.value("storage_type", "");
    const std::string handler_type = binding.value("handler_type", "");

    for (const auto& ckey : cache_keys)
    {
      std::cout << currentPath << " --> "
        << "(" << handler_type << ") "
        << ckey
        << std::endl;

      if ("DIRECT_VALUE" == storage_type) {
        const std::string& type = handler_type;
        if (type == "str")
          store(ckey, value.get<String>());
        else if (type == "int")
          store(ckey, value.get<int>());
        else if (type == "sizet")
          store(ckey, value.get<size_t>());
        else if (type == "Real")
          store(ckey, value.get<Real>());
        else if (type == "strL")
          store(ckey, value.get<StringArray>());
        else if (type == "str2D")
          store(ckey, value.get<String2DArray>());
        else if (type == "intset")
          store(ckey, value.get<IntSet>());
        else if (type == "id_to_index_set")
          store(ckey, value.get<SizetSet>());
        else if (type == "rvec"   ||
                 type == "RealL"  ||
                 type == "RealLb" ||
                 type == "RealUb" ||
                 type == "RealDL")
          store(ckey, value.get<JSONRealVector>().value);
        else if (type == "ivec"   ||
                 type == "IntLb")
          store(ckey, value.get<JSONIntVector>().value);
        else if (type == "categorical")
          store(ckey, value.get<JSONBitArray>().value);
        else if (type == "szarray")
          store(ckey, value.get<SizetArray>());
        else if (type == "usharray")
          store(ckey, value.get<UShortArray>());
        else if (type == "newrvec") {
          std::cout << "DIRECT_VALUE, newrvec: " << value << std::endl;
          store(ckey, value.get<JSONRealSymMatrix>().value);
        }
        else if (type == "resplevs01") {
          std::cout << "DIRECT_VALUE, resplevs01: " << value << std::endl;
          store(ckey, value.get<JSONRealVector>().value);
        }
        else
          std::cout << "DIRECT_VALUE: Need to implement data caching for type "
            << type << std::endl;
      }
      else if ("PRESENCE_LITERAL" == storage_type) {
        if (handler_type == "lit") {
          String keyword = binding.value("stored_value", "");
          store(ckey, std::move(keyword));
          std::cout << "Setting " << ckey
            << " (" << currentPath << ") --> "
            << binding.value("stored_value", "") << std::endl;
        }
        else
          std::cout << "PRESENCE_LITERAL: Need to implement data caching for type "
            << handler_type << std::endl;
      }
      else if ("PRESENCE_TRUE" == storage_type) {
        assert("true" == handler_type);
        if (!value.is_object())
          store(ckey, value.get<bool>());
        else
          store(ckey, true);
      }
      else if ("PRESENCE_FALSE" == storage_type) {
        assert("false" == handler_type);
        if (!value.is_object())
          store(ckey, !value.get<bool>());
        else
          store(ckey, false);
      }
      else if ("PRESENCE_ENUM" == storage_type) {
        const std::string enum_str = binding.value("stored_value", "");
        if (dakEnumMap().count(enum_str)) {
          store(ckey, Ushort(dakEnumMap().at(enum_str)));
          std::cout << "PRESENCE_ENUM: Setting " << ckey << " = " << enum_str << std::endl;
        }
        else
          std::cout << "PRESENCE_ENUM: " << enum_str << " has not been registered." << std::endl;
      }
      else if ("AUGMENT_ENUM" == storage_type) {
        const std::string init_enum_str = binding.value("initial_value", "");
        const std::string aug_enum_str  = binding.value("stored_value", "");
        bool is_valid = true;
        if (dakEnumMap().count(init_enum_str) == 0) {
          std::cout << "AUGMENT_ENUM: " << init_enum_str << " has not been registered." << std::endl;
          is_valid = false;
        }
        if (dakEnumMap().count(aug_enum_str) == 0) {
          std::cout << "AUGMENT_ENUM: " << aug_enum_str << " has not been registered." << std::endl;
          is_valid = false;
        }

        if (is_valid) {
          auto it = currentData_->find(ckey);
          if (it != currentData_->end()) {
            Ushort existing = std::any_cast<Ushort>(it->second);
            assert(existing & dakEnumMap().at(init_enum_str));
            store(ckey, Ushort(existing | dakEnumMap().at(aug_enum_str)));
          }
          else
            store(ckey, Ushort(dakEnumMap().at(init_enum_str) | dakEnumMap().at(aug_enum_str)));
        }
      }
      else if ("CATEGORICAL" == storage_type) {
        handle_categorical(ckey, currentPath, value);
      }
      else if ("TYPE_DATA_COMBINED" == storage_type) {
        handle_type_data_combined(binding, ckey, currentPath, value);
      }
      else if ("ADJACENCY_MATRIX" == storage_type) {
        handle_adjacency_matrix(ckey, currentPath, value);
      }
      else if ("DISCRETE_SET_VALUES" == storage_type) {
        handle_discrete_set_values(binding, ckey, currentPath, value);
      }
      else if ("HISTOGRAM_BIN_UNCERTAIN" == storage_type) {
        handle_histogram_bin_uncertain(ckey, currentPath, value);
      }
      else if ("HISTOGRAM_POINT_UNCERTAIN" == storage_type) {
        handle_histogram_point_uncertain(ckey, currentPath, value);
      }
      else if ("DISCRETE_UNCERTAIN_SET_VALUES_PROBS" == storage_type) {
        handle_discrete_uncertain_set_values_probs(ckey, currentPath, value);
      }
      else if ("CONTINUOUS_INTERVAL_UNCERTAIN" == storage_type) {
        handle_continuous_interval_uncertain(ckey, currentPath, value);
      }
      else if ("DISCRETE_INTERVAL_UNCERTAIN" == storage_type) {
        handle_discrete_interval_uncertain(ckey, currentPath, value);
      }
      else if ("UNCERTAIN_CORRELATION_MATRIX" == storage_type) {
        handle_uncertain_correlation_matrix(ckey, currentPath, value);
      }
      else {
        std::cout << "Unhandled storage_type: " << storage_type
                  << " for path " << currentPath << std::endl;
      }
    }
  }
}

// ---------------------------------------------------------------------------
// handle_categorical
// ---------------------------------------------------------------------------

void
JSONProblemDescDB::handle_categorical(const String& ckey,
                                      const std::string& currentPath,
                                      const json& value)
{
  BitArray bits(value.size());
  for (size_t i = 0; i < value.size(); ++i) {
    const std::string& s = value[i].get<std::string>();
    if (!s.empty()) {
      char c = std::tolower(s[0]);
      bits[i] = (c == 't' || c == 'y');
    }
    else
      bits[i] = false;
  }
  store(ckey, std::move(bits));
  std::cout << "CATEGORICAL: Setting " << ckey
    << " (" << currentPath << ") with " << value.size() << " flags" << std::endl;
}

// ---------------------------------------------------------------------------
// handle_type_data_combined
// ---------------------------------------------------------------------------

void
JSONProblemDescDB::handle_type_data_combined(const json& binding,
                                             const String& ckey,
                                             const std::string& currentPath,
                                             const json& value)
{
  // 1) Store the type discriminant literal
  String stored_val = binding.value("stored_value","");
  store(ckey, std::move(stored_val));
  std::cout << "TYPE_DATA_COMBINED: Setting " << ckey
    << " (" << currentPath << ") --> "
    << binding.value("stored_value","") << std::endl;

  // 2) Store the argument value using its own pdb key
  if( !!binding.contains("argument_pdb_key") )
    return;

  const std::string& arg_key  = binding["argument_pdb_key"];
  const std::string& arg_type = binding["argument_member_variable_type"];

  if( arg_type == "String" )
    store(arg_key, value.get<String>());
  else if( arg_type == "int" )
    store(arg_key, value.get<int>());
  else if( arg_type == "size_t" )
    store(arg_key, value.get<size_t>());
  else if( arg_type == "Real" )
    store(arg_key, value.get<Real>());
  else if( arg_type == "RealVector" )
    store(arg_key, value.get<JSONRealVector>().value);
  else
    std::cout << "TYPE_DATA_COMBINED: Unhandled argument type "
      << arg_type << " for " << arg_key << std::endl;

  std::cout << "TYPE_DATA_COMBINED: Setting argument " << arg_key
    << " (type " << arg_type << ") --> " << value << std::endl;
}

// ---------------------------------------------------------------------------
// handle_adjacency_matrix
// ---------------------------------------------------------------------------

void
JSONProblemDescDB::handle_adjacency_matrix(const String& ckey,
                                           const std::string& currentPath,
                                           const json& value)
{
  if (!currentBlockJson_) {
    std::cout << "ADJACENCY_MATRIX: no block context for " << currentPath << std::endl;
    return;
  }

  // Navigate from currentBlockJson_ by slash-delimited relative path
  auto navigate = [&](const std::string& rel_path) -> std::pair<bool, json> {
    json node = *currentBlockJson_;
    std::istringstream iss(rel_path);
    std::string segment;
    while (std::getline(iss, segment, '/')) {
      if (node.contains(segment))
        node = node[segment];
      else
        return {false, json()};
    }
    return {true, node};
  };

  // Strip the block prefix to get a path relative to the block element.
  // e.g. "variables/discrete_design_set/integer/categorical/adjacency_matrix"
  //    -> "discrete_design_set/integer/categorical/adjacency_matrix"
  std::string rel_path = currentPath;
  auto first_slash = rel_path.find('/');
  if (first_slash != std::string::npos)
    rel_path = rel_path.substr(first_slash + 1);

  // Walk up to the variable type level: strip "adjacency_matrix",
  // then strip "categorical" if present.
  auto pos = rel_path.rfind('/');
  if (pos != std::string::npos)
    rel_path = rel_path.substr(0, pos);
  pos = rel_path.rfind('/');
  if (pos != std::string::npos) {
    std::string last = rel_path.substr(pos + 1);
    if (last == "categorical")
      rel_path = rel_path.substr(0, pos);
  }

  // Determine per-variable element counts
  std::vector<int> elems_per_var;

  auto [epv_found, epv_node] = navigate(rel_path + "/elements_per_variable");
  if (epv_found && epv_node.is_array()) {
    elems_per_var = epv_node.get<std::vector<int>>();
  }
  else {
    // Fall back: use count and assume elements are evenly divided
    auto [cnt_found, cnt_node] = navigate(rel_path + "/count");
    if (cnt_found && cnt_node.is_number_integer()) {
      int num_vars = cnt_node.get<int>();
      if (num_vars > 0) {
        size_t total   = value.size();
        size_t per_var = total / num_vars;
        int n = static_cast<int>(std::round(std::sqrt(
                                   static_cast<double>(per_var))));
        if (static_cast<size_t>(n) * n * num_vars != total) {
          std::cout << "ADJACENCY_MATRIX: flat array size " << total
            << " not consistent with " << num_vars
            << " square matrices of dimension " << n << std::endl;
        }
        elems_per_var.assign(num_vars, n);
        std::cout << "ADJACENCY_MATRIX: Inferred " << num_vars
          << " matrices of dimension " << n << " from count" << std::endl;
      }
    }
    else {
      std::cout << "ADJACENCY_MATRIX: Could not find elements_per_variable"
        << " or count for " << currentPath << std::endl;
    }
  }

  if (elems_per_var.empty())
    return;

  // Reshape flat 0/1 integer array into RealMatrixArray
  auto flat = value.get<std::vector<int>>();
  size_t offset = 0;
  RealMatrixArray rma(elems_per_var.size());

  for (size_t v = 0; v < elems_per_var.size(); ++v) {
    int n = elems_per_var[v];
    size_t n_sq = static_cast<size_t>(n) * n;
    if (offset + n_sq > flat.size()) {
      std::cout << "ADJACENCY_MATRIX: flat array too short for variable "
        << v << " (need " << n_sq << " at offset " << offset
        << ", have " << flat.size() << ")" << std::endl;
      break;
    }
    RealMatrix& mat = rma[v];
    mat.shapeUninitialized(n, n);
    for (int r = 0; r < n; ++r)
      for (int c = 0; c < n; ++c)
        mat(r, c) = static_cast<Real>(flat[offset + r * n + c]);
    offset += n_sq;
  }

  store(ckey, std::move(rma));
  std::cout << "ADJACENCY_MATRIX: Setting " << ckey
    << " (" << currentPath << ") with " << elems_per_var.size()
    << " matrices" << std::endl;
}

// ---------------------------------------------------------------------------
// handle_discrete_set_values
// ---------------------------------------------------------------------------

void
JSONProblemDescDB::handle_discrete_set_values(const json& binding,
                                              const String& ckey,
                                              const std::string& currentPath,
                                              const json& value)
{
  auto navigate = [&](const std::string& rel_path) -> std::pair<bool, json> {
    if (!currentBlockJson_)
      return {false, json()};
    json node = *currentBlockJson_;
    std::istringstream iss(rel_path);
    std::string segment;
    while (std::getline(iss, segment, '/')) {
      if (node.contains(segment))
        node = node[segment];
      else
        return {false, json()};
    }
    return {true, node};
  };

  const json* set_obj = nullptr;
  json legacy_elements;
  json fallback_set_obj;
  std::string rel_base;

  if (value.is_object()) {
    set_obj = &value;
    std::string rel_path = currentPath;
    auto first_slash = rel_path.find('/');
    if (first_slash != std::string::npos)
      rel_path = rel_path.substr(first_slash + 1);
    rel_base = rel_path; // already at .../{integer|string|real}
  }
  else if (value.is_array()) {
    // Backward-compatible path if mapping still points at .../elements
    std::string rel_path = currentPath;
    auto first_slash = rel_path.find('/');
    if (first_slash != std::string::npos)
      rel_path = rel_path.substr(first_slash + 1);
    auto pos = rel_path.rfind('/');
    if (pos == std::string::npos) {
      std::cout << "DISCRETE_SET_VALUES: unexpected path " << currentPath
                << std::endl;
      return;
    }
    rel_base = rel_path.substr(0, pos);
    auto [base_found, base_node] = navigate(rel_base);
    if (!base_found || !base_node.is_object()) {
      std::cout << "DISCRETE_SET_VALUES: could not find object at " << rel_base
                << std::endl;
      return;
    }
    fallback_set_obj = std::move(base_node);
    set_obj = &fallback_set_obj;
    legacy_elements = value;
  }
  else {
    std::cout << "DISCRETE_SET_VALUES: expected object or array at "
              << currentPath << std::endl;
    return;
  }

  if (!set_obj->contains("count") || !(*set_obj)["count"].is_number_integer()) {
    std::cout << "DISCRETE_SET_VALUES: missing integer count at " << rel_base
              << std::endl;
    return;
  }
  const int num_vars = (*set_obj)["count"].get<int>();
  if (num_vars <= 0) {
    std::cout << "DISCRETE_SET_VALUES: non-positive count for " << rel_base
              << std::endl;
    return;
  }

  const json* elements_ptr = nullptr;
  if (set_obj->contains("elements") && (*set_obj)["elements"].is_array())
    elements_ptr = &(*set_obj)["elements"];
  else if (legacy_elements.is_array())
    elements_ptr = &legacy_elements;

  if (!elements_ptr) {
    std::cout << "DISCRETE_SET_VALUES: missing elements array at " << rel_base
              << std::endl;
    return;
  }
  const json& elements = *elements_ptr;

  std::vector<int> elems_per_var;
  if (set_obj->contains("elements_per_variable") &&
      (*set_obj)["elements_per_variable"].is_array()) {
    const json& epv = (*set_obj)["elements_per_variable"];
    if (static_cast<int>(epv.size()) != num_vars) {
      std::cout << "DISCRETE_SET_VALUES: elements_per_variable length "
                << epv.size() << " does not match count " << num_vars
                << " at " << rel_base << std::endl;
      return;
    }
    for (const auto& e : epv)
      elems_per_var.push_back(e.get<int>());
  }
  else {
    const size_t total = elements.size();
    if (total % static_cast<size_t>(num_vars) != 0) {
      std::cout << "DISCRETE_SET_VALUES: element count " << total
                << " not divisible by count " << num_vars
                << " at " << rel_base << std::endl;
      return;
    }
    elems_per_var.assign(num_vars, static_cast<int>(total / num_vars));
  }

  size_t expected_size = 0;
  for (int n : elems_per_var)
    expected_size += static_cast<size_t>(n);
  if (expected_size != elements.size()) {
    std::cout << "DISCRETE_SET_VALUES: expected " << expected_size
              << " elements from apportionment, got " << elements.size()
              << " at " << rel_base << std::endl;
    return;
  }

  const std::string handler_type = binding.value("handler_type", "");

  size_t offset = 0;
  if (handler_type == "newivec") {
    IntSetArray sets(num_vars);
    for (int i = 0; i < num_vars; ++i) {
      IntSet& set_i = sets[i];
      for (int j = 0; j < elems_per_var[i]; ++j, ++offset)
        set_i.insert(elements[offset].get<int>());
    }
    store(ckey, std::move(sets));
  }
  else if (handler_type == "newsarray") {
    StringSetArray sets(num_vars);
    for (int i = 0; i < num_vars; ++i) {
      StringSet& set_i = sets[i];
      for (int j = 0; j < elems_per_var[i]; ++j, ++offset)
        set_i.insert(elements[offset].get<String>());
    }
    store(ckey, std::move(sets));
  }
  else if (handler_type == "newrvec") {
    RealSetArray sets(num_vars);
    for (int i = 0; i < num_vars; ++i) {
      RealSet& set_i = sets[i];
      for (int j = 0; j < elems_per_var[i]; ++j, ++offset) {
        Real v = elements[offset].is_number()
          ? elements[offset].get<Real>()
          : static_cast<Real>(std::stod(elements[offset].get<std::string>()));
        set_i.insert(v);
      }
    }
    store(ckey, std::move(sets));
  }
  else {
    std::cout << "DISCRETE_SET_VALUES: unsupported handler_type "
              << handler_type << " for " << ckey << std::endl;
    return;
  }

  std::cout << "DISCRETE_SET_VALUES: Setting " << ckey
            << " (" << currentPath << ") with " << num_vars
            << " set variable(s)" << std::endl;
}

// ---------------------------------------------------------------------------
// handle_histogram_bin_uncertain
// ---------------------------------------------------------------------------

void
JSONProblemDescDB::handle_histogram_bin_uncertain(const String& ckey,
                                                  const std::string& currentPath,
                                                  const json& value)
{
  // Anchor path: variables/histogram_bin_uncertain
  // Materialize a RealRealMapArray (vector<map<Real, Real>>) equivalent to
  // DataVariablesRep::histogramUncBinPairs, based on legacy Vchk_HistogramBinUnc.

  if (!value.is_object()) {
    std::cout << "HISTOGRAM_BIN_UNCERTAIN: Expected object at " << currentPath << std::endl;
    return;
  }

  if (!value.contains("abscissas")) {
    std::cout << "HISTOGRAM_BIN_UNCERTAIN: Missing abscissas at " << currentPath << std::endl;
    return;
  }

  const auto abscissas = value["abscissas"].get<std::vector<Real>>();
  const size_t num_a = abscissas.size();

  // Optional pairs_per_variable
  bool has_ppv = value.contains("pairs_per_variable") && !value["pairs_per_variable"].is_null();
  std::vector<int> ppv;
  if (has_ppv)
    ppv = value["pairs_per_variable"].get<std::vector<int>>();

  // count (required if no pairs_per_variable)
  size_t count = 0;
  if (value.contains("count"))
    count = value["count"].get<size_t>();

  // density: oneOf ordinates or counts
  if (!value.contains("density") || !value["density"].is_object()) {
    std::cout << "HISTOGRAM_BIN_UNCERTAIN: Missing density object at " << currentPath << std::endl;
    return;
  }
  const json& density = value["density"];

  bool has_ordinates = density.contains("ordinates");
  bool has_counts    = density.contains("counts");

  if (has_ordinates == has_counts) {
    std::cout << "HISTOGRAM_BIN_UNCERTAIN: Expected exactly one of ordinates or counts at "
              << currentPath << std::endl;
    return;
  }

  std::vector<Real> yvals;
  if (has_counts)
    yvals = density["counts"].get<std::vector<Real>>();
  else
    yvals = density["ordinates"].get<std::vector<Real>>();

  const size_t num_y = yvals.size();
  if (num_y != num_a) {
    std::cout << "HISTOGRAM_BIN_UNCERTAIN: Expected " << num_a
              << " ordinates/counts, not " << num_y << " at "
              << currentPath << std::endl;
    return;
  }

  size_t m = 0; // number of variables
  std::vector<int> pairs_per_var;

  if (has_ppv) {
    m = ppv.size();
    pairs_per_var = ppv;

    size_t tothbp = 0;
    for (size_t i = 0; i < m; ++i) {
      int n = pairs_per_var[i];
      if (n < 2) {
        std::cout << "HISTOGRAM_BIN_UNCERTAIN: pairs_per_variable must be >= 2 at "
                  << currentPath << std::endl;
        return;
      }
      tothbp += static_cast<size_t>(n);
    }
    if (tothbp != num_a) {
      std::cout << "HISTOGRAM_BIN_UNCERTAIN: Expected " << tothbp
                << " abscissas, not " << num_a << " at "
                << currentPath << std::endl;
      return;
    }
  }
  else {
    m = count;
    if (m == 0) {
      std::cout << "HISTOGRAM_BIN_UNCERTAIN: count must be > 0 when pairs_per_variable absent at "
                << currentPath << std::endl;
      return;
    }
    if (num_a % m != 0) {
      std::cout << "HISTOGRAM_BIN_UNCERTAIN: Number of abscissas (" << num_a
                << ") not evenly divisible by number of variables (" << m
                << "); use pairs_per_variable for unequal apportionment at "
                << currentPath << std::endl;
      return;
    }
    int avg = static_cast<int>(num_a / m);
    pairs_per_var.assign(m, avg);
  }

  RealRealMapArray hbp;
  hbp.resize(m);

  size_t cntr = 0;
  for (size_t i = 0; i < m; ++i) {
    int nhbpi = pairs_per_var[i];
    RealRealMap& hbpi = hbp[i];

    Real count_sum = 0.0;

    for (int j = 0; j < nhbpi; ++j, ++cntr) {
      Real x = abscissas[cntr];
      Real y = yvals[cntr];

      if (j < nhbpi - 1) {
        Real bin_width = abscissas[cntr + 1] - x;
        if (bin_width <= 0.0) {
          std::cout << "HISTOGRAM_BIN_UNCERTAIN: histogram bin x values must increase at "
                    << currentPath << std::endl;
          return;
        }
        if (y <= 0.0) {
          std::cout << "HISTOGRAM_BIN_UNCERTAIN: nonpositive intermediate histogram bin y value at "
                    << currentPath << std::endl;
          return;
        }

        if (has_counts) {
          count_sum += y;       // accumulate counts
          y /= bin_width;       // convert counts to density
        }
        else {
          count_sum += y * bin_width; // accumulate integral
        }
      }
      else if (y != 0.0) {
        std::cout << "HISTOGRAM_BIN_UNCERTAIN: histogram bin y values must end with 0 at "
                  << currentPath << std::endl;
        return;
      }

      hbpi[x] = y;
    }

    // normalize, omitting last value (terminal y=0)
    if (hbpi.size() >= 2 && count_sum > 0.0) {
      auto it_end = hbpi.end();
      --it_end;
      for (auto it = hbpi.begin(); it != it_end; ++it)
        it->second /= count_sum;
    }
  }

  store(ckey, std::move(hbp));
  std::cout << "HISTOGRAM_BIN_UNCERTAIN: Setting " << ckey
            << " (" << currentPath << ") with " << m << " variables" << std::endl;
}

// ---------------------------------------------------------------------------
// handle_histogram_point_uncertain
// ---------------------------------------------------------------------------

void
JSONProblemDescDB::handle_histogram_point_uncertain(const String& ckey,
                                                    const std::string& currentPath,
                                                    const json& value)
{
  if (!value.is_object()) {
    std::cout << "HISTOGRAM_POINT_UNCERTAIN: Expected object at " << currentPath << std::endl;
    return;
  }
  if (!value.contains("abscissas") || !value.contains("counts")) {
    std::cout << "HISTOGRAM_POINT_UNCERTAIN: Missing abscissas/counts at "
              << currentPath << std::endl;
    return;
  }
  if (!value.contains("count")) {
    std::cout << "HISTOGRAM_POINT_UNCERTAIN: Missing count at "
              << currentPath << std::endl;
    return;
  }

  const size_t count = value["count"].get<size_t>();
  if (count == 0) {
    std::cout << "HISTOGRAM_POINT_UNCERTAIN: count must be > 0 at "
              << currentPath << std::endl;
    return;
  }

  const auto counts = value["counts"].get<std::vector<Real>>();
  const size_t num_c = counts.size();
  const size_t num_a = value["abscissas"].size();
  if (num_a != num_c) {
    std::cout << "HISTOGRAM_POINT_UNCERTAIN: Expected " << num_a
              << " point counts, not " << num_c << " at "
              << currentPath << std::endl;
    return;
  }

  std::vector<int> pairs_per_var;
  if (value.contains("pairs_per_variable") && !value["pairs_per_variable"].is_null()) {
    pairs_per_var = value["pairs_per_variable"].get<std::vector<int>>();
    size_t total = 0;
    for (int n : pairs_per_var) {
      if (n < 1) {
        std::cout << "HISTOGRAM_POINT_UNCERTAIN: pairs_per_variable must be >= 1 at "
                  << currentPath << std::endl;
        return;
      }
      total += static_cast<size_t>(n);
    }
    if (total != num_a) {
      std::cout << "HISTOGRAM_POINT_UNCERTAIN: Expected " << total
                << " point abscissas, not " << num_a << " at "
                << currentPath << std::endl;
      return;
    }
  }
  else {
    if (num_a % count != 0) {
      std::cout << "HISTOGRAM_POINT_UNCERTAIN: Number of abscissas (" << num_a
                << ") not evenly divisible by number of variables (" << count
                << "); use pairs_per_variable for unequal apportionment at "
                << currentPath << std::endl;
      return;
    }
    pairs_per_var.assign(count, static_cast<int>(num_a / count));
  }

  auto is_int   = ckey.find(".point_int_pairs") != std::string::npos;
  auto is_str   = ckey.find(".point_string_pairs") != std::string::npos;
  auto is_real  = ckey.find(".point_real_pairs") != std::string::npos;

  if (!is_int && !is_str && !is_real) {
    std::cout << "HISTOGRAM_POINT_UNCERTAIN: Unknown target key " << ckey << std::endl;
    return;
  }

  size_t idx = 0;
  if (is_int) {
    IntRealMapArray hpp;
    hpp.resize(pairs_per_var.size());
    for (size_t i = 0; i < pairs_per_var.size(); ++i) {
      IntRealMap& m = hpp[i];
      Real sum = 0.0;
      int n = pairs_per_var[i];
      for (int j = 0; j < n; ++j, ++idx) {
        int x = value["abscissas"][idx].get<int>();
        Real y = counts[idx];
        if (j < n - 1 && x >= value["abscissas"][idx + 1].get<int>()) {
          std::cout << "HISTOGRAM_POINT_UNCERTAIN: histogram point x values must increase at "
                    << currentPath << std::endl;
          return;
        }
        if (y <= 0.0) {
          std::cout << "HISTOGRAM_POINT_UNCERTAIN: nonpositive intermediate histogram point y value at "
                    << currentPath << std::endl;
          return;
        }
        m[x] = y;
        sum += y;
      }
      if (sum > 0.0) {
        for (auto& kv : m)
          kv.second /= sum;
      }
    }
    store(ckey, std::move(hpp));
  }
  else if (is_str) {
    StringRealMapArray hpp;
    hpp.resize(pairs_per_var.size());
    for (size_t i = 0; i < pairs_per_var.size(); ++i) {
      StringRealMap& m = hpp[i];
      Real sum = 0.0;
      int n = pairs_per_var[i];
      for (int j = 0; j < n; ++j, ++idx) {
        String x = value["abscissas"][idx].get<String>();
        Real y = counts[idx];
        if (j < n - 1 && x >= value["abscissas"][idx + 1].get<String>()) {
          std::cout << "HISTOGRAM_POINT_UNCERTAIN: histogram point x values must increase at "
                    << currentPath << std::endl;
          return;
        }
        if (y <= 0.0) {
          std::cout << "HISTOGRAM_POINT_UNCERTAIN: nonpositive intermediate histogram point y value at "
                    << currentPath << std::endl;
          return;
        }
        m[x] = y;
        sum += y;
      }
      if (sum > 0.0) {
        for (auto& kv : m)
          kv.second /= sum;
      }
    }
    store(ckey, std::move(hpp));
  }
  else {
    RealRealMapArray hpp;
    hpp.resize(pairs_per_var.size());
    for (size_t i = 0; i < pairs_per_var.size(); ++i) {
      RealRealMap& m = hpp[i];
      Real sum = 0.0;
      int n = pairs_per_var[i];
      for (int j = 0; j < n; ++j, ++idx) {
        Real x = value["abscissas"][idx].get<Real>();
        Real y = counts[idx];
        if (j < n - 1 && x >= value["abscissas"][idx + 1].get<Real>()) {
          std::cout << "HISTOGRAM_POINT_UNCERTAIN: histogram point x values must increase at "
                    << currentPath << std::endl;
          return;
        }
        if (y <= 0.0) {
          std::cout << "HISTOGRAM_POINT_UNCERTAIN: nonpositive intermediate histogram point y value at "
                    << currentPath << std::endl;
          return;
        }
        m[x] = y;
        sum += y;
      }
      if (sum > 0.0) {
        for (auto& kv : m)
          kv.second /= sum;
      }
    }
    store(ckey, std::move(hpp));
  }

  std::cout << "HISTOGRAM_POINT_UNCERTAIN: Setting " << ckey
            << " (" << currentPath << ") with " << pairs_per_var.size()
            << " variables" << std::endl;
}

// ---------------------------------------------------------------------------
// handle_discrete_uncertain_set_values_probs
// ---------------------------------------------------------------------------

void
JSONProblemDescDB::handle_discrete_uncertain_set_values_probs(const String& ckey,
                                                              const std::string& currentPath,
                                                              const json& value)
{
  if (!value.is_object()) {
    std::cout << "DISCRETE_UNCERTAIN_SET_VALUES_PROBS: Expected object at "
              << currentPath << std::endl;
    return;
  }
  if (!value.contains("count") || !value["count"].is_number_integer()) {
    std::cout << "DISCRETE_UNCERTAIN_SET_VALUES_PROBS: Missing integer count at "
              << currentPath << std::endl;
    return;
  }
  if (!value.contains("elements") || !value["elements"].is_array()) {
    std::cout << "DISCRETE_UNCERTAIN_SET_VALUES_PROBS: Missing elements array at "
              << currentPath << std::endl;
    return;
  }

  const int num_vars = value["count"].get<int>();
  if (num_vars <= 0) {
    std::cout << "DISCRETE_UNCERTAIN_SET_VALUES_PROBS: count must be > 0 at "
              << currentPath << std::endl;
    return;
  }

  const json& elements = value["elements"];
  std::vector<int> elems_per_var;
  if (value.contains("elements_per_variable") && value["elements_per_variable"].is_array()) {
    elems_per_var = value["elements_per_variable"].get<std::vector<int>>();
    if (static_cast<int>(elems_per_var.size()) != num_vars) {
      std::cout << "DISCRETE_UNCERTAIN_SET_VALUES_PROBS: elements_per_variable length "
                << elems_per_var.size() << " does not match count " << num_vars
                << " at " << currentPath << std::endl;
      return;
    }
  }
  else {
    const size_t total = elements.size();
    if (total % static_cast<size_t>(num_vars) != 0) {
      std::cout << "DISCRETE_UNCERTAIN_SET_VALUES_PROBS: element count " << total
                << " not divisible by count " << num_vars << " at "
                << currentPath << std::endl;
      return;
    }
    elems_per_var.assign(num_vars, static_cast<int>(total / num_vars));
  }

  size_t total_elems = 0;
  for (int n : elems_per_var) {
    if (n < 1) {
      std::cout << "DISCRETE_UNCERTAIN_SET_VALUES_PROBS: elements_per_variable must be >= 1 at "
                << currentPath << std::endl;
      return;
    }
    total_elems += static_cast<size_t>(n);
  }
  if (total_elems != elements.size()) {
    std::cout << "DISCRETE_UNCERTAIN_SET_VALUES_PROBS: expected " << total_elems
              << " elements from apportionment, got " << elements.size()
              << " at " << currentPath << std::endl;
    return;
  }

  bool has_probs = value.contains("set_probabilities") && value["set_probabilities"].is_array();
  std::vector<Real> probs;
  if (has_probs) {
    probs = value["set_probabilities"].get<std::vector<Real>>();
    if (probs.size() != elements.size()) {
      std::cout << "DISCRETE_UNCERTAIN_SET_VALUES_PROBS: set_probabilities length "
                << probs.size() << " does not match elements length "
                << elements.size() << " at " << currentPath << std::endl;
      return;
    }
  }

  auto is_int  = ckey.find("_int.values_probs") != std::string::npos;
  auto is_str  = ckey.find("_string.values_probs") != std::string::npos;
  auto is_real = ckey.find("_real.values_probs") != std::string::npos;

  size_t offset = 0;
  if (is_int) {
    IntRealMapArray maps(num_vars);
    for (int i = 0; i < num_vars; ++i) {
      IntRealMap& m = maps[i];
      Real default_p = 1.0 / static_cast<Real>(elems_per_var[i]);
      for (int j = 0; j < elems_per_var[i]; ++j, ++offset) {
        int v = elements[offset].get<int>();
        Real p = has_probs ? probs[offset] : default_p;
        if (m.find(v) == m.end())
          m[v] = p;
      }
    }
    store(ckey, std::move(maps));
  }
  else if (is_str) {
    StringRealMapArray maps(num_vars);
    for (int i = 0; i < num_vars; ++i) {
      StringRealMap& m = maps[i];
      Real default_p = 1.0 / static_cast<Real>(elems_per_var[i]);
      for (int j = 0; j < elems_per_var[i]; ++j, ++offset) {
        String v = elements[offset].get<String>();
        Real p = has_probs ? probs[offset] : default_p;
        if (m.find(v) == m.end())
          m[v] = p;
      }
    }
    store(ckey, std::move(maps));
  }
  else if (is_real) {
    RealRealMapArray maps(num_vars);
    for (int i = 0; i < num_vars; ++i) {
      RealRealMap& m = maps[i];
      Real default_p = 1.0 / static_cast<Real>(elems_per_var[i]);
      for (int j = 0; j < elems_per_var[i]; ++j, ++offset) {
        Real v = elements[offset].is_number()
          ? elements[offset].get<Real>()
          : static_cast<Real>(std::stod(elements[offset].get<std::string>()));
        Real p = has_probs ? probs[offset] : default_p;
        if (m.find(v) == m.end())
          m[v] = p;
      }
    }
    store(ckey, std::move(maps));
  }
  else {
    std::cout << "DISCRETE_UNCERTAIN_SET_VALUES_PROBS: Unknown target key " << ckey << std::endl;
    return;
  }

  std::cout << "DISCRETE_UNCERTAIN_SET_VALUES_PROBS: Setting " << ckey
            << " (" << currentPath << ") with " << num_vars
            << " set variable(s)" << std::endl;
}

// ---------------------------------------------------------------------------
// handle_continuous_interval_uncertain
// ---------------------------------------------------------------------------

void
JSONProblemDescDB::handle_continuous_interval_uncertain(const String& ckey,
                                                        const std::string& currentPath,
                                                        const json& value)
{
  if (!value.is_object()) {
    std::cout << "CONTINUOUS_INTERVAL_UNCERTAIN: Expected object at "
              << currentPath << std::endl;
    return;
  }
  if (!value.contains("count") || !value["count"].is_number_integer()) {
    std::cout << "CONTINUOUS_INTERVAL_UNCERTAIN: Missing integer count at "
              << currentPath << std::endl;
    return;
  }
  if (!value.contains("lower_bounds") || !value.contains("upper_bounds")) {
    std::cout << "CONTINUOUS_INTERVAL_UNCERTAIN: Missing lower_bounds/upper_bounds at "
              << currentPath << std::endl;
    return;
  }

  const int count = value["count"].get<int>();
  if (count <= 0) {
    std::cout << "CONTINUOUS_INTERVAL_UNCERTAIN: count must be > 0 at "
              << currentPath << std::endl;
    return;
  }

  const auto lbs = value["lower_bounds"].get<std::vector<Real>>();
  const auto ubs = value["upper_bounds"].get<std::vector<Real>>();
  if (lbs.size() != ubs.size()) {
    std::cout << "CONTINUOUS_INTERVAL_UNCERTAIN: Expected as many lower bounds ("
              << lbs.size() << ") as upper bounds (" << ubs.size() << ") at "
              << currentPath << std::endl;
    return;
  }

  bool has_probs = value.contains("interval_probabilities") &&
                   value["interval_probabilities"].is_array();
  std::vector<Real> probs;
  if (has_probs) {
    probs = value["interval_probabilities"].get<std::vector<Real>>();
    if (probs.size() != lbs.size()) {
      std::cout << "CONTINUOUS_INTERVAL_UNCERTAIN: Expected as many probabilities ("
                << probs.size() << ") as bounds (" << lbs.size() << ") at "
                << currentPath << std::endl;
      return;
    }
  }

  std::vector<int> num_intervals;
  if (value.contains("num_intervals") && value["num_intervals"].is_array()) {
    num_intervals = value["num_intervals"].get<std::vector<int>>();
    if (static_cast<int>(num_intervals.size()) != count) {
      std::cout << "CONTINUOUS_INTERVAL_UNCERTAIN: Expected " << count
                << " num_intervals values, got " << num_intervals.size()
                << " at " << currentPath << std::endl;
      return;
    }
    for (int n : num_intervals) {
      if (n < 1) {
        std::cout << "CONTINUOUS_INTERVAL_UNCERTAIN: num_intervals values must be positive at "
                  << currentPath << std::endl;
        return;
      }
    }
  }
  else {
    if (lbs.size() % static_cast<size_t>(count) != 0) {
      std::cout << "CONTINUOUS_INTERVAL_UNCERTAIN: Number of bounds (" << lbs.size()
                << ") not evenly divisible by number of variables (" << count
                << "); use num_intervals for unequal apportionment at "
                << currentPath << std::endl;
      return;
    }
    num_intervals.assign(count, static_cast<int>(lbs.size() / static_cast<size_t>(count)));
  }

  size_t expected = 0;
  for (int n : num_intervals)
    expected += static_cast<size_t>(n);
  if (expected != lbs.size()) {
    std::cout << "CONTINUOUS_INTERVAL_UNCERTAIN: expected " << expected
              << " bounds from apportionment, got " << lbs.size()
              << " at " << currentPath << std::endl;
    return;
  }

  RealRealPairRealMapArray out;
  out.resize(num_intervals.size());
  size_t k = 0;
  for (size_t i = 0; i < num_intervals.size(); ++i) {
    int n = num_intervals[i];
    RealRealPairRealMap& m = out[i];
    Real default_p = 1.0 / static_cast<Real>(n);
    if (has_probs) {
      Real total = 0.0;
      for (int j = 0; j < n; ++j)
        total += probs[k + static_cast<size_t>(j)];
      if (std::fabs(total - 1.0) > 1.e-10 && total > 0.0) {
        for (int j = 0; j < n; ++j)
          probs[k + static_cast<size_t>(j)] /= total;
      }
    }
    for (int j = 0; j < n; ++j, ++k) {
      Real lb = lbs[k];
      Real ub = ubs[k];
      if (lb > ub) {
        std::cout << "CONTINUOUS_INTERVAL_UNCERTAIN: upper bound less than lower bound ["
                  << lb << ", " << ub << "] at " << currentPath << std::endl;
        return;
      }
      Real p = has_probs ? probs[k] : default_p;
      RealRealPair interval(lb, ub);
      if (!m.insert(std::make_pair(interval, p)).second) {
        std::cout << "CONTINUOUS_INTERVAL_UNCERTAIN: duplicate interval ["
                  << lb << ", " << ub << "] for variable " << i
                  << " at " << currentPath << std::endl;
        return;
      }
    }
  }

  store(ckey, std::move(out));
  std::cout << "CONTINUOUS_INTERVAL_UNCERTAIN: Setting " << ckey
            << " (" << currentPath << ") with " << num_intervals.size()
            << " variables" << std::endl;
}

// ---------------------------------------------------------------------------
// handle_discrete_interval_uncertain
// ---------------------------------------------------------------------------

void
JSONProblemDescDB::handle_discrete_interval_uncertain(const String& ckey,
                                                      const std::string& currentPath,
                                                      const json& value)
{
  if (!value.is_object()) {
    std::cout << "DISCRETE_INTERVAL_UNCERTAIN: Expected object at "
              << currentPath << std::endl;
    return;
  }
  if (!value.contains("count") || !value["count"].is_number_integer()) {
    std::cout << "DISCRETE_INTERVAL_UNCERTAIN: Missing integer count at "
              << currentPath << std::endl;
    return;
  }
  if (!value.contains("lower_bounds") || !value.contains("upper_bounds")) {
    std::cout << "DISCRETE_INTERVAL_UNCERTAIN: Missing lower_bounds/upper_bounds at "
              << currentPath << std::endl;
    return;
  }

  const int count = value["count"].get<int>();
  if (count <= 0) {
    std::cout << "DISCRETE_INTERVAL_UNCERTAIN: count must be > 0 at "
              << currentPath << std::endl;
    return;
  }

  const auto lbs = value["lower_bounds"].get<std::vector<int>>();
  const auto ubs = value["upper_bounds"].get<std::vector<int>>();
  if (lbs.size() != ubs.size()) {
    std::cout << "DISCRETE_INTERVAL_UNCERTAIN: Expected as many lower bounds ("
              << lbs.size() << ") as upper bounds (" << ubs.size() << ") at "
              << currentPath << std::endl;
    return;
  }

  bool has_probs = value.contains("interval_probabilities") &&
                   value["interval_probabilities"].is_array();
  std::vector<Real> probs;
  if (has_probs) {
    probs = value["interval_probabilities"].get<std::vector<Real>>();
    if (probs.size() != lbs.size()) {
      std::cout << "DISCRETE_INTERVAL_UNCERTAIN: Expected as many probabilities ("
                << probs.size() << ") as bounds (" << lbs.size() << ") at "
                << currentPath << std::endl;
      return;
    }
  }

  std::vector<int> num_intervals;
  if (value.contains("num_intervals") && value["num_intervals"].is_array()) {
    num_intervals = value["num_intervals"].get<std::vector<int>>();
    if (static_cast<int>(num_intervals.size()) != count) {
      std::cout << "DISCRETE_INTERVAL_UNCERTAIN: Expected " << count
                << " num_intervals values, got " << num_intervals.size()
                << " at " << currentPath << std::endl;
      return;
    }
    for (int n : num_intervals) {
      if (n < 1) {
        std::cout << "DISCRETE_INTERVAL_UNCERTAIN: num_intervals values must be positive at "
                  << currentPath << std::endl;
        return;
      }
    }
  }
  else {
    if (lbs.size() % static_cast<size_t>(count) != 0) {
      std::cout << "DISCRETE_INTERVAL_UNCERTAIN: Number of bounds (" << lbs.size()
                << ") not evenly divisible by number of variables (" << count
                << "); use num_intervals for unequal apportionment at "
                << currentPath << std::endl;
      return;
    }
    num_intervals.assign(count, static_cast<int>(lbs.size() / static_cast<size_t>(count)));
  }

  size_t expected = 0;
  for (int n : num_intervals)
    expected += static_cast<size_t>(n);
  if (expected != lbs.size()) {
    std::cout << "DISCRETE_INTERVAL_UNCERTAIN: expected " << expected
              << " bounds from apportionment, got " << lbs.size()
              << " at " << currentPath << std::endl;
    return;
  }

  IntIntPairRealMapArray out;
  out.resize(num_intervals.size());
  size_t k = 0;
  for (size_t i = 0; i < num_intervals.size(); ++i) {
    int n = num_intervals[i];
    IntIntPairRealMap& m = out[i];
    Real default_p = 1.0 / static_cast<Real>(n);
    for (int j = 0; j < n; ++j, ++k) {
      int lb = lbs[k];
      int ub = ubs[k];
      if (lb > ub) {
        std::cout << "DISCRETE_INTERVAL_UNCERTAIN: upper bound less than lower bound ["
                  << lb << ", " << ub << "] at " << currentPath << std::endl;
        return;
      }
      Real p = has_probs ? probs[k] : default_p;
      IntIntPair interval(lb, ub);
      if (!m.insert(std::make_pair(interval, p)).second) {
        std::cout << "DISCRETE_INTERVAL_UNCERTAIN: duplicate interval ["
                  << lb << ", " << ub << "] for variable " << i
                  << " at " << currentPath << std::endl;
        return;
      }
    }
  }

  store(ckey, std::move(out));
  std::cout << "DISCRETE_INTERVAL_UNCERTAIN: Setting " << ckey
            << " (" << currentPath << ") with " << num_intervals.size()
            << " variables" << std::endl;
}

// ---------------------------------------------------------------------------
// handle_uncertain_correlation_matrix
// ---------------------------------------------------------------------------

void
JSONProblemDescDB::handle_uncertain_correlation_matrix(const String& ckey,
                                                       const std::string& currentPath,
                                                       const json& value)
{
  if (!value.is_array()) {
    std::cout << "UNCERTAIN_CORRELATION_MATRIX: Expected array at "
              << currentPath << std::endl;
    return;
  }

  std::vector<Real> flat;
  flat.reserve(value.size());
  for (const auto& v : value) {
    if (v.is_number()) {
      flat.push_back(v.get<Real>());
    }
    else if (v.is_string()) {
      const auto s = v.get<std::string>();
      if (s == "inf" || s == "+inf")
        flat.push_back(std::numeric_limits<Real>::infinity());
      else if (s == "-inf")
        flat.push_back(-std::numeric_limits<Real>::infinity());
      else
        flat.push_back(static_cast<Real>(std::stod(s)));
    }
    else {
      std::cout << "UNCERTAIN_CORRELATION_MATRIX: invalid entry at "
                << currentPath << std::endl;
      return;
    }
  }

  const size_t n = flat.size();
  const size_t dim = static_cast<size_t>(std::round(std::sqrt(static_cast<double>(n))));
  if (dim * dim != n) {
    std::cout << "UNCERTAIN_CORRELATION_MATRIX: expected square flattened matrix length, got "
              << n << " at " << currentPath << std::endl;
    return;
  }

  RealSymMatrix rsm;
  rsm.reshape(dim);
  size_t k = 0;
  for (size_t i = 0; i < dim; ++i)
    for (size_t j = 0; j < dim; ++j)
      rsm(i, j) = flat[k++];

  store(ckey, std::move(rsm));
  std::cout << "UNCERTAIN_CORRELATION_MATRIX: Setting " << ckey
            << " (" << currentPath << ") with dimension "
            << dim << "x" << dim << std::endl;
}

} // namespace Dakota
