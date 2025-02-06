/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef RESULTS_DB_BASE_H
#define RESULTS_DB_BASE_H

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


/**
 * Class: ResultsDBBase
 *
 * Description: A map-based container to store DAKOTA Iterator
 * results in underlying boost::anys, with optional metadata
 */
class ResultsDBBase
{

public:
  
  virtual ~ResultsDBBase() { /* empty dtor */ }

  /// If supported, flush data to the database or disk
  virtual
  void flush() const
    {return; }

  // TODO: For the following need const/non-const versions and
  // value/ref versions...

// ##############################################################
// Methods to support HDF5
// ##############################################################

  /// Pre-allocate a vector and (optionally) attach dimension scales and attributes. Insert
  /// elements using insert_into(...)
  virtual void allocate_vector(const StrStrSizet& iterator_id,
              const StringArray &location,
              ResultsOutputType stored_type, 
              const int &len,
              const DimScaleMap &scales = DimScaleMap(),
              const AttributeArray &attrs = AttributeArray()) = 0;

  /// Pre-allocate a matrix and (optionally) attach dimension scales and attributes. Insert
  /// rows or columns using insert_into(...)
  virtual void allocate_matrix(const StrStrSizet& iterator_id,
              const StringArray &location,
              ResultsOutputType stored_type, 
              const int &num_rows, const int &num_cols,
              const DimScaleMap &scales = DimScaleMap(),
              const AttributeArray &attrs = AttributeArray()) = 0;

  /// addition with dimension scales and attributes
  virtual void
  insert(const StrStrSizet& iterator_id,
         const StringArray &location,
         const boost::any& data,
         const DimScaleMap &scales = DimScaleMap(),
         const AttributeArray &attrs = AttributeArray(),
         const bool &transpose = false
         ) = 0;

  /// Insert a row or column into a pre-allocated matrix 
  virtual void
  insert_into(const StrStrSizet& iterator_id,
         const StringArray &location,
         const boost::any& data,
         const int &index, const bool &row) = 0;

  /// Add key:value metadata to a method
  virtual void add_metadata_to_method(
                    const StrStrSizet& iterator_id,
                    const AttributeArray &attrs
                    ) = 0;

  /// Add key:value metadata to an execution
  virtual void add_metadata_to_execution(
                    const StrStrSizet& iterator_id,
                    const AttributeArray &attrs
                    ) = 0;

  /// Associate key:value metadata with the object at the location
  virtual void add_metadata_to_object(const StrStrSizet& iterator_id,
                                      const StringArray &location,
                                      const AttributeArray &attrs) = 0;
 
  /// Associate key:value metadata to the study
  virtual void add_metadata_to_study(const AttributeArray &attrs) = 0;

  // ##############################################################
  // Methods to support legacy text output
  // ##############################################################

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

  /// record addition with metadata map
  virtual void 
  insert(const StrStrSizet& iterator_id,
         const std::string& data_name,
         const boost::any& result,
         const MetaDataType& metadata
         ) = 0;

//  /// return requested data by value in StoredType
//  template<typename StoredType>
//  StoredType get_data(const StrStrSizet& iterator_id,
//		      const std::string& data_name) const;
//
//  /// return requested data from array by value in StoredType
//  template<typename StoredType>
//  StoredType get_array_data(const StrStrSizet& iterator_id,
//			    const std::string& data_name,
//			    size_t index) const;
//
//  /// return pointer to stored data entry
//  template<typename StoredType>
//  const StoredType* get_data_ptr(const StrStrSizet& iterator_id,
//				 const std::string& result_key) const;

//  /// return pointer to stored data at given array location
//  template<typename StoredType>
//  const StoredType* get_array_data_ptr(const StrStrSizet& iterator_id,
//				       const std::string& data_name,
//				       size_t index) const;



protected:

//  /// attempt to find the requested data, erroring if not found
//  const ResultsValueType& lookup_data(const StrStrSizet& iterator_id,
//				      const std::string& data_name) const;
//
//  /// cast the reference to the any data to the requested type
//  template<typename StoredType>
//  StoredType cast_data(const boost::any& dataholder) const;
//
//  /// cast the pointer to the any data to the requested type
//  template<typename StoredType>
//  const StoredType* cast_data_ptr(const boost::any* dataholder) const;

