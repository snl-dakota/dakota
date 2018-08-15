/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

// Results database supporting definitions
//  * Data type definitions for keys and meta data
//  * Free helper functions to create and manage data entries

#ifndef DAKOTA_RESULTS_TYPES_H
#define DAKOTA_RESULTS_TYPES_H

#include <map>
#include <vector>
#include <algorithm>
#include <boost/tuple/tuple.hpp>
#include "dakota_data_types.hpp"
#include <boost/any.hpp>


namespace Dakota {

// TODO: Using a tokenized string "instance::execution::result_key" or
// a list of keys might be more flexible going forward, instead of tuple.

// -----------
// Results Key
// -----------

/// Data type for results key (instance name / id, unique run, label),
/// where data_key is a valid colon-delimited string from ResultsNames 
/// tuple<method_name, method_id, execution_number, data_key>
typedef 
boost::tuple<std::string, std::string, size_t, std::string> ResultsKeyType;


// -----------
// Results Data
// -----------

// NOTE: the ResultsDataType varies by underlying database


// ----------------
// Results Metadata
// ----------------

/// Data type for metadata key
typedef std::string MetaDataKeyType;

/// Data type for metadata value
typedef std::vector<std::string> MetaDataValueType;

/// A single MetaData entry is map<string, vector<string> >  Example:
///   pair( "Column labels", ["Mean", "Std Dev", "Skewness", "Kurtosis"] )
typedef std::map<MetaDataKeyType, MetaDataValueType> MetaDataType;


// --------------
// Other Typedefs
// --------------

// TODO: This can probably go away
//typedef std::pair<std::string, size_t> StringSizetPair;

/// Iterator unique ID: <method_name, method_id, exec_num>
typedef boost::tuple<std::string, std::string, size_t> StrStrSizet;

/// Make a full ResultsKeyType from the passed iterator_id and data_name
ResultsKeyType make_key(const StrStrSizet& iterator_id, 
			const std::string& data_name);

/// create MetaDataValueType from the passed strings
MetaDataValueType 
make_metadatavalue(StringMultiArrayConstView labels);

/// create MetaDataValueType from the passed strings
MetaDataValueType 
make_metadatavalue(StringMultiArrayConstView cv_labels,
		   StringMultiArrayConstView div_labels,
		   StringMultiArrayConstView drv_labels,
		   const StringArray& resp_labels);

/// create MetaDataValueType from the passed strings
MetaDataValueType 
make_metadatavalue(const StringArray& resp_labels);

/// create MetaDataValueType from the passed strings
MetaDataValueType
make_metadatavalue(const std::string&);

/// create MetaDataValueType from the passed strings
MetaDataValueType
make_metadatavalue(const std::string&, const std::string&);

/// create MetaDataValueType from the passed strings
MetaDataValueType
make_metadatavalue(const std::string&, const std::string&, 
		   const std::string&);

/// create MetaDataValueType from the passed strings
MetaDataValueType
make_metadatavalue(const std::string&, const std::string&,
		   const std::string&, const std::string&);



// Inline definitions of helper functions

inline MetaDataValueType 
make_metadatavalue(StringMultiArrayConstView labels)
{
  MetaDataValueType mdv;
  for (size_t i=0; i<labels.size(); ++i)
    mdv.push_back(labels[i]);
  return mdv;
}


inline MetaDataValueType 
make_metadatavalue(StringMultiArrayConstView cv_labels,
		   StringMultiArrayConstView div_labels,
		   StringMultiArrayConstView dsv_labels,
		   StringMultiArrayConstView drv_labels,
		   const StringArray& resp_labels)
{
  // could use iterators for some of these
  MetaDataValueType mdv;
  for (size_t i=0; i<cv_labels.size(); ++i)
    mdv.push_back(cv_labels[i]);
  for (size_t i=0; i<div_labels.size(); ++i)
    mdv.push_back(div_labels[i]);
  for (size_t i=0; i<dsv_labels.size(); ++i)
    mdv.push_back(dsv_labels[i]);
  for (size_t i=0; i<drv_labels.size(); ++i)
    mdv.push_back(drv_labels[i]);
  for (size_t i=0; i<resp_labels.size(); ++i)
    mdv.push_back(resp_labels[i]);
  
  return mdv;
}

inline MetaDataValueType 
make_metadatavalue(const StringArray& resp_labels)
{
  // could use iterators for some of these
  MetaDataValueType mdv;
  for (size_t i=0; i<resp_labels.size(); ++i)
    mdv.push_back(resp_labels[i]);
  
  return mdv;
}

inline MetaDataValueType
make_metadatavalue(const std::string& s1)
{
  MetaDataValueType mdv;
  mdv.push_back(s1);
  return mdv;
}

inline MetaDataValueType
make_metadatavalue(const std::string& s1, const std::string& s2)
{
  MetaDataValueType mdv;
  mdv.push_back(s1);
  mdv.push_back(s2);
  return mdv;
}

inline MetaDataValueType
make_metadatavalue(const std::string& s1, const std::string& s2, 
		   const std::string& s3)
{
  MetaDataValueType mdv;
  mdv.push_back(s1);
  mdv.push_back(s2);
  mdv.push_back(s3);
  return mdv;
}

inline MetaDataValueType
make_metadatavalue(const std::string& s1, const std::string& s2,
		   const std::string& s3, const std::string& s4)
{
  MetaDataValueType mdv;
  mdv.push_back(s1);
  mdv.push_back(s2);
  mdv.push_back(s3);
  mdv.push_back(s4);
  return mdv;
}

inline ResultsKeyType make_key(const StrStrSizet& iterator_id, 
			       const std::string& data_name)
{
  return ResultsKeyType(iterator_id.get<0>(), 
			iterator_id.get<1>(), 
			iterator_id.get<2>(),
			data_name);
}

// For the HDF5 NonDSampling prototype, dimension scales may 
// accompany results to be written by the ResultsManager. They
// are of type multimap<int, boost::any>, with the boost::any
// containing either a RealScale or StringScale. These contain
// a label, which is like a heading for the scale, the items
// in the scale, and a ScaleScope enum, which determines whether
// the scale is SHARED among multiple responses or is unique
// to a particular response (UNHSARED).

enum class ScaleScope {SHARED, UNSHARED};

// RealScale and StringScale avoid making copies of data.
// RealScale stores items in a RealVector, which is capable
// of storing a "View" of the data provided to it.
struct RealScale {
  RealScale(const std::string &label, const RealVector &in_items, 
          ScaleScope scope = ScaleScope::UNSHARED) : 
          label(label), scope(scope) {
            items = RealVector(Teuchos::View, 
                *const_cast<RealVector*>(&in_items));
          }

