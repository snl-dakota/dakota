/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ResultsDBHDF5
//- Description:  Class for interfacing to HDF5 results database in/out of core
//- Owner:        Brian Adams
//- Version: $Id:$

#ifndef DAKOTA_RESULTS_DB_HDF5_H
#define DAKOTA_RESULTS_DB_HDF5_H

#include "ResultsDBBase.hpp"
// This file requires a complete implementation of HDF5_IO, so can't
// conditionally compile
#include "HDF5_IO.hpp"

namespace Dakota {

/// Create a method name (HDF5 link name) from iterator_id
String method_hdf5_link_name(const StrStrSizet& iterator_id);
/// Create an execution name (HDF5 link name) from iterator_id
String execution_hdf5_link_name(const StrStrSizet& iterator_id);
/// Create a scale name (hdf5 link name) for a scale
template<typename ScaleType>
String scale_hdf5_link_name(const StrStrSizet& iterator_id,
                            const String& result_name,
                            const String& response_name,
                            const ScaleType& scale) {
  String result_name_wospace(result_name);
  //    boost::replace_all(data_name_wospace, " ", ":");
  //boost::erase_all(data_name_wospace, " ");
  
  // When scales are SHARED (by all the responses), they are stored under
  // label. When they are UNSHARED, they go under label/response

  String rval = "/_scales" + execution_hdf5_link_name(iterator_id) + '/';
  if(!response_name.empty()) {
    rval += result_name_wospace + "/" + scale.label;
    if(scale.scope == ScaleScope::UNSHARED) 
      rval += '/' + response_name;
  } else { // No response name provided (e.g. correlation matrix)
    rval += scale.label;
  } 
  return rval;
}



// Visitor classes for processing scales and metadata
// These are needed by boost::variant to do compile-time checking of
// types.

class AddAttributeVisitor : public boost::static_visitor <>
{
  public:
    AddAttributeVisitor(const String &location, 
                        const std::shared_ptr<HDF5IOHelper> &hdf5_stream) : 
      location(location), hdf5Stream(hdf5_stream) {};

    template<typename T>
    void operator()(const ResultAttribute<T> & a) const
    {
      hdf5Stream->add_attribute(location, a.label, a.value);
    }

  private:
    String location;
    std::shared_ptr<HDF5IOHelper> hdf5Stream;
    
};

class AttachScaleVisitor : public boost::static_visitor <>
{
  public:
    AttachScaleVisitor(const StrStrSizet& iterator_id,
                       const std::string& result_name,
                       const std::string& response_name,
                       const int &index,
                       const String &dset_name,
                       const std::shared_ptr<HDF5IOHelper> &hdf5_stream) :
      iteratorID(iterator_id), resultName(result_name),
      responseName(response_name), index(index),dsetName(dset_name),
      hdf5Stream(hdf5_stream) {};

    template <typename T>
    void operator()(const T &scale) {
        String name =
          scale_hdf5_link_name(iteratorID, resultName, responseName, scale);
        if(!hdf5Stream->exists(name))
          hdf5Stream->store_vector_data(name, scale.items);
        hdf5Stream->attach_scale(dsetName, name, scale.label, index);
    }

  private:
    StrStrSizet iteratorID;
    String resultName;
    String responseName;
    int index;
    String dsetName;
    std::shared_ptr<HDF5IOHelper> hdf5Stream;
};


class ResultsDBHDF5 : public ResultsDBBase
{

public:

  ResultsDBHDF5(bool in_core, const String& base_filename) :
    hdf5Stream(new HDF5IOHelper(
      base_filename + (in_core ? ".tmp.h5" : ".h5"), true) )
  { 
  }


  /// record addition with metadata map
  void insert(const StrStrSizet& iterator_id,
	                const std::string& data_name,
                        const boost::any& result,
                        const MetaDataType& metadata) override;

  /// insert an arbitrary type (eg RealMatrix) with scales
  void insert(const StrStrSizet& iterator_id,
              const std::string& result_name,
              const std::string& response_name,
              const boost::any& data,
              const HDF5dss &scales = HDF5dss(),
              const AttributeArray &attrs = AttributeArray(),
              const bool &transpose = false) override;

  /// Add attributes to the HDF5 method group
  void add_metadata_for_method(const StrStrSizet& iterator_id,
              const AttributeArray &attrs) override;

  /// Add attributes to the HDF5 execution group
  void add_metadata_for_execution(const StrStrSizet& iterator_id,
              const AttributeArray &attrs) override;
 
  /// allocate an entry with sized array of the StoredType, e.g.,
  /// array across response functions or optimization results sets
  template<typename StoredType>
  void array_allocate(const StrStrSizet& iterator_id,
		      const std::string& data_name, size_t array_size,
		      const MetaDataType& metadata)
  {
    // TODO: consider whether alloc should be no-op in this case,
    // otherwise perhaps it should use array_size and maybe even a
    // non-default (HOW? additional args??) constructed StoredType
    // WJB: add a dbg print msg here since NOT sure tested for 'array_size > 1'
    std::vector<StoredType> initial_array( array_size, StoredType() );

    Cerr << "ResultsDBHDF5::insert is not fully supported.\n";
    abort_handler(-1);
  }
 
  template<typename StoredType>
  void array_insert(const StrStrSizet& iterator_id,
		    const std::string& data_name, size_t index,
		    const StoredType& stored_data)
  {
    Cerr << "ResultsDBHDF5::insert is not fully supported.\n";
    abort_handler(-1);
  }


private:
 
  /// BMA TODO: would prefer not to have a pointer, but no way to
  /// default construct an output handler
  std::shared_ptr<HDF5IOHelper> hdf5Stream;
};


}  // namespace Dakota

#endif //  DAKOTA_RESULTS_DB_HDF5_H
