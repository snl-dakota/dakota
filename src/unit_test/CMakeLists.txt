#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014-2022
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

include(DakotaUnitTest)

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

# ---
# Unit test targets
# ---

# Unit test: minimal Dakota unit test example using header-only Boost.Test
dakota_add_unit_test(NAME min_unit_test SOURCES min_unit_test.cpp
  LINK_DAKOTA_LIBS)
target_link_libraries(min_unit_test Boost::boost)

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
target_link_libraries(test_workdir_utils Boost::boost)

# Unit test: try BOOST_AUTO_TEST_CASE
dakota_add_unit_test(NAME auto_unit_test
  SOURCES auto_unit_test.cpp
  LINK_DAKOTA_LIBS
  # need team buy-in for Boost.Test library deps
  #LINK_LIBS boost_unit_test_framework
  )
target_link_libraries(auto_unit_test Boost::boost)

dakota_add_unit_test(NAME bootstrap_util_test
  SOURCES bootstrap_util_test.cpp
  LINK_DAKOTA_LIBS
  )
target_link_libraries(bootstrap_util_test Boost::boost)

# Unit test: experiment data covariance utilities
dakota_add_unit_test(NAME test_field_covariance_utils
  SOURCES test_field_covariance_utils.cpp
  LINK_DAKOTA_LIBS
  )
target_link_libraries(test_field_covariance_utils Boost::boost)

# Unit test: Leja sampling
dakota_add_unit_test(NAME leja_sampling
  SOURCES leja_sampling.cpp
  LINK_DAKOTA_LIBS
  )
target_link_libraries(leja_sampling Boost::boost)

dakota_add_unit_test(NAME lhs_constants
  SOURCES lhs_constants.cpp
  LINK_DAKOTA_LIBS
  )
target_link_libraries(lhs_constants Boost::boost)

# Unit test: h5py_hdf5
if(DAKOTA_H5PY_FOUND)
  dakota_add_h5py_test(mixed_sampling)
  dakota_add_h5py_test(incremental_sampling)
  if(HAVE_NPSOL)
    dakota_add_h5py_test(simple_calibration)
  endif()
  dakota_add_h5py_test(surrogate_sens)
  dakota_add_h5py_test(discrete_vps)
  dakota_add_h5py_test(multistart)
  dakota_add_h5py_test(pareto)
  dakota_add_h5py_test(dist_params)
  dakota_add_h5py_test(mlmc)
  dakota_add_h5py_test(sampling_metadata)
  #dakota_add_h5py_test(calibration_with_data)
  #dakota_add_h5py_test(mutlisolution_opt)
endif()

dakota_add_unit_test(NAME redirect_regexs
  SOURCES redirect_regexs.cpp
  LINK_DAKOTA_LIBS
  )
target_link_libraries(redirect_regexs Boost::boost)

dakota_add_unit_test(NAME response_io
  SOURCES response_io.cpp
  LINK_DAKOTA_LIBS
  )
target_link_libraries(response_io Boost::boost)

# Unit test: experiment data and readers
# Demonstration of Teuchos test framework to driver several tests related to
# ExperimentData and associated file readers
option(DAKOTA_ENABLE_TEUCHOS_UNIT_TESTS "Enable Teuchos unit tests driver" ON)
remove_definitions("-DHAVE_CONFIG_H")
include_directories(${Teuchos_INCLUDE_DIRS})
# Pecos is an unconditional Dakota dependency
include_directories(${Pecos_SOURCE_DIR}/src)

dakota_add_unit_test(NAME dakota_utils_boost_tests
  SOURCES file_reader.cpp
  LINK_DAKOTA_LIBS
  )
target_link_libraries(dakota_utils_boost_tests Boost::boost)

dakota_add_unit_test(NAME dakota_opt_tpl_adapters_boost_tests
  SOURCES opt_tpl_adapters.cpp
  LINK_DAKOTA_LIBS
  )
target_link_libraries(dakota_opt_tpl_adapters_boost_tests Boost::boost)

dakota_add_unit_test(NAME dakota_opt_api_core_data_xfers_boost_tests
  SOURCES opt_api_core_data_xfers.cpp
  LINK_DAKOTA_LIBS
  )
