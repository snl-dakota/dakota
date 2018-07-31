#if 0 // comment to make this file active
#ifdef DAKOTA_HAVE_HDF5

#include "hdf5.h"        // C API
#include "hdf5_hl.h"     // C API (HDF5 "high-level")
#include "H5Cpp.h"       // C++ API
#include "H5Exception.h" // HDF5 exceptions

#include <iostream>
#include <math.h>
#include <memory>
#include <string>

using namespace H5;

//////////////////////////////////////////////////////////////////////////////////////
//// These are helper functions that should be consolidated into the HDF5_IO class. //
//////////////////////////////////////////////////////////////////////////////////////


// Define globally available custom property lists.
LinkCreatPropList group_create_pl;
DSetCreatPropList dataset_compact_pl;
DSetCreatPropList dataset_contiguous_pl;

/**
 *  Create a new dataset at loc using the Dataset creation property list plist.
 *  It appears to be necessary to do dataset creation this way because H5Location::createDataset
 *  does not accept a LinkCreatPropList, which is needed to encode the name in UTF-8
 */
std::unique_ptr<DataSet> HDF5_create_dataset (
	const H5Location &loc, const std::string &name,	const DataType &type,
	const DataSpace &space, const DSetCreatPropList &plist )
{

	hid_t loc_id   = loc.getId();
	hid_t dtype_id = type.getId();
	hid_t space_id = space.getId();
	hid_t lcpl_id  = group_create_pl.getId();
	hid_t dcpl_id  = plist.getId();

	hid_t dataset_id = H5Dcreate2(
		loc_id, name.c_str(), dtype_id, space_id, lcpl_id, dcpl_id, H5P_DEFAULT
	);

	std::unique_ptr<DataSet> dataset(new DataSet(dataset_id));

	// the dataset_id is "taken over" by this DataSet object. Closing it would cause an
	// error when the dataset is used.

	return dataset;
}

/**
 *  Insert a new method group into a Dakota HDF5 database at "/methods/<your method>."
 *  If "/methods" does not exist yet, it is automatically created for you.
 */
std::unique_ptr<Group> HDF5_add_method_group ( H5File* db_file, std::string method_name )
{
	bool status = db_file->exists("methods");
    Group group_methods;
    if( status ) {
        group_methods = db_file->openGroup("methods");
    } else {
        group_methods = db_file->createGroup("methods", group_create_pl);
    }

    std::unique_ptr<Group> group_method(
		new Group(group_methods.createGroup(method_name, group_create_pl))
	);

    return group_method;
}

/**
 *  Creates an empty, N-dimensional dimension scale dataset.  Returns a pointer to
 *  an open DataSet object.  It is up to the caller to then write to and close the DataSet.
 *
 *  This function uses C API to set dimension scale information, as dimension scales
 *  are not supported by HDF5's C++ API.
 *
 */
std::unique_ptr<DataSet> HDF5_create_dimension_scale (
	Group* parent, std::vector<int> dim_sizes, DataType type,
	std::string label, DSetCreatPropList plist )
{
	int dims = dim_sizes.size();

	hsize_t ds_dims[dims];
	for(int i = 0; i < dims; i++) {
		ds_dims[i] = dim_sizes.at(i);
	}

	DataSpace ds_dataspace( dims, ds_dims );
	std::unique_ptr<DataSet> ds_dataset(
		HDF5_create_dataset( *parent, label, type, ds_dataspace, plist )
	);

	// Use C API to set the appropriate dimension scale metadata.
	hid_t  ds_dataset_id = (ds_dataset.get())->getId();
	herr_t ret_code      = H5DSset_scale( ds_dataset_id, label.c_str() );
	if(ret_code != 0) {
		throw std::runtime_error( "H5DSset_scale returned an error" );
	}

	return ds_dataset;
}

/**
 *  Create a 1D dimension scale with the length and label specified by the arguments.
 *  Returns a unique_ptr to an open DataSet object.  It is up to the caller to then
 *  write to and close the DataSet.
 *
 *  This function uses C API to set dimension scale information, as dimension scales
 *  are not supported by HDF5's C++ API.
 */
std::unique_ptr<DataSet> HDF5_create_1D_dimension_scale (
	Group* parent, int size, DataType type,
	std::string label, DSetCreatPropList plist )
{

	std::vector<int> dim_sizes;
	dim_sizes.push_back(size);

	std::unique_ptr<DataSet> ds_dataset = HDF5_create_dimension_scale(
		parent, dim_sizes, type, label, plist
	);
/*
	hsize_t ds_dims[1];
	ds_dims[0] = size;

	DataSpace ds_dataspace( 1, ds_dims );

	std::unique_ptr<DataSet> ds_dataset(
		new DataSet( parent->createDataSet( label.c_str(), type, ds_dataspace ))
	);
	// Use C API to set the appropriate dimension scale metadata.
	hid_t  ds_dataset_id = (ds_dataset.get())->getId();
	herr_t ret_code      = H5DSset_scale( ds_dataset_id, label.c_str() );
	if(ret_code != 0) {
		throw std::runtime_error( "H5DSset_scale returned an error" );
	}
*/

	return ds_dataset;
}

/**
 *  Attach a dimension scale DataSet to a target DataSet.
 *
 *  This function uses C API to set dimension scale information, as dimension scales
 *  are not supported by HDF5's C++ API.
 */
void HDF5_attach_dimension_scale ( DataSet* target, DataSet* ds_to_attach, int attach_index ) {
	// Use C API to attach a dataset to a dimension scale.
	hid_t target_id = target->getId();
	hid_t ds_id     = ds_to_attach->getId();

	herr_t ret_code = H5DSattach_scale(target_id, ds_id, attach_index );
	if(ret_code != 0) {
		throw std::runtime_error( "H5DSattach_scale returned an error" );
	}
}

#endif
#endif // comment to make this file active
