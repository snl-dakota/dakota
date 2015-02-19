include(AddFileCopyCommand)

# Add a Dakota unit test, creating executable from sources, optionally
# linking Dakota libraries, and registering it with CTest
#
# NAME: the name of the test as seen by CTest, and executable target 
#       created
# SOURCES: source .cpp files
# LABELS: extra labels to apply to this test (UnitTest added by default)
# LINK_DAKOTA_LIBS: if specified will link against Dakota core libraries
#
# TODO: decide if separate args for name and target are needed
function(dakota_add_unit_test)

  set(options LINK_DAKOTA_LIBS)
  set(oneValueArgs NAME)
  set(multiValueArgs SOURCES LABELS DEPENDS LINK_LIBS)
  cmake_parse_arguments(DAUT 
    "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  #message(STATUS "Dakota adding unit test ${DAUT_NAME}")
  set(exe_target ${DAUT_NAME})
  add_executable(${exe_target} ${DAUT_SOURCES})
  if (${DAUT_LINK_DAKOTA_LIBS})
    target_link_libraries(${exe_target} 
      ${Dakota_LIBRARIES} ${Dakota_TPL_LIBRARIES})
  endif()
  if (DAUT_LINK_LIBS)
    target_link_libraries(${exe_target} ${DAUT_LINK_LIBS})
  endif()
  # TODO: support dependencies directly in this call with DEPENDS
  #add_dependencies(${exe_target} ${DAUT_DEPENDS})
  add_test(${DAUT_NAME} ${exe_target})
  set_property(TEST ${DAUT_NAME} PROPERTY LABELS UnitTest ${DAUT_LABELS})

endfunction()


# Add file to list of unit test dependency files for addition to
# higher level target.  This is a macro to make it simpler to set the
# calling envrionment variable.
macro(dakota_copy_test_file src_file dest_file dep_files_variable)
  add_file_copy_command(${src_file} ${dest_file})
  list(APPEND ${dep_files_variable} ${dest_file})
endmacro()