target_link_libraries(dakota_opt_api_core_data_xfers_boost_tests Boost::boost)

dakota_add_unit_test(NAME dakota_opt_api_traits_boost_tests
  SOURCES opt_api_traits.cpp
  LINK_DAKOTA_LIBS
  )
target_link_libraries(dakota_opt_api_traits_boost_tests Boost::boost)

if (NOT DAKOTA_HAVE_MPI)
  dakota_add_unit_test(NAME dakota_opt_tpl_api_boost_tests
    SOURCES opt_tpl_api.cpp
    LINK_DAKOTA_LIBS
    )
  target_link_libraries(dakota_opt_tpl_api_boost_tests Boost::boost)
endif (NOT DAKOTA_HAVE_MPI)

if (HAVE_ROL)

  dakota_add_unit_test(NAME dakota_opt_tpl_rol_test_textbook_boost_tests
    SOURCES opt_tpl_rol_test_textbook.cpp
    LINK_DAKOTA_LIBS
    )
  target_link_libraries(dakota_opt_tpl_rol_test_textbook_boost_tests Boost::boost)

  dakota_add_unit_test(NAME dakota_opt_tpl_rol_test_rol_pbs_boost_tests
    SOURCES opt_tpl_rol_test_rol_pbs.cpp
    LINK_DAKOTA_LIBS
    )
  target_link_libraries(dakota_opt_tpl_rol_test_rol_pbs_boost_tests Boost::boost)

  dakota_add_unit_test(NAME dakota_rol_adapters_boost_tests
    SOURCES rol_adapters.cpp
    LINK_DAKOTA_LIBS
    )
  target_link_libraries(dakota_rol_adapters_boost_tests Boost::boost)

  dakota_add_unit_test(NAME dakota_rol_integration_boost_tests
    SOURCES rol_integration.cpp
    LINK_DAKOTA_LIBS
    )
  target_link_libraries(dakota_rol_integration_boost_tests Boost::boost)

endif (HAVE_ROL)

dakota_add_unit_test(NAME dakota_wilks_sampling_boost_tests
  SOURCES wilks_sampling.cpp
  LINK_DAKOTA_LIBS
  )
target_link_libraries(dakota_wilks_sampling_boost_tests Boost::boost)

dakota_add_unit_test(NAME dakota_covariance_reader_boost_tests
  SOURCES covariance_reader.cpp
  LINK_DAKOTA_LIBS
  )
target_link_libraries(dakota_covariance_reader_boost_tests Boost::boost)

dakota_add_unit_test(NAME dakota_expt_data_boost_tests
  SOURCES expt_data.cpp
  LINK_DAKOTA_LIBS
  )
target_link_libraries(dakota_expt_data_boost_tests Boost::boost)

dakota_add_unit_test(NAME dakota_expt_data_reader_boost_tests
  SOURCES expt_data_reader.cpp
  LINK_DAKOTA_LIBS
  )
target_link_libraries(dakota_expt_data_reader_boost_tests Boost::boost)

dakota_add_unit_test(NAME dakota_data_conversions_boost_tests
  SOURCES data_conversions.cpp
  LINK_DAKOTA_LIBS
  )
target_link_libraries(dakota_data_conversions_boost_tests Boost::boost)

dakota_add_unit_test(NAME dakota_stat_utils_boost_tests
  SOURCES stat_utils.cpp
  LINK_DAKOTA_LIBS
  )
target_link_libraries(dakota_stat_utils_boost_tests Boost::boost)

dakota_add_unit_test(NAME dakota_restart_test_boost_tests
  SOURCES restart_test.cpp
  LINK_DAKOTA_LIBS
  )
target_link_libraries(dakota_restart_test_boost_tests Boost::boost)

