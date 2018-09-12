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
              const HDF5dss &scales = HDF5dss()) override
  {
    // Store the results
    String dset_name = dataset_name(iterator_id, result_name, response_name);
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

    // Store and attach the dimension scales. Hopefully there's a nice way 
    // to refactor this.
    if(scale_is_real(scales)) {
      for(auto p : scales) {
        const int &index = p.first;
        const RealScale &scale = boost::any_cast<RealScale>(p.second);
        String name = scale_name(iterator_id, result_name, response_name, scale);
        if(!hdf5Stream->exists(name))
          hdf5Stream->store_vector_data(name, scale.items);
        hdf5Stream->attach_scale(dset_name, name, scale.label, index);
      }
    } else if(scale_is_string(scales)) {
      for(auto p : scales) {
        const int &index = p.first;
        const StringScale &scale = boost::any_cast<StringScale>(p.second);
        String name = scale_name(iterator_id, result_name, response_name, scale);
        if(!hdf5Stream->exists(name))
          hdf5Stream->store_vector_data(name, scale.items);
        hdf5Stream->attach_scale(dset_name, name, scale.label, index);
      }  
    } else {
      Cerr << "Scale is a disallowed type!" << std::endl;
    }  
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

  /// Create a dataset name from the unique identifiers passed
  String dataset_name(const StrStrSizet& iterator_id, const String& result_name, const String& response_name)
  {
    const String& method_name = iterator_id.get<0>();
    String method_id = iterator_id.get<1>();
    const size_t& exec_num = iterator_id.get<2>();

    if (method_id.empty())
      method_id = "anonymous";

    String result_name_wospace(result_name);
    //    boost::replace_all(data_name_wospace, " ", ":");
    //boost::erase_all(data_name_wospace, " ");
    
    String rval = "/methods/" + method_id + "/execution:" +
      boost::lexical_cast<String>(exec_num) + '/' + result_name_wospace;
    // some types of results, like correlation matrices, may have an empty response name
    if(!response_name.empty()) {
      rval += '/' + response_name;
    }
    return rval;
  }

  template<typename ScaleType>
  String scale_name(const StrStrSizet& iterator_id, const String& result_name, 
                    const String& response_name, const ScaleType& scale) {
    const String& method_name = iterator_id.get<0>();
    String method_id = iterator_id.get<1>();
    const size_t& exec_num = iterator_id.get<2>();

    if (method_id.empty())
      method_id = "anonymous";

    String result_name_wospace(result_name);
    //    boost::replace_all(data_name_wospace, " ", ":");
    //boost::erase_all(data_name_wospace, " ");
    
    // When scales are SHARED (by all the resposnes), they are stored under
    // _scales/label. When they are UNSHARED, they go under _scales/label/response

    String rval = "/_scales/methods/" + method_id + "/execution:" +
      boost::lexical_cast<String>(exec_num) + '/';
    if(!response_name.empty()) {
      rval += result_name_wospace + "/" + scale.label;
      if(scale.scope == ScaleScope::UNSHARED) 
        rval += '/' + response_name;
    } else { // No response name provided (e.g. correlation matrix)
      rval += scale.label;
    } 
    return rval;
  }

  /// BMA TODO: would prefer not to have a pointer, but no way to
  /// default construct an output handler
  std::unique_ptr<HDF5IOHelper> hdf5Stream;
};


}  // namespace Dakota

#endif //  DAKOTA_RESULTS_DB_HDF5_H
