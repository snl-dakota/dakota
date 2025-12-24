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

using json = nlohmann::json;

#define JSON_GET_CACHED_VALUE(GET_VALUE_FN, verbose) \
  if (jsonDB) { \
    try { \
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

    /// constructor
    JSONProblemDescDB(const String& filename);

    /// destructor
    ~JSONProblemDescDB() = default;

    //
    //- Heading: Member methods
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

  //private:
  public:

    // Access JSON values using dot notation
    auto get_value(const String& key) const;

    json jsonOptions;

    // These may not be needed if we adopt named json blocks
    std::set<std::string> allowedBlocks;
    std::map<String, int> blockIds;

  private:
    void handle_keyword(const json& key_map_item, const std::string& currentPath, const json& value);

  private:

    std::map<String, RealMatrixArray>          cachedData_RealMatrixArray;
    std::map<String, RealVector>               cachedData_RealVector;
    std::map<String, SizetArray>               cachedData_SizetArray;
    std::map<String, UShortArray>              cachedData_UShortArray;
    std::map<String, RealSymMatrix>            cachedData_RealSymMatrix;
    std::map<String, RealVectorArray>          cachedData_RealVectorArray;
    std::map<String, IntVectorArray>           cachedData_IntVectorArray;
    std::map<String, IntSet>                   cachedData_IntSet;
    std::map<String, IntSetArray>              cachedData_IntSetArray;
    std::map<String, SizetSet>                 cachedData_SizetSet;
    std::map<String, StringSetArray>           cachedData_StringSetArray;
    std::map<String, RealSetArray>             cachedData_RealSetArray;
    std::map<String, IntRealMapArray>          cachedData_IntRealMapArray;
    std::map<String, StringRealMapArray>       cachedData_StringRealMapArray;
    std::map<String, RealRealMapArray>         cachedData_RealRealMapArray;
    std::map<String, RealRealPairRealMapArray> cachedData_RealRealPairRealMapArray;
    std::map<String, IntIntPairRealMapArray>   cachedData_IntIntPairRealMapArray;
    std::map<String, BitArray>                 cachedData_BitArray;
    std::map<String, IntVector>                cachedData_IntVector;
    std::map<String, StringArray>              cachedData_StringArray;
    std::map<String, String2DArray>            cachedData_String2DArray;
    std::map<String, String>                   cachedData_String;
    std::map<String, Real>                     cachedData_Real;
    std::map<String, int>                      cachedData_int;
    std::map<String, short>                    cachedData_short;
    std::map<String, unsigned short>           cachedData_Ushort;
    std::map<String, size_t>                   cachedData_size_t;
    std::map<String, bool>                     cachedData_bool;
};

inline auto JSONProblemDescDB::get_value(const String& key) const
{
  // Copy appropriate block
  // ... could do better by going one level more
  String block_name = key.substr(0, key.find('.'));
  if( allowedBlocks.find(block_name) == allowedBlocks.end() ) {
    throw(std::runtime_error(
      "JSONProblemDescDB: Invalid json block \""+block_name+"\""));
  }
  int blk_id = blockIds.find(block_name)->second;
  json current = jsonOptions[blk_id];

  // Split the key by dot notation
  String working_key = key;
  size_t pos = 0;
  while ((pos = working_key.find('.')) != String::npos) {
    String token = working_key.substr(0, pos);
    current = current[token];
    working_key.erase(0, pos + 1);
  }
  return current[working_key];
}

#define CACHED_JSONDB_GET_METHOD(TYPE, GET_FN) \
  inline const TYPE& JSONProblemDescDB::GET_FN(const String& key) { \
    if( cachedData_##TYPE.count(key) ) { \
      if( true ) \
        std::cout << "JSONProblemDescDB::" << std::string(#GET_FN) \
                  << ": FOUND cached JSON value for \"" << key << "\"" << std::endl; \
      return cachedData_##TYPE[key]; \
    } \
    else \
      throw(std::runtime_error( \
        "JSONProblemDescDB: no cached value for "+key)); \
  }

CACHED_JSONDB_GET_METHOD   (RealMatrixArray,          get_rma)
CACHED_JSONDB_GET_METHOD   (RealVector,               get_rv)
CACHED_JSONDB_GET_METHOD   (IntVector,                get_iv)
CACHED_JSONDB_GET_METHOD   (BitArray,                 get_ba)
CACHED_JSONDB_GET_METHOD   (SizetArray,               get_sza)
CACHED_JSONDB_GET_METHOD   (UShortArray,              get_usa)
CACHED_JSONDB_GET_METHOD   (RealSymMatrix,            get_rsm)
CACHED_JSONDB_GET_METHOD   (RealVectorArray,          get_rva)
CACHED_JSONDB_GET_METHOD   (IntVectorArray,           get_iva)
CACHED_JSONDB_GET_METHOD   (IntSet,                   get_is)
CACHED_JSONDB_GET_METHOD   (IntSetArray,              get_isa)
CACHED_JSONDB_GET_METHOD   (SizetSet,                 get_szs)
CACHED_JSONDB_GET_METHOD   (StringSetArray,           get_ssa)
CACHED_JSONDB_GET_METHOD   (RealSetArray,             get_rsa)
CACHED_JSONDB_GET_METHOD   (IntRealMapArray,          get_irma)
CACHED_JSONDB_GET_METHOD   (StringRealMapArray,       get_srma)
CACHED_JSONDB_GET_METHOD   (RealRealMapArray,         get_rrma)
CACHED_JSONDB_GET_METHOD   (RealRealPairRealMapArray, get_rrrma)
CACHED_JSONDB_GET_METHOD   (IntIntPairRealMapArray,   get_iirma)
CACHED_JSONDB_GET_METHOD   (StringArray,              get_sa)
CACHED_JSONDB_GET_METHOD   (String2DArray,            get_s2a)
CACHED_JSONDB_GET_METHOD   (String,                   get_string)
CACHED_JSONDB_GET_METHOD   (Real,                     get_real)
CACHED_JSONDB_GET_METHOD   (int,                      get_int)
CACHED_JSONDB_GET_METHOD   (short,                    get_short)
CACHED_JSONDB_GET_METHOD   (Ushort,                   get_ushort)
CACHED_JSONDB_GET_METHOD   (size_t,                   get_sizet)
CACHED_JSONDB_GET_METHOD   (bool,                     get_bool)

} // namespace Dakota

#endif
