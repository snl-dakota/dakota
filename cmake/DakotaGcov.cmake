macro(dakota_gcov_init)
  if(DAKOTA_GCOV)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Type of build" FORCE)
    add_link_options("-lgcov")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 --coverage" CACHE STRING "Dakota Dev Release Flags" FORCE)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 --coverage" CACHE STRING "Dakota Dev Release Flags" FORCE)
    set(CMAKE_Fortran_FLAGS_DEBUG "${CMAKE_Fortran_FLAGS_DEBUG} -O0 --coverage" CACHE STRING "Dakota Dev Release Flags" FORCE)

    add_custom_target(gcov-clean
      COMMAND find . -name *.gcda -exec rm {} +
      COMMAND find . -name *.gcov -exec rm {} +
      WORKING_DIRECTORY ${Dakota_BINARY_DIR}
      )
  message("Code coverage (gcov) enabled")
  endif()
endmacro()


