/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

// #if 0 // comment to make this file active
#ifdef DAKOTA_HAVE_HDF5

#include "util_windows.hpp"

#define BOOST_TEST_MODULE dakota_ResultsDBHDF5_Test
#include <boost/test/included/unit_test.hpp>

#include "hdf5.h"    // C API
#include "hdf5_hl.h" // C API (HDF5 "high-level")
#include "H5Cpp.h"   // C++ API
#include "H5Exception.h" // HDF5 exeptions

#include "dakota_global_defs.hpp"
#include "ResultsManager.hpp"
#include "HDF5_IO.hpp"
#include "ResultsDBHDF5.hpp"
#include <boost/tuple/tuple.hpp>
#include <iostream>
#include <math.h>
#include <memory>
#include <utility>
#include <string>

using namespace H5;
using namespace Dakota;
/**
 *  Test creating and destroying a Dakota HDF5 database.
 */
BOOST_AUTO_TEST_CASE(test_tpl_hdf5_test_results_manager_init) {
  std::string database_name = "database_1.h5";
 
  Dakota::ResultsManager results_manager;
  std::shared_ptr<HDF5IOHelper> hdf5_helper_ptr(new HDF5IOHelper(database_name, true));
  std::unique_ptr<ResultsDBHDF5> db_ptr(new ResultsDBHDF5(false /* in_core */, hdf5_helper_ptr));
  results_manager.add_database(std::move(db_ptr));

  BOOST_CHECK( results_manager.active() );
}

BOOST_AUTO_TEST_CASE(test_tpl_hdf5_test_create_groups) {
  std::string database_name = "database_2.h5";

  Dakota::ResultsManager results_manager;
  std::shared_ptr<HDF5IOHelper> hdf5_helper_ptr(new HDF5IOHelper(database_name, true));
  std::unique_ptr<ResultsDBHDF5> db_ptr(new ResultsDBHDF5(false /* in_core */, hdf5_helper_ptr));
  results_manager.add_database(std::move(db_ptr));

  Dakota::HDF5IOHelper helper(database_name, false);
  // methods treated like a dataset name
  helper.create_groups("/methods");
  BOOST_CHECK(!helper.exists("/methods"));

  // methods treated like a group
  helper.create_groups( "/methods",false );
  BOOST_CHECK( helper.exists("/methods") );

  // sampling treated like a dataset
  helper.create_groups( "/methods/sampling");
  BOOST_CHECK(!helper.exists("/methods/sampling") );

  // sampling treated like a group
  helper.create_groups( "/methods/sampling", false);
  BOOST_CHECK( helper.exists("/methods") );
  BOOST_CHECK( helper.exists("/methods/sampling") );
}

BOOST_AUTO_TEST_CASE(test_tpl_hdf5_test_insert_into) {
  // TODO: This is not a very good test, yet. It passes if no exceptions are throw, but it doesn't read
  // the matrices back in to confirm that they were written correctly.
  std::string database_name = "database_3.h5";
  Dakota::ResultsManager results_manager;
  std::shared_ptr<HDF5IOHelper> hdf5_helper_ptr(new HDF5IOHelper(database_name, true));
  std::unique_ptr<ResultsDBHDF5> db_ptr(new ResultsDBHDF5(false /* in_core */, hdf5_helper_ptr));
  results_manager.add_database(std::move(db_ptr));

  Dakota::StrStrSizet iterator_id = boost::make_tuple(std::string("test_method"), 
                                                        std::string("test_method_id"), 1);
 
  std::vector<double> row0 = {1.0, 2.0, 3.0, 4.0};
  std::vector<double> row1 = {5.0, 6.0, 7.0, 8.0};
  Dakota::RealVector row2(4);
  for(int i = 0; i < 4; ++i)
    row2[i] = double(i) + 9.0;

  Dakota::DimScaleMap m_scales;
  m_scales.emplace(0, Dakota::StringScale("dim_0_test", {"a", "scale", "test"}));
  m_scales.emplace(1, Dakota::StringScale("dim_1_test", {"do", "re", "me", "fa"}));

  Dakota::AttributeArray attrs;
  attrs.push_back(Dakota::ResultAttribute<int>("samples", 5));

  results_manager.allocate_matrix(iterator_id, {std::string("row_result")},
    Dakota::ResultsOutputType::REAL, 3, 4, m_scales, attrs);
  results_manager.insert_into(iterator_id, {std::string("row_result")},  row0, 0); 
  results_manager.insert_into(iterator_id, {std::string("row_result")},  row1, 1); 
  results_manager.insert_into(iterator_id, {std::string("row_result")},  row2, 2); 
  
  results_manager.allocate_matrix(iterator_id, {std::string("column_result")},
    Dakota::ResultsOutputType::REAL, 4,3);
  results_manager.insert_into(iterator_id, {std::string("column_result")},row0, 0, false); 
  results_manager.insert_into(iterator_id, {std::string("column_result")},row1, 1, false); 
  results_manager.insert_into(iterator_id, {std::string("column_result")},row2, 2, false); 

  Dakota::DimScaleMap v_scale;
  v_scale.emplace(0, Dakota::RealScale("a_scale", {1.0, 2.0, 3.0}));
  results_manager.allocate_vector(iterator_id, {std::string("vector_result")},
      Dakota::ResultsOutputType::REAL, 3, v_scale);
  for(int i = 0; i < 3; ++i)
    results_manager.insert_into(iterator_id, {std::string("vector_result")}, double(i+5), i);
}
#endif
// #endif // comment to make this file active
