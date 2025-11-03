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

namespace Dakota {
 
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
            /* 
               Need to implement these as we go ...   
             */
    // /// get a RealMatrixArray out of the database based on an identifier string
    // const RealMatrixArray& get_rma(const String& entry_name) const;
    /// get a RealVector out of the database based on an identifier string
    const RealVector& get_rv(const String& entry_name) const;
    // /// get an IntVector out of the database based on an identifier string
    // const IntVector& get_iv(const String& entry_name) const;
    // /// get a BitArray out of the database based on an identifier string
    // const BitArray& get_ba(const String& entry_name) const;
    // /// get an SizetArray out of the database based on an identifier string
    // const SizetArray& get_sza(const String& entry_name) const;
    // /// get an UShortArray out of the database based on an identifier string
    // const UShortArray& get_usa(const String& entry_name) const;
    // /// get a RealSymMatrix out of the database based on an identifier string
    // const RealSymMatrix& get_rsm(const String& entry_name) const;
    // /// get a RealVectorArray out of the database based on an identifier string
    // const RealVectorArray& get_rva(const String& entry_name) const;
    // /// get an IntVectorArray out of the database based on an identifier string
    // const IntVectorArray& get_iva(const String& entry_name) const;
    // /// get an IntSet out of the database based on an identifier string
    // const IntSet& get_is(const String& entry_name) const;
    // /// get an IntSetArray out of the database based on an identifier string
    // const IntSetArray& get_isa(const String& entry_name) const;
    // /// get a SizetSet out of the database based on an identifier string
    // const SizetSet& get_szs(const String& entry_name) const;
    // /// get an StringSetArray out of the database based on an identifier string
    // const StringSetArray& get_ssa(const String& entry_name) const;
    // /// get a RealSetArray out of the database based on an identifier string
    // const RealSetArray& get_rsa(const String& entry_name) const;
    // /// get an IntRealMapArray out of the database based on an identifier string
    // const IntRealMapArray& get_irma(const String& entry_name) const;
    // /// get an StringRealMapArray out of the database based on an
    // /// identifier string
    // const StringRealMapArray& get_srma(const String& entry_name) const;
    // /// get a RealRealMapArray out of the database based on an identifier string
    // const RealRealMapArray& get_rrma(const String& entry_name) const;
    // /// get a RealRealPairRealMapArray out of the database based on an
    // /// identifier string
    // const RealRealPairRealMapArray& get_rrrma(const String& entry_name) const;
    // /// get an IntIntPairRealMapArray out of the database based on an
    // /// identifier string
    // const IntIntPairRealMapArray& get_iirma(const String& entry_name) const;
    /// copy a StringArray out of the database based on an identifier string
    StringArray get_sa(const String& entry_name) const;
    // /// get a String2DArray out of the database based on an identifier string
    // const String2DArray& get_s2a(const String& entry_name) const;
    /// copy a String out of the database based on an identifier string
    const String& get_string(const String& entry_name) const;
    // /// get a Real out of the database based on an identifier string
    // const Real& get_real(const String& entry_name) const;
    /// copy an int out of the database based on an identifier string
    int get_int(const String& entry_name) const;
    // /// get a short out of the database based on an identifier string
    // short get_short(const String& entry_name) const;
    // /// get an unsigned short out of the database based on an identifier string
    // unsigned short get_ushort(const String& entry_name) const;
    // /// get a size_t out of the database based on an identifier string
    // size_t get_sizet(const String& entry_name) const;
    /// copy a bool out of the database based on an identifier string
    bool get_bool(const String& entry_name) const;

  //private:
  public:

    // Access JSON values using dot notation
    auto get_value(const String& key) const;

    json jsonOptions;

    std::set<std::string> allowedBlocks;

    std::map<String, int> blockIds;

  public:

  private:

    mutable std::map<String, String> cachedStringData;
    mutable std::map<String, RealVector> cachedRealVectorData;
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

inline const RealVector& JSONProblemDescDB::get_rv(const String& key) const
{
  auto val = get_value(key).template get<JSONRealVector>().value;
  cachedRealVectorData[key] = val;
  return cachedRealVectorData[key];
}

inline StringArray JSONProblemDescDB::get_sa(const String& key) const
{
  auto val = get_value(key).get<StringArray>();
  return val;
}

inline const String& JSONProblemDescDB::get_string(const String& key) const
{
  auto val = get_value(key).get<String>();
  cachedStringData[key] = val;
  return cachedStringData[key];
}

inline int JSONProblemDescDB::get_int(const String& key) const
{
  auto val = get_value(key).get<int>();
  return val;
}

inline bool JSONProblemDescDB::get_bool(const String& key) const
{
  auto val = get_value(key).get<bool>();
  return val;
}

} // namespace Dakota

#endif
