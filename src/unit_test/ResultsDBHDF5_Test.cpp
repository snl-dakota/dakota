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
#include <boost/tuple/tuple.hpp>
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

TEUCHOS_UNIT_TEST(tpl_hdf5, test_insert_into_matrix) {
  // TODO: This is not a very good test, yet. It passes if no exceptions are throw, but it doesn't read
  // the matrices back in to confirm that they were written correctly.
  std::string database_name = "database_3";
  Dakota::ResultsManager results_manager;
  results_manager.initialize(database_name, Dakota::RESULTS_OUTPUT_HDF5);
  Dakota::StrStrSizet iterator_id = boost::make_tuple(std::string("test_method"), 
                                                        std::string("test_method_id"), 1);
 
  std::vector<double> row0 = {1.0, 2.0, 3.0, 4.0};
  std::vector<double> row1 = {5.0, 6.0, 7.0, 8.0};
  Dakota::RealVector row2(4);
  for(int i = 0; i < 4; ++i)
    row2[i] = double(i) + 9.0;

  Dakota::DimScaleMap scales;
  scales.emplace(0, Dakota::StringScale("dim_0_test", {"a", "scale", "test"}));
  scales.emplace(1, Dakota::StringScale("dim_1_test", {"do", "re", "me", "fa"}));

  Dakota::AttributeArray attrs;
  attrs.push_back(Dakota::ResultAttribute<int>("samples", 5));

  results_manager.allocate_matrix(iterator_id, std::string("row_result"), std::string(""),
    Dakota::ResultsOutputType::REAL, 3, 4, scales, attrs);
  results_manager.insert_into_matrix(iterator_id, std::string("row_result"), std::string(""), row0, 0); 
  results_manager.insert_into_matrix(iterator_id, std::string("row_result"), std::string(""), row1, 1); 
  results_manager.insert_into_matrix(iterator_id, std::string("row_result"), std::string(""), row2, 2); 
  
  results_manager.allocate_matrix(iterator_id, std::string("column_result"), std::string(""),
    Dakota::ResultsOutputType::REAL, 4,3);
  results_manager.insert_into_matrix(iterator_id, std::string("column_result"), std::string(""), row0, 0, false); 
  results_manager.insert_into_matrix(iterator_id, std::string("column_result"), std::string(""), row1, 1, false); 
  results_manager.insert_into_matrix(iterator_id, std::string("column_result"), std::string(""), row2, 2, false); 
}
#endif
// #endif // comment to make this file active
