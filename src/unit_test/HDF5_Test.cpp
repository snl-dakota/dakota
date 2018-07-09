
// #if 0// comment to make this file active

#include "hdf5.h"    // C API
#include "hdf5_hl.h" // C API (HDF5 "high-level")
#include "H5Cpp.h"   // C++ API
#include <string>
using namespace H5;

#define FILE "file.h5"

int HDF5_Test() {

	/* FIELDS */
	// TODO In general, the following variables should get passed in from elsewhere in Dakota.

	int  num_evaluations = 2;
	int  probability_density_dataset_count = 3;
	std::string  method_name = "sampling";
	int  bin_number = 2;
        float lower_bounds_arr[5] = { 2.7604749078e+11, 3.6000000000e+11, 4.0000000000e+11, 4.4000000000e+11 };
        float upper_bounds_arr[5] = { 3.6000000000e+11, 4.0000000000e+11, 4.4000000000e+11, 5.4196114379e+11 };
	
	/* LOGIC */

	/* Create a new file using default properties.*/
	H5File* file_ptr = new H5File(FILE, H5F_ACC_TRUNC);

	/* Create methods group */
	Group group_methods(file_ptr->createGroup("/methods"));

	/* Create specific method group */
	std::string group_method_path = "/methods/" + method_name;
	Group group_method(group_methods.createGroup(group_method_path));

	/* Create execution groups */
	for(int i = 1; i <= num_evaluations; i++) {
		std::string exec_id_path = group_method_path + "/execution_id_" + std::to_string(i);
		Group group_exec_id(group_method.createGroup(exec_id_path));

		/* Create probability density group */
		std::string probability_density_path = exec_id_path + "/probability_density";
		Group probability_density_group(group_exec_id.createGroup(probability_density_path));

		/* Create _scales group */
		Group scales_group(probability_density_group.createGroup("/_scales"));
		
		/* Dimension scales */
                PredType type_ieee_f64le( PredType::IEEE_F64LE );

		hsize_t dims_lb[1];
		hsize_t dims_ub[1];

		dims_lb[0] = sizeof(lower_bounds_arr);
		dims_ub[0] = sizeof(upper_bounds_arr);

                DataSpace scales_lb_dataspace( 1, dims_lb );
		DataSpace scales_ub_dataspace( 1, dims_ub );

		DataSet dim_scale_ds_lower_bounds = scales_group.createDataSet("lower_bounds", type_ieee_f64le, scales_lb_dataspace);
		DataSet dim_scale_ds_upper_bounds = scales_group.createDataSet("upper_bounds", type_ieee_f64le, scales_ub_dataspace);

		dim_scale_ds_lower_bounds.write(lower_bounds_arr, type_ieee_f64le);
		dim_scale_ds_upper_bounds.write(upper_bounds_arr, type_ieee_f64le);
		
		// Convert datasets to dimension scales (using C API)
		hid_t  dim_scale_ds_lower_bounds_id = dim_scale_ds_lower_bounds.getId();
		hid_t  dim_scale_ds_upper_bounds_id = dim_scale_ds_upper_bounds.getId();

		herr_t ret_code_lb = H5DSset_scale(dim_scale_ds_lower_bounds_id, "lower_bounds");
		herr_t ret_code_ub = H5DSset_scale(dim_scale_ds_upper_bounds_id, "upper_bounds");
		if(ret_code_lb != 0 || ret_code_ub != 0) {
			//TODO Do something to handle the error.
		}

		dim_scale_ds_lower_bounds.close();
		dim_scale_ds_upper_bounds.close();

		/* Probability density datasets */

		hsize_t dims[1]; // dataset dimensions
		dims[0] = bin_number;
		DataSpace dataspace( 1, dims );
		
		for(int j = 1; j < probability_density_dataset_count; j++) {
			std::string probability_density_dataset_name = "resp_desc_" + std::to_string(j);
			DataSet probability_density_dataset = probability_density_group.createDataSet(probability_density_dataset_name, type_ieee_f64le, dataspace);
			
			probability_density_dataset.close();
		}
		
		probability_density_group.close();
		group_exec_id.close();
	}

	/* Close all this stuff */
	group_method.close();
	group_methods.close();
	file_ptr->close();
}
// #endif // comment to make this file active
