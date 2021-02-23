#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014-2020
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________


include(DakotaTestHelpers)
include(FetchContent)

FetchContent_Declare( dakota_examples_repos
  GIT_REPOSITORY      https://gitlab-ex.sandia.gov/dakota/dakota-examples.git
  GIT_TAG             origin/dp394-examples_repos
  )
FetchContent_GetProperties(dakota_examples_repos)
if(NOT dakota_examples_repos_POPULATED)
  FetchContent_Populate(dakota_examples_repos)
endif()

#message(DEBUG "dakota_examples_repos_POPULATED: " ${dakota_examples_repos_POPULATED})
#message(DEBUG "dakota_examples_repos_SOURCE_DIR: " ${dakota_examples_repos_SOURCE_DIR})
#message(DEBUG "dakota_examples_repos_BINARY_DIR: " ${dakota_examples_repos_BINARY_DIR})

#file(GLOB_RECURSE dakota_examples_repos_test_input_files "${dakota_examples_repos_SOURCE_DIR}/official/deterministic_calibration/field_interpolate/*.in")
file(GLOB_RECURSE dakota_examples_repos_test_input_files "${dakota_examples_repos_SOURCE_DIR}/official/*.in")


# Will probably want a copy with dependency as in other test/ copies
# These are copied, not configured, to avoid picking up baselines in the source tree.
file(COPY
  "${Dakota_SOURCE_DIR}/test/dakota_test.perl"
  "${Dakota_SOURCE_DIR}/test/dakota_diff.perl"
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


# Bring in tests for other subdirs
set(used_suite_names)
set(all_exrepo_files)
foreach(extra_test_input_file ${dakota_examples_repos_test_input_files})
  get_filename_component(external_test_dir ${extra_test_input_file} DIRECTORY)
  file(GLOB suite_name_path 
    RELATIVE ${dakota_examples_repos_SOURCE_DIR} ${external_test_dir})
  string(REPLACE "/" "-" suite_name ${suite_name_path})
  if( NOT ${suite_name} IN_LIST used_suite_names )
    list(APPEND used_suite_names ${suite_name})
    dakota_add_regression_tests(${suite_name} ${external_test_dir})
  endif()
  list(APPEND all_exrepo_files "${${suite_name}_copied_files_abs}")
endforeach()

# Gather up all deps into a target (will need list of all deps)
add_custom_target(dakota_exrepo_files ALL
  DEPENDS ${all_exrepo_files}
  COMMENT "Copy example repo files into binary test subdirs"
  )
