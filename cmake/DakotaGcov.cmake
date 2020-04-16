macro(dakota_gcov_init)
  if(DAKOTA_GCOV)
    add_custom_target(gcov-clean
      COMMAND find . -name *.gcno -exec rm {} +
      COMMAND find . -name *.gcda -exec rm {} +
      WORKING_DIRECTORY ${Dakota_BINARY_DIR}
      )
  endif()
endmacro()

macro(dakota_gcov_target target_name)
  if(DAKOTA_GCOV)
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      message(ERROR "Can't do coverage without G++")
    endif()
    target_compile_options(${target_name} PRIVATE "--coverage")
    # Requires CMake 3.13
    target_link_options(${target_name} PRIVATE "--coverage")
  endif()
endmacro()
