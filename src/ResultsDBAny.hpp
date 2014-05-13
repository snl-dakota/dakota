/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ResultsDBAny
//- Description:  Any-based in-core results database
//- Owner:        Brian Adams
//- Version: $Id:$

#ifndef RESULTS_DB_ANY_H
#define RESULTS_DB_ANY_H

#include <string>
#include <vector>
#include <map>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/tuple/tuple_io.hpp>
#include <boost/any.hpp>
#include "dakota_global_defs.hpp"
#include "dakota_results_types.hpp"

// TODO: try/catch around any_cast

namespace Dakota {

/// Core data storage type: boost::any, with optional metadata
/// (see other types in results_types.hpp)
typedef std::pair<boost::any, MetaDataType> ResultsValueType;


/** Class: ResultsDBAny
    Description: A map-based container to store DAKOTA Iterator
    results in underlying boost::anys, with optional metadata */
class ResultsDBAny
{

public:

  
  // Templated allocation, insertion, and retrieval functions

  /// allocate an entry with sized array of the StoredType, e.g.,
  /// array across response functions or optimization results sets
  template<typename StoredType>
  void array_allocate(const StrStrSizet& iterator_id,
		      const std::string& data_name, size_t array_size,
		      const MetaDataType& metadata);

  /// insert sent_data in specified position in previously allocated array
  template<typename StoredType>
  void array_insert(const StrStrSizet& iterator_id,
		    const std::string& data_name, size_t index,
		    const StoredType& sent_data);

  // TODO: For the following need const/non-const versions and
  // value/ref versions...

  /// return requested data by value in StoredType
  template<typename StoredType>
  StoredType get_data(const StrStrSizet& iterator_id,
		      const std::string& data_name) const;

  /// return requested data from array by value in StoredType
  template<typename StoredType>
  StoredType get_array_data(const StrStrSizet& iterator_id,
			    const std::string& data_name,
			    size_t index) const;

  /// return pointer to stored data entry
  template<typename StoredType>
  const StoredType* get_data_ptr(const StrStrSizet& iterator_id,
				 const std::string& result_key) const;

  /// return pointer to stored data at given array location
  template<typename StoredType>
  const StoredType* get_array_data_ptr(const StrStrSizet& iterator_id,
				       const std::string& data_name,
				       size_t index) const;

  /// record addition with metadata map
  void 
  insert(const StrStrSizet& iterator_id,
	 const std::string& data_name,
	 const boost::any& result,
	 const MetaDataType& metadata
	 );

  // NOTE: removed accessors to add metadata only or record w/o metadata

  /// coarsely dump the data to the passed output stream
  void
  dump_data(std::ostream& output_stream);

  /// pretty print the data to the passed output stream
  void
  print_data(std::ostream& output_stream);


private:

  /// attempt to find the requested data, erroring if not found
  const ResultsValueType& lookup_data(const StrStrSizet& iterator_id,
				      const std::string& data_name) const;

  /// cast the reference to the any data to the requested type
  template<typename StoredType>
  StoredType cast_data(const boost::any& dataholder) const;

  /// cast the pointer to the any data to the requested type
  template<typename StoredType>
  const StoredType* cast_data_ptr(const boost::any* dataholder) const;

  /// print metadata to ostream
  void print_metadata(std::ostream& os, const MetaDataType& md);

  /// determine the type of contained data and output it to ostream
  void extract_data(const boost::any& dataholder, std::ostream& os);

  /// output data to ostream
  void output_data(const std::vector<double>& data, std::ostream& os);
  /// output data to ostream
  void output_data(const std::vector<RealVector>& data, std::ostream& os);
  /// output data to ostream
  void output_data(const std::vector<std::string>& data, std::ostream& os);
  /// output data to ostream
  void output_data(const std::vector<std::vector<std::string> >& data,
		   std::ostream& os);

  /// output data to ostream
  void output_data(const std::vector<RealMatrix>& data, 
		   std::ostream& os);
  /// output data to ostream
  void output_data(const RealMatrix& data, std::ostream& os);

