#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2025
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________


# This gets built in test/ so can be packaged in binaries even when
# not building docs

# Extract files for examples/users

include(AddFileCopyCommand)
include(DakotaUsersExamples)

set(test_path ${Dakota_SOURCE_DIR}/test)
set(all_generated_files)

# Each line is "generated file source file test number"
file(STRINGS "${Dakota_BINARY_DIR}/test/dakota_usersexamples.props"
  users_test_inputs)

# Supplementary data files needed and copied files below
set(test_data_files dakota_bayes_expdesign.cand.dat dakota_bayes_expdesign.dat 
  dakota_textbook_nips.lsq.dat dakota_bayes_linear.1.dat dakota_cantilever_queso.withsigma.dat)

foreach(data_file ${test_data_files})

  add_file_copy_command(${test_path}/${data_file} ${CMAKE_CURRENT_BINARY_DIR}/${data_file})
  list(APPEND all_generated_files ${CMAKE_CURRENT_BINARY_DIR}/${data_file})

endforeach()

generate_users_examples(users_test_inputs "${CMAKE_CURRENT_BINARY_DIR}"
  _gen_files_var_out)
list(APPEND all_generated_files ${_gen_files_var_out})

add_custom_target(examples-users ALL DEPENDS ${all_generated_files})

install(FILES ${all_generated_files}
  DESTINATION "${DAKOTA_EXAMPLES_INSTALL}/examples/users")