  RealScale(const std::string &label, const RealArray &in_items, 
          ScaleScope scope = ScaleScope::UNSHARED) : 
          label(label), scope(scope) {
            items = RealVector(Teuchos::View, 
                const_cast<Real*>(in_items.data()),
                in_items.size());
          }


  RealScale(const std::string &label, const Real in_items[], 
          ScaleScope scope = ScaleScope::UNSHARED) : 
          label(label), scope(scope) {
            int len = sizeof(in_items)/sizeof(in_items[0]);
            items = RealVector(Teuchos::View,
                const_cast<Real*>(in_items), len);
          }

  RealScale(const std::string &label, const Real *in_items, const int len,
          ScaleScope scope = ScaleScope::UNSHARED) : 
          label(label), scope(scope) {
            items = RealVector(Teuchos::View,
                const_cast<Real*>(in_items), len);
          }


  std::string label;
  ScaleScope scope;
  RealVector items;
};

// A "view" of strings is stored as a vector of pointers to
// the provided data.
struct StringScale {
  StringScale(const std::string& in_label, const char * const in_items[], 
          ScaleScope in_scope = ScaleScope::UNSHARED) {
    label = in_label;
    int len = sizeof(in_items)/sizeof(in_items[0]);
    items.resize(len);
    std::copy(in_items, in_items + len, items.begin());
    scope = in_scope;
  }

  StringScale(const std::string & in_label, 
        std::initializer_list<const char *> in_items,
        ScaleScope in_scope = ScaleScope::UNSHARED) {
    label = in_label;
    items.resize(in_items.size());
    std::copy(in_items.begin(), in_items.end(), items.begin()); 
    scope = in_scope;
  }

  StringScale(const std::string& in_label, const std::vector<String> &in_items, 
          ScaleScope in_scope = ScaleScope::UNSHARED) {
    label = in_label;
    scope = in_scope;
    items.resize(in_items.size());
    std::transform(in_items.begin(), in_items.end(), items.begin(), 
      [](const String &s) { return s.c_str();});
  }

  std::string label;
  ScaleScope scope;
  std::vector<const char *> items;
};

typedef std::multimap<int, boost::any> HDF5dss;

inline
bool scale_is_real(const HDF5dss::iterator &i) {
  try {
    boost::any_cast<RealScale >(i->second);
    return true;
  }
  catch(const boost::bad_any_cast &) {
      return false;
  }
}

inline
bool scale_is_real(const HDF5dss &s) {
  if(s.size() == 0)
    return false;
  try {
    boost::any_cast<RealScale>(s.begin()->second);
    return true;
  }
  catch(const boost::bad_any_cast &) {
    return false;
 }
}

// Test functions for unpacking boost::anys that contain Real/StringScales.
inline
bool scale_is_string(const HDF5dss::iterator &i) {
  try {
    boost::any_cast<StringScale >(i->second);
    return true;
  }
  catch(const boost::bad_any_cast &) {
    return false;
  }
}

inline
bool scale_is_string(const HDF5dss &s) {
  if(s.size() == 0) {
    std::cerr << "DEBUG in scale_is_string, Size is 0" << std::endl;
    return false;
  }
  try {
    std::cerr << "DEBUG in scale_is_string try block" << std::endl;
    boost::any_cast<StringScale>(s.begin()->second);
    return true;
  }
  catch(const boost::bad_any_cast &) {
    std::cerr << "DEBUG in scale_is_string, execption caught!" << std::endl;
    return false;
  }
}


}  // namespace Dakota

#endif  // DAKOTA_RESULTS_TYPES_H
