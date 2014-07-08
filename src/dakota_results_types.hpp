/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

// Results database supporting definitions
//  * Data type definitions for keys and meta data
//  * Free helper functions to create and manage data entries

#ifndef RESULTS_TYPES_H
#define RESULTS_TYPES_H

#include <boost/tuple/tuple.hpp>
#include "dakota_data_types.hpp"


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

}  // namespace Dakota

#endif  // RESULTS_TYPES_H
