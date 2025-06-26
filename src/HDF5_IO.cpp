/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "HDF5_IO.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "H5CompType.h"
#include "H5Cpp.h"  // C++ API
#include "dakota_data_types.hpp"
#include "dakota_global_defs.hpp"
#include "dakota_system_defs.hpp"
#include "hdf5.h"     // C API
#include "hdf5_hl.h"  // C H5Lite API

namespace Dakota {

//----------------------------------------------------------------
int length(const StringMultiArrayConstView &vec) { return vec.size(); }

HDF5IOHelper::HDF5IOHelper(const std::string &file_name, bool overwrite)
    : fileName(file_name) {
  // create or open a file
  // H5::Exception::dontPrint();
  if (overwrite) {
    // In H5F_ACC_TRUNC mode, if the file exists, HDF5 tries to open it
    // in read/write mode. This fails if the file is corrupt, so we
    // need to remove it first.
    std::remove(fileName.c_str());
    h5File = H5::H5File(fileName.c_str(), H5F_ACC_TRUNC);
  } else {
    try {
      h5File = H5::H5File(fileName.c_str(), H5F_ACC_RDWR);
    } catch (const H5::FileIException &) {
      h5File = H5::H5File(fileName.c_str(), H5F_ACC_TRUNC);
    }
  }
  // Initialize global Link Creation Property List to encode all link
  // (group, dataset) names in UTF-8
  linkCreatePL.setCharEncoding(H5T_CSET_UTF8);
}

void HDF5IOHelper::attach_scale(const String &dset_name,
                                const String &scale_name, const String &label,
                                const int &dim) const {
  create_groups(dset_name);

  H5::DataSet scale_ds(h5File.openDataSet(scale_name));
  H5::DataSet ds(h5File.openDataSet(dset_name));
  if (!is_scale(scale_ds)) {
    H5DSset_scale(scale_ds.getId(), label.c_str());
  }
  H5DSattach_scale(ds.getId(), scale_ds.getId(), dim);
}

bool HDF5IOHelper::exists(const String location_name) const {
  // the first group will be empty due to leading delimiter
  // the last group will be the dataset name
  std::vector<std::string> objects;
  boost::split(objects, location_name, boost::is_any_of("/"));

  // index instead of pruning first or clever iterators
  std::string full_path;
  for (size_t i = 1; i < objects.size(); ++i) {
    full_path += '/' + objects[i];
    // if doesn't exist, add
    if (!h5File.exists(full_path.c_str())) {
      return false;
    }
  }
  return true;
}

void HDF5IOHelper::store_scalar(const std::string &dset_name,
                                const String &val) {
  store_scalar(dset_name, val.c_str());
}

/// Store matrix (2D) information to a dataset
void HDF5IOHelper::store_matrix(const std::string &dset_name,
                                const std::vector<String> &buf,
                                const int &num_cols,
                                const bool &transpose) const {
  std::vector<const char *> ptrs_to_buf = pointers_to_strings(buf);
  store_matrix(dset_name, ptrs_to_buf, num_cols, transpose);
}

/// Set a scalar in a 1D dataset at index using an object
void HDF5IOHelper::set_scalar(const String &dset_name, H5::DataSet &ds,
                              const String &data, const int &index) {
  set_scalar(dset_name, ds, data.c_str(), index);
}

// There should be some template magic to combine this and the next
// case
void HDF5IOHelper::set_vector(const String &dset_name, H5::DataSet &ds,
                              const StringMultiArrayConstView &data,
                              const int &index, const bool &row) {
  // See coment comment on the String * version of
  // HDF5IOHelper::store_vector for an explanation of why this
  // overload is needed.

  std::vector<const char *> ptrs_to_data = pointers_to_strings(data);
  set_vector(dset_name, ds, ptrs_to_data, index, row);
}

void HDF5IOHelper::set_vector(const String &dset_name, H5::DataSet &ds,
                              const std::vector<String> &data, const int &index,
                              const bool &row) {
  // See coment comment on the String * version of
  // HDF5IOHelper::store_vector for an explanation of why this
  // overload is needed.

  std::vector<const char *> ptrs_to_data = pointers_to_strings(data);
  set_vector(dset_name, ds, ptrs_to_data, index, row);
}

/// Set a field on all elements of a 1D dataset of compound type using a ds
/// object.
void HDF5IOHelper::set_vector_vector_field(const String &dset_name,
                                           H5::DataSet &ds,
                                           const std::vector<String> &data,
                                           const size_t length,
                                           const String &field_name) {
  std::vector<const char *> ptrs_to_data = pointers_to_strings(data);
  set_vector_vector_field(dset_name, ds, ptrs_to_data, length, field_name);
}

/// Append a scalar to a 1D dataset
void HDF5IOHelper::append_scalar(const String &dset_name, const String &data) {
  append_scalar(dset_name, data.c_str());
}

/// Append a vector as a row or column to a 2D dataset
void HDF5IOHelper::append_vector(const String &dset_name,
                                 const std::vector<String> &data,
                                 const bool &row) {
  std::vector<const char *> ptrs_to_data = pointers_to_strings(data);
  append_vector(dset_name, ptrs_to_data, row);
}

/// Append a vector as a row or column to a 2D dataset
void HDF5IOHelper::append_vector(const String &dset_name,
                                 const StringMultiArrayConstView &data,
                                 const bool &row) {
  std::vector<const char *> ptrs_to_data = pointers_to_strings(data);
  append_vector(dset_name, ptrs_to_data, row);
}

/// Store vector (1D) information to a dataset
void HDF5IOHelper::store_vector(const std::string &dset_name,
                                const StringMultiArrayConstView &vec) {
  store_vector(dset_name, &vec[0], vec.size());
  return;
}

/// Store vector (1D) information to a dataset
void HDF5IOHelper::store_vector(const std::string &dset_name,
                                const SizetMultiArrayConstView &vec) {
  store_vector(dset_name, &vec[0], vec.size());
  return;
}

// Store vector of Strings using a pointer to the first element and
// length.
void HDF5IOHelper::store_vector(const String &dset_name, const String *data,
                                const int &len) const {
  // This overload is a workaround for a strange issue that
  // cropped up in GCC 5. Attemping to store string data caused a segfault in
  // libc. We could conditionally compile this code, but the overheard of
  // creating and assigning a vector of pointers usually won't be large, and
  // conditional compilation creates a maintenance issue.

  hsize_t dims[1];
  dims[0] = len;
  H5::DataSpace dataspace = H5::DataSpace(1, dims);
  H5::DataType f_datatype = h5_file_dtype(*data);
  H5::DataType m_datatype = h5_mem_dtype(*data);
  // Assume dset_name is syntactically correct - will need some utils - RWH
  create_groups(dset_name);
  H5::DataSet dataset(create_dataset(h5File, dset_name, f_datatype, dataspace));
  std::vector<const char *> ptrs_to_data(len);
  std::transform(data, data + len, ptrs_to_data.begin(),
                 [](const String &s) { return s.c_str(); });
  dataset.write(ptrs_to_data.data(), m_datatype);
  return;
}

void HDF5IOHelper::read_scalar(const std::string &dset_name, String &val) {
  if (!exists(dset_name)) {
    Cerr << "\nError: HDF5 file \"" << fileName << "\""
         << " does not contain data path \"" << dset_name << "\"" << std::endl;
    abort_handler(-1);
  }
  // JAS: We need some verification here that the dataset is really a scalar,
  // has the right type, etc..
  H5::DataSet dataset = h5File.openDataSet(dset_name);
  dataset.read(val, h5_mem_dtype(val));
  return;
}
/// Read vector (1D) String information from a dataset
void HDF5IOHelper::read_vector(const std::string &dset_name,
                               StringArray &array) const {
  // Reading an array of Strings is a special case because the C++ api for
  // reading a dataset expects a void * buffer. We have to create a char*[]
  // buffer, read into it, and then copy the strings into the StringArray.
  if (!exists(dset_name)) {
    Cerr << "\nError: HDF5 file \"" << fileName << "\""
         << " does not contain data path \"" << dset_name << "\"" << std::endl;
    abort_handler(-1);
  }

  H5::DataSet dataset = h5File.openDataSet(dset_name);

  // Get dims and size of dataset
  H5::DataSpace dspace = dataset.getSpace();
  assert(dspace.isSimple());
  int ndims = dspace.getSimpleExtentNdims();
  assert(ndims == 1);
  std::vector<hsize_t> dims(ndims, hsize_t(1));
  dspace.getSimpleExtentDims(dims.data());

  H5::DataType dtype = h5_mem_dtype(array[0]);
  array.resize(dims[0]);
  std::unique_ptr<char *[]> temp_array(new char *[dims[0]]);
  dataset.read(temp_array.get(), dtype);
  std::copy(temp_array.get(), temp_array.get() + dims[0], array.begin());
  return;
}

int HDF5IOHelper::append_empty(const String &dset_name) {
  H5::DataSet &ds = datasetCache[dset_name];
  H5::DataSpace f_space = ds.getSpace();
  hsize_t rank = f_space.getSimpleExtentNdims();
  // hsize_t dim[rank], maxdim[rank];
  // f_space.getSimpleExtentDims(dim, maxdim);
  std::unique_ptr<hsize_t[]> dim(new hsize_t[rank]), maxdim(new hsize_t[rank]);
  f_space.getSimpleExtentDims(dim.get(), maxdim.get());
  if (maxdim[0] != H5S_UNLIMITED) {
    flush();
    throw std::runtime_error(
        String("Attempt to append empty 'element' to a fixed-sized datasset ") +
        dset_name + " failed");
  }
  ++dim[0];
  ds.extend(dim.get());
  return dim[0] - 1;
}

// Create groups for the absolute path in name.
// The includes_dset option controls whether the final token in name is
// a dataset.
//
// Suppose name is /path/to/object
// For includes_dset == true, object is interpreted as the name of a
// dataset and only the groups /path/ and /path/to are created.
// For includes_dset == false, the groups /path, /path/to,
// and /path/to/object are created.
H5::Group HDF5IOHelper::create_groups(const std::string &name,
                                      bool includes_dset) const {
  // the first group will be empty due to leading delimiter
  // the last group will be the dataset name
  std::vector<std::string> groups;
  boost::split(groups, name, boost::is_any_of("/"));

  // index instead of pruning first and/or last or clever iterators
  std::string full_path;
  H5::Group new_group;
  size_t num_groups = (includes_dset) ? groups.size() - 1 : groups.size();
  for (size_t i = 1; i < num_groups; ++i) {
    full_path += '/' + groups[i];
    // if doesn't exist, add
    bool grpexists = h5File.exists(full_path.c_str());
    if (!grpexists) {
      new_group = create_group(h5File, full_path.c_str());
      /* Add Exception handling
      if (create_status < 0)
      {
        Cerr << "\nError: Could not create group hierarchy \""
             << full_path << "\""
             << " for HDF5 file \"" << fileName << "\"."
             << std::endl;
        abort_handler(-1);
      }
      */
    }
  }
  return new_group;
}

H5::DataSet HDF5IOHelper::create_dataset(
    const H5::H5Location &loc, const std::string &name,
    const H5::DataType &type, const H5::DataSpace &space,
    const H5::DSetCreatPropList &create_plist,
    const H5::DSetAccPropList &access_plist) const {
  hid_t loc_id = loc.getId();
  hid_t dtype_id = type.getId();
  hid_t space_id = space.getId();
  hid_t lcpl_id = linkCreatePL.getId();
  hid_t dcpl_id = create_plist.getId();
  hid_t dapl_id = access_plist.getId();
  hid_t dset_id = H5Dcreate2(loc_id, name.c_str(), dtype_id, space_id, lcpl_id,
                             dcpl_id, dapl_id);
  if (dset_id > 0) {
    H5::DataSet dataset(dset_id);
    H5Dclose(dset_id);
    return dataset;
  } else {
    flush();
    throw std::runtime_error(String("Attempt to create HDF5 dataset ") + name +
                             " failed");
  }
}

H5::Group HDF5IOHelper::create_group(const H5::H5Location &loc,
                                     const std::string &name) const {
  hid_t loc_id = loc.getId();
  hid_t lcpl_id = linkCreatePL.getId();
  hid_t gcpl_id = H5Pcreate(H5P_GROUP_CREATE);
  H5Pset_link_creation_order(gcpl_id, H5P_CRT_ORDER_TRACKED);
  hid_t group_id =
      H5Gcreate2(loc_id, name.c_str(), lcpl_id, gcpl_id, H5P_DEFAULT);
  H5Pclose(gcpl_id);
  if (group_id > 0) {
    H5::Group group(group_id);
    H5Gclose(group_id);
    return group;
  } else {
    flush();
    throw std::runtime_error(String("Attempt to create HDF5 group ") + name +
                             " failed");
  }
}

void HDF5IOHelper::create_empty_dataset(const String &dset_name,
                                        const IntArray &dims,
                                        ResultsOutputType stored_type,
                                        int chunk_size, const void *fill_val) {
  create_groups(dset_name);
  H5::DataType h5_type = h5_file_dtype(stored_type);
  H5::DataType fill_type = h5_mem_dtype(stored_type);
  hsize_t element_size = h5_type.getSize();
  int rank = dims.size();
  // hsize_t fdims[rank];
  // std::copy(dims.begin(), dims.end(), fdims);
  std::unique_ptr<hsize_t[]> fdims(new hsize_t[rank]);
  std::copy(dims.begin(), dims.end(), fdims.get());
  /* This block of code allows any dimenion to be unlimited
  if( std::find(dims.begin(), dims.end(), 0) != dims.end() ) { // dataset with
  unlmited dimension hsize_t chunks[rank]; hsize_t maxdims[rank]; for(int i = 0;
  i < rank; ++i) { if(dims[i]) maxdims[i] = chunks[i] = dims[i]; else {
        maxdims[i] = H5S_UNLIMITED;
        chunks[i] = chunk_size; // TODO: This only makes sense for 1 unlimited
  dimension
      }
    }
    H5::DataSpace dataspace = H5::DataSpace(rank, fdims, maxdims);
    H5::DSetCreatPropList plist;
    plist.setChunk(rank, chunks);
    create_dataset(h5File, dset_name, h5_type, dataspace, plist);
  } */
  if (!dims[0]) {
    if (std::any_of(++dims.begin(), dims.end(),
                    [](const int &a) { return a <= 0; })) {
      flush();
      throw std::runtime_error(
          String("Invalid dimensions supplied to HDF5IOHelper::") +
          "create_empty_dataset() for dataset " + dset_name);
    }
    // hsize_t chunks[rank];
    // hsize_t maxdims[rank];
    // std::copy(dims.begin(), dims.end(), chunks);
    // std::copy(dims.begin(), dims.end(), maxdims);
    std::unique_ptr<hsize_t[]> chunks(new hsize_t[rank]),
        maxdims(new hsize_t[rank]);
    std::copy(dims.begin(), dims.end(), chunks.get());
    std::copy(dims.begin(), dims.end(), maxdims.get());

    maxdims[0] = H5S_UNLIMITED;
    int num_layer_elements =
        std::accumulate(++dims.begin(), dims.end(), 1, std::multiplies<int>());
    int layer_size = element_size * num_layer_elements;
    int chunk0 = chunk_size / layer_size;
    chunks[0] = (chunk0) ? chunk0 : 1;
    int actual_chunksize =
        element_size *
        std::accumulate(&chunks[0], &chunks[rank], 1, std::multiplies<int>());
    H5::DataSpace dataspace = H5::DataSpace(rank, fdims.get(), maxdims.get());
    H5::DSetCreatPropList create_plist;
    create_plist.setChunk(rank, chunks.get());
    if (fill_val) create_plist.setFillValue(fill_type, fill_val);
    H5::DSetAccPropList access_plist;
    // See the C API documentation for H5P_set_chunk_cache for guidance
    const size_t cache_size = 20 * actual_chunksize;
    const size_t nslots = 2003;  // prime number ~ 100*10
    const double rddc_w0 = 0.9;
    access_plist.setChunkCache(nslots, cache_size, rddc_w0);

    datasetCache[dset_name] = create_dataset(
        h5File, dset_name, h5_type, dataspace, create_plist, access_plist);
  } else {  // fixed size
    H5::DataSpace dataspace = H5::DataSpace(rank, fdims.get());
    create_dataset(h5File, dset_name, h5_type, dataspace);
  }
}

/// Create a dataset with compound type
void HDF5IOHelper::create_empty_dataset(
    const String &dset_name, const IntArray &dims,
    const std::vector<VariableParametersField> &fields) {
  create_groups(dset_name);
  // 1. Create the fields as a vector of H5::DataTypes
  // 2. Create the Compound
  std::vector<std::unique_ptr<H5::DataType> > field_t;
  for (const auto &f : fields) {
    if (f.dims.empty()) {  // scalar
      field_t.emplace(field_t.end(), new H5::DataType(h5_file_dtype(f.type)));
    } else {  // non-scalar dataset field
      int ndims = f.dims.size();
      std::unique_ptr<hsize_t[]> field_dims(new hsize_t[ndims]);
      std::copy(f.dims.begin(), f.dims.end(), field_dims.get());
      field_t.emplace(
          field_t.end(),
          new H5::ArrayType(h5_file_dtype(f.type), ndims, field_dims.get()));
    }
  }
  size_t comp_t_size = 0;
  for (const auto &f : field_t) comp_t_size += f->getSize();
  H5::CompType comp_t(comp_t_size);
  size_t comp_t_offset = 0;
  for (int i = 0; i < fields.size(); ++i) {
    comp_t.insertMember(fields[i].name, comp_t_offset, *field_t[i]);
    comp_t_offset += field_t[i]->getSize();
  }

  int rank = dims.size();
  std::unique_ptr<hsize_t[]> fdims(new hsize_t[rank]);
  std::copy(dims.begin(), dims.end(), fdims.get());
  H5::DataSpace dataspace = H5::DataSpace(rank, fdims.get());
  create_dataset(h5File, dset_name, comp_t, dataspace);
}

bool HDF5IOHelper::is_scale(const H5::DataSet dset) const {
  htri_t status = H5DSis_scale(dset.getId());
  if (status > 0) {
    return true;
  } else if (status == 0) {
    return false;
  } else {
    Cerr << "Attempt to determine whether dataset is a scale failed.\n";
    return false;
  }
}

// Create a link at link_location, which is the full path to the link, that
// refers to the object at source_location.
void HDF5IOHelper::create_softlink(const String &link_location,
                                   const String &source_location) {
  create_groups(link_location);  // make sure a group exists to hold the link.
                                 // The source doesn't have to exist yet.
  h5File.link(H5G_LINK_SOFT, source_location, link_location);
}

void HDF5IOHelper::flush() const { h5File.flush(H5F_SCOPE_LOCAL); }

void HDF5IOHelper::report_num_open() {
  unsigned types[5] = {H5F_OBJ_FILE, H5F_OBJ_DATASET, H5F_OBJ_GROUP,
                       H5F_OBJ_DATATYPE, H5F_OBJ_ATTR};
  for (int i = 0; i < 5; ++i) {
    ssize_t cnt = h5File.getObjCount(types[i]);
    Cout << "Count of type " << i << " is " << cnt << std::endl;
  }
}

void HDF5IOHelper::add_attribute(const String &location, const String &label,
                                 const String &value) {
  add_attribute(location, label, value.c_str());
}

/*  H5::DataSet HDF5IOHelper::open_dataset(const String &dset_name) {
  if(datasetCache.find(dset_name) != datasetCache.end())
      return datasetCache[dset_name];
    else
      return h5File.openDataSet(dset_name);
}
*/
}  // namespace Dakota
