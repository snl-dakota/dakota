
// #if 0 // comment to make this file active
#ifdef DAKOTA_HAVE_HDF5

#include <Teuchos_UnitTestHarness.hpp>

#include "hdf5.h"    // C API
#include "hdf5_hl.h" // C API (HDF5 "high-level")
#include "H5Cpp.h"   // C++ API

#include <iostream>
#include <math.h>
#include <memory>
#include <string>

using namespace H5;

// Define globally available custom property lists.
LinkCreatPropList group_create_pl;


#define FILE "file.h5"

/**
 *  Insert a new method group into a Dakota HDF5 database at "/methods/<your method>."
 *  If "/methods" does not exist yet, it is automatically created for you.
 */
std::unique_ptr<Group> HDF5_add_method_group ( H5File* db_file, std::string method_name ) {
        bool status = db_file->exists("methods");
	Group group_methods;
	if( status ) {
		group_methods = db_file->openGroup("methods");
	} else {
		group_methods = db_file->createGroup("methods", group_create_pl);
	}

	std::unique_ptr<Group> group_method( new Group(group_methods.createGroup(method_name, group_create_pl)));
	return group_method;
}

/**
 * Creates an empty, N-dimensional dimension scale dataset.  Returns a pointer to
 * an open DataSet object.  It is up to the caller to then write to and close the DataSet.
 *
 * This function uses C API to set dimension scale information, as dimension scales
 * are not supported by HDF5's C++ API.
 *
 */