  /// core data storage (map from key to value type)
  std::map<ResultsKeyType, ResultsValueType> iteratorData;

}; // class ResultsDBBase


// templated function definitions

template<typename StoredType>
void ResultsDBBase::
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
void ResultsDBBase::
array_insert(const StrStrSizet& iterator_id,
             const std::string& data_name, size_t index,
             const StoredType& sent_data)
{
  ResultsKeyType key = make_key(iterator_id, data_name);

  std::map<ResultsKeyType, ResultsValueType>::iterator data_it = 
    iteratorData.find(key);

  if (data_it != iteratorData.end()) {
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


//template<typename StoredType>
//StoredType ResultsDBBase::
//get_data(const StrStrSizet& iterator_id,
//	 const std::string& data_name) const
//{
//  const ResultsValueType& result_value = lookup_data(iterator_id, data_name);
//
//  // data is in first, discard metadata in second
//  return cast_data<StoredType>(result_value.first);
//}


//template<typename StoredType>
//StoredType ResultsDBBase::
//get_array_data(const StrStrSizet& iterator_id,
//	       const std::string& data_name, size_t index) const
//{
//  const ResultsValueType& result_value = lookup_data(iterator_id, data_name);
//
//  //TODO: bounds checking
//
//  // data is in first, discard metadata in second
//  return cast_data<std::vector<StoredType> >(result_value.first)[index];
//
//}


//template<typename StoredType>
//const StoredType* ResultsDBBase::
//get_data_ptr(const StrStrSizet& iterator_id,
//	     const std::string& data_name) const
//{
//  const ResultsValueType& result_value = lookup_data(iterator_id, data_name);
//
//  // data is in first, discard metadata in second
//  return cast_data_ptr<StoredType>(result_value.first);
//}


//template<typename StoredType>
//const StoredType* ResultsDBBase::
//get_array_data_ptr(const StrStrSizet& iterator_id,
//		   const std::string& data_name, size_t index) const
//{
//  const ResultsValueType& result_value = lookup_data(iterator_id, data_name);
//
//  // data is in first, discard metadata in second
//
//  // pointer to the array of data
//  // const std::vector<StoredType> *array_ptr = 
//  //   cast_data_ptr<const std::vector<StoredType> >(result_value.first);
//  // return (*array_ptr).data() + index;
//
//  //Call cast_data_ptr on pointer to any: any* = &result_value.first
//  const std::vector<StoredType>* array = 
//    cast_data_ptr<std::vector<StoredType> >(&result_value.first);
//  // TODO: FIX!!!
//  return &(*array)[index];
//
//  // get a pointer to the stored any as the reference will go out of scope...
//  // const std::vector<StoredType>* array = 
//  //   cast_data<std::vector<StoredType>*>(result_value.first);
//
//  // the std vector itself is (*array)
//  //return &(*array)[index];
//  //    return NULL;
//
//}


// Can't return const& through boost::any; instead one function to
// return value, another for pointer...
// these will make copies...

//template<typename StoredType>
//StoredType ResultsDBBase::cast_data(const boost::any& dataholder) const
//{
//
//  if (dataholder.type() != typeid(StoredType)) {
//    Cerr << "Couldn't cast retrieved data" << std::endl;
//    abort_handler(-1);
//  }
//   
//  return boost::any_cast<StoredType>(dataholder);
//
//}
//
//template<typename StoredType>
//const StoredType* ResultsDBBase::
//cast_data_ptr(const boost::any* dataholder) const
//{
//
//  if (dataholder->type() != typeid(StoredType)) {
//    Cerr << "Couldn't cast retrieved data" << std::endl;
//    abort_handler(-1);
//  }
//   
//  return boost::any_cast<StoredType>(dataholder);
//
//}


} // namespace Dakota

#endif  // RESULTS_DB_BASE_H
