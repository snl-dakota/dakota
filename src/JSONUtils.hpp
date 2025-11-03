/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "JSONResultsParser.hpp"
#include <iostream>
#include <string>

            /*
               Need to implement these as we go ...
             */
    // /// get a RealMatrixArray out of the database based on an identifier string
    // const RealMatrixArray& get_rma(const String& entry_name) const;
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
    // /// copy a StringArray out of the database based on an identifier string
    // StringArray get_sa(const String& entry_name) const;
    // /// get a String2DArray out of the database based on an identifier string
    // const String2DArray& get_s2a(const String& entry_name) const;
    // /// copy a String out of the database based on an identifier string
    // const String& get_string(const String& entry_name) const;
    // /// get a Real out of the database based on an identifier string
    // const Real& get_real(const String& entry_name) const;
    // /// copy an int out of the database based on an identifier string
    // int get_int(const String& entry_name) const;
    // /// get a short out of the database based on an identifier string
    // short get_short(const String& entry_name) const;
    // /// get an unsigned short out of the database based on an identifier string
    // unsigned short get_ushort(const String& entry_name) const;
    // /// get a size_t out of the database based on an identifier string
    // size_t get_sizet(const String& entry_name) const;
    // /// copy a bool out of the database based on an identifier string
    // bool get_bool(const String& entry_name) const;


namespace Dakota {

// store the result of getting a number of string "encoded" number from JSON
struct JSONDoubleElement {
  double value;
};

// store the result of getting a RealVector from JSON.
struct JSONRealVector {
  RealVector value;
};

// store the result of getting a IntVector from JSON.
struct JSONIntVector {
  IntVector value;
};

// store the result of getting a RealSymMatrix from JSON.
struct JSONRealSymMatrix {
  RealSymMatrix value;
};

class JSONStoreError : public std::exception {
  // For errors encountered while storing JSON objects
  public:
    JSONStoreError(const std::string msg) : msg_(msg) {}

    virtual const char* what() const noexcept override {
        return msg_.c_str();
    }
  private:
    std::string msg_;
};


/// Read a number or string-encoded number from JSON
inline void from_json(const json &j, JSONDoubleElement &e) {
     e.value = (j.is_number()) ? j.template get<double>() :
            std::stod(j.template get<std::string>());
}


/// Read a RealVector of numbers or string-encoded numbers from JSON
inline void from_json(const json &j, JSONRealVector &e) {
    if(!j.is_array())
        throw JSONStoreError("expected an array object");
    auto n = j.size();
    auto &v = e.value;
    v.size(n);
    for(size_t i = 0; i < n; ++i)
        v[i] = j[i].template get<JSONDoubleElement>().value;
}


/// Read a IntVector of numbers from string-encoded numbers from JSON
inline void from_json(const json &j, JSONIntVector &e) {
    if(!j.is_array())
        throw JSONStoreError("expected an array object");
    auto n = j.size();
    auto &v = e.value;
    v.size(n);
    for(size_t i = 0; i < n; ++i)
        v[i] = j[i].get<int>();
}


/// Read a RealSymMatrix of numbers or string-encoded numbers from JSON
inline void from_json(const json &j, JSONRealSymMatrix &e) {
    if(!j.is_array())
         throw JSONStoreError(std::string("expected array object"));  
    auto &h = e.value;
    auto n = j.size();
    h.shape(n);
    for(const auto &row : j) {
        if(!row.is_array())
	    throw JSONStoreError(std::string("expected element to be an array object"));
	if(row.size() != n)
            throw JSONStoreError(std::string("matrix is not square"));
    }
    for(int i = 0; i < n; ++i) 
        for(int k = 0; k <= i; ++k) 
                h(i, k) = j[i][k].template get<JSONDoubleElement>().value;
}

} // dakota namespace
