/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"
#include "dakota_global_defs.hpp"

#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string.hpp>

#include "H5Cpp.h"      // C++ API
#include "hdf5.h"       // C API
#include "hdf5_hl.h"    // C H5Lite API

#include <iostream>
#include <limits>
#include <memory>
#include <cmath>
#include <string>
#include <vector>
#include <functional>
#include "HDF5_IO.hpp"

namespace Dakota
{

//----------------------------------------------------------------
int length(const StringMultiArrayConstView &vec) {
  return vec.size();
}


  void HDF5IOHelper::attach_scale( const String& dset_name,
                                   const String& scale_name,
                                   const String& label,
                                   const int& dim) const
  {
    create_groups(dset_name);

    H5::DataSet scale_ds(h5File.openDataSet(scale_name));
    H5::DataSet ds(h5File.openDataSet(dset_name));
    if(!is_scale(scale_ds)) {
      H5DSset_scale(scale_ds.getId(), label.c_str() );
    }
    H5DSattach_scale(ds.getId(), scale_ds.getId(), dim );
  }


  bool HDF5IOHelper::exists(const String location_name) const
  {
    // the first group will be empty due to leading delimiter
    // the last group will be the dataset name
    std::vector<std::string> objects;
    boost::split(objects, location_name, boost::is_any_of("/"));

    // index instead of pruning first or clever iterators
    std::string full_path;
    for( size_t i=1; i<objects.size(); ++i ) {
      full_path += '/' + objects[i];
      // if doesn't exist, add
      if(!h5File.exists(full_path.c_str())) {
        return false;
      }
    }
    return true;
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
  H5::Group HDF5IOHelper::create_groups(const std::string& name,
                                        bool includes_dset) const
  {
    // the first group will be empty due to leading delimiter
    // the last group will be the dataset name
    std::vector<std::string> groups;
    boost::split(groups, name, boost::is_any_of("/"));

    // index instead of pruning first and/or last or clever iterators
    std::string full_path;
    H5::Group new_group;
    size_t num_groups = (includes_dset) ? groups.size() -1 : groups.size();
    for( size_t i = 1; i < num_groups; ++i ) {
      full_path += '/' + groups[i];
      // if doesn't exist, add
      bool grpexists = h5File.exists(full_path.c_str());
      if( !grpexists ) {
        new_group = h5File.createGroup(full_path.c_str(), linkCreatePL);
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
    const H5::DataType &type,  const H5::DataSpace &space,
    const H5::DSetCreatPropList &create_plist,
    const H5::DSetAccPropList &access_plist) const
  {
    hid_t loc_id   = loc.getId();
    hid_t dtype_id = type.getId();
    hid_t space_id = space.getId();
    hid_t lcpl_id  = linkCreatePL.getId();
    hid_t dcpl_id  = create_plist.getId();
    hid_t dapl_id  = access_plist.getId();
    hid_t dset_id =  H5Dcreate2(loc_id, name.c_str(), dtype_id, space_id, 
        lcpl_id, dcpl_id, dapl_id);
    if(dset_id > 0) {
      H5::DataSet dataset(dset_id);
      H5Dclose(dset_id);
      return dataset;
    }
    else {
      flush();
      throw std::runtime_error(String("Attempt to create HDF5 dataset ") + name + " failed" );
    }
  }

  void HDF5IOHelper::
  create_empty_dataset(const String &dset_name, const IntArray &dims, 
                    ResultsOutputType stored_type, int chunk_size) 
  {
    create_groups(dset_name);
    H5::DataType h5_type;
    switch (stored_type) {
      case ResultsOutputType::REAL:
        h5_type = h5_file_dtype(double(0.0));
        break;
      case ResultsOutputType::INTEGER:
        h5_type = h5_file_dtype(int(0));
        break;
      case ResultsOutputType::STRING:
        h5_type = h5_file_dtype(String(""));
        break;
    }
    hsize_t element_size = h5_type.getSize();
    int rank = dims.size();
    //hsize_t fdims[rank];
    //std::copy(dims.begin(), dims.end(), fdims);
	std::unique_ptr<hsize_t[]> fdims(new hsize_t[rank]);
	std::copy(dims.begin(), dims.end(), fdims.get());
    /* This block of code allows any dimenion to be unlimited 
    if( std::find(dims.begin(), dims.end(), 0) != dims.end() ) { // dataset with unlmited dimension
      hsize_t chunks[rank];
      hsize_t maxdims[rank];
      for(int i = 0; i < rank; ++i) {
        if(dims[i])
          maxdims[i] = chunks[i] = dims[i];
        else {
          maxdims[i] = H5S_UNLIMITED;
          chunks[i] = chunk_size; // TODO: This only makes sense for 1 unlimited dimension
        }
      }
      H5::DataSpace dataspace = H5::DataSpace(rank, fdims, maxdims);
      H5::DSetCreatPropList plist;
      plist.setChunk(rank, chunks);
      create_dataset(h5File, dset_name, h5_type, dataspace, plist);
    } */
    if(!dims[0]) {
      if(std::any_of(++dims.begin(), dims.end(), [](const int &a) {return a <= 0;})) {
        flush();
        throw std::runtime_error(String("Invalid dimensions supplied to HDF5IOHelper::") +
            "create_empty_dataset() for dataset " + dset_name);
      }
      //hsize_t chunks[rank];
      //hsize_t maxdims[rank];
      //std::copy(dims.begin(), dims.end(), chunks);
      //std::copy(dims.begin(), dims.end(), maxdims);
	  std::unique_ptr<hsize_t[]> chunks(new hsize_t[rank]), maxdims(new hsize_t[rank]);
	  std::copy(dims.begin(), dims.end(), chunks.get());
	  std::copy(dims.begin(), dims.end(), maxdims.get());

	  maxdims[0] = H5S_UNLIMITED;
      int num_layer_elements = std::accumulate(++dims.begin(), dims.end(), 1, std::multiplies<int>() );
      int layer_size = element_size*num_layer_elements;
      int chunk0 = chunk_size/layer_size;
      chunks[0] = (chunk0) ? chunk0 : 1;
      int actual_chunksize = element_size * std::accumulate(&chunks[0], &chunks[rank], 1, 
                                                            std::multiplies<int>() );
      H5::DataSpace dataspace = H5::DataSpace(rank, fdims.get(), maxdims.get());
      H5::DSetCreatPropList create_plist;
      create_plist.setChunk(rank, chunks.get());
      H5::DSetAccPropList access_plist;
      // See the C API documentation for H5P_set_chunk_cache for guidance
      const size_t cache_size = 20*actual_chunksize;
      const size_t nslots = 2003; // prime number ~ 100*10
      const double rddc_w0 = 0.9;
      access_plist.setChunkCache(nslots, cache_size, rddc_w0);
       
      datasetCache[dset_name] =  create_dataset(h5File, dset_name, h5_type, dataspace, create_plist, access_plist);
    } else { // fixed size
      H5::DataSpace dataspace = H5::DataSpace(rank, fdims.get());
      create_dataset(h5File, dset_name, h5_type, dataspace);
    }
  }

  bool HDF5IOHelper::is_scale(const H5::DataSet dset) const
  {
    htri_t status = H5DSis_scale(dset.getId());
    if(status > 0) {
      return true;
    } else if(status == 0) {
      return false;
    } else {
      Cerr << "Attempt to determine whether dataset is a scale failed.\n";
      return false;
    }
  }

  // Create a link at link_location, which is the full path to the link, that refers to
  // the object at source_location.
  void HDF5IOHelper::create_softlink(const String &link_location, const String &source_location) {
    create_groups(link_location); // make sure a group exists to hold the link. The source doesn't
                                  // have to exist yet.
    h5File.link(H5G_LINK_SOFT, source_location, link_location);
  }

  void HDF5IOHelper::flush() const {
    h5File.flush(H5F_SCOPE_LOCAL);
  }


  void HDF5IOHelper::report_num_open() {
    unsigned types[5] = {H5F_OBJ_FILE,
                         H5F_OBJ_DATASET,
                         H5F_OBJ_GROUP,
                         H5F_OBJ_DATATYPE,
                         H5F_OBJ_ATTR};
    for(int i = 0; i < 5; ++i) {
      ssize_t cnt = h5File.getObjCount(types[i]);
      Cout << "Count of type " << i << " is " << cnt << std::endl;
  
    }
  }
 
  H5::DataSet HDF5IOHelper::open_dataset(const String &dset_name) {
    if(datasetCache.find(dset_name) != datasetCache.end())
        return datasetCache[dset_name];
      else
        return h5File.openDataSet(dset_name);
  }
}
