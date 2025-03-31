#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2024
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
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
# TODO: make sure preprocessor defines agree with src/ and test/
# (manage via variable at top-level)

# Needed when linking with QUESO
link_directories(${Dakota_LIBRARY_DIRS})

# ---
# Unit test targets
# ---

# Demo: minimal Dakota unit test example using header-only Boost.Test
add_subdirectory(dakota_min_unit)

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

add_subdirectory(dakota_auto_unit)

remove_definitions("-DHAVE_CONFIG_H")
include_directories(${Teuchos_INCLUDE_DIRS})

add_subdirectory(dakota_workdir_utils)

add_subdirectory(dakota_bootstrap_util)

add_subdirectory(dakota_field_covariance_utils)

add_subdirectory(dakota_tolerance_intervals)

add_subdirectory(dakota_leja_sampling)

add_subdirectory(dakota_lhs_constants)

add_subdirectory(dakota_redirect_regexs)

add_subdirectory(dakota_response_io)

add_subdirectory(dakota_file_reader)

if (HAVE_DEMO_TPL)
  add_subdirectory(dakota_opt_tpl_adapters)
endif()

add_subdirectory(dakota_opt_api_core_data_xfers)

add_subdirectory(dakota_opt_api_traits)

add_subdirectory(dakota_wilks_sampling)

add_subdirectory(dakota_covariance_reader)

add_subdirectory(dakota_expt_data)

add_subdirectory(dakota_expt_data_reader)

add_subdirectory(dakota_data_conversions)

add_subdirectory(dakota_stat_utils)

add_subdirectory(dakota_restart)

add_subdirectory(dakota_global_sa_metrics)

add_subdirectory(dakota_low_discrepancy_driver)

add_subdirectory(dakota_rank_1_lattice_test)

add_subdirectory(dakota_digital_net_test)

add_subdirectory(dakota_json_results_parser)

# Copy needed unit test auxiliary data files
dakota_copy_test_file("${CMAKE_CURRENT_SOURCE_DIR}/expt_data_test_files"
  "${CMAKE_CURRENT_BINARY_DIR}/expt_data_test_files"
  dakota_unit_test_copied_files
  )
dakota_copy_test_file("${CMAKE_CURRENT_SOURCE_DIR}/dakota_opt_tpl_rol_textbook/opt_tpl_test_files"
  "${CMAKE_CURRENT_BINARY_DIR}/dakota_opt_tpl_rol_textbook/opt_tpl_test_files"
  dakota_unit_test_copied_files
  )
dakota_copy_test_file("${CMAKE_CURRENT_SOURCE_DIR}/dakota_stat_utils/stat_util_test_files"
  "${CMAKE_CURRENT_BINARY_DIR}/dakota_stat_utils/stat_util_test_files"
  dakota_unit_test_copied_files
  )
FILE( COPY
  "${CMAKE_CURRENT_SOURCE_DIR}/dakota_surr_reduced_basis/dakota_tabular_svd.dat"
  DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/dakota_surr_reduced_basis"
  )

if (NOT DAKOTA_HAVE_MPI)
  add_subdirectory(dakota_opt_tpl_api)
endif (NOT DAKOTA_HAVE_MPI)


# Unit test: h5py_hdf5
if(DAKOTA_H5PY_FOUND)
  dakota_add_h5py_test(mixed_sampling)
  if (DAKOTA_MODULE_SURROGATES)
    dakota_add_h5py_test(incremental_sampling)
  endif()
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
  dakota_add_h5py_test(variable_categories_sampling)
  dakota_add_h5py_test(pce)
  #dakota_add_h5py_test(calibration_with_data)
  #dakota_add_h5py_test(mutlisolution_opt)
endif()


if (HAVE_ROL)
  add_subdirectory(dakota_opt_tpl_rol_textbook)

  add_subdirectory(dakota_opt_tpl_rol_pbs)

  add_subdirectory(dakota_rol_adapters)

  add_subdirectory(dakota_rol_integration)
endif (HAVE_ROL)


if (DAKOTA_MODULE_SURROGATES)
  add_subdirectory(dakota_surr_reduced_basis)

if (HAVE_ROL)
  add_subdirectory(dakota_surr_gauss_proc)
  dakota_copy_test_file("${CMAKE_CURRENT_SOURCE_DIR}/dakota_surr_gauss_proc/gauss_proc_test_files"
    "${CMAKE_CURRENT_BINARY_DIR}/dakota_surr_gauss_proc/gauss_proc_test_files"
    dakota_unit_test_copied_files)
endif (HAVE_ROL)

endif()


if(DAKOTA_HAVE_HDF5)
  include_directories(${HDF5_INCLUDE_DIRS})
  link_directories(${HDF5_LIBRARY_DIRS})

  add_subdirectory(dakota_hdf5_serial)

  add_subdirectory(dakota_hdf5_utils)

  add_subdirectory(dakota_hdf5_resultsDB)
endif()


if (HAVE_MUQ)
  add_subdirectory(dakota_muq_mcmc)
endif()

if(DAKOTA_TEST_PREPROC)
  add_subdirectory(dakota_preproc_tests)
  dakota_copy_test_file("${CMAKE_CURRENT_SOURCE_DIR}/dakota_preproc_tests/preproc_dakota.tmpl"
    "${CMAKE_CURRENT_BINARY_DIR}/dakota_preproc_tests/preproc_dakota.tmpl"
    dakota_unit_test_copied_files
    )
  dakota_copy_test_file("${CMAKE_CURRENT_SOURCE_DIR}/dakota_preproc_tests/preproc_dakota.base"
    "${CMAKE_CURRENT_BINARY_DIR}/dakota_preproc_tests/preproc_dakota.base"
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
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_dakota_python_env.py
    $<TARGET_FILE_DIR:environment>
    )
  set_property(TEST dakota_python_env PROPERTY LABELS Unit)
endif()


# Pecos is an unconditional Dakota dependency
include_directories(${Pecos_SOURCE_DIR}/src)


# Add a single target for all copied files
add_custom_target(dakota_unit_test_files ALL
  DEPENDS ${dakota_unit_test_copied_files})
