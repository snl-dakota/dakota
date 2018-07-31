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
TEUCHOS_UNIT_TEST(tpl_hdf5, new_hdf5_results_db) {
	std::string database_name = "database_1";

	Dakota::ResultsManager results_manager;
	results_manager.initialize(database_name);

	TEST_ASSERT( results_manager.active() );
	TEST_ASSERT( results_manager.isHDF5DBActive() );
}

TEUCHOS_UNIT_TEST(tpl_hdf5, create_one_group) {
	std::string database_name = "database_2";

	Dakota::ResultsManager results_manager;
	results_manager.initialize(database_name);

	Dakota::HDF5IOHelper helper(database_name + ".h5", true);
	helper.create_groups( "/methods");

	//TODO Read and make sure the group was written as expected.

	TEST_ASSERT( true ); //Successfully terminated
}

#endif
// #endif // comment to make this file active
