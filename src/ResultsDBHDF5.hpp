/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_RESULTS_DB_HDF5_H
#define DAKOTA_RESULTS_DB_HDF5_H
#include <memory>
#include <utility>
#include <set>
#include "ResultsDBBase.hpp"
// This file requires a complete implementation of HDF5_IO, so can't
// conditionally compile
#include "HDF5_IO.hpp"
#include "DakotaBuildInfo.hpp"

namespace Dakota {

/// Create a method name (HDF5 link name) from iterator_id
String method_hdf5_link_name(const StrStrSizet& iterator_id);
/// Create a method results name (HDF5 link name) from iterator_id
String method_results_hdf5_link_name(const StrStrSizet& iterator_id);
/// Create an execution name (HDF5 link name) from iterator_id
String execution_hdf5_link_name(const StrStrSizet& iterator_id);
/// Create a scale name (hdf5 link name) for a scale from an iterator_id,
/// the name of the result, the name of the response (can be empty), and
/// the scale itself.
template<typename ScaleType>
String scale_hdf5_link_name(const StrStrSizet& iterator_id,
                            const StringArray &location,
                            const ScaleType& scale) {
 
  // When scales are SHARED (by all the responses), they are stored under
  // label. When they are UNSHARED, they go under label/response

  String rval = "/_scales" + execution_hdf5_link_name(iterator_id);
  for(size_t i = 0; i < location.size() - 1; ++i)
    rval += "/" + location[i];
  
  if(scale.scope == ScaleScope::SHARED)
    rval += "/" + scale.label;
  else
    rval += "/" + *(location.end() - 1) + "/" + scale.label;
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
    /// Construct with context for attaching the scale, including the iterator and
    /// location used to construct the scale, the dimension and name of
    /// the dataset to attach the scale to, the HDF5IOHelper instance.
    AttachScaleVisitor(const StrStrSizet& iterator_id,
                       const StringArray &location,
                       const int &dim,
                       const String &dset_name,
                       const std::shared_ptr<HDF5IOHelper> &hdf5_stream) :
      iteratorID(iterator_id), location(location), dimension(dim), dsetName(dset_name),
      hdf5Stream(hdf5_stream) {};

    /// Called by boost::apply_vistitor to process a RealScale
    void operator()(const RealScale &scale) {
        String name =
          scale_hdf5_link_name(iteratorID, location, scale);
        if(!hdf5Stream->exists(name)) {
          if(scale.isMatrix) {
            Teuchos::SerialDenseMatrix<int, Real>scale_matrix(Teuchos::View, scale.items.values(), 
                sizeof(scale.items[0]), scale.items.length()/scale.numCols, scale.numCols);
            hdf5Stream->store_matrix(name, scale_matrix);
          } else
            hdf5Stream->store_vector(name, scale.items);
        }
        hdf5Stream->attach_scale(dsetName, name, scale.label, dimension);
    }

    /// Called by boost::apply_vistitor to process a StringScale
    void operator()(const StringScale &scale) {
        String name =
          scale_hdf5_link_name(iteratorID, location, scale);
        if(!hdf5Stream->exists(name)) {
          if(scale.isMatrix) 
            hdf5Stream->store_matrix(name, scale.items, scale.numCols);
          else
            hdf5Stream->store_vector(name, scale.items);
        }
        hdf5Stream->attach_scale(dsetName, name, scale.label, dimension);
    }

    /// Called by boost::apply_vistitor to process an IntegerScale
    void operator()(const IntegerScale &scale) {
        String name =
          scale_hdf5_link_name(iteratorID, location, scale);
        if(!hdf5Stream->exists(name)) {
          if(scale.isMatrix) 
            hdf5Stream->store_matrix(name, scale.items, scale.numCols);
          else
            hdf5Stream->store_vector(name, scale.items);
        }
        hdf5Stream->attach_scale(dsetName, name, scale.label, dimension);
    }

  private:
    /// Iterator ID for the method and execuation
    StrStrSizet iteratorID;
    /// Location used to create the dataset 
    StringArray location;
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
  // We are always overriting any existing HDF5 files. If we eventually
  // support appending to existing files, then the methodNameCache
  // will need to be initialized from the file.
  ResultsDBHDF5(bool in_core, std::shared_ptr<HDF5IOHelper> hdf5_helper_ptr) :
    hdf5Stream(hdf5_helper_ptr) {

    AttributeArray root_attrs = {
        ResultAttribute<String>("dakota_version", DakotaBuildInfo::get_release_num()),
        ResultAttribute<String>("dakota_revision", DakotaBuildInfo::get_rev_number()),
        ResultAttribute<String>("output_version", ResultsDBHDF5::outputVersion)
      };
    add_attributes("/", root_attrs);
  }

  /// Flush HDF5 cache to disk
  void flush() const override;

  /// Pre-allocate a vector and (optionally) attach dimension scales and attributes. Insert
  /// elements using insert_into(...)
  void allocate_vector(const StrStrSizet& iterator_id,
              const StringArray &location,
              ResultsOutputType stored_type, 
              const int &len,
              const DimScaleMap &scales = DimScaleMap(),
              const AttributeArray &attrs = AttributeArray()) override;

  /// Pre-allocate a matrix and (optionally) attach dimension scales and attributes. Insert
  /// rows or columns using insert_into(...)
  void allocate_matrix(const StrStrSizet& iterator_id,
              const StringArray &location,
              ResultsOutputType stored_type, 
              const int &num_rows, const int &num_cols,
              const DimScaleMap &scales = DimScaleMap(),
              const AttributeArray &attrs = AttributeArray()) override;


  /// Insert a row or column into a pre-allocated matrix 
  void insert_into(const StrStrSizet& iterator_id,
         const StringArray &location,
         const boost::any& data,
         const int &index, const bool &row) override;

  /// insert an arbitrary type (eg RealMatrix) with scales
  void insert(const StrStrSizet& iterator_id,
              const StringArray &location,
              const boost::any& data,
              const DimScaleMap &scales = DimScaleMap(),
              const AttributeArray &attrs = AttributeArray(),
              const bool &transpose = false) override;

  /// Add attributes to the HDF5 method group
  void add_metadata_to_method(const StrStrSizet& iterator_id,
              const AttributeArray &attrs) override;

  /// Add attributes to the HDF5 execution group
  void add_metadata_to_execution(const StrStrSizet& iterator_id,
              const AttributeArray &attrs) override;

  /// Associate key:value metadata with the object at the location
  void add_metadata_to_object(const StrStrSizet& iterator_id,
                              const StringArray &location,
                              const AttributeArray &attrs) override; 

  /// Associate key:value metadata with the study
  void add_metadata_to_study(const AttributeArray &attrs) override; 

  // ##############################################################
  // Methods to support legacy Any DB (no-op for HDF5)
  // ##############################################################

  /// record addition with metadata map
  void insert(const StrStrSizet& iterator_id,
	                const std::string& data_name,
                        const boost::any& result,
                        const MetaDataType& metadata) override
  { return; }

  /// allocate an entry with sized array of the StoredType, e.g.,
  /// array across response functions or optimization results sets
  template<typename StoredType>
  void array_allocate(const StrStrSizet& iterator_id,
		      const std::string& data_name, size_t array_size,
		      const MetaDataType& metadata)
  { return; }
 
  template<typename StoredType>
  void array_insert(const StrStrSizet& iterator_id,
		    const std::string& data_name, size_t index,
		    const StoredType& stored_data)
  { return; }

private:

  /// Version of the output file. See comments near the definition in
  /// ResultsDBHDF5.cpp.
  static const std::string outputVersion;

  /// Attach a scale to a dataset
  void attach_scales(const String &dset_name,
            const StrStrSizet& iterator_id,
            const StringArray &location,
            const DimScaleMap &scales);

  /// Add attributes to the object with linkname.
  void add_attributes(const String &linkname, const AttributeArray &attrs);

  /// Add the name (Dakota keyword) as metadata to a method group
  void add_name_to_method(const StrStrSizet &iterator_id);

  /// Check whether the name has already been added to a  method group 
  bool method_in_cache(const StrStrSizet &iterator_id) const;

  /// Cached method Ids; used to know which methods have already had their 
  /// method_name attribute set. Hopefully faster than querying the HDF5 file.
  std::set<String> methodIdCache;
  /// Instance of HDF5IOHelper (must be a pointer because it's shared with the global
  /// evaluation store instance 
  std::shared_ptr<HDF5IOHelper> hdf5Stream;
};


}  // namespace Dakota

#endif //  DAKOTA_RESULTS_DB_HDF5_H
