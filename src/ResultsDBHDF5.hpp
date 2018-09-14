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
#include "HDF5_IO.hpp"

namespace Dakota {

// Helper functions for naming datasets and scales

/// Create a method name (HDF5 link name) from iterator_id
inline String method_hdf5_link_name(const StrStrSizet& iterator_id) {
  String method_id = iterator_id.get<1>();

  if (method_id.empty())
    method_id = "anonymous";

  String rval = "/methods/" + method_id;
  return rval;
}

/// Create an execution name (HDF5 link name) from iterator_id
inline String execution_hdf5_link_name(const StrStrSizet& iterator_id) {
  const size_t& exec_num = iterator_id.get<2>();
  String rval = method_hdf5_link_name(iterator_id) + "/execution:" +
    boost::lexical_cast<String>(exec_num);
  return rval;
}

/// Create a dataset name from the unique identifiers passed
inline String dataset_hdf5_link_name(const StrStrSizet& iterator_id, const String& result_name, const String& response_name)
{
  String result_name_wospace(result_name);
  //    boost::replace_all(data_name_wospace, " ", ":");
  //boost::erase_all(data_name_wospace, " ");
  
  String rval = execution_hdf5_link_name(iterator_id) + '/' + result_name_wospace;
  // some types of results, like correlation matrices, may have an empty response name
  if(!response_name.empty()) {
    rval += '/' + response_name;
  }
  return rval;
}

/// Create a scale name (hdf5 link name) for a scale
template<typename ScaleType>
String scale_hdf5_link_name(const StrStrSizet& iterator_id, const String& result_name, 
                  const String& response_name, const ScaleType& scale) {
  String result_name_wospace(result_name);
  //    boost::replace_all(data_name_wospace, " ", ":");
  //boost::erase_all(data_name_wospace, " ");
  
  // When scales are SHARED (by all the resposnes), they are stored under
  // _scales/label. When they are UNSHARED, they go under _scales/label/response

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
      iteratorID(iterator_id), resultName(result_name), responseName(response_name),
      index(index),dsetName(dset_name), hdf5Stream(hdf5_stream) {};

    template <typename T>
    void operator()(const T &scale) {
        String name = scale_hdf5_link_name(iteratorID, resultName, responseName, scale);
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
    hdf5Stream(new HDF5IOHelper(base_filename + (in_core ? ".tmp.h5" : ".h5"), true) )
  { 
  }


  /// record addition with metadata map
  void 
  insert(const StrStrSizet& iterator_id,
	 const std::string& data_name,
	 const boost::any& result,
	 const MetaDataType& metadata
	 ) override
  { std::cout << "ResultsDBHDF5 needs to implement insert(...) with metadata." << std::endl; }

  /// insert an arbitrary type (eg RealMatrix) with scales
  void insert(const StrStrSizet& iterator_id,
              const std::string& result_name,
              const std::string& response_name,
              const boost::any& data,
              const HDF5dss &scales = HDF5dss(),
              const AttributeArray &attrs = AttributeArray()) override
  {
    // Store the results
    String dset_name = dataset_hdf5_link_name(iterator_id, result_name, response_name);
    // Need to fix this to use incoming "data"
    if (data.type() == typeid(std::vector<double>)) {
      hdf5Stream->store_vector_data(dset_name, boost::any_cast<std::vector<double> >(data));
    }
    else if (data.type() == typeid(RealVector)) {
      hdf5Stream->store_vector_data(dset_name, boost::any_cast<RealVector>(data));
    }
    //  ----------------------------
    //  These are some more types that HDF5 I/O utils will need to support ...
    //  ----------------------------
    //else if (data.type() == typeid(std::vector<std::string>)) {
    //  hdf5Stream->store_vector_data(dset_name, boost::any_cast<std::vector<std::string> >(data));
    //}
    //else if (data.type() == typeid(std::vector<RealVector>)) {
    //  hdf5Stream->store_vector_data(dset_name, boost::any_cast<std::vector<RealVector> >(data));
    //}
    //else if (data.type() == typeid(std::vector<RealMatrix>)) {
    //  hdf5Stream->store_vector_data(dset_name, boost::any_cast<std::vector<RealMatrix> >(data));
    //}
    else
    {
      Cerr << "Warning: unknown type of any: " << data.type().name() << std::endl;
      abort_handler(-1);
    }

    // Store and attach the dimension scales.
    // Iteration must be explicit for the dimension scales because they are stored in a
    // multimap, which is a container of pairs, not of boost::variants
    for(auto &s : scales) {  // s is a std::pair<int, boost::variant<StringScale, RealScale> >
      int index = s.first;
      AttachScaleVisitor visitor(iterator_id, result_name, response_name, index, dset_name, hdf5Stream);
      boost::apply_visitor(visitor, s.second);

    }
    // Add metadata to the dataset
    AddAttributeVisitor attribute_adder(dset_name, hdf5Stream);
    std::for_each(attrs.begin(), attrs.end(), boost::apply_visitor(attribute_adder));
  }

  void add_metadata_for_method(const StrStrSizet& iterator_id,
              const AttributeArray &attrs = AttributeArray()) override
  {
    String name = method_hdf5_link_name(iterator_id);
    AddAttributeVisitor attribute_adder(name, hdf5Stream);
    std::for_each(attrs.begin(), attrs.end(), boost::apply_visitor(attribute_adder));
  }

  void add_metadata_for_execution(const StrStrSizet& iterator_id,
              const AttributeArray &attrs = AttributeArray()) override
  {
    String name = execution_hdf5_link_name(iterator_id);
    AddAttributeVisitor attribute_adder(name, hdf5Stream);
    std::for_each(attrs.begin(), attrs.end(), boost::apply_visitor(attribute_adder));
  }

 
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
