/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef JSON_PROBLEM_DESC_DB_H
#define JSON_PROBLEM_DESC_DB_H

#include "ProblemDescDB.hpp" // might derive from this - RWH
#include "JSONUtils.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define JSON_GET_VALUE(GET_VALUE_FN, verbose) \
  if (jsonDB) { \
    /* \
    std::cout << "Trying ProblemDescDB::" << std::string(#GET_VALUE_FN) \
              << ": for \"" << entry_name << "\"" << std::endl; \
    */ \
    try { \
      const auto& val = jsonDB->GET_VALUE_FN(entry_name); \
      if( verbose ) \
        std::cout << "ProblemDescDB::" << std::string(#GET_VALUE_FN) \
                  << ": FOUND JSON value for \"" << entry_name << "\"" << std::endl; \
      return val; \
    } catch (const json::exception& e) { \
      if( verbose ) \
        std::cout << "ProblemDescDB::" << std::string(#GET_VALUE_FN) \
                  << ": no JSON value for \"" << entry_name << "\"" << std::endl; \
      /* no-op; */ \
    } catch (const std::exception& e) { \
      if( verbose ) \
        std::cout << "ProblemDescDB::" << std::string(#GET_VALUE_FN) \
                  << ": no JSON value for \"" << entry_name << "\"" << std::endl; \
      /* no-op; */ \
    } \
  }

namespace Dakota {

  using Ushort = unsigned short;
 
class JSONProblemDescDB //: public ProblemDescDB
{
  public:

    /// constructor
    JSONProblemDescDB(const String& filename);

    /// destructor
    ~JSONProblemDescDB() = default;

    //
    //- Heading: Member methods
    //

    // /// get a RealMatrixArray out of the database based on an identifier string
    // const RealMatrixArray& get_rma(const String& entry_name) const;
    /// get a RealVector out of the database based on an identifier string
    const RealVector& get_rv(const String& entry_name) const;
    /// get an IntVector out of the database based on an identifier string
    const IntVector& get_iv(const String& entry_name) const;
    // /// get a BitArray out of the database based on an identifier string
    // const BitArray& get_ba(const String& entry_name) const;
    /// get an SizetArray out of the database based on an identifier string
    const SizetArray& get_sza(const String& entry_name) const;
    /// get an UShortArray out of the database based on an identifier string
    const UShortArray& get_usa(const String& entry_name) const;
    /// get a RealSymMatrix out of the database based on an identifier string
    const RealSymMatrix& get_rsm(const String& entry_name) const;
    ///// get a RealVectorArray out of the database based on an identifier string
    //const RealVectorArray& get_rva(const String& entry_name) const;
    ///// get an IntVectorArray out of the database based on an identifier string
    //const IntVectorArray& get_iva(const String& entry_name) const;
    /// get an IntSet out of the database based on an identifier string
    const IntSet& get_is(const String& entry_name) const;
    /// get an IntSetArray out of the database based on an identifier string
    const IntSetArray& get_isa(const String& entry_name) const;
    /// get a SizetSet out of the database based on an identifier string
    const SizetSet& get_szs(const String& entry_name) const;
    /// get an StringSetArray out of the database based on an identifier string
    const StringSetArray& get_ssa(const String& entry_name) const;
    /// get a RealSetArray out of the database based on an identifier string
    const RealSetArray& get_rsa(const String& entry_name) const;
    /// get an IntRealMapArray out of the database based on an identifier string
    const IntRealMapArray& get_irma(const String& entry_name) const;
    /// get an StringRealMapArray out of the database based on an
    /// identifier string
    const StringRealMapArray& get_srma(const String& entry_name) const;
    /// get a RealRealMapArray out of the database based on an identifier string
    const RealRealMapArray& get_rrma(const String& entry_name) const;
    /// get a RealRealPairRealMapArray out of the database based on an
    /// identifier string
    const RealRealPairRealMapArray& get_rrrma(const String& entry_name) const;
    /// get an IntIntPairRealMapArray out of the database based on an
    /// identifier string
    const IntIntPairRealMapArray& get_iirma(const String& entry_name) const;
    /// get a StringArray out of the database based on an identifier string
    const StringArray& get_sa(const String& entry_name) const;
    /// get a String2DArray out of the database based on an identifier string
    const String2DArray& get_s2a(const String& entry_name) const;
    /// get a String out of the database based on an identifier string
    const String& get_string(const String& entry_name) const;
    /// get a Real out of the database based on an identifier string
    const Real& get_real(const String& entry_name) const;
    /// get an int out of the database based on an identifier string
    const int& get_int(const String& entry_name) const;
    /// get a short out of the database based on an identifier string
    const short& get_short(const String& entry_name) const;
    /// get an unsigned short out of the database based on an identifier string
    const Ushort& get_ushort(const String& entry_name) const;
    /// get a size_t out of the database based on an identifier string
    const size_t& get_sizet(const String& entry_name) const;
    /// get a bool out of the database based on an identifier string
    const bool& get_bool(const String& entry_name) const;

  //private:
  public:

    // Access JSON values using dot notation
    auto get_value(const String& key) const;

    json jsonOptions;

    std::set<std::string> allowedBlocks;

    std::map<String, int> blockIds;

  public:

  private:

    mutable std::map<String, RealVector>               cachedData_RealVector;
    mutable std::map<String, SizetArray>               cachedData_SizetArray;
    mutable std::map<String, UShortArray>              cachedData_UShortArray;
    mutable std::map<String, RealSymMatrix>            cachedData_RealSymMatrix;
    mutable std::map<String, RealVectorArray>          cachedData_RealVectorArray;
    mutable std::map<String, IntVectorArray>           cachedData_IntVectorArray;
    mutable std::map<String, IntSet>                   cachedData_IntSet;
    mutable std::map<String, IntSetArray>              cachedData_IntSetArray;
    mutable std::map<String, SizetSet>                 cachedData_SizetSet;
    mutable std::map<String, StringSetArray>           cachedData_StringSetArray;
    mutable std::map<String, RealSetArray>             cachedData_RealSetArray;
    mutable std::map<String, IntRealMapArray>          cachedData_IntRealMapArray;
    mutable std::map<String, StringRealMapArray>       cachedData_StringRealMapArray;
    mutable std::map<String, RealRealMapArray>         cachedData_RealRealMapArray;
    mutable std::map<String, RealRealPairRealMapArray> cachedData_RealRealPairRealMapArray;
    mutable std::map<String, IntIntPairRealMapArray>   cachedData_IntIntPairRealMapArray;
    mutable std::map<String, IntVector>                cachedData_IntVector;
    mutable std::map<String, StringArray>              cachedData_StringArray;
    mutable std::map<String, String2DArray>            cachedData_String2DArray;
    mutable std::map<String, String>                   cachedData_String;
    mutable std::map<String, Real>                     cachedData_Real;
    mutable std::map<String, int>                      cachedData_int;
    mutable std::map<String, short>                    cachedData_short;
    mutable std::map<String, unsigned short>           cachedData_Ushort;
    mutable std::map<String, size_t>                   cachedData_size_t;
    mutable std::map<String, bool>                     cachedData_bool;
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

#define STANDARD_JSONDB_GET_METHOD(TYPE, GET_FN) \
  inline const TYPE& JSONProblemDescDB::GET_FN(const String& key) const { \
    auto val = get_value(key).get<TYPE>(); \
    cachedData_##TYPE[key] = val; \
    return cachedData_##TYPE[key]; \
  }

#define CUSTOM_JSONDB_GET_METHOD(TYPE, GET_FN) \
  inline const TYPE& JSONProblemDescDB::GET_FN(const String& key) const { \
    auto val = get_value(key).template get<JSON##TYPE>().value; \
    cachedData_##TYPE[key] = val; \
    return cachedData_##TYPE[key]; \
  }

//CUSTOM_JSONDB_GET_METHOD  (RealMatrixArray,        get_rma)
CUSTOM_JSONDB_GET_METHOD  (RealVector,               get_rv)
CUSTOM_JSONDB_GET_METHOD  (IntVector,                get_iv)
//CUSTOM_JSONDB_GET_METHOD(BitArray,                 get_ba)
STANDARD_JSONDB_GET_METHOD(SizetArray,               get_sza)
STANDARD_JSONDB_GET_METHOD(UShortArray,              get_usa)
CUSTOM_JSONDB_GET_METHOD  (RealSymMatrix,            get_rsm)
//CUSTOM_JSONDB_GET_METHOD  (RealVectorArray,        get_rva)
//CUSTOM_JSONDB_GET_METHOD  (IntVectorArray,         get_iva)
STANDARD_JSONDB_GET_METHOD(IntSet,                   get_is)
STANDARD_JSONDB_GET_METHOD(IntSetArray,              get_isa)
STANDARD_JSONDB_GET_METHOD(SizetSet,                 get_szs)
STANDARD_JSONDB_GET_METHOD(StringSetArray,           get_ssa)
STANDARD_JSONDB_GET_METHOD(RealSetArray,             get_rsa)
STANDARD_JSONDB_GET_METHOD(IntRealMapArray,          get_irma)
STANDARD_JSONDB_GET_METHOD(StringRealMapArray,       get_srma)
STANDARD_JSONDB_GET_METHOD(RealRealMapArray,         get_rrma)
STANDARD_JSONDB_GET_METHOD(RealRealPairRealMapArray, get_rrrma)
STANDARD_JSONDB_GET_METHOD(IntIntPairRealMapArray,   get_iirma)
STANDARD_JSONDB_GET_METHOD(StringArray,              get_sa)
STANDARD_JSONDB_GET_METHOD(String2DArray,            get_s2a)
STANDARD_JSONDB_GET_METHOD(String,                   get_string)
STANDARD_JSONDB_GET_METHOD(Real,                     get_real)
STANDARD_JSONDB_GET_METHOD(int,                      get_int)
STANDARD_JSONDB_GET_METHOD(short,                    get_short)
STANDARD_JSONDB_GET_METHOD(Ushort,                   get_ushort)
STANDARD_JSONDB_GET_METHOD(size_t,                   get_sizet)
STANDARD_JSONDB_GET_METHOD(bool,                     get_bool)

//inline const RealVector& JSONProblemDescDB::get_rv(const String& key) const
//{
//  auto val = get_value(key).template get<JSONRealVector>().value;
//  cachedData_RealVector[key] = val;
//  return cachedData_RealVector[key];
//}

//inline const BitArray& JSONProblemDescDB::get_ba(const String& key) const
//{
//  auto val = get_value(key).template get<BitArray>().value;
//  cachedData_BitArray[key] = val;
//  return cachedData_BitArray[key];
//}

} // namespace Dakota

#endif