if (DAKOTA_MODULE_SURROGATES)
  dakota_add_unit_test(NAME dakota_reduced_basis_boost_tests
    SOURCES reduced_basis.cpp
    LINK_DAKOTA_LIBS
    )
  target_link_libraries(dakota_reduced_basis_boost_tests dakota_surrogates Boost::boost)

  dakota_add_unit_test(NAME dakota_gauss_proc_test_boost_tests
    SOURCES gauss_proc_test.cpp
    LINK_DAKOTA_LIBS
    )
  target_link_libraries(dakota_gauss_proc_test_boost_tests dakota_surrogates Boost::boost)

  dakota_copy_test_file("${CMAKE_CURRENT_SOURCE_DIR}/gauss_proc_test_files"
    "${CMAKE_CURRENT_BINARY_DIR}/gauss_proc_test_files"
    dakota_unit_test_copied_files
  )
endif()

if(DAKOTA_ENABLE_TEUCHOS_UNIT_TESTS)
  set(dakota_utils_unit_tests
    demo_teuchos.cpp
    )
  dakota_add_unit_test(NAME utils_unit_tests
    SOURCES teuchos_unit_test_driver.cpp ${dakota_utils_unit_tests}
    LINK_DAKOTA_LIBS
    )
  target_link_libraries(utils_unit_tests Boost::boost)

  if (DAKOTA_HAVE_MUQ)
    dakota_add_unit_test(NAME muq_mcmc
      SOURCES teuchos_unit_test_driver.cpp muq_mcmc.cpp
      LINK_DAKOTA_LIBS
      )
    target_link_libraries(muq_mcmc Boost::boost)
  endif()
  dakota_copy_test_file("${CMAKE_CURRENT_SOURCE_DIR}/expt_data_test_files"
    "${CMAKE_CURRENT_BINARY_DIR}/expt_data_test_files"
    dakota_unit_test_copied_files
    )
  dakota_copy_test_file("${CMAKE_CURRENT_SOURCE_DIR}/opt_tpl_test_files"
    "${CMAKE_CURRENT_BINARY_DIR}/opt_tpl_test_files"
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

# Unit test: Dakota output to binary HDF5 (depends on Teuchos and HDF5)
  if(DAKOTA_HAVE_HDF5)
    include_directories(${HDF5_INCLUDE_DIRS})
    link_directories(${Teuchos_LIBRARY_DIRS} ${HDF5_LIBRARY_DIRS})
    dakota_add_unit_test(NAME hdf5_tests SOURCES teuchos_unit_test_driver hdf5_test_serial.cpp hdf5_utils.cpp ResultsDBHDF5_Test.cpp
      LINK_DAKOTA_LIBS
      )
    target_link_libraries(hdf5_tests Boost::boost)
  endif() # DAKOTA_HAVE_HDF5


endif() # DAKOTA_ENABLE_TEUCHOS_UNIT_TESTS

if(DAKOTA_TEST_PREPROC)
  dakota_add_unit_test(NAME preproc_tests SOURCES test_preproc.cpp
    LINK_DAKOTA_LIBS)
  target_link_libraries(preproc_tests Boost::filesystem Boost::boost)
  dakota_copy_test_file("${CMAKE_CURRENT_SOURCE_DIR}/preproc_dakota.tmpl"
    "${CMAKE_CURRENT_BINARY_DIR}/preproc_dakota.tmpl"
    dakota_unit_test_copied_files
    )
  dakota_copy_test_file("${CMAKE_CURRENT_SOURCE_DIR}/preproc_dakota.base"
    "${CMAKE_CURRENT_BINARY_DIR}/preproc_dakota.base"
    dakota_unit_test_copied_files
    )
endif()


# Test top-level dakota.dakota module
if(DAKOTA_PYTHON_WRAPPER)
  dakota_copy_test_file("${CMAKE_CURRENT_SOURCE_DIR}/test_dakota_python_env.in"
    "${CMAKE_CURRENT_BINARY_DIR}/test_dakota_python_env.in"
    dakota_unit_test_copied_files
    )
  add_test(NAME dakota_python_env
    COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_dakota_python_env.py
    $<TARGET_FILE_DIR:environment>
    )
  set_property(TEST dakota_python_env PROPERTY LABELS Unit)
endif()



# Add a single target for all copied files
add_custom_target(dakota_unit_test_files ALL
  DEPENDS ${dakota_unit_test_copied_files})
