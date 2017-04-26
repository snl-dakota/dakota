include(DakotaUnitTest)
#include(FindBoost)

# List of test files to copy into this directory
set(dakota_unit_test_copied_files)

# --- Dakota dependencies
# It is desirable to split-up dakota into packages of re-useable components
# but that is a significant re-factoring effort in its own right, so
# link with the whole dakota library (for now)
#include_directories(${Dakota_SOURCE_DIR}/src)
include_directories(${Dakota_INCLUDE_DIRS}/src) # include tpl directories 
# TODO: make sure preprocessor defines agree with src/ and test/
# (manage via variable at top-level)

# Needed when linking with QUESO
link_directories(${Dakota_LIBRARY_DIRS})

# --- Boost dependencies
include_directories(${Boost_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
# TODO: Understand leveraging BOOST_DISABLE_ASSERTS wrt to Unit Testing
option(BOOST_DISABLE_ASSERTS "Toggle BoostAssert, default is AssertDisabled" ON)
if (BOOST_DISABLE_ASSERTS)
  add_definitions("-DBOOST_DISABLE_ASSERTS")
endif(BOOST_DISABLE_ASSERTS)

# ---
# Unit test targets
# ---

# Unit test: minimal Dakota unit test example using header-only Boost.Test
dakota_add_unit_test(NAME min_unit_test SOURCES min_unit_test.cpp
  LINK_DAKOTA_LIBS)


# Unit test: work directory utilities
# Copy workdir unit_test helper script into the build tree at build time
# NOTE:  at runtime the workdir test will look for the script in the parent dir
set(ut_helper_script "uthelper")
set(script_dest_path "${Dakota_BINARY_DIR}/src")
if(MSVC)
  set(script_ext ".bat")
  set(ut_helper_script ${ut_helper_script}${script_ext})
  set(script_dest_path ${CMAKE_CURRENT_BINARY_DIR})
endif(MSVC)

dakota_copy_test_file(${CMAKE_CURRENT_SOURCE_DIR}/${ut_helper_script}
  ${script_dest_path}/${ut_helper_script} dakota_unit_test_copied_files)

dakota_add_unit_test(NAME test_workdir_utils
  SOURCES test_workdir_utils.cpp LINK_DAKOTA_LIBS)


# Unit test: try BOOST_AUTO_TEST_CASE
dakota_add_unit_test(NAME auto_unit_test
  SOURCES auto_unit_test.cpp
  LINK_DAKOTA_LIBS
  # need team buy-in for Boost.Test library deps
  #LINK_LIBS boost_unit_test_framework
  )

dakota_add_unit_test(NAME bootstrap_util_test
  SOURCES bootstrap_util_test.cpp
  LINK_DAKOTA_LIBS
  )


# Unit test: experiment data covariance utilities
dakota_add_unit_test(NAME test_field_covariance_utils
  SOURCES test_field_covariance_utils.cpp
  LINK_DAKOTA_LIBS
  )

# Unit test: Leja sampling
dakota_add_unit_test(NAME leja_sampling
  SOURCES leja_sampling.cpp
  LINK_DAKOTA_LIBS
  )


# Unit test: experiment data and readers
# Demonstration of Teuchos test framework to driver several tests related to
# ExperimentData and associated file readers
option(DAKOTA_ENABLE_TEUCHOS_UNIT_TESTS "Enable Teuchos unit tests driver" ON)
remove_definitions("-DHAVE_CONFIG_H")
include_directories(${Teuchos_INCLUDE_DIRS})
# Pecos is an unconditional Dakota dependency
include_directories(${Pecos_SOURCE_DIR}/src)

if(DAKOTA_ENABLE_TEUCHOS_UNIT_TESTS)
  set(dakota_teuchos_unit_tests
    demo_teuchos.cpp
    covariance_reader.cpp
    expt_data.cpp
    expt_data_reader.cpp
    file_reader.cpp
    data_conversions.cpp
    opt_api_core_data_xfers.cpp
    opt_tpl_api.cpp
    reduced_basis.cpp
    stat_utils.cpp
    wilks_sampling.cpp
    )
  if (DAKOTA_HAVE_MPI)
    list(REMOVE_ITEM dakota_teuchos_unit_tests opt_tpl_api.cpp opt_tpl_test.cpp)
  endif(DAKOTA_HAVE_MPI)

  dakota_add_unit_test(NAME teuchos_unit_tests
    SOURCES teuchos_unit_test_driver.cpp ${dakota_teuchos_unit_tests}
    LINK_DAKOTA_LIBS
    )
  dakota_copy_test_file("${CMAKE_CURRENT_SOURCE_DIR}/expt_data_test_files"
    "${CMAKE_CURRENT_BINARY_DIR}/expt_data_test_files"
    dakota_unit_test_copied_files
    )
  dakota_copy_test_file("${CMAKE_CURRENT_SOURCE_DIR}/stat_util_test_files"
    "${CMAKE_CURRENT_BINARY_DIR}/stat_util_test_files"
    dakota_unit_test_copied_files
    )
  FILE( COPY
    "${CMAKE_CURRENT_SOURCE_DIR}/dakota_tabular_svd.dat"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
    )
endif() # DAKOTA_ENABLE_TEUCHOS_UNIT_TESTS


# Unit test: Dakota output to binary HDF5 (depends on Teuchos and HDF5)
if(DAKOTA_HAVE_HDF5)
  include_directories(${HDF5_INCLUDE_DIRS})
  link_directories(${Teuchos_LIBRARY_DIRS} ${HDF5_LIBRARY_DIRS})
  dakota_add_unit_test(NAME binary_io_test SOURCES binary_io_test.cpp
    LINK_LIBS ${Teuchos_LIBRARIES} ${TEUCHOS_LIBRARY}
              ${HDF5_HL_LIBRARY} ${HDF5_LIBRARIES}
    )
endif()


# Add a single target for all copied files
add_custom_target(dakota_unit_test_files ALL
  DEPENDS ${dakota_unit_test_copied_files})
