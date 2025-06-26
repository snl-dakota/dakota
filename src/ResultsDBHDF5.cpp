/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"
#include "dakota_global_defs.hpp"

#include <string>
#include "ResultsDBHDF5.hpp"

namespace Dakota { 

// Output file version: Major.minor.patch
// - Increment the patch version when making an update that does not change 
//   the content or layout of the output
// - Increment the minor version for an update that adds content to the file
// - Increment the major version for an update that changes the layout of 
//   existing content (i.e. could break existing user-created tools that consume
//   the output file)
//
// When incrementing the the major version, reset the minor and patch levels to 
// 0. Similarly, when incrementing the minor version, reset the patch level to 
// 0.

const std::string ResultsDBHDF5::outputVersion = "2.1.0";


// Helper functions for naming datasets and scales
String method_results_hdf5_link_name(const StrStrSizet& iterator_id) {
  return method_hdf5_link_name(iterator_id) + "/results";
}


// Helper functions for naming datasets and scales
String method_hdf5_link_name(const StrStrSizet& iterator_id) {
  String method_id = iterator_id.get<1>();

  // BMA: Method IDs should never be empty anymore, but verify before
  // throwing error
  if (method_id.empty())
    method_id = "anonymous";

  String rval = "/methods/" + method_id;
  return rval;
}

// Create an execution name from the unique identifiers passed
String execution_hdf5_link_name(const StrStrSizet& iterator_id) {
  const size_t& exec_num = iterator_id.get<2>();
  String rval = method_results_hdf5_link_name(iterator_id) + "/execution:" +
    std::to_string(exec_num);
  return rval;
}

// Create a dataset name from the unique identifiers passed
String object_hdf5_link_name(const StrStrSizet& iterator_id,
                              const StringArray &location) {
  String rval = execution_hdf5_link_name(iterator_id);

  for(const String &s : location)
    rval += "/" + s;
  return rval;
}

void ResultsDBHDF5::allocate_vector(const StrStrSizet& iterator_id,
              const StringArray &location,  
              ResultsOutputType stored_type, 
              const int &len,
              const DimScaleMap &scales,
              const AttributeArray &attrs) {
  bool method_exists = method_in_cache(iterator_id);
  String dset_name = object_hdf5_link_name(iterator_id, location);
  IntArray dims = {len};
  hdf5Stream->create_empty_dataset(dset_name, dims, stored_type);
  attach_scales(dset_name, iterator_id, location, scales);
  add_attributes(dset_name, attrs);
  if(!method_exists)
    add_name_to_method(iterator_id);
} 

void ResultsDBHDF5::allocate_matrix(const StrStrSizet& iterator_id,
              const StringArray &location,
              ResultsOutputType stored_type, 
              const int &num_rows, const int &num_cols,
              const DimScaleMap &scales,
              const AttributeArray &attrs) {
  bool method_exists = method_in_cache(iterator_id);
  String dset_name = object_hdf5_link_name(iterator_id, location);
  IntArray dims = {num_rows, num_cols};
  hdf5Stream->create_empty_dataset(dset_name, dims, stored_type);
  attach_scales(dset_name, iterator_id, location, scales);
  add_attributes(dset_name, attrs);
  if(!method_exists)
    add_name_to_method(iterator_id);
} 

/// Insert a row or column into a pre-allocated matrix 
void ResultsDBHDF5:: 
insert_into(const StrStrSizet& iterator_id,
         const StringArray &location,
         const boost::any& data,
         const int &index, const bool &row) {
   // Store the results
  String dset_name =
    object_hdf5_link_name(iterator_id, location);
  // Need to fix this to use incoming "data"
  if (data.type() == typeid(std::vector<double>)) {
    hdf5Stream->set_vector(dset_name, boost::any_cast<std::vector<double> >(data), index, row);
  } else if (data.type() == typeid(RealVector)) {
    hdf5Stream->set_vector(dset_name, boost::any_cast<RealVector >(data), index, row);
  } else if (data.type() == typeid(IntVector)) {
    hdf5Stream->set_vector(dset_name, boost::any_cast<IntVector >(data), index, row);
  } else if (data.type() == typeid(StringMultiArrayConstView)) {
    hdf5Stream->set_vector(dset_name, boost::any_cast<StringMultiArrayConstView>(data), index, row);
  } else if (data.type() == typeid(Real)) {
    hdf5Stream->set_scalar(dset_name, boost::any_cast<Real>(data), index);
  } else if (data.type() == typeid(int)) {
    hdf5Stream->set_scalar(dset_name, boost::any_cast<int>(data), index);
  } else if (data.type() == typeid(String)) {
    hdf5Stream->set_scalar(dset_name, boost::any_cast<String>(data), index);
  } else {
    Cerr << "Warning: dset " << dset_name << " of unknown type of any: " << data.type().name()
         << std::endl;
    abort_handler(-1);
  }
}


/// insert an arbitrary type (eg RealMatrix) with scales
void ResultsDBHDF5::insert(const StrStrSizet& iterator_id,
            const StringArray &location,
            const boost::any& data,
            const DimScaleMap &scales,
            const AttributeArray &attrs,
            const bool &transpose) 
{
  bool method_exists = method_in_cache(iterator_id);
  // Store the results
  String dset_name =
    object_hdf5_link_name(iterator_id, location);
  // Need to fix this to use incoming "data"
  if (data.type() == typeid(std::vector<double>)) {
    hdf5Stream->store_vector(
      dset_name, boost::any_cast<std::vector<double> >(data));
  }
  else if (data.type() == typeid(RealVector)) {
    hdf5Stream->store_vector(
      dset_name, boost::any_cast<RealVector>(data));
  }
  else if (data.type() == typeid(IntVector)) {
    hdf5Stream->store_vector(
      dset_name, boost::any_cast<IntVector>(data));
   }
  else if (data.type() == typeid(StringMultiArrayConstView)) {
    hdf5Stream->store_vector(dset_name, boost::any_cast<StringMultiArrayConstView>(data));
  }
  else if (data.type() == typeid(RealMatrix)) {
    hdf5Stream->store_matrix(dset_name, boost::any_cast<RealMatrix>(data), transpose);
  } else if (data.type() == typeid(Real)) {
    hdf5Stream->store_scalar(dset_name, boost::any_cast<Real>(data));
  }
  //  ----------------------------
  //  These are some more types that HDF5 I/O utils will need to support ...
  //  ----------------------------
  //else if (data.type() == typeid(std::vector<std::string>)) {
  //  hdf5Stream->store_vector(dset_name, boost::any_cast<std::vector<std::string> >(data));
  //}
  //else if (data.type() == typeid(std::vector<RealVector>)) {
  //  hdf5Stream->store_vector(dset_name, boost::any_cast<std::vector<RealVector> >(data));
  //}
  //else if (data.type() == typeid(std::vector<RealMatrix>)) {
  //  hdf5Stream->store_vector(dset_name, boost::any_cast<std::vector<RealMatrix> >(data));
  //}
  else
  {
    Cerr << "Warning: dset " << dset_name << " of unknown type of any: " << data.type().name()
         << std::endl;
    abort_handler(-1);
  }
  attach_scales(dset_name, iterator_id, location, scales);
  add_attributes(dset_name, attrs);
  if(!method_exists)
    add_name_to_method(iterator_id);
}

void ResultsDBHDF5::add_metadata_to_method(const StrStrSizet& iterator_id,
            const AttributeArray &attrs)
{
  String name = method_results_hdf5_link_name(iterator_id);
  add_attributes(name, attrs);
}

void ResultsDBHDF5::add_name_to_method(const StrStrSizet &iterator_id)
{
  String link_name = method_hdf5_link_name(iterator_id);
  String method_name = iterator_id.get<0>();
  String method_id = iterator_id.get<1>();
  size_t exec_id = iterator_id.get<2>();
  AttributeArray attrs({ResultAttribute<String>("method_name",method_name)});
  AddAttributeVisitor attribute_adder(link_name, hdf5Stream);
  add_attributes(link_name, attrs);
  methodIdCache.insert(method_id);
}

void ResultsDBHDF5::add_metadata_to_execution(const StrStrSizet& iterator_id,
            const AttributeArray &attrs) 
{
  String name = execution_hdf5_link_name(iterator_id);
  add_attributes(name, attrs);
}

void ResultsDBHDF5::add_metadata_to_object(const StrStrSizet& iterator_id,
                                           const StringArray &location,
                                           const AttributeArray &attrs) {
  String name = object_hdf5_link_name(iterator_id, location);
  add_attributes(name, attrs);
} 

void ResultsDBHDF5::add_metadata_to_study(const AttributeArray &attrs) {
  add_attributes(String("/"), attrs);
} 

void ResultsDBHDF5::
attach_scales(const String &dset_name,
            const StrStrSizet& iterator_id,
            const StringArray &location,
            const DimScaleMap &scales) {

  // Store and attach the dimension scales.
  // Iteration must be explicit for the dimension scales because they are stored in a
  // multimap, which is a container of pairs, not of boost::variants
  for(auto &s : scales) {  // s is a std::pair<int, boost::variant<StringScale, RealScale> >
    int dimension = s.first;
    AttachScaleVisitor visitor(
      iterator_id, location, dimension, dset_name, hdf5Stream
    );
    boost::apply_visitor(visitor, s.second);
  }
}

void ResultsDBHDF5::
add_attributes(const String &linkname, const AttributeArray &attrs) {
   // Add metadata to the dataset
  AddAttributeVisitor attribute_adder(linkname, hdf5Stream);
  std::for_each(
    attrs.begin(), attrs.end(), boost::apply_visitor(attribute_adder)
  );
} 


void ResultsDBHDF5::flush() const {
  hdf5Stream->flush();
}

bool ResultsDBHDF5::method_in_cache(const StrStrSizet &iterator_id) const {
  String method_id = iterator_id.get<1>();
  String method_name = iterator_id.get<0>();
  if(methodIdCache.find(method_id) != methodIdCache.end()) 
    return true;
  else 
    return false;
}

} // Dakota namespace
