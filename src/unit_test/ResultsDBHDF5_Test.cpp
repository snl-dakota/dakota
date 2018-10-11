// #if 0 // comment to make this file active
#ifdef DAKOTA_HAVE_HDF5

#include <Teuchos_UnitTestHarness.hpp>

#include "hdf5.h"    // C API
#include "hdf5_hl.h" // C API (HDF5 "high-level")
#include "H5Cpp.h"   // C++ API
#include "H5Exception.h" // HDF5 exeptions

#include "dakota_global_defs.hpp"
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
  results_manager.initialize(database_name, Dakota::RESULTS_OUTPUT_HDF5);

  TEST_ASSERT( results_manager.active() );
}

TEUCHOS_UNIT_TEST(tpl_hdf5, test_create_groups) {
  std::string database_name = "database_2";

  Dakota::ResultsManager results_manager;
  results_manager.initialize(database_name, Dakota::RESULTS_OUTPUT_HDF5);

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

#endif
// #endif // comment to make this file active