  /// core data storage (map from key to value type)
  std::map<ResultsKeyType, ResultsValueType> iteratorData;

}; // class ResultsDBAny


// templated function definitions

template<typename StoredType>
void ResultsDBAny::
array_allocate(const StrStrSizet& iterator_id,
	       const std::string& data_name, size_t array_size,
	       const MetaDataType& metadata) 
{
  // add a vector of the StoredType, with metadata
  insert(iterator_id, data_name, std::vector<StoredType>(array_size),
	 metadata);
}

/** insert requires previous allocation, and does not allow metadata update */
template<typename StoredType>
void ResultsDBAny::
array_insert(const StrStrSizet& iterator_id,
	     const std::string& data_name, size_t index,
	     const StoredType& sent_data)
{
  ResultsKeyType key = make_key(iterator_id, data_name);

  std::map<ResultsKeyType, ResultsValueType>::iterator data_it = 
    iteratorData.find(key);

  if (data_it == iteratorData.end()) {
    Cerr << "\nError: Must allocate array before insert" 
	 << "\n  Iterator ID: " << iterator_id
	 << "\n  Data name: " << data_name
	 << std::endl;
    abort_handler(-1);
  }
  else {

    // update the any contained in the result set, which is a vector
    // of the StoredType
    ResultsValueType& result_value = data_it->second;

    // the first is the data, the second, the metadata
    std::vector<StoredType>& stored_data = 
      boost::any_cast<std::vector<StoredType>& >(result_value.first);

    if (stored_data.size() <= index) {
      Cerr << "\nResultsDB: array index exceeds allocated size." << std::endl;
      abort_handler(-1);
    }
    stored_data[index] = sent_data;

  }  

}


template<typename StoredType>
StoredType ResultsDBAny::
get_data(const StrStrSizet& iterator_id,
	 const std::string& data_name) const
{
  const ResultsValueType& result_value = lookup_data(iterator_id, data_name);

  // data is in first, discard metadata in second
  return cast_data<StoredType>(result_value.first);
}


template<typename StoredType>
StoredType ResultsDBAny::
get_array_data(const StrStrSizet& iterator_id,
	       const std::string& data_name, size_t index) const
{
  const ResultsValueType& result_value = lookup_data(iterator_id, data_name);

  //TODO: bounds checking

  // data is in first, discard metadata in second
  return cast_data<std::vector<StoredType> >(result_value.first)[index];

}


template<typename StoredType>
const StoredType* ResultsDBAny::
get_data_ptr(const StrStrSizet& iterator_id,
	     const std::string& data_name) const
{
  const ResultsValueType& result_value = lookup_data(iterator_id, data_name);

  // data is in first, discard metadata in second
  return cast_data_ptr<StoredType>(result_value.first);
}


template<typename StoredType>
const StoredType* ResultsDBAny::
get_array_data_ptr(const StrStrSizet& iterator_id,
		   const std::string& data_name, size_t index) const
{
  const ResultsValueType& result_value = lookup_data(iterator_id, data_name);

  // data is in first, discard metadata in second

  // pointer to the array of data
  // const std::vector<StoredType> *array_ptr = 
  //   cast_data_ptr<const std::vector<StoredType> >(result_value.first);
  // return (*array_ptr).data() + index;

  //Call cast_data_ptr on pointer to any: any* = &result_value.first
  const std::vector<StoredType>* array = 
    cast_data_ptr<std::vector<StoredType> >(&result_value.first);
  // TODO: FIX!!!
  return &(*array)[index];

  // get a pointer to the stored any as the reference will go out of scope...
  // const std::vector<StoredType>* array = 
  //   cast_data<std::vector<StoredType>*>(result_value.first);

  // the std vector itself is (*array)
  //return &(*array)[index];
  //    return NULL;

}


// Can't return const& through boost::any; instead one function to
// return value, another for pointer...
// these will make copies...

template<typename StoredType>
StoredType ResultsDBAny::cast_data(const boost::any& dataholder) const
{

  if (dataholder.type() != typeid(StoredType)) {
    Cerr << "Couldn't cast retrieved data" << std::endl;
    abort_handler(-1);
  }
   
  return boost::any_cast<StoredType>(dataholder);

}

template<typename StoredType>
const StoredType* ResultsDBAny::
cast_data_ptr(const boost::any* dataholder) const
{

  if (dataholder->type() != typeid(StoredType)) {
    Cerr << "Couldn't cast retrieved data" << std::endl;
    abort_handler(-1);
  }
   
  return boost::any_cast<StoredType>(dataholder);

}


} // namespace Dakota

#endif  // RESULTS_DB_ANY_H
