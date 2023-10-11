#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014-2023
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

# Demo: minimal Dakota unit test example using header-only Boost.Test
dakota_add_unit_test(NAME dakota_min_unit
  SOURCES min_unit_test.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

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

# Demo: BOOST_AUTO_TEST_CASE
dakota_add_unit_test(NAME dakota_auto_unit
  SOURCES auto_unit_test.cpp
  LINK_DAKOTA_LIBS
  # need team buy-in for Boost.Test library deps
  #LINK_LIBS boost_unit_test_framework
  LINK_LIBS Boost::boost)

remove_definitions("-DHAVE_CONFIG_H")
include_directories(${Teuchos_INCLUDE_DIRS})

dakota_add_unit_test(NAME dakota_workdir_utils
  SOURCES test_workdir_utils.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

dakota_add_unit_test(NAME dakota_bootstrap_util
  SOURCES bootstrap_util_test.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

dakota_add_unit_test(NAME dakota_field_covariance_utils
  SOURCES test_field_covariance_utils.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

dakota_add_unit_test(NAME dakota_tolerance_intervals
  SOURCES test_tolerance_intervals.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

dakota_add_unit_test(NAME dakota_leja_sampling
  SOURCES leja_sampling.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

dakota_add_unit_test(NAME dakota_lhs_constants
  SOURCES lhs_constants.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

dakota_add_unit_test(NAME dakota_redirect_regexs
  SOURCES redirect_regexs.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

dakota_add_unit_test(NAME dakota_response_io
  SOURCES response_io.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

dakota_add_unit_test(NAME dakota_file_reader
  SOURCES file_reader.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

if (HAVE_DEMO_TPL)
  dakota_add_unit_test(NAME dakota_opt_tpl_adapters
    SOURCES opt_tpl_adapters.cpp
    LINK_DAKOTA_LIBS
    LINK_LIBS Boost::boost)
endif()

dakota_add_unit_test(NAME dakota_opt_api_core_data_xfers
  SOURCES opt_api_core_data_xfers.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

dakota_add_unit_test(NAME dakota_opt_api_traits
  SOURCES opt_api_traits.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

dakota_add_unit_test(NAME dakota_wilks_sampling
  SOURCES wilks_sampling.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

dakota_add_unit_test(NAME dakota_covariance_reader
  SOURCES covariance_reader.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

dakota_add_unit_test(NAME dakota_expt_data
  SOURCES expt_data.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

dakota_add_unit_test(NAME dakota_expt_data_reader
  SOURCES expt_data_reader.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

dakota_add_unit_test(NAME dakota_data_conversions
  SOURCES data_conversions.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

dakota_add_unit_test(NAME dakota_stat_utils
  SOURCES stat_utils.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

dakota_add_unit_test(NAME dakota_restart
  SOURCES restart_test.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

dakota_add_unit_test(NAME dakota_global_sa_metrics
  SOURCES global_sa_metrics.cpp
  LINK_DAKOTA_LIBS
  LINK_LIBS Boost::boost)

# Copy needed unit test auxiliary data files
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


if (NOT DAKOTA_HAVE_MPI)
  dakota_add_unit_test(NAME dakota_opt_tpl_api
    SOURCES opt_tpl_api.cpp
    LINK_DAKOTA_LIBS
    LINK_LIBS Boost::boost)
endif (NOT DAKOTA_HAVE_MPI)


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
  dakota_add_h5py_test(variable_categories_sampling)
  dakota_add_h5py_test(pce)
  #dakota_add_h5py_test(calibration_with_data)
  #dakota_add_h5py_test(mutlisolution_opt)
endif()


if (HAVE_ROL)
  dakota_add_unit_test(NAME dakota_opt_tpl_rol_textbook
    SOURCES opt_tpl_rol_test_textbook.cpp
    LINK_DAKOTA_LIBS
    LINK_LIBS Boost::boost)

  dakota_add_unit_test(NAME dakota_opt_tpl_rol_pbs
    SOURCES opt_tpl_rol_test_rol_pbs.cpp
    LINK_DAKOTA_LIBS
    LINK_LIBS Boost::boost)

  dakota_add_unit_test(NAME dakota_rol_adapters
    SOURCES rol_adapters.cpp
    LINK_DAKOTA_LIBS
    LINK_LIBS Boost::boost)

  dakota_add_unit_test(NAME dakota_rol_integration
    SOURCES rol_integration.cpp
    LINK_DAKOTA_LIBS
    LINK_LIBS Boost::boost)
endif (HAVE_ROL)


if (DAKOTA_MODULE_SURROGATES)
  dakota_add_unit_test(NAME dakota_surr_reduced_basis
    SOURCES reduced_basis.cpp
    LINK_DAKOTA_LIBS
    LINK_LIBS dakota_surrogates Boost::boost)

  dakota_add_unit_test(NAME dakota_surr_gauss_proc
    SOURCES gauss_proc_test.cpp
    LINK_DAKOTA_LIBS
    LINK_LIBS dakota_surrogates Boost::boost)

  dakota_copy_test_file("${CMAKE_CURRENT_SOURCE_DIR}/gauss_proc_test_files"
    "${CMAKE_CURRENT_BINARY_DIR}/gauss_proc_test_files"
    dakota_unit_test_copied_files)
endif()


if(DAKOTA_HAVE_HDF5)
  include_directories(${HDF5_INCLUDE_DIRS})
  link_directories(${HDF5_LIBRARY_DIRS})

  dakota_add_unit_test(NAME dakota_hdf5_serial
    SOURCES hdf5_test_serial.cpp
    LINK_DAKOTA_LIBS
    LINK_LIBS Boost::boost)

  dakota_add_unit_test(NAME dakota_hdf5_utils
    SOURCES hdf5_utils.cpp
    LINK_DAKOTA_LIBS
    LINK_LIBS Boost::boost)

  dakota_add_unit_test(NAME dakota_hdf5_resultsDB
    SOURCES ResultsDBHDF5_Test.cpp
    LINK_DAKOTA_LIBS
    LINK_LIBS Boost::boost)
endif()


if (HAVE_MUQ)
  dakota_add_unit_test(NAME dakota_muq_mcmc
    SOURCES muq_mcmc.cpp
    LINK_DAKOTA_LIBS
    LINK_LIBS Boost::boost)
endif()


if(DAKOTA_TEST_PREPROC)
  dakota_add_unit_test(NAME dakota_preproc_tests
    SOURCES test_preproc.cpp
    LINK_DAKOTA_LIBS
    LINK_LIBS Boost::filesystem Boost::boost)
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


# Pecos is an unconditional Dakota dependency
include_directories(${Pecos_SOURCE_DIR}/src)


# Add a single target for all copied files
add_custom_target(dakota_unit_test_files ALL
  DEPENDS ${dakota_unit_test_copied_files})
