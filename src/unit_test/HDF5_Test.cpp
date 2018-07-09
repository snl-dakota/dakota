
#if 0// comment to make this file active

#include "H5Cpp.h" // C++ API header file
#include <string>

using namespace H5;

#define FILE "file.h5"

int main() {

	/* FIELDS */

	int  num_evaluations = 2;					//TODO: This should be an argument.
	int  probability_density_dataset_count = 3;	//TODO: This should be an argument.
	std::string  method_name = "sampling";		//TODO: This should be an argument.
	int  bin_number = 2;
	
	/* LOGIC */

	/* Create a new file using default properties.*/
	H5File* file = new H5File(FILE, H5F_ACC_TRUNC);

	/* Create methods group */
	Group* group_methods(file->createGroup("/methods"));

	/* Create specific method group */
	std::string group_method_path = "/methods/" + method_name;
	Group* group_method(group_methods->createGroup(group_method_path));

	/* Create execution groups */
	for(int i = 1; i <= num_evaluations; i++) {
		std::string exec_id_path = group_method_path + "/execution_id_" + std::to_string(i);
		Group* group_exec_id(group_method->createGroup(exec_id_path));

		/* Create probability density group */
		std::string probability_density_path = exec_id_path + "/probability_density";
		Group* probability_density_group(group_exec_id->createGroup(probability_density_path));
		
		/* Create probability density datasets */
				
		/* Scale dataset */
		PredType type_ieee_f64le( PredType::IEEE_F64LE );
		
		hsize_t scale_dims[1];
		scale_dims[0] = 2;
		DataSpace scales_dataspace( 1, scale_dims );		
		DataSet scales_dataset = probability_density_group->createDataSet("_scales", type_ieee_f64le, dataspace);
		
		int scales_data[2];
		scales_data[0] = "lower_bound";
		scales_data[1] = "upper_bound";
		
		VarLenType scales_type( PredType::C_S1 ); // ????
		scales_dataset.write( scales_data, scales_type );
		
		scales_dataset.close();
		
		hsize_t dims[1]; // dataset dimensions
		dims[0] = bin_number;
		DataSpace dataspace( 1, dims );
		
		for(int j = 1; j < probability_density_dataset_count; j++) {
			std::string probability_density_dataset_name = "resp_desc_" + std::to_string(j);
			DataSet probability_density_dataset = probability_density_group->createDataSet(probability_density_dataset_name, type_ieee_f64le, dataspace);
			
			probability_density_dataset->close();
		}
		
		probability_density_group->close();
		group_exec_id->close();
	}

	/* Close all this stuff */
	status = H5Gclose(group_methods_id);
	group_method->close();
	group_methods->close();
	file->close();
}
#endif // comment to make this file active
