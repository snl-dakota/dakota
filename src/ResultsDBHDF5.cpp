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
String dataset_hdf5_link_name(const StrStrSizet& iterator_id,
                                     const String& lvl_1_name,
                                     const String& lvl_2_name)
{
  String rval = execution_hdf5_link_name(iterator_id)
                + '/' + lvl_1_name;
  // some types of results, like correlation matrices, may
  // have an empty lvl_2_name.
  if(!lvl_2_name.empty()) {
    rval += '/' + lvl_2_name;
  }
  return rval;
}

void ResultsDBHDF5::allocate_matrix(const StrStrSizet& iterator_id,
              const std::string& lvl_1_name,
              const std::string& lvl_2_name,
              ResultsOutputType stored_type, 
              int num_rows, int num_cols,
              const DimScaleMap &scales,
              const AttributeArray &attrs) {
  String dset_name = dataset_hdf5_link_name(iterator_id, lvl_1_name, lvl_2_name);
  IntArray dims = {num_rows, num_cols};
  hdf5Stream->add_empty_dataset(dset_name, dims, stored_type);
  attach_scales(dset_name, iterator_id, lvl_1_name, lvl_2_name, scales);
  add_attributes(dset_name, attrs);
} 

/// Insert a row or column into a pre-allocated matrix 
void ResultsDBHDF5:: 
insert_into_matrix(const StrStrSizet& iterator_id,
         const std::string& lvl_1_name,
         const std::string& lvl_2_name,
         const boost::any& data,
         const int &index, const bool &row) {
   // Store the results
  String dset_name =
    dataset_hdf5_link_name(iterator_id, lvl_1_name, lvl_2_name);
  // Need to fix this to use incoming "data"
  if (data.type() == typeid(std::vector<double>)) {
    hdf5Stream->store_row_or_column(dset_name, boost::any_cast<std::vector<double> >(data), index, row);
  }
  else if (data.type() == typeid(RealVector)) {
    hdf5Stream->store_row_or_column(dset_name, boost::any_cast<RealVector >(data), index, row);
  }
  else if (data.type() == typeid(IntVector)) {
    hdf5Stream->store_row_or_column(dset_name, boost::any_cast<IntVector >(data), index, row);
  }
  else if (data.type() == typeid(StringMultiArrayConstView)) {
    hdf5Stream->store_row_or_column(dset_name, boost::any_cast<StringMultiArrayConstView>(data), index, row);
  } else {
    Cerr << "Warning: dset " << dset_name << " of unknown type of any: " << data.type().name()
         << std::endl;
    abort_handler(-1);
  }
}

void 
ResultsDBHDF5::insert(const StrStrSizet& iterator_id,
       const std::string& data_name,
       const boost::any& result,
       const MetaDataType& metadata
       ) 
{
  return;
}

/// insert an arbitrary type (eg RealMatrix) with scales
void ResultsDBHDF5::insert(const StrStrSizet& iterator_id,
            const std::string& lvl_1_name,
            const std::string& lvl_2_name,
            const boost::any& data,
            const DimScaleMap &scales,
            const AttributeArray &attrs,
            const bool &transpose) 
{
  // Store the results
  String dset_name =
    dataset_hdf5_link_name(iterator_id, lvl_1_name, lvl_2_name);
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
  attach_scales(dset_name, iterator_id, lvl_1_name, lvl_2_name, scales);
  add_attributes(dset_name, attrs);

}

void ResultsDBHDF5::add_metadata_for_method(const StrStrSizet& iterator_id,
            const AttributeArray &attrs)
{
  String name = method_hdf5_link_name(iterator_id);
  AddAttributeVisitor attribute_adder(name, hdf5Stream);
  std::for_each(
    attrs.begin(), attrs.end(), boost::apply_visitor(attribute_adder)
  );
}

void ResultsDBHDF5::add_metadata_for_execution(const StrStrSizet& iterator_id,
            const AttributeArray &attrs) 
{
  String name = execution_hdf5_link_name(iterator_id);
  AddAttributeVisitor attribute_adder(name, hdf5Stream);
  std::for_each(
    attrs.begin(), attrs.end(), boost::apply_visitor(attribute_adder)
  );
}

void ResultsDBHDF5::
attach_scales(const String &dset_name,
            const StrStrSizet& iterator_id,
            const std::string& lvl_1_name,
            const std::string& lvl_2_name,
            const DimScaleMap &scales) {

  // Store and attach the dimension scales.
  // Iteration must be explicit for the dimension scales because they are stored in a
  // multimap, which is a container of pairs, not of boost::variants
  for(auto &s : scales) {  // s is a std::pair<int, boost::variant<StringScale, RealScale> >
    int dimension = s.first;
    AttachScaleVisitor visitor(
      iterator_id, lvl_1_name, lvl_2_name, dimension, dset_name, hdf5Stream
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


} // Dakota namespace
