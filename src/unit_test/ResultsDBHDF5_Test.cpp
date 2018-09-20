// #if 0 // comment to make this file active
#ifdef DAKOTA_HAVE_HDF5

#include <Teuchos_UnitTestHarness.hpp>

#include "hdf5.h"    // C API
#include "hdf5_hl.h" // C API (HDF5 "high-level")
#include "H5Cpp.h"   // C++ API
#include "H5Exception.h" // HDF5 exeptions

#include "ResultsManager.hpp"
#include "HDF5_IO.hpp"

#include <iostream>
#include <math.h>
#include <memory>
#include <string>

using namespace H5;

/**
 *  Test creating and destroying a Dakota HDF5 database.
 */
TEUCHOS_UNIT_TEST(tpl_hdf5, test_results_manager_init) {
  std::string database_name = "database_1";

  Dakota::ResultsManager results_manager;
  results_manager.initialize(database_name);

  TEST_ASSERT( results_manager.active() );
  TEST_ASSERT( results_manager.isHDF5DBActive() );
}

TEUCHOS_UNIT_TEST(tpl_hdf5, test_create_groups) {
  std::string database_name = "database_2";

  Dakota::ResultsManager results_manager;
  results_manager.initialize(database_name);

  Dakota::HDF5IOHelper helper(database_name + ".h5", false);
  // methods treated like a dataset name
  helper.create_groups("/methods");
  TEST_ASSERT(!helper.exists("/methods"));

  // methods treated like a group
  helper.create_groups( "/methods",false );
  TEST_ASSERT( helper.exists("/methods") );

  // sampling treated like a dataset
  helper.create_groups( "/methods/sampling");
  TEST_ASSERT(!helper.exists("/methods/sampling") );

  // sampling treated like a group
  helper.create_groups( "/methods/sampling", false);
  TEST_ASSERT( helper.exists("/methods") );
  TEST_ASSERT( helper.exists("/methods/sampling") );
}

TEUCHOS_UNIT_TEST(tpl_hdf5, test_create_1D_dimension_scales) {
  std::string database_name = "database_3";

  Dakota::ResultsManager results_manager;
  results_manager.initialize(database_name);

  Dakota::HDF5IOHelper helper(database_name + ".h5", false);
  H5::Group group = helper.create_groups( "/exec_id_1", false);

  std::array<double, 4> lower_bounds_arr = {
    2.7604749078e+11, 3.6e+11, 4.0e+11, 4.4e+11
  };	
  H5::DataSet ds_lower_bounds = helper.create_1D_dimension_scale (
    group, lower_bounds_arr.size(),
    Dakota::h5_file_dtype( lower_bounds_arr[0] ),
    "lower_bounds", helper.datasetContiguousPL
  );
  ds_lower_bounds.write( lower_bounds_arr.data(), PredType::NATIVE_DOUBLE );

  TEST_ASSERT( helper.exists("/exec_id_1") );

  /* Test lower_bounds dimension scale.
	H5::DataSet dataset_lower_bounds = file_ptr->openDataSet("/exec_id_1");
    double data_out[4];
    hsize_t dimsm[1];  // memory space dimensions
    dimsm[0] = 4;
    DataSpace memspace( 1, dimsm );
    DataSpace dataspace = dataset_lower_bounds.getSpace();

    dataset_lower_bounds.read(
      data_out, PredType::NATIVE_DOUBLE, memspace, dataspace
    );
    TEST_FLOATING_EQUALITY( data_out[0], 2.7604749078e+11, TOL );
    TEST_FLOATING_EQUALITY( data_out[1], 3.6e+11, TOL );
    TEST_FLOATING_EQUALITY( data_out[2], 4.0e+11, TOL );
    TEST_FLOATING_EQUALITY( data_out[3], 4.4e+11, TOL );
  */
}

#endif
// #endif // comment to make this file active