std::unique_ptr<DataSet> HDF5_create_dimension_scale (
        Group* parent, std::vector<int> dim_sizes, DataType type, std::string label) {

    int dims = dim_sizes.size();

    hsize_t ds_dims[dims];
    for(int i = 0; i < dims; i++) {
        ds_dims[i] = dim_sizes.at(i);
    }

    DataSpace ds_dataspace( dims, ds_dims );

    std::unique_ptr<DataSet> ds_dataset(
        new DataSet( parent->createDataSet( label.c_str(), type, ds_dataspace ) )
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
		Group* parent, int size, DataType type, std::string label ) {

	std::vector<int> dim_sizes;
	dim_sizes.push_back(size);

	std::unique_ptr<DataSet> ds_dataset = HDF5_create_dimension_scale(
		parent, dim_sizes, type, label
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

/**
 *  Test writing results from a probability density to an HDF5 database.
 */
TEUCHOS_UNIT_TEST(tpl_hdf5, new_hdf5_test) {

	/* FIELDS */
        // Use H5T_VARIABLE to create a variable-length string datatype.
	StrType str_type(0, H5T_VARIABLE);
        str_type.setCset(H5T_CSET_UTF8);  // set character encoding to UTF-8

        // Customize property lists
	group_create_pl.setCharEncoding(H5T_CSET_UTF8);
	// TODO The following variables should get passed in from elsewhere in Dakota.

	int  num_evaluations = 2;
	std::string  sampling_method_name = "sampling";

	std::array<double, 4> lower_bounds_arr = { 2.7604749078e+11, 3.6e+11, 4.0e+11, 4.4e+11 };
	std::array<double, 4> upper_bounds_arr = { 3.6e+11, 4.0e+11, 4.4e+11, 5.4196114379e+11 };
        std::array<std::array<double, 4>, 3> probability_density_arrs =
		{{{ 5.3601733194e-12, 4.25e-12, 3.75e-12, 2.2557612778e-12 },
		  { 2.8742313192e-05, 6.4e-05, 4.0e-05, 1.0341896485e-05 },
		  { 4.2844660868e-06, 8.6e-06, 1.8e-06, 1.8e-06 }}};

	double confidence_intervals_arrs[2][2];
        // lower and upper bounds for the mean
	confidence_intervals_arrs[0][0] = 4.25e-12;
	confidence_intervals_arrs[0][1] = 5.3601733194e-12;
	// lower and upper bounds for std deviation
        confidence_intervals_arrs[1][0] = 2.8742313192e-05;
	confidence_intervals_arrs[1][1] = 6.4e-05;

	/* LOGIC */
	// Part 1:  Write the data.

	std::unique_ptr<H5File> file( new H5File(FILE, H5F_ACC_TRUNC) );
	std::unique_ptr<Group>  group_method = HDF5_add_method_group(file.get(), sampling_method_name);

	for(int i = 1; i <= num_evaluations; i++) {
		std::string exec_id_path = "execution_id_" + std::to_string(i);
		Group group_exec_id( group_method->createGroup(exec_id_path, group_create_pl) );

		// Probability densities
		Group group_prob_dens( group_exec_id.createGroup("probability_density", group_create_pl) );
		Group group_prob_dens_scales( group_prob_dens.createGroup("_scales", group_create_pl) );

		std::unique_ptr<DataSet> ds_lower_bounds = HDF5_create_1D_dimension_scale (
			&group_prob_dens_scales, lower_bounds_arr.size(), PredType::IEEE_F64LE, "lower_bounds"
		);
	
		std::unique_ptr<DataSet> ds_upper_bounds = HDF5_create_1D_dimension_scale (
			&group_prob_dens_scales, upper_bounds_arr.size(), PredType::IEEE_F64LE, "upper_bounds"
		);

		ds_lower_bounds->write( lower_bounds_arr.data(), PredType::NATIVE_DOUBLE );
		ds_upper_bounds->write( upper_bounds_arr.data(), PredType::NATIVE_DOUBLE );

		for(int j = 0; j < probability_density_arrs.size(); j++) {
			hsize_t dims_ds[1];
			dims_ds[0] = probability_density_arrs[j].size();
			DataSpace probability_density_dataspace( 1, dims_ds );

			std::string dataset_resp_desc_name = "resp_desc_" + std::to_string(j+1);
			DataSet dataset_resp_desc = group_prob_dens.createDataSet(
				dataset_resp_desc_name, PredType::IEEE_F64LE, probability_density_dataspace
			);
			dataset_resp_desc.write(
				probability_density_arrs[j].data(), PredType::NATIVE_DOUBLE
			);

			HDF5_attach_dimension_scale ( &dataset_resp_desc, ds_lower_bounds.get(), 0 );
			HDF5_attach_dimension_scale ( &dataset_resp_desc, ds_upper_bounds.get(), 0 );
			
			// EMR close() happens automatically in object destructors.
			// probability_density_dataspace.close();
			// probability_density_dataset.close();
		}
		// JAS closing these dimension scale datasets invalidates their IDs, so it
		// must be deferred until after they are attached.	
		// EMR But do we even need to worry about closing explicitly?  Since closing &
		// object destruction are both handled automatically by respective APIs.
		ds_lower_bounds->close();
		ds_upper_bounds->close();
	
		// Confidence intervals	
		Group group_conf_int( group_exec_id.createGroup("confidence_intervals", group_create_pl) );
		Group group_conf_int_scales( group_conf_int.createGroup("_scales", group_create_pl) );

		std::array<const char*, 2> moments_arr = { "mean", "std_dev" };
		std::array<const char*, 2> bounds_arr  = { "lower_bounds", "upper_bounds" };

		std::unique_ptr<DataSet> ds_moments = HDF5_create_1D_dimension_scale (
            &group_conf_int_scales, moments_arr.size(), str_type, "moments"
        );
		ds_moments->write( moments_arr.data(), str_type );

		std::unique_ptr<DataSet> ds_bounds = HDF5_create_1D_dimension_scale (
            &group_conf_int_scales, bounds_arr.size(), str_type, "bounds"
        );
        ds_bounds->write( bounds_arr.data(), str_type );

		for(int j = 0; j < 3; j++) {
			hsize_t dims_ds[2];
			dims_ds[0] = 2;
			dims_ds[1] = 2;
			DataSpace conf_int_dataspace( 2, dims_ds );
			std::string dataset_resp_desc_name = "resp_desc_" + std::to_string(j+1);
			DataSet dataset_resp_desc = group_conf_int.createDataSet(
				dataset_resp_desc_name, PredType::IEEE_F64LE, conf_int_dataspace
			);
                        // Write dataset all at once from contiguous memory
			//dataset_resp_desc.write(confidence_intervals_arrs, 
                        //                        PredType::NATIVE_DOUBLE);
		        // Write a dataset one row at a time using hyperslab selections
			// Steps:
                        // 1. Create a 1x2 dataspace for the memory being read from.
                        // 2. Select a 1x2 hyperslab to write into the dataset 
                        // 3. Write the first row
                        // 4. Reset the selection for the hyperslab and change the offset
                        //    to write into the next row.
                        // 5. Write the second row.
			
			// Row 0
                        hsize_t mem_hs_ds[1] = {2};
                        DataSpace mem_hs(1, mem_hs_ds); // Memory dataspace for
                        DataSpace disk_hs = conf_int_dataspace;
                        hsize_t offset[2] = {0, 0}; // start position of hyperslab 
                        hsize_t count[2] = {1, 2};  // number of elements in each dimension for hyperslab
                        disk_hs.selectHyperslab(H5S_SELECT_SET, count, offset);
                        dataset_resp_desc.write(confidence_intervals_arrs, PredType::NATIVE_DOUBLE, 
                                                mem_hs, disk_hs);
                        // Row 1
                        disk_hs.selectNone(); // reset the selection.
                        offset[0] = 1;  // change the offset for the hyperslab to the next row
                        disk_hs.selectHyperslab(H5S_SELECT_SET, count, offset); // make a new selection
                        dataset_resp_desc.write(confidence_intervals_arrs + 2, PredType::NATIVE_DOUBLE, 
                                                mem_hs, disk_hs);

			HDF5_attach_dimension_scale ( &dataset_resp_desc, ds_moments.get(), 0 );
			HDF5_attach_dimension_scale ( &dataset_resp_desc, ds_bounds.get(), 1 );

		}

		ds_moments->close();
		ds_bounds->close();
	}

	// Part 2:  Re-open and verify the data.

	double TOL  = 1.0e-15;
	H5File h5file( FILE, H5F_ACC_RDONLY );

	Group group_methods          = h5file.openGroup("/methods");
	Group group_sampling         = group_methods.openGroup("sampling");
	Group group_exec_id_1        = group_sampling.openGroup("execution_id_1");
	Group group_prob_dens        = group_exec_id_1.openGroup("probability_density");
	Group group_prob_dens_scales = group_prob_dens.openGroup("_scales");

	Group group_conf_int         = group_exec_id_1.openGroup("confidence_intervals");
    Group group_conf_int_scales  = group_conf_int.openGroup("_scales");

    // Test lower_bounds dimension scale.
	DataSet dataset_lower_bounds = group_prob_dens_scales.openDataSet("lower_bounds");
	double data_out[4];
	hsize_t dimsm[1];  // memory space dimensions
	dimsm[0] = 4;
	DataSpace memspace( 1, dimsm );
	DataSpace dataspace = dataset_lower_bounds.getSpace();

	dataset_lower_bounds.read( data_out, PredType::NATIVE_DOUBLE, memspace, dataspace );
    TEST_FLOATING_EQUALITY( data_out[0], 2.7604749078e+11, TOL );
	TEST_FLOATING_EQUALITY( data_out[1], 3.6e+11, TOL );
	TEST_FLOATING_EQUALITY( data_out[2], 4.0e+11, TOL );
	TEST_FLOATING_EQUALITY( data_out[3], 4.4e+11, TOL );

	// Test upper_bounds dimension scale.
	DataSet dataset_upper_bounds = group_prob_dens_scales.openDataSet("upper_bounds");
	dataspace = dataset_upper_bounds.getSpace();

	dataset_upper_bounds.read( data_out, PredType::NATIVE_DOUBLE, memspace, dataspace );
	TEST_FLOATING_EQUALITY( data_out[0], 3.6e+11, TOL );
	TEST_FLOATING_EQUALITY( data_out[1], 4.0e+11, TOL );
	TEST_FLOATING_EQUALITY( data_out[2], 4.4e+11, TOL );
	TEST_FLOATING_EQUALITY( data_out[3], 5.4196114379e+11, TOL );

	// Test resp_desc datasets for probability densities.
	DataSet dataset_resp_desc_1 = group_prob_dens.openDataSet("resp_desc_1");
	dataspace = dataset_resp_desc_1.getSpace();
    dataset_resp_desc_1.read( data_out, PredType::NATIVE_DOUBLE, memspace, dataspace );
    TEST_FLOATING_EQUALITY( data_out[0], 5.3601733194e-12, TOL );
    TEST_FLOATING_EQUALITY( data_out[1], 4.25e-12, TOL );
    TEST_FLOATING_EQUALITY( data_out[2], 3.75e-12, TOL );
    TEST_FLOATING_EQUALITY( data_out[3], 2.2557612778e-12, TOL );

	DataSet dataset_resp_desc_2 = group_prob_dens.openDataSet("resp_desc_2");
	dataspace = dataset_resp_desc_2.getSpace();
    dataset_resp_desc_2.read( data_out, PredType::NATIVE_DOUBLE, memspace, dataspace );
    TEST_FLOATING_EQUALITY( data_out[0], 2.8742313192e-05, TOL );
    TEST_FLOATING_EQUALITY( data_out[1], 6.4e-05, TOL );
    TEST_FLOATING_EQUALITY( data_out[2], 4.0e-05, TOL );
    TEST_FLOATING_EQUALITY( data_out[3], 1.0341896485e-05, TOL );

	DataSet dataset_resp_desc_3 = group_prob_dens.openDataSet("resp_desc_3");
	dataspace = dataset_resp_desc_3.getSpace();
    dataset_resp_desc_3.read( data_out, PredType::NATIVE_DOUBLE, memspace, dataspace );
    TEST_FLOATING_EQUALITY( data_out[0], 4.2844660868e-06, TOL );
    TEST_FLOATING_EQUALITY( data_out[1], 8.6e-06, TOL );
    TEST_FLOATING_EQUALITY( data_out[2], 1.8e-06, TOL );
    TEST_FLOATING_EQUALITY( data_out[3], 1.8e-06, TOL );
	
	// Test confidence interval dimension scales.

	DataSet ds_moments = group_conf_int_scales.openDataSet("moments");
    char* rdata[2];

	hid_t		native_type;

	// Get datatype for dataset
    DataType dtype = ds_moments.getDataType();

    // Construct native type
    if((native_type = H5Tget_native_type(dtype.getId(), H5T_DIR_DEFAULT)) < 0) {
        std::cerr << "H5Tget_native_type failed!!! \n";
	}

    // Check if the data type is equal
    if(!H5Tequal(native_type, str_type.getId())) {
        std::cerr << "native type is not custom-defined srt_type!!! \n";
	}

    // Read dataset from disk
    ds_moments.read((void*)rdata, dtype);

    TEST_EQUALITY( std::strcmp(rdata[0], "mean"), 0 );
	TEST_EQUALITY( std::strcmp(rdata[1], "std_dev"), 0 );

	h5file.close();
	TEST_ASSERT( true );  // successfully terminated
}

#endif
// #endif // comment to make this file active
