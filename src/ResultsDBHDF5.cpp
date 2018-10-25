/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 ResultsDBHDF5
//- Description: Implementation code for ResultsDBHDF5 class
//- Owner:       J. Adam Stephens
//- Checked by:
//- Version:

#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"
#include "dakota_global_defs.hpp"

#include <string>
#include <boost/lexical_cast.hpp>
#include "ResultsDBHDF5.hpp"

namespace Dakota { 
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
  String rval = method_hdf5_link_name(iterator_id) + "/execution:" +
    boost::lexical_cast<String>(exec_num);
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
  hdf5Stream->add_empty_dataset(dset_name, dims, stored_type);
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
  hdf5Stream->add_empty_dataset(dset_name, dims, stored_type);
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
    hdf5Stream->store_row_or_column(dset_name, boost::any_cast<std::vector<double> >(data), index, row);
  } else if (data.type() == typeid(RealVector)) {
    hdf5Stream->store_row_or_column(dset_name, boost::any_cast<RealVector >(data), index, row);
  } else if (data.type() == typeid(IntVector)) {
    hdf5Stream->store_row_or_column(dset_name, boost::any_cast<IntVector >(data), index, row);
  } else if (data.type() == typeid(StringMultiArrayConstView)) {
    hdf5Stream->store_row_or_column(dset_name, boost::any_cast<StringMultiArrayConstView>(data), index, row);
  } else if (data.type() == typeid(Real)) {
    hdf5Stream->store_element(dset_name, boost::any_cast<Real>(data), index);
  } else if (data.type() == typeid(int)) {
    hdf5Stream->store_element(dset_name, boost::any_cast<int>(data), index);
  } else if (data.type() == typeid(String)) {
    hdf5Stream->store_element(dset_name, boost::any_cast<String>(data), index);
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
    hdf5Stream->store_vector_data(
      dset_name, boost::any_cast<std::vector<double> >(data));
  }
  else if (data.type() == typeid(RealVector)) {
    hdf5Stream->store_vector_data(
      dset_name, boost::any_cast<RealVector>(data));
  }
  else if (data.type() == typeid(IntVector)) {
    hdf5Stream->store_vector_data(
      dset_name, boost::any_cast<IntVector>(data));
   }
  else if (data.type() == typeid(StringMultiArrayConstView)) {
    hdf5Stream->store_vector_string_data(dset_name, boost::any_cast<StringMultiArrayConstView>(data));
  }
  else if (data.type() == typeid(RealMatrix)) {
    hdf5Stream->store_matrix_data(dset_name, boost::any_cast<RealMatrix>(data), transpose);
  } else if (data.type() == typeid(Real)) {
    hdf5Stream->store_scalar_data(dset_name, boost::any_cast<Real>(data));
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
  String name = method_hdf5_link_name(iterator_id);
  AddAttributeVisitor attribute_adder(name, hdf5Stream);
  std::for_each(
    attrs.begin(), attrs.end(), boost::apply_visitor(attribute_adder)
  );
}

void ResultsDBHDF5::add_name_to_method(const StrStrSizet &iterator_id)
{
  String link_name = method_hdf5_link_name(iterator_id);
  AttributeArray attrs({ResultAttribute<String>("method_name", iterator_id.get<0>())});
  AddAttributeVisitor attribute_adder(link_name, hdf5Stream);
  std::for_each(
    attrs.begin(), attrs.end(), boost::apply_visitor(attribute_adder)
  );
  methodNameCache.insert(iterator_id.get<0>());
}

void ResultsDBHDF5::add_metadata_to_execution(const StrStrSizet& iterator_id,
            const AttributeArray &attrs) 
{
  String name = execution_hdf5_link_name(iterator_id);
  AddAttributeVisitor attribute_adder(name, hdf5Stream);
  std::for_each(
    attrs.begin(), attrs.end(), boost::apply_visitor(attribute_adder)
  );
}

  /// Associate key:value metadata with the object at the location
void ResultsDBHDF5::add_metadata_to_object(const StrStrSizet& iterator_id,
                                           const StringArray &location,
                                           const AttributeArray &attrs) {
  String name = object_hdf5_link_name(iterator_id, location);
  AddAttributeVisitor attribute_adder(name, hdf5Stream);
  std::for_each(
    attrs.begin(), attrs.end(), boost::apply_visitor(attribute_adder)
  );
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
add_attributes(const String dset_name, const AttributeArray &attrs) {
   // Add metadata to the dataset
  AddAttributeVisitor attribute_adder(dset_name, hdf5Stream);
  std::for_each(
    attrs.begin(), attrs.end(), boost::apply_visitor(attribute_adder)
  );
} 


void ResultsDBHDF5::flush() const {
  hdf5Stream->flush();
}

bool ResultsDBHDF5::method_in_cache(const StrStrSizet &iterator_id) const {
  if(methodNameCache.find(iterator_id.get<0>()) == methodNameCache.end())
    return false;
  else
    return true;
}

} // Dakota namespace
