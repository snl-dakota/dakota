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
/// Create a scale name (hdf5 link name) for a scale from an iterator_id,
/// the name of the result, the name of the response (can be empty), and
/// the scale itself.
template<typename ScaleType>
String scale_hdf5_link_name(const StrStrSizet& iterator_id,
                            const String& result_name,
                            const String& response_name,
                            const ScaleType& scale) {
 
  // When scales are SHARED (by all the responses), they are stored under
  // label. When they are UNSHARED, they go under label/response

  String rval = "/_scales" + execution_hdf5_link_name(iterator_id) + '/';
  if(!response_name.empty()) {
    rval += result_name + "/" + scale.label;
    if(scale.scope == ScaleScope::UNSHARED) 
      rval += '/' + response_name;
  } else { // No response name provided (e.g. correlation matrix)
    rval += scale.label;
  } 
  return rval;
}



// Visitor classes for processing scales and attributes
// These are needed by boost::variant to do compile-time checking of
// types.

/// Objects of this class are called by boost::appy_visitor to add attributes
/// to HDF5 objects
class AddAttributeVisitor : public boost::static_visitor <>
{
  public:
    /// The attributes will be added to the HDF5 object at location, using
    /// the HDF5IOHelper instance hdf5_stream
    AddAttributeVisitor(const String &location, 
                        const std::shared_ptr<HDF5IOHelper> &hdf5_stream) : 
      location(location), hdf5Stream(hdf5_stream) {};

    /// Called by boost::apply_vistitor to process a ResultAttribute
    template<typename T>
    void operator()(const ResultAttribute<T> & a) const
    {
      hdf5Stream->add_attribute(location, a.label, a.value);
    }

  private:
    /// Link name of the HDF5 object to add attributes to
    String location;
    /// HDF5IOHelper instance
    std::shared_ptr<HDF5IOHelper> hdf5Stream;
    
};

/// Objects of this class are called by boost::appy_visitor to add dimension
/// scales (RealScale or StringScale) to HDF5 datasets
class AttachScaleVisitor : public boost::static_visitor <>
{
  public:
    /// Construct with context for attaching the scale, including the iterator id,
    /// the name of the result and response (can be empty), the dimension and name of
    /// the dataset to attach the scale to, the HDF5IOHelper instance.
    AttachScaleVisitor(const StrStrSizet& iterator_id,
                       const std::string& result_name,
                       const std::string& response_name,
                       const int &dim,
                       const String &dset_name,
                       const std::shared_ptr<HDF5IOHelper> &hdf5_stream) :
      iteratorID(iterator_id), resultName(result_name),
      responseName(response_name), dimension(dim),dsetName(dset_name),
      hdf5Stream(hdf5_stream) {};

    /// Called by boost::apply_vistitor to process a RealScale
    void operator()(const RealScale &scale) {
        String name =
          scale_hdf5_link_name(iteratorID, resultName, responseName, scale);
        if(!hdf5Stream->exists(name)) {
          if(scale.isMatrix) {
            Teuchos::SerialDenseMatrix<int, Real>scale_matrix(Teuchos::View, scale.items.values(), 
                sizeof(scale.items[0]), scale.items.length()/scale.numCols, scale.numCols);
            hdf5Stream->store_matrix_data(name, scale_matrix);
          } else
            hdf5Stream->store_vector_data(name, scale.items);
        }
        hdf5Stream->attach_scale(dsetName, name, scale.label, dimension);
    }

    /// Called by boost::apply_vistitor to process a StringScale
    void operator()(const StringScale &scale) {
        String name =
          scale_hdf5_link_name(iteratorID, resultName, responseName, scale);
        if(!hdf5Stream->exists(name)) {
          if(scale.isMatrix) 
            hdf5Stream->store_matrix_data(name, scale.items, scale.numCols);
          else
            hdf5Stream->store_vector_data(name, scale.items);
        }
        hdf5Stream->attach_scale(dsetName, name, scale.label, dimension);
    }


  private:
    /// Iterator ID for the method and execuation
    StrStrSizet iteratorID;
    /// Name of the result stored in the dataset
    String resultName;
    /// Possibly empty name of the response that the result is for
    String responseName;
    /// Dimension of the dataset to attach the scale to
    int dimension;
    /// Name of the dataset to attach the scale to
    String dsetName;
    /// Instance of HDF5IOHelper
    std::shared_ptr<HDF5IOHelper> hdf5Stream;
};

/// Manage interactions between ResultsManager and the low-level HDFIOHelper class
class ResultsDBHDF5 : public ResultsDBBase
{

public:

  ResultsDBHDF5(bool in_core, const String& base_filename) :
    ResultsDBBase(base_filename + (in_core ? ".tmp.h5" : ".h5")),
    hdf5Stream(new HDF5IOHelper(fileName, true))
    { }


  /// record addition with metadata map
  void insert(const StrStrSizet& iterator_id,
	                const std::string& data_name,
                        const boost::any& result,
                        const MetaDataType& metadata) override;

  /// insert an arbitrary type (eg RealMatrix) with scales
  void insert(const StrStrSizet& iterator_id,
              const std::string& lvl_1_name,
              const std::string& lvl_2_name,
              const boost::any& data,
              const DimScaleMap &scales = DimScaleMap(),
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
    return;
  }
 
  template<typename StoredType>
  void array_insert(const StrStrSizet& iterator_id,
		    const std::string& data_name, size_t index,
		    const StoredType& stored_data)
  {
    return;
  }

  /// Flush HDF5 cache to disk
  void flush() const;
private:
 
  /// Instance of HDF5IOHelper
  std::shared_ptr<HDF5IOHelper> hdf5Stream;
};


}  // namespace Dakota

#endif //  DAKOTA_RESULTS_DB_HDF5_H
