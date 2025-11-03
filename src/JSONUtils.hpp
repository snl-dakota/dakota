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
