/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef JSON_PROBLEM_DESC_DB_H
#define JSON_PROBLEM_DESC_DB_H

#include "JSONUtils.hpp"
#include <nlohmann/json.hpp>
#include <any>

using json = nlohmann::json;

#define JSON_GET_CACHED_VALUE(GET_VALUE_FN, verbose) \
  if (jsonDB) { \
    try { \
      if( false ) \
        std::cout << "Trying JSONProblemDescDB::" << std::string(#GET_VALUE_FN) \
                  << " ... for \"" << entry_name << "\"" << std::endl; \
      return jsonDB->GET_VALUE_FN(entry_name); \
    } catch (const std::exception& e) { \
      if( verbose ) \
        std::cout << "JSONProblemDescDB::" << std::string(#GET_VALUE_FN) \
                  << ": no cached JSON value for \"" << entry_name << "\"" << std::endl; \
    } \
  }

namespace Dakota {

  using Ushort = unsigned short;

class JSONProblemDescDB
{
  public:

    using DataMap = std::map<String, std::any>;

    /// constructor
    JSONProblemDescDB(const String& filename);

    /// destructor
    ~JSONProblemDescDB() = default;

    //
    //- Heading: Getter methods
    //

    /// get a RealMatrixArray out of the database based on an identifier string
    const RealMatrixArray& get_rma(const String& entry_name);
    /// get a RealVector out of the database based on an identifier string
    const RealVector& get_rv(const String& entry_name);
    /// get an IntVector out of the database based on an identifier string
    const IntVector& get_iv(const String& entry_name);
    /// get a BitArray out of the database based on an identifier string
    const BitArray& get_ba(const String& entry_name);
    /// get an SizetArray out of the database based on an identifier string
    const SizetArray& get_sza(const String& entry_name);
    /// get an UShortArray out of the database based on an identifier string
    const UShortArray& get_usa(const String& entry_name);
    /// get a RealSymMatrix out of the database based on an identifier string
    const RealSymMatrix& get_rsm(const String& entry_name);
    /// get a RealVectorArray out of the database based on an identifier string
    const RealVectorArray& get_rva(const String& entry_name);
    /// get an IntVectorArray out of the database based on an identifier string
    const IntVectorArray& get_iva(const String& entry_name);
    /// get an IntSet out of the database based on an identifier string
    const IntSet& get_is(const String& entry_name);
    /// get an IntSetArray out of the database based on an identifier string
    const IntSetArray& get_isa(const String& entry_name);
    /// get a SizetSet out of the database based on an identifier string
    const SizetSet& get_szs(const String& entry_name);
    /// get an StringSetArray out of the database based on an identifier string
    const StringSetArray& get_ssa(const String& entry_name);
    /// get a RealSetArray out of the database based on an identifier string
    const RealSetArray& get_rsa(const String& entry_name);
    /// get an IntRealMapArray out of the database based on an identifier string
    const IntRealMapArray& get_irma(const String& entry_name);
    /// get an StringRealMapArray out of the database based on an
    /// identifier string
    const StringRealMapArray& get_srma(const String& entry_name);
    /// get a RealRealMapArray out of the database based on an identifier string
    const RealRealMapArray& get_rrma(const String& entry_name);
    /// get a RealRealPairRealMapArray out of the database based on an
    /// identifier string
    const RealRealPairRealMapArray& get_rrrma(const String& entry_name);
    /// get an IntIntPairRealMapArray out of the database based on an
    /// identifier string
    const IntIntPairRealMapArray& get_iirma(const String& entry_name);
    /// get a StringArray out of the database based on an identifier string
    const StringArray& get_sa(const String& entry_name);
    /// get a String2DArray out of the database based on an identifier string
    const String2DArray& get_s2a(const String& entry_name);
    /// get a String out of the database based on an identifier string
    const String& get_string(const String& entry_name);
    /// get a Real out of the database based on an identifier string
    const Real& get_real(const String& entry_name);
    /// get an int out of the database based on an identifier string
    const int& get_int(const String& entry_name);
    /// get a short out of the database based on an identifier string
    const short& get_short(const String& entry_name);
    /// get an unsigned short out of the database based on an identifier string
    const Ushort& get_ushort(const String& entry_name);
    /// get a size_t out of the database based on an identifier string
    const size_t& get_sizet(const String& entry_name);
    /// get a bool out of the database based on an identifier string
    const bool& get_bool(const String& entry_name);

    //
    //- Heading: Active block selection
    //

    /// set the active method block index
    void set_active_method(size_t idx);
    /// set the active model block index
    void set_active_model(size_t idx);
    /// set the active variables block index
    void set_active_variables(size_t idx);
    /// set the active interface block index
    void set_active_interface(size_t idx);
    /// set the active responses block index
    void set_active_responses(size_t idx);

  //private:
  public:

    // Access JSON values using dot notation
    auto get_value(const String& key) const;

    json jsonOptions;

    // These may not be needed if we adopt named json blocks
    std::set<std::string> allowedBlocks;
    std::map<String, int> blockIds;

  private:

    //
    //- Heading: Internal helpers
    //

    /// Retrieve a cached value by key, routed to the active block instance
    template<typename T>
    const T& get_cached(const String& entry_name);

    /// Return the active DataMap for a given block name
    DataMap& active_data(const std::string& block_name);

    /// Return the block list for a given block name (used during construction)
    std::vector<DataMap>& get_block_list(const std::string& block_name);

    //
    //- Heading: Keyword handlers
    //

    void handle_keyword(const json& key_map_item,
                        const std::string& currentPath,
                        const json& value);

    void handle_categorical(const String& ckey,
                            const std::string& currentPath,
                            const json& value);

    void handle_type_data_combined(const json& binding,
                                   const String& ckey,
                                   const std::string& currentPath,
                                   const json& value);

    void handle_adjacency_matrix(const String& ckey,
                                 const std::string& currentPath,
                                 const json& value);

    void handle_discrete_set_values(const json& binding,
                                    const String& ckey,
                                    const std::string& currentPath,
                                    const json& value);

    void handle_histogram_bin_uncertain(const String& ckey,
                                        const std::string& currentPath,
                                        const json& value);

    void handle_histogram_point_uncertain(const String& ckey,
                                          const std::string& currentPath,
                                          const json& value);

    void handle_discrete_uncertain_set_values_probs(const String& ckey,
                                                    const std::string& currentPath,
                                                    const json& value);

    void handle_continuous_interval_uncertain(const String& ckey,
                                              const std::string& currentPath,
                                              const json& value);

    void handle_discrete_interval_uncertain(const String& ckey,
                                            const std::string& currentPath,
                                            const json& value);

    void handle_uncertain_correlation_matrix(const String& ckey,
                                             const std::string& currentPath,
                                             const json& value);


    /// Store a value in the current DataMap
    template<typename T>
    void store(const String& ckey, T&& val);

    //
    //- Heading: Per-block-type storage
    //

    DataMap                  environmentData;
    std::vector<DataMap>     methodList;
    std::vector<DataMap>     modelList;
    std::vector<DataMap>     variablesList;
    std::vector<DataMap>     interfaceList;
    std::vector<DataMap>     responsesList;

    size_t activeMethodIdx    = 0;
    size_t activeModelIdx     = 0;
    size_t activeVariablesIdx = 0;
    size_t activeInterfaceIdx = 0;
    size_t activeResponsesIdx = 0;

    /// Points to the DataMap currently being populated during construction
    DataMap* currentData_ = nullptr;

    /// Points to the JSON object for the current block element being parsed.
    /// Allows handlers to look up sibling/ancestor fields within the block.
    const json* currentBlockJson_ = nullptr;
};

// ---------------------------------------------------------------------------
// Inline implementations
// ---------------------------------------------------------------------------

inline auto JSONProblemDescDB::get_value(const String& key) const
{
  // Interpret `key` as dot-delimited, with the first token being the top-level block:
  //   environment.*, method.*, model.*, variables.*, interface.*, responses.*
  //
  // This matches the validated JSON shape where:
  //   - environment is an object (optional)
  //   - the other blocks are arrays of objects, indexed by the active_* index.
  auto dot = key.find('.');
  if (dot == String::npos)
    throw std::runtime_error(
      "JSONProblemDescDB::get_value: key missing block prefix: " + key);

  const std::string block = key.substr(0, dot);
  const std::string rest  = key.substr(dot + 1);

  const json* current = nullptr;

  if (block == "environment") {
    if (!jsonOptions.contains("environment") || !jsonOptions["environment"].is_object())
      throw std::runtime_error(
        "JSONProblemDescDB::get_value: environment block missing or not an object");
    current = &jsonOptions["environment"];
  }
  else {
    if (!jsonOptions.contains(block))
      throw std::runtime_error(
        "JSONProblemDescDB::get_value: top-level block missing: " + block);
    const json& arr = jsonOptions[block];
    if (!arr.is_array())
      throw std::runtime_error(
        "JSONProblemDescDB::get_value: expected array for block: " + block);

    size_t idx = 0;
    if      (block == "method")     idx = activeMethodIdx;
    else if (block == "model")      idx = activeModelIdx;
    else if (block == "variables")  idx = activeVariablesIdx;
    else if (block == "interface")  idx = activeInterfaceIdx;
    else if (block == "responses")  idx = activeResponsesIdx;
    else
      throw std::runtime_error(
        "JSONProblemDescDB::get_value: unknown block: " + block);

    if (idx >= arr.size())
      throw std::runtime_error(
        "JSONProblemDescDB::get_value: active index out of range for block: " + block);

    current = &arr[idx];
  }

  // Traverse remaining tokens within the selected block object.
  json node = *current;
  String working = rest;
  size_t pos = 0;
  while ((pos = working.find('.')) != String::npos) {
    String token = working.substr(0, pos);
    node = node[token];
    working.erase(0, pos + 1);
  }
  return node[working];
}


template<typename T>
const T& JSONProblemDescDB::get_cached(const String& entry_name)
{
  auto dot = entry_name.find('.');
  if (dot == String::npos)
    throw std::runtime_error(
      "JSONProblemDescDB: entry_name missing block prefix: " + entry_name);

  std::string block = entry_name.substr(0, dot);
  const DataMap& dm = active_data(block);
  auto it = dm.find(entry_name);
  if (it == dm.end())
    throw std::runtime_error(
      "JSONProblemDescDB: no cached value for " + entry_name);
  return std::any_cast<const T&>(it->second);
}

template<typename T>
void JSONProblemDescDB::store(const String& ckey, T&& val)
{
  (*currentData_)[ckey] = std::any(std::forward<T>(val));
}

inline void JSONProblemDescDB::set_active_method(size_t idx)
{ activeMethodIdx = idx; }

inline void JSONProblemDescDB::set_active_model(size_t idx)
{ activeModelIdx = idx; }

inline void JSONProblemDescDB::set_active_variables(size_t idx)
{ activeVariablesIdx = idx; }

inline void JSONProblemDescDB::set_active_interface(size_t idx)
{ activeInterfaceIdx = idx; }

inline void JSONProblemDescDB::set_active_responses(size_t idx)
{ activeResponsesIdx = idx; }

// Type-specific getters: thin wrappers around get_cached<T>

#define JSONDB_GETTER(TYPE, FN) \
  inline const TYPE& JSONProblemDescDB::FN(const String& key) { \
    return get_cached<TYPE>(key); \
  }

JSONDB_GETTER(RealMatrixArray,          get_rma)
JSONDB_GETTER(RealVector,               get_rv)
JSONDB_GETTER(IntVector,                get_iv)
JSONDB_GETTER(BitArray,                 get_ba)
JSONDB_GETTER(SizetArray,               get_sza)
JSONDB_GETTER(UShortArray,              get_usa)
JSONDB_GETTER(RealSymMatrix,            get_rsm)
JSONDB_GETTER(RealVectorArray,          get_rva)
JSONDB_GETTER(IntVectorArray,           get_iva)
JSONDB_GETTER(IntSet,                   get_is)
JSONDB_GETTER(IntSetArray,              get_isa)
JSONDB_GETTER(SizetSet,                 get_szs)
JSONDB_GETTER(StringSetArray,           get_ssa)
JSONDB_GETTER(RealSetArray,             get_rsa)
JSONDB_GETTER(IntRealMapArray,          get_irma)
JSONDB_GETTER(StringRealMapArray,       get_srma)
JSONDB_GETTER(RealRealMapArray,         get_rrma)
JSONDB_GETTER(RealRealPairRealMapArray, get_rrrma)
JSONDB_GETTER(IntIntPairRealMapArray,   get_iirma)
JSONDB_GETTER(StringArray,              get_sa)
JSONDB_GETTER(String2DArray,            get_s2a)
JSONDB_GETTER(String,                   get_string)
JSONDB_GETTER(Real,                     get_real)
JSONDB_GETTER(int,                      get_int)
JSONDB_GETTER(short,                    get_short)
JSONDB_GETTER(Ushort,                   get_ushort)
JSONDB_GETTER(size_t,                   get_sizet)
JSONDB_GETTER(bool,                     get_bool)

#undef JSONDB_GETTER

} // namespace Dakota

#endif
